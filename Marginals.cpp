#include "Weights.h"
#include "HypergraphImpl.h"
#include "Tagger.h"
#include <HypergraphAlgorithms.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "common.h"
using namespace Scarab::HG;
int main(int argc, char ** argv) {
  
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  wvector * weight = load_weights_from_file( argv[1]); 

  for (int i =atoi(argv[3]); i <= atoi(argv[4]); i++) {   
    stringstream fname;
    fname << argv[2] << i;

    cerr << "Margs " << i << endl; 
    cout << "SENT " << i << endl; 

    Tagger f(200);
    f.build_from_file(fname.str().c_str());

    HypergraphAlgorithms ha(f);
    EdgeCache * edge_weights = ha.cache_edge_weights(*weight);
  
    NodeCache  inside_memo_table(f.num_nodes()), outside_memo_table(f.num_nodes()), marginals(f.num_nodes()); ; 
    
    ha.inside_scores( *edge_weights, inside_memo_table);
    ha.outside_scores( *edge_weights, inside_memo_table, outside_memo_table);

    ha.collect_marginals(inside_memo_table, outside_memo_table, marginals);
    
    foreach (HNode node, f.nodes()) {
      if (marginals.has_key(*node)) {
        if (f.node_has_tag(*node)) {
          cout << "Node " <<  node->id() << " "<< marginals.get(*node) << " " <<inside_memo_table.get(*node)<<" "<< outside_memo_table.get(*node) << " " << f.node_to_tag(*node)<< endl;
        }
      }
    }
  }
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
