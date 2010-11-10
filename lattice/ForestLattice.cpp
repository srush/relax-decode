#include "ForestLattice.h"
#include <iostream>
#include <iomanip>

using namespace std;


ForestLattice::ForestLattice(const Lattice & lat) {
  _words.resize(NUMSTATES);
  original_nodes.resize(NUMSTATES);
  num_nodes = lat.node_size();

  for (int i = 0; i < lat.node_size(); i++) {
    const Lattice_Node & node =  lat.node(i);

    //cout << node.id()<<endl;
    assert (_nodes.size() == node.id());
    _nodes.push_back(new LatNode(node.id()));
    
    node_edges[node.id()] = node.edge_size();
    for (int j =0; j < node.edge_size(); j++) {
      const Lattice_Edge & edge = node.edge(j);
      graph[node.id()][j] = edge.to_id();
    }


    if (node.GetExtension(is_word)) {
      word_node[node.id()] = 1; //node.getExtension(word);
      edge_node[node.id()] = -1;
      //cout << node.GetExtension(word) << endl;
      //cout <<node.id() << endl;
      _words[node.id()]= node.GetExtension(word);
    } else {
      word_node[node.id()] = -1;
      edge_node[node.id()] = 1;
    }
    int orig_node =node.GetExtension(original_node);
    bool ignore = node.GetExtension(ignore_node);
    if (ignore) {
      //cout << "IGNORING " << node.id() <<endl;
      assert (orig_node == -1);
    }
    if (orig_node != -1) {
      assert (orig_node >= 0 && orig_node < NUMSTATES);
      original_nodes[orig_node].push_back(node.id());
    }
    
    ignore_nodes[node.id()] = node.GetExtension(ignore_node);    

  }

  start = lat.start();
  for (int i=0; i < lat.final_size(); i++) {
    final[lat.final(i)] = 1;
  }  
} 


