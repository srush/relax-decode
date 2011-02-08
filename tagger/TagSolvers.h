#ifndef TAGSOLVERS_H
#define TAGSOLVERS_H
#include "Tagger.h"
#include "TagConstraints.h"
#include "DualDecomposition.h"

class TaggerDual:public DualDecompositionSubproblem {
 public:
 TaggerDual(vector <const Tagger*> & taggers, const wvector & base_weights,  const TagConstraints & cons): 
    _taggers(taggers), 
      _base_weights(base_weights),
      _tag_constraints(cons){
      _cur_weights = new wvector();
      _dirty_cache.resize(_taggers.size());
      _subgrad_cache.resize(_taggers.size());
      _dual_cache.resize(_taggers.size());
      _primal_cache.resize(_taggers.size());
      for (int i = 0; i < taggers.size(); i++) {
        _dirty_cache[i] = true;
      }
    }

  void solve(double & primal, double & dual, wvector &, 
                      int) ;
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
         int sent= _tag_constraints._all_constraints[it->first / Tag::MAX_TAG].sent_num;
         _dirty_cache[sent] = true;
         dirtied ++;
       }
     }
     cout << "dirtied: " << dirtied << endl;
  }
 protected:
  const vector <const Tagger*> & _taggers;
  const wvector & _base_weights;
  const TagConstraints & _tag_constraints;
  wvector * _cur_weights;
  vector <wvector> _subgrad_cache;
  vector <double> _primal_cache;
  vector <double> _dual_cache;
  vector <bool> _dirty_cache;

};

class ConstrainerDual:public DualDecompositionSubproblem {
 public:
  ConstrainerDual(const TagConstraints & cons): _tag_constraints(cons){
    _cur_weights = new wvector();
  };
  void solve(double & primal, 
              double & dual, wvector &, 
              int) ;

   void update_weights(const wvector & updates,  
                       wvector * weights, 
                       double mult) {
     _cur_weights = new wvector();
     for (wvector::const_iterator it = weights->begin(); it != weights->end(); it++) {
       (*_cur_weights)[it->first] = mult*it->second; 
     }
   }
 protected:
   const TagConstraints & _tag_constraints;
   wvector * _cur_weights; 

};

#endif
