#ifndef TAGGER_H_
#define TAGGER_H_

#include "tag.pb.h"
#include "dep.pb.h"
#include "Hypergraph.h"
#include "HypergraphImpl.h"
#include "EdgeCache.h"
using namespace Scarab::HG;

typedef uint POS;

struct Tag {
public:
  int ind;
  int length;
  POS tag;

  const static int MAX_TAG = 45;
  
  Tag(){}
  
  Tag(uint ind_, POS tag_ , int len_): 
    ind(ind_), 
    tag(tag_), 
    length(len_) {
      assert(ind_ < len_);
      //assert(tag_ < MAX_TAG); 
      _id= tag * len_ + ind;
  }

  const bool operator<(const Tag & other) const {
    if (ind != other.ind) 
      return ind < other.ind;
    else 
      return tag < other.tag;
  }

  int id() const {return _id;}

private:
  int _id;
};

ostream& operator<<(ostream& output, const Tag& h);

class Tagger : public Scarab::HG::HypergraphImpl {
 public:
  int num_tag;
  
  Tagger(int num_tags_): 
    num_tag(num_tags_) {
  }

  ~Tagger() {
    //delete _edge_map;
  }

  void print() const {}
  
  void set_up(const Hypergraph & hgraph);

  const Hypergraph & hypergraph() const {
    return *_h;
  }
  
  //const Hypergraph & weights() const {
  //return *_weights;
  //}

  
  /** 
   * Enumerate all the possible tags for this sentence
   * 
   * @return An enumerator to all the tags in the sentence
   */
  vector <Tag > tags() const {
    return _tags;
  }

  uint num_tags() const {
    return _tag_length;
  }

  uint sent_length() const {
    return _sent_length;
  } 

  Tag make_tag(int ind, int tag) const {
    return Tag(ind, tag, _sent_length);
  }

  const vector<const Hyperedge*> & tag_to_edge(const Tag & tag) const {
    return _edge_map->get(tag);
  }

  bool tag_has_edge(const Tag & tag) const {
    return _edge_map->has_key(tag);
  }

  const Tag & edge_to_tag(const Hyperedge & edge) const {
    return _tag_map->get(edge);
  }

  bool edge_has_tag(const Hyperedge & edge) const {
    return _tag_map->has_key(edge);
  }
  
 protected:
  void make_edge(const Hypergraph_Edge & edge, 
                         const Scarab::HG::Hyperedge * our_edge);

 private:

  // The parse forest in hypergraph form
  int _sent_length;
  int _tag_length;
  Hypergraph * _h;
  Cache <Tag, vector <const Hyperedge *> > *  _edge_map;
  Cache <Hyperedge, Tag> * _tag_map;
  //Cache <Hyperedge, double> _weights;
  vector <Tag> _tags; 
};


#endif
