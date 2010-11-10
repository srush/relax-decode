#ifndef DUALSUB_H_
#define DUALSUB_H_

//#include "Bigram.h"
#include "ForestLattice.h"
#include "EdgeCache.h"
#include "GraphDecompose.h"
#include "BigramRescore.h"
#include <Ngram.h>
#define NUMWORDS 300

class Subproblem {
 public: 
  vector<int> cur_best_one[NUMSTATES];
  vector<int> cur_best_two[NUMSTATES];
  float cur_best_score[NUMSTATES];
  int cur_best_count[NUMSTATES];

  int last_best_one[NUMSTATES];
  int last_best_two[NUMSTATES];
  float last_best_score[NUMSTATES];


  int cur_best_bigram[NUMSTATES];
  float cur_best_score_bigram[NUMSTATES];

  Subproblem(const ForestLattice *g, Ngram * lm_in, const GraphDecompose * gd_in, const Cache<LatNode, int> & word_node_cache_in);
  void update_weights(vector <int> u_pos, vector <float> u_values, bool first);
  void solve();
  //void solve_bigram();
  vector <int> get_best_nodes_between(int w1, int w2, bool first);
  float get_best_bigram_weight(int w1, int w2 , bool first);
  float primal_score(int word[NUMWORDS], int l);
  
  double word_prob(int, int, int );
  double word_backoff(int );
  double word_backoff_two(int i, int j);
  double word_prob_reverse(int, int, int);
  double word_prob_bigram_reverse(int i, int j);
  int word_bow_reverse(int i, int j, int k);
 private:
  
  bool first_time;

  // Weight management
  bitset <NUMSTATES> update_filter;
  
  // PROBLEMS
  
  // The lagragian score associated with a bigram 
  //vector<float> bigram_weights[NUMSTATES][NUMSTATES];

  // current best weight associated with a 
  //float bigram_weights[NUMSTATES][NUMSTATES];
  //vector <int> bigram_path[NUMSTATES][NUMSTATES];



  float best_lm_score[NUMSTATES][NUMSTATES];  
  float bigram_score_cache[NUMSTATES][NUMSTATES];  

  vector <int> * forward_trigrams [NUMSTATES][NUMSTATES];  
  vector <double> * forward_trigrams_score [NUMSTATES][NUMSTATES];  
  //Bigram valid_bigrams[NUMSTATES*NUMSTATES];
  
  const ForestLattice * graph;
  Ngram * lm;
  
  const GraphDecompose * gd;
  const Cache<LatNode, int> _word_node_cache;

  BigramRescore * bi_rescore_first;
  BigramRescore * bi_rescore_two;
};

//Subproblem * initialize_subproblem(const char* graph_file, const char* word_file, const char * lm_file );

#endif
