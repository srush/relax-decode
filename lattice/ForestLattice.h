#ifndef FORESTLATTICE_H_
#define FORESTLATTICE_H_



#include "lattice.pb.h"

#include <string>
#include <vector>
using namespace std;
using namespace lattice;

class LatNode {
 public:
 LatNode(int id):_id(id){}
  
  int id() const  {return _id;}
 private:
  int _id;
};

class ForestLattice {
 public:
  bool is_node_word(int node_num) const {
    return word_node[node_num] != -1;
  }
  string get_word(int node_num) const {
    assert (is_node_word(node_num));
    return _words[node_num];
  }

  const LatNode & node(int i) const {
    assert (i < num_nodes);
    return *_nodes[i];
  };

  // Graph interface
  int num_nodes;

  
  vector<int> word_node;
  vector<int> edge_node;
  vector<int> ignore_nodes;
  
  vector<int> final;
  int start;
  ForestLattice(const Lattice & lattice);

  vector <vector <int> > original_nodes;

  int get_edge(int n1, int edge_num) const {
    assert (edge_num < node_edges[n1]);
    assert (n1 < num_nodes);
    return graph[n1][edge_num];
  }

  int num_edges(int n) const {
    assert (n < num_nodes);
    return node_edges[n];
  }

 private:
  vector<int> node_edges;
  vector<string> _words;  
  vector <LatNode *> _nodes; 
  vector<vector<int> > graph;
};


struct Bigram{ 
  int w1;
  int w2;
  Bigram(int word1,int word2): w1(word1), w2(word2) {}
  Bigram(){}
};


#endif
