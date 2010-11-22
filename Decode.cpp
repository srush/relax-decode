
#include "Decode.h"
#include "util.h"
#include "time.h"
#define TIMING 0
#define DEBUG 0

void Decode::update_weights(const wvector & update,  wvector * weights ) {
  vector <int> u_pos1, u_pos2;
  vector <float> u_val1, u_val2;
  for (wvector::const_iterator it = update.begin(); it != update.end(); it++) {
    if (it->second == 0.0) continue;
    if (it->first >= GRAMSPLIT) {
      u_pos2.push_back(it->first - GRAMSPLIT);
      u_val2.push_back(-it->second);
    } else {
      u_pos1.push_back(it->first);
      u_val1.push_back(-it->second);
    }
  }
  if (DEBUG) {
    cout << "UPDATING WEIGHTS " << u_pos1.size() << " " << u_pos2.size() <<endl;
  }
  clock_t begin, end;
  if (TIMING) {
    begin=clock();
  }
  _subproblem->update_weights(u_pos1, u_val1, true);
  _subproblem->update_weights(u_pos2, u_val2, false);
  if (TIMING) {
    end=clock();
    cout << "UPDATE TIME: " << double(diffclock(end,begin)) << " ms"<< endl;
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

//vector <int > Decode::get_lat_nodes(int edge_id) {
//return _lattice.original_nodes[edge_id];
//}

vector <int > Decode::get_lat_edges(int edge_id) {
  //  assert(false);
  return _lattice.original_edges[edge_id];
}


void Decode::print_output(const wvector & subgrad) {
  for (wvector::const_iterator it = subgrad.begin(); it != subgrad.end(); it++) {
    if (it->second != 0.0) {
      cout << it->first << " " << it->second << endl;
      if (it->first < GRAMSPLIT && _lattice.is_word(it->first)) { 
        cout << _lattice.get_word(it->first) << endl;
      }
      if (it->first >= GRAMSPLIT && _lattice.is_word(it->first - GRAMSPLIT)) { 
        cout << _lattice.get_word(it->first - GRAMSPLIT) << endl;
      }
    }
  }
  cout << endl << endl;
}

void Decode::solve(double & primal , double & dual, wvector & subgrad) {
  clock_t begin, end;
  if (TIMING) {
    cout << "Solving" << endl;
    begin=clock();
  }  
  _subproblem->solve();
  if (TIMING) {
    end=clock();      
    cout << "SOLVE TIME: " << double(diffclock(end,begin)) << " ms"<< endl;
  }


  // now add the forward trigrams at each node
  if (TIMING) {
    begin=clock();  
  }
  EdgeCache self_penalty_cache(_forest.num_edges());
  EdgeCache trigram_penalty_cache(_forest.num_edges());
  int num_edges = _forest.num_edges();
  for (unsigned int i=0; i < num_edges; i++) { 
    const ForestEdge & edge = _forest.get_edge(i);
    assert (edge.id() == i);
    double total_score = 0.0;
    
    // trigram penalties 
    {
      vector <int> lat_edges = get_lex_lat_edges(edge.id()); 
      for (unsigned int j =0; j <  lat_edges.size(); j++) {
        int graph_id = lat_edges[j];
        
        double score = _subproblem->cur_best_score[graph_id];
        assert (score != 1000000);
        //cout << graph_id << " " << score << endl;
        total_score += score;          
      }
      //cout << edge.id()  << " " << total_score << endl;
      trigram_penalty_cache.set_value(edge, total_score); 
    }
    
    {
      double total_score = 0.0;
      // self penalties
      {
        vector <int> lat_edges = get_lat_edges(edge.id()); 
        for (unsigned int j =0; j < lat_edges.size(); j++) {
          int lat_id = lat_edges[j];
          total_score += (*_lagrange_weights)[lat_id];
          total_score += (*_lagrange_weights)[GRAMSPLIT + lat_id ];
        }
      }    
      self_penalty_cache.set_value(edge, total_score); 
    }

    
  }
  if (TIMING) {
    end=clock();      
    cout << "PENALTY CACHE: " << double(diffclock(end,begin)) << " ms"<< endl;
    
    begin=clock();  
  }
  EdgeCache * total1 = combine_edge_weights(_forest, self_penalty_cache, *_cached_weights);
  EdgeCache * total = combine_edge_weights(_forest, trigram_penalty_cache, *total1);
  NodeCache scores(_forest.num_nodes()), scores2(_forest.num_nodes());
  NodeBackCache back_pointers(_forest.num_nodes()), back_pointers2(_forest.num_nodes());

  if (TIMING) {
    end=clock();      
    cout << "COMBINE: " << double(diffclock(end,begin)) << " ms"<< endl;
  }
  //double simple = best_path(_forest, *_cached_weights, scores2, back_pointers2);
  
  //cout << "SIMPLE Score " << simple << endl; 
  if (TIMING) {
    begin=clock();  
  }
  dual = best_path(_forest, *total, scores, back_pointers);
  double trig_total = 0.0;
  //cout << "INITIAL DUAL Score" << dual << endl;

  vector <int> used_edges = construct_best_edges(_forest, back_pointers); 
  vector <const ForestNode *> used_words = construct_best_fringe(_forest, back_pointers); 

  if (TIMING) {
    end=clock();  
    cout << "Parse time: " << double(diffclock(end,begin)) << " ms"<< endl;
  }
  //assert abs(best -best_fv.dot(cur_weights)) < 1e-4, str(best) + " " + str(best_fv.dot(cur_weights))
  //lagrangians_parse = 0.0
  //lagrangians_other = 0.0

  //tri_pairs = [];
  
   begin=clock();  

  //bitset <NUMSTATES> parse_states;
  for (int i =0; i < used_edges.size(); i++){ 
    int edge_id= used_edges[i];
    // + lagrangians (FROM PARSE SIDE)
    vector <int> lat_edges = get_lat_edges(edge_id); 
    for (int j =0; j < lat_edges.size(); j++) {
      int lat_id = lat_edges[j];
      subgrad[lat_id] += 1;
      subgrad[GRAMSPLIT + lat_id ] += 1;
      //parse_states.set(lat_id);
    }
  }

  for (int i =0; i < used_edges.size(); i++){ 
    int edge_id= used_edges[i];
    // - lagrangians (FROM LM SIDE)
    vector <int> lex_lat_edges = get_lex_lat_edges(edge_id); 
    //cout << "lex lat " << lex_lat_edges.size() << endl;
    for (int j =0; j < lex_lat_edges.size(); j++) {
      int graph_id = lex_lat_edges[j];
        
      //Bigram forbigram = _subproblem->get_best_trigram(graph_id);
      
      // Choose end and mid to be the ones directly before me (if possible)
      int pos = -1;
      for (int p = 0; p < used_words.size() ; p++) {
        if (_lattice.get_word(graph_id) == used_words[p]->word()) {
          pos = p;
          break;
        } 
      }

      int end_at = _subproblem->cur_best_two[graph_id][0];
      int mid_at = _subproblem->cur_best_one[graph_id][0];
      int start_from = graph_id;
      double lag_total = 0.0;
      //cout << "SCORE " << _subproblem->cur_best_score[graph_id] << " " << _lattice.get_word(graph_id) << " " << graph_id << " " <<  _lattice.lookup_word(graph_id) << " " << mid_at << " " << _lattice.get_word(mid_at)<< " "<< _lattice.lookup_word(mid_at) << " " << end_at << " " << _lattice.get_word(end_at) << " " << _lattice.lookup_word(end_at)<< " "<<endl;
      for (int k =0; k < _subproblem->cur_best_two[graph_id].size(); k++ ) {
        int e_at = _subproblem->cur_best_two[graph_id][k];
        int m_at = _subproblem->cur_best_one[graph_id][k];
        //if (pos ==-1 || pos < 2) {
          //cout << "ignored " <<endl;
          //break;
        //} 
        if (((pos ==0 && e_at == 0) || (pos ==1 && e_at == 1)
             || ( pos > 1 && e_at == _lattice.lookup_word_by_hypergraph_node(used_words[pos-2]->id())))
             && (( pos ==0 && m_at == 1) ||
                 (pos >= 1 && m_at == _lattice.lookup_word_by_hypergraph_node(used_words[pos-1]->id())))) {
          end_at = e_at;
          mid_at = m_at;
          //cout << "hit " <<endl;
          break;
        }
      }

      //cout << end_at << " " << mid_at << " " << start_from << endl; 
      //cout << "DUAL " << _lattice.get_word(end_at) << " " <<  
      //_lattice.get_word(mid_at) << " " <<  
      //_lattice.get_word(graph_id) << " " << 
      //_subproblem->cur_best_score[graph_id] << " " <<
      //(-0.141221) *   _subproblem->word_prob_reverse(start_from, mid_at, end_at) << endl;

      
      vector <int > between1 = _subproblem->get_best_nodes_between(start_from,mid_at, true);
      //cout << "Size!" << between1.size() << endl;
      for (int k = 0; k < between1.size(); k++) {        
        int node_id = between1[k];
        assert(!_lattice.is_word(node_id) || node_id == mid_at);
        //cout << "DEC!" << node_id << endl;
        assert(node_id >= 0);
        subgrad[node_id] -= 1;
        lag_total += (*_lagrange_weights)[node_id] ;
      }
      
      const vector <int> between2 = _subproblem->get_best_nodes_between(mid_at,end_at, false);

      //cout << "Size!" << between2.size() << endl;
      for (int k = 0; k < between2.size(); k++) {
        int node_id = between2[k];
        //cout << "DEC!" << node_id << endl;
        assert(node_id >= 0);
        subgrad[node_id + GRAMSPLIT] -= 1;
        lag_total += (*_lagrange_weights)[node_id + GRAMSPLIT] ;
      }
      if (DEBUG) {
        double lm_score = (-0.141221) * _subproblem->word_prob_reverse(graph_id, mid_at, end_at);
        //cout << lm_score << " " << -lag_total << " " << _subproblem->cur_best_score[graph_id] << endl;
        assert(fabs(-lag_total + lm_score - _subproblem->cur_best_score[graph_id]) < 1e-4);

        cout << "SCORE " << graph_id << " " << _lattice.get_word(end_at) << " " << _lattice.get_word(mid_at)<< " "<< _lattice.get_word(graph_id) << " " << graph_id <<" " << mid_at << " " << end_at << " " << _lattice.lookup_word(graph_id) << " " <<   -lag_total << " " << lm_score << " " << lm_score - lag_total<<endl;
        trig_total += _subproblem->cur_best_score[graph_id];


      }




    }
  }
  
  //cout << "Early dual " << dual <<endl;


 
  //print_output(subgrad);
   
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
    trig_total -= (*_lagrange_weights)[feat];  
  }


  // over counted at k and j
  {
    int feat = 1;
    subgrad[feat] += 1 ;
    dual += (*_lagrange_weights)[feat];
    trig_total -= (*_lagrange_weights)[feat];  

    feat = 1 + GRAMSPLIT; //"2UNI:"+str(bounds[1][1]);
    subgrad[feat] += 1 ;
    dual += (*_lagrange_weights)[feat]  ;
    trig_total -= (*_lagrange_weights)[feat];  
  }

  // START BOUNDARY
  //first word <s>
  {
    int id = _lattice.num_word_nodes-1;
    //vector <int> lex_lat_edges = get_lex_lat_edges(id); 
  
    double score = _subproblem->cur_best_score[id];
    //cout << "START1" << score << endl;
    //(forbigram,  score) = self.subproblem.get_best_trigram(id) ;
    dual += score;
    trig_total += score;
    
    vector <int> between2 = _subproblem->get_best_nodes_between(_subproblem->cur_best_one[id][0], _subproblem->cur_best_two[id][0], false);
    for (unsigned int k = 0; k < between2.size(); k++) {
        int node_id = between2[k];
        subgrad[node_id + GRAMSPLIT] -= 1;
    }
  }

  {
    int id = _lattice.num_word_nodes-2;
    
    // second word <s>
    double score = _subproblem->cur_best_score[id]; 
    //cout << "START2" <<score << endl;

    dual += score;
    trig_total += score;
    vector <int> between1 = _subproblem->get_best_nodes_between(id, _subproblem->cur_best_one[id][0], true);

    for (unsigned int k = 0; k < between1.size(); k++) {
        int node_id = between1[k];
        subgrad[node_id] -= 1;
    }

    vector <int> between2 = _subproblem->get_best_nodes_between( _subproblem->cur_best_one[id][0],  _subproblem->cur_best_two[id][0], false);
    for (unsigned int k = 0; k < between2.size(); k++) {
        int node_id = between2[k];
        subgrad[node_id + GRAMSPLIT] -= 1;
    }

  }
  if (TIMING) {
   end=clock();
   cout << "Construct lagrangian: " << double(diffclock(end,begin)) << " ms"<< endl;
  }

  //primal = 0.0;//self.compute_primal(best_fv, subtree)    
  if (TIMING) { 
    begin=clock();
  }
  primal = compute_primal(used_edges, used_words);
  if (TIMING) {
    end=clock();
    cout << "COMPUTE PRIMA: " << double(diffclock(end,begin)) << " ms"<< endl;
  }



  if (DEBUG) {
    cout << "TRIG TOTAL: " << trig_total<< endl;    
    print_output(subgrad);
    for (int i=0; i < used_words.size(); i++ ) {
      cout << used_words[i]->word() << " ";
    }
    cout << endl;
  

    cout << "DUAL Score" << dual << endl;
    cout << "PRIMAL " << primal << endl;


    cout << endl;
  }
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


double Decode::compute_primal(const vector <int> used_edges, const vector <const ForestNode *> used_nodes) {
  double total= 0;
  for (unsigned int i=0; i < used_edges.size(); i++ ) {
    total += _cached_weights->store[used_edges[i]] ;
  }

  
  vector <string> used_strings;
  vector <int> used_lat_nodes;
  used_strings.push_back("<s>");
  used_strings.push_back("<s>");
  used_lat_nodes.push_back(0);
  used_lat_nodes.push_back(1);
  for (int i =0; i < used_nodes.size(); i++) {
    used_strings.push_back(used_nodes[i]->word());
    used_lat_nodes.push_back(_lattice.lookup_word_by_hypergraph_node(used_nodes[i]->id()));
  }
  used_strings.push_back("</s>");
  used_strings.push_back("</s>");
  used_lat_nodes.push_back(_lattice.num_word_nodes-2);
  used_lat_nodes.push_back(_lattice.num_word_nodes-1);

  double lm_score =0.0;
  if (DEBUG) {
    cout << endl;
  }
  for (int i =0; i < used_strings.size()-2; i++) {
    VocabIndex context [] = {lookup_string(used_strings[i+1]), lookup_string(used_strings[i]), Vocab_None};
    if (DEBUG) {
      cout << "PRIMAL " << used_strings[i] << " " <<  used_strings[i+1]<< " " <<  used_strings[i+2] << " " << (-0.141221) *   _lm.wordProb(lookup_string(used_strings[i+2]), context) << endl;
    }
    lm_score += _lm.wordProb(lookup_string(used_strings[i+2]), context);
  }
  //cout << "total " << total << endl;


  if (DEBUG) {
    
    cout << " STARTING FEASIBLE" << endl; 
    double total = 0.0;
    for (int i =2; i < used_lat_nodes.size(); i++) {
      int start_from = used_lat_nodes[i];
      int graph_id = used_lat_nodes[i];
      
      int mid_at   = used_lat_nodes[i-1];
      int end_at   = used_lat_nodes[i-2];
      assert(_lattice.is_word(graph_id));
      assert(_lattice.is_word(mid_at));
      assert(_lattice.is_word(end_at));
      double lag_total =0.0;
      
      vector <int > between1 = _subproblem->get_best_nodes_between(start_from,mid_at, true);
      for (int k = 0; k < between1.size(); k++) {        
        int node_id = between1[k];
        lag_total += (*_lagrange_weights)[node_id] ;
      }
      
      const vector <int> between2 = _subproblem->get_best_nodes_between(mid_at,end_at, false);

      for (int k = 0; k < between2.size(); k++) {
        int node_id = between2[k];
        lag_total += (*_lagrange_weights)[node_id + GRAMSPLIT] ;
      }

      double lm_score = (-0.141221) * _subproblem->word_prob_reverse(graph_id, mid_at, end_at);
    
      cout << "FEASIBLE " << graph_id << " " << _lattice.get_word(end_at) << " " << _lattice.get_word(mid_at)<< " "<< _lattice.get_word(graph_id) << " " << end_at << " " << mid_at << " " <<  graph_id << " " << _lattice.lookup_word(graph_id) << " " <<  lm_score << " " << - lag_total << " " << lm_score - lag_total <<endl;
      total += lm_score - lag_total;
    }
    cout << "FEAS TOTAL " << total << endl;
  }
  
  return total + (-0.141221) *  lm_score;
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
  
  _cached_words = new Cache <LatNode, int> (_lattice.num_word_nodes);
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



