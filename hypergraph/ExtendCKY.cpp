#include "ExtendCKY.h"

typedef StoreCache <Hypothesis, double> BestHyp;
#include <iostream>
using namespace std;

void show_hyp(const Hypothesis & hyp) {
  //cout << hyp.right_side[0] << " " << hyp.right_side[1] << " "<< hyp.hook[0]<< " "  << hyp.hook[1] << endl;

}

// find the best path through a hypergraph with some extra information stored
void ExtendCKY::node_best_path(const ForestNode & node) {

  if (_memo_table.has_key(node)) {
    return;
  }

  int id = node.id();

  ForestEdge * best_edge = NULL; 
  
  //cout << "EDGES: "<< node.num_edges() <<endl; 
  BestHyp best_node_hypotheses(_controller.size());
  
  if (node.num_edges() == 0) {
    
    assert (node.is_word());
    _controller.initialize_hypotheses(node, best_node_hypotheses);
   
  } else {
    for (int i=0; i< node.num_edges(); i++) {
      const ForestEdge & edge = node.edge(i);
      
      double edge_value= _edge_weights.get_value(edge);
      
      BestHyp *last_best, * best_edge_hypotheses; 
      // viterbi to find best edge
      for (int j=0; j < edge.num_nodes(); j++ ) {
        //if (j!=0)
        //delete last_best;
        last_best = best_edge_hypotheses;
        best_edge_hypotheses = new BestHyp(_controller.size());

        const ForestNode & sub_node = edge.tail_node(j);
        node_best_path(sub_node);
        const BestHyp & local_best = _memo_table.store[sub_node.id()];
        assert(local_best.size() == _controller.size());
        assert(j ==0 || last_best->size() == _controller.size());
        //local_best.resize(_controller.size());
        assert (local_best.size() != 0);
        
        if (j ==0) {
          // at left most, all are valid
          for (int iter = 0; iter< local_best.size(); iter++) {
            if (!local_best.has_key(iter)) continue;
            
            const Hypothesis & hyp = local_best.full_keys[iter]; 
            double score = local_best.store[iter];
            Hypothesis new_hyp(hyp.hook, hyp.right_side, &edge, _controller.dim());
            new_hyp.prev_hyp.push_back(&hyp);
            
            //cout << "init " << new_hyp.id() << endl;
            show_hyp(new_hyp);
            best_edge_hypotheses->set_value(new_hyp,  score);
            assert(best_edge_hypotheses->has_key( new_hyp.id()));
          }
        } else {
          // match hook sig at left 
          
          for (int iter = 0; iter< local_best.size(); iter++) {
            if (!local_best.has_key(iter)) continue;
           
            const Hypothesis & hyp1 = local_best.full_keys[iter]; 
            double score1 = local_best.store[iter];
            //cout << "local_best" <<  hyp1.id() << endl;
            for (int iter2 = 0; iter2< last_best->size(); iter2++) {
              //cout << "last_best may fail" <<  iter2 << endl;
              if (!last_best->has_key(iter2)) continue;
              
              //vector<int> hook = iter2->first.hook;
              //vector<int> right = iter->first.right_side;
              
              const Hypothesis & hyp2 = last_best->full_keys[iter2]; 
              double score2 = last_best->store[iter2];
              //cout << "last_best" <<  hyp2.id() << endl;
              //cout << "attempting" << endl;
              show_hyp(hyp1);
              show_hyp(hyp2);

              if (!hyp2.match(hyp1)) continue;
              Hypothesis join(_controller.dim());
              //cout << "successful match" << endl;
              show_hyp(hyp1);
              show_hyp(hyp2);
              join.back_edge = &edge;
              assert(hyp2.prev_hyp.size() == j);
              double join_score = score1 + score2 +
                _controller.combine(hyp2, hyp1, join);
              assert(join.prev_hyp.size() == j+1);
              //assert(join.prev_hyp[j] == iter->first);
              
              
              //check = best_edge_hypotheses->find(join);
              if (!best_edge_hypotheses->has_key(join) ||
                  join_score < best_edge_hypotheses->get_value(join)) {
                //cout << "Inserting " << join.hook.size() << endl;
                best_edge_hypotheses->set_value(join, join_score); 
              }
            }
          }
        }
      }

      assert (best_edge_hypotheses->size() != 0);
      //cout << edge_value << endl;
      //      BestHyp::const_iterator iter, check;
      // (iter = best_edge_hypotheses->begin(); iter!=best_edge_hypotheses->end(); iter++) {
      for (int iter = 0; iter< best_edge_hypotheses->size(); iter++) {
        if (!best_edge_hypotheses->has_key(iter)) continue;

        const Hypothesis & hyp1 = best_edge_hypotheses->full_keys[iter]; 
        double score1 = best_edge_hypotheses->store[iter];

        double score = score1 + edge_value;
        //check = best_node_hypotheses.find(iter->first);
        if (!best_node_hypotheses.has_key(hyp1)
            || score < best_node_hypotheses.get_value(hyp1)) {
 
          best_node_hypotheses.set_value(hyp1, score);
          //cout << "Possible at " << node.id()  << endl;
          double check_score =0.0;
          for (int i =0; i < hyp1.back_edge->num_nodes();i++) {
            check_score += _memo_table.get_value(hyp1.back_edge->tail_node(i)).get_value(*hyp1.prev_hyp[i]);
          }
          double sub_val = _edge_weights.get_value(*(hyp1.back_edge));
          check_score += sub_val;
          //cout << node.id() << " " << iter->first.back_edge->id() << endl;
          assert(fabs(score-check_score)  < 1e-4 );
        } 
      }
    }
  }

  assert (best_node_hypotheses.size() != 0);
  _memo_table.set_value(node, best_node_hypotheses);
} 


double ExtendCKY::best_path(NodeBackCache & back_pointers) {
  node_best_path(_forest.root());
  BestHyp & at_root = _memo_table.store[_forest.root().id()];
  double best = 1e20;
  
  Hypothesis best_hyp(_controller.dim());

  best = _controller.find_best(at_root, best_hyp);
  
  
  extract_back_pointers(_forest.root(), best_hyp, back_pointers);

  return best;
}


void ExtendCKY::extract_back_pointers(const ForestNode & node, const Hypothesis & best_hyp, 
                                      NodeBackCache & back_pointers) {
  if (back_pointers.has_key(node)) {
    assert(false);
    return;
  } else if (node.num_edges() == 0) {
    assert(node.is_word());
    //cout << best_hyp.hook << " " << best_hyp.original_value << " " << node.word();
    
    return;
  } else {
    back_pointers.set_value(node, best_hyp.back_edge);
    
    const ForestEdge & edge = *best_hyp.back_edge;
    //cout << node.id() << " " << edge.id() << endl;
    assert(edge.num_nodes() == best_hyp.prev_hyp.size());
    for (int j=0; j < edge.num_nodes(); j++ ) {
      const ForestNode & sub_node = edge.tail_node(j);
      extract_back_pointers(sub_node, *best_hyp.prev_hyp[j], back_pointers);
    }
  }
  return;
}
