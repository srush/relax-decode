#include "DepParser.h"


ostream& operator<<(ostream& output, const Dependency& h) {
  output << h.mod << "_" << h.head;// << "("<< h.length << ")";
  return output;
} 
