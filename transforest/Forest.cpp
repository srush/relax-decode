#include "Forest.h"
#include "features.pb.h"
#include "translation.pb.h"
#include "hypergraph.pb.h"
#include "lexical.pb.h"

#include <iomanip>
#include <vector>
#include "cy_svector.hpp"

using namespace std;

void Forest::append_end_nodes() {
  ForestNode * node = (ForestNode *)_root;

  Scarab::HG::HyperedgeImpl * edge =  (Scarab::HG::HyperedgeImpl * ) node->_edges[0];
  str_vector * features = svector_from_str<int, double>("");
  ForestNode * s1 = new ForestNode("", num_nodes(), features, "<s>", true);   
  s1->add_in_edge(edge);
  _nodes.push_back(s1);
  ForestNode * s2 = new ForestNode("", num_nodes(), features, "<s>", true);   
  s2->add_in_edge(edge);
_nodes.push_back(s2);
  ForestNode * se1 = new ForestNode("", num_nodes(), features, "</s>", true);   
  se1->add_in_edge(edge);
  _nodes.push_back(se1);
  ForestNode * se2 = new ForestNode("", num_nodes(), features, "</s>", true);   
  se2->add_in_edge(edge);
  _nodes.push_back(se2);
  

  edge->_tail_nodes.resize(5);
  edge->_tail_nodes[2] = edge->_tail_nodes[0];
  edge->_tail_nodes[0] = s1;
  edge->_tail_nodes[1] = s2;
  edge->_tail_nodes[3] = se1;
  edge->_tail_nodes[4] = se2;
}


Scarab::HG::Hypernode* Forest::make_node(const Hypergraph_Node & node, wvector * features) {
  ForestNode * forest_node = 
    new ForestNode(node.label(), node.id(), features, node.GetExtension(word), node.GetExtension(is_word)); 
  //assert(((ForestNode*)_nodes[forest_node->id()])->is_word() == node.GetExtension(is_word));  
  //cout << "make node working!" << endl;
  return forest_node;
}



//   ::Hypergraph hgraph;
    
//   {
//     GOOGLE_PROTOBUF_VERIFY_VERSION;
//     fstream input(file_name, ios::in | ios::binary);
//     if (!hgraph.ParseFromIstream(&input)) {
//       assert (false);
//     } 
//   }
//   //return Forest(hgraph);

//   assert (hgraph.node_size() > 0);
//   for (int i = 0; i < hgraph.node_size(); i++) {
//     const Hypergraph_Node & node = hgraph.node(i);
    
//     string feat_str = node.GetExtension(node_fv);
//     str_vector * features = svector_from_str<int, double>(feat_str);
    
//     ForestNode * forest_node = 
//       new ForestNode(node.label(), node.id(), features, node.GetExtension(word), node.GetExtension(is_word)); 
//     //assert (forest_node->
//     assert (_nodes.size() == (uint)node.id());
//     _nodes.push_back(forest_node);
//     assert(_nodes[forest_node->id()]->id() == forest_node->id());
//     assert(((ForestNode*)_nodes[forest_node->id()])->is_word() == node.GetExtension(is_word));  
//   }

//   int edge_id = 0;
//   for (int i = 0; i < hgraph.node_size(); i++) {
//     const Hypergraph_Node& node = hgraph.node(i);
//     assert (node.id()  == i);

//     for (int j=0; j < node.edge_size(); j++) {
//       const Hypergraph_Edge& edge = node.edge(j);
//       str_vector * features;
//       if (edge.HasExtension(edge_fv)) { 
//         const string & feat_str = edge.GetExtension(edge_fv);  
//         features = svector_from_str<int, double>(feat_str);
//         //cout << feat_str << endl;
//         //cout << edge.label() << endl;
//         //cout << svector_str(*features) << endl;
//         //cout << feat_str << endl;
//       } else {
//         features = new svector<int, double>();
//       }

//       vector <Scarab::HG::Hypernode *> tail_nodes;
//       for (int k =0; k < edge.tail_node_ids_size(); k++ ){
//         int id = edge.tail_node_ids(k);
        
//         tail_nodes.push_back( _nodes[id]);
        
//       } 

//       Scarab::HG::Hyperedge * forest_edge = new Scarab::HG::HyperedgeImpl(edge.label(), features, edge_id, tail_nodes, _nodes[node.id()]);
      
//       for (int k =0; k < edge.tail_node_ids_size(); k++ ){
//         int id = edge.tail_node_ids(k);
//         ((ForestNode*)_nodes[id])->add_in_edge(forest_edge);
//       }

//       edge_id++;
//       ((ForestNode*)_nodes[node.id()])->add_edge(forest_edge);
//       //int for_edge_id = forest_edge->id();
//       _edges.push_back(forest_edge);//[for_edge_id] = forest_edge;
//     }
//     //cout << node.id() << " "<<  _nodes[node.id()]->num_edges() << " " << node.edge_size() << " " << _nodes[node.id()]->is_word() << endl;
//     assert (_nodes[node.id()]->num_edges() == (uint)node.edge_size() );
//   }
//   assert (_nodes.size() == (uint)hgraph.node_size());

  
//   _root = ((ForestNode*)_nodes[hgraph.root()]);//_nodes[_nodes.size()-1];
// }


void Forest::print() const {
  
}

Forest Forest::from_file(const char * file) {
  //return Forest(file);
  Forest f;
  f.build_from_file(file);
  return f;
  /*Hypergraph hgraph;
    
  {
    stringstream fname;
    fname <<argv[1] << i;
    //cout << fname.str() << endl;
    fstream input(fname.str().c_str(), ios::in | ios::binary);
    if (!hgraph.ParseFromIstream(&input)) {
      assert (false);
    } 
  }
  return Forest(hgraph);*/
}


// const vector <Scarab::HG::Hypernode*> & Forest::nodes() const {
//   return _nodes;
// }

// const vector <Scarab::HG::Hyperedge*> & Forest::edges() const {
//   return _edges;
// } 

//  const vector <Scarab::HG::Hypernode*> & ForestEdge::tail_nodes() const {
//   return _tail_nodes;
// }

//const vector <Scarab::HG::Hyperedge*> & ForestNode::edges() const {
//return _edges;
//}

//const vector <Scarab::HG::Hyperedge*> & ForestNode::in_edges() const {
//return _in_edges;
//}

