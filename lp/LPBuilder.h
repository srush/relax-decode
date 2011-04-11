#ifndef LPBUILDER_H_
#define LPBUILDER_H_

#include "Hypergraph.h"
#include "ForestLattice.h"
#include "GraphDecompose.h"
#include <EdgeCache.h>
#include <string>
#include "gurobi_c++.h"
#include <Ngram.h>

using namespace std;

namespace Scarab { 
  namespace HG { 

struct LatticeVars;
class LPBuilder {

 public:
  
  LPBuilder(const HGraph & forest, const ForestLattice & lat ): _forest(forest), _lattice(lat) {
    
  }

  void solve_hypergraph(const Cache<Hyperedge, double> & );

  void solve_full(int run_num, const Cache<Hyperedge, double> & _weights,Ngram &lm, double lm_weight, const Cache <Graphnode, int> & word_cache);

 private:
  const HGraph & _forest;
  double _lm_weight;
  void build_hypergraph_lp(vector <GRBVar> & node_vars, vector <GRBVar> & edge_vars, const Cache<Hyperedge, double> & _weights);
  //void build_all_pairs_lp(const ForestLattice & _lattice, GRBModel & model, Ngram &lm, const Cache <Graphnode, int> & word_cache);
  void build_all_pairs_lp(         Ngram &lm, 
                                   const Cache <Graphnode, int> & word_cache,
                                   
                                   vector < GRBVar > & word_used_vars,
                                   vector < vector < vector < GRBVar > > > & word_tri_vars,
                                   LatticeVars & lv,
                                   const GraphDecompose & gd);

    const ForestLattice & _lattice;
    GRBModel * model;
    void initialize_word_pairs(Ngram &lm, 
                                      const Cache <Graphnode, int> & word_cache,
                                      const GraphDecompose & gd, 
                                      vector < GRBVar > & word_used_vars,
                                      vector < vector < GRBVar >  > & word_tri_vars,
                               vector < vector < vector < GRBVar > > > & );
    void build_all_tri_pairs_lp(Ngram &lm, 
                                const Cache <Graphnode, int> & word_cache,
                                vector < GRBVar > & word_used_vars,
                                vector < vector < vector < GRBVar > > > & word_tri_vars,
                                LatticeVars & lv,
                                LatticeVars & lv2,
                                const GraphDecompose & gd);

};
  }}
#endif
  
