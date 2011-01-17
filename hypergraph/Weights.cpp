#include "Weights.h"

wvector * load_weights_from_file(const char * file)  {
  fstream input(file, ios::in );
  char buf[1000];
  input.getline(buf, 100000);
  string s(buf);
  return svector_from_str<int, double>(s);
}
