
#include "Weights.h"
#include "Tagger.h"
#include <HypergraphAlgorithms.h>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string>

#include "hypergraph.pb.h"
#include "features.pb.h"
#include "tag.pb.h"
#include "dep.pb.h" 
#include <google/protobuf/io/coded_stream.h>
using namespace google::protobuf::io;
using namespace std;
int main(int argc, char ** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  //string root = "/home/srush/Projects/relax_decode/data_management/";
  int sent = -1;
  int last_node = -1;
  string name = argv[1];
  //open( name , "w").close()
  Hypergraph * h;
  vector <Hypergraph_Node *> nodes(10);
  int cur_edge_id = 0;
  fstream in(argv[2], ios::in | ios::binary);
  Hypergraph_Node * node;
  Hypergraph_Edge * edge;
  //CodedOutputStream::SetTotalBytesLimit(5000000000, 5000000000);
  while (in) {
    string blank;
    string t1;
    in >> blank >> t1;
    //t = l.strip().split();
    if (t1 == "START") {
      h = new Hypergraph();
      cur_edge_id = 0 ;
      sent +=1;
      nodes.clear();
    }  else if (t1 == "END") {
      h->set_root( last_node);
      //print "ROOT NODE", last_node;
      stringstream file_name;
      file_name << name << sent;
      fstream output(file_name.str().c_str(), ios::out | ios::binary);
      cout << "writing " << endl;
      h->SerializeToOstream(&output);
      cout << "done" << endl;
      output.close();
    } else if ( t1 == "EDGE") {
      
      int to_id, from_id; // = int(t[4]);
      double cost;
      string label;

      in >> label >> from_id >> to_id >> cost;
      //cout << "edge " <<label << " "  << from_id << " " << to_id << endl;      
      edge = nodes[from_id]->add_edge();
      stringstream fv;
      fv << "value=" << cost;
      edge->SetExtension(edge_fv,  fv.str());
      edge->add_tail_node_ids( to_id);
      edge->set_label(label);
    
      edge->set_id( cur_edge_id);
      cur_edge_id += 1;

    } else if (t1 == "NODE") {
      node = h->add_node();
      int t2;
      string t3;
      in >> t2 >> t3;
      //cout << "node " <<t2 << " "  << t3 << endl;
      node->set_id(t2);
      node->set_label(t3);
      if (nodes.size()<=node->id()) { 
        nodes.resize(node->id()+1);
      }
      nodes[t2] = node;
      last_node = node->id();
      

      if (t3 != "final" and t3 != "START") {
        Tagging * ta = node.MutableExtension(tagging);

        std::vector<std::string> strs;
        boost::split(strs, t3, boost::is_any_of(":"));
        ta->set_ind(atoi(strs[0].c_str()));
        ta->set_tag_id(atoi(strs[1].c_str()));
                
        node.SetExtension(has_tagging, true);
      }
              

    }
  }
  google::protobuf::ShutdownProtobufLibrary();
}
