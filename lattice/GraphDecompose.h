#ifndef GRAPHDECOMPOSE_H_
#define GRAPHDECOMPOSE_H_


#include <vector> 
#include <bitset> 
#include "ForestLattice.h"

using namespace std; 

class GraphDecompose {
 public: 

  
  vector <vector <int> > forward_bigrams;
  vector <vector <int> > backward_bigrams;
  
  GraphDecompose( const ForestLattice & lattice): _lat(lattice){}
  void decompose();

  
  bool path_exists(const Graphnode & n1, const Graphnode & n2) const {
    //assert (w1 < _lat.num_nodes && w2 < _lat.num_nodes);
    return all_pairs_path_exist[n1.id()][n2.id()];
  }

  /* bool path_exists(int w1, int w2) const { */
  /*   //assert (w1 < _lat.num_nodes && w2 < _lat.num_nodes); */
  /*   return all_pairs_path_exist[w1][w2]; */
  /* } */


  const vector <Node> * get_path(const Graphnode & n1, const Graphnode & n2) const {
    return all_pairs_path[n1.id()][n2.id()];
  }


  // Do Not use
  vector <Node> * get_path(int w1, int w2) const {
    return all_pairs_path[w1][w2];
  }

  const vector <WordBigram> & valid_bigrams() const {
    return _valid_bigrams;
  }

 private:
    // DP chart - points to the next node on the path
  
  vector< vector<vector<Node> * > > all_pairs_path;
  vector< vector<bool> > all_pairs_path_exist;

  vector <vector <vector <vector <int> > > >  bigram_paths;
  const ForestLattice & _lat;

  vector <WordBigram> _valid_bigrams;

  void compute_bigrams();
  void graph_to_all_pairs();
  //  void reconstruct_path(const Graphnode &  n, const Graphnode & n2, vector <vector <int> > & array);
};

#endif
