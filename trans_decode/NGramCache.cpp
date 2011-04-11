#include "NGramCache.h"
#include "../CommandLine.h"

DEFINE_string(lm_file, "", "file with the sri language model");
static const bool lm_dummy =  RegisterFlagValidator(&FLAGS_lm_file, &ValidateFile);

DEFINE_double(lm_weight, 0.0, "lm weight value"); // was 4
static const bool lm_weight_dummy = RegisterFlagValidator(&FLAGS_lm_weight, &ValidateNum);


// assume we are at a point in the trie, just look one step more
LogP NgramCache::wordProbFromCache(VocabIndex word, const VocabIndex *context) {

  LogP cur_logp = logp;
  LogP cur_bow = bow;

  BOtrie *next = trieNode->findTrie(context[i]);
  if (next) {
    /*
     * Accumulate backoff weights 
     */
    cur_bow += next->value().bow;
    //trieNode = next;
  

    LogP *prob = next->value().probs.find(word);  
    if (prob) {
      /*
       * If a probability is found at this level record it as the 
       * most specific one found so far and reset the backoff weight.
       */
      cur_logp = *prob;
      cur_bow = LogP_One;
      //cur_found = i + 1;
    } 
  }  
    
  return cur_logp + cur_bow;
}


LogP NgramCache::wordProbPrimeCache(VocabIndex word, const VocabIndex *context) {
  
  // Reset to original values
  logp = LogP_Zero;
  bow = LogP_One;
  found = 0;
  
  trieNode = &contexts;
  i = 0;

  do {
    LogP *prob = trieNode->value().probs.find(word);
    
    if (prob) {
      /*
       * If a probability is found at this level record it as the 
       * most specific one found so far and reset the backoff weight.
       */
      logp = *prob;
      bow = LogP_One;
      found = i + 1;
    } 
    
    if  ( context[i] == Vocab_None) break;
    
    BOtrie *next = trieNode->findTrie(context[i]);
    if (next) {
      /*
       * Accumulate backoff weights 
       */
      bow += next->value().bow;
      trieNode = next;
      i ++;
    } else {
      break;
    }
  } while (1);

  return logp + bow;
}

NgramCache * load_ngram_cache(const char * filename) {
  Vocab * all = new Vocab();
  all->unkIsWord() = true;
  NgramCache * lm = new NgramCache(*all, 3);

  File file(filename, "r", 0);    
  if (!lm->read(file, false)) {
    assert(false);
  }
  return lm;
}

NgramCache * cmd_lm()  {
  return load_ngram_cache(FLAGS_lm_file.c_str());
}

double lm_weight() {
  return FLAGS_lm_weight;
}
