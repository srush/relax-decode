#include "Weights.h"
#include "Forest.h"
#include <HypergraphAlgorithms.h>

#include <iostream>
#include <iomanip>

#include "common.h"
#include <boost/program_options.hpp>
//#include "lexical.pb.h"
#include "HypergraphLP.h"
//#include "HardTagConstraints.h"
#include "TagConstraints.h"
#include "TagLP.h"
#include "Tagger.h"
#include "HardPosConstraints.h"

using namespace std;
using namespace Scarab::HG;
namespace po = boost::program_options;



int main(int argc, char ** argv) {
  
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  // Viterbi
  wvector * weight = load_weights_from_file( argv[1]); //vm["weights"].as< string >().c_str());
  double penalty = atof(argv[6]);
  
  GRBEnv env = GRBEnv();
  GRBModel model(env);

  try{
  //HardConstraints hard_cons;
  //hard_cons.read_from_file(argv[5]);
  vector <TagLP * > lp_vars;

  TagConstraints tag_cons(44);
  tag_cons.read_from_file(argv[5]);

  double total =0.0;
  for (int i=atoi(argv[3]); i <= atoi(argv[4]); i++) {  
    stringstream fname;
    fname << argv[2] << i;
  
    Tagger * f = new Tagger(100);
    cout << fname.str() << endl;
    f->build_from_file(fname.str().c_str());
  
    HypergraphAlgorithms ha(*f);
    EdgeCache * edge_weights = ha.cache_edge_weights(*weight);
  
    NodeCache  score_memo_table(f->num_nodes()); 
    
    NodeBackCache  back_memo_table(f->num_nodes());
    
    double score = ha.best_path( *edge_weights, score_memo_table, back_memo_table);


    total += score;
    stringstream prefix;
    prefix << "parse" << i;
    TagLP * lp_parse = TagLPBuilder::add_tagging(*f, *edge_weights, prefix.str(), 
                                                       model, GRB_CONTINUOUS);
    lp_vars.push_back(lp_parse);
  }
  cout << "SCORE" << total <<endl;
  
  HardPosConstraintsLP hard_cons(tag_cons, penalty);
  hard_cons.add_to_lp(lp_vars, model);

  model.write("/tmp/model.lp");
  model.set(GRB_IntAttr_ModelSense, 1);
  model.optimize();

  for (int i=0; i< lp_vars.size(); i++) {
    cout << "PARSE: " << i << endl;
    cout << "SENT: ";
    TagLPBuilder::show_results(*lp_vars[i]);
  }
  hard_cons.show_results();  
  }

  catch (GRBException e) {
    cerr << "Error code = " << e.getErrorCode() << endl;
    cerr << e.getMessage() << endl;
    
  }

  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
