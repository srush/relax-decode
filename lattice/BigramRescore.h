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

  
  void recompute_bigram_weights(bool init);
  

  vector <int> get_bigram_path(int w1, int w2) {
    if (bigram_path[w1][w2] == NULL) {
      bigram_path[w1][w2] = new vector<int>();
    }

    if (bigram_path[w1][w2]->empty()) {
      reconstruct_path(w1, w2, best_split, *bigram_path[w1][w2]);
    }

    assert (bigram_path[w1][w2] != NULL); 
    return (*bigram_path[w1][w2]);
  }

  float get_bigram_weight(int i, int j) {
    return bigram_weights[i][j];
  }


 private:
  void reconstruct_path(int n1, int n2, const vector <vector< int> > & best_split, vector <int > & array );

  vector <float > current_weights;
  vector <int > update_position;
  vector <bool > update_filter;
  int update_len;


  vector < vector < vector <int> *> > bigram_path;

  vector < vector < int > > best_split;
  vector< vector<float> > bigram_weights;


  vector<vector <bool> > need_to_recompute;
  //vector <Bigram> for_updates[NUMSTATES];
  
  
  
  int recomputed , score_changed;
  void cache_paths(int, int);
  void cache_forward();
  vector< vector<int> > forward_paths;
  vector< vector<int> > backward_paths;
  
  void setup_problems();

  void find_shortest(int n1, int n2);
  const GraphDecompose * gd;  
  const ForestLattice * graph;  
  //vector <vector <bitset <NUMSTATES> > > bigram_cache;

};
#endif
