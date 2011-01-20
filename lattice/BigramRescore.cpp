#include "BigramRescore.h"
#include "GraphDecompose.h"
#include "../util.h"
#include <time.h>
#include <iostream>
#include <assert.h>
using namespace std;

#define TIMING 0
#define OPTIMIZE 1


BigramRescore::BigramRescore(const ForestLattice * graph_in, const GraphDecompose * gd_in):
  gd(gd_in), graph(graph_in){
  // huge hack
  int num_nodes = max(graph_in->num_nodes, graph_in->num_word_nodes);
  current_weights.resize(num_nodes);
  update_position.resize(num_nodes);
  update_filter.resize(num_nodes);



  int num_actual_nodes = graph_in->num_nodes;
  
  forward_paths.resize(num_actual_nodes);
  backward_paths.resize(num_actual_nodes);
  need_to_recompute.resize(num_nodes);
  
  bigram_weights.resize(num_nodes);

  best_split.resize(num_nodes);
  bigram_path.resize(num_nodes);


  for (int i=0; i < num_nodes; i++) {
    current_weights[i] = 0.0;
    need_to_recompute[i].resize(num_nodes);
    best_split[i].resize(num_nodes);
    bigram_path[i].resize(num_nodes);
    bigram_weights[i].resize(num_nodes);
    for (int j=0; j < num_nodes; j++) {
      bigram_path[i][j] = NULL;
    }
    bigram_weights[i][i] = 0.0;
  }

  cache_forward();
}

void BigramRescore::update_weights(vector <int>  u_pos, vector <float> u_values, int len) {
  clock_t begin=clock();


  for (int i=0; i< graph->num_nodes; i++) {
    update_filter[i] = 0;
  }

  update_len = len;
  for (int i=0; i< len; i++) {
    int pos = u_pos[i];
    //update_values[i] = u_values[i];
    update_position[i] = pos;
    current_weights[pos] += u_values[i];
    update_filter[pos] = 1;
    //update_filter[pos] = true; 
  }
  
  recompute_bigram_weights(false);
  if (TIMING) {
    clock_t end=clock();
    cout << "RECOMPUTE: " << double(diffclock(end,begin)) << " ms"<< endl;
  }
}



/*void BigramRescore::cache_paths(int n1, int n2) {  
  //assert(!bigram_cache[n1][n2].any());
  vector <int> * path = gd->get_path(n1,n2);
  int s = path->size();
  for (int split=0; split < s; split++) {
    int k = (*path)[split];

    if (k == n2) {
      bigram_cache[n1][n2][n2] = true;
    } else {

      if (!bigram_cache[n1][k].any()) {
        cache_paths(n1, k);
      } 

      if (!bigram_cache[k][n2].any()) {
        cache_paths(k, n2);
      } 
      bigram_cache[n1][n2] = bigram_cache[n1][k] | bigram_cache[k][n2]; 
    }
  }
  assert(bigram_cache[n1][n2].any());
  }*/


void BigramRescore::cache_forward() {  
  //assert(!bigram_cache[n1][n2].any());

  for (int n1 =0; n1 < graph->num_nodes; n1++) {   
    for (int i = 0; i< graph->num_nodes; i++) {
      if (!graph->is_phrase_node(n1)) {
        if (gd->path_exists(n1,i) || i == n1) {
          forward_paths[n1].push_back(i);
        }
      } 
      if (gd->path_exists(i,n1)) {
        backward_paths[n1].push_back(i);
      }
    }
    if (graph->is_phrase_node(n1)) {
      forward_paths[n1].push_back(n1);
    }
  }
}

void BigramRescore::reconstruct_path(int n1, int n2, const vector<vector <int> > & best_split, vector <int > & array ) {
  if (n1 == n2) {
    return;
  } 
  // reconstruct edges
  int k = best_split[n1][n2];
  if (k != n2) { 
    reconstruct_path(n1, k, best_split, array);
    reconstruct_path(k, n2, best_split, array);
  } else {
    int edge_label =graph->get_edge_label(n1, n2);
    if (edge_label != -1) {
      assert(!graph->is_word(edge_label));
      array.push_back(edge_label);
    }
    //array.push_back(k);
  }
}

