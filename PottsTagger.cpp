
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
  vector <TagLP * > lp_vars;
  vector <MRFLP *> mrf_lps; 

  TagMrfAligner tag_align;
  tag_align.build_from_constraints(argv[8]);

  // TODO

  for (int i=atoi(argv[6]); i <= atoi(argv[7]); i++) {  
    stringstream fname;
    fname << argv[3] << i;
    MRF * mrf =new MRF();  
    cout << fname.str() <<endl;
    clock_t s=clock();
    mrf->build_from_file(fname.str().c_str());
    stringstream prefix;
    prefix << "cgroup_"<< i;
    LPConfig * lp_conf = new LPConfig(prefix.str(), model, GRB_CONTINUOUS);
    MRFLP * mrf_lp = new MRFLP(*mrf);
    mrf_lp->set_lp_conf(lp_conf);
    mrf_lps.push_back(mrf_lp);    
  }
  
  // Optimization trick, load in all the lp's first and then add constraints in waves
  foreach (MRFLP* mrf_lp, mrf_lps) {
    mrf_lp->add_vars();
  }

  cout << "Update 1" << endl;
  model.update();
  cout << "Update 1 Finished" << endl;
  cout << "Update 2" << endl;

  foreach (MRFLP* mrf_lp, mrf_lps) {
    mrf_lp->add_constraints();
  }
  model.update();
  
  cout << "Update 2 finished" << endl;
  

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
    
    TagLP * lp_parse = new TagLP(*f, *edge_weights);

    stringstream prefix;
    prefix << "parse" << i;
    LPConfig * lp_conf = new LPConfig(prefix.str(), model, GRB_CONTINUOUS);
    
    lp_parse->set_lp_conf(lp_conf);
    lp_vars.push_back(lp_parse);
  }

  foreach(TagLP * lp_var, lp_vars) {
    lp_var->add_vars();
  }
  model.update();

  foreach(TagLP * lp_var, lp_vars) {
    lp_var->add_constraints();
  }
  model.update();
  

  cout << "SCORE" << total <<endl;
  
  vector <const MRFLP* > const_mrf_lps;
  vector <const TagLP* > const_tag_lps;
  for (int i =0; i < mrf_lps.size(); i++) {
    const_mrf_lps.push_back(mrf_lps[i]);
  }

  for (int i =0; i < lp_vars.size(); i++) {
    const_tag_lps.push_back(lp_vars[i]);
  }


  TagMrfLP::align_tag_mrf(const_mrf_lps, const_tag_lps, tag_align, model, GRB_CONTINUOUS);

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
    lp_vars[i] ->show();
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
