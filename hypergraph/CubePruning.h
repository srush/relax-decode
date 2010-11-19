#ifndef CUBEPRUNING_H_
#define CUBEPRUNING_H_

#include <set>
#include "Forest.h"
#include "EdgeCache.h"
#include <queue>
#include "svector.hpp"
using namespace std;
// Some non local feature scorer

typedef vector<int> Sig;

struct Hyp {
public:
  Hyp(){}
  
  Hyp(double score_in, Sig sig_in, vector<int> full_der):
  score(score_in), sig(sig_in), full_derivation(full_der){}

  Sig sig;
  double score;
  vector <int> full_derivation;
  bool operator<(const Hyp & other) const {
    return score < other.score;
  }
};
class NonLocal  {
 public:
  virtual void compute(const ForestEdge &,
                       const vector <vector <int> > &,
                       double & score,
                       vector <int>  & full_derivation,
                       Sig  & sig
                       ) const = 0; 

  virtual Hyp initialize(const ForestNode &) const =0;

};

class BlankNonLocal: public NonLocal {
 public:
  BlankNonLocal() {
    
  }

  void compute(const ForestEdge & edge,
               const vector <vector <int> > & subder,
               double & score,
               vector <int>  & full_derivation,
               Sig  & sig
               ) const {
    score = 0.0;
    sig.push_back(edge.id());
  }

  virtual Hyp initialize(const ForestNode & node) const {
    return Hyp(0.0, vector<int>(), vector<int>()); 
  }
};

struct Candidate {
  Candidate( Hyp h,  const ForestEdge & e,  const vector <int> & v) 
    : hyp(h), edge(e), vec(v){}
  
  Hyp hyp;
  const ForestEdge & edge;
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
 CubePruning(const Forest & forest, const Cache <ForestEdge, double> & weights, const NonLocal & non_local, int k, int ratio):
  _forest(forest), _non_local(non_local), _k(k), _ratio(ratio), _weights(weights),
    _hypothesis_cache(forest.num_nodes()), _oldvec(forest.num_edges())
    {}

  void parse();
  void run(const ForestNode & cur_node, vector <Hyp> & kbest_hyps);
  void init_cube(const ForestNode & cur_node, Candidates & cands);
  void kbest(Candidates & cands, vector <Hyp> &);
  void next(const ForestEdge & cedge, const vector <int > & cvecj, Candidates & cands);
  bool gethyp(const ForestEdge & cedge, const vector <int> & vecj, Hyp & item);
 private:
  //void run(const ForestNode & cur_node);

  const Forest & _forest;
  const NonLocal & _non_local;
  Cache<ForestNode, vector <Hyp> > _hypothesis_cache;
  Cache<ForestEdge, set < vector <int> > > _oldvec;
  //const Cache<ForestNode, Float> & _hypothesis_cache;
  //const PriorityQueue _candidates;
  const int _k;
  const int _ratio;
  const Cache <ForestEdge, double>  & _weights;
};

#endif
