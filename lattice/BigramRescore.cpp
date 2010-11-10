#include "BigramRescore.h"
#include "GraphDecompose.h"
#include "../util.h"
#include <time.h>
#include <iostream>
#include <assert.h>
using namespace std;
#define INF 1000000
#define TIMING 0
#define OPTIMIZE 1

BigramRescore::BigramRescore(const ForestLattice * graph_in, const GraphDecompose * gd_in):
  gd(gd_in), graph(graph_in){
  for (int i=0; i < NUMSTATES; i++) {
    current_weights[i] = 0.0;
  }
  for (int i=0; i < NUMSTATES; i++) {
    for (int j=0; j < NUMSTATES; j++) {
      bigram_path[i][j] = NULL;
    }
  }
  cache_forward();
}

void BigramRescore::update_weights(vector <int>  u_pos, vector <float> u_values, int len) {
  clock_t begin=clock();

  update_filter.reset();
  //for (int i=0; i< NUMSTATES; i++) {
    //update_values[i] = 0.0;
  //}

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



void BigramRescore::cache_paths(int n1, int n2) {  
  //assert(!bigram_cache[n1][n2].any());
  int s = gd->all_pairs_path[n1][n2]->size();
  for (int split=0; split < s; split++) {
    int k = (*gd->all_pairs_path[n1][n2])[split];

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
}


void BigramRescore::cache_forward() {  
  //assert(!bigram_cache[n1][n2].any());
  

  for (int n1 =0; n1 < graph->num_nodes; n1++) {   
    for (int i = 0; i< graph->num_nodes; i++) {
      if (graph->word_node[n1] == -1) {
        if (gd->all_pairs_path_exist[n1][i] || i == n1) {
          forward_paths[n1].push_back(i);
        }
      } 
      if (gd->all_pairs_path_exist[i][n1]) {
        backward_paths[n1].push_back(i);
      }
    }
    if (graph->word_node[n1] != -1) {
      forward_paths[n1].push_back(n1);
    }
  }
}

void BigramRescore::reconstruct_path(int n1, int n2, int best_split[NUMSTATES][NUMSTATES], vector <int > & array ) {
  int k = best_split[n1][n2];
  if (k != n2) { 
    reconstruct_path(n1, k, best_split, array);
    reconstruct_path(k, n2, best_split, array);
  } else {
    array.push_back(k);
  }
}

void BigramRescore::find_shortest(int n1, int n2,
                               int best_split[NUMSTATES][NUMSTATES], 
                               float best_split_score[NUMSTATES][NUMSTATES]) {
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
  best_split_score[n1][n2] = INF;
  int s = gd->all_pairs_path[n1][n2]->size();
  for (int split=0; split < s; split++) {
    int k = (*gd->all_pairs_path[n1][n2])[split];
    
    if (k == n2) {
      //base case
      float val =  current_weights[k];
      // should only get to base case on update
      //cout << "Update" << k << endl;
      assert (update_filter[k]);
      if (val < best_split_score[n1][n2]) {
        best_split[n1][n2] = k;
        best_split_score[n1][n2] = val;
      }
    } else {
      float a, b;
      if (need_to_recompute[n1][k]) {
        find_shortest(n1, k, best_split, best_split_score);      
      } 
      a = best_split_score[n1][k];
      
      if (need_to_recompute[k][n2]) {
        find_shortest(k, n2, best_split, best_split_score);      
      } 
      b = best_split_score[k][n2];
      
      if (a + b < best_split_score[n1][n2]) {
        best_split[n1][n2] = k;
        best_split_score[n1][n2] = a+b;
        //array.clear();
        //array.insert(array.end(), one.begin(), one.end());
        //array.insert(array.end(), two.begin(), two.end());
      }
    }
  }
}


void BigramRescore::recompute_bigram_weights(bool initialize) {

  int count = 0;
  recomputed =0;

  if (initialize || !OPTIMIZE) {
    update_filter.set();
    for (int i=0; i < graph->num_nodes; i++) {
      for (int j=0;j < graph->num_nodes; j++) {
        need_to_recompute[i][j] = 1;
      }
    }
  } else { 
    bitset <NUMSTATES> seen;
    seen.reset();
    for (int k=0; k < update_len; k++ ) {
      int up_node = update_position[k];
      //cout << up_node << endl;
      //if (seen[up_node]) continue;

      for (int i=0; i < forward_paths[up_node].size(); i++) {
        int rnode = forward_paths[up_node][i];
        if (graph->word_node[rnode] != 1) {
          seen[rnode] = 1;
        }

        for (int j=0; j < backward_paths[up_node].size(); j++) {

          int lnode = backward_paths[up_node][j];

          //cout << lnode << " " << rnode << endl;
          count ++;
          need_to_recompute[lnode][rnode] = 1; 
        }
      }
    }
  } 

  for (unsigned int i=0; i< gd->valid_bigrams.size() ;i++) {
        
    Bigram b = gd->valid_bigrams[i];
    int w1 = b.w1;
    int w2 = b.w2;
     
    // first do it with recursion
    if (need_to_recompute[w1][w2]) {
      find_shortest(w1, w2, best_split, bigram_weights); 
       if (bigram_path[w1][w2] != NULL) {
         bigram_path[w1][w2]->clear();
       }
       //reconstruct_path(w1, w2, best_split, bigram_path[w1][w2]); 
    }
  }
  cout << "COUNT " << count << " " << recomputed << endl;
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
