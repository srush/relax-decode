#include "ExtendCKY.h"
#include <cmath>
//typedef StoreCache <Hypothesis, double> BestHyp;
#include <iostream>
#include "HypergraphAlgorithms.h"
#include <limits>
#include "../common.h"
using namespace std;

#define DEBUG 0

namespace Scarab{
  namespace HG{

void ExtendCKY::forward_edge(const Hyperedge & edge,  vector <BestHyp> & best_edge_hypotheses) {
  // viterbi to find best edge
  for (uint j=0; j < edge.num_nodes(); j++ ) {
  //foreach (const Hypernode * sub_node, edge.tail_nodes()) {
    
    const Hypernode * sub_node = &edge.tail_node(j);
    
    // memoize the best path 
    node_best_path(*sub_node);

    // 
    const BestHyp & local_best = _memo_table.get(*sub_node);
    
    if (j ==0) {
      // at leftmost, all are valid
      for (int iter = 0; iter< local_best.size(); iter++) {
                            
        const Hypothesis & hyp = local_best.get_hyp(iter); 
        double score = local_best.get_score(iter);
        Hypothesis * new_hyp = 
          new Hypothesis(hyp.hook, hyp.right_side, &edge);
        new_hyp->prev_hyp.push_back(hyp.id());
        
        bool worked = best_edge_hypotheses[0].try_set_hyp(new_hyp,  score);
        assert(worked);
        //assert(best_edge_hypotheses->has_key( new_hyp.id()));
      }
    } else {
      // match hook sig at left 
          
      for (int iter = 0; iter< local_best.size(); iter++) {
        //if (!local_best.has_key(iter)) continue;           
        const Hypothesis & hyp1 = local_best.get_hyp(iter); 
        double score1 = local_best.get_score(iter);
        
        BestHyp & last_best = best_edge_hypotheses[j-1];
        vector <int> pos = last_best.join(hyp1);
        
        for (uint iter2 = 0; iter2< pos.size(); iter2++) {
          
          const Hypothesis & hyp2 = last_best.get_hyp(pos[iter2]); 
          double score2 = last_best.get_score(pos[iter2]);
   
          assert(hyp2.match(hyp1));
          
          Hypothesis * join = new Hypothesis(_controller.dim());
                    
          join->back_edge = &edge;
          assert(hyp2.prev_hyp.size() == j);
          double join_score = score1 + score2 +
            _controller.combine(hyp2, hyp1, *join);
          assert(join->prev_hyp.size() == j+1);

          /*cout << "For Combine " << endl;
          show_hyp(hyp1);
          show_hyp(hyp2);
          cout << "For " <<  join_score <<endl; 
          show_hyp(join);
          */
             
          best_edge_hypotheses[j].try_set_hyp(join, join_score);
        }
      }
    }
  }
}


void ExtendCKY::backward_edge(const Hyperedge & edge,  vector <BestHyp> & best_edge_back_hypotheses) {
  // viterbi to find best edge
  int last= edge.num_nodes()-1;
  for (int j=last; j >= 0; j-- ) {

    const Hypernode & sub_node = edge.tail_node(j);

    // memoize the best path 
    node_best_path(sub_node);

    const BestHyp & local_best = _memo_table.store[sub_node.id()];
    
    if (j == last) {
      // at rightmost, all are valid
      for (int iter = 0; iter< local_best.size(); iter++) {
                            
        const Hypothesis & hyp = local_best.get_hyp(iter); 
        double score = local_best.get_score(iter);
        Hypothesis * new_hyp = new Hypothesis(hyp.hook, hyp.right_side, &edge);
        new_hyp->prev_hyp.push_back(hyp.id());
        
        bool worked = best_edge_back_hypotheses[j].try_set_hyp(new_hyp,  score);
        assert(worked);
        //assert(best_edge_hypotheses->has_key( new_hyp.id()));
      }
    } else {
      // match hook sig at right
          
      for (int iter = 0; iter< local_best.size(); iter++) {
        //if (!local_best.has_key(iter)) continue;           
        const Hypothesis & hyp1 = local_best.get_hyp(iter); 
        double score1 = local_best.get_score(iter);
        
        BestHyp & last_best = best_edge_back_hypotheses[j+1];
        vector <int> pos = last_best.join_back(hyp1);
        
        for (uint iter2 = 0; iter2< pos.size(); iter2++) {
          
          const Hypothesis & hyp2 = last_best.get_hyp(pos[iter2]); 
          double score2 = last_best.get_score(pos[iter2]);
   
          assert(hyp1.match(hyp2));
          
          Hypothesis * join = new Hypothesis();
                    
          join->back_edge = &edge;
          //assert(hyp2.prev_hyp.size() == j);
          double join_score = score1 + score2 +
            _controller.combine_back(hyp2, hyp1, *join);
          //assert(join.prev_hyp.size() == j+1);
          /*cout << "Back Combine " << endl;
          show_hyp(hyp1);
          show_hyp(hyp2);
          cout << "Back " <<  join_score <<endl; 
          show_hyp(join);
          */
          best_edge_back_hypotheses[j].try_set_hyp(join, join_score);
        }
      }
    }
  }
}

// find the best path through a hypergraph with some extra information stored
void ExtendCKY::node_best_path(const Hypernode & node) {

  if (_memo_table.has_key(node)) {
    return;
  }

  //int id = node.id();

  //Hyperedge * best_edge = NULL; 
  
  //cout << "EDGES: "<< node.num_edges() <<endl; 
  
  // BestHyp best_node_hypotheses(_controller.size());
  BestHyp best_node_hypotheses;
  //bool has_node_changed = false;  


  if (node.is_terminal()) {
    vector <Hypothesis *> hyps;
    vector <double> scores; 
    _controller.initialize_hypotheses(node, hyps, scores);
    assert(hyps.size() == scores.size());
    assert(hyps.size() != 0);
    for (uint i=0; i < hyps.size(); i++) {
      best_node_hypotheses.try_set_hyp(hyps[i], scores[i]); 
    }
  } else {
    Cache <Hyperedge, bool> redo_edge(_forest.num_edges());  

    foreach (HEdge edge, node.edges()) { 
      foreach (HNode sub_node, edge->tail_nodes()) { 
        node_best_path(*sub_node);
      }
    }
      
    // otherwise 
    foreach (HEdge edge, node.edges()) {
      double edge_value= _edge_weights.get_value(*edge);        
      vector<BestHyp> best_edge_hypotheses(edge->num_nodes()); 
      vector<BestHyp> best_edge_back_hypotheses(edge->num_nodes()); 
      
      forward_edge(*edge, best_edge_hypotheses);
 
      // used for outside_scores
      backward_edge(*edge, best_edge_back_hypotheses);
      
     
      _memo_edge_table.set_value(*edge, best_edge_hypotheses);
      _memo_edge_back_table.set_value(*edge, best_edge_back_hypotheses);
        
      int last = edge->num_nodes() -1; 

      assert(best_edge_hypotheses[last].size() != 0);
      for (int iter = 0; iter< best_edge_hypotheses[last].size(); iter++) {
        
        Hypothesis * hyp1 = best_edge_hypotheses[last].hyps[iter];
        
        double score1 = best_edge_hypotheses[last].get_score(iter);
            
        double score = score1 + edge_value;
        
        if (DEBUG) {
          cout << "INSIDE " << edge->id() << " " << node.id() << " " << hyp1->id() << " " << score1 << " " 
               << edge_value << " " << score << endl;
        }
        bool w = best_node_hypotheses.try_set_hyp(hyp1, score);
      }
    }
  } 
  

  assert (best_node_hypotheses.size() != 0);
  
  if (DEBUG ) {
    cout << "Setting "<<node.id() << endl;
    for (int iter = 0; iter< best_node_hypotheses.size(); iter++) {
      cout << " " <<iter <<"=" <<best_node_hypotheses.get_score(iter) << " ";
    }
    //if (best_node_hypotheses.size() > _controller.prune_to()) {
    //best_node_hypotheses.prune(_controller.prune_to());
    //}
    cout << endl;
    cout << "After "<<node.id() << endl;
    for (int iter = 0; iter< best_node_hypotheses.size(); iter++) {
      cout << " " <<iter <<"=" <<best_node_hypotheses.get_score(iter) << " ";
    }
    cout << endl;
  }
  _memo_table.set_value(node, best_node_hypotheses);
} 


double ExtendCKY::best_path(NodeBackCache & back_pointers) {
  //_old_memo_table = _memo_table;
  //_memo_table = new Cache<Hypernode, BestHyp>(_forest.num_nodes());
  
  
  node_best_path(_forest.root());
  BestHyp & at_root = _memo_table.store[_forest.root().id()];
  double best = 1e20;
  
  Hypothesis best_hyp;

  best = _controller.find_best(at_root.hyps, at_root.scores, best_hyp);
  _total_best = best;
  
  extract_back_pointers(_forest.root(), best_hyp, _memo_table, back_pointers);

  //for (int i=0; i < _to_delete.size(); i++) {
  //delete _to_delete[i];
  //}
  //_to_delete.clear();
  
  return best;
}

void ExtendCKY::node_best_out_fast(const Hypernode & node) {
  // when you get to a node it is done already 
  assert (_outside_memo_table.has_key(node));
  assert(_out_done.find(node.id()) == _out_done.end());
  BestHyp & above = _outside_memo_table.store[node.id()];
  //BestHyp & above_inside = _memo_table.store[node.id()];

  // do all its children
  //int id = node.id();
  // calculate the outside score for a node
  
  //for (int i=0; i< node.num_edges(); i++) {
  //const Hyperedge & edge = node.edge(i);
  foreach (HEdge edge, node.edges()) {
    double edge_value= _edge_weights.get_value(*edge);        

    vector <BestHyp> & outside_edge = _outside_edge_memo_table.store[edge->id()]; 
    _outside_edge_memo_table.has_value[edge->id()] = true;
    outside_edge.resize(edge->num_nodes());
    
    vector <BestHyp> & edge_forward_hyps = _memo_edge_table.store[edge->id()]; 
    vector <BestHyp> & edge_backward_hyps = _memo_edge_back_table.store[edge->id()]; 
    
    uint last = edge->num_nodes()-1;
    for (uint j=0; j < edge->num_nodes(); j++) {
      uint pos = j;
      const Hypernode & sub_node = edge->tail_node(j);

      // make sure we are in breadth first order
      assert(_out_done.find(sub_node.id()) == _out_done.end());

      // square sub_node
      _out_queue.push(sub_node.id());
      BestHyp & best_at_node = _outside_memo_table.store[sub_node.id()];
      _outside_memo_table.has_value[sub_node.id()] = true;
      
      BestHyp & below = _memo_table.store[sub_node.id()];

      for (int iter = 0; iter < above.size(); iter++) {
        const Hypothesis & hyp1 = above.get_hyp(iter);
        double score1 = above.get_score(iter);

        // dot outside S ->NP . VP

        for (int iter2 =0; iter2 < edge_forward_hyps[pos].size(); iter2++) {
          const Hypothesis & hyp2 = edge_forward_hyps[pos].get_hyp(iter2);
          
          double inside = edge_forward_hyps[pos].get_score(iter2);
          double inside_top = _memo_table.store[node.id()].get_score_by_id(hyp1.id());

          if (!(hyp1.hook == hyp2.hook)) continue;
          // right side
          double right_score;
          if (pos == last) {
            if (hyp2.right() != hyp1.right()) continue;
            right_score = 0.0;
          } else {
            int id = Hypothesis(hyp2.right_side, hyp1.right_side).id(); 
            if (!edge_backward_hyps[pos+1].has_id(id))  continue;
            right_score = edge_backward_hyps[pos +1].get_score_by_id(id);
          }
          // top outside + edge + right_side
          double score = score1 + right_score + edge_value; 
        

          Hypothesis * h = new Hypothesis(hyp2.hook, hyp2.right_side, edge);
          outside_edge[pos].try_set_hyp(h, score);
          //cout << "Outside dot " << edge.id() << " "  << pos << " " << hyp2.id() << " "<<score << " " << b << endl; 
        
          assert (inside_top <= (inside + right_score + edge_value) + 1e-4); 
          assert(_total_best <=  (score + inside) + 1e-4) ;
          if (DEBUG) {
            cout << "Outside EDGE " << edge->id()<< " " << pos << " " <<h->id() << " " << score + inside<<" "<< _total_best << " " << inside << " " << inside + right_score + edge_value << " " << inside_top << endl;
          }
        }



        // nodes below
         
        for (int iter2 =0; iter2 < below.size(); iter2++) {
          const Hypothesis & hyp2 = below.get_hyp(iter2);
          //double score2 = below.get_score(iter2);
           
          // left side
          double left_score;
          if (pos == 0) {
            if (hyp2.left() != hyp1.left()) continue;
            left_score = 0.0; 
          } else {
            int id = Hypothesis(hyp1.hook, hyp2.hook).id(); 
            if (!edge_forward_hyps[pos-1].has_id(id))  continue;
            left_score = edge_forward_hyps[pos -1].get_score_by_id(id);
          }
          // right side
          double right_score;
          if (pos == last) {
            if (hyp2.right() != hyp1.right()) continue;
            right_score = 0.0;
          } else {
            int id = Hypothesis(hyp2.right_side, hyp1.right_side).id(); 
            if (!edge_backward_hyps[pos+1].has_id(id))  continue;
            right_score = edge_backward_hyps[pos +1].get_score_by_id(id);
          }
         
          // top outside + left inside + right inside + edge score
          double total_score = left_score + right_score + edge_value + score1;
          //BestHyp & at = _outside_memo_table.store[node];
          Hypothesis * h = new Hypothesis(hyp2.hook, hyp2.right_side, edge);
          double inside = _memo_table.store[sub_node.id()].get_score_by_id(h->id());
          double inside_top = _memo_table.store[node.id()].get_score_by_id(hyp1.id());
          
           
          if (DEBUG) {
            cout << "Outside node " <<  edge->id() << " " << node.id() << " " << sub_node.id() << " " << hyp1.id() << " " <<total_score + inside<< " " << _total_best << " " << total_score << " " 
                 << left_score << " " << right_score << " " << edge_value << " "  << (left_score + right_score + edge_value + inside) << " " << inside_top << " " <<score1 << " " << 
              inside << " " <<endl;
          }
           assert (inside_top <= (left_score + right_score + edge_value + inside) + 1e-4); 
           
           best_at_node.try_set_hyp(h, total_score);
           assert(_total_best <=  (total_score + inside) + 1e-4) ;
           assert(_total_best <=  (total_score + inside) + 1e-4) ;
         }
       }
     }
   }
 }

