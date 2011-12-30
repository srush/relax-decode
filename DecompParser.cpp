#include "Weights.h"
#include "Forest.h"
#include <HypergraphAlgorithms.h>


#include <iostream>
#include <iomanip>

#include "common.h"
#include "lexical.pb.h"
#include "parse/ParseConstraints.h"
#include "parse/ParseSolvers.h"
#include "parse/SOEisnerToHypergraph.h"
#include "DualDecomposition.h"
#include "Subgradient.h"
#include "MRF.h"
#include "MRFSolvers.h"
#include "Rates.h"
#include <gflags/gflags.h>

DEFINE_string(parse_file, "", 
              "The file with precomputed parse features.");
DEFINE_string(feature_weights_file, "", 
              "The file with weights for 'value' feature.");

DEFINE_string(alignment_file, "", 
              "The file aligning the mrf/parse models.");
DEFINE_int32(mrf_start, 0, 
             "Start index of mrf.");
DEFINE_int32(mrf_end, 0, 
             "End index of mrf.");
DEFINE_string(mrf_file_prefix, "", 
              "Prefix of the mrf file.");


using namespace std;
using namespace Scarab::HG;


int main(int argc, char ** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::ParseCommandLineFlags(&argc, &argv, true);

  // Viterbi
  //argv[1]
  wvector * weight = load_weights_from_file(FLAGS_feature_weights_file.c_str()); 
  
  vector <DepParser * > parsers;
  vector <MRF *> mrfs; 
  ParseMrfAligner parse_align;

  // 8 
  parse_align.build_from_constraints(FLAGS_alignment_file.c_str());

  // 3 4
  cout << "parse file " << FLAGS_parse_file;
  parsers = SecondOrderConverter().convert_file(FLAGS_parse_file.c_str());

  for (int i=FLAGS_mrf_start; i <= FLAGS_mrf_end; i++) {  
    stringstream fname;
    fname << FLAGS_mrf_file_prefix << i;
    MRF * mrf =new MRF();  
    cout << fname.str() << endl;

    mrf->build_from_file(fname.str().c_str());
    mrfs.push_back(mrf);   
  }
  
  
  ParserDual p_dual(parsers, *weight, parse_align);
  
  wvector * simple = svector_from_str<int, double>("value=-1");
  ConstrainerDual<ParseIndex> mrf_dual(mrfs, *simple, parse_align);
  
  ParseRate rate;
  DualDecomposition d(p_dual, mrf_dual, rate);
  //d._subgradsolver.rate = new ParseRate();
  d.solve(0);
  
  for (int i =0; i < p_dual.best_derivations.size(); i++) {
    //foreach (HEdges derivations, p_dual.best_derivations) {
    cout << "SENTENCE: ";    
    parsers[i]->show_derivation(p_dual.best_derivations[i]);
  }

  for (int i =0; i < mrf_dual.best_derivations.size(); i++) {
    //foreach (HEdges derivations, p_dual.best_derivations) {
    // Don't Delete, needed for scoring.
    cout << "CONSTRAINT: NEW" << endl;
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
