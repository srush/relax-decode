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
  gd(gd_in), graph(graph_in)  {
  // huge hack
  int num_nodes = max((int)graph_in->get_graph().num_nodes(), graph_in->num_word_nodes);
  current_weights.resize(num_nodes);
  update_position.resize(num_nodes);
  update_filter.resize(num_nodes);

  int num_actual_nodes = graph_in->get_graph().num_nodes();
  
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


  for (int i=0; i< graph->get_graph().num_nodes(); i++) {  
    update_filter[i] =  0;
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
    cout << "RECOMPUTE: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
  }
}


void BigramRescore::cache_forward() {  

  foreach (Node n1, graph->get_graph().nodes()) {
    foreach (Node i, graph->get_graph().nodes()) {

      if (!graph->is_phrase_node(*n1)) {
        if (gd->path_exists(*n1, *i) || i->id() == n1->id()) {
          forward_paths[n1->id()].push_back(i->id());
        }
      } 
      if (gd->path_exists(*i, *n1)) {
        backward_paths[n1->id()].push_back(i->id());
      }
    }

    if (graph->is_phrase_node(*n1)) {
      forward_paths[n1->id()].push_back(n1->id());
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

void BigramRescore::find_shortest(const Graphnode & no1, const Graphnode & no2) {
  int n1 = no1.id();
  int n2 = no2.id();
  
  assert (need_to_recompute[n1][n2]);
  recomputed++;
  
  need_to_recompute[n1][n2] =0;

  int old_split = best_split[n1][n2];

  if (n1 != n2) 
    bigram_weights[n1][n2] = INF;

  assert(gd->path_exists(no1, no2));

  const vector <Node> * path = gd->get_path(no1,no2);
  int s = path->size();
  for (int split=0; split < s; split++) {
    Node k = (*path)[split];
    
    if (k->id() == n2) {
      //base case
      int edge_id = graph->get_edge_label(no1, no2);
      
      float val;
      if (edge_id != -1) 
        val = current_weights[edge_id];
      else
        val = 0.0;
      // should only get to base case on update
      //cout << "Update" << k << endl;
      //assert (update_filter[k]);
      if (val < bigram_weights[n1][n2]) {
        best_split[n1][n2] = k->id();
        bigram_weights[n1][n2] = val;
      }
    } else {
      float a, b;
      if (need_to_recompute[n1][k->id()]) {
        find_shortest(no1, *k);      
      } 
      a = bigram_weights[n1][k->id()];
      
      if (need_to_recompute[k->id()][n2]) {
        find_shortest(k->id(), no2);      
      } 
      b = bigram_weights[k->id()][n2];
      
      if (a + b < bigram_weights[n1][n2]) {
        best_split[n1][n2] = k->id();
        bigram_weights[n1][n2] = a+b;
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
    
    for (int i=0; i < graph->get_graph().num_nodes(); i++) {
      update_filter[i] = 1;
      for (int j=0;j < graph->get_graph().num_nodes(); j++) {
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
    cout << "Dirty: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
  
    begin = clock();
  }
  //for (unsigned int i=0; i< gd->valid_bigrams.size() ;i++) {
  foreach (const WordBigram & b, gd->valid_bigrams()) {
    int w1 = b.w1.id();
    int w2 = b.w2.id();

    Node n1 = graph->lookup_word(b.w1);
    Node n2 = graph->lookup_word(b.w2);
     
    // first do it with recursion
    if (need_to_recompute[n1->id()][n2->id()] && gd->path_exists(*n1, *n2)) {
      find_shortest(n1->id(), n2->id()); 
      if (bigram_path[n1->id()][n2->id()] != NULL) {
        bigram_path[n1->id()][n2->id()]->clear();
      }
    }
  }
  if (TIMING) {
    cout << "COUNT " << count << " " << recomputed << " " << score_changed << endl;
    end=clock();
    cout << "FIND Shortest: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
  }
}


