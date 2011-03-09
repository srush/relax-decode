#include "TagSolvers.h"

#include "HypergraphAlgorithms.h"
#include <algorithm>

bool TaggerDual::tag_to_lag(int sent_num, const Tag & t, int & lag ) {
  TagIndex tag_ind(sent_num, t.ind, t.tag);
  if (!_tag_consistency.other_aligned(tag_ind)) return false;
  lag= _tag_consistency.other_id(tag_ind);
  return true;
}

int TaggerDual::lag_to_sent_num(int lag) {
  return _tag_consistency.id_other(lag).sent_num;
}


wvector TaggerDual::build_tagger_subgradient(int sent_num, const Tagger & tagger, 
                                             HNodes best_nodes) {
  wvector ret;
  foreach (HNode n, best_nodes) {
    if (tagger.node_has_tag(*n)) {
      const Tag & t = tagger.node_to_tag(*n);
      int lag;
      if (tag_to_lag(sent_num, t, lag)) {
        ret[lag] +=1;
      }      
    }
  }
  return ret;
}


EdgeCache TaggerDual::build_tagger_constraint_vector(int sent_num, const Tagger & tagger) {
  EdgeCache ret(tagger.num_edges());
  foreach (const Tag & t, tagger.tags()) {
    int lag;
    if (!tag_to_lag(sent_num, t, lag)) continue;
    if (!tagger.tag_has_node(t)) continue;

    HNodes nodes = tagger.tag_to_node(t);

    foreach (HNode node, nodes) {
      foreach (HEdge edge, node->edges()) {
        if (!ret.has_key(*edge)) {
          ret.set_value(*edge, 0.0);
        }
        ret.set_value(*edge, ret.get(*edge) + ((*_cur_weights)[lag]));
      }
    }
  } 
  return ret;
} 

void TaggerDual::solve_one(int sent_num, double & primal, double & dual, wvector & subgrad) {
  const Tagger & tagger = *_taggers[sent_num];
  
  HypergraphAlgorithms ha(tagger);

  EdgeCache * edge_weights = ha.cache_edge_weights(_base_weights);    

  EdgeCache added = build_tagger_constraint_vector(sent_num, tagger) ;
  
  EdgeCache * final_weights = ha.combine_edge_weights(*edge_weights, added);
  
  NodeCache score_memo_table(tagger.num_nodes()); 
      
  NodeBackCache  back_memo_table(tagger.num_nodes());

  dual = ha.best_path(*final_weights, score_memo_table, back_memo_table);
      
  wvector feature_vec = ha.construct_best_feature_vector(back_memo_table);
    
  HNodes best_nodes = ha.construct_best_node_order(back_memo_table);


  primal = feature_vec.dot(_base_weights);
  subgrad = build_tagger_subgradient(sent_num, tagger, best_nodes);

  best_derivations[sent_num] = best_nodes;  
}



