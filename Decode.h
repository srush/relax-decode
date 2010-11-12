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

typedef svector<int, double> wvector;

class Decode: public SubgradientProducer {
 public:
 Decode(const Forest & forest, const ForestLattice & lattice, const wvector & weight, Ngram & lm) 
   :_forest(forest), _lattice(lattice), _weight(weight), _lm(lm)
  {
    _cached_weights = cache_edge_weights(forest, weight);
    cout<<"decomposing" << endl;
    _gd.decompose(&lattice);
    cout<<"done decomposing" << endl;
    sync_lattice_lm();
    _subproblem = new Subproblem(&lattice, & lm, &_gd, *_cached_words);
    _lagrange_weights = new svector<int, double>();
    cout<<"ready to roll" << endl;
  }
  
  void solve(double & primal, double & dual, wvector &);
  void update_weights(const wvector & updates,  wvector * weights );

 private:
  double compute_primal(const vector <int> used_edges, const vector <const ForestNode *> used_nodes);
  int lookup_string(string word);
  Subproblem * _subproblem;
  const Forest & _forest;
  const ForestLattice & _lattice;
  const wvector & _weight;
  wvector * _lagrange_weights;
  Ngram & _lm; 
  GraphDecompose _gd;
  Cache <ForestEdge, double> * _cached_weights;
  Cache <LatNode, int> * _cached_words;
  vector <int > get_lat_nodes(int edge_id);
  vector <int > get_lex_lat_nodes(int edge_id);
  void sync_lattice_lm();
};

#endif
