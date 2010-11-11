#ifndef GRAPHDECOMPOSE_H_
#define GRAPHDECOMPOSE_H_


#include <vector> 
#include <bitset> 
#include "ForestLattice.h"

using namespace std; 

class GraphDecompose {
 public: 
  //vector<int> all_pairs_path_length[NUMSTATES][NUMSTATES];

  vector <Bigram> valid_bigrams;
  //vector <Bigram> for_updates[NUMSTATES];
  vector <vector <int> > forward_bigrams;
  //vector <bitset <NUMSTATES> > bigram_bitset[NUMSTATES][NUMSTATES];

  //vector <vector <int> >  bigram_pairs[NUMSTATES][NUMSTATES];
  GraphDecompose();
  void decompose( const ForestLattice * g);

  bool path_exists(int w1, int w2) const {
    assert (w1 < g->num_nodes && w2 < g->num_nodes);
    return all_pairs_path_exist[w1][w2];
  }

  vector <int> * get_path(int w1, int w2) const {
    return all_pairs_path[w1][w2];
  }

 private:
    // DP chart - points to the next node on the path
  
  vector< vector<vector<int> * > > all_pairs_path;
  vector< vector<bool> > all_pairs_path_exist;

  
  const ForestLattice * g;

  void compute_bigrams();
  void graph_to_all_pairs();
  void reconstruct_path(int n, int n2, vector <vector <int> > & array);
  //void all_pairs_to_bigram();
};

#endif
