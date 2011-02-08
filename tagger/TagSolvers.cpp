#include "TagSolvers.h"

#include "HypergraphAlgorithms.h"
#include <algorithm>


void TaggerDual::solve(double & primal, double & dual, wvector & subgrad,
                           int round) {
  cout << "Round " << round;
  int sent_num=0;
  dual =0;
  primal = 0;
  foreach (const Tagger * ptagger, _taggers) {
    if (_dirty_cache[sent_num] || round == 100) {

      const Tagger & tagger = *ptagger;

      HypergraphAlgorithms ha(tagger);
      EdgeCache * edge_weights = ha.cache_edge_weights(_base_weights);    
      EdgeCache added = _tag_constraints.build_tagger_constraint_vector(sent_num, tagger, *_cur_weights) ;
    
      EdgeCache * final_weights = ha.combine_edge_weights(*edge_weights, added);

      NodeCache  score_memo_table(tagger.num_nodes()); 
    
      NodeBackCache  back_memo_table(tagger.num_nodes());

      _dual_cache[sent_num] = ha.best_path(*final_weights, score_memo_table, back_memo_table);
      
      wvector feature_vec = ha.construct_best_feature_vector(back_memo_table);
    
      _primal_cache[sent_num] = feature_vec.dot(_base_weights);
      HEdges best_edges = ha.construct_best_edges(back_memo_table);

      if (round == 100) {
        cout << endl;
        vector <Tag> res;
        foreach (HEdge edge, best_edges) {
          if (tagger.edge_has_tag(*edge)) {
            Tag d = tagger.edge_to_tag(*edge);
            res.push_back(d);
          }
        }
        sort(res.begin(), res.end());
        cout << "SENT: "; 
        foreach (Tag d, res) {
          cout << d << " ";
        }
        cout << endl;
      }

      _subgrad_cache[sent_num] = _tag_constraints.build_tagger_subgradient(sent_num, tagger, best_edges);
      _dirty_cache[sent_num] = false;
    }
    subgrad += _subgrad_cache[sent_num];
    dual += _dual_cache[sent_num];
    primal += _primal_cache[sent_num];
    sent_num++;
  }
  
}



void ConstrainerDual::solve(double & primal, double & dual, wvector & subgrad, 
                            int round) {
  wvector weights =  (*_cur_weights);
  subgrad = _tag_constraints.solve_hard(weights);
  dual = subgrad.dot( (*_cur_weights));
  primal = 0.0;
}
