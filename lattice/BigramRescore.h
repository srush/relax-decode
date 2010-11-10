#ifndef BIGRAMRESCORE_H_
#define BIGRAMRESCORE_H_

#include "ForestLattice.h"
#include "GraphDecompose.h"

#include <vector>
using namespace std;
class BigramRescore {

 public:   

  BigramRescore(const ForestLattice * graph_in, const GraphDecompose * gd_in);
  void update_weights(vector<int> u_pos, vector<float> u_values, int len);
  float bigram_weights[NUMSTATES][NUMSTATES];

  float current_weights[NUMSTATES];
  vector <int> * bigram_path[NUMSTATES][NUMSTATES];

  //float update_values[NUMSTATES];
  int update_position[NUMSTATES];
  int update_len;
  bitset <NUMSTATES> update_filter;
  void recompute_bigram_weights(bool init);
  int move_direction[NUMSTATES][NUMSTATES]; 

  int best_split[NUMSTATES][NUMSTATES]; 
  void reconstruct_path(int n1, int n2, int best_split[NUMSTATES][NUMSTATES], vector <int > & array );
 private:
  bool need_to_recompute[NUMSTATES][NUMSTATES];
  vector <Bigram> for_updates[NUMSTATES];
  
  float best_split_score[NUMSTATES][NUMSTATES]; 
  
  int recomputed;
  void cache_paths(int, int);
  void cache_forward();
  vector<int> forward_paths[NUMSTATES];
  vector<int> backward_paths[NUMSTATES];
  
  void setup_problems();

  void find_shortest(int n1, int n2,
                               int best_split[NUMSTATES][NUMSTATES], 
                                 float best_split_score[NUMSTATES][NUMSTATES]);

  const GraphDecompose * gd;  
  const ForestLattice * graph;  
  vector <vector <bitset <NUMSTATES> > > bigram_cache;

};
#endif
