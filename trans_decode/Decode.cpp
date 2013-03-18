#include <algorithm>
#include <iomanip>

#include "EdgeCache.h"
#include "Forest.h"
#include "Decode.h"
#include "time.h"
#include "ExtendCKY.h"

#include "AStar.h"

#include "SplitDecoder.h"
#include "LMNonLocal.h"
#include "dual_nonlocal.h"
#include "../common.h"

#define TIMING 0
#define DEBUG 0
#define SIMPLE_DEBUG 0
#define GREEDY 0

#define COLOR_WORDS 1


// TODO(srush) - Make this support n-grams
void Decode::update_weights(const wvector &update,
                            wvector *weights ) {
  // There are two sets of updates. 1 -> bigram, 2 -> trigram
  vector <int> u_pos1, u_pos2;
  vector <float> u_val1, u_val2;


  for (wvector::const_iterator it = update.begin();
       it != update.end();
       it++) {
    // Skip blank updates
    if (it->second == 0.0) continue;

    // Trigram weights are offset by GRAMSPLIT
    if (it->first >= GRAMSPLIT) {
      u_pos2.push_back(it->first - GRAMSPLIT);
      u_val2.push_back(-it->second);
    } else {
      u_pos1.push_back(it->first);
      u_val1.push_back(-it->second);
    }
  }
  _subproblem->update_weights(u_pos1, u_val1, 0);
  _subproblem->update_weights(u_pos2, u_val2, 1);
  _lagrange_weights = weights;
}

vector <int > Decode::get_lex_lat_edges(int edge_id) {
  vector <int> all = get_lat_edges(edge_id);
  vector <int> ret;
  for (unsigned int i = 0; i< all.size(); i++) {
    if (_lattice.is_word(all[i])) {
      ret.push_back(all[i]);
    }
  }
  return ret;
}

vector <int > Decode::get_lat_edges(int edge_id) {
  return _lattice.original_edges[edge_id];
}

void Decode::add_subgrad(wvector &subgrad,
                         int start_from,
                         int mid_at,
                         int end_at,
                         bool first) {
  double local_lag_total =0.0;
  if (! first) {
    vector <int > between1 =
      _subproblem->get_best_nodes_between(start_from,
                                          mid_at, 0);
    double b1_lag = 0.0;

    for (int k = between1.size() -1 ; k >=0 ; k--) {
      int node_id = between1[k];
      assert(!_lattice.is_word(node_id) || node_id == mid_at);
      assert(node_id >= 0);


      if (DEBUG) {
        b1_lag += (*_lagrange_weights)[node_id ] ;
      }
      // - Lagrangians
      subgrad[node_id] -= 1;
    }

    if (DEBUG) {
      assert(fabs(-b1_lag -
                  _subproblem->get_best_bigram_weight(start_from, mid_at, 0)) < 1e-3);
      local_lag_total +=  b1_lag;
      lag_total +=  b1_lag;
    }
  }
  const vector <int> between2 =
    _subproblem->get_best_nodes_between(mid_at, end_at, 1);
  double b2_lag = 0.0;
  for (int k = between2.size()-1; k >=0 ; k--) {
    int node_id = between2[k];
    assert(node_id >= 0);

    // Trigram lagrangian.
    subgrad[node_id + GRAMSPLIT] -= 1;

    if (DEBUG) {
      b2_lag += (*_lagrange_weights)[node_id + GRAMSPLIT] ;
    }
  }
  if (DEBUG) {
    assert((-b2_lag - _subproblem->get_best_bigram_weight(mid_at, end_at, 1)) < 1e-3);
    lag_total +=  b2_lag;
    local_lag_total +=  b2_lag;
  }

  if (DEBUG) {
    double lm_score = (lm_weight()) * _subproblem->word_prob_reverse(start_from, mid_at, end_at);
    double w =0;
    w += _subproblem->get_best_bigram_weight(start_from, mid_at, 0);
    w += _subproblem->get_best_bigram_weight(mid_at, end_at, 1);

    lm_total += lm_score;
    if (!_subproblem->overridden[start_from]) {
      o_total +=  _subproblem->best_score(start_from, mid_at, end_at);
    }
  }
}

