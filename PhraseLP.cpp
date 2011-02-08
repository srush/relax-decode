#include "Weights.h"
#include "PhraseBased.h"
#include <HypergraphAlgorithms.h>

#include <iostream>
#include <iomanip>
#include <algorithm>

#include "HypergraphLP.h"
#include "common.h"

using namespace std;
using namespace Scarab::HG;

int main(int argc, char ** argv) {
  
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  
  wvector * weights = load_weights_from_file( argv[1]); //vm["weights"].as< string >().c_str());
  double total_score = 0.0;

  for (int i=atoi(argv[3]); i <= atoi(argv[4]); i++) {  

    stringstream fname;
    fname << argv[2] << i;
    
    // Load in the hypergraph for phrase based 
    PhraseBased f;
    f.build_from_file(fname.str().c_str());
    
    cout << "Read" << endl;
    
    HypergraphAlgorithms ha(f);
    EdgeCache * edge_weights = ha.cache_edge_weights(*weights);
    
    GRBEnv env = GRBEnv();
    GRBModel model(env);

    HypergraphLP * lp_trans = HypergraphLPBuilder::add_hypergraph(f, *edge_weights, "" , model, 
                                                                  GRB_CONTINUOUS);

    model.write("/tmp/model.lp");
    model.set(GRB_IntAttr_ModelSense, 1);
    model.optimize();

    HypergraphLPBuilder::show_hypergraph(*lp_trans);
    
    cout << endl;
    //cout << "Score is : " << -score << endl;

    //    total_score += score;
  }

  //cout << "Total Score " << -total_score << endl;
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
