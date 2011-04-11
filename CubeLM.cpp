#include <google/protobuf/stubs/common.h>

#include "CubePruning.h"
#include "CubeLM.h"
#include "HypergraphAlgorithms.h"
#include <svector.hpp>
#include "Forest.h"
#include "Hypergraph.h"
#include <fstream>
#include <iostream>
#include <Vocab.h>
#include <Ngram.h>
#include <NGramCache.h>
#include <File.h>
#include <iomanip>
#include <sstream>
#include <time.h>
#include "common.h"
using namespace std;

Cache <Hypernode, int > * cache_word_nodes(Ngram lm, const Forest & forest) {
  int max = lm.vocab.numWords();
  int unk = lm.vocab.getIndex(Vocab_Unknown);
  
  Cache <Hypernode, int > * words = new Cache <Hypernode, int >(forest.num_nodes());
  foreach (HNode hnode, forest.nodes()) { //int i=0; i< forest.num_nodes(); i++ ) {
    const ForestNode & node = * ((ForestNode*)hnode);// (ForestNode) forest.get_node(i);
    if (node.is_word()) {
      string str = node.word();
      int ind = lm.vocab.getIndex(str.c_str());
      //cout << node.id() << endl;
      if (ind == -1 || ind > max) { 
        words->set_value(node, unk);
        //cout << "Word " << unk;
      } else {
        words->set_value(node, ind);
        //cout << "Word " << ind;
      }   
    }
  }
  return words;
}


DEFINE_string(forest_prefix, "", "prefix of the forest files"); 
DEFINE_string(forest_range, "", "range of forests to use (i.e. '0 10')"); 
DEFINE_int64(cube_size, 100, "size of the beam cube"); 

static const bool forest_dummy = RegisterFlagValidator(&FLAGS_forest_prefix, &ValidateReq);
static const bool range_dummy = RegisterFlagValidator(&FLAGS_forest_range, &ValidateRange);


int main(int argc, char ** argv) {
  //cout << argc << endl;
  google::ParseCommandLineFlags(&argc, &argv, true);
  

  wvector * weight = cmd_weights();
  Ngram * lm = cmd_lm();
   

  //cout << "START!!!!" << endl;
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  istringstream range(FLAGS_forest_range);
  int start_range, end_range;
  range >> start_range >> end_range;
  for (int i = start_range; i <= end_range; i++) {     

    
    //Hypergraph hgraph;
    stringstream fname;
    fname << FLAGS_forest_prefix << i;
    Forest f = Forest::from_file(fname.str().c_str());
    
    // Optional:  Delete all global objects allocated by libprotobuf.
    //google::protobuf::ShutdownProtobufLibrary();
    

    //f.append_end_nodes();
    HypergraphAlgorithms ha(f);
    Cache<Hyperedge, double> * w = ha.cache_edge_weights( *weight);
    Cache<Hypernode, int> * words = cache_word_nodes(*lm, f);
    
    clock_t begin=clock();    
    int cube = FLAGS_cube_size;
    CubePruning p(f, *w, LMNonLocal(f, *lm, lm_weight(), *words), cube, 3);
    double v =p.parse();    
    clock_t end=clock();
    cout << "*TRANS* " << i << " ";
    vector <int> sent;
    p.get_derivation(sent);
    foreach (int s, sent) {
      cout <<((ForestNode *) &f.get_node(s))->word() << " ";
    }
    cout << endl;
    cout << "*END*" << i << " "<< v << " " << cube<<" " <<  (double)Clock::diffclock(end,begin) << endl;
    
  }
  return 0;
}

