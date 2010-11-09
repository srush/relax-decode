#include <cy_svector.hpp>
#include <svector.hpp>
#include "Forest.h"
#include "ForestLattice.h"
#include <fstream>
#include <iostream>
#include <Vocab.h>
#include <Ngram.h>
#include <File.h>
#include "Decode.h"
#include "Subgradient.h"
using namespace std;

int main(int argc, char ** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  Hypergraph hgraph;

  {
    // Read the existing address book.
    fstream input(argv[1], ios::in | ios::binary);
    if (!hgraph.ParseFromIstream(&input)) {
      assert (false);
    }
    
  }

  Forest f (hgraph);


  Lattice lat;

  {
    // Read the existing address book.
    fstream input(argv[2], ios::in | ios::binary);
    if (!lat.ParseFromIstream(&input)) {
      assert (false);
    }
    
  }

  ForestLattice graph (lat);
  
  svector<int, double> * weight;

  {
    // Read the existing address book.
    fstream input(argv[3], ios::in );
    string buf;
    input >> buf;
    weight = svector_from_str<int, double>(buf);
  }

  
  Vocab * all = new Vocab();
  all->unkIsWord() = true;
  LM * lm = new Ngram(*all, 3);

  File file(argv[4], "r", 0);    
  if (!lm->read(file, false)) {
    cerr << "READ FAILURE\n";
  }

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();
  
  Decode d(f, graph, *weight, *lm);

  Subgradient s(&d);
  s.solve();
  return 0;
}
