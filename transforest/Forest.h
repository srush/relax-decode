#ifndef FOREST_H_
#define FOREST_H_

#include <vector>
#include "svector.hpp"
#include <string>
#include "assert.h"
#include "Hypergraph.h"

using namespace std;
//using namespace Hypergraph;


typedef svector<int, double> str_vector;


class ForestNode;


class ForestEdge: public Scarab::HG::Hyperedge{
public:
  ~ForestEdge(){}
 ForestEdge(const string & label, str_vector * features, int id, vector <Scarab::HG::Hypernode *> tail_nodes, ForestNode * head_node):
  Hyperedge(id), 
  _label(label), _tail_nodes(tail_nodes), _head_node(head_node), _features(features) {}
  
  //int id() const {return _id;} 

  const Scarab::HG::Hypernode & tail_node(unsigned int i) const {
    return * ((Scarab::HG::Hypernode*) _tail_nodes[i]);
  }

  unsigned int num_nodes() const{
    return _tail_nodes.size();
  }  
  
  

  const str_vector & fvector() const {
    return *_features;
  }

  const Scarab::HG::Hypernode & head_node() const { 
    return (*(Scarab::HG::Hypernode*)_head_node);
  }

  const string & _label;  
  vector <Scarab::HG::Hypernode *> _tail_nodes;
  ForestNode * _head_node;

  const vector <Scarab::HG::Hypernode*> & tail_nodes() const ;
private:
  str_vector * _features;
  //const int _id;
};

class ForestNode: public Scarab::HG::Hypernode {
public:
    ~ForestNode(){}
  ForestNode(const string & label, int id, str_vector * features, string word, bool is_word) :
  Scarab::HG::Hypernode(id),
  _label(label), _features(features), _word(word), _is_word(is_word) {}
  void add_edge(ForestEdge * edge) {
    _edges.push_back(edge);
  }

  void add_in_edge(ForestEdge * edge) {
    _in_edges.push_back(edge);
  }

  bool is_terminal() const {
    return is_word();
  }

  //int id() const {return _id;}

  const ForestEdge & edge(unsigned int i ) const {
    return *(ForestEdge*)_edges[i];
  }

  unsigned int num_edges() const{
    return _edges.size();
  }

  unsigned int num_in_edges() const{
    return _in_edges.size();
  }

  const ForestEdge & in_edge(uint i) const {
    return *(ForestEdge*)_in_edges[i];
  }


  bool is_word() const {
    return _is_word;
  }

  string word() const {
    assert(_is_word);
    return _word;
  }

  

  //void print();
  vector < Scarab::HG::Hyperedge *> _edges; 
  const string & _label;

  const vector <Scarab::HG::Hyperedge*> & edges() const; 
  const vector <Scarab::HG::Hyperedge*> & in_edges() const; 

private:
  
  //const int  _id;
  str_vector * _features;
  
  vector <Scarab::HG::Hyperedge *> _in_edges; 
  const string _word;
  const bool _is_word;


};


class Forest : public Scarab::HG::Hypergraph{
 public:
  ~Forest(){}
  Forest(const char* filename);//const Hypergraph & pb);
  void print() const;
  
  void append_end_nodes();
  const ForestNode & root() const {
    return *_root;
  }

  unsigned int num_edges() const{
    return _edges.size();
  }

  unsigned int num_nodes() const{
    return _nodes.size();
  }

  const ForestNode & get_node(unsigned int i) const {
    const ForestNode & node =* (ForestNode*) _nodes[i];
    assert (node.id() == i);
    return node;
  }


  const ForestEdge & get_edge(unsigned int i) const {
    const ForestEdge & edge =*(ForestEdge*)_edges[i];
    assert (edge.id() == i);
    return edge;
  }
 

  static Forest from_file(const char * file_name);
  const vector <Scarab::HG::Hypernode*> & nodes() const;
  const vector <Scarab::HG::Hyperedge*> & edges() const;

 private:
  ForestNode * _root;
  vector <Scarab::HG::Hypernode *> _nodes;
  vector <Scarab::HG::Hyperedge *> _edges; 
};


#endif
