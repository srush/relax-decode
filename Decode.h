#ifndef DECODE_H_
#define DECODE_H_


#include "Subgradient.h"
#include <Forest.h>
#include <ForestLattice.h>
#include <ForestAlgorithms.h>
#include <vector>
#include "svector.hpp"
#include <Ngram.h>
#include "dual_subproblem.h"
#include "EdgeCache.h"

using namespace std;
#define GRAMSPLIT 100000
#define GRAMSPLIT2 200000

typedef svector<int, double> wvector;

class Decode: public SubgradientProducer {
 public:
  Decode(const Forest & forest, const ForestLattice & lattice, const wvector & weight, NgramCache & lm, const SkipTrigram & skip) 
   :_forest(forest), _lattice(lattice), _weight(weight), _lm(lm)
  {
    _cached_weights = cache_edge_weights(forest, weight);
    //cout<<"decomposing" << endl;
    _gd.decompose(&lattice);
    //cout<<"done decomposing" << endl;
    sync_lattice_lm();
    _subproblem = new Subproblem(&lattice, & lm, skip , &_gd, *_cached_words);
    _lagrange_weights = new svector<int, double>();
    _projection =  _subproblem->rand_projection(2);
    _proj_dim = 2;

    //cout<<"ready to roll" << endl;
    //_projection = _subproblem->rand_projection(2);
  }
   
  void solve(double & primal, double & dual, wvector &);
  void update_weights(const wvector & updates,  wvector * weights );
  
 private:
  void debug(int start_from, int dual_mid, int dual_end, string primal_mid, string primal_end);
  void add_subgrad( wvector & subgrad, int start_from, int mid_at, int end_at, bool first);
  double compute_primal(const vector <int> used_edges, const vector <const ForestNode *> used_nodes);
  int lookup_string(string word);
  Subproblem * _subproblem;
  const Forest & _forest;
  const ForestLattice & _lattice;
  const wvector & _weight;
  wvector * _lagrange_weights;
  NgramCache & _lm; 
  GraphDecompose _gd;
  Cache <ForestEdge, double> * _cached_weights;
  Cache <LatNode, int> * _cached_words;
  vector <int > get_lat_edges(int edge_id);
  vector <int > get_lex_lat_edges(int edge_id);
  void sync_lattice_lm();
  void print_output(const wvector & );

  vector <int > _projection;
  int _proj_dim;
  double lm_total, o_total, lag_total;
};

#endif
