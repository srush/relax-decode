#include "Weights.h"
#include "Forest.h"
#include <HypergraphAlgorithms.h>

#include <iostream>
#include <iomanip>

#include "common.h"
#include <boost/program_options.hpp>
#include "lexical.pb.h"
#include "HypergraphLP.h"
#include "DepParseLP.h"
#include "MRFLP.h"
#include "ParseMrfLP.h"
#include "parse/ParseConstraints.h"

using namespace std;
using namespace Scarab::HG;

int main(int argc, char ** argv) {
  
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  // Viterbi
  wvector * weight = load_weights_from_file( argv[1]); //vm["weights"].as< string >().c_str());
  
  GRBEnv env = GRBEnv();
  GRBModel model(env);

  vector <DepParserLP * > lp_vars;
  vector <MRFLP *> mrf_lps; 

  ParseMrfAligner parse_align;
  parse_align.build_from_constraints(argv[8]);


  for (int i=atoi(argv[6]); i <= atoi(argv[7]); i++) {  
    stringstream fname;
    fname << argv[5] << i;
    MRF * mrf =new MRF();  
    cout << fname.str() <<endl;

    mrf->build_from_file(fname.str().c_str());
    stringstream prefix;
    prefix << "cgroup_"<< i;
    LPConfig * lp_conf = new LPConfig(prefix.str(), model, GRB_CONTINUOUS);
    MRFLP * mrf_lp = new MRFLP(*mrf);
    mrf_lp->set_lp_conf(lp_conf);
    mrf_lp->add_vars();
    model.update();
    mrf_lp->add_constraints();
    mrf_lps.push_back(mrf_lp);   
  }
  
  for (int i=atoi(argv[3]); i <= atoi(argv[4]); i++) {  
    stringstream fname;
    fname << argv[2] << i;
  
    DepParser * f = new DepParser();
    f->build_from_file(fname.str().c_str());
  
    HypergraphAlgorithms ha(*f);
    EdgeCache * edge_weights = ha.cache_edge_weights(*weight);
   
    stringstream prefix;
    prefix << "parse" << i;
    LPConfig * lp_conf = new LPConfig(prefix.str(),model, GRB_CONTINUOUS);
    DepParserLP * lp_parse = new DepParserLP(*f, *edge_weights);
    lp_parse->set_lp_conf(lp_conf);
    lp_parse->add_vars();
    model.update();
    lp_parse->add_constraints();
    lp_vars.push_back(lp_parse);
  }
  LPConfig align("", model, GRB_CONTINUOUS);

  ParseMrfLP::align_parse_mrf(mrf_lps, lp_vars, parse_align, model, GRB_CONTINUOUS);
  model.update();


  model.write("/tmp/model.lp");
  model.set(GRB_IntAttr_ModelSense, 1);
  model.optimize();



  for (int i=0; i< lp_vars.size(); i++) {
    cout << "PARSE " << i << endl;
    cout << "SENTENCE: ";
    lp_vars[i]->show();
  }

  for (int i=0; i< mrf_lps.size(); i++) {
    cout << "CONSTRAINT " << i << endl;
    mrf_lps[i]->show();
  }


  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
