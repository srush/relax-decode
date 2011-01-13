#ifndef HYPOTHESIS_H_
#define HYPOTHESIS_H_


#include <assert.h>
#include <map>
#include <vector>
#include "Hypergraph.h"
#include "EdgeCache.h"
#include "../common.h"
#include <cmath>
using namespace std;

namespace Scarab {
  namespace HG {

struct Hypothesis;


// fsa state 
struct State {
  State(){}
  State (const vector <int> & ids, uint dim) :_state(ids), _dim(dim){}

  friend ostream& operator<<(ostream& output, const State& p);
  
  State project(int split, int down_to) const {
    vector <int> new_state(2);
    new_state[0] = _state[0] < split ? 0 : 1;
    new_state[1] = _state[1] < split ? 0 : 1;
    return State(new_state, down_to);
  }

  int id() const {
    int mult = 1;
    int total = 0.0;
    foreach (int sub_state, _state) {
      total += mult *sub_state;
      mult*= _dim;
    } 
    return total;
  }

  bool operator==(const State & other) const {
    assert (other._dim == _dim);
    return other._state == _state;
  }

  bool compatible(const State & other) const {
    return _dim == other._dim;
  }

  bool operator<(const State & other) const {
    assert (other._dim == _dim);
    return other._state < _state;
  }


  int possible_states() const {
    return pow(_dim, _state.size());
  }

  // TODO : private
  vector<int> _state;
protected:
  
  uint _dim; 
  
};

//typedef vector<int> State;

void show_hyp(const Hypothesis & hyp);



struct Hypothesis {
public:

  
  /** 
   * A Hypothesis represents the path through an intersection 
   * of a hypergraph and finite state automata.
   *
   * @param left_hook The expected fsa state on the left hand side  
   * @param right The current fsa state on the right hand side
   * @param back_pointer The last hyperedge taken on the path
   * 
   * @return 
   */  
  Hypothesis(const State & left_hook, 
             const State & right, 
             HEdge back_pointer) 
  : hook(left_hook), right_side(right), back_edge(back_pointer), is_done(false) {
    assert(hook.compatible(right_side));
  }

  
  Hypothesis(const State & left_hook, 
             const State & right) : hook(left_hook), right_side(right), back_edge(NULL), is_done(false) {}


  Hypothesis(int d ):is_done(false) {}  
  

  Hypothesis(): is_done(false) {}  

  friend ostream& operator<<(ostream& output, const Hypothesis& h);

  State hook;
  State right_side;
  const Hyperedge * back_edge;
  bool is_done;

  
  // by hyp id
  
  vector <int> prev_hyp;
  double original_value;

  /** 
   * Is it valid to combine this hypothesis with another 
   * (do the fsa states match up)
   * @param other The hypothesis to join on the right
   * 
   * @return True, if they match
   */
  bool match (const Hypothesis & other) const {
    return right_side == other.hook;
  }

  /** 
   * Get the unique identifier for this hypothesis.
   * 
   * @return 
   */
  int id() const { 
    //assert(dim != -1);
    //assert(dim != 0);
    return hook.id() + right_side.possible_states() * right_side.id();
  }

  /** 
   * Get the unique identifier for the left state of the hypothesis.
   * 
   * @return Left side id
   */
  int left() const {
    return hook.id();
  }

  /** 
   * Get the unique identifier for the right state of the hypothesis.
   * 
   * @return Right side id
   */
  int right() const {
    return right_side.id();
  }


  bool operator<(const Hypothesis & other) const {
    assert(hook.compatible(other.hook));
    assert(right_side.compatible(other.right_side));
    if (!(hook == other.hook)) {
      return hook < other.hook;
    } else {
      return right_side < other.right_side;
    }
  }

};


class Controller {
 public: 

  /** 
   * 
   * 
   * @param a Left hypothesis
   * @param b Right Hypothesis
   * @param ret New constructed hypothesis
   * 
   * @return Added weight
   */
  double combine(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const {
    ret.hook = a.hook;
    ret.right_side = b.right_side;
        
    // append 
    ret.prev_hyp.insert(ret.prev_hyp.end(), a.prev_hyp.begin(), a.prev_hyp.end());
    ret.prev_hyp.push_back(b.id());
    return 0.0;
  }

  double combine_back(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const {
    ret.hook = b.hook;
    ret.right_side = a.right_side;
    
    ret.prev_hyp.insert(ret.prev_hyp.end(), a.prev_hyp.begin(), a.prev_hyp.end());
    
    ret.prev_hyp.push_back(b.id());

    return 0.0;
  }
  
  virtual void initialize_hypotheses(const Hypernode & node, 
                                     vector <Hypothesis *> & initialize, 
                                     vector <double> & scores) const = 0;
  virtual void initialize_out_root(vector <Hypothesis *> & hyps, vector <double> & scores)  const =0;
  virtual double find_best( vector <Hypothesis *> & at_root, vector <double> & scores, Hypothesis & best_hyp) const= 0;

  virtual int size() const =0;
  virtual int dim() const =0;
};

/* class TrivialController : public Controller { */
/*  public: */
/*   int size() const {return 1;} */
/*   int dim() const {return 1;} */
/*   double combine(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const { */
/*     ret.hook = vector<int>(); */
/*     ret.right_side = vector<int>(); */
/*     foreach (int hyp, a.prev_hyp) { //int i=0;i<a.prev_hyp.size();i++) { */
/*       ret.prev_hyp.push_back(hyp); */
/*     } */
/*     ret.prev_hyp.push_back(b.id()); */
/*     return 0.0; */
/*   } */

/*   double combine_back(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const { */
/*     return combine(a,b, ret); */
/*   } */
/*   void initialize_out_root(vector<Hypothesis *> & hyps, vector <double> & scores) const {     */
/*     return; */
/*   } */
/*   void initialize_hypotheses(const Hypernode & node, vector<Hypothesis *> & hyps, vector <double> & scores) const {     */
/*     Hypothesis * h = new Hypothesis(vector<int>(),vector<int>(), NULL, dim()); */
/*     hyps.push_back(h); */
/*     scores.push_back(0.0); */
/*     //   bool w; */
/*     //hyps.try_set_hyp(h, 0.0, w, true); */
/*   } */
  
/*   double find_best( vector <Hypothesis *>  & at_root, vector <double> & scores, Hypothesis & best_hyp) const { */
/*     //BestHyp::const_iterator iter, check; */
/*     double best = 1e20; */
/*     for (unsigned int iter = 0; iter < at_root.size(); iter++) { */
/*       //if (!at_root.has_key(iter)) continue;  */
/*       Hypothesis hyp = *at_root[iter]; */
/*       double score = scores[iter]; */
/*       if (score < best) { */
/*         best = score; */
/*         best_hyp = hyp; */
/*       } */
/*     } */
/*     return best; */
/*   } */
/* }; */
/*   } */
/* } */

  } }
#endif
