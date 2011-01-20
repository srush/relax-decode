#include "Weights.h"
#include "Forest.h"

#include "ForestLattice.h"
#include "Decode.h"
#include "NGramCache.h"
#include "Subgradient.h"

#include <iostream>
#include <Vocab.h>
#include <File.h>
#include <iomanip>
#include <cstdlib>
using namespace std;

int main(int argc, char ** argv) {
  srand(0);
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  wvector * weight = load_weights_from_file(argv[3]);
  NgramCache * lm = load_ngram_cache(argv[4]);


  for (int i =atoi(argv[5]); i <= atoi(argv[6]); i++) { 

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    stringstream fname;
    fname << argv[1] << i;
    Forest f = Forest::from_file(fname.str().c_str());

    Lattice lat;
  
    {
      stringstream fname;
      fname << argv[2] << i;
      //cout << fname << endl; 
      fstream input(fname.str().c_str(), ios::in | ios::binary);
      if (!lat.ParseFromIstream(&input)) {
        assert (false);
      }
    }

    ForestLattice graph (lat);
  
    //cout << "START!!!!" << endl;
    Decode * d = new Decode(f, graph, *weight, *lm);
    cout << i << " ";
    Subgradient * s = new Subgradient(d);
    s->solve(i);
  }
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
