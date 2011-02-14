
#include "CubePruning.h"
#include "EdgeCache.h"
#include "Forest.h"

#include "Decode.h"
#include "util.h"
#include "time.h"
#include "ExtendCKY.h"
#include <iomanip>
#include "AStar.h"
#include "common.h"
#include <boost/foreach.hpp>
#define TIMING 0
#define IS_TRY 0
//#define PROJECT 1si
//#define TRIPROJECT 0
#define DEBUG 0
#define SIMPLE_DEBUG 0
#define GREEDY 0
#define BACK 2

#define foreach BOOST_FOREACH



class SplitHeuristic : public Heuristic {
public :
  SplitHeuristic(const Cache <Hypernode, BestHyp> & outside_scores, 
                 Cache <Hyperedge, vector<BestHyp> >  & outside_edge_scores): 
    _outside_scores(outside_scores),
    _outside_edge_scores(outside_edge_scores){}

  int lower_id(const Hypothesis & hyp) const {
    return Hypothesis(hyp.hook.project(BACK, 2), 
                      hyp.right_side.project(BACK, 2)).id();
  }

  inline bool has_value(const Location & l, const Hypothesis & hyp) const  {
    if (l.location == NODE) {
      int node_id = l.node_id;
      int low_id = lower_id(hyp);
      const BestHyp & bhyp = _outside_scores.store[node_id];
      return bhyp.has_id(low_id);
    } else if (l.location == EDGE)  {
      
      int edge_id = l.edge_id;
      int low_id = lower_id(hyp);
      //cout << "Looking for " <<  edge_id << " " << low_id << " " << l.edge_pos << endl;
      const BestHyp & bhyp = _outside_edge_scores.store[edge_id][l.edge_pos];
      return bhyp.has_id(low_id);
    }
    assert(false);
    return 0.0;
  }
  
  inline double get_value(const Location & l, const Hypothesis & hyp) const {
    if (l.location == NODE) {
      int node_id = l.node_id;
      return _outside_scores.store[node_id].get_score_by_id(lower_id(hyp));
    } else if (l.location == EDGE ) {
      int edge_id = l.edge_id;
      return _outside_edge_scores.store[edge_id][l.edge_pos].get_score_by_id(lower_id(hyp));
    }
    assert(false);
    return 0.0;
  }


private: 
  const Cache <Hypernode , BestHyp> & _outside_scores;
  const Cache <Hyperedge, vector<BestHyp> >  & _outside_edge_scores;
};


class SplitController : public Controller {
public:
  SplitController (const Subproblem & s, const ForestLattice & l, bool two_classes) : _subproblem(s), _lattice(l), _two_classes(two_classes) {
    assert(_subproblem.projection_dims > BACK || !two_classes);
    if (_two_classes) {
      _classes.resize(2);
      //_inner_projection;
      for (int i=0; i < BACK; i++) {
        _classes[0].push_back(i);
        _inner_projection.push_back(0);
      }
      for (int i=BACK; i < _subproblem.projection_dims; i++) {
        _classes[1].push_back(i);
        _inner_projection.push_back(1);
      }
    } else {
      _classes.resize(_subproblem.projection_dims);
      for (int i=0; i < _subproblem.projection_dims; i++) {
        _inner_projection.push_back(i);
        _classes[i].push_back(i);
      }
    }     
  }

  int project_word(int w) const {
    return _inner_projection[_subproblem.project_word(w)];
  }

  inline int size()  const{
    int d = dim();
    return d * d* d *d;
  }

  inline int dim() const {
    return _classes.size();
  }

  void initialize_hypotheses(const Hypernode & node, vector <Hypothesis *> & hyps, vector <double> & scores) const {    
    int graph_id = _lattice.get_word_from_hypergraph_node(node.id());
    if (_subproblem.overridden[graph_id]) {
      
      for (int d2 = 0; d2 < dim(); d2++) {
        vector <int> hooks(2);
        vector <int> right_side(2);
        
        int w1 = _subproblem.overridden_by(graph_id);
          
        hooks[0] = project_word(w1);
        hooks[1] = d2;
        right_side[0] =project_word(graph_id);
        right_side[1] = hooks[0];
        Hypothesis * h = new Hypothesis(State(hooks, dim()), State(right_side,dim()));
        h->original_value = 0.0;
        hyps.push_back(h);
        scores.push_back(0.0);
          
      }
      
    } else {
      for (int d=0; d < dim(); d++) {
      
        assert(node.is_terminal());
        
        assert(_lattice.is_word(graph_id));
        for (int d2 = 0; d2 < dim(); d2++) {
            
          vector <int> hooks(2);
          vector <int> right_side(2);
                    hooks[0] = d;
          hooks[1] = d2;
          right_side[0] = project_word(graph_id);
          right_side[1] =d;
          Hypothesis * h = new Hypothesis(State(hooks, dim()), State(right_side,dim()));
          double score = _subproblem.best_score_dim_min(graph_id, 
                                                        _classes[d], 
                                                        _classes[d2]);
          
          if (score >=  1000) continue;            
          //if (_subproblem.projection_dims > 0) {
          //cout << graph_id << " " << d << " " << d2 << " " << score << " " << is_new<< endl;  
          //}
          
          h->original_value = score;
          
          hyps.push_back(h);
          scores.push_back(score);
          
          //cout << "Initial " << score << endl;
          
        }
      }
    }
  }

  
  void initialize_out_root(vector <Hypothesis *> & hyps, 
                           vector <double> & scores)  const {
    int s_first_projection= project_word(0);
    int s_projection= project_word(1);
    for (int d=0; d < dim(); d++) {
      for (int d2 = 0; d2 < dim(); d2++) {
        vector <int> hooks(2);
        vector <int> right_side(2);
        hooks[0] = s_projection;
        hooks[1] = s_first_projection;
        right_side[0] = d2;
        right_side[1] =d;

        Hypothesis * h = new Hypothesis(State(hooks,dim()), State(right_side,dim()));
        double my_score = 0.0;
        if (!_subproblem.overridden[_lattice.num_word_nodes-2]) {
          int id = _lattice.num_word_nodes-2;
          //cout << "Best " << hyp1.right_side << endl;
          my_score += 
            _subproblem.best_score_dim_min(id, 
                                           _classes[h->right_side._state[0]], 
                                           _classes[h->right_side._state[1]]);
        }
        //cout << "Middle " << my_score << endl;
      
        if (!_subproblem.overridden[_lattice.num_word_nodes-1]) {
          int id = _lattice.num_word_nodes-1;
          int d = project_word(_lattice.num_word_nodes-2);
       
          my_score += _subproblem.best_score_dim_min(id, 
                                                     _classes[d], 
                                                     _classes[h->right_side._state[0]]); 
        }
        hyps.push_back(h);
        scores.push_back(my_score);
      }
    }
  }

