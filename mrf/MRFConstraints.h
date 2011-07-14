#ifndef MRFCONSTRAINTS_H
#define MRFCONSTRAINTS_H

#include "MRF.h"
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <map>

using namespace std;

struct MrfIndex {
MrfIndex(){}
MrfIndex(const MrfIndex & other ):group(other.group), node(other.node), state(other.state) {}
MrfIndex(int group_, int node_, int state_): group(group_), node(node_), state(state_)  {}
  int group, node, state ;

  bool operator<(const MrfIndex & other) const {
    if (group != other.group) {
      return group < other.group;
    }
    if (node != other.node) {
      return node < other.node;
    }
    if (state != other.state) {
      return state < other.state;
    }
    return false;
  }
};



// Class for building an mrf and aligning it with the tag sequence
template <class Other>
class MrfAligner {
 public:
  bool align(const Other & other_ind, MrfIndex & mrf_ind) const {
    typename map < Other , MrfIndex>::const_iterator iter = alignment.find(other_ind);
    if (iter == alignment.end()) {
      return false;
    } else {
      mrf_ind = iter->second;
      return true;
    }
  }

  bool other_aligned(const Other & other_ind) const {
    return other_id_map.find(other_ind) != other_id_map.end();
  }

  int other_id(const Other & other_ind) const {
    return other_id_map.find(other_ind)->second;
  }

  bool cons_aligned(const MrfIndex & mrf_ind) const {
    return cons_id_map.find(mrf_ind) != cons_id_map.end();
  }

  int cons_id(const MrfIndex & mrf_ind) const {
    return cons_id_map.find(mrf_ind)->second;
  }
  
  MrfIndex id_cons(int id) const {
    return id_cons_map.find(id)->second;
  }

  Other id_other(int id) const {
    return id_other_map.find(id)->second;
  }

  
 protected:
  map <Other, MrfIndex> alignment;
  map <Other, int> other_id_map;
  map <MrfIndex, int> cons_id_map;
  map <int, Other> id_other_map;
  map <int, MrfIndex> id_cons_map;
  
};




#endif
