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

DEFINE_string(forest_prefix, "", "prefix of the forest files"); 
DEFINE_string(forest_range, "", "range of forests to use (i.e. '0 10')"); 
DEFINE_int64(cube_size, 100, "size of the beam cube"); 

static const bool forest_dummy = RegisterFlagValidator(&FLAGS_forest_prefix, &ValidateReq);
static const bool range_dummy = RegisterFlagValidator(&FLAGS_forest_range, &ValidateRange);

// Build a cache mapping each nodes to their LM index.
Cache<Hypernode, int > *cache_word_nodes(Ngram lm, const Forest & forest) {
  int max = lm.vocab.numWords();
  int unk = lm.vocab.getIndex(Vocab_Unknown);
  
  Cache<Hypernode, int > * words = new Cache <Hypernode, int >(forest.num_nodes());
  foreach (HNode hnode, forest.nodes()) {
    const ForestNode & node = * ((ForestNode*)hnode);
    if (node.is_word()) {
      string str = node.word();
      int ind = lm.vocab.getIndex(str.c_str());
      // Unknown cases. 
      if (ind == -1 || ind > max) { 
        words->set_value(node, unk);
      } else {
        words->set_value(node, ind);
      }   
    }
  }
  return words;
}

int main(int argc, char ** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  

  wvector * weight = cmd_weights();
  Ngram * lm = cmd_lm();
  int n_best = 10;

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  istringstream range(FLAGS_forest_range);
  int start_range, end_range;
  range >> start_range >> end_range;
  for (int i = start_range; i <= end_range; i++) {     
    stringstream fname;
    fname << FLAGS_forest_prefix << i;
    Forest f = Forest::from_file(fname.str().c_str());
        
    HypergraphAlgorithms ha(f);
    Cache<Hyperedge, double> * w = ha.cache_edge_weights( *weight);
    Cache<Hypernode, int> * words = cache_word_nodes(*lm, f);
    
    clock_t begin=clock();    
    int cube = FLAGS_cube_size;
    CubePruning p(f, *w, LMNonLocal(f, *lm, lm_weight(), *words), cube, 3);
    double v =p.parse();
    clock_t end=clock();
    //cout << "*TRANS* " << i << " ";
    for (int n = 0; n < n_best; ++n) {
      vector<int> sent;
      p.get_derivation(sent, n);
      cout << i << " ||| ";
      double lm_score = 0.0;
      for (int j = 0; j < sent.size(); ++j) {
        int s = sent[j];
        string word = ((ForestNode *) &f.get_node(s))->word();
        if (!(word == "<s>" || word == "</s>")) { 
          cout << word << " ";
        }
        if (j > 1) {
          VocabIndex context [] = { words->store[sent[j - 1]], 
                                    words->store[sent[j - 2]], 
                                    Vocab_None };
          lm_score += lm->wordProb(words->store[sent[j]],  context);
        }
      }
      cout << " ||| " ;
      vector<int> edges;
      p.get_edges(edges, n);
      svector<int, double> vector;
      foreach (int e, edges) {
        vector += f.get_edge(e).fvector();
      }
      for(int i = 0; i < vector.size(); ++i) {
        cout << vector[i] << " ";
      }
      cout << lm_score << " ";
      double score = p.get_score(n);

      cout << " ||| " << -score;
      cout << endl;
    }
    //cout << "*END*" << i << " "<< v << " " << cube<<" " <<  (double)Clock::diffclock(end,begin) << endl;
    
  }
  return 0;
}

