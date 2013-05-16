#ifndef CUBEPRUNING_H_
#define CUBEPRUNING_H_

#include <set>
#include "Hypergraph.h"
#include "EdgeCache.h"
#include <queue>
#include "svector.hpp"
#include "AStar.h"
using namespace Scarab::HG;
using namespace std;
// Some non local feature scorer

#define DEBUG_CUBE 0
typedef vector<int> Sig;

struct Hyp {
public:
  Hyp(){}

Hyp(double score_in,
    double heuristic_in,
    Sig sig_in,
    vector<int> full_der,
    const vector<int> &edges_)
  : score(score_in),
    total_heuristic(heuristic_in),
    sig(sig_in),
    full_derivation(full_der),
    edges(edges_){

  }

  // The inside score of the hypothesis.
  double score;

  // The inside score + outside heuristic of the hypothesis.
  double total_heuristic;

  // A signature indicating the current fsa states of the hypothesis.
  Sig sig;

  // The representative full derivation of the hypothesis.
  vector <int> full_derivation;

  vector<int> edges;

  // Comparison operator, chooses lower inside score.
  bool operator<(const Hyp & other) const {
    //return total_heuristic < other.total_heuristic;
    return score < other.score;
  }
};

// Interface for non-local scoring functions. Can be thought of
// as moving dual values to primal values.
class NonLocal  {
 public:

  //virtual ~NonLocal() {};
  // Compute the non-local score by combining the sub_ders.
  virtual bool compute(const Hyperedge &edge,
                       int edge_pos,
                       double bound,
                       const vector<const vector <int> *> &sub_ders,
                       double &score,
                       vector <int> &full_derivation,
                       Sig &sig) const = 0;

  // Initialize a hypothesis for a hypernode.
  virtual Hyp initialize(const Hypernode &) const =0;
};


// A trivial instantiation of non-local.
class BlankNonLocal: public NonLocal {
 public:
  BlankNonLocal() {}

  bool compute(const Hyperedge &edge,
               int edge_pos,
               double,
               const vector<const vector <int> *> &subder,
               double &score,
               vector<int>  &full_derivation,
               Sig &sig
               ) const {
    score = 0.0;
    sig.push_back(edge.id());
  }

  virtual Hyp initialize(const Hypernode &node) const {
    return Hyp(0.0, 0.0, vector<int>(), vector<int>(), vector<int>());
  }
};

// A candidate.
struct Candidate {
  Candidate(Hyp h, const Hyperedge &e,  const vector<int> &v)
  : hyp(h),
    edge(e),
    vec(v) {}

  // The current hypothesis.
  Hyp hyp;

  // The associated edge.
  const Hyperedge &edge;

  // The back pointers to previous candidates.
  vector <int> vec;

  bool operator<(const Candidate & other) const {
    return hyp < other.hyp;
  }
};

struct candidate_compare :
  public std::binary_function<Candidate*, Candidate*, bool> {
  bool operator()(const Candidate * a, const Candidate * b) const {
    return (*b) <  (* a) ;
  }
};

typedef priority_queue <Candidate *, vector<Candidate*>, candidate_compare> Candidates;


class CubePruning {
 public:
 CubePruning(const HGraph & forest,
             const Cache <Hyperedge, double> & weights,
             const NonLocal & non_local,
             int k,
             int ratio)
  : _forest(forest),
    _weights(weights),
    _non_local(non_local),
    _k(k),
    _ratio(ratio),
    _hypothesis_cache(forest.num_nodes()), _oldvec(forest.num_edges()),
    use_bound_(false),
    use_heuristic_(false),
    fail_(false) {
    check_ = 0;
    check_bounded_ = 0;
    cube_enum_ = false;
  }


  int  get_num_derivations() {
    return _hypothesis_cache.store[_forest.root().id()].size();
  }

  void get_derivation(vector<int> &der, int n) {
    der = _hypothesis_cache.store[_forest.root().id()][n].full_derivation;
  }

  void get_derivation(vector<int> &der) { get_derivation(der, 0); }


  bool has_derivation() {
    return _hypothesis_cache.has_value[_forest.root().id()];
  }

  void get_edges(vector<int> &edges, int n) {
    edges = _hypothesis_cache.store[_forest.root().id()][n].edges;
  }

  double get_score(int n) {
    return _hypothesis_cache.store[_forest.root().id()][n].score;
  }

  double parse() {
    run(_forest.root(), _hypothesis_cache.store[_forest.root().id()]);
    return _hypothesis_cache.store[_forest.root().id()][0].score;
  }

  void set_duals(const Cache<Hyperedge, double> *dual_scores) {
    dual_scores_ = dual_scores;
  }

  void set_bound(double bound) {
    use_bound_ = true;
    bound_ = bound;
  }

  void set_heuristic(const Cache<Hypernode, double> *heuristic) {
    use_heuristic_ = true;
    heuristic_ = heuristic;
  }

  void set_edge_heuristic(const Cache <Hyperedge, vector<BestHyp> *> *heuristic) {
    edge_heuristic_ = heuristic;
  }

  void set_cube_enum() {
    cube_enum_ = true;
  }

  bool is_exact() { return !fail_; }

 private:

  void run(const Hypernode & cur_node,
           vector <Hyp> & kbest_hyps);

  void init_cube(const Hypernode & cur_node,
                 Candidates &cands);

  // Implementation of Chiang and Huang, k-best algorithm 2.
  void kbest(Candidates & cands,
             vector <Hyp> &,
             bool recombine);

  // @param cedge - the edge that we just took a candidate from
  // @param cvecj - the current position on the cedge cube
  // @param cands - current candidate list
  // for each dimension of the cube
  void next(const Hyperedge & cedge,
            const vector <int > & cvecj,
            Candidates & cands);


  // Return the score and signature of the element obtained from combining the
  // vecj-best parses along cedge. Also, apply non-local feature functions (LM)

  bool gethyp(const Hyperedge & cedge,
              const vector <int> & vecj,
              Hyp & item, bool,
              bool *bounded,
              bool *early_bounded);

  void kbest_enum(const Hypernode &node,
                  vector <Hyp> &newhypvec);

  bool gethyp_enum(const Hyperedge & cedge,
                   int pos,
                   bool unary,
                   const Hyp &first,
                   const Hyp &second,
                   Hyp &item,
                   bool *bounded,
                   bool *early_bounded);


  double bound_;
  bool use_bound_;

  const Cache<Hypernode, double>  *heuristic_;
  const Cache <Hyperedge, vector<BestHyp> * > *edge_heuristic_;
  bool use_heuristic_;

  const Cache <Hyperedge, double>  *dual_scores_;

  const HGraph & _forest;
  const Cache <Hyperedge, double>  & _weights;
  const NonLocal & _non_local;
  const uint _k;
  const uint _ratio;

  int check_;
  int check_bounded_;

  Cache<Hypernode, vector<Hyp> > _hypothesis_cache;
  Cache<Hyperedge, set<vector <int> > > _oldvec;

  bool fail_;
  bool cube_enum_;
};

#endif
