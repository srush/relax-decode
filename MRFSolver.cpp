
#include "Weights.h"
#include "Forest.h"
#include <HypergraphAlgorithms.h>

#include <iostream>
#include <iomanip>

#include "common.h"
#include "HypergraphLP.h"


#include "MRFLP.h"
#include "MRF.h"
#include "MRFHypergraph.h"

using namespace std;
using namespace Scarab::HG;

int main(int argc, char ** argv) {
  
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  // Viterbi
  wvector * weight = load_weights_from_file( argv[1]); //vm["weights"].as< string >().c_str());
  
  GRBEnv env = GRBEnv();
  GRBModel model(env);

  stringstream fname;
  fname << argv[2];
  MRF * mrf =new MRF();  
  cout << fname.str() <<endl;
  mrf->build_from_file(fname.str().c_str());
    
  int i = 1;
  stringstream prefix;
  prefix << "cgroup_"<< i;
  MRFLP * mrf_lp = MRFBuilderLP::add_mrf(*mrf, prefix.str(), model, GRB_CONTINUOUS);    

  MRFHypergraph mrf_hyp = MRFHypergraph::from_mrf(*mrf);

  cout << "NODE " << mrf_hyp.nodes().size() << endl;
  cout << "EDGE " << mrf_hyp.edges().size() << endl;

  mrf_hyp.write_to_file("/tmp/out_mrf");

  HypergraphAlgorithms ha(mrf_hyp);

  EdgeCache * edge_weights = ha.cache_edge_weights(*weight);
  
  NodeCache  score_memo_table(mrf_hyp.num_nodes()); 
  
  NodeBackCache  back_memo_table(mrf_hyp.num_nodes());
  
  double score = ha.best_path( *edge_weights, score_memo_table, back_memo_table);

  cout << "SCORE" << score <<endl;

  model.update();  
  //hard_cons.add_to_lp(lp_vars, model);

  //model.computeIIS();
  //model.write("/tmp/model.ilp");
  model.write("/tmp/model.lp");
  model.set(GRB_IntAttr_ModelSense, 1);
  model.optimize();


  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
