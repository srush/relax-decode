#ifndef SUBGRADIENT_H_
#define SUBGRADIENT_H_


#include "svector.hpp"
#include <vector>
using namespace std;

typedef svector<int, double> wvector;

class SubgradRate {
 public:
  virtual double get_alpha(vector <double> & duals,
                           vector <double> & primals,
                           int size, bool aggressive, bool is_stuck) = 0;

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
  
};

class TranslationRate: public SubgradRate {
 public:
  int _nround;
  double _base_weight;
 TranslationRate():_nround(0) {}
  double get_alpha(vector <double> & duals,
                   vector <double> & primals,
                   int size, bool aggressive, bool is_stuck) {
    int dualsize = duals.size();
    if  (dualsize > 2 && duals[dualsize -1] <= duals[dualsize -2]) { 
      _nround += 1;
      if (aggressive && _nround > 2) {
        _base_weight *= 0.7;
        _nround = 0;
      } else if (!is_stuck && _nround >= 10) {
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
    rate = new TranslationRate();
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

  SubgradRate * rate;

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