  double find_best( vector <Hypothesis *> & root_hyps, vector<double > & scores, 
                    Hypothesis & best_hyp) const {
    //BestHyp::const_iterator iter, check;
    double best = 1e20;
    //<s> projection
    int s_first_projection= project_word(0);
    int s_projection= project_word(1);
    for (uint iter = 0; iter< root_hyps.size(); iter++) {
      //if (!root_hyps.has_key(iter)) continue;
      
      const Hypothesis & hyp1 = *root_hyps[iter]; 
      double score1 = scores[iter];
      if (hyp1.hook._state[0] != s_projection || hyp1.hook._state[1] != s_first_projection) {
        continue;
      }
      
      double my_score = score1;
      //cout << "Before " << my_score << endl;
      // factor in last score
      
        
      if (!_subproblem.overridden[_lattice.num_word_nodes-2]) {
        int id = _lattice.num_word_nodes-2;
        //cout << "Best " << hyp1.right_side << endl;
        my_score += _subproblem.best_score_dim_min(id, 
                                                   _classes[hyp1.right_side._state[0]], 
                                                   _classes[hyp1.right_side._state[1]]);
      }
      //cout << "Middle " << my_score << endl;
      
      if (!_subproblem.overridden[_lattice.num_word_nodes-1]) {
        int id = _lattice.num_word_nodes-1;
        int d = project_word(_lattice.num_word_nodes-2);
        
        my_score += _subproblem.best_score_dim_min(id, 
                                                   _classes[d],
                                                   // TODO: FIX ME
                                                   _classes[hyp1.right_side._state[0]]); 
      }
      

      
      //cout << "After " << my_score << endl;
      if (my_score < best) {
        best = my_score;
        best_hyp = hyp1;
      }
    }
    

    return best;
  }

private:
  const Subproblem & _subproblem;
  const ForestLattice & _lattice;
  bool _two_classes;
  vector <vector <int> >  _classes;
  vector <int> _inner_projection;
};


void Decode::update_weights(const wvector & update,  wvector * weights ) {
  vector <int> u_pos1, u_pos2;
  vector <float> u_val1, u_val2;
  for (wvector::const_iterator it = update.begin(); it != update.end(); it++) {
    if (it->second == 0.0) continue;
    if (IS_TRY && it->first >= GRAMSPLIT2 ) {
      u_pos1.push_back(it->first - GRAMSPLIT2);
      u_val1.push_back(it->second);
      u_pos2.push_back(it->first - GRAMSPLIT2);
      u_val2.push_back(-it->second);      
    } else if (it->first >= GRAMSPLIT ) {
      u_pos2.push_back(it->first - GRAMSPLIT);
      u_val2.push_back(-it->second);
    } else {
      u_pos1.push_back(it->first);
      u_val1.push_back(-it->second);
    }
  }
  //cout << "UPDATING WEIGHTS " << u_pos1.size() << " " << u_pos2.size() <<endl;
  clock_t begin, end;
  if (TIMING) {
    begin=clock();
  }
  _subproblem->update_weights(u_pos1, u_val1, true);
  _subproblem->update_weights(u_pos2, u_val2, false);
  if (TIMING) {
    end=clock();
    cout << "UPDATE TIME: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
  }
  _lagrange_weights = weights;
  
}

vector <int > Decode::get_lex_lat_edges(int edge_id) {
  //  assert(false);
  vector <int> all = get_lat_edges(edge_id);
  vector <int> ret;
  for (unsigned int i=0; i< all.size(); i++) {
    if (_lattice.is_word(all[i])) {
      ret.push_back(all[i]);
    }
  }
  return ret;
}

//vector <int > Decode::get_lat_ndoes(int edge_id) {
//return _lattice.original_nodes[edge_id];
//}

vector <int > Decode::get_lat_edges(int edge_id) {
  //  assert(false);
  return _lattice.original_edges[edge_id];
}


