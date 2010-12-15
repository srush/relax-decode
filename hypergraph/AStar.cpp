#include "AStar.h"
#include <queue>
#include <iostream>
using namespace std;


#define DEBUG 0


void AStar::add_to_queue( Hypothesis * hyp, double score, Location * w) {
  //assert(_outside_scores.get_value(hyp.node).hasby_id(hyp.id()))
  _num_pushes++;
  if (DEBUG) {
    cout << "Adding to queue " <<  score << endl;
    show_hyp(*hyp);
    w->show();
  }
  double with_astar;
  double heuristic = 0;  
  if (!hyp->is_done) { 
    //const ForestNode & node =  _forest.get_node(node_id);
    if (!_heuristic.has_value(*w, *hyp)) {
      if (DEBUG)
        cout << "Skipping" << endl; 
      return;
    }
    heuristic = _heuristic.get_value(*w, *hyp);
    with_astar = score + heuristic;
  } else {
    with_astar = score;
  }
  if (DEBUG)  
    cout << " +AStar" << with_astar <<  " " << heuristic << endl; 
  QueueHyp elem(hyp, with_astar, w);

  //assert (with_astar >= _best_so_far - 1e-4);
  _queue.push(elem);
}

void AStar::get_next(Hypothesis *& hyp, double & score, Location *& w) {
  
  QueueHyp elem = _queue.top();
  _queue.pop();
  _num_pops++;

  hyp = elem.h;
  // remove heuristic
  if (DEBUG) {
    cout << "Pop from queue " <<  elem.score << endl;
    elem.where->show();
    show_hyp(*hyp);
  }
  double heuristic = 0.0;
  //node_id = elem.node_id;
  w = elem.where;
  _best_so_far = max(_best_so_far, elem.score);
  if (!hyp->is_done) { 
    heuristic = _heuristic.get_value(*w, *hyp); 
    score = elem.score - heuristic;
  } else {
    score = elem.score;
  }
  if (DEBUG)
    cout << " -AStar" << score << " " <<heuristic << endl; 
}

// add words to queue
void AStar::initialize_queue() {
  for (int i =0; i < _forest.num_nodes(); i++) {
    const ForestNode & node = _forest.get_node(i);
    if (!node.is_word()) continue;
    
    // get the words
    vector <Hypothesis *> hyps; 
    vector <double> scores; 
    _controller.initialize_hypotheses(node, hyps, scores);
    assert(scores.size() == hyps.size());
    
    Location * l = alloc_loc();
    l->location = NODE;
    l->node_id = node.id();

    for (int i=0; i < hyps.size() ;i++) {
      //hyps[i].at_node_id = node.id();      
      //Hypothesis * h = new Hypothesis(hyps[i]);
      //alloc_hyp();
      //*h = hyps[i]; 
      add_to_queue(hyps[i], scores[i], l);
    }
  }
}

void AStar::main_loop(Hypothesis * & best, double & best_score ) {
  bool found = false;
  while (!_queue.empty()) {
    Hypothesis * h; 
    double score;
    int node_id;
    Location * l;
    get_next(h, score, l);

  
    if (h->is_done) {
      best = h;
      best_score = score;
      //cout << "Is done" << endl;
      found = true;
      break;
    }

    //assert(l->location == NODE);    
    if (l->location == NODE) {
      const ForestNode & node = _forest.get_node(l->node_id);    
      // get the memo table for the node
      BestHyp & best = _memo_table.store[node.id()];
      _memo_table.has_value[node.id()] = true;
      bool is_set = best.try_set_hyp(h, score);
      //_memo_table.set_value(node, best);
      if (DEBUG) {
        if (node.id() == _forest.root().id()) {
          cout << "HIT ROOT early" << endl;
        }
        cout << "Trying to set " << node.id() <<  " " << h->id() << endl;
      }


      // it replaced the value! Need to recompute everything above me. 
      if (is_set) {
        //cout << "SETTING" << endl;
        // dirty and rerun all the other edges that were relying on my value. 
        if (node.id() == _forest.root().id()) {
          // special case, finish it off. 
          //cout << "HIT ROOT" << endl;
          vector <Hypothesis *> hyps;
          vector <double> scores;
          hyps.push_back(h);
          scores.push_back(score);
          Hypothesis * best_hyp = alloc_hyp();
          double final_score =_controller.find_best(hyps, scores, *best_hyp);
          best_hyp->is_done = true;
          Location*  l = alloc_loc();
          l->location = TOP;
          add_to_queue(best_hyp, final_score, l);
        } else {
          recompute_node(node, *h, score);
        }
      }
    } else if (l->location==EDGE) {

      const ForestEdge & edge = _forest.get_edge(l->edge_id);    
      // get the memo table for the node
      vector<BestHyp> & best = _memo_edge_table.store[edge.id()];
      _memo_edge_table.has_value[edge.id()] = true;
      bool is_set = best[l->edge_pos].try_set_hyp(h, score);
      assert(h->prev_hyp.size() == l->edge_pos+1);
      if (is_set) {
        recompute_edge(edge, l->edge_pos, *h, score);
      } 
    }
  }
  assert(found);
  //cout << "ENDED" << endl;
}



