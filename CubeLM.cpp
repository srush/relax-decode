#include <google/protobuf/stubs/common.h>

#include "CubePruning.h"
#include "CubeLM.h"
#include "HypergraphAlgorithms.h"
//#include "ForestAlgorithms.h"
//#include <cy_svector.hpp>
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
#include "util.h"
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


int main(int argc, char ** argv) {
  //cout << argc << endl;

  

  wvector * weight = load_weights_from_file(argv[2]);
  Ngram * lm = load_ngram_cache(argv[3]);
   

  //cout << "START!!!!" << endl;
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  for (int i=atoi(argv[5]); i <=atoi(argv[6]); i++) {
    

    
    //Hypergraph hgraph;
    stringstream fname;
    fname << argv[1] << i;

    Forest f = Forest::from_file(fname.str().c_str());
    
    // Optional:  Delete all global objects allocated by libprotobuf.
    //google::protobuf::ShutdownProtobufLibrary();
    

    f.append_end_nodes();
    HypergraphAlgorithms ha(f);
    Cache<Hyperedge, double> * w = ha.cache_edge_weights( *weight);
    Cache<Hypernode, int> * words = cache_word_nodes(*lm, f);
    
    clock_t begin=clock();    
    int cube = atoi(argv[4]);
    CubePruning p(f, *w, LMNonLocal(f, *lm, *words), cube, 3);
    double v =p.parse();    
    clock_t end=clock();
    cout << "*TRANS* " << i << " ";
    vector <int> sent;
    p.get_derivation(sent);
    foreach (int s, sent) {
      cout <<((ForestNode *) &f.get_node(s))->word() << " ";
    }
    cout << endl;
    cout << "*END*" << i << " "<< v << " " << cube<<" " <<  (double)diffclock(end,begin) << endl;
    
  }
  return 0;
}

