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
#include <iomanip>
#include "Subgradient.h"
using namespace std;

int main(int argc, char ** argv) {

  cout << argc << endl;

  svector<int, double> * weight;

  {
    // Read the existing address book.
    fstream input(argv[3], ios::in );
    char buf[1000];
    input.getline(buf, 100000);
    string s (buf);
    weight = svector_from_str<int, double>(s);
  }

  
  Vocab * all = new Vocab();
  all->unkIsWord() = true;
  Ngram * lm = new Ngram(*all, 3);

  File file(argv[4], "r", 0);    
  if (!lm->read(file, false)) {
    cerr << "READ FAILURE\n";
  }


  for (int i =1; i <= 10; i++) { 


    GOOGLE_PROTOBUF_VERIFY_VERSION;
  
    Hypergraph hgraph;

    {
      stringstream fname;
      fname <<argv[1] << i;


      fstream input(fname.str().c_str(), ios::in | ios::binary);
      if (!hgraph.ParseFromIstream(&input)) {
        assert (false);
      } 
    }
    
    Forest f (hgraph);

    
    Lattice lat;
  
    {
      stringstream fname;
      fname <<argv[2] << i;

      fstream input(fname.str().c_str(), ios::in | ios::binary);
      if (!lat.ParseFromIstream(&input)) {
        assert (false);
      }
      
    }

    ForestLattice graph (lat);
  
  
    // Optional:  Delete all global objects allocated by libprotobuf.
    //google::protobuf::ShutdownProtobufLibrary();
  
    cout << "START!!!!" << endl;
    Decode * d = new Decode(f, graph, *weight, *lm);
    
    Subgradient * s = new Subgradient(d);
    s->solve();
  }
  return 0;
}
