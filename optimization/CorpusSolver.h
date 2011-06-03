#ifndef CORPUSSOLVERS_H
#define CORPUSSOLVERS_H


#include "DualDecomposition.h"
#include "Weights.h"

class CorpusSolver:public DualDecompositionSubproblem {
 public:
 CorpusSolver(int corpus_size):_corpus_size(corpus_size) {
    _cur_weights = new wvector();
    _dirty_cache.resize(corpus_size);
    _subgrad_cache.resize(corpus_size);
    _dual_cache.resize(corpus_size);
    _primal_cache.resize(corpus_size);
    for (int i = 0; i < corpus_size; i++) {
      _dirty_cache[i] = true;
    }
  }

  virtual void solve_one(int sent_num, double & primal, double & dual, wvector & subgrad) = 0; 
  void solve(double & primal, double & dual, wvector &, int);
  void update_weights(const wvector & updates,  
                      wvector * weights, 
                      double mult) {
    _cur_weights = new wvector();
     for (wvector::const_iterator it = weights->begin(); it != weights->end(); it++) {
       (*_cur_weights)[it->first] = mult * it->second; 
     }
     int dirtied = 0;
     for (wvector::const_iterator it = updates.begin(); it != updates.end(); it++) {
       if (it->second != 0.0) {
         int sent =  lag_to_sent_num(it->first); //_tag_consistency._all_constraints[it->first / Tag::MAX_TAG].sent_num;
         //cout << "Update " << sent <<endl;
         //assert(false);
         if (!_dirty_cache[sent]) {
           dirtied ++;
         }
         _dirty_cache[sent] = true;
         //constraints[group]->show_derivation(best_derivations[group]);
         
       }
     }
     cout << "dirtied: " << dirtied << endl;
  }
 protected:
  virtual int lag_to_sent_num(int lag) = 0;
  wvector * _cur_weights;
  vector <wvector> _subgrad_cache;
  vector <double> _primal_cache;
  vector <double> _dual_cache;
  vector <bool> _dirty_cache;  
  int _corpus_size;
};



#endif
