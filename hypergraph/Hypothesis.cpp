#include "Hypothesis.h"
#include <iostream>
using namespace std;


int make_id(const vector <int> & hook, const vector <int> & right_side, int dim) {
  return hook[0] + (dim) * right_side[0] + (dim * dim)*right_side[1] + (dim*dim*dim)* hook[1];
}

void show_hyp(const Hypothesis & hyp) {
  cout << hyp.right_side[0] << " " << hyp.right_side[1] << " "<< hyp.hook[0]<< " "  << hyp.hook[
                                                                                                1] << endl;

}

