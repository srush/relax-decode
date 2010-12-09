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

struct LatticeVars;
class LPBuilder {

 public:
  
  LPBuilder(const Forest & forest, const ForestLattice & lat ): _forest(forest), _lattice(lat) {
    
  }

  void solve_hypergraph(const Cache<ForestEdge, double> & );

  void solve_full(const Cache<ForestEdge, double> & _weights,Ngram &lm, const Cache <LatNode, int> & word_cache);

 private:
  const Forest & _forest;
  void build_hypergraph_lp(vector <GRBVar> & node_vars, vector <GRBVar> & edge_vars, const Cache<ForestEdge, double> & _weights);
  //void build_all_pairs_lp(const ForestLattice & _lattice, GRBModel & model, Ngram &lm, const Cache <LatNode, int> & word_cache);
  void build_all_pairs_lp(         Ngram &lm, 
                                   const Cache <LatNode, int> & word_cache,
                                   
                                   vector < GRBVar > & word_used_vars,
                                   vector < vector < vector < GRBVar > > > & word_tri_vars,
                                   LatticeVars & lv,
                                   const GraphDecompose & gd);

    const ForestLattice & _lattice;
    GRBModel * model;
    void initialize_word_pairs(Ngram &lm, 
                                      const Cache <LatNode, int> & word_cache,
                                      const GraphDecompose & gd, 
                                      vector < GRBVar > & word_used_vars,
                                      vector < vector < GRBVar >  > & word_tri_vars,
                               vector < vector < vector < GRBVar > > > & word_tri_vars);
    void build_all_tri_pairs_lp(Ngram &lm, 
                                const Cache <LatNode, int> & word_cache,
                                vector < GRBVar > & word_used_vars,
                                vector < vector < vector < GRBVar > > > & word_tri_vars,
                                LatticeVars & lv,
                                LatticeVars & lv2,
                                const GraphDecompose & gd);

};
#endif
