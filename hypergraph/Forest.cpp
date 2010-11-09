#include "Forest.h"

#include <iomanip>
#include <vector>
#include "cy_svector.hpp"

using namespace std;



Forest::Forest(const Hypergraph& hgraph) {
  _nodes.resize(10000);
  for (int i = 0; i < hgraph.node_size(); i++) {
    const Hypergraph_Node & node = hgraph.node(i);
    
    
    string feat_str = node.GetExtension(node_fv);
    str_vector * features = svector_from_str<int, double>(feat_str);

    ForestNode * forest_node = new ForestNode(node.label(), node.id(), features, node.GetExtension(word), node.GetExtension(is_word)); 
    

    _nodes[forest_node->id()] = forest_node;
    cout << _nodes.size() << " " << forest_node->id() << endl;
    
  
  }

  int edge_id = 0;
  for (int i = 0; i < hgraph.node_size(); i++) {
    const Hypergraph_Node& node = hgraph.node(i);
    
    for (int j=0; j < node.edge_size(); j++) {
      const Hypergraph_Edge& edge = node.edge(j);
      string feat_str = node.GetExtension(node_fv);  
      str_vector * features = svector_from_str<int, double>(feat_str);

      vector <ForestNode* > tail_nodes;
      for (int k =0; k < edge.tail_node_ids_size(); k++ ){
        int id = edge.tail_node_ids(k);
        
        tail_nodes.push_back(_nodes[id]);
      } 

      ForestEdge * forest_edge = new ForestEdge(edge.label(), features, edge_id, tail_nodes);
      edge_id++;
      _nodes[node.id()]->add_edge(forest_edge);
      _edges[forest_edge->id()] = forest_edge;
    }
  }
}


void Forest::print() const {
  
}


