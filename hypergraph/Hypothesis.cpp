#include "Hypothesis.h"
#include <iostream>
using namespace std;

namespace Scarab{
  namespace HG{

    ostream& operator<<(ostream& os, const State& s){ 
      os << s._state[0] << " " << s._state[1] << endl;
      return os;
    }


    ostream& operator<<(ostream& os, const Hypothesis& h) {
      os << h.right_side << " "<< h.hook << endl;
      return os;
    }
}}
