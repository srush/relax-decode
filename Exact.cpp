#include "HypergraphAlgorithms.h"
#include <cy_svector.hpp>
#include <svector.hpp>
#include "Forest.h"
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
#include "Hypothesis.h"
#include "ForestLattice.h"
#include "GraphDecompose.h"
#include "common.h"
#include "ExtendCKY.h"
using namespace std;

using namespace Scarab::HG;

class ExactController : public Controller {
public:
  ExactController (const ForestLattice & l, Ngram & lm, const GraphDecompose & gd, const Cache<Graphnode, int> & words ) :
    _lattice(l), _lm(lm), _gd(gd), _words(words)
  {}

  // double prune_to() const {
  //   return 0.0;
  // }
  
  double combine_back(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const {
    ret.hook = b.hook;
    ret.right_side = a.right_side;

    for (int i=0;i<a.prev_hyp.size();i++) {
      ret.prev_hyp.push_back(a.prev_hyp[i]);
    }
    ret.prev_hyp.push_back(b.id());
    return 0.0;
  }

  inline int size()  const{
    int d = dim();
    return d * d* d *d;
  }
  double combine(const Hypothesis & a, const Hypothesis & b, Hypothesis & ret) const {
    ret.hook = a.hook;
    ret.right_side = b.right_side;
    for (int i=0;i<a.prev_hyp.size();i++) {
      ret.prev_hyp.push_back(a.prev_hyp[i]);
    }
    ret.prev_hyp.push_back(b.id());
    return 0.0;
  }

  void initialize_out_root(vector <Hypothesis *> & hyps, 
                           vector <double> & scores)  const {
    assert(false);
  }

  inline int dim() const {
    return _lattice.num_word_nodes;
  }

  void initialize_hypotheses(const Hypernode & node, 
                             vector <Hypothesis *> & hyps, 
                             vector <double> & scores) const {
    double _lm_weight = lm_weight();
    int graph_id = _lattice.get_word_from_hypergraph_node(node.id());
    int w1 = _lattice.get_word_from_hypergraph_node(node.id());

    bool has_trigram = _gd.forward_bigrams[w1].size() > 1 || 
      (  _gd.forward_bigrams[w1].size() == 1 &&  
         _gd.forward_bigrams[_gd.forward_bigrams[w1][0]].size() > 0);
    if (has_trigram) { 
      foreach (int w2, _gd.forward_bigrams[w1]) { 
        foreach (int w3, _gd.forward_bigrams[w2]) { 
            
            vector <int> hooks(2);
            vector <int> right_side(2);
            
            hooks[0] = w2;
            hooks[1] = w3;
            right_side[0] = w1;
            right_side[1] = w2;
            Hypothesis * h = new Hypothesis(State(hooks, dim()), State(right_side,  dim()));
            
            VocabIndex context [] = {_words.store[w2], _words.store[w3], Vocab_None};
            double score = _lm_weight * _lm.wordProb(_words.store[w1], context);
              
            h->original_value = score;
            hyps.push_back(h);
            scores.push_back(score);
            //hyps.try_set_hyp(h, score, w, h.is_new);
            //cout << "Initial " << score << endl;        
          }
      }
    } else {
      // This is an <s> 
      vector <int> hooks(2);
      vector <int> right_side(2);
      bool has_bigram = _gd.forward_bigrams[w1].size() == 1;
      int w2 = -1;
      if (has_bigram) 
        w2 = _gd.forward_bigrams[w1][0];
      hooks[0] = w2;
      hooks[1] = -1;
      right_side[0] = w1;
      right_side[1] = w2;
      Hypothesis * h = new Hypothesis(State(hooks, dim()), State(right_side,  dim()));
      
      double score = 0;
      
      h->original_value = score;
      hyps.push_back(h);
      scores.push_back(score);
    }
  }
  

  double find_best( vector <Hypothesis *> & root_hyps, vector<double > & scores, 
                    Hypothesis & best_hyp) const {

    double best = 1e20;
    
    // If we did it correctly
    for (uint iter = 0; iter < root_hyps.size(); iter++) {
      const Hypothesis & hyp1 = *root_hyps[iter]; 
      double my_score = scores[iter];
      if (my_score < best) {
        best = my_score;
        best_hyp = hyp1;
      }
    }


    // //BestHyp::const_iterator iter, check;
    // double best = 1e20;
    // //<s> projection
    // int s_first_projection= 0;
    // int s_projection= 1;
    
    // for (int iter = 0; iter< root_hyps.size(); iter++) {
    //   //if (!root_hyps.has_key(iter)) continue;
    //   //cout << scores[iter] << endl;
    //   const Hypothesis & hyp1 = *root_hyps[iter]; 
    //   double score1 = scores[iter];
    //   if (hyp1.hook[0] != s_projection || hyp1.hook[1] != s_first_projection) {
    //     continue;
    //   }
      
      
    //   double my_score = score1;
    //   //cout << "Before " << my_score << endl;
    //   // factor in last score
      
    //   {
    //     int id = _lattice.num_word_nodes-2;
    //     //cout << "Best " << hyp1.right_side << endl;
    //     VocabIndex context [] = {_words.store[hyp1.right_side[0]], _words.store[hyp1.right_side[1]], Vocab_None};
    //     my_score += LM * _lm.wordProb(_words.store[id], context);
    //   }
    //   //cout << "Middle " << my_score << endl;
      
      
    //   { 
    //     int id = _lattice.num_word_nodes-1;
    //     int d = _lattice.num_word_nodes-2;

    //     VocabIndex context [] = {_words.store[d], _words.store[hyp1.right_side[0]], Vocab_None};
    //     my_score += LM * _lm.wordProb(_words.store[id], context);
    //   }

      
    
      
    //   //cout << "After " << my_score << endl;
    //   if (my_score < best) {
    //     best = my_score;
    //     best_hyp = hyp1;
    //     //cout << "found" << endl;
    //   }
    // }
    

    return best;
  }

private:
  const GraphDecompose & _gd;
  const ForestLattice & _lattice;
  Ngram & _lm;
  const Cache <Graphnode, int > & _words;
};




Cache <Graphnode, int > * sync_lattice_lm(Ngram & _lm, const ForestLattice & _lattice) {
  Cache <Graphnode, int > * _cached_words = new Cache <Graphnode, int> (_lattice.num_word_nodes);
  int max = _lm.vocab.numWords();
  int unk = _lm.vocab.getIndex(Vocab_Unknown);
  //assert(false);
  for (int n=0; n < _lattice.num_word_nodes; n++ ) {
    if (!_lattice.is_word(n)) continue;
    
    //const Graphnode & node = _lattice.node(n); 
    //assert (node.id() == n);
    string str = _lattice.get_word(n);
    int ind = _lm.vocab.getIndex(str.c_str());
    if (ind == -1 || ind > max) {
      //cout << "Unknown " << str << endl; 
      _cached_words->store[n] = unk;
    } else {
      _cached_words->store[n] = ind;
    }
  }
  return _cached_words;
}

DEFINE_string(forest_prefix, "", "prefix of the forest files"); 
DEFINE_string(forest_range, "", "range of forests to use (i.e. '0 10')"); 
DEFINE_string(lattice_prefix, "", "prefix of the lattice files"); 

static const bool forest_dummy = RegisterFlagValidator(&FLAGS_forest_prefix, &ValidateReq);
static const bool range_dummy = RegisterFlagValidator(&FLAGS_forest_range, &ValidateRange);
static const bool lattice_dummy = RegisterFlagValidator(&FLAGS_lattice_prefix, &ValidateReq);

int main(int argc, char ** argv) {
  //cout << argc << endl;
  google::ParseCommandLineFlags(&argc, &argv, true);
  GOOGLE_PROTOBUF_VERIFY_VERSION;  
  wvector * weight = cmd_weights();
  Ngram * lm = cmd_lm();

  istringstream range(FLAGS_forest_range);
  int start_range, end_range;
  range >> start_range >> end_range;
  for (int i = start_range; i <= end_range; i++) {     
   
    stringstream fname;
    fname << FLAGS_forest_prefix << i;
    Forest _forest = Forest::from_file(fname.str().c_str());
   

    // Load lattice
    stringstream fname2;
    fname2 << FLAGS_lattice_prefix << i;
    ForestLattice _lattice = ForestLattice::from_file(fname2.str());
 
    
    GraphDecompose gd(_lattice);
    gd.decompose();
    HypergraphAlgorithms alg(_forest);
    Cache<Hyperedge, double> * w = alg.cache_edge_weights(*weight);
    Cache<Graphnode, int> * words = sync_lattice_lm(*lm, _lattice);
    
    clock_t begin=clock();    

    NodeBackCache back_pointers(_forest.num_nodes());
    ExactController c(_lattice, *lm, gd, *words);
    ExtendCKY ecky(_forest, *w, c);
    double v = ecky.best_path(back_pointers);
    
    //CubePruning p(f, *w, LMNonLocal(f, *lm, *words), cube, 2);
    //double v =p.parse();    
    clock_t end=clock();
    cout << "*END* " << i << " "<< v <<" " <<  (double)Clock::diffclock(end,begin) << endl;
  }
  return 0;
}

