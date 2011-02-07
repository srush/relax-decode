#ifndef PHRASEBASED_H_
#define PHRASEBASED_H_

#include "Hypergraph.h"
#include "HypergraphImpl.h"
#include "EdgeCache.h"
using namespace Scarab::HG;


class PhraseBased : public Scarab::HG::HypergraphImpl {
 public:
  void print() const {}
  
  void set_up(const Hypergraph & hgraph){}
  
 protected:

};


#endif
