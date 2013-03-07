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

  Scarab::HG::HyperedgeImpl * edge =  
    (Scarab::HG::HyperedgeImpl * ) node->_edges[0];
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





void Forest::print() const {
  
}

Forest Forest::from_file(const char * file) {
  //return Forest(file);
  Forest f;
  f.build_from_file(file);
  return f;
}


