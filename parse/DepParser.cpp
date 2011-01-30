#include "DepParser.h"


ostream& operator<<(ostream& output, const Dependency& h) {
  output << h.head << "_" << h.mod;// << "("<< h.length << ")";
  return output;
} 
