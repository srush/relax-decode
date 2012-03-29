#ifndef CUBELM_H_
#define CUBELM_H_

#include "CubePruning.h"
#include "EdgeCache.h"
#include "Hypergraph.h"
#include <Ngram.h>
#include "common.h"
class LMNonLocal: public NonLocal {
 public:
 LMNonLocal(const HGraph & forest,  
            Ngram & lm, 
            double lm_weight, 
            const Cache <Hypernode, int> & word_cache) 
   : _forest(forest), _lm(lm), _lm_weight(lm_weight), _word_cache(word_cache) {}
  
  void compute(const Hyperedge &edge,
               const vector <vector <int> > &subder,
               double &score,
               vector <int> &full_derivation,
               vector <int> &signature) const {
    full_derivation.clear();
    signature.clear();
    score =0.0;

    for (uint i = 0; i < subder.size(); i++) {
      uint size = full_derivation.size(); 
      int orig = subder[i][0];
      int w = _word_cache.store[orig];

      if (size >= 2) {      
        VocabIndex context [] = {_word_cache.store[full_derivation[size - 1]], 
                                 _word_cache.store[full_derivation[size - 2]], 
                                 Vocab_None};
        score += _lm.wordProb(w, context);

        // Subtract out unigram probability.
        if (w != 1 && w != 2) {
          const VocabIndex context2 [] = {Vocab_None};
          score -= _lm.wordProb(w, context2);        
        }

      } else if (size ==1 && w != 1 ) {
        
        if (w != 1 && w != 2) {
          VocabIndex context [] = {_word_cache.store[full_derivation[size-1]], Vocab_None};
          score += _lm.wordProb(w, context);        
     
          // Subtract out unigram probability.
          const VocabIndex context2 [] = {Vocab_None};
          score -= _lm.wordProb(w, context2);        
          //cout << "bonus" << endl;
        }
      }

      if (size >=1 && subder[i].size() > 1 ) {
        const VocabIndex context [] = {w, _word_cache.store[full_derivation[size-1]], Vocab_None};
        score += _lm.wordProb(_word_cache.store[subder[i][1]], context);
        
        if ( _word_cache.store[subder[i][1]]!= 1 && _word_cache.store[subder[i][1]]!=2) {
          const VocabIndex context2 [] = {w, Vocab_None};
          score -= _lm.wordProb(_word_cache.store[subder[i][1]], context2);        
          }
      }
      foreach (int final, subder[i]) {
        full_derivation.push_back(final);
      }
    }

    score *= _lm_weight;
    int size = full_derivation.size();

    signature.push_back(_word_cache.store[full_derivation[0]]);
    signature.push_back(_word_cache.store[full_derivation[size-1]]);
    assert(size > 0);
    if (size!=1) {
      signature.push_back(_word_cache.store[full_derivation[1]]);
      signature.push_back(_word_cache.store[full_derivation[size-2]]);
    }
  }
  
  // Initialize the hypothesis for leave nodes.
  Hyp initialize(const Hypernode &node) const {
    assert(node.is_terminal());
    int word_index = _word_cache.get_value(node);
    double score = 0.0; 
    VocabIndex context [] = {Vocab_None};

    // Not a special word (todo: fix).
    if (word_index != 1 && word_index != 2) {
      // Unigram probability.
      score += _lm.wordProb(word_index, context);
      score *= _lm_weight;
    }

    // Signature (left and right words).
    vector <int> signature;
    signature.push_back(word_index);
    signature.push_back(word_index);

    // Build up the dervation of hypernodes.
    vector <int> derivation;
    derivation.push_back(node.id());

    vector <int> edges;
    return Hyp(score, signature, derivation, edges); 
  }

 private:
  // The underlying hypergraph.
  const HGraph  & _forest;  

  // The language model.
  Ngram & _lm;

  // Weight to give to the language model.
  const double _lm_weight;

  // The language model index for each hypernode.
  const Cache <Hypernode, int> & _word_cache;  

};

#endif
