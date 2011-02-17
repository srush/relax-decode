#ifndef FORESTLATTICE_H_
#define FORESTLATTICE_H_


#include <string>
#include "lattice.pb.h"

#include <string>
#include <vector>
#include "Graph.h"
using namespace std;
using namespace lattice;
using namespace Scarab::Graph;
struct Word{
  const int id() const {
    return _id;
  }
Word(int id) :_id(id){
    
  }
private:
  int _id;
};


struct Bigram{ 
  int w1;
  int w2;
  Bigram(int word1,int word2): w1(word1), w2(word2) {}
  Bigram(){}
};



/*class LatNode : public Graphnode {
 public:
 LatNode(int id, vector <const Graphedge *> edges):_id(id), _edges(edges){}
    
  uint id() const  {return _id;}

  unsigned int num_edges() const {return _edges.size();}
  const Edges & edges() const  {return _edges;}
 private:
  int _id;
  const Edges & _edges;
  };*/

class ForestLattice {
 public:
  //bool is_node_word(int node_num) const {
  //return word_node[node_num] != -1;
  //}
  string get_word(int word_num) const {
    assert(is_word(word_num));
    return _words[word_num];
  }

  const Graph & get_graph() const {
    return *_proper_graph;
  }

  const Graphnode & node(int i) const {
    //assert (i < num_nodes);
    return _proper_graph->node(i);//*_nodes[i];
  };

  // Deprecated
  bool is_phrase_node(int n) const {
    return word_node[n] != -1;
  }
  
  bool is_phrase_node(const Graphnode & node) const {
    return word_node[node.id()] != -1;
  }

  const Nodes & phrase_nodes() const {
    return _phrase_nodes;
  }

  bool is_word(int w) const {
    assert(w >= 0 && w < num_word_nodes);
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
    //assert (n < num_nodes);
    return node_edges[n];
  }

  inline int lookup_word(int w) const {
    //assert (n < num_nodes);
    return _words_lookup[w];
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

  inline int num_last_bigrams(int n) const {
    assert (is_phrase_node(n));
    return _last_bigrams[n].size();
  }


  inline int first_words(int n, int i) const {
    assert (is_phrase_node(n));
    return _first_words[n][i].id();
  }

  inline const vector <Word> & first_words(const Graphnode & n) const {
    assert (is_phrase_node(n));
    return _first_words[n.id()];
  }

  inline const vector <Word> & last_words(const Graphnode & n) const {
    assert (is_phrase_node(n));
    return _last_words[n.id()];
  }

  // Deprecated
  inline int last_words(int n, int i) const {
    assert (is_phrase_node(n));
    return _last_words[n][i].id();
  }

  inline Bigram last_bigrams(int n, int i) const {
    assert (is_phrase_node(n));
    return _last_bigrams[n][i];
  }


  inline int get_same(int w) const {
    return _last_same[w];
  }

  inline int get_hypergraph_node_from_word(int w) const {
    assert(is_word(w));
    return _lat_word_to_hyp_node[w];
  }

  inline int get_word_from_hypergraph_node(int n) const { 
    return _hyp_node_to_lat_word[n];
  }
  void make_proper_graph(const Lattice & lat);
  
  const vector <Bigram> & get_bigrams_at_node(const Graphnode & node) const {
    return bigrams_at_node[node.id()];
  }

  vector<vector<Bigram> > bigrams_at_node;
  vector <string>  _edge_label_by_nodes; 
 private:
  Graph * _proper_graph;


  vector<int> word_node;

  vector<int> node_edges;
  vector<string> _words;  
  vector<int> _is_word;  
  vector<int> _words_lookup;  
  //vector <LatNode *> _nodes; 
  vector<vector<int> > graph;
  vector<vector<int> > _edge_by_nodes;

  vector<vector<Word> > _first_words;
  vector<vector<Word> > _last_words;


  vector<vector<Bigram> > _last_bigrams;

  vector <const Graphnode * > _phrase_nodes;

  vector<int> _last_same;

  vector<int> edge_node;
  vector<int> ignore_nodes;

  vector <Bigram>  _original_id_to_edge;

  vector <int>  _lat_word_to_hyp_node;
  vector <int>  _hyp_node_to_lat_word;


};




#endif
