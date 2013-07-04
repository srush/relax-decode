
#include "Subgradient.h"
#include "math.h"
#include <iostream>
#include <iomanip>
#include <time.h>
#include "../common.h"
#define INF 1e8
#define TIMING 0
using namespace std;

void print_vec(const wvector & subgrad) {
  for (wvector::const_iterator it = subgrad.begin(); it != subgrad.end(); it++) {
    if (it->second != 0.0) {
      cerr << it->first << " " << it->second << endl;
    }
  }
  cerr << endl << endl;
}

bool Subgradient::solve(int example) {
  bool optimal = false;
  clock_t start = clock();
  while(run_one_round(&optimal) && _round < _max_round && clock() - start < 100000000) {
    _round++;
  }
  clock_t end = clock();
  return optimal;
}

bool Subgradient::run_one_round(bool *optimal) {
  (*optimal) = false;
  clock_t start=clock();
  // bool bump =false;
  // bool no_update = false;
  SubgradResult result;
  SubgradState info;
  info.round = _round;
  info.is_stuck = _is_stuck;
  info.best_primal = _best_primal;
  info.best_dual = _best_dual;
  _s.solve(info, result); //  primal, dual, subgrad, _round, _is_stuck, bump,no_update);

  clock_t end;
  if (TIMING) {
    end=clock();
    cout << "JUST UPDATE "<< double(Clock::diffclock(end,start)) << endl;
  }


  if (result.primal < _best_primal) {
    _best_primal_iteration = _round;
    _best_primal = result.primal;
  }

  if (result.dual > _best_dual) {
    _best_dual = result.dual;
  }

  _duals.push_back(result.dual);
  _primals.push_back(result.primal);

  if (_debug) {
    cerr << "Round " << _round;
    cerr << " BEST_PRIMAL " << _best_primal;
    cerr << " BEST_DUAL " << _best_dual;
    cerr << " CUR_DUAL " << result.dual;
    cerr << " CUR_PRIMAL " << result.primal;
    cerr << " ALPHA " << _last_alpha;
    cerr << " GAP " << fabs(_best_primal - _best_dual) << endl;
    //print_vec(result.subgrad);
  }


  if (result.subgrad.normsquared() > 0.0) {
    if (fabs(result.dual - result.primal) < 1e-4) {
      *optimal = true;
      cerr << "Found best" << endl;
      return false;
    }
    update_weights(result.subgrad, result.bump_rate);
    return true;
  } else {
    if (fabs(result.dual - result.primal) > 1e-4) {
      cerr << "FAILURE" << endl;
      exit(1);
    } else {
      cerr << "Found best" << endl;
      *optimal = true;
    }
    return false;
  }


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
    //rate->_base_weight *=10.0;
    _rate.bump();
  }

  double alpha = _rate.get_alpha(_duals, _primals, size, _aggressive, _is_stuck);
  _last_alpha = alpha;
  svector<int, double> updates = alpha * subgrad;
  if (_debug) {
    cerr << "DUAL " << _duals[dualsize -1]<<" " << _duals[dualsize -2] <<  endl;
    cerr << "PRIMAL " << _primals[dualsize -1]<<" " << _primals[dualsize -2] <<  endl;
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
   }

  _weights += updates;

  _s.update_weights(updates, &_weights);
}



