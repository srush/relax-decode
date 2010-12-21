#ifndef CUBELM_H_
#define CUBELM_H_

#include "CubePruning.h"
#include "EdgeCache.h"
#include "Forest.h"
#include <Ngram.h>
#include "common.h"
class LMNonLocal: public NonLocal {
 public:
 LMNonLocal(const Forest & forest,  Ngram & lm, const Cache <ForestNode, int> & word_cache) 
   : _word_cache(word_cache), _lm(lm), _forest(forest) {}
  
  void compute(const ForestEdge & edge,
               const vector <vector <int> > & subder,
               double & score,
               vector <int>  & full_derivation,
               vector <int> & sig
               ) const {
    full_derivation.clear();
    sig.clear();
    score =0.0;
    //cout << "COMBINE " << subder.size() <<endl;
    for (int i =0; i < subder.size(); i++) {
      int size = full_derivation.size(); 
      int orig = subder[i][0];
      int w = _word_cache.store[orig];

      if (size >= 2) {
      
        VocabIndex context [] = {_word_cache.store[full_derivation[size-1]], 
                                 _word_cache.store[full_derivation[size-2]], 
                                 Vocab_None};
        score += _lm.wordProb(w, context);
        
        // subtract out uni
        if (w!=1 && w!=2) {
          const VocabIndex context2 [] = {Vocab_None};
          score -= _lm.wordProb(w, context2);        
          //cout << "bonus" << endl;
        }
        //cout << "\t" << score <<endl;
        //cout << "\t" <<  "TRIGRAM " << w << " " << full_derivation[size-1] << " " << full_derivation[size-2] <<endl;
      } else if (size ==1 && w != 1 ) {
        
        if (w !=1 && w!= 2) {
          VocabIndex context [] = {_word_cache.store[full_derivation[size-1]], Vocab_None};
          score += _lm.wordProb(w, context);        
        

        // subtract out uni

          const VocabIndex context2 [] = {Vocab_None};
          score -= _lm.wordProb(w, context2);        
          //cout << "bonus" << endl;
        }
      }

      if (size >=1 && subder[i].size() > 1 ) {
        const VocabIndex context [] = {w, _word_cache.store[full_derivation[size-1]], Vocab_None};
        score += _lm.wordProb(_word_cache.store[subder[i][1]], context);
        //cout << "\t" << score <<endl;
        //cout << "\t" <<  "Wait TRIGRAM " << subder[i][1] << " " << w << " " << full_derivation[size-1] << " " << full_derivation[size-2] <<endl;
        //cout << "\t" <<  "SCORE " << _lm.wordProb(subder[i][1], context) << endl;
        
        if ( _word_cache.store[subder[i][1]]!= 1 && _word_cache.store[subder[i][1]]!=2) {
          const VocabIndex context2 [] = {w, Vocab_None};
          score -= _lm.wordProb(_word_cache.store[subder[i][1]], context2);        
          //cout << "bonus" << endl;
          }
      }
      //cout << "\t" << size <<endl;
      for (int j=0; j < subder[i].size(); j++) {         
        //cout  << _lm.vocab.getWord(subder[i][j]) << " " ;
        full_derivation.push_back(subder[i][j]);
      }
    }
    //cout << endl;
    score *= LM;
    //cout << score <<endl;
    //cout << full_derivation.size() << endl;;
    int size = full_derivation.size();

    sig.push_back(_word_cache.store[full_derivation[0]]);
    sig.push_back(_word_cache.store[full_derivation[size-1]]);
    assert(size > 0);
    if (size!=1) {
      sig.push_back(_word_cache.store[full_derivation[1]]);
      sig.push_back(_word_cache.store[full_derivation[size-2]]);
    }
  }
  
  Hyp initialize(const ForestNode & node) const {
    assert (node.is_word());
    int original = node.id();
    int w = _word_cache.get_value(node);
    double score = 0.0; 
    VocabIndex context [] = {Vocab_None};
    if (w!=1 && w!=2) {
      score += _lm.wordProb(w, context);
      score *= LM;
    }
    //cout << "WORD " << _word_cache.get_value(node) << " "<< _lm.vocab.getWord(w)<<  endl;
    vector <int> sig;
    sig.push_back(w);
    sig.push_back(w);
    vector <int> der;
    der.push_back(original);
    return Hyp(score, sig, der); 
  }
 private:
  Ngram & _lm;
  const Cache <ForestNode, int> & _word_cache;  
  const Forest  & _forest;  
};

#endif
