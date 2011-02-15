
//#include "CubePruning.h"
#include "EdgeCache.h"
#include "Forest.h"

#include "Decode.h"
#include "time.h"
#include "ExtendCKY.h"
#include <iomanip>
#include "AStar.h"

#include "SplitDecoder.h"
#include "../common.h"
#define TIMING 0

#define DEBUG 0
#define SIMPLE_DEBUG 0
#define GREEDY 0
//#define BACK 2


// TODO - Make this support n-grams 
void Decode::update_weights(const wvector & update,  wvector * weights ) {
  
  // There are two sets of updates. 1 -> bigram, 2 -> trigram
  vector <int> u_pos1, u_pos2;
  vector <float> u_val1, u_val2;


  for (wvector::const_iterator it = update.begin(); it != update.end(); it++) {
    // Skip blank updates
    if (it->second == 0.0) continue;
   
    // Trigram weights are offset by GRAMSPLIT
    if (it->first >= GRAMSPLIT ) {
      u_pos2.push_back(it->first - GRAMSPLIT);
      u_val2.push_back(-it->second);
    } else {
      u_pos1.push_back(it->first);
      u_val1.push_back(-it->second);
    }
  }

  _subproblem->update_weights(u_pos1, u_val1, true);
  _subproblem->update_weights(u_pos2, u_val2, false);
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

vector <int > Decode::get_lat_edges(int edge_id) {
  return _lattice.original_edges[edge_id];
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

bool Decode::solve_ngrams(int round, bool is_stuck) {
  bool bump_rate = false;

  if (round ==1) {
    _proj_dim = 1;
    //vector <int> _projection(_lattice.num_word_nodes);
    _projection.resize(_lattice.num_word_nodes);
    for (int i=0; i < _lattice.num_word_nodes; i++) {
      _projection[i] = 0; // _subproblem->rand_projection(1);
    }
  }

    

  
  if ((round ==145 || is_stuck) && !_maintain_constraints) {
    //cout << "DUAL STUCK Round "<< round << endl;
    _maintain_constraints = true;
    _is_stuck_round = round;
    bump_rate = true;
  }


  if (round >=_is_stuck_round +50) {
    int limit = 25;
    _subproblem->projection_with_constraints(limit, _proj_dim, _constraints, _projection);
  }  
  
  
  _subproblem->project(_proj_dim, _projection);    
  _subproblem->solve();
  
  
  

  return bump_rate;
}


EdgeCache Decode::compute_edge_penalty_cache() {
  EdgeCache ret(_forest.num_edges());
  
  foreach (HEdge edge, _forest.edges()) { 
    double total_score = 0.0;
  
    // self penalties
    vector <int> lat_edges = get_lat_edges(edge->id()); 
    for (unsigned int j =0; j < lat_edges.size(); j++) {
      int lat_id = lat_edges[j];
      total_score += (*_lagrange_weights)[lat_id];
      total_score += (*_lagrange_weights)[GRAMSPLIT + lat_id ];
    }
        
    ret.set_value(*edge, total_score); 

  }
  return ret;
}

double Decode::best_modified_derivation(const EdgeCache & edge_weights, const HypergraphAlgorithms & ha, NodeBackCache & back_pointers) {
  
  // OPTIMIZATION: Only run A-Star when we have a hard problem (several dimensions)  
  bool run_astar = _subproblem->projection_dims > BACK;
  clock_t begin, end;
  SplitController c(*_subproblem, _lattice, run_astar);
  
  // Boiler plate for cky
  ExtendCKY ecky(_forest, edge_weights, c);
  

  if (run_astar) {     
    
    // 1) Run inside-outside viterbi to collect approximate max-marginals.
    
 
    // The back pointers from the heuristic run
    NodeBackCache  temp_back_pointers(_forest.num_nodes());
    double approx_dual = ecky.best_path(temp_back_pointers);
    
    if (SIMPLE_DEBUG) {
      cout << "Approx dual is "<< approx_dual << endl;
    }
    
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
      
    // 2) Create a heuristic out of the approximate outside values 
    SplitHeuristic heu(ecky._outside_memo_table, 
                       ecky._outside_edge_memo_table);


    // 3) Run astar on the full problem using outside as an admissable heuristic
    SplitController c_astar(*_subproblem, _lattice, false);
    AStar astar(_forest, c_astar, edge_weights, heu);
    return astar.best_path(back_pointers);
    
    // TESTING
    // SplitController c2(*_subproblem, _lattice, false);
    //     ExtendCKY ecky2(_forest);
    //     ecky2.set_params(total, &c2);
    //     double dual_test = ecky2.best_path(back_pointers3);
    
    //     assert(fabs(dual - dual_test) < 1e-4); 
    
    } else {

      // Otherwise just do it as simply as possible
      return  ecky.best_path(back_pointers);
      
    }
  }

wvector Decode::construct_parse_subgrad(const HEdges used_edges) {
  wvector subgrad;
  foreach (HEdge edge, used_edges) { 
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
  return subgrad;
}

void Decode::solve(double & primal , double & dual, wvector & subgrad, int round, bool is_stuck, bool & bump_rate) {
  clock_t begin, end;


  if (TIMING) {
    cout << "Solving" << endl;
    begin=clock();
  }  

  /****************************************** 
   * 1) Solve for  the individual ngrams
   ******************************************/


  bump_rate = solve_ngrams(round, is_stuck);
  

  if (TIMING) {
    end=clock();      
    cout << "SOLVE TIME: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
    begin=clock();  
  }
  
  // DEBUG
  o_total=0.0; lm_total=0.0; lag_total =0.0;

  /****************************************** 
   * 2) Compute the penalties for each hyperedge
   ******************************************/

  EdgeCache penalty_cache = compute_edge_penalty_cache(); 


  if (TIMING) {
    end=clock();      
    cout << "PENALTY CACHE: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;    
    begin=clock();  
  }

  

  if (TIMING) {
    begin=clock();  
  }


  /****************************************** 
   * 3) a) Find the best derivation including 
   *       the ngram costs.  
   *    b) Collect the target words along the fringe
   *       of the derivation
   ******************************************/
  
  HypergraphAlgorithms ha(_forest);
  NodeBackCache back_pointers(_forest.num_nodes());
  EdgeCache * total = ha.combine_edge_weights(penalty_cache, *_cached_weights);
  dual = best_modified_derivation(*total, ha, back_pointers);

  HEdges used_edges = ha.construct_best_edges(back_pointers); 
  vector <const ForestNode *> used_words;
  {
    vector <const Hypernode *> tmp_words = ha.construct_best_fringe(back_pointers); 
    foreach (const Hypernode * word, tmp_words ) {
      used_words.push_back((ForestNode*)word);
    }
  }

  vector <string> used_strings;

  // UGH! UGH! TODO get this <s> business out of here
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


  if (TIMING) {
    end=clock();  
    cout << "Parse time: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
    begin=clock();
  }  

  
  subgrad += construct_parse_subgrad(used_edges);



  double cost_total = 0.0;
  
  // - lagrangians (FROM LM SIDE)
  //vector <int> lex_lat_edges = get_lex_lat_edges(edge_id); 
  //cout << "lex lat " << lex_lat_edges.size() << endl;
  //cout << endl;
  
  //foreach (HNode word_node, used_words) {
  for (uint j =0; j < used_words.size(); j++) {
    HNode word_node = used_words[j];


    int graph_id = _lattice.get_word_from_hypergraph_node(word_node->id());

    // will be explained by another node
    if (_subproblem->overridden[graph_id]) {
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
  }
  
  int size =0;
  for (wvector::const_iterator it = subgrad.begin(); it != subgrad.end(); it++) {
    if (it->second != 0.0) {
      size++;
    }
  }
  
  if (DEBUG) {    
    cout << "DUAL LM: " << lm_total << endl;
    cout << "DUAL LM (check): " << o_total + lag_total << endl;
    cout << endl;
  }

  primal = compute_primal(used_edges, used_words);

  assert(fabs((cost_total + edge_total) - dual) < 1e-4);


  if (TIMING) {
   end=clock();
   cout << "Construct lagrangian: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
    begin=clock();

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
}
