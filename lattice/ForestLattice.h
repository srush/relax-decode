#ifndef FORESTLATTICE_H_
#define FORESTLATTICE_H_


#include <string>
#include "lattice.pb.h"

#include <string>
#include <fstream>
#include <vector>
#include "Graph.h"

#include "EdgeCache.h"

using namespace std;
using namespace lattice;
using namespace Scarab::Graph;
struct Word{
public:
  const int id() const {
    return _id;
  }
Word(int id) :_id(id){}
private:
  int _id;
};


struct Bigram{ 
  int w1;
  int w2;
  Bigram(int word1, int word2): w1(word1), w2(word2) {}
  Bigram(){}
};


struct WordBigram{ 
  Word w1;
  Word w2;
  WordBigram(Word word1, Word word2): w1(word1), w2(word2) {}
  //WordBigram(){}
};

ostream& operator<<(ostream& os, const Word& w); 
ostream& operator<<(ostream& os, const WordBigram& w); 



class ForestLattice {
 public:
  string get_word(int word_num) const {
    assert(is_word(word_num));
    return _words.store[word_num];
  }

  string get_word(const Word & word) const {
    return _words.get(word);
  }

  const Graph & get_graph() const {
    return *_proper_graph;
  }

  const Graphnode &node(int i) const {
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

  const Nodes &phrase_nodes() const {
    return _phrase_nodes;
  }

  int num_word_nodes;

  bool is_word(int w) const {
    assert(w >= 0 && w < num_word_nodes);
    return _is_word[w];
  }

  static ForestLattice from_file(const string &  file) {
    Lattice lat;  
    {
      fstream input(file.c_str(), ios::in | ios::binary);
      if (!lat.ParseFromIstream(&input)) {
        assert (false);
      }
    }
    return ForestLattice(lat);
  }
  
  vector<int> final;
  int start;
  ForestLattice(const Lattice & lattice);

  vector<vector <int> > original_edges;
  //vector<vector <int> > original_edges_position;
  vector<vector <int> > edges_original;

  inline Node lookup_word(int w) const {
    return _words_lookup.store[w];
  }

  inline Node lookup_word(const Word &w) const {
    return _words_lookup.get(w);
  }

  
  inline int get_edge_label(int n1, int n2) const {
    return _edge_by_nodes[n1][n2];
  }

  inline int get_edge_label(const Graphnode &n1, 
                            const Graphnode &n2) const {
    return _edge_by_nodes[n1.id()][n2.id()];
  }

  inline Bigram get_nodes_by_labels(int orig_id) const {    
    return _original_id_to_edge[orig_id];
  }


  inline int num_first_words(int n) const {
    assert (is_phrase_node(n));
    return _first_words.store[n].size();
  }

  inline int num_last_words(int n) const {
    assert (is_phrase_node(n));
    return _last_words.store[n].size();
  }

  inline int num_last_bigrams(int n) const {
    assert (is_phrase_node(n));
    return _last_bigrams[n].size();
  }


  inline int first_words(int n, int i) const {
    assert (is_phrase_node(n));
    return _first_words.store[n][i].id();
  }

  inline const vector <Word> & first_words(const Graphnode & n) const {
    assert (is_phrase_node(n));
    return _first_words.get_default(n, empty_);
  }

  inline const vector <Word> & last_words(const Graphnode & n) const {
    assert (is_phrase_node(n));
    return _last_words.get_default(n, empty_);
  }

  // Deprecated
  inline int last_words(int n, int i) const {
    assert (is_phrase_node(n));
    return _last_words.store[n][i].id();
  }

  inline Bigram last_bigrams(int n, int i) const {
    assert (is_phrase_node(n));
    return _last_bigrams[n][i];
  }


  inline int get_same(int w) const {
    return _last_same[w];
  }

  // to remove
  inline int get_hypergraph_node_from_word(int w) const {
    assert(is_word(w));
    return _lat_word_to_hyp_node.store[w];
  }

  inline int get_hypergraph_node_from_word(const Word & w) const {
    return _lat_word_to_hyp_node.get(w);
  }

  inline int get_word_from_hypergraph_node(int n) const { 
    return _hyp_node_to_lat_word[n];
  }


  void make_proper_graph(const Lattice & lat);
  
  const vector <WordBigram> & 
    get_bigrams_at_node(const Graphnode & node) const {
    return bigrams_at_node.get_default(node, vector<WordBigram>());
  }

  vector <string>  _edge_label_by_nodes; 
 private:
  Cache<Graphnode, vector<WordBigram> > bigrams_at_node;
  Graph * _proper_graph;

  vector<Word> empty_;
  
  vector<int> word_node;
  vector<int> node_edges;

  Cache <Word, string> _words;  
  Cache<Word, Node> _words_lookup;  

  vector<int> _is_word;  

  //vector <LatNode *> _nodes; 
  //vector<vector<int> > graph;
  vector<vector<int> > _edge_by_nodes;

  Cache <Graphnode, vector<Word> > _first_words;
  Cache <Graphnode, vector<Word> > _last_words;


  vector<vector<Bigram> > _last_bigrams;

  vector <const Graphnode * > _phrase_nodes;

  vector<int> _last_same;

  vector<int> edge_node;
  vector<int> ignore_nodes;

  vector <Bigram>  _original_id_to_edge;

  Cache <Word, int>  _lat_word_to_hyp_node;
  vector <int>  _hyp_node_to_lat_word;


};




#endif
