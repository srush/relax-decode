#include <iostream>
#include <iomanip>
#include "common.h"
#include "Rates.h"
#include "DualDecomposition.h"
using namespace std;

/**
 * This is a very simple example problem. 
 * There will be two lists A and B of length n. 
 * We need to choose on/off for each position. On means 
 * we keep the value in both lists.  
 *
 * The dual decomposition is to have a subsolver for each list.
 */
 
class ListSolver : public DualDecompositionSubproblem {
public:
  ListSolver(vector<int> list): _list(list), 
                                _mult(0.0){
    _weights = new wvector();
  }
  
  void solve(const SubgradState & info,
             SubgradResult & results) {
    results.primal = 0.0;
    results.dual = 0.0;
    for (int i=0; i < _list.size(); i++) {
      double score = _list[i] - _mult*(*_weights)[i];
      cerr << score << endl;
      if (score > 0.0) {
        results.subgrad[i] += 1;
        //results.primal += _list[i];
        results.dual -= score;
      } else {
      }
    }
  }

  void update_weights(const wvector & updates,  
                      wvector * weights, 
                      double mult) {
    _weights = weights;
    _mult = mult;
  }

private:
  vector <int> _list;
  wvector *_weights;
  double _mult;
};

int main(int argc, char ** argv) {
  
  vector<int> rand;
  rand.push_back(3);
  rand.push_back(5);
  rand.push_back(-7);
  rand.push_back(3);
  vector<int> rand2;
  rand2.push_back(-5);
  rand2.push_back(-3);
  rand2.push_back(1);
  rand2.push_back(10);
  ListSolver solver1(rand); 
  ListSolver solver2(rand2); 
  FallingRate rate;
  DualDecomposition dd(solver1, solver2, rate);
  dd.subgradsolver.set_debug();  
  dd.solve(1);
}
