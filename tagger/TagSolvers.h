#ifndef TAGSOLVERS_H
#define TAGSOLVERS_H
#include "Tagger.h"
#include "TagConstraints.h"
#include "DualDecomposition.h"
#include "CorpusSolver.h"

class TaggerDual:public CorpusSolver {
 public:
 TaggerDual(vector < Tagger*> & taggers, 
            const wvector & base_weights,  
            const TagMrfAligner & consistency): 
  CorpusSolver(taggers.size()),
  _taggers(taggers), 
  _base_weights(base_weights),
    _tag_consistency(consistency),
    best_derivations(taggers.size())
      { }
  vector< HNodes> best_derivations;

 protected:
  const vector < Tagger*> & _taggers;
  const wvector & _base_weights;
  const TagMrfAligner & _tag_consistency;

  bool tag_to_lag(int sent_num, const Tag & t, int & lag );

  void solve_one(int sent_num, double & primal, double & dual, wvector & subgrad) ;
  int lag_to_sent_num(int lag) ;

  wvector build_tagger_subgradient(int sent_num, const Tagger & tagger, 
                                               HNodes best_nodes);

  EdgeCache build_tagger_constraint_vector(int sent_num, const Tagger & tagger);
  
};



#endif
