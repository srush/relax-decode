#ifndef EXTENDCKY_H_
#define EXTENDCKY_H_

#include "EdgeCache.h"
#include "Forest.h"
#include <assert.h>
#include <map>
#include <vector>

// #define PDIM 3

using namespace std;
typedef Cache <ForestNode, const ForestEdge *> NodeBackCache;




struct Hypothesis {
  //vector <const int> sig;  
  vector <int> hook;
  vector <int> right_side;
  int dim;
  const ForestEdge * back_edge;
  vector <const Hypothesis * > prev_hyp;
  double original_value;

  bool match (const Hypothesis & other) const {
    return right_side[0] == other.hook[0] && right_side[1] == other.hook[1];
  }

  Hypothesis(vector<int> h, vector<int> r, const ForestEdge * be, int d) 
    : hook(h), right_side(r), back_edge(be), dim(d) {}

  Hypothesis(int d ):dim(d)  {}  
  Hypothesis(){ dim = -1;}  

  
  //int id() const { return hook[0] + 2 * right_side[0];}
  int id() const { 
    assert(dim != -1);
    assert(dim != 0);
    return hook[0] + (dim) * right_side[0] + (dim * dim)*right_side[1] + (dim*dim*dim)* hook[1];
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
typedef StoreCache <Hypothesis, double> BestHyp;

class Controller {
 public: 
  virtual double combine(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const  = 0;
  virtual void initialize_hypotheses(const ForestNode & node, BestHyp & initialize) const = 0;
  virtual double find_best( BestHyp & at_root, Hypothesis & best_hyp) const= 0;
  virtual int size() const =0;
  virtual int dim() const =0;
};

class TrivialController : public Controller {
 public:
  int size() const {return 1;}
  int dim() const {return 1;}
  double combine(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const {
    ret.hook = vector<int>();
    ret.right_side = vector<int>();
    for (int i=0;i<a.prev_hyp.size();i++) {
      ret.prev_hyp.push_back(a.prev_hyp[i]);
    }
    ret.prev_hyp.push_back(&b);
    return 0.0;
  }

  void initialize_hypotheses(const ForestNode & node, BestHyp & hyps) const {    
    Hypothesis h(vector<int>(),vector<int>(), NULL, dim());
    hyps.set_value(h, 0.0);
  }
  
  double find_best( BestHyp & at_root, Hypothesis & best_hyp) const {
    //BestHyp::const_iterator iter, check;
    double best = 1e20;
    for (int iter = 0; iter < at_root.size(); iter++) {
      if (!at_root.has_key(iter)) continue; 
      Hypothesis hyp = at_root.full_keys[iter];
      double score = at_root.store[iter];
      if (score < best) {
        best = score;
        best_hyp = hyp;
      }
    }
    return best;
  }
};


class ExtendCKY {
 public:
  ExtendCKY(const Forest & forest, const Cache <ForestEdge, double> & edge_weights, const Controller & cont): 
    _forest(forest), _memo_table(forest.num_nodes()), _edge_weights(edge_weights), _controller(cont){}

    double best_path(NodeBackCache & back_pointers);

 private:
  const Forest & _forest;
  const Cache <ForestEdge, double> & _edge_weights;
  const Controller & _controller;
  Cache <ForestNode, BestHyp > _memo_table;

  void node_best_path(const ForestNode & node); 
  void extract_back_pointers(const ForestNode & node, const Hypothesis & best_hyp, 
                                        NodeBackCache & back_pointers);

};




#endif
