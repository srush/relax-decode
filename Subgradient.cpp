#include "Subgradient.h"
#include "math.h"
void Subgradient::solve() {
  while(run_one_round()) {}
}


bool Subgradient::run_one_round() {
  double primal, dual; 
  wvector & subgrad = _s->solve(primal, dual);
  
  if (primal > _best_primal) {
    _best_primal = primal;
  }

  if (dual < _best_dual) {
    _best_dual = dual;
  } 

  _duals.push_back(dual);
  _primals.push_back(primal);

  if (subgrad.normsquared() > 0.0) {
    update_weights(subgrad);
    return true;
  } else {
    return false;
  }

  
}

void Subgradient::update_weights(wvector & subgrad) {
  int dualsize = _duals.size();
  int size = subgrad.size();
  if  (dualsize > 2 && _duals[dualsize -1] < _duals[dualsize -2]) { 
    _nround += 1;
  } else if ( dualsize == 1) {
    _base_weight = (_primals[-1] - _duals[-1]) / max((double)size,1.0);
  }

  double alpha = _base_weight * pow(0.99, 10*(float)_nround);
  
  svector<int, double> updates = alpha * subgrad;
  
  _weights += updates;
  
  _s->update_weights(updates, _weights);
}
