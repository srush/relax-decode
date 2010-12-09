#ifndef FOREST_H_
#define FOREST_H_

#include "features.pb.h"
#include "translation.pb.h"
#include "hypergraph.pb.h"
#include "lexical.pb.h"
#include <vector>
#include "svector.hpp"
#include <string>
#include "assert.h"
using namespace std;


class ForestNode;

typedef  svector<int, double> str_vector;

class ForestEdge {
public:
 ForestEdge(const string & label, str_vector * features, int id, vector <ForestNode *> tail_nodes):
  _label(label), _features(features), _id(id), _tail_nodes(tail_nodes) {}
  
  int id() const {return _id;} 

  const ForestNode & tail_node(int i) const {
    return *_tail_nodes[i];
  }

  int num_nodes() const{
    return _tail_nodes.size();
  }  


  const str_vector & fvector() const {
    return *_features;
  }
  vector <ForestNode *> _tail_nodes;
private:
  const string & _label; 
  str_vector * _features;
  const int _id;

  
};

class ForestNode {
public:
  ForestNode(const string & label, int id, str_vector * features, string word, bool is_word) :
  _label(label), _id(id), _features(features), _word(word), _is_word(is_word) {}
  void add_edge(ForestEdge * edge) {
    _edges.push_back(edge);
  }

  void add_in_edge(ForestEdge * edge) {
    _in_edges.push_back(edge);
  }

  int id() const {return _id;}

  const ForestEdge & edge(int i ) const {
    return *_edges[i];
  }

  int num_edges() const{
    return _edges.size();
  }

  int num_in_edges() const{
    return _in_edges.size();
  }

  const ForestEdge & in_edge(int i) const {
    return *_in_edges[i];
  }


  bool is_word() const {
    return _is_word;
  }

  string word() const {
    assert(_is_word);
    return _word;
  }

  

  //void print();
  vector <ForestEdge *> _edges; 
private:
  const string & _label;
  const int  _id;
  str_vector * _features;
  
  vector <ForestEdge *> _in_edges; 
  const string _word;
  const bool _is_word;

};


class Forest {
 public:
  Forest(const Hypergraph& pb);
  void print() const;
  
  void append_end_nodes();
  const ForestNode & root() const {
    return *_root;
  }

  int num_edges() const{
    return _edges.size();
  }

  int num_nodes() const{
    return _nodes.size();
  }

  const ForestNode & get_node(int i) const {
    const ForestNode & node =*_nodes[i];
    assert (node.id() == i);
    return node;
  }


  const ForestEdge & get_edge(int i) const {
    const ForestEdge & edge =*_edges[i];
    assert (edge.id() == i);
    return edge;
  }
 

 private:
  ForestNode * _root;
  vector <ForestNode *> _nodes;
  vector <ForestEdge *> _edges; 
};


#endif
