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
 Decode(const Forest & forest, const ForestLattice & lattice, const wvector & weight, LM & lm): 
  _forest(forest), _lattice(lattice), _weight(weight), _lm(lm)
  {
    _cached_weights = cache_edge_weights(forest, weight);
    _gd.decompose(&lattice);
    _subproblem = new Subproblem(&lattice, & lm, &_gd, *_cached_words);
    sync_lattice_lm();
  }
  
  wvector & solve(double & primal, double & dual);
  void update_weights(const wvector & updates, const wvector & weights );

 private:
  Subproblem * _subproblem;
  const Forest & _forest;
  const ForestLattice & _lattice;
  const wvector & _weight;
  LM & _lm; 
  GraphDecompose _gd;
  Cache <ForestEdge, double> * _cached_weights;
  Cache <LatNode, int> * _cached_words;
  vector <int > get_lat_nodes(int edge_id);
  vector <int > get_lex_lat_nodes(int edge_id);
  void sync_lattice_lm();
};

#endif
