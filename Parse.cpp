#include "Weights.h"
#include "DepParser.h"
#include <HypergraphAlgorithms.h>

#include <iostream>
#include <iomanip>

#include "common.h"
#include "parse/SOEisnerToHypergraph.h"
#include "lexical.pb.h"
#include <gflags/gflags.h>
using namespace std;
using namespace Scarab::HG;


DEFINE_string(parse_file, "", 
              "The file with precomputed parse features.");
DEFINE_string(feature_weights_file, "", 
              "The file with weights for 'value' feature.");


int main(int argc, char ** argv) {
  

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::ParseCommandLineFlags(&argc, &argv, true);

  wvector * weight = load_weights_from_file(FLAGS_parse_file.c_str()); //vm["weights"].as< string >().c_str());
  double total_score = 0.0;
  vector<DepParser * > parsers;
  parsers = SecondOrderConverter().convert_file(FLAGS_parse_file.c_str());
  

  for (uint i=0; i <= parsers.size(); i++) {  
//     stringstream fname;
//     fname << argv[2] << i;
    DepParser &f = *parsers[i];
    //f.build_from_file(fname.str().c_str());
    
    //bool lp = (int)atoi(argv[3]);
    
    
    HypergraphAlgorithms ha(f);
    EdgeCache * edge_weights = ha.cache_edge_weights(*weight);
    
    cout << "START!!!!" << endl;
    
    NodeCache  score_memo_table(f.num_nodes()); 
    
    NodeBackCache  back_memo_table(f.num_nodes());
    
    double score = ha.best_path( *edge_weights, score_memo_table, back_memo_table);
    
    HNodes best_nodes = ha.construct_best_node_order(back_memo_table);
    
    HEdges best_edges = ha.construct_best_edges(back_memo_table);
    
    cout << endl << "SENTENCE: ";
    f.show_derivation(best_edges);
    
    //foreach (HNode node, best_nodes) { 
    //cout << ((ForestNode *)node)->_label << endl;
    //b}
    cout << "Score is : " << -score << endl;

    total_score += score;
  }

  cout << "Total Score " << -total_score << endl;
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
