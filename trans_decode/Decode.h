#ifndef DECODE_H_
#define DECODE_H_

#include <map>
#include <string>
#include <set>
#include <vector>

#include "Subgradient.h"
#include <Forest.h>
#include <ForestLattice.h>
#include <HypergraphAlgorithms.h>
#include "svector.hpp"
#include <Ngram.h>
#include "dual_subproblem.h"
#include "EdgeCache.h"
#include "ExtendCKY.h"

using namespace std;

#define GRAMSPLIT 100000
#define GRAMSPLIT2 200000

typedef svector<int, double> wvector;
using namespace Scarab::HG;


class Decode: public SubgradientProducer {
 public:
  static const int kCubing = 1;
  static const int kProjecting = 2;

  Decode(const Forest & forest,
         const ForestLattice & lattice,
         const wvector & weight,
         NgramCache & lm)
    :_forest(forest),
      _lattice(lattice),
      _weight(weight),
      _lm(lm),
      _gd(lattice),
      approx_mode_(false) {
    _cached_weights = HypergraphAlgorithms(forest).cache_edge_weights(weight);

    _gd.decompose();

    sync_lattice_lm();
    _subproblem = new Subproblem(&lattice, & lm, &_gd, *_cached_words);
    _lagrange_weights = new svector<int, double>();
    _maintain_constraints = false;
    _is_stuck_round = 10000;
  }

  ~Decode() {
    delete _subproblem;
    /* delete _lagrange_weights; */
    delete _cached_words;
  }

  void solve(const SubgradState & state, SubgradResult & result);
  void update_weights(const wvector & updates,  wvector * weights);

  void set_cached_words(Cache<Hypernode, int> *cached_words) {
    cached_cube_words_ = cached_words;
  }

  void set_approx_mode(bool approx_mode) {
    approx_mode_ = approx_mode;
  }

  void set_ilp_mode(int ilp_mode) {
    ilp_mode_ = ilp_mode;
  }

 private:
  void debug(int start_from,
             int dual_mid,
             int dual_end,
             int primal_mid,
             int primal_end);
  void greedy_projection(int dual_mid,
                         int dual_end,
                         int primal_mid,
                         int primal_end);
  void add_subgrad(wvector *subgrad,
                   int start_from,
                   int mid_at,
                   int end_at,
                   bool first);
  double compute_primal(HEdges used_edges,
                        const vector <const ForestNode *> used_nodes,
                        const EdgeCache & edge_lag);
  int lookup_string(string word);
  Subproblem * _subproblem;
  const Forest & _forest;
  const ForestLattice & _lattice;
  const wvector & _weight;
  wvector * _lagrange_weights;
  NgramCache & _lm;
  GraphDecompose _gd;
  Cache<Hyperedge, double> * _cached_weights;
  Cache<Graphnode, int> * _cached_words;
  map<string, vector<int> >  _cached_word_str;
  vector<int> get_lat_edges(int edge_id);
  vector<int> get_lex_lat_edges(int edge_id);
  void sync_lattice_lm();
  void print_output(const wvector &vector);

  vector<int> _projection;
  int _proj_dim;
  double lm_total, o_total, lag_total;

  map<int, set <int> > _constraints;
  map<string, set <string> > _constraints_words;
  bool _maintain_constraints;
  int _is_stuck_round;

  bool solve_ngrams(int round, bool is_stuck);
  EdgeCache compute_edge_penalty_cache();
  double best_modified_derivation(const EdgeCache& penalty_cache,
                                  const HypergraphAlgorithms & ha,
                                  NodeBackCache & back_pointers);
  wvector construct_parse_subgrad(const HEdges used_edges);
  wvector construct_lm_subgrad(const vector <const ForestNode*> &used_words,
                               const vector <int> & used_lats,
                               const vector <string> & used_strings,
                               double & dual,
                               double & cost_total);
  void remove_lm(int feat, wvector & subgrad,
                 double & dual, double &cost_total);

  Cache<Hypernode, int> * cached_cube_words_;


  // Use approximation when calculating subgradients.
  bool approx_mode_;

  // Method for tightening the ilp.
  int ilp_mode_;
};

#endif
