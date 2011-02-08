#ifndef TAGCONSTRAINTS_H
#define TAGCONSTRAINTS_H

#include <Tagger.h>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include "Weights.h"
#include "EdgeCache.h"
#include "HypergraphAlgorithms.h"

#define PENALTY 1000
using namespace std;

struct PossibleTag {
  int id;
  int ind;
  int sent_num;
  int group;
  string group_name;
  int training_count;
  int test_count;
  
  int weight_id(POS tag) const {
    return id* Tag::MAX_TAG + tag; 
  }

  bool operator<(const PossibleTag & other) const {
    return id < other.id;
  }
};

ostream& operator<<(ostream& output, const PossibleTag& ptag);

class ConstraintGroup {
 public:
  vector <PossibleTag> group;

  wvector solve_hard(wvector & weights) const;
};

class TagConstraints {
 public:
  vector <ConstraintGroup > _constraint_struct;
  set<int> groups;
  
  vector <vector <PossibleTag> > _constrained_words;
  vector <PossibleTag > _all_constraints;
  TagConstraints(int num_tags) : _num_tags(num_tags) {}

  /**
   * Given an sent_num, the tagger, and a weight vector (indexed on constrained words)
   * produce a weight vector indexed on edges.
   */
  EdgeCache build_tagger_constraint_vector(int sent_num, const Tagger & tagger,  wvector & orig_weights ) const;

  wvector build_tagger_subgradient(int sent_num, const Tagger & tagger, const vector<const Hyperedge *> used_edges) const ;


  void read_from_file(string file_name) ;
  int _num_tags;

  wvector solve_hard( wvector & model) const; 
 private:

};





#endif
