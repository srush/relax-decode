#include "Weights.h"
#include "Forest.h"
#include <HypergraphAlgorithms.h>

#include <iostream>
#include <iomanip>

#include "common.h"
#include <boost/program_options.hpp>
#include "lexical.pb.h"


using namespace std;
using namespace Scarab::HG;
namespace po = boost::program_options;



int main(int argc, char ** argv) {
  

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("forest,F", po::value< string >(), "Forest file")
    ("weights,w", po::value< string >(), "Weight file")
    ;

 
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);    
  
  if (vm.count("help")) {
    cout << desc << "\n";
    return 1;
  }

  cout << vm["weights"].as< string >() << " " << vm["forest"].as< string >()<<endl;
  // Viterbi
  wvector * weight = load_weights_from_file( vm["weights"].as< string >().c_str());

  stringstream fname;
  fname << vm["forest"].as< string >();
  Forest f = Forest::from_file(fname.str().c_str());
  
  //cout << "START!!!!" << endl;
  NodeCache  score_memo_table(f.num_nodes()); 
  NodeBackCache  back_memo_table(f.num_nodes());
  HypergraphAlgorithms ha(f);
  EdgeCache * edge_weights = ha.cache_edge_weights(*weight);

  double score = ha.best_path( *edge_weights, score_memo_table, back_memo_table);
  
  HNodes best_nodes = ha.construct_best_node_order(back_memo_table);
  
  //foreach (HNode node, best_nodes) { 
  //cout << ((ForestNode *)node)->_label << endl;
  //}


  cout << "Score is : " << -score << endl;

  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
