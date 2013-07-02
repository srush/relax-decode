#ifndef LMNONLOCAL_H_
#define LMNONLOCAL_H_

#include <vector>
#include <string>
#include <iostream>

#include "CubePruning.h"
#include "EdgeCache.h"
#include "Hypergraph.h"
#include <Ngram.h>

using namespace std;

// Build a cache mapping each node to its LM index.
/* Cache<Hypernode, int > *cache_word_nodes_cube(Ngram lm, const Forest & forest) { */
/*   int max = lm.vocab.numWords(); */
/*   int unk = lm.vocab.getIndex(Vocab_Unknown); */

/*   Cache<Hypernode, int > * words = */
/*       new Cache<Hypernode, int >(forest.num_nodes()); */
/*   foreach (HNode hnode, forest.nodes()) { */
/*     const ForestNode & node = * (static_cast<const ForestNode *>(hnode)); */
/*     if (node.is_word()) { */
/*       string str = node.word(); */
/*       int ind = lm.vocab.getIndex(str.c_str()); */
/*       // Unknown cases. */
/*       if (ind == -1 || ind > max) { */
/*         words->set_value(node, unk); */
/*       } else { */
/*         words->set_value(node, ind); */
/*       } */
/*     } */
/*   } */
/*   return words; */
/* } */


template <class TDerivation>
class BaseNonLocal : public NonLocal<TDerivation> {
public:
  BaseNonLocal(const HGraph & forest,
               Ngram & lm,
               double lm_weight,
               const Cache <Hypernode, int> & word_cache,
               bool prescore)
    : _forest(forest), _lm(lm), _lm_weight(lm_weight),
      _word_cache(word_cache), _prescore(prescore) {}
  double unigram(int word_index) const {
    assert(word_index != -1);
    const VocabIndex context[] = {Vocab_None};
    return _lm.wordProb(word_index, context) * _lm_weight;
  }

  double bigram(int word_index, int w2) const {
    assert(word_index != -1);
    VocabIndex context[] = {w2, Vocab_None};
    return _lm.wordProb(word_index, context) * _lm_weight;
  }

  double trigram(int word_index, int w2, int w3) const {
    assert(word_index != -1);
    VocabIndex context[] = {w2, w3, Vocab_None};
    return _lm.wordProb(word_index, context) * _lm_weight;
  }

  int index(int word) const {
    return _word_cache.store[word];
  }

  bool special(int word_index) const {
    return word_index == 1 || word_index == 2;
  }

 protected:
  // The underlying hypergraph.
  const HGraph  & _forest;

  // The language model.
  Ngram & _lm;

  // Weight to give to the language model.
  const double _lm_weight;

  // The language model index for each hypernode.
  const Cache <Hypernode, int> & _word_cache;

  bool _prescore;

};

class LMNonLocal: public BaseNonLocal<vector<int> > {
 public:
  LMNonLocal(const HGraph &forest,
            Ngram & lm,
            double lm_weight,
            const Cache <Hypernode, int> & word_cache,
            bool prescore)
      : BaseNonLocal<vector<int> >(forest, lm, lm_weight, word_cache, prescore) {}


  // Compute takes the hyperedge and sub-derivations to combine.
  // Returns the new score, derivation and signature.
  virtual bool compute(const Hyperedge &edge,
               int edge_pos,
               double bound,
               const vector<const vector<int> *> &subder,
               double &score,
               vector <int> &full_derivation,
               vector <int> &signature) const {
    full_derivation.clear();
    signature.clear();
    score = 0.0;

    foreach (const vector<int> *sub, subder) {
      uint size = full_derivation.size();
      int w = index((*sub)[0]);
      int w2 = -1;
      int last1 = -1;
      int last2 = -1;
      if (sub->size() > 1) w2 = index((*sub)[1]);
      if (size >= 1) last1 = index(full_derivation[size - 1]);
      if (size >= 2) last2 = index(full_derivation[size - 2]);

      // Handle word w (left-most).
      if (size >= 2 && last2 != -1) {
        // Create a new trigram.
        score += trigram(w, last1, last2);
        if (_prescore && !special(w)) score -= unigram(w);
      } else if (size == 1 && w != 1) {
        // Create a new bigram.

       if (_prescore && !special(w)) {
         score += bigram(w, last1);
         score -= unigram(w);
        }
      }

      // Handle word w2 (second to left).
      if (size >= 1 && sub->size() > 1) {
        // Create a new trigram.
        score += trigram(w2, w, last1);
        if (_prescore && !special(w2)) score -= bigram(w2, w);
      }

      // Add full signature to derivation.
      foreach (int final, *sub) {
        full_derivation.push_back(final);
      }
    }

    assert(score < 10000);
    int full_size = full_derivation.size();
    assert(full_size > 0);

    // New signature is w_0 w_n w_1 w_{n-1}.
    signature.push_back(index(full_derivation[0]));
    signature.push_back(index(full_derivation[full_size - 1]));
    if (full_size != 1) {
      signature.push_back(index(full_derivation[1]));
      signature.push_back(index(full_derivation[full_size - 2]));
    }
    return true;
  }

  // Initialize the hypothesis for leaf nodes.
  virtual Hyp<vector<int> > initialize(const Hypernode &node) const {
    assert(node.is_terminal());

    // Get the index of the word at this node
    int word_index = _word_cache.get_value(node);
    double score = 0.0;

    // If this is not a special node, add in the unigram score.
    if (_prescore && !special(word_index)) {
      score += unigram(word_index);
    }

    // The signature (left and right words).
    vector<int> signature;
    signature.push_back(word_index);
    signature.push_back(word_index);

    // So far dervation is just this node
    vector<int> derivation;
    derivation.push_back(node.id());

    // There are no edges.
    vector<int> edges;

    return Hyp<vector<int> >(score, score, signature, derivation, edges);
  }
};


#endif
