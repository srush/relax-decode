#include "Weights.h"
#include "Forest.h"
#include <HypergraphAlgorithms.h>

#include <iostream>
#include <iomanip>

#include "common.h"
#include <boost/program_options.hpp>
#include "lexical.pb.h"
#include "HypergraphLP.h"
#include "HardConstraints.h"
#include "DepParseLP.h"

using namespace std;
using namespace Scarab::HG;
namespace po = boost::program_options;



int main(int argc, char ** argv) {
  
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  // Viterbi
  wvector * weight = load_weights_from_file( argv[1]); //vm["weights"].as< string >().c_str());
  
  GRBEnv env = GRBEnv();
  GRBModel model(env);

  HardConstraints hard_cons;
  hard_cons.read_from_file(argv[5]);
  vector <DepParserLP * > lp_vars;
  
  for (int i=atoi(argv[3]); i <= atoi(argv[4]); i++) {  
    stringstream fname;
    fname << argv[2] << i;
  
    DepParser * f = new DepParser();
    f->build_from_file(fname.str().c_str());
  
    HypergraphAlgorithms ha(*f);
    EdgeCache * edge_weights = ha.cache_edge_weights(*weight);
   
    stringstream prefix;
    prefix << "parse" << i;
    DepParserLP * lp_parse = DepParserLPBuilder::add_parse(*f, *edge_weights, prefix.str(), 
                                                           model, GRB_CONTINUOUS);
    lp_vars.push_back(lp_parse);
  }

  hard_cons.add_to_lp(lp_vars, model);

  model.write("/tmp/model.lp");
  model.set(GRB_IntAttr_ModelSense, 1);
  model.optimize();

  for (int i=0; i< lp_vars.size(); i++) {
    cout << "PARSE " << i << endl;
    DepParserLPBuilder::show_results(*lp_vars[i]);
  }
  hard_cons.show_results();  

  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
