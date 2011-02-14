#ifndef MRFHYPERGRAPH_H
#define MRFHYPERGRAPH_H

#include "MRF.h"
#include <Hypergraph.h>
#include <HypergraphImpl.h>
#include <EdgeCache.h>
using namespace Scarab::HG;
class MRFHypergraph : public HypergraphImpl {
 public:
  static MRFHypergraph from_mrf(const MRF & mrf);

  NodeAssignment assignment_from_node(Hypernode* node) {
    return _canonical_assignment->get(*node);
  }

  void print() const {}  

  HNode assignment_from_node(NodeAssignment & node_assign) {
    return _canonical_hnode->get(node_assign);
  }


 private:
  Cache <NodeAssignment, Hypernode * > * _canonical_hnode;
  Cache <Hypernode, NodeAssignment > * _canonical_assignment;
};


#endif