void Decode::print_output(const wvector & subgrad) {
  int bis = 0;
  int tris = 0;

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

  if ((round==145 || is_stuck) && !_maintain_constraints) {
    _maintain_constraints = true;
    _is_stuck_round = round;
    bump_rate = true;
  }

  if (false && round >=_is_stuck_round + 50) {
    cerr << "PROJECTION!!" << " "  << round << endl;
    int limit = 25;
    _subproblem->projection_with_constraints(limit,
                                             _proj_dim,
                                             _constraints,
                                             _projection);
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
    vector<int> lat_edges = get_lat_edges(edge->id());
    //cerr << edge->label() << endl;
    //cerr << edge->head_node().id() << endl;
    for (unsigned int j = 0; j < lat_edges.size(); j++) {
      int lat_id = lat_edges[j];
      // cerr << lat_id << " ";
      // if (!_lattice.is_word(lat_id)) {
      //   cerr << _lattice._edge_label_by_nodes[lat_id] << endl;
      // } else {
      //   cerr << _lattice.get_word(lat_id)  << endl;
      // }
      // if (lat_id == 0) {
      //   cerr << "0000000000000" << endl;
      // }
      total_score += (*_lagrange_weights)[lat_id];
      total_score += (*_lagrange_weights)[GRAMSPLIT + lat_id];
    }
    //cerr << endl;
    ret.set_value(*edge, total_score);

  }
  return ret;
}

double Decode::best_modified_derivation(const EdgeCache & edge_weights,
                                        const HypergraphAlgorithms & ha,
                                        NodeBackCache & back_pointers) {

  // OPTIMIZATION: Only run A-Star when we have a hard problem (several dimensions)
  bool run_astar = _subproblem->projection_dims > BACK;
  //cerr << "projection dims " << _subproblem->projection_dims << endl;
  clock_t begin, end;
  SplitController c(*_subproblem, _lattice, run_astar);

  // Boiler plate for cky
  ExtendCKY ecky(_forest, edge_weights, c);

  if (run_astar) {
    cerr << "running astar" << endl;
    // 1) Run inside-outside viterbi to collect approximate max-marginals.

    // The back pointers from the heuristic run
    NodeBackCache  temp_back_pointers(_forest.num_nodes());
    if (TIMING) {
      begin=clock();
    }
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
  } else {
    // Otherwise just do it as simply as possible
    return  ecky.best_path(back_pointers);
  }
}

void Decode::remove_lm(int feat, wvector & subgrad, double & dual, double &cost_total) {
  subgrad[feat] -= 1;
  dual -= (*_lagrange_weights)[feat];
  cost_total -= (*_lagrange_weights)[feat];
  //cout << "Removing " << feat << endl;
}

