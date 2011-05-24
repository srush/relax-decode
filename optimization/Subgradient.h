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

struct SubgradState {
  int round;
  bool is_stuck;
  bool no_update;
};

struct SubgradResult {
  double primal;
  double dual;
  wvector subgrad;
  bool bump_rate;
};


class SubgradientProducer {
 public:
  virtual void solve(const SubgradState & cur_state, SubgradResult & result) = 0;
  virtual void update_weights(const wvector & updates, wvector * weights )=0;
};

class Subgradient {

 public:

  /** 
   * 
   * 
   * @param subgrad_producer Gives the subgradient at the current position 
   */
 Subgradient(SubgradientProducer & subgrad_producer, SubgradRate & update_rate ): 
  _s(subgrad_producer), 
  _rate(update_rate){
    _best_dual = -1e20;
    _best_primal = 1e20;
    _round = 1;
    _nround = 1;
    _is_stuck = false;
    _first_stuck_iteration = -1;
    _aggressive = false;
    //rate ;  = new TranslationRate();
  } ;



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
};

#endif
