#include "Decode.h"

void Decode::update_weights(const wvector & update, const wvector & weights ) {
  vector <int> u_pos1, u_pos2;
  vector <float> u_val1, u_val2;
  for (wvector::const_iterator it = update.begin(); it != update.end(); it++) {
    if (it->second > GRAMSPLIT) {
      u_pos2.push_back(it->first - GRAMSPLIT);
      u_val2.push_back(it->second);
    } else {
      u_pos1.push_back(it->first);
      u_val1.push_back(it->second);
    }
  }
  _subproblem->update_weights(u_pos1, u_val1, true);
  _subproblem->update_weights(u_pos2, u_val2, false);
}

vector <int > Decode::get_lex_lat_nodes(int edge_id) {
  vector <int> all = get_lat_nodes(edge_id);
  vector <int> ret;
  for (int i=0; i< all.size(); i++) {
    if (_lattice.is_node_word(all[i])) {
      ret.push_back(all[i]);
    }
  }
  return ret;
}

vector <int > Decode::get_lat_nodes(int edge_id) {
  return _lattice.original_nodes[edge_id];
}

wvector & Decode::solve(double & primal , double & dual) {
  wvector subgrad;
  _subproblem->solve();
  
    
  // now add the forward trigrams at each node
  EdgeCache penalty_cache;
  for (int i; i < _forest.num_edges(); i++) { 
    const ForestEdge & edge = _forest.get_edge(i); 
    double total_score = 0.0;
    vector <int> lat_nodes = get_lex_lat_nodes(edge.id()); 
    for (int j =0; j <  lat_nodes.size(); j++) {
      int graph_id = lat_nodes[i];
      
      double score = _subproblem->cur_best_score[graph_id];
      total_score += score;          
    }
    penalty_cache.set_value(edge, total_score); 
  }
  
  EdgeCache * total = combine_edge_weights(_forest, penalty_cache, *_cached_weights);
  NodeCache scores;
  NodeBackCache back_pointers;
  dual = best_path(_forest, *total, scores, back_pointers);

  vector <int> used_edges = construct_best_edges(_forest, back_pointers); 

 

  //assert abs(best -best_fv.dot(cur_weights)) < 1e-4, str(best) + " " + str(best_fv.dot(cur_weights))
  //lagrangians_parse = 0.0
  //lagrangians_other = 0.0

  //tri_pairs = [];
    
  for (int i =0; i < used_edges.size(); i++){ 
    int edge_id= used_edges[i];
    // - lagrangians (FROM LM SIDE)
    vector <int> lex_lat_nodes = get_lex_lat_nodes(edge_id); 
    for (int j =0; j < lex_lat_nodes.size(); j++) {
      int graph_id = lex_lat_nodes[j];
      
      
      //Bigram forbigram = _subproblem->get_best_trigram(graph_id);
      
      int end_at = _subproblem->cur_best_two[graph_id];
      int mid_at = _subproblem->cur_best_one[graph_id];
      int start_from = graph_id;

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

      // + lagrangians (FROM PARSE SIDE)
    vector <int> lat_nodes = get_lat_nodes(edge_id); 
    for (int j =0; j < lat_nodes.size(); j++) {
      int lat_id = lat_nodes[j];
      subgrad[lat_id] += 1;
      subgrad[GRAMSPLIT + lat_id ] += 1;
    }
  }
    /*
    // BOUNDARY CONDITIONS
    bounds = [(0,1), (self.graph.size()-1,self.graph.size()-2) ]
    bounds.reverse()

    # END BOUNDARY
    # add in the last node, and second to last (trigram)
    # over counted at k
    feat = "2UNI:"+str(bounds[1][0])
    ret[feat] += 1 
    
    best += self.lagrangians[feat]  

    # over counted at k and j
    feat = "1UNI:"+str(bounds[1][1])
    ret[feat] += 1 
    best += self.lagrangians[feat]

    feat = "2UNI:"+str(bounds[1][1])
    ret[feat] += 1 
    best += self.lagrangians[feat]  

    # START BOUNDARY
    # first word <s>
    (forbigram,  score) = self.subproblem.get_best_trigram(bounds[0][0])    
    best += score

    tri_pairs.append((self.graph.nodes[bounds[0][0]].lex, self.graph.nodes[forbigram.w1].lex,self.graph.nodes[forbigram.w2].lex))
    #bi_pairs.append((self.graph.nodes[forbigram.w1].lex, self.graph.nodes[forbigram.w2].lex))
    
    #between1 = self.subproblem.get_best_nodes_between(0,forbigram.w1)
    #for b in between1:
      #ret["UNI:"+str(b)] -= 1 

    #print "Boundaries \n\n"
    
    between2 = self.subproblem.get_best_nodes_between(forbigram.w1, forbigram.w2, False)
    for b in between2:
      ret["2UNI:"+str(b)] -= 1 
      
      if DEBUG:
        print "\t", b, -self.lagrangians["2UNI:"+str(b)] 
        print "LM", self.graph.nodes[b]
      
    # second word <s>


    (forbigram,  score) = self.subproblem.get_best_trigram(bounds[0][1])
    best += score

    tri_pairs.append((self.graph.nodes[bounds[0][1]].lex, 
                      self.graph.nodes[forbigram.w1].lex,
                      self.graph.nodes[forbigram.w2].lex))
    
    between1 = self.subproblem.get_best_nodes_between(bounds[0][1], forbigram.w1, True)
    if DEBUG: self.debug_bigram(self.graph.nodes[1], forbigram, score)

    for b in between1:
      ret["1UNI:"+str(b)] -= 1

      if DEBUG:
        print "\t", b, -self.lagrangians["1UNI:"+str(b)]
        print "LM", self.graph.nodes[b]

    between2 = self.subproblem.get_best_nodes_between(forbigram.w1, forbigram.w2, False)
    for b in between2:
      ret["2UNI:"+str(b)] -= 1

      if DEBUG:
        print "\t", b, -self.lagrangians["2UNI:"+str(b)]
        print "LM", self.graph.nodes[b]

    for f in ret:
      if ret[f] == 0.0:
        del ret[f]
    */
  primal = 0.0;//self.compute_primal(best_fv, subtree)    
    
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
}


void Decode::sync_lattice_lm() {
  
  int max = _lm.vocab.numWords();
  int unk = _lm.vocab.getIndex(Vocab_Unknown);
  for (int n=0; n < _lattice.num_nodes; n++ ) {
    if (!_lattice.is_node_word(n)) continue;
    const LatNode & node = _lattice.node(n); 
    int ind = _lm.vocab.getIndex(_lattice.get_word(n).c_str());
    _cached_words->set_value(node, ind);
  }
}