void Decode::debug(int start_from, int dual_mid, int dual_end, int primal_mid, int primal_end) {
  if (SIMPLE_DEBUG) {
    bool same = primal_end == dual_end;
    bool same2 = primal_mid == dual_mid;
    string diff = " ";
    if (!same) {
      diff = "E";
    } 
    if (!same2) {
      diff = "M";
    }
    if (!same && ! same2) {
      diff = "B";
    }
    string over = "|";
    if (_subproblem->overridden[start_from]) {
      over = "O";
    }
    cout << setiosflags(ios::left);
    cout << setw(3) << primal_end << " " << setw(2) << _subproblem->project_word(primal_end) << " " << setw(15) << _lattice.get_word(primal_end) <<  " " << setw(3) << primal_mid << " " << setw(2) << _subproblem->project_word(primal_mid) << " " << setw(15) << _lattice.get_word(primal_mid) ;
    cout << ("  "+diff+"   ")
         << setw(3) << dual_end << " " << setw(2) << _subproblem->project_word(dual_end) << " " << setw(15)<<  _lattice.get_word(dual_end) << " " << setw(3) << dual_mid <<" " << setw(2)<< _subproblem->project_word(dual_mid) << " "<< setw(15) <<_lattice.get_word(dual_mid) 
         << " "<<over<<" " <<setw(15)<< _lattice.get_word(start_from) << endl;
  
  }

  
}



void Decode::greedy_projection(int dual_mid, int dual_end, int primal_mid, int primal_end) {
  if (!GREEDY) {
    if (_maintain_constraints) {
      if (primal_mid != dual_mid) {
        int w1 = dual_mid;
        int w2 = primal_mid;
        //if (w1 < w2) 

          if (_subproblem->projection_dims > 1 && 
              (_constraints[w1].find(w2) != _constraints[w1].end() ||
               _constraints[w2].find(w1) != _constraints[w2].end() 
               )) {
            //assert(/false);
            //cout << "COLORING fail " << w1 << " " << w2<< endl;
          }

          _constraints[w2].insert(w1);
          //else 
          _constraints[w1].insert(w2);

      }
      
      if (primal_end != dual_end) {
        int w1 = dual_end;
        int w2 = primal_end;

          if (_subproblem->projection_dims > 1 && 
              (_constraints[w1].find(w2) != _constraints[w1].end() ||
               _constraints[w2].find(w1) != _constraints[w2].end() 
               )) {
            //assert(false);
            //cout << "COLORING fail " << w1 << " " << w2<<endl;
          }

        //if (w1 < w2) 
        _constraints[w2].insert(w1);
        //else 
        _constraints[w1].insert(w2);
      }
    }
  } else {
    /*if (primal_mid != dual_mid) {
      if (_projection[dual_mid] == _projection[primal_mid] )
        if (rand() %2 == 0) {
          _projection[dual_mid] = (_projection[dual_mid] + 1) % _proj_dim; 
        } else {
          _projection[primal_mid] = (_projection[primal_mid] + 1) % _proj_dim; 
        }
      //_subproblem->separate(dual_mid, primal_mid);
    }

    if (primal_end != dual_end) {
      if (_projection[primal_end] == _projection[dual_end] )
        if (rand() %2 == 0) {
          _projection[dual_end] = (_projection[dual_end] + 1) % _proj_dim; 
        } else {
          _projection[primal_end] = (_projection[primal_end] + 1) % _proj_dim; 
        }
      //_subproblem->separate(dual_end, primal_end);
      }*/
  }
}

void Decode::add_subgrad(wvector & subgrad, int start_from, int mid_at, int end_at, bool first) {
  //cout << _lattice.get_word(end_at) << " "<<_lattice.get_word(mid_at) << " " << _lattice.get_word(start_from) << " " <<endl;      
  if (! first) {
  vector <int > between1 = _subproblem->get_best_nodes_between(start_from,mid_at, true);
  //cout << "Size!" << between1.size() << endl;
  //double lag_total =0.0;
  for (int k = between1.size() -1 ; k >=0 ; k--) {        
    int node_id = between1[k];
    assert(!_lattice.is_word(node_id) || node_id == mid_at);
    //cout << "DEC!" << node_id << endl;
    assert(node_id >= 0);
     

    if (IS_TRY) {
      subgrad[node_id + GRAMSPLIT2] += 1;
    }

    if (DEBUG) {
      cout << _lattice._edge_label_by_nodes[node_id] << " (" << node_id << ") ";  
      
      lag_total += (*_lagrange_weights)[node_id ] ;
      
    }

      subgrad[node_id] -= 1;
    
  }
  
  if (DEBUG) {
    cout << endl;
  }
  }
  const vector <int> between2 = _subproblem->get_best_nodes_between(mid_at, end_at, false);
  
  //cout << "Size!" << between2.size() << endl;
  for (int k = between2.size()-1; k >=0 ; k--) {
    int node_id = between2[k];
    //cout << "DEC!" << node_id << endl;
    assert(node_id >= 0);
    subgrad[node_id + GRAMSPLIT] -= 1;
   
    if (DEBUG) {
      lag_total += (*_lagrange_weights)[node_id + GRAMSPLIT] ;
      
      cout << _lattice._edge_label_by_nodes[node_id] << " [" << node_id << "] ";
    }

    if (IS_TRY) {
      subgrad[node_id + GRAMSPLIT2] -= 1;
    }

  }
  if (DEBUG) {
    cout << endl;
  }

  if (DEBUG) {
    double lm_score = (LMWEIGHT) * _subproblem->word_prob_reverse(start_from, mid_at, end_at);
    //cout << lm_score << " " << -lag_total << " " << _subproblem->cur_best_score[start_from] << endl;

    cout << "SCORE " << start_from << " " << _lattice.get_word(end_at) << " " << _lattice.get_word(mid_at) 
         << " "<< _lattice.get_word(start_from) << " " << start_from <<" " << mid_at << " " << end_at << " " << 
      _lattice.lookup_word(start_from) << " " <<   -lag_total << " " << lm_score << " " << lm_score - lag_total<<endl;

    lm_total += lm_score;
    if (!_subproblem->overridden[start_from]) { 
      o_total +=  _subproblem->best_score(start_from, mid_at, end_at);
    }
  }


  //cout << "SCORE " << start_from << " " << _lattice.get_word(end_at) << " " << _lattice.get_word(mid_at)
  //    << " "<< _lattice.get_word(start_from) << " " << start_from <<" " << mid_at << " " << end_at << 
  // " "<< _lattice.get_hypergraph_node_from_word(end_at)<< " "<<_lattice.get_hypergraph_node_from_word(mid_at) << " " << _lattice.get_hypergraph_node_from_word(start_from)<< endl;

  //cout << "SCORE " << _subproblem->cur_best_score[start_from] << " " << _lattice.get_word(start_from) << " " << start_from << " " <<  _lattice.lookup_word(start_from) << " " << mid_at << " " << _lattice.get_word(mid_at)<< " "<< _lattice.lookup_word(mid_at) << " " << end_at << " " << _lattice.get_word(end_at) << " " << _lattice.lookup_word(end_at)<< " "<<endl;
}

