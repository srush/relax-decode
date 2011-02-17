#ifndef FOREST_H_
#define FOREST_H_

#include <vector>
#include "svector.hpp"
#include <string>
#include "assert.h"
#include "Hypergraph.h"
#include "HypergraphImpl.h"
#include "hypergraph.pb.h"
using namespace std;
//using namespace Hypergraph;


typedef svector<int, double> str_vector;



class ForestNode: public Scarab::HG::HypernodeImpl {
 public:
 ForestNode(const string & label, int id, str_vector * features, string word, bool is_word) : 
  HypernodeImpl(label, id, features),  _word(word), _is_word(is_word) {}

  bool is_word() const {
    return _is_word;
  }

  string word() const {
    assert(_is_word);
    return _word;
  }
  
 private:
  const string _word;
  const bool _is_word;

};



class Forest : public Scarab::HG::HypergraphImpl{
 public:
  ~Forest(){}

  void print() const;
  
  void append_end_nodes();
   
  static Forest from_file(const char * file_name);

 protected:
  Scarab::HG::Hypernode* make_node(const Hypergraph_Node & node, wvector * features);
};


#endif