void BigramRescore::find_shortest(int n1, int n2) {
  
  assert (need_to_recompute[n1][n2]);
  recomputed ++;
  /*bool has_update = false;
  for (int i=0; i < update_len; i++) {
    
    if (bigram_cache[n1][n2][update_position[i]]) {
      has_update = true;
      break;
    }
    }*/
  
  need_to_recompute[n1][n2] =0;

  //if (update_len != 0 && !has_update ) {    
    //cout << "skip" << endl;
    //return;
  //}

  //assert(best_split_score[n1][n2] == 0 || update_len != 0);
  //double old_score = bigram_weights[n1][n2]; 
  int old_split = best_split[n1][n2];
  if (n1 != n2) 
    bigram_weights[n1][n2] = INF;

  assert(gd->path_exists(n1, n2));
  vector <int> * path = gd->get_path(n1,n2);
  int s = path->size();
  for (int split=0; split < s; split++) {
    int k = (*path)[split];
    
    if (k == n2) {
      //base case
      int edge_id = graph->get_edge_label(n1, n2);
      
      float val;
      if (edge_id != -1) 
        val = current_weights[edge_id];
      else
        val = 0.0;
      // should only get to base case on update
      //cout << "Update" << k << endl;
      //assert (update_filter[k]);
      if (val < bigram_weights[n1][n2]) {
        best_split[n1][n2] = k;
        bigram_weights[n1][n2] = val;
      }
    } else {
      float a, b;
      if (need_to_recompute[n1][k]) {
        find_shortest(n1, k);      
      } 
      a = bigram_weights[n1][k];
      
      if (need_to_recompute[k][n2]) {
        find_shortest(k, n2);      
      } 
      b = bigram_weights[k][n2];
      
      if (a + b < bigram_weights[n1][n2]) {
        best_split[n1][n2] = k;
        bigram_weights[n1][n2] = a+b;
        //array.clear();
        //array.insert(array.end(), one.begin(), one.end());
        //array.insert(array.end(), two.begin(), two.end());
      }
    }
  }
  if (old_split != best_split[n1][n2]) 
    score_changed++;
}


void BigramRescore::recompute_bigram_weights(bool initialize) {

  int count = 0;
  recomputed =0;
  score_changed =0;

  clock_t begin=clock();

  if (initialize || !OPTIMIZE) {
    
    for (int i=0; i < graph->num_nodes; i++) {
      update_filter[i] = 1;
      for (int j=0;j < graph->num_nodes; j++) {
        if (i!= j)
          need_to_recompute[i][j] = 1;
      }
    }
  } else { 
    //assert (false);
    // need to figure out edge conversion
    for (int k=0; k < update_len; k++ ) {
      int up_pos = update_position[k];
      if (graph->is_word(up_pos)) continue;
      //cout << "UPDATE " << up_pos << endl;
      Bigram b = graph->get_nodes_by_labels(up_pos);
      //if (graph->is_word(up_pos)) continue;
      //int up_node = graph->orig_id_to_edge
      
      //cout << up_node << endl;
      //if (seen[up_node]) continue;
      //cout << up_pos << " " << b.w1 << " " << b.w2 << endl;
      //cout << forward_paths[b.w1].size() << endl;

      for (uint i=0; i < forward_paths[b.w2].size(); i++) {
        int rnode = forward_paths[b.w2][i];
    
        for (uint j=0; j < backward_paths[b.w1].size(); j++) {

          int lnode = backward_paths[b.w1][j];

          //cout << lnode << " " << rnode << endl;
          count ++;
          //cout << b.w1 << " " << lnode << " " << rnode << endl;
          need_to_recompute[lnode][rnode] = 1; 
        }
      }
    }
  } 
  clock_t end;
  if (TIMING) {
    end=clock();
    cout << "Dirty: " << double(diffclock(end,begin)) << " ms"<< endl;
  
    begin = clock();
  }
  for (unsigned int i=0; i< gd->valid_bigrams.size() ;i++) {
        
    Bigram b = gd->valid_bigrams[i];
    int w1 = b.w1;
    int w2 = b.w2;

    int n1 = graph->lookup_word(w1);
    int n2 = graph->lookup_word(w2);
     
    // first do it with recursion
    if (need_to_recompute[n1][n2] && gd->path_exists(n1, n2)) {
      find_shortest(n1, n2); 
      if (bigram_path[n1][n2] != NULL) {
        bigram_path[n1][n2]->clear();
      }
       //reconstruct_path(w1, w2, best_split, bigram_path[w1][w2]); 
    }
  }
  if (TIMING) {
    cout << "COUNT " << count << " " << recomputed << " " << score_changed << endl;
    end=clock();
    cout << "FIND Shortest: " << double(diffclock(end,begin)) << " ms"<< endl;
  }
}


  //cout << "Percent moved: " << moved/ float(total) << endl;
  /*for (int i=0; i < graph->num_nodes; i++) {
    update_filter[i] = 1;
    for (int j=0;j<graph->num_nodes; j++) {
      need_to_recompute[i][j] = 1;
    }
  }


  for (unsigned int i=0; i< gd->valid_bigrams.size() ;i++) {
    Bigram b = gd->valid_bigrams[i];
    int w1 = b.w1;
    int w2 = b.w2;
    
    // Basically we need to find the best path between w1 and w2
    // using only gd->all_pairs_path
        
    // first do it with recursion
    if (need_to_recompute[w1][w2]) {
       find_shortest(w1, w2, best_split, best_split_score2); 
       if (best_split_score[w1][w2] != best_split_score2[w1][w2]) {
         cout << "ERROR " << w1 << " " <<w2 << " " <<best_split_score[w1][w2] << " "<<best_split_score2[w1][w2] << endl;
         assert(false);
       }
    }

    }*/


  //clock_t end=clock();
  //cout << "Weight Update: " << double(diffclock(end,begin)) << " ms"<< endl;
