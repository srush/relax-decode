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
  bool is_new;

  bool match (const Hypothesis & other) const {
    return right_side[0] == other.hook[0] && right_side[1] == other.hook[1];
  }

  Hypothesis(vector<int> h, vector<int> r, const ForestEdge * be, int d, bool n) 
    : hook(h), right_side(r), back_edge(be), dim(d), is_new(n) {}

  Hypothesis(int d ):dim(d)  {}  
  Hypothesis(){ dim = -1;}  

  
  //int id() const { return hook[0] + 2 * right_side[0];}
  int id() const { 
    assert(dim != -1);
    assert(dim != 0);
    return hook[0] + (dim) * right_side[0] + (dim * dim)*right_side[1] + (dim*dim*dim)* hook[1];
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


class BestHyp {
 private:
  vector <Hypothesis> hyps;
  vector <double> scores;
  map <int, int> index_by_id;
  map <int, vector<int> > index_by_right;
  
  // have I seen a new score


  //map <int, vector<int> > index_by_right;
 public: 
  bool has_new;
  BestHyp() {
    has_new = false;
  }

  inline int size()  const{
    return hyps.size();
  }

  inline void clear() {
    hyps.clear();
    scores.clear();
    index_by_right.clear();
    index_by_id.clear();
  }

  

  inline const Hypothesis & get_hyp(int i) const {
    return hyps[i];
  }

  inline double get_score(int i) const {
    return scores[i];
  }

  inline vector<int> join(const Hypothesis & other) const {
    map <int, vector<int> >::const_iterator check = 
      index_by_right.find(other.left());
    if (check != index_by_right.end()) {
      return check->second;
    }
    vector <int > result;
    return result;
  }

  inline void try_set_hyp(Hypothesis hyp, double score, bool & set, bool is_new) {
    
    set = false;
    map <int, int >::const_iterator check = 
      index_by_id.find(hyp.id());
    if (check == index_by_id.end()) {
      hyps.push_back(hyp);
      scores.push_back(score);
      // update indexes
      index_by_id[hyp.id()] = hyps.size()-1;
      index_by_right[hyp.right()].push_back(hyps.size()-1);
      set = true;
      has_new = has_new || is_new;
    } else {
      int internal_ind = index_by_id[hyp.id()];
      double old_score = scores[internal_ind];
      if (score < old_score) {
        hyps[internal_ind] = hyp;
        scores[internal_ind] = score;
        set = true;
        has_new = has_new || is_new;
      }
    }
  }
};


//typedef StoreCache <Hypothesis, double> BestHyp;

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
    Hypothesis h(vector<int>(),vector<int>(), NULL, dim(), true);
    bool w;
    hyps.try_set_hyp(h, 0.0, w, true);
  }
  
  double find_best( BestHyp & at_root, Hypothesis & best_hyp) const {
    //BestHyp::const_iterator iter, check;
    double best = 1e20;
    for (int iter = 0; iter < at_root.size(); iter++) {
      //if (!at_root.has_key(iter)) continue; 
      Hypothesis hyp = at_root.get_hyp(iter);
      double score = at_root.get_score(iter);
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
  ExtendCKY(const Forest & forest):
    _forest(forest), _memo_table(forest.num_nodes()) {
    _is_first = true;
  }
    
    double best_path(NodeBackCache & back_pointers);
    void set_params(Cache <ForestEdge, double> * edge_weights,  Controller * cont) {
      _old_edge_weights = _edge_weights;
      _edge_weights = edge_weights;
      _controller = cont;
    }
 private:
  const Forest & _forest;
  Cache <ForestEdge, double>  * _edge_weights;
  Cache <ForestEdge, double>  * _old_edge_weights;
  Controller * _controller;
  //Cache <ForestNode, BestHyp > * _old_memo_table;
  Cache <ForestNode, BestHyp >  _memo_table;

  void node_best_path(const ForestNode & node); 
  void extract_back_pointers(const ForestNode & node, const Hypothesis & best_hyp, 
                                        NodeBackCache & back_pointers);

  vector <BestHyp *> _to_delete;
  bool _is_first;
};




#endif
