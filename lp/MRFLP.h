#ifndef MRFLP_H
#define MRFLP_H

#include "gurobi_c++.h"
#include "MRF.h"
#include "Graph.h"

using namespace std;

struct MRFLP {
MRFLP(const MRF &p) : mrf(p), node_vars(p.graph().num_nodes()), edge_vars(p.graph().num_edges()), 
    from_state_blank_vars(p.graph().num_edges()),
    to_state_blank_vars(p.graph().num_edges()) {}
  Cache<Graphnode, Cache<State, GRBVar> * >  node_vars;
  Cache<Graphedge, Cache<State, Cache <State , GRBVar > * > *> edge_vars;
  Cache<Graphedge, Cache<State, GRBVar> * > from_state_blank_vars;
  Cache<Graphedge, Cache<State, GRBVar> * > to_state_blank_vars;
  const MRF & mrf; 
};


class MRFBuilderLP {
 public:
  static MRFLP * add_mrf(const MRF & mrf, string prefix, GRBModel & model, int var_type);
 private:
  const MRF & _mrf; 
};

#endif
