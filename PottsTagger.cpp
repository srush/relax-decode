
#include "Weights.h"
#include "Forest.h"
#include <HypergraphAlgorithms.h>

#include <iostream>
#include <iomanip>

#include "common.h"
#include <boost/program_options.hpp>
//#include "lexical.pb.h"
#include "HypergraphLP.h"

#include "TagLP.h"
#include "TagMrfLP.h"
#include "Tagger.h"

#include "MRFLP.h"


using namespace std;
using namespace Scarab::HG;


int main(int argc, char ** argv) {
  
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  // Viterbi
  wvector * weight = load_weights_from_file( argv[1]); //vm["weights"].as< string >().c_str());
  double penalty = atof(argv[6]);
  
  GRBEnv env = GRBEnv();
  GRBModel model(env);

  // try{
  //HardConstraints hard_cons;
  //hard_cons.read_from_file(argv[5]);
  vector <const TagLP * > lp_vars;
  vector <const MRFLP *> mrf_lp; 

  TagMrfAligner tag_align;
  tag_align.build_from_constraints(argv[8]);

  // TODO
  for (int i=atoi(argv[6]); i <= atoi(argv[7]); i++) {  
    stringstream fname;
    fname << argv[3] << i;
    MRF * mrf =new MRF();  
    cout << fname.str() <<endl;
    mrf->build_from_file(fname.str().c_str());
    
    stringstream prefix;
    prefix << "cgroup_"<< i;
    mrf_lp.push_back((const MRFLP *)
                     MRFBuilderLP::add_mrf(*mrf, prefix.str(), model, GRB_CONTINUOUS));    
  }

  double total =0.0;
  for (int i=atoi(argv[4]); i <= atoi(argv[5]); i++) {  
    stringstream fname;
    fname << argv[2] << i;
  
    Tagger * f = new Tagger(200);
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
    lp_vars.push_back((const TagLP*)lp_parse);
  }
  cout << "SCORE" << total <<endl;
  
  TagMrfLP::align_tag_mrf(mrf_lp, lp_vars, tag_align, model, GRB_CONTINUOUS);

  model.update();  
  //hard_cons.add_to_lp(lp_vars, model);

  //model.computeIIS();
  //model.write("/tmp/model.ilp");
  model.write("/tmp/model.lp");
  model.set(GRB_IntAttr_ModelSense, 1);
  model.optimize();

  for (int i=0; i< lp_vars.size(); i++) {
    cout << "PARSE: " << i << endl;
    cout << "SENT: ";
    TagLPBuilder::show_results(*lp_vars[i]);
  }
  //hard_cons.show_results();  
  // }

  // catch (GRBException e) {
  //   cerr << "Error code = " << e.getErrorCode() << endl;
  //   cerr << e.getMessage() << endl;
    
  // }

  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