wvector Decode::construct_lm_subgrad(const vector <const ForestNode *> &used_words,
                                     const vector <int> &used_lats,
                                     const vector<string> &used_strings,
                                     double & dual, double & cost_total) {
  wvector subgrad;

  // Walk along the fringe.
  for (uint j =0; j < used_words.size(); j++) {
    HNode word_node = used_words[j];
    int hnode_id = word_node->id();
    int graph_id =
      _lattice.get_word_from_hypergraph_node(word_node->id());

    if (_subproblem->is_overridden(graph_id)) continue;

    // Get lattice Lex node directly before me
    uint node_for_graph_id =
      (uint)_lattice.get_hypergraph_node_from_word(graph_id);

    if (j < ORDER - 1) continue;

    int pre_previous_graph_id =
      _lattice.get_word_from_hypergraph_node(used_words[j-2]->id());;
    int previous_graph_id =
      _lattice.get_word_from_hypergraph_node(used_words[j-1]->id());

    int next_graph_id = -1;

    // Is it in the last position? </s>
    if (j+1 != used_words.size()) {
        next_graph_id =
          _lattice.get_word_from_hypergraph_node(used_words[j+1]->id());
    }

    int start_from = graph_id;
    int mid_at = _subproblem->best_one(graph_id,
                                       previous_graph_id,
                                       pre_previous_graph_id);
    int end_at = _subproblem->best_two(graph_id,
                                       previous_graph_id,
                                       pre_previous_graph_id);


    cost_total +=
      _subproblem->best_score(start_from,
                              mid_at,
                              pre_previous_graph_id);

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
    debug(start_from, mid_at, end_at, used_lats[j-1], used_lats[j-2]);
    greedy_projection(mid_at, end_at, used_lats[j-1], used_lats[j-2]);

    // the next node is determined by my choice
    if (next_graph_id != -1 && _subproblem->is_overridden(next_graph_id)) {
      int end_at = mid_at;
      int mid_at = start_from;
      int start_from = next_graph_id;
      add_subgrad(subgrad, start_from, mid_at, end_at, false);
      debug( start_from, mid_at, end_at, used_lats[j], used_lats[j-1]);
      greedy_projection( mid_at, end_at, used_lats[j], used_lats[j-1]);
    }
    assert(fabs(lm_total - (o_total + lag_total)) < 1e-3);
  }



  // **************************
  // * BOUNDARY CONDITIONS
  // **************************
  // TODO - eliminate this nonsense

  //bounds = [(0,1), (self.graph.size()-1,self.graph.size()-2) ]
  //bounds.reverse()

  // END BOUNDARY
  // add in the last node, and second to last (trigram)
  // over counted at k
  //feat = "2UNI:"+str(bounds[1][0])

  // FULLBUILT: The graph will have <s_1> on twice, but the lat we only use it once in gramsplit, so fake it
  remove_lm(used_lats[0], subgrad, dual, cost_total);

  // FULLBUILT: The graph will have <e_1> on twice, but we use it only once
  remove_lm(used_lats[used_lats.size()-2] + GRAMSPLIT, subgrad, dual, cost_total);


  // FULLBUILT: The graph will have <e_2> on twice, but we never use it
  remove_lm(used_lats[used_lats.size()-1], subgrad, dual, cost_total);
  remove_lm(used_lats[used_lats.size()-1] + GRAMSPLIT, subgrad, dual, cost_total);

  // const Hypernode & root = _forest.root();
  // // only one edge at root
  // vector <int> lat_edges = get_lat_edges(root.edges()[0]->id());
  // //up = _forest.root();
  // //doww = _forest.root();
  // //foreach (int feat, lat_edges) {
  // typedef vector<int>::iterator iter;
  // iter min = min_element(lat_edges.begin(), lat_edges.end());
  // iter max = max_element(lat_edges.begin(), lat_edges.end());
  // remove_lm(*min, subgrad, dual, cost_total);
  // remove_lm(*min + GRAMSPLIT, subgrad, dual, cost_total);
  // remove_lm(*max, subgrad, dual, cost_total);
  // remove_lm(*max + GRAMSPLIT, subgrad, dual, cost_total);
    //}

  return subgrad;
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

void Decode::solve(const SubgradState & cur_state,
                   SubgradResult & result ) {
  clock_t begin, end, total_begin;

  //no_update = false;
  if (TIMING) {
    cout << "Solving" << endl;
    begin=clock();
    total_begin = clock();
  }

  /******************************************
   * 1) Solve for the individual ngrams
   ******************************************/


  result.bump_rate =
    solve_ngrams(cur_state.round, cur_state.is_stuck);


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
    cout << "PENALTY CACHE TIME: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
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

  EdgeCache *total = ha.combine_edge_weights(penalty_cache, *_cached_weights);

  result.dual = best_modified_derivation(*total, ha, back_pointers);

  HEdges used_edges = ha.construct_best_edges(back_pointers);
  if (SIMPLE_DEBUG) {
    for (int i = 0; i < used_edges.size(); ++i) {
      cerr << "Used edges: " << used_edges[i]->id() << endl;
    }
  }
  // CHANGES!!!
  if (cur_state.round >= _is_stuck_round + 20) {
    time_t begin = clock();
    NodeBackCache back_pointers2(_forest.num_nodes());
    cerr << "CUBING!!" <<  cur_state.round << endl;
    SplitController c(*_subproblem, _lattice, false);
    ExtendCKY ecky(_forest, *total, c);
    ecky.best_path(back_pointers2);
    cerr << "outside "<< endl;
    ecky.outside();
    cerr << "Back " << clock() - begin << endl;
    begin = clock();
    //Cache<Hypernode, int> * words = cache_word_nodes(_lm, _forest);
    cerr << "Drop " << clock() - begin << endl;
    begin = clock();
    NodeCache node_outside(_forest.num_nodes());
    foreach (HNode node, _forest.nodes()) {
      //if (ecky._outside_memo_table.get(*node)->size() >= 1) {
        node_outside.set_value(*node,
                               ecky._outside_memo_table.get(*node)->get_score(0));
        //}
    }


    // Compute best trigram.
    NodeCache node_best_trigram(_forest.num_nodes());
    foreach (HNode node, _forest.nodes()) {
      if (((ForestNode *)node)->is_word()) {
        int graph_id = _lattice.get_word_from_hypergraph_node(node->id());
        double score = _subproblem->best_score_dim(graph_id, 0, 0);
        //assert(score != 0.0);
        node_best_trigram.set_value(*node, score);
      }
    }
    cerr << "prep " << begin - clock() << endl;
    int cube = 1000;
    begin = clock();
    double cube_primal;
    if (true) {
      CubePruning p_temp(_forest, *_cached_weights, LMNonLocal(_forest, _lm, lm_weight(), *cached_cube_words_, true), 10, 3);
      cube_primal = p_temp.parse();
    }
    DualNonLocal non_local(_forest, _lm, lm_weight(), *cached_cube_words_, node_best_trigram, _lattice, _lagrange_weights, _subproblem, used_edges);
    CubePruning p(_forest, *total, non_local, cube, 3);
    double bound = min(cube_primal, cur_state.best_primal) + 0.01;
    cerr << "bound is: " << bound << endl;
    p.set_bound(bound);
    //p.set_bound(12.0);
    p.set_duals(total);
    p.set_cube_enum();
    p.set_heuristic(&node_outside);
    p.set_edge_heuristic(&ecky._outside_edge_memo_table);
    double v = p.parse();
    cerr << "CubePruning " << v << " " << clock() - begin << endl;
    if (abs(v) > 1e-4) {
      result.primal = v;
      if (p.is_exact()) {
        result.dual = v;
        return;
      }
    } else {
      result.primal = 10000000;
    }
  } else {
    result.primal = 10000000;
  }

  if (SIMPLE_DEBUG) {

    // foreach ( HEdge edge, _forest.edges()) {
    //   cout << edge->head_node().label() << " " << edge->label() << " " << total->get_value(*edge) << endl;
    // }

    // cout << "EDGES!";
    // foreach (HEdge edge, used_edges) {
    //   cout << edge->head_node().label() <<  " " << edge->label() << " " << total->get_value(*edge) << endl;
    // }
  }


  vector <const ForestNode *> used_words;
  {
    vector <const Hypernode *> tmp_words = ha.construct_best_fringe(back_pointers);
    foreach (const Hypernode * word, tmp_words ) {
      used_words.push_back((ForestNode*)word);
    }
  }

  vector <string> used_strings;

  // UGH! UGH! TODO get this <s> business out of here
  foreach (HNode used, used_words) {  // int i =0; i < used_words.size(); i++) {
    //cout <<used_words[i]->word() << " ";
    used_strings.push_back(((const ForestNode*)used)->word());
  }
  //cout <<endl;
  //assert(used_words.size() > 3);

  vector <int> used_lats;
  foreach (HNode word_node, used_words) {
    used_lats.push_back(_lattice.get_word_from_hypergraph_node( word_node->id()));
  }

  if (TIMING) {
    end=clock();
    cout << "PARSE TIME: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
    begin=clock();
  }

  /******************************************
   * 4) a) Reconstruct the subgrad values from parse derivation
   *    b) Reconstruct the subgrad values from lm derivation
   ******************************************/


  result.subgrad += construct_parse_subgrad(used_edges);



  double cost_total = 0.0;

  if (SIMPLE_DEBUG) cout << "predual " << result.dual << endl;
  result.subgrad += construct_lm_subgrad(used_words, used_lats, used_strings, result.dual, cost_total);



  if (DEBUG) {
    cout << "DUAL LM: " << lm_total << endl;
    cout << "DUAL LM (check): " << o_total + lag_total << endl;
    cout << "DUAL LM with lag (check): " << o_total  << endl;
    cout << endl;
  }


  /******************************************
   * 5) a) Compute the primal score (just derivation and implied lm)
   *
   ******************************************/

  double round_primal = compute_primal(used_edges,
                                       used_words,
                                       penalty_cache);
  //cerr << "Round Primal " << round_primal << endl;
  result.primal = min(result.primal, round_primal);


  double edge_total= 0.0;

  foreach (HNode node, _forest.nodes()) {
    if (!((ForestNode* )node)->is_word() && back_pointers.has_key(*node)) {
      edge_total += total->get_value(*(back_pointers.get_value(*node)));
    }
  }


  assert(fabs((cost_total + edge_total) - result.dual) < 1e-4);


  if (TIMING) {
    end=clock();
    cout << "CONSTRUCT LAGRANGIAN TIME: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
    begin=clock();

    end=clock();
    cout << "COMPUTE PRIMAL TIME: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;

    cout << "TOTAL TIME: " << double(Clock::diffclock(end, total_begin)) << " ms"<< endl;
  }

  /*for (int i=0; i < used_words.size(); i++ ) {
    cout << used_words[i]->word() << " ";
  }
  cout << endl;
  */
  if (DEBUG || SIMPLE_DEBUG) {
    cout << "DUAL Score" << result.dual << endl;
    cout << "PRIMAL Score " << result.primal << endl;
    cout << endl;
  }
  assert((result.dual - result.primal) < 1e-3);

  //print_output(result.subgrad);

  // if (result.dual - result.primal > 1e-3) {

  //   cout << "DUAL PRIMAL mismatch. You have a bug." << endl;
  //   exit(0);
  // }
}


double Decode::compute_primal(const HEdges used_edges, const vector <const ForestNode *> used_nodes, const EdgeCache &  edge_lag) {
  double total= 0;
  double lag = 0.0;
  for (unsigned int i=0; i < used_edges.size(); i++ ) {
    total += _cached_weights->store[used_edges[i]->id()] ;
    lag += edge_lag.store[used_edges[i]->id()];
    // if (DEBUG) {
    //   cout << "PRIMALEDGE "<<   used_edges[i]->head_node().label() << " "  << used_edges[i]->label() << " " << _cached_weights->store[used_edges[i]->id()] << " " << edge_lag.store[used_edges[i]->id()] << endl;

    //   vector <int> lat_edges = get_lat_edges(used_edges[i]->id());
    //   for (unsigned int j =0; j < lat_edges.size(); j++) {
    //     cout << lat_edges[j]<<" ";

    //   }
    //   cout << endl;
    // }

  }
  //cout << "PRIMAL Parse " << total << endl;
  //cout << "PRIMAL2 Parse " << total + lag<< endl;

  vector <string> used_strings;
  for (uint i =0; i < used_nodes.size(); i++) {
    used_strings.push_back(used_nodes[i]->word());
  }
  double lm_score =0.0;
  double primal2 = 0.0;
  for (uint i =0; i < used_strings.size()-2; i++) {
    VocabIndex context [] = {lookup_string(used_strings[i+1]), lookup_string(used_strings[i]), Vocab_None};

    if (DEBUG) {
      double lm_score = (lm_weight()) *   _lm.wordProb(lookup_string(used_strings[i+2]), context);
      cout << "PRIMAL " << used_strings[i] << " " <<  used_strings[i+1]<< " " <<  used_strings[i+2] << " " << lm_score << endl;

      int start_from = _lattice.get_word_from_hypergraph_node(used_nodes[i+2]->id());
      int mid = _lattice.get_word_from_hypergraph_node(used_nodes[i+1]->id());
      int end = _lattice.get_word_from_hypergraph_node(used_nodes[i]->id());
      //cout << "PRIMAL2 " << used_nodes[i] << " " <<  used_nodes[i+1];
      //<< " " <<  used_strings[i+2] << " " << (lm_weight()) *   _lm.wordProb(lookup_string(used_strings[i+2]), context) << endl;
      double w =0;
      w += _subproblem->get_best_bigram_weight(start_from, mid, 0);
      w += _subproblem->get_best_bigram_weight(mid, end, 1);
      //cout << "PRIMAL2 " << w + lm_score << endl;
      primal2 += w + lm_score;
    }
    lm_score += _lm.wordProb(lookup_string(used_strings[i+2]), context);
  }
  if (DEBUG) {
    cout << "PRIMAL LMWEIGHT: " << (lm_weight()) *lm_score << endl;
    cout << "PRIMAL2 : " << primal2 << endl;
    cout << endl;
  }
  //cout << "total " << total << endl;

  return total + (lm_weight()) *  lm_score;
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
  //_cached_word_str = new Cache <Graphnode, string> (_lattice.num_word_nodes);

  int max = _lm.vocab.numWords();
  int unk = _lm.vocab.getIndex(Vocab_Unknown);
  //assert(false);
  for (int n=0; n < _lattice.num_word_nodes; n++ ) {
    if (!_lattice.is_word(n)) continue;

    //const LatNode & node = _lattice.node(n);
    //assert (node.id() == n);
    string str = _lattice.get_word(n);
    //cout << "INITIALIZING " << str << " " << n << endl;
    _cached_word_str[str].push_back(n);
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

    // vector <int > between1 = _subproblem->get_best_nodes_between(start_from,dual_mid, true);
    // foreach (int node_id, between1) {
    //   cout << _lattice._edge_label_by_nodes[node_id] << " (" << node_id << ") ";
    // }

    // vector <int > between2 = _subproblem->get_best_nodes_between(dual_mid, dual_end, false);
    // foreach (int node_id, between2) {
    //   cout << _lattice._edge_label_by_nodes[node_id] << " (" << node_id << ") ";
    // }
    // cout << endl;
  }
}



void Decode::greedy_projection(int dual_mid, int dual_end, int primal_mid, int primal_end) {
  //int dual_mid_node = _lattice.get_hypergraph_node_from_word(dual_mid);
  //ForestNode * node = (ForestNode*) & _forest.get_node(dual_mid_node);
  //cout << "PROJECT WORD " <<  node->word();

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

          if (!COLOR_WORDS) {
            _constraints[w2].insert(w1);
            _constraints[w1].insert(w2);
          } else {
            string w1_str = _lattice.get_word(w1);
            string w2_str = _lattice.get_word(w2);
            if (w1_str!=w2_str) {
            foreach (int n1, _cached_word_str[w1_str]) {
              foreach (int n2, _cached_word_str[w2_str]) {
                if (n1!=n2) {
                  _constraints[n1].insert(n2);
                  _constraints[n2].insert(n1);
                }
              }
            }
            } else {
              _constraints[w2].insert(w1);
              _constraints[w1].insert(w2);

            }
          }
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

          if (!COLOR_WORDS) {
            _constraints[w2].insert(w1);
            //else
            _constraints[w1].insert(w2);
          } else {
            string w1_str = _lattice.get_word(w1);
            string w2_str = _lattice.get_word(w2);
            if (w1_str!=w2_str) {
            foreach (int n1, _cached_word_str[w1_str]) {
              foreach (int n2, _cached_word_str[w2_str]) {
                if (n1!=n2) {
                  _constraints[n1].insert(n2);
                  _constraints[n2].insert(n1);
                }
              }
            }
            } else {
              _constraints[w2].insert(w1);
              _constraints[w1].insert(w2);

            }
          }

        // //if (w1 < w2)
        // _constraints[w2].insert(w1);
        // //else
        // _constraints[w1].insert(w2);
      }
    }
}
