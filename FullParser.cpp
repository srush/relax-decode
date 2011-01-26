#include "Weights.h"
#include "Forest.h"
#include <HypergraphAlgorithms.h>

#include <iostream>
#include <iomanip>

#include "common.h"
#include <boost/program_options.hpp>
#include "lexical.pb.h"
#include "HypergraphLP.h"

using namespace std;
using namespace Scarab::HG;
namespace po = boost::program_options;



int main(int argc, char ** argv) {
  
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  // Viterbi
  wvector * weight = load_weights_from_file( argv[1]); //vm["weights"].as< string >().c_str());
  
  GRBEnv env = GRBEnv();
  GRBModel model(env);

  for (int i=atoi(argv[3]); i <= atoi(argv[4]); i++) {  
    stringstream fname;
    fname << argv[2] << i;
  
    Forest f = Forest::from_file(fname.str().c_str());
  
    HypergraphAlgorithms ha(f);
    EdgeCache * edge_weights = ha.cache_edge_weights(*weight);
   
    stringstream prefix;
    prefix << "parse" << i;
    DepParseLPBuilter::add_hypergraph(f, *edge_weights, "parse", model, GRB_CONTINUOUS);
  }

  model.write("/tmp/model.lp");
  model.set(GRB_IntAttr_ModelSense, 1);
  model.optimize();
  
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
