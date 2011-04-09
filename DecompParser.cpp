#include "Weights.h"
#include "Forest.h"
#include <HypergraphAlgorithms.h>


#include <iostream>
#include <iomanip>

#include "common.h"
#include <boost/program_options.hpp>
#include "lexical.pb.h"
#include "parse/ParseConstraints.h"
#include "parse/ParseSolvers.h"
#include "DualDecomposition.h"
#include "Subgradient.h"
#include "MRF.h"
#include "MRFSolvers.h"
using namespace std;
using namespace Scarab::HG;

int main(int argc, char ** argv) {
  
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  // Viterbi
  wvector * weight = load_weights_from_file( argv[1]); //vm["weights"].as< string >().c_str());
  

  vector <DepParser * > parsers;
  vector <MRF *> mrfs; 

  ParseMrfAligner parse_align;
  parse_align.build_from_constraints(argv[8]);

  for (int i=atoi(argv[6]); i <= atoi(argv[7]); i++) {  
    stringstream fname;
    fname << argv[5] << i;
    MRF * mrf =new MRF();  
    cout << fname.str() <<endl;

    mrf->build_from_file(fname.str().c_str());
    mrfs.push_back(mrf);   
  }
  
  for (int i=atoi(argv[3]); i <= atoi(argv[4]); i++) {  
    stringstream fname;
    fname << argv[2] << i;
  
    DepParser * f = new DepParser();
    f->build_from_file(fname.str().c_str());  
    cout << fname.str() << endl ;
    parsers.push_back(f);
  }

  ParserDual p_dual(parsers, *weight, parse_align);
  
  wvector * simple = svector_from_str<int, double>("value=-1");
  ConstrainerDual<ParseIndex> mrf_dual(mrfs, *simple, parse_align);
  
  DualDecomposition d(p_dual, mrf_dual);
  d._subgradsolver.rate = new ParseRate();
  d.solve(0);
  
  for (int i =0; i < p_dual.best_derivations.size(); i++) {
    //foreach (HEdges derivations, p_dual.best_derivations) {
    cout << "SENTENCE: ";    
    parsers[i]->show_derivation(p_dual.best_derivations[i]);
  }

  for (int i =0; i < mrf_dual.best_derivations.size(); i++) {
    //foreach (HEdges derivations, p_dual.best_derivations) {
    //cout << "CONSTRAINT: NEW" << endl;
    mrfs[i]->show_derivation(mrf_dual.best_derivations[i]);
    cout << endl;
    cout << "CONSTRAINT: DONE" << endl;
  }


//   for (int i=0; i< mrf_lps.size(); i++) {
//     cout << "CONSTRAINT " << i << endl;
//     mrf_lps[i]->show();
//   }


  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
