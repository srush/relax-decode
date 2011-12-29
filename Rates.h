#ifndef RATES_H
#define RATES_H

#include "DualDecomposition.h"

class ConstantRate: public SubgradRate {
 public:
  double _constant ;
 ConstantRate(double constant):_constant(constant)  {}
  double get_alpha(vector <double> & duals,
                   vector <double> & primals,
                   int size, bool aggressive, bool is_stuck) {
    return _constant;
  }
  void bump() {}
};

class FallingRate: public SubgradRate {
 public:
  int _round ;
  double _constant;
 FallingRate():_round(0), _constant(1.0)  {}
 FallingRate(double constant):_round(0), _constant(constant)  {}
  double get_alpha(vector <double> & duals,
                   vector <double> & primals,
                   int size, bool aggressive, bool is_stuck) {
    _round++;

    return _constant/_round;
  }
  void bump() {}
  
};


class ParseRate: public SubgradRate {
 public:
  int _nround;
  double _base_weight;
  int _round ;
 ParseRate():_nround(0),_round(0)  {}
  double get_alpha(vector <double> & duals,
                   vector <double> & primals,
                   int size, bool aggressive, bool is_stuck) {
    int dualsize = duals.size();
    _round++;
    if  (dualsize > 2 && duals[dualsize -1] <= duals[dualsize -2]) { 
      _nround += 1;
      if ( _nround >= 2) {
        //else if (_nround >= 3) {// 10) {
        _base_weight *= 0.7;
        _nround =0;
      } 
    } else if ( dualsize == 1) {
      _base_weight = 2.0;
      _nround =0;
    }
    return _base_weight;
  }
  void bump() {
  }
  
};


class TranslationRate: public SubgradRate {
 public:
  int _nround;
  double _base_weight;
  bool post_bump;
 TranslationRate():_nround(0), post_bump(false) {}
  void bump() {
    //_base_weight *=10.0;
    post_bump = true;
  }
  double get_alpha(vector <double> & duals,
                   vector <double> & primals,
                   int size, bool aggressive, bool is_stuck) {
    int dualsize = duals.size();
    double best_dual = -1e8; 
    foreach(double dual, duals) {
      best_dual = max(dual, best_dual);
    }
    if  (dualsize > 2 &&  (duals[dualsize-2] - duals[dualsize -1] > -1e-4)) { 
      _nround += 1;
      /* if (post_bump) { */
      /*   _base_weight *= 0.7; */
      /* } else */
      if (aggressive && _nround > 2) {
        _base_weight *= 0.7;
        _nround = 0;
      } else if (_nround >= 10) {
        //else if (_nround >= 3) {// 10) {
        _base_weight *= 0.7;
        _nround =0;
      }
    } else if ( dualsize == 1) {
      _base_weight = (primals[primals.size()-1] - duals[duals.size()-1]) / max((double)size,1.0);  
    }
    return _base_weight;
  }
  
  
};


#endif
