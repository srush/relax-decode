#ifndef NGRAMCACHE_H_
#define NGRAMCACHE_H_

#include <Ngram.h>
#include <Prob.h>



class NgramCache : public Ngram {
 public:
  NgramCache(Vocab & v, int i) 
    :Ngram(v, i) {}


    bool hasNext(const VocabIndex next) {
      return (bool) trieNode->findTrie(next);
    }
    

    LogP wordProbPrimeCache(VocabIndex word, const VocabIndex *context);
    LogP wordProbFromCache(VocabIndex word, const VocabIndex *context);
 private:
    LogP logp;
    LogP bow;
    unsigned found;
    
    BOtrie *trieNode;
    unsigned i;

};



#endif 