void Decode::print_output(const wvector & subgrad) {
  int bis =0;
  int tris =0;
  
  for (wvector::const_iterator it = subgrad.begin(); it != subgrad.end(); it++) {
    if (it->second != 0.0) {
      cout << it->first << " " << it->second << " " << (*_lagrange_weights)[it->first]<< endl;
      if (it->first < GRAMSPLIT && _lattice.is_word(it->first)) { 
        cout << _lattice.get_word(it->first) << " " << _subproblem->project_word(it->first) <<  endl;
        bis++;
      } else if (it->first < GRAMSPLIT && _lattice.is_word(it->first)) { 
        cout << _lattice._edge_label_by_nodes[it->first] <<  endl;
      }
      if (it->first >= GRAMSPLIT && it->first < GRAMSPLIT2 && _lattice.is_word(it->first - GRAMSPLIT)) { 
        cout << _lattice.get_word(it->first - GRAMSPLIT) << " " << _subproblem->project_word(it->first -GRAMSPLIT) << endl;
        tris++;
      } else if (it->first >= GRAMSPLIT && it->first < GRAMSPLIT2) { 
        cout << _lattice._edge_label_by_nodes[it->first- GRAMSPLIT] <<  endl;
      }
      if (it->first >= GRAMSPLIT2 &&  _lattice.is_word(it->first - GRAMSPLIT2)) { 
        cout << _lattice.get_word(it->first - GRAMSPLIT2) << " " << _subproblem->project_word(it->first -GRAMSPLIT2) << endl;
        tris++;
      } else if (it->first >= GRAMSPLIT2) { 
        cout << _lattice._edge_label_by_nodes[it->first- GRAMSPLIT2] <<  endl;
      }
      

    }
  }
  cout << bis << " " ;
  cout << tris << endl;
  //cout << endl << endl;
}

