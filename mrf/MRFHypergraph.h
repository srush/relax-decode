#ifndef MRFHYPERGRAPH_H
#define MRFHYPERGRAPH_H

#include "MRF.h"
#include <algorithm>
#include <Hypergraph.h>
#include <HypergraphImpl.h>
#include <EdgeCache.h>
using namespace Scarab::HG;
class MRFHypergraph : public HypergraphImpl {
 public:
  static MRFHypergraph * from_mrf(const MRF & mrf);

 MRFHypergraph(const MRF & mrf) : _mrf(mrf) {}

  const NodeAssignment & assignment_from_node(HNode node) const {
    return (*_canonical_assignment)[node->id()];
  }

  bool node_has_assignment(HNode node) const {
    return _canonical_assignment->find(node->id()) != _canonical_assignment->end();
  }


  void print() const {}  

  HNode node_from_assignment(const NodeAssignment & node_assign) const {
    return _canonical_hnode->get(node_assign);
  }

  const MRF &  mrf() const  {
    return _mrf;
  }

  vector <NodeAssignment> derivation_to_assignments(HNodes best_nodes) const {
    vector <NodeAssignment> res;

    foreach (HNode node, best_nodes) {
      if (node_has_assignment(node)) {
        NodeAssignment d = assignment_from_node(node);
        res.push_back(d);
      }
    }
    sort(res.begin(), res.end());
    return res;
  }

 private:
  const MRF & _mrf;
  Cache <NodeAssignment, Hypernode * > * _canonical_hnode;
/*   Cache <Hypernode, NodeAssignment > * _canonical_assignment; */
  map <int, NodeAssignment > *_canonical_assignment;
};


#endif
