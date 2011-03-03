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
#include "MRFConstraints.h"

using namespace std;

/* struct PossibleTag { */
/*   int id; */
/*   int ind; */
/*   int sent_num; */
/*   int group; */
/*   string group_name; */
/*   //double deviance_penalty; */
/*   int training_count; */
/*   int test_count; */
  
/*   int weight_id(POS tag) const { */
/*     return id* Tag::MAX_TAG + tag;  */
/*   } */

/*   bool operator<(const PossibleTag & other) const { */
/*     return id < other.id; */
/*   } */
/* }; */

/* ostream& operator<<(ostream& output, const PossibleTag& ptag); */


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

class TagMrfAligner : public MrfAligner<TagIndex> {
 public:
  void build_from_constraints(string file_name);
};


#endif
