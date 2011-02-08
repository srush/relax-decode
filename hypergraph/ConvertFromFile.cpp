
#include "Weights.h"
#include "Tagger.h"
#include <HypergraphAlgorithms.h>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>

#include  "hypergraph_pb2"
#include "features_pb2.h"
#include "tag_pb2.h" 
#include "dep_pb2.h" 

using namespace std;
int main(int argc, char ** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  string root = "/home/srush/Projects/relax_decode/data_management/";
  int sent = -1;
  int last_node = -1;
  string name = argv[1];
  //open( name , "w").close()
  Hypergraph * h;
  vector <Hypergraph_Node *> nodes;
  int cur_edge_id = 0;
  while (cin) {
    string t1;
    cin >> t1;
    //t = l.strip().split();
    if (t1 == "START") {
      h = new Hypergraph();
      cur_edge_id = 0 ;
      sent +=1;
      nodes.clear();
    }  else if (t1 == "END") {
      h.root = last_node;
      //print "ROOT NODE", last_node;
      stringstream file_name;
      file_name << name << sent;
      fstream output(file_name.str(), ios::out | ios::binary);

      h->SerializeToOstream(&output);
    
      output.close();
    }

    else if ( t1 == "EDGE") {
      
      int to_id, from_id; // = int(t[4]);
      double cost;
      string label;
      
      cin >> label >> to_id >> from_id >> cost;
      Hypergraph_Edge * edge = nodes[from_id].edge.add_mutable();
      edge->SetExtension(edge_fv,  "value="+ cost);
      edge->tail_node_ids.append( to_id);
      edge->set_label(label);
    
      edge->set_id( cur_edge_id);
      cur_edge_id += 1;

    } else if (t1 == "NODE") {
      Hypergraph_Node * node = h.node.add_mutable();
      int t2;
      string t3;
      cin >> t2 >> t3;
      node->set_id(t2);
      node->set_label(t3);
      if (nodes.size()<=node->id()) { 
        nodes.resize(node->id());
      }
      nodes[node->id()] = node;
      last_node = node->id();
    }
  }
  google::protobuf::ShutdownProtobufLibrary();
}