/*
void BigramRescore::recompute_bigram_weights_old() {
  
  clock_t begin=clock();

  for (int i=0;i < graph->num_nodes; i++) {
    for (int j=0;j<graph->num_nodes; j++) {
      //best_split_score[i][j] =0;
      need_to_recompute[i][j] = 1;
    }
  }



  for (unsigned int i=0; i< gd->valid_bigrams.size() ;i++) {
    Bigram b = gd->valid_bigrams[i];
    int w1 = b.w1;
    int w2 = b.w2;
    
    // Basically we need to find the best path between w1 and w2
    // using only gd->all_pairs_path
        
    // first do it with recursion
    find_shortest(w1, w2, best_split, best_split_score);
    bigram_weights[w1][w2] = best_split_score[w1][w2]; 
    bigram_path[w1][w2].clear();
    reconstruct_path(w1, w2, best_split, bigram_path[w1][w2]);
  }

  clock_t end=clock();
  //cout << "Weight Update: " << double(diffclock(end,begin)) << " ms"<< endl;
  
}
*/
// try it as a loop
  /*
  for (int n=0;n < graph->num_nodes;n++) {
    for (int n2=0;n2 < graph->num_nodes;n2++) {
      if (need_to_recompute[n][n2]) {
        bigram_weights[n][n2] = INF;
      }
    }
    
    // If path exists set it to 1
    for (int j=0;j < graph->node_edges[n]; j++) {
      int n2 = graph->graph[n][j];
      float val =  current_weights[n2];
      // should only get to base case on update
      
      if (val < bigram_weights[n][n2]) {
        best_split[n][n2] = n2;
        bigram_weights[n][n2] = val;
      }
    }
  }


  for (int k=0;k < graph->num_nodes; k++) {
    
    // lex nodes can't be in the middle of a path
    if (graph->word_node[k]!= -1) continue;
    
    for (int n=0; n < graph->num_nodes; n++) {
      if (!gd->all_pairs_path_exist[n][k]) continue;
      
      for (int n2=0; n2 < graph->num_nodes; n2++) {
        if (need_to_recompute[n][n2] && gd->all_pairs_path_exist[n][k] &&  gd->all_pairs_path_exist[k][n2] ) {
          //for_updates[k].push_back( Bigram(n, n2));
          double score = bigram_weights[n][k] + bigram_weights[k][n2];
          if (score < bigram_weights[n][n2]) {
            best_split[n][n2] = k;
            bigram_weights[n][n2] = score;
          }
        }
      }
    }
  }
  */
