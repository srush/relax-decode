#include "Weights.h"
#include "Forest.h"
#include <HypergraphAlgorithms.h>

#include <iostream>
#include <iomanip>

#include "common.h"
#include <boost/program_options.hpp>

#include "TagConstraints.h"
#include "TagSolvers.h"
#include "Tagger.h"

#include "DualDecomposition.h"

using namespace std;
using namespace Scarab::HG;


int main(int argc, char ** argv) {
  
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  // Viterbi
  wvector * weight = load_weights_from_file( argv[1]); //vm["weights"].as< string >().c_str());
  
  
  
  vector <const Tagger * > taggers;

  TagConstraints tag_cons(44);
  tag_cons.read_from_file(argv[5]);

  double total =0.0;
  for (int i=atoi(argv[3]); i <= atoi(argv[4]); i++) {  
    stringstream fname;
    fname << argv[2] << i;
  
    Tagger * f = new Tagger(100);
    cout << fname.str() << endl;
    f->build_from_file(fname.str().c_str());
    taggers.push_back(f);
  }

  TaggerDual tagger(taggers, *weight, tag_cons);
  ConstrainerDual constrainer(tag_cons);

  DualDecomposition d(tagger, constrainer);
  d.solve(0);


  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
