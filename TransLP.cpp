#include "Weights.h"
#include "Forest.h"
#include "ForestLattice.h"
#include "NGramCache.h"
#include "LPBuilder.h"
#include "HypergraphAlgorithms.h"
#include <iostream>
#include <iomanip>
#include "CommandLine.h"
using namespace std;

using namespace Scarab::HG;

DEFINE_string(forest_prefix, "", "prefix of the forest files"); // was 1
DEFINE_string(lattice_prefix, "", "prefix of the lattice files"); // was 2
DEFINE_string(forest_range, "", "range of forests to use (i.e. '0 10')"); // was 5 6
DEFINE_bool(continuous, true, "use continuous variables in the LP"); 

static const bool forest_dummy = RegisterFlagValidator(&FLAGS_forest_prefix, &ValidateReq);
static const bool lattice_dummy = RegisterFlagValidator(&FLAGS_lattice_prefix, &ValidateReq);
static const bool range_dummy = RegisterFlagValidator(&FLAGS_forest_range, &ValidateRange);

int main(int argc, char ** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  
  google::ParseCommandLineFlags(&argc, &argv, true);
  

  wvector * weight = cmd_weights();
  Ngram * lm = cmd_lm();

  istringstream range(FLAGS_forest_range);
  int start_range, end_range;
  range >> start_range >> end_range;
  for (int i = start_range; i <= end_range; i++) {     

    stringstream fname;
    fname << FLAGS_forest_prefix << i;
    Forest f = Forest::from_file(fname.str().c_str());


    // Load lattice
    stringstream fname2;
    fname2 << FLAGS_lattice_prefix << i;
    ForestLattice graph = ForestLattice::from_file(fname2.str());
      
    int var_type;
    if (FLAGS_continuous == 1) {
      var_type = GRB_CONTINUOUS;
    } else {
      var_type = GRB_BINARY;
    }
    LPBuilder lp(f, graph, var_type);
      

    const Cache <Graphnode, int> * word_cache = sync_lattice_lm(graph, *lm); 
    
    HypergraphAlgorithms alg(f); 
    EdgeCache * w = alg.cache_edge_weights( *weight);
    
    try {
      lp.solve_full(i, *w,  *lm, lm_weight(), *word_cache);
    } 
    catch (GRBException e) {
      cerr << "Error code = " << e.getErrorCode() << endl;
      cerr << e.getMessage() << endl;
      cout << "*END* " << i<< " "<<0 << " " << 200 << " "<<  0 << " " << 0 << endl;
    }
    

    
    NodeBackCache bcache(f.num_nodes());     
    
    NodeCache ncache(f.num_nodes());
    //double best = best_path(f, *w, ncache, bcache);
    //cout << best << endl;
  }  
  return 1;
}
