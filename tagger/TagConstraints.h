#ifndef TAGCONSTRAINTS_H
#define TAGCONSTRAINTS_H

#include <Tagger.h>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include "Weights.h"
#include "EdgeCache.h"
//#include "PottsModel.h"
#include "MRF.h"
#include "HypergraphAlgorithms.h"

using namespace std;

struct PossibleTag {
  int id;
  int ind;
  int sent_num;
  int group;
  string group_name;
  //double deviance_penalty;
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

/* class ConstraintGroup { */
/*  public: */
/*   vector <PossibleTag> group; */

/*   wvector solve_hard(wvector & weights) const; */
/* }; */

/* class TagConstraints { */
/*  public: */
/*   vector <ConstraintGroup > _constraint_struct; */
/*   set<int> groups; */
  
/*   vector <vector <PossibleTag> > _constrained_words; */
/*   vector <PossibleTag > _all_constraints; */
/*   TagConstraints(int num_tags) : _num_tags(num_tags) {} */

/*   /\** */
/*    * Given an sent_num, the tagger, and a weight vector (indexed on constrained words) */
/*    * produce a weight vector indexed on edges. */
/*    *\/ */
/*   EdgeCache build_tagger_constraint_vector(int sent_num, const Tagger & tagger,  wvector & orig_weights ) const; */

/*   wvector build_tagger_subgradient(int sent_num, const Tagger & tagger, const vector<const Hyperedge *> used_edges) const ; */


/*   void read_from_file(string file_name) ; */
/*   int _num_tags; */

/*   wvector solve_hard( wvector & model) const;  */
/*  private: */
/* }; */



struct TagIndex {
TagIndex() {}
TagIndex(int sent_num_, int ind_, int tag_): sent_num(sent_num_), ind(ind_), tag(tag_) {}
  int sent_num;
  int ind;
  POS tag;
  bool operator<(const TagIndex & other) const {
    if (sent_num != other.sent_num) {
      return sent_num < other.sent_num;
    }
    if (ind != other.ind) {
      return ind < other.ind;
    }
    if (tag != other.tag) {
      return tag < other.tag;
    }
    return false;
  }
};


struct MrfIndex {
MrfIndex(){}
MrfIndex(int group_, int node_, int state_): group(group_), node(node_), state(state_)  {}
  int group, node, state ;
};

// Class for building an mrf and aligning it with the tag sequence
class TagMrfAligner {
 public:

  void build_from_constraints(string file_name);

  bool align(TagIndex tag_ind, MrfIndex & mrf_ind) {
    map <TagIndex, MrfIndex>::iterator iter = alignment.find(tag_ind);
    if (iter == alignment.end()) {
      return false;
    } else {
      mrf_ind = iter->second;
      return true;
    }
  }

  vector <MRF *> mrf_models;
  vector <vector <TagIndex> > tag_constraints;
 private:
  map <TagIndex, MrfIndex> alignment;
};




#endif
