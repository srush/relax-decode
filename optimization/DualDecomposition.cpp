#include "DualDecomposition.h"


void DualDecompositionRunner::solve(const SubgradState & info, SubgradResult & result) { 
  SubgradResult result1, result2;
  _sub_producer1.solve(info, result1);
  _sub_producer2.solve(info, result2);
  
  result.subgrad = result1.subgrad - result2.subgrad;
  result.dual = result1.dual + result2.dual;
  result.primal = result1.primal + result2.primal;
}

void DualDecompositionRunner::update_weights(const wvector & updates,  
                    wvector * weights) {

_sub_producer1.update_weights(updates, weights, 1.0);
_sub_producer2.update_weights(updates, weights, -1.0);
}


    //int count = 0;
    //for (wvector::const_iterator it = subgrad.begin(); it != subgrad.end(); it++) {
    //  if (it->second != 0.0) {
    //    count++;
    //  }
    //}
    //cout << "SIZE is : "  << count;
    //cout << "DECOMP PRIMALS : "  << primal1 << " " << primal2 << endl;
    //cout << "DECOMP DUALS : "  << dual1 << " " << dual2 << endl;

    /* if (_is_first) { */
    /*   wvector wv = 0.00001 * subgrad1; */
    /*   _sub_producer1.update_weights(wv, &wv, 1.0); */
    /*   _sub_producer2.update_weights(wv, &wv, -1.0); */
    /*   _is_first = false; */
    /*   subgrad = wv; */
    /*   dual = 0.0; */
    /*   primal =0.0; */
    /*   return; */
    /* } */
