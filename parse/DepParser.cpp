#include "DepParser.h"


ostream& operator<<(ostream& output, const Dependency& h) {
  output << h.head << "|" << h.mod << ":0";
  return output;
} 
