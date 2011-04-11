// Special Lines

// Sent 
// #SENT [sent num] [words] [nodes] [edges] 

// Hypernodes 
// #I [node num] [leftspan] [right span] [non-terminal sym]

// Hyperedges - Two lines (below hypernode)
// [viterbi best prob?] [non-term nodes below]  [hnode for rule] ... [rule num] [headsym] [rule] ...
// [LM] [other features...] 

// In example2, features are 

// 0 - LM
// 1 - 1st rule feat
// 2 - 2nd rule feat
// 3 - 3rd rule feat
// 4 - Length penalty


#include "Weights.h"
#include "Tagger.h"
#include <HypergraphAlgorithms.h>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string>


#include "features.pb.h"
#include "translation.pb.h"
#include "hypergraph.pb.h"
#include "lexical.pb.h"
#include <google/protobuf/io/coded_stream.h>
#include "../CommandLine.h"

using namespace google::protobuf::io;
using namespace google;
using namespace std;

DEFINE_string(joshua_out_file, "", "The output  of joshua hypergraph serializer");
DEFINE_string(hypergraph_prefix, "", "Prefix of the hypergraphs to write"); 
static const bool forest_dummy = RegisterFlagValidator(&FLAGS_joshua_out_file, &ValidateReq);
static const bool hyper_dummy = RegisterFlagValidator(&FLAGS_hypergraph_prefix, &ValidateReq);

int main(int argc, char ** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::ParseCommandLineFlags(&argc, &argv, true);

  int sent = -1;
  int last_nonterm_node = -1;
  string name = FLAGS_hypergraph_prefix;
  Hypergraph * h;
  vector <Hypergraph_Node *> nodes(10);
  int cur_edge_id = 0;
  int cur_word_node_id = 0;
  ifstream in(FLAGS_joshua_out_file.c_str(), ios::in | ios::binary);
  Hypergraph_Node * node;
  Hypergraph_Edge * edge;
  int sent_num, length, num_nodes,num_edges;
  //CodedOutputStream::SetTotalBytesLimit(5000000000, 5000000000);
  while (in) {
    string blank;
    string t1;
    in >> t1;
    //t = l.strip().split();
    if (t1 == "#SENT:") {
      // flush last sent
      if (sent!= -1) {

        // need to add <s> and </s>
        int subroot = last_nonterm_node;
        int newroot;

        {
          node = h->add_node();
          edge = node->add_edge();
          Hypergraph_Node * wnode;
          for (int start=0; start < 2; start++) {
            wnode = h->add_node();
            wnode->set_id(cur_word_node_id);
            wnode->SetExtension(is_word, true);
            wnode->SetExtension(word, "<s>");
            edge->add_tail_node_ids(cur_word_node_id);
            cur_word_node_id++;
          }
          edge->add_tail_node_ids(subroot);
          for (int end=0; end < 2; end++) {
            wnode = h->add_node();
            wnode->set_id(cur_word_node_id);
            wnode->SetExtension(is_word, true);
            wnode->SetExtension(word, "</s>");
            edge->add_tail_node_ids(cur_word_node_id);
            cur_word_node_id++;
          }
          node->set_id(cur_word_node_id);
          h->set_root(cur_word_node_id);
          cur_word_node_id++;
        }

        stringstream file_name;
        file_name << name << sent;
        fstream output(file_name.str().c_str(), ios::out | ios::binary);
        h->SerializeToOstream(&output);
        output.close();
      }

      // prep new sent
      h = new Hypergraph();
      cur_edge_id = 0 ;
      sent +=1;
      nodes.clear();

      string buf;
      in >> sent_num >> length >> num_nodes >> num_edges; 
      // need to add nodes for each word
      cur_word_node_id = num_nodes;
      getline(in, buf);

    } else if (t1 == "#I") {
      node = h->add_node();
      int id, left_span, right_span;
      string sym;
      string ig1, ig2, ig3;
      in >> id >> left_span >> right_span >> sym >> ig1 >> ig2 >> ig3;

      stringstream label;
      label << id -1 << " ["<< left_span << ", "<<right_span << "] " <<sym;
      node->set_id(id-1);
      node->set_label(label.str());
      if (nodes.size()<=node->id()) { 
        nodes.resize(node->id()+1);
      }
      nodes[node->id()] = node;
      last_nonterm_node = node->id();      
      cout << "Node id " << node->id() << endl;
    } else {
      double best_prob, num_tail;
      edge = nodes[last_nonterm_node]->add_edge();
      cout << "EDGE " << endl;
      in >> num_tail; 
      vector < int > tail_nodes;
      for (int t = 0; t < num_tail; t++) {
        int tail;
        in >> tail;
        tail_nodes.push_back(tail - 1 );
        //edge->add_tail_node_ids( tail);
      }

      int rulenum;
      string s;
      getline(in, s);
      istringstream rule_s(s);
      vector <string> rhs;
      string lhs;
      rule_s >> rulenum;
      int oov_count =0;
      if (rulenum == -1) {
        edge->add_tail_node_ids( tail_nodes[0]);
      } else {
        rule_s >> lhs ;
        //int o =0;
        while (rule_s) {
          string r;
          rule_s >> r;
          if (r=="") break;
          rhs.push_back(r);
          if (r[0] == '[') {
            // HACK! (assume that there are <10 rules and sym are one letter
            int o = (int)(r[3] -'0')-1;
            // has a node
            edge->add_tail_node_ids( tail_nodes[o]);
            cout << "TAIL " << tail_nodes[o] << endl;
            //o++;
          }  else {
            // make a new word node
            
            node = h->add_node();
            stringstream label;
            label << cur_word_node_id << " " << r;
            node->set_id(cur_word_node_id);
            node->set_label(label.str());
            node->SetExtension(is_word, true);
            node->SetExtension(word, r);
            assert(r!="");
            edge->add_tail_node_ids( cur_word_node_id);
            cout << "WTAIL " << cur_word_node_id << endl;
            cur_word_node_id++;

            string oov = "_OOV";
            // check ends with
            if (std::equal(oov.rbegin(), oov.rend(), r.rbegin())) {
              oov_count++;
            }

          }
        }
      }
      
      vector <double> feat_vals; 
      for (int f=0; f < 5; f++) {
        double t;
        in >> t;
        feat_vals.push_back(t);
      }


      stringstream fv;
      fv << "phrasemodel0=" << feat_vals[1] << " phrasemodel1=" << feat_vals[2] << " phrasemodel2=" << feat_vals[3] << " text-length=" << feat_vals[4] << " unilm=" << feat_vals[0] << " oov=" << oov_count;
      edge->SetExtension(edge_fv,  fv.str());

      edge->set_label(s);
    
      edge->set_id( cur_edge_id);
      cur_edge_id += 1;
    } 

  }
  google::protobuf::ShutdownProtobufLibrary();
}
