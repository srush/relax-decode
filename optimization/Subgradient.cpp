
#include "Subgradient.h"
#include "math.h"
#include <iostream>
#include <iomanip>
#include <time.h>
#include "../common.h"
#define INF 1e8
#define TIMING 0
using namespace std;

void Subgradient::solve(int example) {
  clock_t start=clock();
  clock_t s=clock();
  while(run_one_round() && _round < 200) {
    _round++;
    if (TIMING) {
      cout << "ITER TIME "<< double(Clock::diffclock(clock(),s)) << endl;
      s=clock();
    }
    //clock_t e=clock();
    //cout << endl << "*ITER* " << example << " " << _round << " " <<  _best_primal << " " << _best_dual << " " << _base_weight << " " << _first_stuck_iteration << " " << _best_primal_iteration << " " 
    //  << double(diffclock(e,start)) << endl ;
  }
  //if (TIMING) {
    clock_t end=clock();
 

    //}
 if (_round < 400 && ( _best_primal - _best_dual) > 1e-3) {
   cout << "optimization is ambiguous" << endl;
   //exit(0);
    //assert (_best_primal == _best_dual);
    //cout << _best_primal << endl;
    //cout << "CONVERGED" << endl;
  }

 cout << "*END*  "<< example << " " << _best_primal << " " << _best_dual << " " << _first_stuck_iteration << " " << _best_primal_iteration << " " << _round << " " 
      << double(Clock::diffclock(end,start)) << endl ;
}


bool Subgradient::run_one_round() {
  double primal, dual; 
  wvector subgrad;
  //cout << endl;
  clock_t start=clock();
  bool bump =false; 
  _s.solve(primal, dual, subgrad, _round, _is_stuck, bump);

  clock_t end;
  if (TIMING) {
      end=clock();
      cout << "JUST UPDATE "<< double(Clock::diffclock(end,start)) << endl;
  }

  
  if (primal < _best_primal) {
    _best_primal_iteration = _round; 
    _best_primal = primal;
  }

  if (dual > _best_dual) {
    _best_dual = dual;
  } 

  _duals.push_back(dual);
  _primals.push_back(primal);
  if (TIMING) {
  
    cout << "BEST PRIMAL" << _best_primal << endl;
    cout << "BEST DUAL" << _best_dual << endl;
    cout << "Round " << _round << endl; 
  }
  //assert (_best_primal >= _best_dual);

  if (subgrad.normsquared() > 0.0) {
    update_weights(subgrad, bump);
    //cout << endl;
    return true;
  } else {
    return false;
  }

  
}

void print_vec(const wvector & subgrad) {
  for (wvector::const_iterator it = subgrad.begin(); it != subgrad.end(); it++) {
    if (it->second != 0.0) {
      cout << it->first << " " << it->second << endl;
    }
  }
  cout << endl << endl;
}


void Subgradient::update_weights(wvector & subgrad, bool bump) {
  int dualsize = _duals.size();
  int size = 0;
  for (wvector::const_iterator it = subgrad.begin(); it != subgrad.end(); it++) {
    if (it->second != 0.0) {
      size++;
      //cout << it->first << " " << it->second << endl;
    }
  }

  if (bump) {
    _aggressive = true;
    //_base_weight *=0.1;
  }

  if  (dualsize > 2 && _duals[dualsize -1] < _duals[dualsize -2]) { 
    _nround += 1;
    if (_aggressive && _nround > 2) {
      _base_weight *= 0.7;
      _nround = 0;
    } else if (!_is_stuck && _nround == 10) {
      _base_weight *= 0.7;
      _nround =0;
    }
  } else if ( dualsize == 1) {
    _base_weight = (_primals[_primals.size()-1] - _duals[_duals.size()-1]) / max((double)size,1.0);  
  }

  //double alpha = _base_weight * pow(0.99, (double)_nround);
   
  double alpha = _base_weight;
  //double alpha = _base_weight / ((float)_nround / 10.0);
  svector<int, double> updates = alpha * subgrad;
  if (TIMING) {
    cout << "DUAL " << _duals[dualsize -1]<<" " << _duals[dualsize -2] <<  endl;
  }

  // has a dual value become stuck
  _is_stuck = false;
  if (_round > 5) {
    double upper=-INF;
    double lower=INF;
    for (int i=1; i <= 5 ; i++)  {
      upper = max(upper, _duals[dualsize -i]);
      lower = min(lower, _duals[dualsize -i]);
    }

    _is_stuck = fabs(upper-lower) < 0.20;
    if (_is_stuck && _first_stuck_iteration == -1) {
      _first_stuck_iteration = _round;
    }
    //cout << "STUCK " <<  _round << " "<< upper << " " << lower << " " <<_is_stuck <<endl;
  }

  _weights += updates;
  if (TIMING) {
    cout << "CHANGED " << size << endl ;
    cout << "UPDATES" << " " << "Alpha " <<alpha << endl ;
    cout << endl;
  }
  //print_vec(updates) ;
  //print_vec(_weights) ;


  clock_t start=clock();
  _s.update_weights(updates, &_weights);
  if (TIMING) {
    clock_t end=clock();
    cout << "JUST UPDATE "<< double(Clock::diffclock(end,start)) << endl;
  }
}
