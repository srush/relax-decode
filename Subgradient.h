#ifndef SUBGRADIENT_H_
#define SUBGRADIENT_H_


#include "svector.hpp"
#include <vector>
using namespace std;

typedef svector<int, double> wvector;

class SubgradientProducer {
 public:
  virtual void  solve(double & primal, double & dual, wvector &, int) =0;
  virtual void update_weights(const wvector & updates,  wvector * weights )=0;
};

class Subgradient {

 public:
 Subgradient(SubgradientProducer * s): _s(s){
    _best_dual = -1e20;
    _best_primal = 1e20;
    _round = 1;
    _nround = 1;
  } ;

  void solve();
  bool run_one_round();  

 private:
  SubgradientProducer * _s;

  double _best_dual, _best_primal;
  

  void update_weights(wvector & subgrad);
  int _round, _nround;
  wvector _weights;
  vector <double> _primals;
  vector <double> _duals;
  double _base_weight;
};

#endif
