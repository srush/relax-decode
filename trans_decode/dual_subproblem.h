#ifndef DUALSUB_H_
#define DUALSUB_H_

//#include "Bigram.h"
#include "ForestLattice.h"
#include "EdgeCache.h"
#include "GraphDecompose.h"
#include "BigramRescore.h"
#include "NGramCache.h"
//#define NUMWORDS 300
#include "../common.h"
#include <fstream>
#include <set>
#define MAX_PROJ 30
#define PROJECT 0
#define TRIPROJECT 1

// transition variable
// turn on to include <s> in tree
#define FULLBUILT 1
#define ORDER 3

using namespace std;


struct ProjMax {
  double score;
  vector <int> ord_best;
  bool is_new;
};

class Subproblem {
 public: 

  vector <bool > overridden;

  inline bool has_no_trigram(int w) const {
    for (uint j =0; j < gd->forward_bigrams[w].size(); j++) {
      int w1 = gd->forward_bigrams[w][j];
      if (!gd->forward_bigrams[w1].empty()) return false;
    }
    return true;
  }

  inline int overridden_by(int w) const {
    assert(overridden[w]);
    return gd->forward_bigrams[w][0];
  }

  int projection_dims;
  void project(int proj_dim, vector <int> projection);

  inline int project_word(int w ) const {
    return projection[w];
  }

  inline void separate(int w1, int w2) {
    if (projection[w1] == projection[w2] )
      projection[w1] = (projection[w1] + 1) % projection_dims; 
  }

  //vector <int> rand_projection(int k) ;
  void projection_with_constraints(int limit, int & k, map<int, set <int> > & constraints, vector <int> &) ;
  // End Project

  inline int best_one(int w1, int w2, int w3) const {
    int d = projection[w2];
    int d2 = projection[w3];
    return cur_best_for_projection[d][d2][w1].ord_best[0];
  }

  inline int best_two(int w1, int w2, int w3) const {
    int d = projection[w2];
    int d2 = projection[w3];
    return cur_best_for_projection[d][d2][w1].ord_best[1];
  }

  inline double best_score_dim(int w1, int d, int d2) const {
    return cur_best_for_projection[d][d2][w1].score;
  }

  inline double best_score_dim_min(int w1, vector<int> ds, vector<int> ds2) const {
    double m = INF;
    foreach (int d, ds) {
      foreach (int d2, ds2) {
        m = min(m, (double)cur_best_for_projection[d][d2][w1].score);
      }
    }
    return m;
  }


  inline bool is_new_dim(int w1, int d, int d2) const {
    return cur_best_for_projection[d][d2][w1].is_new;
  }



  

  inline double best_score(int w1, int w2, int w3) const {
    int d = projection[w2];
    int d2 = projection[w3];
    return cur_best_for_projection[d][d2][w1].score;
  }



  Subproblem(const ForestLattice *g, NgramCache * lm_in, const GraphDecompose * gd_in, 
             const Cache<Graphnode, int> & word_node_cache_in);
  void update_weights(vector <int> u_pos, vector <float> u_values, int pos);
  void solve();
  void initialize_caches();
  //void solve_bigram();
  vector <int> get_best_nodes_between(int w1, int w2, int pos);
  float get_best_bigram_weight(int w1, int w2 , int pos);
  float primal_score(int word[], int l);
  
  double word_prob(int, int, int );
  double word_backoff(int );
  double word_backoff_two(int i, int j);
  double word_prob_reverse(int, int, int);
  double word_prob_bigram_reverse(int i, int j);
  int word_bow_reverse(int i, int j, int k);
  int word_bow_bigram_reverse(int i, int j);

  bool is_overridden(int graph_id) const {
    return overridden[graph_id];
  }

  void try_set_max(vector <ProjMax> & proj_best, int w1, int w2, int w3, double score, bool is_new ) {
    assert (score < 1000);
    if (score < proj_best[w1].score || proj_best[w1].score == INF) {
      proj_best[w1].score = score;          
      proj_best[w1].ord_best[0] = w2;
      proj_best[w1].ord_best[1] = w3;
      proj_best[w1].is_new = true;
      assert(proj_best[w1].ord_best[0] != proj_best[w1].ord_best[1]);           
    } 
  }

 private:
  
  bool first_time;

  int fixed_last_bigram(int w1);  
  // PROBLEMS
  

  const double _lm_weight;

  // BIG
  vector < vector<float > >  best_lm_score;
  // BIG
  vector < vector<float > >  bigram_score_cache;

  vector < vector<float > >  backoff_score_cache;
  
  vector < vector < vector<float > > >  bigram_weight_cache;

  vector < vector<bool > >   bigram_in_lm;
  // BIG
  vector <vector <vector <int> *> > forward_trigrams;
  //BIG
  vector < vector< vector <double> * > > forward_trigrams_score;

  
  const ForestLattice * graph;
  NgramCache * lm;
  
  const GraphDecompose * gd;
  const Cache<Graphnode, int> _word_node_cache;


  vector <BigramRescore *> bi_rescore;

  bool _first_time_proj[MAX_PROJ][MAX_PROJ];

  // For project
  vector <int> projection;
  vector <vector<int> > cur_best_at_bi;
  vector <vector <float> > cur_best_at_bi_score;

  vector <vector <vector<ProjMax> > > cur_best_for_projection; 

  void solve_proj(int d2, int d3, bool first_time_proj, 
                  vector <ProjMax> & proj_best, 
                  bool is_simple
                  ) ;

};

#endif

