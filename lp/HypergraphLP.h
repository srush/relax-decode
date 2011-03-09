#ifndef HYPERGRAPHLP_H_
#define HYPERGRAPHLP_H_
#include <Hypergraph.h>
#include <gurobi_c++.h>
#include "EdgeCache.h"
#include "LPCommon.h"
#include "../common.h"

namespace Scarab {
  namespace HG {

    class HypergraphLP : public LPBuilder{
    public:
   HypergraphLP(const HGraph & h, const Cache<Hyperedge, double > & weights ):  
    _h(h), node_vars(h.num_nodes()), 
      edge_vars(h.num_edges()), _weights(weights) {}

  Cache<Hypernode, GRBVar>  node_vars;
  Cache<Hyperedge, GRBVar>  edge_vars;
  const HGraph & _h; 
  const Cache<Hyperedge, double> & _weights;

  void add_vars();
  void add_constraints();
  void show() const; 
  
  
};


  } } 

/* class HypergraphLPBuilder {   */
/*  public: */
/* }; */
/*   } */
/* } */

#endif
