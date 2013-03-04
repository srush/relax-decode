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
#include <LMNonLocal.h>
#include <File.h>
#include <iomanip>
#include <sstream>
#include <time.h>
#include "common.h"
using namespace std;

DEFINE_string(forest_prefix, "", "The prefix of the forest files"); 
DEFINE_string(forest_range, "", "The range of forests to use (i.e. '0 10')"); 
DEFINE_int64(cube_size, 100, "The size of the beam for cube pruning."); 

static const bool forest_dummy = RegisterFlagValidator(&FLAGS_forest_prefix, &ValidateReq);
static const bool range_dummy = RegisterFlagValidator(&FLAGS_forest_range, &ValidateRange);

int main(int argc, char ** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  // Read weight vector and language model from command-line.
  wvector * weight = cmd_weights();
  Ngram * lm = cmd_lm();
  int n_best = 10;

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  
  // Get range from command-line.
  istringstream range(FLAGS_forest_range);
  int start_range, end_range;
  range >> start_range >> end_range;

  for (int i = start_range; i <= end_range; i++) {     

    // Read in the forest. 
    stringstream fname;
    fname << FLAGS_forest_prefix << i;
    Forest f = Forest::from_file(fname.str().c_str());
        
    // Initialize the weight of each edge and the word on each node. 
    HypergraphAlgorithms ha(f);
    Cache<Hyperedge, double> * w = ha.cache_edge_weights( *weight);
    Cache<Hypernode, int> * words = cache_word_nodes(*lm, f);
    
    // Run cube pruning. 
    clock_t begin=clock();    
    int cube = FLAGS_cube_size;
    CubePruning p(f, *w, LMNonLocal(f, *lm, lm_weight(), *words, true), cube, 3);
    double v = p.parse();
    clock_t end = clock();

    //cout << "*TRANS* " << i << " ";

    // Output each of the n-best derivations. 
    for (int n = 0; n < min(p.get_num_derivations(), n_best); ++n) {

      // Initialize sent to be the derivation. 
      vector<int> sent;
      p.get_derivation(sent, n);
      cout << i << " ||| ";

      // Check the language model score. 
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
          cout << lm_weight() * lm->wordProb(words->store[sent[j]],  context) << " " ;
        }
      }
      cout << " ||| " ;

      // Recompute vector scores. 
      vector<int> edges;
      p.get_edges(edges, n);
      svector<int, double> vector;
      double total = 0.0;
      foreach (int e, edges) {
        vector += f.get_edge(e).fvector();
        total += weight->dot(f.get_edge(e).fvector());
      }
      
      for(int i = 0; i < vector.size(); ++i) {
        cout << -vector[i] << " ";
      }
      cout << -lm_score << " ";
      double score = p.get_score(n);

      cout << " ||| " << -score;
      cout << endl;
      //cerr << lm_score * lm_weight() + total << endl;
    }
    cout << "*END*" << i << " "<< v << " " << cube<<" " <<  (double)Clock::diffclock(end,begin) << endl;
  }
  return 0;
}


      //foreach (int e, edges) {
        //cerr << f.get_edge(e).id() << endl;
        //cerr << ((HyperedgeImpl *)&f.get_edge(e))->feature_string() << endl;
      //}