 void ExtendCKY::node_best_out_path(const Hypernode & node) {
  
  if (_outside_memo_table.has_key(node)) {
    return;
  }

  // Edge case root
  if (node.id() == _forest.root().id()) {
    //_outside_memo_table[node.id()].set_value()
    vector <Hypothesis *> hyps;
    vector <double > scores;
    BestHyp best_hyps; 
    _controller.initialize_out_root(hyps, scores);


    assert(hyps.size() == scores.size());
    for (uint i=0;i < hyps.size(); i++) {
      
      if (_memo_table.get_value(node).has_id(hyps[i]->id())) {
        double inside = _memo_table.get_value(node).get_score_by_id(hyps[i]->id());
        assert(_total_best <=  (scores[i] + inside) + 1e-4); 
        best_hyps.try_set_hyp(hyps[i], scores[i]);
      }
    }
    _outside_memo_table.set_value(node,  best_hyps);

    return; 
  }
  
  // calculate the outside score for a node
  BestHyp best_at_node;
  // do this in the simplest! way possible. Look at all the ways we can get to a node
  foreach (HEdge edge, node.in_edges()) { // int i =0; i < node.num_in_edges(); i++) {
    //const Hyperedge & edge = node.in_edge(i);
    const Hypernode & top_node = edge->head_node();
    vector <BestHyp> & outside_edge = _outside_edge_memo_table.store[edge->id()]; 
    _outside_edge_memo_table.has_value[edge->id()] = true;
    outside_edge.resize(edge->num_nodes());

    // cache outside above 
    node_best_out_path(top_node);
    double edge_value= _edge_weights.get_value(*edge); 

    int pos = -1;
    uint last = edge->num_nodes()-1;
    for (uint j =0; j < edge->num_nodes(); j++) {
      if (edge->tail_node(j).id() == node.id()) 
        pos = j;
    }
    assert(pos!=-1);
    BestHyp & above = _outside_memo_table.store[top_node.id()];
    //BestHyp & above_inside = _memo_table.store[top_node.id()];
    
    BestHyp & below = _memo_table.store[node.id()];
    vector <BestHyp> & edge_forward_hyps = _memo_edge_table.store[edge->id()]; 
    vector <BestHyp> & edge_backward_hyps = _memo_edge_back_table.store[edge->id()]; 

    for (int iter = 0; iter < above.size(); iter++) {
      const Hypothesis & hyp1 = above.get_hyp(iter);
      double score1 = above.get_score(iter);
      double inside_top = _memo_table.get_value(top_node).get_score_by_id(hyp1.id());
      // dot outside S ->NP . VP
      for (int iter2 =0; iter2 < edge_forward_hyps[pos].size(); iter2++) {
        const Hypothesis & hyp2 = edge_forward_hyps[pos].get_hyp(iter2);
        double inside = edge_forward_hyps[pos].get_score(iter2);
        if (!(hyp1.hook == hyp2.hook)) continue;

        // right side
        double right_score;
        if (pos == (int)last) {
          if (hyp2.right() != hyp1.right()) continue;
          right_score = 0.0;
        } else {
          int id = Hypothesis(hyp2.right_side, hyp1.right_side).id(); 
          if (!edge_backward_hyps[pos+1].has_id(id))  continue;
          right_score = edge_backward_hyps[pos+1].get_score_by_id(id);
        }
        // top outside + edge + right_side
        double score = score1 + right_score + edge_value; 
        
        Hypothesis * h = new Hypothesis(hyp2.hook, hyp2.right_side, edge);
        
        assert (inside_top <= (inside + right_score + edge_value) + 1e-4); 
        assert(_total_best <=  (score + inside) + 1e-4) ;

        cout << "EDGE " << edge->id() << " " <<h->id() << " " << _total_best << " " << inside << " " << inside + right_score + edge_value << " " << inside_top << endl;

        outside_edge[pos].try_set_hyp(h, score);
      }

      // node outside

      for (int iter2 =0; iter2 < below.size(); iter2++) {
        const Hypothesis & hyp2 = below.get_hyp(iter2);
        //double score2 = below.get_score(iter2);
      
        // left side
        double left_score;
        if (pos == 0) {
          if (hyp2.left() != hyp1.left()) continue;
          left_score = 0.0; 
        } else {
          int id = Hypothesis(hyp1.hook, hyp2.hook).id(); 
          if (!edge_forward_hyps[pos-1].has_id(id))  continue;
          left_score = edge_forward_hyps[pos -1].get_score_by_id(id);
        }
        // right side
        double right_score;
        if (pos == (int)last) {
          if (hyp2.right() != hyp1.right()) continue;
          right_score = 0.0;
        } else {
          int id = Hypothesis(hyp2.right_side, hyp1.right_side).id(); 
          if (!edge_backward_hyps[pos+1].has_id(id))  continue;
          right_score = edge_backward_hyps[pos +1].get_score_by_id(id);
        }

        // top outside + left inside + right inside + edge score
        double total_score = left_score + right_score + edge_value + score1;
        //BestHyp & at = _outside_memo_table.store[node];
        Hypothesis * h = new Hypothesis(hyp2.hook, hyp2.right_side, edge);
        double inside = _memo_table.get_value(node).get_score_by_id(h->id());
        
        
        if (DEBUG) {
          cout << "Outside " <<  edge->id() << " " << top_node.id() << " " << hyp1.id() << " " <<total_score << " " 
             << left_score << " " << right_score << " " << edge_value << " "  << (left_score + right_score + edge_value + inside) << " " << inside_top << " " <<score1 << " " << 
            inside << " " <<endl;
        }
        assert (inside_top <= (left_score + right_score + edge_value + inside) + 1e-4); 

        best_at_node.try_set_hyp(h, total_score);
        assert(_total_best <=  (total_score + inside) + 1e-4) ;
        assert(_total_best <=  (total_score + inside) + 1e-4) ;
      }
    }
  }

  if (!node.is_terminal()) {
    double best = INF;
    for (int i =0 ; i < best_at_node.size(); i++) {
      const Hypothesis & h  = best_at_node.get_hyp(i);
      
      best = min(best, best_at_node.get_score(i) + _memo_table.get_value(node).get_score_by_id(h.id()));
    }
    //assert(fabs(_total_best - best) < 1e-4) ;
    assert(_total_best <=  best + 1e-4) ;
  }
  _outside_memo_table.set_value(node,  best_at_node);
} 

void ExtendCKY::outside() {
  // first initialize root
    vector <Hypothesis *> hyps;
    vector <double > scores;
    BestHyp best_hyps; 
    _controller.initialize_out_root(hyps, scores);


    const Hypernode & root = _forest.root();
    assert(hyps.size() == scores.size());
    for (uint i=0;i < hyps.size(); i++) {
      if (_memo_table.get_value(root).has_id(hyps[i]->id())) {
        double inside = _memo_table.get_value(root).get_score_by_id(hyps[i]->id());
        assert(_total_best <=  (scores[i] + inside) + 1e-4); 
        best_hyps.try_set_hyp(hyps[i], scores[i]);
      }
    }
    _outside_memo_table.set_value(root,  best_hyps);


    vector <const Hypernode *> node_order =
      HypergraphAlgorithms(_forest).topological_sort();

    //for (int i=0; i < node_order.size() ;i ++ ) {
    foreach (const Hypernode * node, node_order) { 
      int id = node->id(); //node_order[i];
      if (_out_done.find(id) == _out_done.end()) {
        node_best_out_fast(*node);
        _out_done.insert(id);
      }
    }

    //outside_scores = _outside_memo_table;    
    //outside_edge_scores = _outside_edge_memo_table;
  // assume that we have a _memo_table to play with
  
  /*for (int i = 0; i < _forest.num_nodes(); i++) {
    const Hypernode & node = _forest.get_node(i);
    if (!node.is_word()) continue;
    node_best_out_path(node);
  }
*/
}
  }}
