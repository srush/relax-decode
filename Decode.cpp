#include "Decode.h"
#include "util.h"

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
  //cout << "UPDATING WEIGHTS " << u_pos1.size() << " " << u_pos2.size() <<endl;
  clock_t begin=clock();
  _subproblem->update_weights(u_pos1, u_val1, true);
  _subproblem->update_weights(u_pos2, u_val2, false);
  clock_t end=clock();
  cout << "UPDATE TIME: " << double(diffclock(end,begin)) << " ms"<< endl;
  _lagrange_weights = weights;
  
}

vector <int > Decode::get_lex_lat_nodes(int edge_id) {
  vector <int> all = get_lat_nodes(edge_id);
  vector <int> ret;
  for (unsigned int i=0; i< all.size(); i++) {
    if (_lattice.is_node_word(all[i])) {
      ret.push_back(all[i]);
    }
  }
  return ret;
}

vector <int > Decode::get_lat_nodes(int edge_id) {
  return _lattice.original_nodes[edge_id];
}


void print_output(const wvector & subgrad) {
  for (wvector::const_iterator it = subgrad.begin(); it != subgrad.end(); it++) {
    if (it->second != 0.0) {
      cout << it->first << " " << it->second << endl;
    }
  }
  cout << endl << endl;
}

void Decode::solve(double & primal , double & dual, wvector & subgrad) {
  _subproblem->solve();
    
  // now add the forward trigrams at each node
  EdgeCache penalty_cache(_forest.num_edges());
  int num_edges = _forest.num_edges();
  for (unsigned int i=0; i < num_edges; i++) { 
    const ForestEdge & edge = _forest.get_edge(i);
    assert (edge.id() == i);
    double total_score = 0.0;
    
    // trigram penalties 
    {
      vector <int> lat_nodes = get_lex_lat_nodes(edge.id()); 
      for (unsigned int j =0; j <  lat_nodes.size(); j++) {
        int graph_id = lat_nodes[j];
        
        double score = _subproblem->cur_best_score[graph_id];
        //cout << graph_id << " " << score << endl;
        total_score += score;          
      }
      //cout << edge.id()  << " " << total_score << endl;
    }
    // self penalties
    {
      vector <int> lat_nodes = get_lat_nodes(edge.id()); 
      for (unsigned int j =0; j < lat_nodes.size(); j++) {
        int lat_id = lat_nodes[j];
        total_score += (*_lagrange_weights)[lat_id];
        total_score += (*_lagrange_weights)[GRAMSPLIT + lat_id ];
      }
    }    
    penalty_cache.set_value(edge, total_score); 
    
  }
  
  EdgeCache * total = combine_edge_weights(_forest, penalty_cache, *_cached_weights);
  NodeCache scores(_forest.num_nodes()), scores2(_forest.num_nodes());
  NodeBackCache back_pointers(_forest.num_nodes()), back_pointers2(_forest.num_nodes());
  
  double simple = best_path(_forest, *_cached_weights, scores2, back_pointers2);
  //cout << "SIMPLE Score " << simple << endl; 
  
  dual = best_path(_forest, *total, scores, back_pointers);

  //cout << "INITIAL DUAL Score" << dual << endl;

  vector <int> used_edges = construct_best_edges(_forest, back_pointers); 
  vector <const ForestNode *> used_words = construct_best_fringe(_forest, back_pointers); 

 

  //assert abs(best -best_fv.dot(cur_weights)) < 1e-4, str(best) + " " + str(best_fv.dot(cur_weights))
  //lagrangians_parse = 0.0
  //lagrangians_other = 0.0

  //tri_pairs = [];
  
  //bitset <NUMSTATES> parse_states;
  for (int i =0; i < used_edges.size(); i++){ 
    int edge_id= used_edges[i];
    // + lagrangians (FROM PARSE SIDE)
    vector <int> lat_nodes = get_lat_nodes(edge_id); 
    for (int j =0; j < lat_nodes.size(); j++) {
      int lat_id = lat_nodes[j];
      subgrad[lat_id] += 1;
      subgrad[GRAMSPLIT + lat_id ] += 1;
      //parse_states.set(lat_id);
    }
  }

  for (int i =0; i < used_edges.size(); i++){ 
    int edge_id= used_edges[i];
    // - lagrangians (FROM LM SIDE)
    vector <int> lex_lat_nodes = get_lex_lat_nodes(edge_id); 
     
    for (int j =0; j < lex_lat_nodes.size(); j++) {
      int graph_id = lex_lat_nodes[j];
      
    
  
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
      int mid_at = _subproblem->cur_best_one[graph_id][0];;
      int start_from = graph_id;
      
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
      //(-0.141221) *   _subproblem->word_prob_reverse(start_from, mid_at, end_at) << endl;

      //cout << "COUNT ! " << _subproblem->cur_best_count[graph_id] << endl;
      vector <int > between1 = _subproblem->get_best_nodes_between(start_from,mid_at, true);
      //tri_pairs.append((self.graph.nodes[start_from].lex, self.graph.nodes[mid_at].lex, self.graph.nodes[end_at].lex))
      for (int k = 0; k < between1.size(); k++) {
        int node_id = between1[k];
        subgrad[node_id] -= 1;
      }
      
      vector <int> between2 = _subproblem->get_best_nodes_between(mid_at,end_at, false);
      for (int k = 0; k < between2.size(); k++) {
        int node_id = between2[k];
        subgrad[node_id + GRAMSPLIT] -= 1;
      }
    }
  }

  //print_output(*_lagrange_weights);
   
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
  }


  // over counted at k and j
  {
    int feat = 1;
    subgrad[feat] += 1 ;
    dual += (*_lagrange_weights)[feat];

    feat = 1 + GRAMSPLIT; //"2UNI:"+str(bounds[1][1]);
    subgrad[feat] += 1 ;
    dual += (*_lagrange_weights)[feat]  ;
  }

  // START BOUNDARY
  //first word <s>
  {
    int id = _lattice.num_nodes-1;
    vector <int> lex_lat_nodes = get_lex_lat_nodes(id); 
  
    double score = _subproblem->cur_best_score[id]; 
    //(forbigram,  score) = self.subproblem.get_best_trigram(id) ;
    dual += score;
    
    vector <int> between2 = _subproblem->get_best_nodes_between(_subproblem->cur_best_one[id][0], _subproblem->cur_best_two[id][0], false);
    for (unsigned int k = 0; k < between2.size(); k++) {
        int node_id = between2[k];
        subgrad[node_id + GRAMSPLIT] -= 1;
    }
  }

  {
    int id = _lattice.num_nodes-2;
      
    // second word <s>
    double score = _subproblem->cur_best_score[id]; 
    dual += score;
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

  //primal = 0.0;//self.compute_primal(best_fv, subtree)    

  
  primal = compute_primal(used_edges, used_words);
  //cout << "DUAL Score" << dual << endl;
  //cout << "PRIMAL " << primal << endl;
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
  used_strings.push_back("<s>");
  used_strings.push_back("<s>");
  for (int i =0; i < used_nodes.size(); i++) {
    used_strings.push_back(used_nodes[i]->word());
  }
  used_strings.push_back("</s>");
  used_strings.push_back("</s>");
  double lm_score =0.0;
  for (int i =0; i < used_strings.size()-2; i++) {
    VocabIndex context [] = {lookup_string(used_strings[i+1]), lookup_string(used_strings[i]), Vocab_None};
    //cout << "PRIMAL " << used_strings[i] << " " <<  used_strings[i+1]<< " " <<  used_strings[i+2] << " " << (-0.141221) *   _lm.wordProb(lookup_string(used_strings[i+2]), context) << endl;
    lm_score += _lm.wordProb(lookup_string(used_strings[i+2]), context);
  }
  //cout << "total " << total << endl;
  
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
  
  _cached_words = new Cache <LatNode, int> (_lattice.num_nodes);
  int max = _lm.vocab.numWords();
  int unk = _lm.vocab.getIndex(Vocab_Unknown);
  for (int n=0; n < _lattice.num_nodes; n++ ) {
    if (!_lattice.is_node_word(n)) continue;
    const LatNode & node = _lattice.node(n); 
    assert (node.id() == n);
    string str = _lattice.get_word(n);
    int ind = _lm.vocab.getIndex(str.c_str());
    if (ind == -1 || ind > max) { 
      _cached_words->set_value(node, unk);
    } else {
      _cached_words->set_value(node, ind);
    }
  }
}



