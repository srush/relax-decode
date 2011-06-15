#include "ParseSolvers.h"

#include "HypergraphAlgorithms.h"
#include <algorithm>

bool ParserDual::dep_to_lag(int sent_num, const Dependency & t, int & lag ) {
  ParseIndex parse_ind(sent_num, t.mod, t.head);
  if (!_parse_consistency.other_aligned(parse_ind)) return false;
  lag= _parse_consistency.other_id(parse_ind);
  return true;
}

int ParserDual::lag_to_sent_num(int  lag )  {
  return _parse_consistency.id_other(lag).sent_num;
}


wvector ParserDual::build_parser_subgradient(int sent_num, const DepParser & parser, 
                                             HEdges best_edges) {
  wvector ret;
  foreach (HEdge e, best_edges) {
    if (parser.edge_has_dep(*e)) {
      const Dependency & t = parser.edge_to_dep(*e);
      int lag;
      if (dep_to_lag(sent_num, t, lag)) {
        ret[lag] +=1;
      }      
    }
  }
  return ret;
}


EdgeCache ParserDual::build_parser_constraint_vector(int sent_num, const DepParser & parser) {
  EdgeCache ret(parser.num_edges());
  foreach (const Dependency & t, parser.dependencies()) {
    int lag;
    if (!dep_to_lag(sent_num, t, lag)) continue;

    HEdges edges = parser.dep_to_edge(t);

    foreach (HEdge edge, edges) { 
      if (!ret.has_key(*edge)) {
        ret.set_value(*edge, 0.0);
      }
      ret.set_value(*edge, ret.get(*edge) + ((*_cur_weights)[lag]));
    }
  } 
  return ret;
} 

void ParserDual::solve_one(int sent_num, double & primal, double & dual, wvector & subgrad) {
  const DepParser & parser = *_parsers[sent_num];
  
  HypergraphAlgorithms ha(parser);

  EdgeCache * edge_weights = ha.cache_edge_weights(_base_weights);    

  EdgeCache added = build_parser_constraint_vector(sent_num, parser) ;
  
  EdgeCache * final_weights = ha.combine_edge_weights(*edge_weights, added);
  
  NodeCache score_memo_table(parser.num_nodes()); 
      
  NodeBackCache  back_memo_table(parser.num_nodes());
      

  dual = ha.best_path(*final_weights, score_memo_table, back_memo_table);
  wvector feature_vec = ha.construct_best_feature_vector(back_memo_table);
    
  HEdges best_edges = ha.construct_best_edges(back_memo_table);

  primal = feature_vec.dot(_base_weights);
  subgrad = build_parser_subgradient(sent_num, parser, best_edges);
  
  best_derivations[sent_num] = best_edges;
  
  cout << endl << "REDO: " <<sent_num << " ";
  parser.show_derivation(best_derivations[sent_num]);  
  
}



