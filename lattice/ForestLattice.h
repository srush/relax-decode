#ifndef FORESTLATTICE_H_
#define FORESTLATTICE_H_



#include "lattice.pb.h"

#include <string>
#include <vector>
using namespace std;
using namespace lattice;

struct Bigram{ 
  int w1;
  int w2;
  Bigram(int word1,int word2): w1(word1), w2(word2) {}
  Bigram(){}
};

class LatNode {
 public:
 LatNode(int id):_id(id){}
  
  int id() const  {return _id;}
 private:
  int _id;
};

class ForestLattice {
 public:
  //bool is_node_word(int node_num) const {
  //return word_node[node_num] != -1;
  //}
  string get_word(int word_num) const {
    assert(is_word(word_num));
    return _words[word_num];
  }

  const LatNode & node(int i) const {
    assert (i < num_nodes);
    return *_nodes[i];
  };

  bool is_phrase_node(int n) const {
    return word_node[n] != -1;
  }

  bool is_word(int w) const {
    return _is_word[w];
  }

  // Graph interface
  int num_nodes;
  int num_word_nodes;

  
  
  vector<int> final;
  int start;
  ForestLattice(const Lattice & lattice);

  vector <vector <int> > original_edges;
  vector <vector <int> > edges_original;


  inline int get_edge(int n1, int edge_num) const {
    assert (edge_num < node_edges[n1]);
    assert (n1 < num_nodes);
    return graph[n1][edge_num];
  }

  inline int num_edges(int n) const {
    assert (n < num_nodes);
    return node_edges[n];
  }

  inline int lookup_word(int w) const {
    //assert (n < num_nodes);
    return _words_lookup[w];
  }

  inline int lookup_word_by_hypergraph_node(int hnode) const {
    //assert (n < num_nodes);
    int w =  _hnode_from_word_lookup[hnode];
    assert(is_word(w));
    return w;
  }
  
  inline  int get_edge_label(int n1, int n2) const {
    return _edge_by_nodes[n1][n2];
  }

  inline  Bigram get_nodes_by_labels(int orig_id) const {
    
    return _original_id_to_edge[orig_id];
  }


  inline int num_first_words(int n) const {
    assert (is_phrase_node(n));
    return _first_words[n].size();
  }

  inline int num_last_words(int n) const {
    assert (is_phrase_node(n));
    return _last_words[n].size();
  }

  inline int first_words(int n, int i) const {
    assert (is_phrase_node(n));
    return _first_words[n][i];
  }

  inline int last_words(int n, int i) const {
    assert (is_phrase_node(n));
    return _last_words[n][i];
  }

  inline int get_same(int w) const {
    return _last_same[w];
  }
  
  vector<vector<Bigram> > bigrams_at_node;
 private:
  vector<int> word_node;

  vector<int> node_edges;
  vector<string> _words;  
  vector<int> _is_word;  
  vector<int> _words_lookup;  
  vector <LatNode *> _nodes; 
  vector<vector<int> > graph;
  vector<vector<int> > _edge_by_nodes;

  vector<vector<int> > _first_words;
  vector<vector<int> > _last_words;

  vector<int> _last_same;

  vector<int> edge_node;
  vector<int> ignore_nodes;
  vector<int> _hnode_from_word_lookup;
  vector <Bigram>  _original_id_to_edge;
};




#endif