void Decode::solve(double & primal , double & dual, wvector & subgrad, int round, bool is_stuck, bool & bump_rate) {
  clock_t begin, end;
  if (TIMING) {
    cout << "Solving" << endl;
    begin=clock();
  }  
  if (TRIPROJECT) {
    if (round ==1) {
      _proj_dim = 1;
      //vector <int> _projection(_lattice.num_word_nodes);
      _projection.resize(_lattice.num_word_nodes);
      for (int i=0; i < _lattice.num_word_nodes; i++) {
        _projection[i] = 0; // _subproblem->rand_projection(1);
      }
    }

    

    //cout << "DSTUCK " << is_stuck << " " << _maintain_constraints << endl;
    if ((round ==145 || is_stuck) && !_maintain_constraints) {
      //cout << "DUAL STUCK Round "<< round << endl;
      _maintain_constraints = true;
      _is_stuck_round = round;
      bump_rate = true;
    }


    /*_maintain_constraints =true;
    if (round >=3) {
      int limit = 3;
      _subproblem->projection_with_constraints(limit, _proj_dim, _constraints, _projection);
      } */
    /*
    if (round >=120) {
      int limit = 4;
      _subproblem->projection_with_constraints(limit, _proj_dim, _constraints, _projection);
    } 

    if (round >=125) {
      int limit = 5;
      _subproblem->projection_with_constraints(limit, _proj_dim, _constraints, _projection);
    } */

    
    //if (round >=_is_stuck_round + 40) {
    //int limit = 2;
    // _subproblem->projection_with_constraints(limit, _proj_dim, _constraints, _projection);
    //}
    if (round >=_is_stuck_round +50) {
      int limit = 25;
      _subproblem->projection_with_constraints(limit, _proj_dim, _constraints, _projection);
    } 
 
   /*if (round >=290) {
     //_proj_dim = 5;
     _subproblem->projection_with_constraints(5, _proj_dim, _constraints, _projection);
   } 
   if (round >=305) {
     //_proj_dim = 7;
     _subproblem->projection_with_constraints(14, _proj_dim, _constraints, _projection);
     } */
     

    //if (round ==200) {
    //_proj_dim = 3;
    //_projection =  _subproblem->rand_projection(3);
    //}
  }

  if (PROJECT || TRIPROJECT) {    
    _subproblem->project(_proj_dim, _projection);    
  }

  _subproblem->solve();

  o_total=0.0; lm_total=0.0; lag_total =0.0;


  if (TIMING) {
    end=clock();      
    cout << "SOLVE TIME: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
  }


  // now add the forward trigrams at each node
  if (TIMING) {
    begin=clock();  
  }
  EdgeCache penalty_cache(_forest.num_edges());
  
  foreach (HEdge edge, _forest.edges()) {// i < num_edges; i++) { 
    double total_score = 0.0;
    
    // trigram penalties 
    // added in newd guy
//     {
//       vector <int> lat_edges = get_lex_lat_edges(edge.id()); 
//       for (unsigned int j =0; j <  lat_edges.size(); j++) {
//         int graph_id = lat_edges[j];
//         //FIX ME
//         double score = _subproblem->best_score(graph_id, 0);
//         assert (score != 1000000);
//         //cout << graph_id << " " << score << endl;
//         total_score += score;          
//       }
//       //cout << edge.id()  << " " << total_score << endl;
//     }
    // self penalties
    {
      vector <int> lat_edges = get_lat_edges(edge->id()); 
      for (unsigned int j =0; j < lat_edges.size(); j++) {
        int lat_id = lat_edges[j];
        total_score += (*_lagrange_weights)[lat_id];
        total_score += (*_lagrange_weights)[GRAMSPLIT + lat_id ];
      }
    }    
    penalty_cache.set_value(*edge, total_score); 
    
  }
  if (TIMING) {
    end=clock();      
    cout << "PENALTY CACHE: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
    
    begin=clock();  
  }
  HypergraphAlgorithms ha(_forest);
  EdgeCache * total = ha.combine_edge_weights(penalty_cache, *_cached_weights);
  NodeCache scores(_forest.num_nodes()), scores2(_forest.num_nodes());
  NodeBackCache back_pointers(_forest.num_nodes()), 
    back_pointers2(_forest.num_nodes()), back_pointers3(_forest.num_nodes());;

  if (TIMING) {
    end=clock();      
    cout << "COMBINE: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
  }
  //double simple = best_path(_forest, *_cached_weights, scores2, back_pointers2);
  
  //cout << "SIMPLE Score " << simple << endl; 
  if (TIMING) {
    begin=clock();  
  }

  bool run_astar = _subproblem->projection_dims > BACK;
  SplitController c(*_subproblem, _lattice, run_astar);
  ExtendCKY ecky(_forest, *total, c);
  //ecky.set_params(total, &c);

  if (run_astar) { 
    
    dual = ecky.best_path(back_pointers2);
    
    if (SIMPLE_DEBUG) {
      cout << "Approx dual is "<< dual << endl;
    }
    //Cache <ForestNode, BestHyp> outside_scores(_forest.num_nodes());
    //Cache <ForestNode, BestHyp> outside_edge_memo_table(_forest.num_nodes());
    
    if (TIMING) { 
      end=clock();  
      cout << "INSIDE time: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
      begin = clock();
    }

    ecky.outside();

    if (TIMING) {
      end=clock();  
      cout << "OUTSIDE time: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
      begin = clock();
    }


    SplitHeuristic heu(ecky._outside_memo_table, 
                       ecky._outside_edge_memo_table);
    SplitController c_astar(*_subproblem, _lattice, false);
    AStar astar(_forest, c_astar, *total, heu);
    dual = astar.best_path(back_pointers);
    
    // for testing - takeout!
    /*SplitController c2(*_subproblem, _lattice, false);
    ExtendCKY ecky2(_forest);
    ecky2.set_params(total, &c2);
    double dual_test = ecky2.best_path(back_pointers3);
    
    assert(fabs(dual - dual_test) < 1e-4); */
    
  } else {
    dual = ecky.best_path(back_pointers);
    // use simple
    //dual = best_path(_forest, *total, scores, back_pointers);
    
  }
  //ecky.fill_back_pointers()

  //dual = best_path(_forest, *total, scores, back_pointers);
  


  //cout << "INITIAL DUAL Score" << dual << endl;

  HEdges used_edges = ha.construct_best_edges(back_pointers); 
  vector <const Hypernode *> tmp_words = ha.construct_best_fringe(back_pointers); 
  vector <const ForestNode *> used_words;
  foreach (const Hypernode * word, tmp_words ) {
    used_words.push_back((ForestNode*)word);
  }

  //vector <int> all_nodes = construct_best_node_order(_forest, back_pointers);
  //cout << endl;
  if (TIMING) {
    end=clock();  
    cout << "Parse time: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
  }
  //assert abs(best -best_fv.dot(cur_weights)) < 1e-4, str(best) + " " + str(best_fv.dot(cur_weights))
  //lagrangians_parse = 0.0
  //lagrangians_other = 0.0

  //tri_pairs = [];
  
  begin=clock();  

  foreach (HEdge edge, used_edges) { // int i =0; i < used_edges.size(); i++){ 
    int edge_id= edge->id(); //used_edges[i]->id();
    // + lagrangians (FROM PARSE SIDE)
    vector <int> lat_edges = get_lat_edges(edge_id); 
    for (uint j =0; j < lat_edges.size(); j++) {
      int lat_id = lat_edges[j];
      subgrad[lat_id] += 1;
      subgrad[GRAMSPLIT + lat_id ] += 1;
      //parse_states.set(lat_id);
    }
  }
  vector <string> used_strings;
  //cout << "*PRIMAL* " << round << " ";
  used_strings.push_back("<s>");
  used_strings.push_back("<s>");
  
  foreach (HNode used, used_words) {  // int i =0; i < used_words.size(); i++) {
    //cout <<used_words[i]->word() << " ";
    used_strings.push_back(((const ForestNode*)used)->word());
  }
  //cout <<endl;
  used_strings.push_back("</s>");
  used_strings.push_back("</s>");
  //assert(used_words.size() > 3);

  vector <int> used_lats;
  used_lats.push_back(0);
  used_lats.push_back(1);
  foreach (HNode word_node, used_words) {
    used_lats.push_back(_lattice.get_word_from_hypergraph_node( word_node->id()));
  }
  used_lats.push_back(_lattice.num_word_nodes-2);
  used_lats.push_back(_lattice.num_word_nodes-1);


  double cost_total = 0.0;
    // - lagrangians (FROM LM SIDE)
    //vector <int> lex_lat_edges = get_lex_lat_edges(edge_id); 
    //cout << "lex lat " << lex_lat_edges.size() << endl;
  //cout << endl;
  //foreach (HNode word_node, used_words) {
  for (uint j =0; j < used_words.size(); j++) {
    HNode word_node = used_words[j];
    //for (int j =0; j < lex_lat_edges.size(); j++) {
    int graph_id = _lattice.get_word_from_hypergraph_node(word_node->id());
    //cout << used_words[j]->word() << " ";
    // will be explained by another node
    if (_subproblem->overridden[graph_id]) {
      //cout << "skip" << graph_id << " ";
      continue;
    }
        
    //Bigram forbigram = _subproblem->get_best_trigram(graph_id);
    
    // Get lattice Lex node directly before me
    //int pos = -1;
    uint node_for_graph_id =(uint) _lattice.get_hypergraph_node_from_word(graph_id);
    int previous_graph_id; 
    int pre_previous_graph_id; 
    int next_graph_id;
    //cout << "b ";
    //foreach (HNode word_node, used_words) { 
    //int id = _lattice.get_word_from_hypergraph_node(word_node->id());
      //cout << _subproblem->project_word(id) << " " ;
    //}
      //cout << endl;
    
    //foreach (HNode word_node, used_words) { 
    for (uint p = 0; p < used_words.size() ; p++) {
      if (node_for_graph_id == used_words[p]->id()) {
        // assume projected consistency with previous node
        if (p == 0) {
          // <s>
          pre_previous_graph_id = 0;
          previous_graph_id = 1;
          next_graph_id = _lattice.get_word_from_hypergraph_node(used_words[p+1]->id());;
        } else if (p==1) {
          pre_previous_graph_id = 1;
          previous_graph_id = _lattice.get_word_from_hypergraph_node(used_words[p-1]->id());
          next_graph_id = _lattice.get_word_from_hypergraph_node(used_words[p+1]->id());
        } else if (p == used_words.size()-1) {
          pre_previous_graph_id = _lattice.get_word_from_hypergraph_node(used_words[p-2]->id());
          previous_graph_id = _lattice.get_word_from_hypergraph_node(used_words[p-1]->id());;
          next_graph_id = _lattice.num_word_nodes-2;
        } else {
          pre_previous_graph_id = _lattice.get_word_from_hypergraph_node(used_words[p-2]->id());
          previous_graph_id = _lattice.get_word_from_hypergraph_node(used_words[p-1]->id());
          next_graph_id = _lattice.get_word_from_hypergraph_node(used_words[p+1]->id());
        }
        break;
      } 
    }
    
    int start_from = graph_id;
    int mid_at = _subproblem->best_one(graph_id, previous_graph_id, pre_previous_graph_id);
    int end_at = _subproblem->best_two(graph_id, previous_graph_id, pre_previous_graph_id);      
    
    //cout << _lattice.get_word(end_at) << " "<<_lattice.get_word(mid_at) << " " << _lattice.get_word(start_from) << " " <<endl;
    

    cost_total += _subproblem->best_score(start_from, mid_at, pre_previous_graph_id);    
    
    /*if (false && previous_graph_id == 1) {
      // unconstrained for now
      mid_at = _subproblem->cur_best_one[graph_id][0];
      end_at = _subproblem->cur_best_two[graph_id][0];
      assert(_lattice.is_word(mid_at));
      cost_total += _subproblem->cur_best_score[graph_id];
      //cout << " " << _subproblem->cur_best_score[graph_id] << " ";
    } else {

      //cout << " " << _subproblem->best_score(graph_id, mid_at) << " ";
      }*/
    
    
    add_subgrad(subgrad, start_from, mid_at, end_at, false);
    debug(start_from, mid_at, end_at, used_lats[j+2-1], used_lats[j+2-2]);
    greedy_projection(mid_at, end_at, used_lats[j+2-1], used_lats[j+2-2]);
    
    // the next node is determined by my choice
    if (_subproblem->overridden[next_graph_id]) {
      int end_at = mid_at;      
      int mid_at = start_from;
      int start_from = next_graph_id;
      add_subgrad(subgrad, start_from, mid_at, end_at, false);
      debug( start_from, mid_at, end_at, used_lats[j+2], used_lats[j+2-1]);
      greedy_projection( mid_at, end_at, used_lats[j+2], used_lats[j+2-1]);
    }
    assert(fabs(lm_total - (o_total + lag_total)) < 1e-3);
  }
  
  
  double edge_total= 0.0;
  {
    foreach (HNode node, _forest.nodes()) { //for (int i =0; i < _forest.num_nodes(); i++) {
      //const ForestNode & node = _forest.get_node(i);
    
      if (!((ForestNode* )node)->is_word() && back_pointers.has_key(*node)) {
        //assert(bcache.get_value(node) == bcache2.get_value(node));
        edge_total += total->get_value(*(back_pointers.get_value(*node)));
      }
    }
  }
  //cout << endl;
  
  
  //cout << "Early dual " << dual <<endl;

  //cout << "Primal " << primal << endl; 
  // BOUNDARY CONDITIONS
  //bounds = [(0,1), (self.graph.size()-1,self.graph.size()-2) ]
  //bounds.reverse()
    
  // END BOUNDARY
  // add in the last node, and second to last (trigram)
  // over counted at k
  //feat = "2UNI:"+str(bounds[1][0])
  {
    int feat = 0 + GRAMSPLIT ;
    subgrad[feat] += 1; 
    dual += (*_lagrange_weights)[feat];  
    cost_total += (*_lagrange_weights)[feat];  
    
    if (IS_TRY) {
      // overcounted
      subgrad[0 + GRAMSPLIT2] +=1;
    }
  }


  // over counted at k and j  
  {
    int feat = 1;
    subgrad[feat] += 1 ;
    dual += (*_lagrange_weights)[feat];
    cost_total += (*_lagrange_weights)[feat];  
    
    feat = 1 + GRAMSPLIT; //"2UNI:"+str(bounds[1][1]);
    subgrad[feat] += 1 ;

    dual += (*_lagrange_weights)[feat]  ;
    cost_total += (*_lagrange_weights)[feat];
  }

    // START BOUNDARY
  if (!_subproblem->overridden[_lattice.num_word_nodes-2]) {
    int id = _lattice.num_word_nodes-2;

    //assert(!_subproblem->overridden[id]);
    // second word <s>
    int start_from =id;
    int mid_at = _subproblem->best_one(id, 
                                       _lattice.get_word_from_hypergraph_node(used_words[used_words.size()-1]->id()),
                                       _lattice.get_word_from_hypergraph_node(used_words[used_words.size()-2]->id()));
    int end_at = _subproblem->best_two(id, 
                                       _lattice.get_word_from_hypergraph_node(used_words[used_words.size()-1]->id()),
                                       _lattice.get_word_from_hypergraph_node(used_words[used_words.size()-2]->id()));
    
    //cout << start_from << " " << mid_at << " " << end_at << endl; 
    //cheat on last one (unconstrained)
    //int mid_at = _subproblem->best_one(id, used_words[used_word.length]);//_subproblem->best_one(id, previous_graph_id);
    //double score = _subproblem->cur_best_score[id];
    cost_total += _subproblem->best_score(start_from, mid_at, end_at);//_lattice.get_word_from_hypergraph_node(used_words[used_words.size()-1]->id()));
    //cout << "Best " << _subproblem->project_word(_lattice.get_word_from_hypergraph_node(used_words[used_words.size()-1]->id())) << endl;
    //cout << "START2" <<score << endl;
    //dual += score;

    add_subgrad(subgrad, start_from, mid_at, end_at, false);
    debug(start_from, mid_at, end_at, used_lats[used_strings.size()-3], used_lats[used_strings.size()-4]);
    greedy_projection(mid_at, end_at, used_lats[used_strings.size()-3], used_lats[used_strings.size()-4]);
  }

  //first word <s>
  {
    int id = _lattice.num_word_nodes-1;
    int start_from = id;
    //vector <int> lex_lat_edges = get_lex_lat_edges(id); 
    int mid_at = _lattice.num_word_nodes-2;
    int end_at;
    if (!_subproblem->overridden[id]) {
      end_at = _subproblem->best_two(id, mid_at,_lattice.get_word_from_hypergraph_node(used_words[used_words.size()-1]->id()));
    } else {
      end_at = _subproblem->best_one(mid_at, 
                                       _lattice.get_word_from_hypergraph_node(used_words[used_words.size()-1]->id()),
                                       _lattice.get_word_from_hypergraph_node(used_words[used_words.size()-2]->id())
                                       );
    }


    //cout << start_from << " " << mid_at << " " << end_at << endl; 
    if (!_subproblem->overridden[id]) {
      // Overridden (no score)
      //double score = _subproblem->cur_best_score[id];
      //dual += score;
      cost_total += _subproblem->best_score(id, mid_at, end_at);
      o_total += cost_total;
    }
    add_subgrad(subgrad, start_from, mid_at, end_at, true);
    debug(start_from, mid_at, end_at, used_lats[used_strings.size()-2], used_lats[used_strings.size()-3]);
    greedy_projection(mid_at, end_at, used_lats[used_strings.size()-2], used_lats[used_strings.size()-3]);
    
    /*vector <int> between2 = _subproblem->get_best_nodes_between(mid_at, end_at, false);
    for (int k = between2.size() -1; k >=0; k--) {
        int node_id = between2[k];
        subgrad[node_id + GRAMSPLIT] -= 1;
        if (DEBUG) {
          cout << _lattice._edge_label_by_nodes[node_id] << " ";
          lag_total += (*_lagrange_weights)[node_id + GRAMSPLIT];
        }
        
        if (IS_TRY) {
          subgrad[node_id + GRAMSPLIT2] -= 1;
        }

    }
    if (DEBUG) {
      double lm_score = (LMWEIGHT) * _subproblem->word_prob_reverse(start_from, mid_at, end_at);
      lm_total += lm_score;
      cout << "SCORE " << start_from << " " << _lattice.get_word(end_at) << " " << _lattice.get_word(mid_at)
           << " "<< _lattice.get_word(start_from) << " " << start_from <<" " << mid_at << " " << end_at << endl;
           }*/

  }
  
  int size =0;
  for (wvector::const_iterator it = subgrad.begin(); it != subgrad.end(); it++) {
    if (it->second != 0.0) {
      size++;
      //cout << it->first << " " << it->second << endl;
    }
  }

  //cout << "*DECODE* " << round << " " << _subproblem->projection_dims << " " << size << endl;  
  //assert(fabs(lm_total - o_total) < 1e-4);
  
  if (DEBUG) {
    
    cout << "DUAL LM: " << lm_total << endl;
    cout << "DUAL LM (check): " << o_total + lag_total << endl;
    cout << endl;
  }
  primal = compute_primal(used_edges, used_words);

  assert(fabs((cost_total + edge_total) - dual) < 1e-4);

  //print_output(subgrad);

  if (TIMING) {
   end=clock();
   cout << "Construct lagrangian: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
  }

  //primal = 0.0;//self.compute_primal(best_fv, subtree)    
  if (TIMING) { 
    begin=clock();
  }

  if (TIMING) {
    end=clock();
    cout << "COMPUTE PRIMA: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
  }

  /*for (int i=0; i < used_words.size(); i++ ) {
    cout << used_words[i]->word() << " ";
  }
  cout << endl;
  */
  if (DEBUG || SIMPLE_DEBUG) {
    cout << "DUAL Score" << dual << endl;
    cout << "PRIMAL Score " << primal << endl;
    cout << endl;
  }
  assert((dual - primal) < 1e-3);
  if (dual - primal > 1e-3) {
    cout << "DUAL PRIMAL mismatch. You have a bug." << endl;
    exit(0);
  }
  //print_output(subgrad);
  //cout << endl;
  /*    if BEST:
      print "Best score", best
      print subtree
    if DEBUG:
      for (a,b,c) in tri_pairs:
        print a, b, c, self.weights["lm"]* self.lm.word_prob_bystr(strip_lex(c), strip_lex(a) + " " + strip_lex(b))
        
    


    if BEST:
      print "Primal", primal
      print "final best", best

    self.last_weights = cur_weights
    return ( ret, best, primal)*/ 
  //return subgrad;
}


