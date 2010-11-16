#ifndef LPBUILDER_H_
#define LPBUILDER_H_

#include "Forest.h"
#include "ForestLattice.h"
#include "GraphDecompose.h"
#include <EdgeCache.h>
#include <string>
#include "gurobi_c++.h"
#include <Ngram.h>

using namespace std;
class LPBuilder {


 public:
  
  LPBuilder(const Forest & forest): _forest(forest) {
    
  }

  void solve_hypergraph(const Cache<ForestEdge, double> & );

  void solve_full(const Cache<ForestEdge, double> & _weights, const ForestLattice & _lattice,  Ngram &lm, const Cache <LatNode, int> & word_cache);

 private:
  const Forest & _forest;
  void build_hypergraph_lp(GRBModel & model, vector <GRBVar> & node_vars, vector <GRBVar> & edge_vars, const Cache<ForestEdge, double> & _weights);
  //void build_all_pairs_lp(const ForestLattice & _lattice, GRBModel & model, Ngram &lm, const Cache <LatNode, int> & word_cache);
  void build_all_pairs_lp(const ForestLattice & _lattice, 
                                   GRBModel & model, 
                                   Ngram &lm, 
                                   const Cache <LatNode, int> & word_cache,
                                   vector < vector < GRBVar > > & all_pairs_exist_vars,
                                   vector < GRBVar > & word_used_vars,
                                   vector < vector < vector < GRBVar > > > & word_tri_vars,
                                   const GraphDecompose & gd);

};
#endif
