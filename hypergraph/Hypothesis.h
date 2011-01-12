#ifndef HYPOTHESIS_H_
#define HYPOTHESIS_H_


#include <assert.h>
#include <map>
#include <vector>
#include "Hypergraph.h"
#include "EdgeCache.h"
#include "../common.h"
using namespace std;

namespace Scarab {
  namespace HG {

struct Hypothesis;

//typedef Cache <Hypernode, const Hyperedge *> NodeBackCache;


int make_id(const vector <int> & hook, const vector <int> & right_side, int dim);

void show_hyp(const Hypothesis & hyp);

struct Hypothesis {
public:

  Hypothesis(vector<int> h, vector<int> r, const Hyperedge * be, int d, bool n) 
  : hook(h), right_side(r), back_edge(be), dim(d), is_new(n), is_done(false) {}

  Hypothesis(int d ):dim(d), is_done(false)  {}  


  // TODO: Remove
  Hypothesis(): is_done(false){ dim = -1;}  


  //vector <const int> sig;  
  vector <int> hook;
  vector <int> right_side;
  const Hyperedge * back_edge;
  int dim;

  bool is_new;
  bool is_done;

  
  

  // by hyp id
  vector <int> prev_hyp;
  double original_value;


  bool match (const Hypothesis & other) const {
    return right_side[0] == other.hook[0] && right_side[1] == other.hook[1];
  }

  
  //int id() const { return hook[0] + 2 * right_side[0];}
  int id() const { 
    assert(dim != -1);
    assert(dim != 0);
    return make_id(hook, right_side, dim);
  }

  int left() const {
    return hook[0] + (dim)* hook[1];
  }

  int right() const {
    return right_side[0] + (dim)* right_side[1];
  }


  bool operator<(const Hypothesis & other) const {
    assert(hook.size() == other.hook.size());
    assert(right_side.size() == other.right_side.size());
    if (hook != other.hook) {
      return hook < other.hook;
    } else {
      return right_side < other.right_side;
    }
  }

};


class Controller {
 public: 

  virtual double combine(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const  = 0;
  virtual double combine_back(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const  = 0;
  virtual void initialize_hypotheses(const Hypernode & node, vector <Hypothesis *> & initialize, vector <double> & scores) const = 0;
  virtual void initialize_out_root(vector <Hypothesis *> & hyps, vector <double> & scores)  const =0;
  virtual double find_best( vector <Hypothesis *> & at_root, vector <double> & scores, Hypothesis & best_hyp) const= 0;
  virtual int size() const =0;
  virtual int dim() const =0;
  virtual double prune_to() const =0;
};

class TrivialController : public Controller {
 public:
  int size() const {return 1;}
  double prune_to() const {return 1;}
  int dim() const {return 1;}
  double combine(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const {
    ret.hook = vector<int>();
    ret.right_side = vector<int>();
    foreach (int hyp, a.prev_hyp) { //int i=0;i<a.prev_hyp.size();i++) {
      ret.prev_hyp.push_back(hyp);
    }
    ret.prev_hyp.push_back(b.id());
    return 0.0;
  }

  double combine_back(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const {
    return combine(a,b, ret);
  }
  void initialize_out_root(vector<Hypothesis *> & hyps, vector <double> & scores) const {    
    return;
  }
  void initialize_hypotheses(const Hypernode & node, vector<Hypothesis *> & hyps, vector <double> & scores) const {    
    Hypothesis * h = new Hypothesis(vector<int>(),vector<int>(), NULL, dim(), true);
    hyps.push_back(h);
    scores.push_back(0.0);
    //   bool w;
    //hyps.try_set_hyp(h, 0.0, w, true);
  }
  
  double find_best( vector <Hypothesis *>  & at_root, vector <double> & scores, Hypothesis & best_hyp) const {
    //BestHyp::const_iterator iter, check;
    double best = 1e20;
    for (unsigned int iter = 0; iter < at_root.size(); iter++) {
      //if (!at_root.has_key(iter)) continue; 
      Hypothesis hyp = *at_root[iter];
      double score = scores[iter];
      if (score < best) {
        best = score;
        best_hyp = hyp;
      }
    }
    return best;
  }
};
  }
}

#endif