double Decode::compute_primal(const HEdges used_edges, const vector <const ForestNode *> used_nodes) {
  double total= 0;
  for (unsigned int i=0; i < used_edges.size(); i++ ) {
    total += _cached_weights->store[used_edges[i]->id()] ;
  }

  vector <string> used_strings;
  used_strings.push_back("<s>");
  used_strings.push_back("<s>");
  for (uint i =0; i < used_nodes.size(); i++) {
    used_strings.push_back(used_nodes[i]->word());
  }
  used_strings.push_back("</s>");
  used_strings.push_back("</s>");
  double lm_score =0.0;
  
  for (uint i =0; i < used_strings.size()-2; i++) {
    VocabIndex context [] = {lookup_string(used_strings[i+1]), lookup_string(used_strings[i]), Vocab_None};
    if (DEBUG) {
      cout << "PRIMAL " << used_strings[i] << " " <<  used_strings[i+1]<< " " <<  used_strings[i+2] << " " << (LMWEIGHT) *   _lm.wordProb(lookup_string(used_strings[i+2]), context) << endl;
      
    }
    lm_score += _lm.wordProb(lookup_string(used_strings[i+2]), context);
  }
  if (DEBUG) {
    cout << "PRIMAL LMWEIGHT: " << (LMWEIGHT) *lm_score << endl;
    cout << endl;
  }
  //cout << "total " << total << endl;
  
  return total + (LMWEIGHT) *  lm_score;
}
int Decode::lookup_string(string word) {
  int max = _lm.vocab.numWords();
  int unk = _lm.vocab.getIndex(Vocab_Unknown);
  int ind = _lm.vocab.getIndex(word.c_str());
  if (ind == -1 || ind > max) { 
    return unk;
  } else {
    return ind;
  }
}

