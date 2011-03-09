#ifndef DUALDECOMPOSITION_H
#define DUALDECOMPOSITION_H
#include <iostream>
#include "Subgradient.h"

using namespace std;
class DualDecompositionSubproblem {
 public:
  virtual void solve(double & primal, double & dual, wvector &, 
                      int) =0;
  virtual void update_weights(const wvector & updates,  
                              wvector * weights, 
                              double mult)=0;


};

class DualDecompositionRunner : public SubgradientProducer{
 public:
  bool _is_first;
 DualDecompositionRunner(DualDecompositionSubproblem & s1, DualDecompositionSubproblem & s2 ):
  _sub_producer1(s1), _sub_producer2(s2) {
    _is_first = true;
 }

  void  solve(double & primal, double & dual, wvector & subgrad, 
              int round, bool is_stuck, bool & bump_rate) {
    
    double primal1, primal2, dual1, dual2;
    wvector subgrad1, subgrad2;
    bool bump1, bump2;
    
    _sub_producer1.solve(primal1, dual1, subgrad1, round);

    if (_is_first) {
      cout << "Bump" << endl;
      wvector wv = 0.00001 * subgrad1;
      _sub_producer1.update_weights(wv, &wv, 1.0);
      _sub_producer2.update_weights(wv, &wv, -1.0);
      _is_first = false;
      subgrad = wv;
      dual = 0.0;
      primal =0.0;
      return;
    }

    _sub_producer2.solve(primal2, dual2, subgrad2, round);
    
    subgrad = subgrad1 - subgrad2;
    int count = 0;
    for (wvector::const_iterator it = subgrad.begin(); it != subgrad.end(); it++) {
      if (it->second != 0.0) {
        count++;
        cout << "SUBGRAD: " << it->first << " " << it->second << " " << it->first / 45<< endl;
      }
    }
    cout << "SIZE is : "  << count;

    dual = dual1 + dual2;
    primal = primal1 + primal2;
    cout << "DECOMP PRIMALS : "  << primal1 << " " << primal2 << endl;
    cout << "DECOMP DUALS : "  << dual1 << " " << dual2 << endl;
    bump_rate = false;
    
  }

  void update_weights(const wvector & updates,  
                      wvector * weights) {

    _sub_producer1.update_weights(updates, weights, 1.0);
    _sub_producer2.update_weights(updates, weights, -1.0);
  }
  
  DualDecompositionSubproblem & _sub_producer1;
  DualDecompositionSubproblem & _sub_producer2;

};

class DualDecomposition {
 public:

  DualDecomposition(DualDecompositionSubproblem & s1, 
                    DualDecompositionSubproblem & s2): _runner(s1, s2), _subgradsolver(_runner){
    
  }

  void solve(int example) {
    _subgradsolver.solve(example);

  }
  Subgradient _subgradsolver;
 private:
  
  DualDecompositionRunner _runner;



};

#endif
