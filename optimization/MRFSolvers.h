#ifndef MRFSOLVERS_H
#define MRFSOLVERS_H

#include "DualDecomposition.h"
#include "MRFHypergraph.h"
#include "HypergraphAlgorithms.h"
#include "MRFConstraints.h"
#include "EdgeCache.h"

#include "MRF.h"


template <class Other>
class ConstrainerDual:public DualDecompositionSubproblem {
 public:
   ConstrainerDual(const vector <MRF*> & constraints, 
                   const wvector & base_weights,
                   const MrfAligner<Other> & cons): 
  constraints(constraints), 
    best_derivations(constraints.size()), 
    _consistency(cons), 
    _base_weights(base_weights)
    {
    _cur_weights = new wvector();
    _hypergraphs.resize(constraints.size());

    // cache
    _dirty_cache.resize(constraints.size());
    _subgrad_cache.resize(constraints.size());
    _dual_cache.resize(constraints.size());
    _primal_cache.resize(constraints.size());
    for (int i = 0; i < constraints.size(); i++) {
      _dirty_cache[i] = true;
    }

    _is_first = true;
  };

  

  void solve(double & primal, 
              double & dual, wvector &, 
              int);

  void update_weights(const wvector & updates,  
                      wvector * weights, 
                      double mult) {
    _cur_weights = new wvector();

    for (wvector::const_iterator it = weights->begin(); it != weights->end(); it++) {
      (*_cur_weights)[it->first] = mult*it->second; 
    }

    int dirtied = 0;
    for (wvector::const_iterator it = updates.begin(); it != updates.end(); it++) {
      if (it->second != 0.0) {
        MrfIndex index =  lag_to_assign(it->first); 
        int group = index.group;
        if (!_dirty_cache[group]) {
          dirtied ++;
        }
        _dirty_cache[group] = true;
        //cout << "Update " << index.group <<endl;
      }
    }
    cout << "dirtied: " << dirtied << endl;

/*     for (int i = 0; i < constraints.size(); i++) { */
/*       _dirty_cache[i] = true; */
/*     } */

  }

  vector < vector< NodeAssignment> > best_derivations;
 protected:
  bool _is_first;
  const MrfAligner<Other> & _consistency;
  const vector < MRF*> & constraints;
  wvector * _cur_weights; 
  vector <MRFHypergraph *> _hypergraphs;

  const wvector & _base_weights;
  bool assign_to_lag(int group_num, const NodeAssignment & a, int & lag);
  MrfIndex lag_to_assign(int lag);
  wvector build_mrf_subgradient(int group_num, 
                                const MRFHypergraph & mrf, 
                                HNodes best_nodes);
  EdgeCache build_mrf_constraint_vector(int group_num, 
                                        const MRFHypergraph & mrf);
  
  // cache
  vector <wvector> _subgrad_cache;
  vector <double> _primal_cache;
  vector <double> _dual_cache;
  vector <bool> _dirty_cache;  

  
};


template <class Other>
bool ConstrainerDual<Other>::assign_to_lag(int group_num, const NodeAssignment & a, int & lag) {
  MrfIndex mrf_ind(group_num, a.node_id, a.s.id()); 
  if (!_consistency.cons_aligned(mrf_ind)) return false;
  lag = _consistency.cons_id(mrf_ind);
  return true;
}

template <class Other>
MrfIndex ConstrainerDual<Other>::lag_to_assign(int lag) {
  return _consistency.id_cons(lag);
}

template <class Other>
wvector ConstrainerDual<Other>::build_mrf_subgradient(int group_num, 
                                                      const MRFHypergraph & mrf, 
                                                      HNodes best_nodes) {
  wvector ret;
  foreach (HNode n, best_nodes) {
    if (mrf.node_has_assignment(n)) {
      const NodeAssignment & a = mrf.assignment_from_node(n);
      int lag;
      if (assign_to_lag(group_num, a, lag)) {
        ret[lag] +=1;
      }      
    }
  }
  return ret;
}
template <class Other>
EdgeCache ConstrainerDual<Other>::build_mrf_constraint_vector(int group_num, 
                                                               const MRFHypergraph & mrf) {
  EdgeCache ret(mrf.num_edges());

  foreach (Node n, mrf.mrf().graph().nodes()) {
    foreach (const State & s, mrf.mrf().states(*n)) {
      
      int lag;
      NodeAssignment assign = mrf.mrf().make_assignment(*n, s);
      if (!assign_to_lag(group_num, assign, lag)) continue;

      
      HNode hnode = mrf.node_from_assignment(assign);

      foreach (HEdge edge, hnode->edges()) {
        if (!ret.has_key(*edge)) {
          ret.set_value(*edge, 0.0);
        }
        ret.set_value(*edge, ret.get(*edge) + (*_cur_weights)[lag]);
      }
    }
  } 
  return ret;
} 

template <class Other>
void ConstrainerDual<Other>::solve(double & primal, double & dual, 
                                   wvector & subgrad, int round) {
  wvector weights =  (*_cur_weights);
  cout << " constrainer " << endl;
  //int sent_num=0;
  dual =0;
  primal = 0;
  int group = 0;

  clock_t s = clock();
  cout <<"Num constraints " <<  constraints.size() << endl;
  
  foreach (const MRF * pmrf, constraints) {
    const MRF & mrf = *pmrf;

    if (_is_first) {
      //cout << " Build hypergraph " << endl;
      _hypergraphs[group] = MRFHypergraph::from_mrf(mrf);
    }
    if (_dirty_cache[group]) {
      MRFHypergraph * hypergraph = _hypergraphs[group];

      HypergraphAlgorithms ha(*hypergraph);   
      NodeCache  score_memo_table(hypergraph->num_nodes()); 
    
      NodeBackCache  back_memo_table(hypergraph->num_nodes());
      EdgeCache * edge_weights = ha.cache_edge_weights(_base_weights);
      EdgeCache added = build_mrf_constraint_vector(group, *hypergraph) ;
      EdgeCache * final_weights = ha.combine_edge_weights(*edge_weights, added);

      _dual_cache[group] = ha.best_path(*final_weights, score_memo_table, back_memo_table);

      //wvector feature_vec = ha.construct_best_feature_vector(back_memo_table);
      HNodes best_nodes = ha.construct_best_node_order(back_memo_table);
      best_derivations[group] = hypergraph->derivation_to_assignments(best_nodes);
      HEdges best_edges = ha.construct_best_edges(back_memo_table);
      wvector feature_vec = ha.construct_best_feature_vector(back_memo_table);
      _primal_cache[group] = feature_vec.dot(_base_weights);
      wvector local_subgrad = build_mrf_subgradient(group, *hypergraph, best_nodes);
    
      _subgrad_cache[group] = local_subgrad;
      _dirty_cache[group] = false;
    }

    dual += _dual_cache[group];
    primal += _primal_cache[group];
    subgrad += _subgrad_cache[group];
    group++;
  }

/*   for (wvector::const_iterator it = subgrad.begin(); it != subgrad.end(); it++) { */
/*     if (it->second !=0.0) {  */
/*       MrfIndex mrf_index = lag_to_assign(it->first); */
/*       cout << "Subgrad " << it->first << " " << mrf_index.group << " " <<  */
/*         mrf_index.node << " " << mrf_index.state << endl; */
/*     } */
/*   } */

  _is_first = false;
  cout << "MRF dual: " << dual << endl;
  cout << "MRF primal: " << primal << endl;
  cout << "Clock " << double(Clock::diffclock( clock(), s)) <<endl;
}

#endif