void AStar::recompute_edge(const ForestEdge & edge,
                           int pos,
                           const Hypothesis & h, 
                           double original_score) {
  int last = edge.num_nodes() -1;

  double edge_value= _edge_weights.get_value(edge);
  // dot has reached end. finish me
  if (pos == last) {
    assert(false);
  } else {
    // not last, just in middle, sum over possible next
    const ForestNode & sub_node = edge.tail_node(pos+1);
    const BestHyp & next_best = _memo_table.store[sub_node.id()];    
    vector <int> next_pos = next_best.join_back(h);    
    
    for (int iter2 = 0; iter2< next_pos.size(); iter2++) {

      const Hypothesis & hyp2 = next_best.get_hyp(next_pos[iter2]); 
      double score2 = next_best.get_score(next_pos[iter2]);
      
      assert(h.match(hyp2));
      
      Hypothesis  * join = alloc_hyp();
      join->dim =(_controller.dim());
      
      join->back_edge = &edge;
      assert(h.prev_hyp.size() == pos+1);
      double join_score = original_score + score2 +
        _controller.combine(h, hyp2, *join);
      assert(join->prev_hyp.size() == pos+2);

      if (pos +1 == last) {
        Location * l = alloc_loc();
        l->location = NODE;
        l->node_id = edge.head_node().id();
        double score = join_score + edge_value;
        add_to_queue(join, score, l);

      } else {
        Location * l = alloc_loc();
        l->location = EDGE;
        l->edge_pos = pos +1;
        l->edge_id = edge.id();
        
        add_to_queue(join, join_score, l);
      }
        //best_edge_hypotheses[pos].try_set_hyp(join, join_score   
    } 
  }
}


void AStar::recompute_node(const ForestNode & node, 
                           const Hypothesis & h, 
                           double original_score) {
  _num_recompute++;
  // can only produce dots 
  for (int i =0; i < node.num_in_edges(); i++) {
    // The edge to recompute

    const ForestEdge & edge = node.in_edge(i);
    double edge_value= _edge_weights.get_value(edge)    ;
    int last = edge.num_nodes() -1;

    //const ForestNode & top_node = edge.head_node();
    //const BestHyp & best_node_hypotheses = _memo_table.store[node.id()];
    
    vector<BestHyp> & best_edge_hypotheses = _memo_edge_table.store[edge.id()];
    
    if (!_memo_edge_table.has_value[edge.id()]) {
      best_edge_hypotheses.resize(edge.num_nodes());
      _memo_edge_table.has_value[edge.id()] = true;
    }


    int pos = -1;
    for (int j =0; j < edge.num_nodes(); j++) {
      if (edge.tail_node(j).id() == node.id()) 
        pos = j;
    }
    assert (pos!=-1);
    
    Location * l = alloc_loc();
    double score = original_score;

    if (pos == last) {      
      l->location = NODE;
      l->node_id = edge.head_node().id();
      score += edge_value;
    } else {
      l->edge_id = edge.id();
      l->edge_pos = pos; 
      l->location = EDGE;
    }
    

    // at beginning of edge, advance by one
    if (pos ==0) {
      Hypothesis * new_hyp = alloc_hyp(h.hook, h.right_side, &edge, _controller.dim());
      new_hyp->prev_hyp.push_back(h.id());

      add_to_queue(new_hyp, score, l);
      //best_edge_hypotheses[0].try_set_hyp(new_hyp,  score);
    } else {
      const BestHyp & last_best = best_edge_hypotheses[pos-1];
      vector <int> last_pos = last_best.join(h);
        
      for (int iter2 = 0; iter2< last_pos.size(); iter2++) {
     
        const Hypothesis & hyp2 = last_best.get_hyp(last_pos[iter2]); 
        double score2 = last_best.get_score(last_pos[iter2]);
   
        assert(hyp2.match(h));
        
        Hypothesis * join = alloc_hyp();
        join->dim = (_controller.dim());
          
        join->back_edge = &edge;
        assert(hyp2.prev_hyp.size() == pos);
        double join_score = score + score2 +
          _controller.combine(hyp2, h, *join);
        assert(join->prev_hyp.size() == pos+1);

        add_to_queue(join, join_score, l);
        //best_edge_hypotheses[pos].try_set_hyp(join, join_score);
      }    
    }
    // redo the forward edge, with this node fixed
    //forward_edge(edge, best_edge_hypotheses, pos, h.id());    
  }  
}



