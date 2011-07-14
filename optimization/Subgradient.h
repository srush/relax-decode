#ifndef SUBGRADIENT_H_
#define SUBGRADIENT_H_

#include "svector.hpp"
#include <vector>
#include <../common.h>
using namespace std;

typedef svector<int, double> wvector;

class SubgradRate {
 public:
  virtual double get_alpha(vector <double> & duals,
                           vector <double> & primals,
                           int size, bool aggressive, bool is_stuck) = 0;
  virtual void bump() {}
};

// Input to the subgradient client
struct SubgradState {
  // The current round of the subgradient algorithm 
  int round;
  // ignore for now
  bool is_stuck;
  bool no_update;
};

// Output of the subgradient client
struct SubgradResult {
  // The primal value (score of the resulting structure)
  double primal;
  // The dual value (score of the resulting structure with dual penalties)
  double dual;
  // The subgradient at this iteration
  wvector subgrad;
  // ignore
  bool bump_rate;
};


/**
 * Interface for subgradient clients. 
 */
class SubgradientProducer {
 public:
  // Solve the problem with the current dual weights.
  virtual void solve(const SubgradState & cur_state, SubgradResult & result) = 0;

  // Update the dual variables. Updates is the delta for this round, 
  // weights is a pointer to the current weights.
  virtual void update_weights(const wvector & updates, wvector * weights )=0;
};


/** 
 * Subgradient optimization manager. Takes an object to produce
 * subgradients given current dual values as well as an object 
 * to determine the current update rate.
 */
class Subgradient {
 public:

  /** 
   * 
   * @param subgrad_producer Gives the subgradient at the current position 
   * @param update_rate A class to decide the alpha to use at the current iteration
   */
 Subgradient(SubgradientProducer & subgrad_producer, 
             SubgradRate & update_rate ): 
  _s(subgrad_producer), 
  _rate(update_rate){
    _best_dual = -1e20;
    _best_primal = 1e20;
    _round = 1;
    _nround = 1;
    _is_stuck = false;
    _first_stuck_iteration = -1;
    _aggressive = false;
    _debug = false;
    _max_round = 200;
  } ;

  void set_debug(){ _debug = true;} 
  void set_max_rounds(int max_round){_max_round = max_round;}

  void solve(int example);

  /** 
   * As the optimization probably reached a  fixed point
   * 
   * @return true when stuck 
   */
  bool is_stuck() const {
    return _is_stuck;
  }



 private:
  bool run_one_round();  

  
  SubgradientProducer & _s;

  double _best_dual, _best_primal;
  
  bool _aggressive;
  void update_weights(wvector & subgrad, bool);
  int _round, _nround;
  wvector _weights;
  vector <double> _primals;
  vector <double> _duals;
  double _base_weight;
  bool _is_stuck;
  int _first_stuck_iteration;
  int _best_primal_iteration;
  SubgradRate & _rate;
  bool _debug;
  int _max_round;
  double _last_alpha;
};

#endif
