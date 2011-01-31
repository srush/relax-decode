#ifndef SUBGRADIENT_H_
#define SUBGRADIENT_H_


#include "svector.hpp"
#include <vector>
using namespace std;

typedef svector<int, double> wvector;


class SubgradientProducer {
 public:
  virtual void  solve(double & primal, double & dual, wvector &, 
                      int, bool, bool &) =0;
  virtual void update_weights(const wvector & updates,  
                              wvector * weights )=0;
};

class Subgradient {

 public:

  /** 
   * 
   * 
   * @param subgrad_producer Gives the subgradient at the current position 
   */
 Subgradient(SubgradientProducer & subgrad_producer): _s(subgrad_producer){
    _best_dual = -1e20;
    _best_primal = 1e20;
    _round = 1;
    _nround = 1;
    _is_stuck = false;
    _first_stuck_iteration = -1;
    _aggressive = false;
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
};

#endif
