
#ifndef BESTHYP_H_
#define BESTHYP_H_

#include <assert.h>
#include <map>
#include <vector>
#include "Hypergraph.h"
#include "Hypothesis.h"
#include "HypergraphAlgorithms.h"
using namespace std;
namespace Scarab {
  namespace HG {

class BestHyp {
 private:
  
  map <int, int> index_by_id;
  map <int, vector<int> > index_by_right;
  map <int, vector<int> > index_by_left;
  
  // have I seen a new score


  //map <int, vector<int> > index_by_right;
 public: 
  vector <Hypothesis *> hyps;
  vector <double> scores;

  bool has_new;
  BestHyp() {
    has_new = false;
    clear();
  }

  /*  inline void prune(double prune_to) {    
    vector <Hypothesis *> best; 
    vector <double> best_scores; 
    //for (int k =0; k < prune_to; k++) {
    bool first = true;
    while (true) {
      double best_score = INF;
      int pos = -1;
      for (int i=0; i < scores.size() ;i++) {
        if (scores[i] < best_score) {
          best_score = scores[i];
          pos = i;
        }
      }
      if (!first && (best_score - best_scores[0]) > prune_to) break;
      if (pos == -1) break; 
      best.push_back(hyps[pos]);
      best_scores.push_back(best_score);
      scores[pos] = INF;
      first =false;
    }
    
    // reset 
    index_by_id.clear();
    index_by_right.clear();
    index_by_left.clear();
    hyps = best;
    scores = best_scores;
    
    for (int i=0; i < hyps.size();i++) {
      index_by_id[hyps[i].id()] =i;
      index_by_right[hyps[i].right()].push_back(i);
      index_by_left[hyps[i].left()].push_back(i);
    } 
    }*/

  inline int size()  const{
    return hyps.size();
  }

  inline void clear() {
    hyps.clear();
    scores.clear();
    index_by_right.clear();
    index_by_left.clear();
    index_by_id.clear();
  }

  

  inline const Hypothesis & get_hyp(int i) const {
    return *hyps[i];
  }

  inline double get_score(int i) const {
    return scores[i];
  }

  inline double get_score_by_id(int id ) const {
    map <int, int >::const_iterator check = 
      index_by_id.find(id);
    assert(check != index_by_id.end());
    return scores[check->second];
  }

  inline const Hypothesis & get_hyp_by_id(int id) const {
    map <int, int >::const_iterator check = 
      index_by_id.find(id);
    assert(check != index_by_id.end());
    return *hyps[check->second];
  }


  inline bool has_id(int id) const {
    map <int, int >::const_iterator check = 
      index_by_id.find(id);
    return check != index_by_id.end();
  } 

  inline vector<int> join(const Hypothesis & other) const {
    map <int, vector<int> >::const_iterator check = 
      index_by_right.find(other.left());
    if (check != index_by_right.end()) {
      return check->second;
    }
    vector <int > result;
    return result;
  }


  inline vector<int> join_back(const Hypothesis & other) const {
    map <int, vector<int> >::const_iterator check = 
      index_by_left.find(other.right());
    if (check != index_by_left.end()) {
      return check->second;
    }
    vector <int > result;
    return result;
  }

  inline bool try_set_hyp(Hypothesis * hyp , double score) {
    
    bool set = false;
    map <int, int >::const_iterator check = 
      index_by_id.find(hyp->id());
    if (check == index_by_id.end()) {
      hyps.push_back(hyp);
      scores.push_back(score);
      // update indexes
      index_by_id[hyp->id()] = hyps.size()-1;
      index_by_right[hyp->right()].push_back(hyps.size()-1);
      index_by_left[hyp->left()].push_back(hyps.size()-1);
      set = true;
      //has_new = has_new || is_new;
    } else {
      int internal_ind = index_by_id[hyp->id()];
      double old_score = scores[internal_ind];
      if (score < old_score) {
        hyps[internal_ind] = hyp;
        scores[internal_ind] = score;
        set = true;
        //has_new = has_new || is_new;
      }
    }
    return set;
  }
};

void extract_back_pointers(const Hypernode & node, 
                           const Hypothesis & best_hyp, 
                           const Cache <Hypernode, BestHyp *> &memo_table, 
                           NodeBackCache & back_pointers);

  }}
#endif
