#ifndef BIGRAMRESCORE_H_
#define BIGRAMRESCORE_H_

#include <cmath>
#include "ForestLattice.h"
#include "GraphDecompose.h"
#include <iostream>
#include "../common.h"
#include <vector>
using namespace std;
class BigramRescore {

 public:

  BigramRescore(const ForestLattice * graph_in, const GraphDecompose * gd_in);
  void update_weights(vector<int> u_pos, vector<float> u_values, int len);


  void recompute_bigram_weights(bool init);


  const vector <int> get_bigram_path(int w1, int w2) {
    assert(w1 >= 0);
    assert(w2 >= 0);

    // for testing
    //get_bigram_weight(w1, w2);

    Node no1 = graph->lookup_word(w1);
    Node no2 = graph->lookup_word(w2);

    int n1 = no1->id();
    int n2 = no2->id();

    assert(gd->path_exists(n1, n2));
    if (bigram_path[n1][n2] == NULL) {
      bigram_path[n1][n2] = new vector<int>();
    }

    if (bigram_path[n1][n2]->empty()) {
      reconstruct_path(n1, n2, best_split, *bigram_path[n1][n2]);

      // We do NOT want this since many node-node paths
      // bigram_path[n1][n2]->push_back(w2);
     }

    //cout << "PATH " << n1 << " " << n2 << endl;
    // SANITY CHECK (remove!)

    //double should_score = 0.0;
    /* foreach (int id, (*bigram_path[n1][n2])) { */
    /*   //cout << "\t on path" << (*bigram_path[n1][n2])[i]<< endl; */
    /*   should_score += current_weights[id]; */
    /*   cout << "BIGRAMPATH" << " " << id << " " << current_weights[id] << endl; */
    /* } */
    //cout << should_score << " " << bigram_weights[n1][n2] << " " << n1 << " " << n2 << endl;
    //assert (fabs(should_score -bigram_weights[n1][n2]) < 1e-4);

    assert (bigram_path[n1][n2] != NULL);
    //cout << bigram_path[n1][n2]->size() << endl;
    return (*bigram_path[n1][n2]);
  }

  inline float get_bigram_weight(int w1, int w2) {
    // w1 and w2 are word ids
    Node n1 = graph->lookup_word(w1);
    Node n2 = graph->lookup_word(w2);
    //assert (gd->path_exists(n1, n2));
    //assert(bigram_weights[n1][n2] != INF);
    //cout << "Bigram weights "<<  bigram_weights[n1][n2] << " " <<  current_weights[w2] << endl;
    return bigram_weights[n1->id()][n2->id()] + current_weights[w2];
  }


 private:
  void reconstruct_path(int n1, int n2, const vector <vector< int> > & best_split, vector <int > & array );

  vector <float > current_weights;
  vector <int > update_position;
  //vector <bool > update_filter;
  vector <bool > update_filter;
  int update_len;


  vector < vector < vector <int> *> > bigram_path;

  vector < vector < int > > best_split;
  vector< vector<float> > bigram_weights;


  vector<vector <bool> > need_to_recompute;
  //vector <Bigram> for_updates[NUMSTATES];



  int recomputed, score_changed;
  void cache_paths(int, int);
  void cache_forward();
  vector<vector<int> > forward_paths;
  vector<vector<int> > backward_paths;

  void setup_problems();
  void find_shortest(const Graphnode & n1, const Graphnode & n2);

  const GraphDecompose * gd;
  const ForestLattice * graph;
  //vector <vector <bitset <NUMSTATES> > > bigram_cache;

};
#endif