double AStar::best_path(NodeBackCache & back_pointers) {
  initialize_queue();
  Hypothesis * best_hyp;
  double best_score;
  main_loop(best_hyp, best_score);

  extract_back_pointers(_forest.root(), *best_hyp, _memo_table, back_pointers);
  if (DEBUG) {
    cout << "Numd pops " << _num_pops << endl;
    cout << "Num pushes " << _num_pushes << endl;
    cout << "Num recomputes " << _num_recompute << endl;
  }
  return best_score;
}



// void AStar::forward_edge(const ForestEdge & edge,  vector <BestHyp> & best_edge_hypotheses,  int pos_changed, int id) {
//   // viterbi to find best edge
//   for (int j=0; j < edge.num_nodes(); j++ ) {

//     const ForestNode & sub_node = edge.tail_node(j);

//     const BestHyp & local_best = _memo_table.store[sub_node.id()];
    
//     if (j ==0) {
//       // at leftmost, all are valid
       
//       for (int iter = 0; iter< local_best.size(); iter++) {
       
//         const Hypothesis & hyp = local_best.get_hyp(iter); 
//         if (j == pos_changed && hyp.id() != id )  continue;
//         double score = local_best.get_score(iter);
//         Hypothesis new_hyp(hyp.hook, hyp.right_side, &edge, _controller.dim(), hyp.is_new);
//         new_hyp.prev_hyp.push_back(hyp.id());
        
//         bool worked = best_edge_hypotheses[0].try_set_hyp(new_hyp,  score);
//         assert(worked);
//         //assert(best_edge_hypotheses->has_key( new_hyp.id()));
//       }
      
//     } else {
//       // match hook sig at left 
          
//       for (int iter = 0; iter< local_best.size(); iter++) {
//         //if (!local_best.has_key(iter)) continue;           
//         const Hypothesis & hyp1 = local_best.get_hyp(iter); 
//         double score1 = local_best.get_score(iter);
//          if (j == pos_changed && hyp1.id() != id )  continue;

//         BestHyp & last_best = best_edge_hypotheses[j-1];
//         vector <int> pos = last_best.join(hyp1);
        
//         for (int iter2 = 0; iter2< pos.size(); iter2++) {
          
//           const Hypothesis & hyp2 = last_best.get_hyp(pos[iter2]); 
//           double score2 = last_best.get_score(pos[iter2]);
   
//           assert(hyp2.match(hyp1));
          
//           Hypothesis join(_controller.dim());
                    
//           join.back_edge = &edge;
//           assert(hyp2.prev_hyp.size() == j);
//           double join_score = score1 + score2 +
//             _controller.combine(hyp2, hyp1, join);
//           assert(join.prev_hyp.size() == j+1);

//           /*cout << "For Combine " << endl;
//           show_hyp(hyp1);
//           show_hyp(hyp2);
//           cout << "For " <<  join_score <<endl; 
//           show_hyp(join);
//           */
             
//           best_edge_hypotheses[j].try_set_hyp(join, join_score);
//         }
//       }
//     }
//   }
// }
