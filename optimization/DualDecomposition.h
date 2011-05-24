/** 
 * Dual Decomposition abstractions implemented on 
 * top of subgradient solver
 * 
 * 
 */

#ifndef DUALDECOMPOSITION_H
#define DUALDECOMPOSITION_H
#include <iostream>
#include "Subgradient.h"

using namespace std;


/** 
 * Virtual client class for dual decomposition subproblem
 * (each subproblem in a decomposition should be an instance  
 * of this class)
 * 
 */
class DualDecompositionSubproblem {
 public:
  
  /** 
   * Solve the current dual subproblem 
   * 
   * @param SubgradInfo The current state of the algorithm
   * @param SubgradResult Output of the subproblem
   */
  virtual void solve(const SubgradState & info,
                     SubgradResult & result) = 0;
                      

  /** 
   * Update the weight of the subproblem 
   * 
   * @param updates The delta change in the weights
   * @param weights The full set of updated weights 
   * @param mult (Hack) projection for two problems
   */
  virtual void update_weights(const wvector & updates,  
                              wvector * weights, 
                              double mult)=0;

};



/** 
 * Internal wrapper class to run two subproblem dual decomposition
 * as a subgradient optimization problem. 
 * 
 */

class DualDecompositionRunner : public SubgradientProducer{
 public:


 DualDecompositionRunner(DualDecompositionSubproblem & s1, DualDecompositionSubproblem & s2 ):
  _sub_producer1(s1), _sub_producer2(s2) {}

  
  void solve(const SubgradState & info, SubgradResult & result);
  
  void update_weights(const wvector & updates,  
                      wvector * weights);
  
  DualDecompositionSubproblem & _sub_producer1;
  DualDecompositionSubproblem & _sub_producer2;
};


/** 
 * Dual Decomposition Manager. 
 * Call with two subproblems runs dual decomposition
 * 
 */

class DualDecomposition {
 public:

  DualDecomposition(DualDecompositionSubproblem & s1, 
                    DualDecompositionSubproblem & s2,
                    SubgradRate & rate
                    ): 
  _runner(s1, s2), 
    _subgradsolver(_runner, rate){}

  /** 
   * Solve the subgradient problem
   * 
   * @param example For logging 
   */  
  void solve(int example) {
    _subgradsolver.solve(example);
  }

  Subgradient _subgradsolver;

 private:  
  DualDecompositionRunner _runner;
};




#endif
