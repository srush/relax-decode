#include <iomanip>
#include <iostream>
#include <string>

#include "hypergraph/Weights.h"
#include "transforest/Forest.h"

#include "lattice/ForestLattice.h"
#include "trans_decode/Decode.h"
#include "trans_decode/NGramCache.h"
#include "optimization/Subgradient.h"

#include "./CommandLine.h"
#include "./Rates.h"

using namespace std;


DEFINE_string(forest_prefix, "", "prefix of the forest files");
DEFINE_string(lattice_prefix, "", "prefix of the lattice files");
DEFINE_string(forest_range, "", "range of forests to use (i.e. '0 10')");
DEFINE_bool(approx_mode, false, "Use approximate LM updates.");
DEFINE_string(ilp_mode, "proj", "Method to use for tightening.");
DEFINE_int64(simple_cube_size, 100, "Size of the beam for simple cube.");

static const bool forest_dummy =
    RegisterFlagValidator(&FLAGS_forest_prefix, &ValidateReq);
static const bool lattice_dummy =
    RegisterFlagValidator(&FLAGS_lattice_prefix, &ValidateReq);
static const bool range_dummy =
    RegisterFlagValidator(&FLAGS_forest_range, &ValidateRange);

// Build a cache mapping each node to its LM index.
Cache<Hypernode, int > *cache_word_nodes(Ngram lm, const Forest & forest) {
  int max = lm.vocab.numWords();
  int unk = lm.vocab.getIndex(Vocab_Unknown);

  Cache<Hypernode, int > *words =
      new Cache <Hypernode, int >(forest.num_nodes());
  foreach (HNode hnode, forest.nodes()) {
    const ForestNode & node = *(static_cast<const ForestNode*>(hnode));
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

  wvector * weight = cmd_weights();
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
    // decoder
    clock_t setup_begin = clock();
    Decode * d = new Decode(f, graph, *weight, *lm);
    d->set_approx_mode(FLAGS_approx_mode);
    d->set_cached_words(words);
    // Solve
    cout << i << " ";
    TranslationRate tr;
    Subgradient * s = new Subgradient(*d, tr);

    s->set_max_rounds(250);
    int mode;
    if (FLAGS_ilp_mode == "proj") {
      mode = Decode::kProjecting;
    } else if (FLAGS_ilp_mode == "cube") {
      mode = Decode::kCubing;
    } else if (FLAGS_ilp_mode == "simplecube") {
      mode = Decode::kSimpleCubing;
      s->set_max_rounds(1);
      d->set_simple_cube_size(FLAGS_simple_cube_size);
    } else {
      cerr << "Bad ilp mode arg " << FLAGS_ilp_mode;
    }
    d->set_ilp_mode(mode);
    cout << "SETUP TIME "
         << Clock::diffclock(clock(), setup_begin) << endl;
    s->set_debug();
    clock_t begin = clock();
    bool optimal = s->solve(i);
    double v = s->best_primal();
    clock_t end = clock();
    cout << "*END*" << i << " "<< v << "  "
         << Clock::diffclock(end, begin) << " " << mode << " " << optimal << endl;
    delete d;
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