void Decode::sync_lattice_lm() {
  
  _cached_words = new Cache <Graphnode, int> (_lattice.num_word_nodes);
  int max = _lm.vocab.numWords();
  int unk = _lm.vocab.getIndex(Vocab_Unknown);
  //assert(false);
  for (int n=0; n < _lattice.num_word_nodes; n++ ) {
    if (!_lattice.is_word(n)) continue;
    
    //const LatNode & node = _lattice.node(n); 
    //assert (node.id() == n);
    string str = _lattice.get_word(n);
    int ind = _lm.vocab.getIndex(str.c_str());
    if (ind == -1 || ind > max) {
      //cout << "Unknown " << str << endl; 
      _cached_words->store[n] = unk;
    } else {
      _cached_words->store[n] = ind;
    }
  }
}



      //cout << "SCORE " << _lattice.get_word(end_at) << " " << _lattice.get_word(mid_at)<< " "<< _lattice.get_word(graph_id) << " " << graph_id << " " << _lattice.lookup_word(graph_id) << " " <<    _subproblem->cur_best_score[graph_id] << " "<<endl;
      
      //cout << "SCORE " << _subproblem->cur_best_score[graph_id] << " " << _lattice.get_word(graph_id) << " " << graph_id << " " <<  _lattice.lookup_word(graph_id) << " " << mid_at << " " << _lattice.get_word(mid_at)<< " "<< _lattice.lookup_word(mid_at) << " " << end_at << " " << _lattice.get_word(end_at) << " " << _lattice.lookup_word(end_at)<< " "<<endl;
      /*for (int k =0; k < _subproblem->cur_best_two[graph_id].size(); k++ ) {
        int e_at = _subproblem->cur_best_two[graph_id][k];
        int m_at = _subproblem->cur_best_one[graph_id][k];
        if (pos ==-1 || pos < 2) {
          cout << "ignored " <<endl;
          break;} 
        if (_lattice.get_word(e_at) == used_words[pos-2]->word()
            && _lattice.get_word(m_at) == used_words[pos-1]->word()) {
          end_at = e_at;
          mid_at = m_at;
          break;
        }
        }*/

      //cout << end_at << " " << mid_at << " " << start_from << endl; 
      //cout << "DUAL " << _lattice.get_word(end_at) << " " <<  
      //_lattice.get_word(mid_at) << " " <<  
      //_lattice.get_word(graph_id) << " " << 
      //_subproblem->cur_best_score[graph_id] << " " <<
      //(LMWEIGHT) *   _subproblem->word_prob_reverse(start_from, mid_at, end_at) << endl;
