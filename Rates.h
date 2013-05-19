#ifndef RATES_H
#define RATES_H

#include "DualDecomposition.h"

class ConstantRate: public SubgradRate {
 public:
  double _constant ;
 ConstantRate(double constant):_constant(constant)  {}
  double get_alpha(vector <double> & duals,
                   vector <double> & primals,
                   const wvector &subgrad,
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
                   const wvector &subgrad,
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
                   const wvector &subgrad,
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
                   const wvector &subgrad,
                   int size, bool aggressive, bool is_stuck) {
    int dualsize = duals.size();
    double best_dual = -1e8;
    foreach(double dual, duals) {
      best_dual = max(dual, best_dual);
    }
    if (dualsize > 2 &&  (duals[dualsize-2] - duals[dualsize -1] > -1e-4)) {
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
    cerr << _base_weight << endl;
    return _base_weight;
  }
};


class PolyakRate: public SubgradRate {
 public:
  int _nround;
  double _base_weight;
  bool post_bump;
  double opt_;

PolyakRate(double opt):_nround(0), post_bump(false), opt_(opt) {}
  double get_alpha(vector<double> &duals,
                   vector<double> &primals,
                   const wvector &subgrad,
                   int size,
                   bool aggressive,
                   bool is_stuck) {
    _nround++;
    double best_primal = opt_;
    foreach (double primal, primals) {
      best_primal = min(primal, best_primal);
    }


    cerr << "getting" << endl;
    if (duals.size() == 0 && primals.size() == 0) {
      return 1;
    } else {
      cerr << "getting denom" << endl;
      double denom = subgrad.normsquared();
      double num =
          fabs(duals[duals.size() - 1] - best_primal);
      cerr << num / denom << endl;
      return num / denom;
    }
  }
};


class PolyakTranslationRate: public SubgradRate {
 public:
  int _nround;
  double _base_weight;
  bool post_bump;
  double opt_;

  PolyakTranslationRate(double opt):_nround(0), post_bump(false), opt_(opt) {}
  void bump() {
    post_bump = true;
  }

  double get_alpha(vector <double> & duals,
                   vector <double> & primals,
                   const wvector &subgrad,
                   int size, bool aggressive, bool is_stuck) {
    int dualsize = duals.size();
    double best_dual = -1e8;
    foreach(double dual, duals) {
      best_dual = max(dual, best_dual);
    }
    double best_primal = opt_;
    foreach (double primal, primals) {
      best_primal = min(primal, best_primal);
    }

    if (dualsize >= 40 && dualsize > 2 &&  (duals[dualsize-2] - duals[dualsize -1] > -1e-4)) {
      _nround += 1;
      if (aggressive && _nround > 2) {
        _base_weight *= 0.7;
        _nround = 0;
      } else if (_nround >= 5) {
        _base_weight *= 0.7;
        _nround = 0;
      }
    } else if (dualsize > 1  && dualsize < 40) {
      double denom = subgrad.normsquared();
      double num = fabs(duals[duals.size() - 1] - best_primal);
      cerr << num / denom << endl;
      _base_weight = num / denom;
    }
    cerr << _base_weight << endl;
    return _base_weight;
  }
};

#endif
