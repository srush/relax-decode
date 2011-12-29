// #include "MRFSolvers.h"
// #include "MRF.h"
// #include "MRFConstraints.h"

// template <class Other>
// bool ConstrainerDual<Other>::assign_to_lag(int sent_num, const NodeAssignment & a, int & lag) {
//   MrfIndex mrf_ind(sent_num, a.node_id, a.s.id()); 
//   if (!_consistency.cons_aligned(mrf_ind)) return false;
//   lag = _consistency.cons_id(mrf_ind);
//   return true;
// }

// template <class Other>
// wvector ConstrainerDual<Other>::build_mrf_subgradient(int sent_num, 
//                                                       const MRFHypergraph & mrf, 
//                                                       HNodes best_nodes) {
//   wvector ret;
//   foreach (HNode n, best_nodes) {
//     if (mrf.node_has_assignment(n)) {
//       const NodeAssignment & a = mrf.assignment_from_node(n);
//       int lag;
//       if (assign_to_lag(sent_num, a, lag)) {
//         ret[lag] +=1;
//       }      
//     }
//   }
//   return ret;
// }
// template <class Other>
// EdgeCache ConstrainerDual<Other>::build_mrf_constraint_vector(int sent_num, 
//                                                               const MRFHypergraph & mrf) {
//   EdgeCache ret(mrf.num_edges());

//   foreach (Node n, mrf.nodes()) {
//     foreach (const State & s, mrf.mrf().states(*n)) {
      
//       int lag;
//       NodeAssignment assign = mrf.mrf().make_assignment(*n, s);
//       if (!assign_to_lag(sent_num, assign, lag)) continue;

      
//       HNode n = mrf.node_from_assignment(assign);

//       foreach (HEdge edge, n->edges()) {
//         if (!ret.has_key(*edge)) {
//           ret.set_value(*edge, 0.0);
//         }
//         ret.set_value(*edge, ret.get(*edge) + (*_cur_weights)[lag]);
//       }
//     }
//   } 
//   return ret;
// } 

// template <class Other>
// void ConstrainerDual<Other>::solve(double & primal, double & dual, 
//                                    wvector & subgrad, int round) {
//   wvector weights =  (*_cur_weights);

//   int sent_num=0;
//   dual =0;
//   primal = 0;
//   foreach (const MRF * pmrf, constraints) {
//     const MRF & mrf = *pmrf;
//     MRFHypergraph hypergraph = MRFHypergraph::from_mrf(mrf);

//     HypergraphAlgorithms ha(hypergraph);   
//     NodeCache  score_memo_table(hypergraph.num_nodes()); 
    
//     NodeBackCache  back_memo_table(hypergraph.num_nodes());
//     EdgeCache * edge_weights = ha.cache_edge_weights(_base_weights);
//     EdgeCache added = build_mrf_constraint_vector(sent_num, mrf) ;
//     EdgeCache * final_weights = ha.combine_edge_weights(*edge_weights, added);

//     ha.best_path(*final_weights, score_memo_table, back_memo_table);

//     //wvector feature_vec = ha.construct_best_feature_vector(back_memo_table);
//       HNodes best_nodes = ha.construct_best_node_order(back_memo_table);
//       HEdges best_edges = ha.construct_best_edges(back_memo_table);

//     build_mrf_subgradient(sent_num, mrf, best_edges);
//   }
// }
