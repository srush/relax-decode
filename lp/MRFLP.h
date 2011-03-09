#ifndef MRFLP_H
#define MRFLP_H

#include "gurobi_c++.h"
#include "MRF.h"
#include "Graph.h"
#include "../common.h"
#include <sstream>
#include "LPCommon.h"
using namespace std;


struct MRFLP : public LPBuilder {
MRFLP(const MRF &p) : mrf(p), node_vars(p.graph().num_nodes()), edge_vars(p.graph().num_edges()), 
    from_state_blank_vars(p.graph().num_edges()),
    to_state_blank_vars(p.graph().num_edges()) {}
  Cache<Graphnode, Cache<State, GRBVar> * >  node_vars;
  Cache<Graphedge, Cache<State, Cache <State , GRBVar > * > *> edge_vars;
  Cache<Graphedge, Cache<State, GRBVar> * > from_state_blank_vars;
  Cache<Graphedge, Cache<State, GRBVar> * > to_state_blank_vars;
  const MRF & mrf; 

private:
  void add_node_vars();
  void add_edge_vars();
  void add_node_constraints();
  void add_edge_constraints();

public :
  void show() const;
  void add_vars() {
    add_node_vars();
    add_edge_vars();
  }
  void add_constraints() {
    add_node_constraints();
    add_edge_constraints();
  }
};


/* class MRFBuilderLP { */
/*  public: */
/*   static MRFLP *  */
/*  private: */
/*   const MRF & _mrf;  */
/* }; */

#endif
