#ifndef PARSECONSTRAINTS_H
#define PARSECONSTRAINTS_H
#include <DepParser.h>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include "Weights.h"
#include "EdgeCache.h"
//#include "PottsModel.h"
#include "MRF.h"
#include "MRFConstraints.h"
#include "HypergraphAlgorithms.h"


struct ParseIndex {
ParseIndex() {}
ParseIndex(int sent_num_, int ind_, int head_): sent_num(sent_num_), ind(ind_), head(head_) {}
  int sent_num;
  int ind;
  int head;
  bool operator<(const ParseIndex & other) const {
    if (sent_num != other.sent_num) {
      return sent_num < other.sent_num;
    }
    if (ind != other.ind) {
      return ind < other.ind;
    }
    if (head != other.head) {
      return head < other.head;
    }
    return false;
  }
};



// Class for building an mrf and aligning it with the tag sequence
class ParseMrfAligner: public MrfAligner<ParseIndex> {
 public:
  void build_from_constraints(string file_name);
};

#endif
