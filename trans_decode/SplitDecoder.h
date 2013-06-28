#ifndef SPLITDECODER_H_
#define SPLITDECODER_H_

#include <vector>

#include <Forest.h>
#include <ForestLattice.h>
#include "EdgeCache.h"
#include "Hypergraph.h"
#include "AStar.h"
#include "../common.h"
#include "ExtendCKY.h"

#include "dual_subproblem.h"

#define BACK 2

using namespace Scarab::HG;

class SplitHeuristic : public Heuristic {
 public :
  SplitHeuristic(const Cache<Hypernode, BestHyp *> &outside_scores,
                 Cache<Hyperedge, vector<BestHyp> *> &outside_edge_scores)
  : _outside_scores(outside_scores),
    _outside_edge_scores(outside_edge_scores) {}

  bool has_value(const Location & l,
                 const Hypothesis & hyp) const;

  double get_value(const Location & l,
                   const Hypothesis & hyp) const;


 protected:
  int lower_id(const Hypothesis & hyp) const {
    return Hypothesis(hyp.hook.project(BACK, 2),
                      hyp.right_side.project(BACK, 2)).id();
  }

  const Cache <Hypernode , BestHyp *> & _outside_scores;
  const Cache <Hyperedge, vector<BestHyp> *>  & _outside_edge_scores;
};

class SplitController : public Controller {
 public:
  SplitController(const Subproblem & s,
                  const ForestLattice & l,
                  bool two_classes);

  int project_word(int w) const;

  int size()  const {
    int d = dim();
    return d * d * d * d;
  }
    int dim() const {
    return _classes.size();
  }

  void initialize_hypotheses(const Hypernode & node,
                             vector <Hypothesis *> & hyps,
                             vector <double> & scores) const;

  void initialize_out_root(vector <Hypothesis *> & hyps,
                           vector <double> & scores)  const;

  double find_best(vector <Hypothesis *> & root_hyps,
                   vector<double > & scores,
                   Hypothesis & best_hyp) const;

 private:
  const Subproblem & _subproblem;
  const ForestLattice & _lattice;
  bool _two_classes;
  vector <vector <int> >  _classes;
  vector <int> _inner_projection;
};

#endif
