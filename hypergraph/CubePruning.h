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
  
Hyp(double score_in, double heuristic_in, Sig sig_in, vector<int> full_der, const vector<int> &edges_):
  score(score_in), total_heuristic(heuristic_in), sig(sig_in), full_derivation(full_der), edges(edges_){}
  double score;
  Sig sig;
  vector <int> full_derivation;
  vector<int> edges;
  double total_heuristic;
  bool operator<(const Hyp & other) const {
    //return total_heuristic < other.total_heuristic;
    
    return score < other.score;
  }
};
class NonLocal  {
 public:
  //virtual ~NonLocal() {};
  virtual void compute(const Hyperedge &,
                       int edge_pos,
                       const vector<vector <int> > &,
                       double &score,
                       vector <int>  &full_derivation,
                       Sig &sig
                       ) const = 0; 

  virtual Hyp initialize(const Hypernode &) const =0;

};

class BlankNonLocal: public NonLocal {
 public:
  BlankNonLocal() {
    
  }

  void compute(const Hyperedge & edge,
               int edge_pos,
               const vector <vector <int> > & subder,
               double & score,
               vector <int>  & full_derivation,
               Sig  & sig
               ) const {
    score = 0.0;
    sig.push_back(edge.id());
  }

  virtual Hyp initialize(const Hypernode & node) const {
    return Hyp(0.0, 0.0, vector<int>(), vector<int>(), vector<int>()); 
  }
};

struct Candidate {
  Candidate(Hyp h, const Hyperedge &e,  const vector<int> &v) 
    : hyp(h), edge(e), vec(v){}
  
  Hyp hyp;
  const Hyperedge &edge;
  vector <int> vec;

  bool operator<(const Candidate & other ) const {
    return hyp < other.hyp;
  }  
};

struct candidate_compare : public std::binary_function<Candidate*, Candidate*, bool> {
  bool operator()(const Candidate * a, const Candidate * b) const {
    return (*b) <  (* a) ; 
  }
};

typedef priority_queue <Candidate *, vector<Candidate*>, candidate_compare> Candidates;


class CubePruning {
 public:
 CubePruning(const HGraph & forest, const Cache <Hyperedge, double> & weights, const NonLocal & non_local, 
             int k, int ratio):
  _forest(forest), _weights(weights), _non_local(non_local), _k(k), _ratio(ratio), 
    _hypothesis_cache(forest.num_nodes()), _oldvec(forest.num_edges()),
    use_bound_(false), use_heuristic_(false), fail_(false)
    {}
   
  void get_derivation(vector<int> &der);
  int  get_num_derivations();
  void get_derivation(vector<int> &der, int n);
  void get_edges(vector<int> &edges, int n);
  double get_score(int n);

  double parse();
  void run(const Hypernode & cur_node, vector <Hyp> & kbest_hyps);
  void init_cube(const Hypernode & cur_node, Candidates &cands);
  void kbest(Candidates & cands, vector <Hyp> &, bool recombine);
  void next(const Hyperedge & cedge, const vector <int > & cvecj, Candidates & cands);

  bool gethyp(const Hyperedge & cedge, const vector <int> & vecj, Hyp & item, bool, bool *bounded, bool *early_bounded);
  bool has_derivation();
  void kbest_enum(const Hypernode &node,
                  vector <Hyp> &newhypvec);

  void set_duals(const Cache<Hyperedge, double>  *dual_scores) {
    dual_scores_ = dual_scores;
  }
  void set_bound(double bound) {
    use_bound_ = true;
    bound_ = bound;
  }
  void set_heuristic(const Cache<Hypernode, double>  *heuristic) {
    use_heuristic_ = true;
    heuristic_ = heuristic;
  }
 
  void set_edge_heuristic(const Cache <Hyperedge, vector<BestHyp> > *heuristic) {
    edge_heuristic_ = heuristic;
  }
  bool failed() { return fail_; }


 private:

  bool gethyp_enum(const Hyperedge & cedge, 
                   int pos,
                   bool unary,
                   const Hyp &first,
                   const Hyp &second,
                   Hyp &item, 
                   bool *bounded, 
                   bool *early_bounded);
  //void run(const Hypernode & cur_node);

  double bound_;
  bool use_bound_;

  const Cache<Hypernode, double>  *heuristic_;
  const Cache <Hyperedge, vector<BestHyp> > *edge_heuristic_;
  bool use_heuristic_;

  const Cache <Hyperedge, double>  *dual_scores_;

  const HGraph & _forest;
  const Cache <Hyperedge, double>  & _weights;
  const NonLocal & _non_local;
  const uint _k;
  const uint _ratio;

  Cache<Hypernode, vector<Hyp> > _hypothesis_cache;
  Cache<Hyperedge, set<vector <int> > > _oldvec;
  //const Cache<Hypernode, Float> & _hypothesis_cache;
  //const PriorityQueue _candidates;

  bool fail_;
};

#endif
