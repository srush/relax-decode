#include "Weights.h"
#include "Forest.h"

#include "ForestLattice.h"
#include "Decode.h"
#include "NGramCache.h"
#include "Subgradient.h"

#include <iostream>
#include <iomanip>
#include "CommandLine.h"
#include "Rates.h"
using namespace std;


DEFINE_string(forest_prefix, "", "prefix of the forest files"); // was 1
DEFINE_string(lattice_prefix, "", "prefix of the lattice files"); // was 2
DEFINE_string(forest_range, "", "range of forests to use (i.e. '0 10')"); // was 5 6

static const bool forest_dummy = RegisterFlagValidator(&FLAGS_forest_prefix, &ValidateReq);
static const bool lattice_dummy = RegisterFlagValidator(&FLAGS_lattice_prefix, &ValidateReq);
static const bool range_dummy = RegisterFlagValidator(&FLAGS_forest_range, &ValidateRange);

// Build a cache mapping each node to its LM index.
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
  srand(0);
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::ParseCommandLineFlags(&argc, &argv, true);

  // weights
  wvector * weight = cmd_weights();
  // lm
  NgramCache * lm = cmd_lm();

  istringstream range(FLAGS_forest_range);
  int start_range, end_range;
  range >> start_range >> end_range;
  for (int i = start_range; i <= end_range; i++) { 

    // Load forest
    stringstream fname;
    fname << FLAGS_forest_prefix << i;
    Forest f = Forest::from_file(fname.str().c_str());

    // Load lattice
    stringstream fname2;
    fname2 << FLAGS_lattice_prefix << i;
    ForestLattice graph = ForestLattice::from_file(fname2.str());
    Cache<Hypernode, int> * words = cache_word_nodes(*lm, f);

    cerr << i << endl;
    clock_t begin=clock();    
    // decoder 
    clock_t setup_begin=clock();    
    Decode * d = new Decode(f, graph, *weight, *lm);
    d->set_cached_words(words);
    // Solve
    cout << i << " ";
    TranslationRate tr;
    //ConstantRate tr(0.01);
    //FallingRate tr(0.05);
    Subgradient * s = new Subgradient(*d, tr);
    cout << "SETUP TIME " <<  (double)Clock::diffclock(clock(),setup_begin) << endl;
    //s->set_debug();
    s->solve(i);
    double v = s->best_primal();
    clock_t end = clock();
    cout << "*END*" << i << " "<< v << "  " <<  (double)Clock::diffclock(end,begin) << endl;
  }
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}


    // transition to having <s> border words
    // if (FULLBUILT) {
    //   //f.append_end_nodes();
    //    }

    // Lattice lat;  
    // {
    //   //cout << fname << endl; 
    //   fstream input(fname.str().c_str(), ios::in | ios::binary);
    //   if (!lat.ParseFromIstream(&input)) {
    //     assert (false);
    //   }
    // }
