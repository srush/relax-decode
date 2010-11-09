#include "ForestLattice.h"

ForestLattice::ForestLattice(const Lattice & lat) {
  _words.resize(NUMSTATES);
  original_nodes.resize(NUMSTATES);
  for (int i = 0; i < lat.node_size(); i++) {
    const Lattice_Node & node =  lat.node(i);
    node_edges[node.id()] = node.edge_size();
    for (int j =0; j < node.edge_size(); j++) {
      const Lattice_Edge & edge = node.edge(j);
      graph[node.id()][j] = edge.id();
    }

    _nodes.push_back(new LatNode(node.id()));

    if (node.GetExtension(is_word)) {
      word_node[node.id()] = 1; //node.getExtension(word);
      edge_node[node.id()] = -1;
      _words[node.id()]= node.GetExtension(word);
    } else {
      word_node[node.id()] = -1;
      edge_node[node.id()] = 1;
    }

    original_nodes[node.GetExtension(original_node)].push_back(node.id());
  }

  start = lat.start();
  for (int i=0; i < lat.final_size(); i++) {
    final[lat.final(i)] = 1;
  }

  
} 
