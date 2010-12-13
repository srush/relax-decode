#include "ExtendCKY.h"

//typedef StoreCache <Hypothesis, double> BestHyp;
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
  
  // BestHyp best_node_hypotheses(_controller.size());
  BestHyp best_node_hypotheses;
  bool has_node_changed = false;  


  if (node.num_edges() == 0) {
    
    assert (node.is_word());
    _controller->initialize_hypotheses(node, best_node_hypotheses);
    has_node_changed |= best_node_hypotheses.has_new;
    //cout << "NODE " << node.id() << " " << has_node_changed <<endl;
  } else {
    Cache <ForestEdge, bool> redo_edge(_forest.num_edges());  
    // check if it is a repeat 
    if (_is_first) {
      has_node_changed = true; 
    } else {
      //cout << "checking under" << endl;
      for (int i=0; i< node.num_edges(); i++) {
        const ForestEdge & edge = node.edge(i); 
        bool edge_changed = _edge_weights->get_value(edge) != _old_edge_weights->get_value(edge);
        //cout << "edge "<< edge.id() << " " << edge_changed << endl;
        has_node_changed |= edge_changed;
        
        for (int j=0; j < edge.num_nodes(); j++ ) {
          const ForestNode & sub_node = edge.tail_node(j);
          node_best_path(sub_node);
          const BestHyp & local_best = _memo_table.store[sub_node.id()];
          has_node_changed |= local_best.has_new;
          edge_changed |= local_best.has_new;
          
          //cout << "node "<< sub_node.id() << " " << local_best.has_new << endl;
        }
        redo_edge.set_value(edge, edge_changed);
        //cout << "EDGE "<< edge.id() << " " << edge_changed << endl;
      }
    }

    //cout << "NODE " << id << " " << has_node_changed <<endl;
    
    

    if (false && !has_node_changed) {
      // we're not new, so use old hypothesis
      //best_node_hypotheses =  _old_memo_table->get_value(node);
      //best_node_hypotheses.has_new = 0;
    } else {
      // otherwise
      for (int i=0; i< node.num_edges(); i++) {
        const ForestEdge & edge = node.edge(i);
        
        double edge_value= _edge_weights->get_value(edge);
        
        BestHyp *last_best, * best_edge_hypotheses; 
        // viterbi to find best edge
        for (int j=0; j < edge.num_nodes(); j++ ) {
          //if (j!=0)
          //delete last_best;
          last_best = best_edge_hypotheses;
          //best_edge_hypotheses = new BestHyp(_controller.size());
          
          best_edge_hypotheses = new BestHyp();
          _to_delete.push_back(best_edge_hypotheses);
          
          const ForestNode & sub_node = edge.tail_node(j);
          //cout << "EMPTY (" << endl;
          node_best_path(sub_node);
          //cout << ")" << endl;
          const BestHyp & local_best = _memo_table.store[sub_node.id()];
          //assert(local_best.size() == _controller.size());
          //assert(j ==0 || last_best->size() == _controller.size());
          //local_best.resize(_controller.size());
          //assert (local_best.size() != 0);
          
          if (j ==0) {
            // at left most, all are valid
            for (int iter = 0; iter< local_best.size(); iter++) {
              //if (!local_best.has_key(iter)) continue;
              
              const Hypothesis & hyp = local_best.get_hyp(iter); 
              double score = local_best.get_score(iter);
              Hypothesis new_hyp(hyp.hook, hyp.right_side, &edge, _controller->dim(), hyp.is_new);
              new_hyp.prev_hyp.push_back(&hyp);
              
            //cout << "init " << new_hyp.id() << endl;
              show_hyp(new_hyp);
              bool worked;
            // is this a new hypothessis?
              best_edge_hypotheses->try_set_hyp(new_hyp,  score, worked, new_hyp.is_new);
              assert(worked);
              //assert(best_edge_hypotheses->has_key( new_hyp.id()));
            }
          } else {
            // match hook sig at left 
          
            for (int iter = 0; iter< local_best.size(); iter++) {
              //if (!local_best.has_key(iter)) continue;           
              const Hypothesis & hyp1 = local_best.get_hyp(iter); 
              double score1 = local_best.get_score(iter);
            
              vector <int> pos = last_best->join(hyp1);

              for (int iter2 = 0; iter2< pos.size(); iter2++) {
              //cout << "last_best may fail" <<  iter2 << endl;
              //if (!last_best->has_key(iter2)) continue;
              
              //vector<int> hook = iter2->first.hook;
              //vector<int> right = iter->first.right_side;
              
                const Hypothesis & hyp2 = last_best->get_hyp(pos[iter2]); 
                double score2 = last_best->get_score(pos[iter2]);
              //cout << "last_best" <<  hyp2.id() << endl;
              //cout << "attempting" << endl;
                show_hyp(hyp1);
                show_hyp(hyp2);

              //if (!hyp2.match(hyp1)) continue;
                assert(hyp2.match(hyp1));

                Hypothesis join(_controller->dim());
                //cout << "successful match" << endl;
                show_hyp(hyp1);
                show_hyp(hyp2);
                join.back_edge = &edge;
                assert(hyp2.prev_hyp.size() == j);
                double join_score = score1 + score2 +
                  _controller->combine(hyp2, hyp1, join);
                assert(join.prev_hyp.size() == j+1);
                //assert(join.prev_hyp[j] == iter->first);
                
                bool w;
                best_edge_hypotheses->try_set_hyp(join, join_score, w, join.is_new);
                //check = best_edge_hypotheses->find(join);
                /*if (!best_edge_hypotheses->has_key(join) ||
                  join_score < best_edge_hypotheses->get_value(join)) {
                  //cout << "Inserting " << join.hook.size() << endl;
                  best_edge_hypotheses->set_value(join, join_score); 
                  }*/
              }
            }
          }
        }
      
        // Do I need?
        //assert (best_edge_hypotheses->size() != 0);
        

        //cout << edge_value << endl;
        //      BestHyp::const_iterator iter, check;
        // (iter = best_edge_hypotheses->begin(); iter!=best_edge_hypotheses->end(); iter++) {
        for (int iter = 0; iter< best_edge_hypotheses->size(); iter++) {
          //if (!best_edge_hypotheses->has_key(iter)) continue;
          
          const Hypothesis & hyp1 = best_edge_hypotheses->get_hyp(iter); 
          double score1 = best_edge_hypotheses->get_score(iter);
          
          double score = score1 + edge_value;
          //check = best_node_hypotheses.find(iter->first);
          bool w;
          best_node_hypotheses.try_set_hyp(hyp1, score, w, hyp1.is_new);
          if (w) {
            //cout << "Possible at " << node.id()  << endl;
          /*double check_score =0.0;
          for (int i =0; i < hyp1.back_edge->num_nodes();i++) {
            check_score += _memo_table.get_value(hyp1.back_edge->tail_node(i)).get_value(*hyp1.prev_hyp[i]);
          }
          double sub_val = _edge_weights.get_value(*(hyp1.back_edge));
          check_score += sub_val;
          //cout << node.id() << " " << iter->first.back_edge->id() << endl;
          assert(fabs(score-check_score)  < 1e-4 );*/
          } 
        }
      }
    }
  }
  /*
  if (!_is_first && !has_node_changed) {
    // old should be same as new
    BestHyp old =  _old_memo_table->get_value(node);
    for (int iter = 0; iter< old.size(); iter++) {
      const Hypothesis & hyp = old.get_hyp(iter); 
      double score1 = old.get_score(iter);
      const Hypothesis & hyp2 = best_node_hypotheses.get_hyp(iter); 
      double score2 = best_node_hypotheses.get_score(iter);
      
      //assert(fabs(score1 - score2) < 1e-3 );
    }
    }*/
  assert (best_node_hypotheses.size() != 0);
  cout << "Setting "<<node.id() << endl;
  for (int iter = 0; iter< best_node_hypotheses.size(); iter++) {
    cout << " " <<iter <<"=" <<best_node_hypotheses.get_score(iter) << " ";
  }
  //if (best_node_hypotheses.size() > _controller->prune_to()) {
    best_node_hypotheses.prune(_controller->prune_to());
    //}
  cout << endl;
  cout << "After "<<node.id() << endl;
  for (int iter = 0; iter< best_node_hypotheses.size(); iter++) {
    cout << " " <<iter <<"=" <<best_node_hypotheses.get_score(iter) << " ";
  }
  cout << endl;
  _memo_table.set_value(node, best_node_hypotheses);
} 


double ExtendCKY::best_path(NodeBackCache & back_pointers) {
  //_old_memo_table = _memo_table;
  //_memo_table = new Cache<ForestNode, BestHyp>(_forest.num_nodes());
  
  
  node_best_path(_forest.root());
  BestHyp & at_root = _memo_table.store[_forest.root().id()];
  double best = 1e20;
  
  Hypothesis best_hyp(_controller->dim());

  best = _controller->find_best(at_root, best_hyp);
  
  
  extract_back_pointers(_forest.root(), best_hyp, back_pointers);

  //for (int i=0; i < _to_delete.size(); i++) {
  //delete _to_delete[i];
  //}
  //_to_delete.clear();
  _is_first = false;
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
