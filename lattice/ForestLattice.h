#ifndef FORESTLATTICE_H_
#define FORESTLATTICE_H_

#define NUMSTATES 1500

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
  int graph[NUMSTATES][NUMSTATES];
  int node_edges[NUMSTATES];
  
  int word_node[NUMSTATES];
  int edge_node[NUMSTATES];
  int ignore_nodes[NUMSTATES];
  
  int final[NUMSTATES];
  int start;
  ForestLattice(const Lattice & lattice);

  vector <vector <int> > original_nodes;

 private:
  vector<string> _words;  
  vector <LatNode *> _nodes; 


};


struct Bigram{ 
  int w1;
  int w2;
  Bigram(int word1,int word2): w1(word1), w2(word2) {}
  Bigram(){}
};


#endif
