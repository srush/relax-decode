#ifndef DUALSUB_H_
#define DUALSUB_H_

#include <fstream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <string>



#include "ForestLattice.h"
#include "EdgeCache.h"
#include "GraphDecompose.h"
#include "BigramRescore.h"
#include "NGramCache.h"

#include "../common.h"
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
  // TODO(srush)
  Subproblem(const ForestLattice *g,
             NgramCache * lm_in,
             const GraphDecompose * gd_in,
             const Cache<Graphnode, int> & word_node_cache_in);

  // TODO(srush)
  void update_weights(vector<int> u_pos, vector<float> u_values, int pos);

  // TODO(srush)
  void solve();

  // TODO(srush)
  void project(int proj_dim, vector<int> projection);

  // TODO(srush)
  inline int project_word(int w) const { return projection[w]; }

  // TODO(srush)
  void projection_with_constraints(int limit,
                                   int & k,
                                   map<int, set <int> > & constraints,
                                   vector<int> &);

  // TODO(srush)
  vector<int> get_best_nodes_between(int w1, int w2, int pos);

  // TODO(srush)
  float get_best_bigram_weight(int w1, int w2 , int pos);

  // TODO(srush): move to helper.
  double word_prob_reverse(int i, int j, int k) {
    VocabIndex context[] = {_word_node_cache.store[j],
                            _word_node_cache.store[k], Vocab_None};
    return lm->wordProb(_word_node_cache.store[i], context);
  }

  // TODO(srush) move to helper
  bool is_overridden(int graph_id) const { 
    return overridden[graph_id]; 
  }

  // TODO(srush) move to helper
  inline bool has_no_trigram(int w) const {
    for (uint j =0; j < gd->forward_bigrams[w].size(); j++) {
      int w1 = gd->forward_bigrams[w][j];
      if (!gd->forward_bigrams[w1].empty()) return false;
    }
    return true;
  }

  // TODO(srush)
  inline int overridden_by(int w) const {
    assert(overridden[w]);
    return gd->forward_bigrams[w][0];
  }

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

  inline double best_score_dim_min(int w1,
                                   vector<int> ds,
                                   vector<int> ds2) const {
    double m = INF;
    foreach (int d, ds) {
      foreach (int d2, ds2) {
        m = min(m,
                static_cast<double>(
                    cur_best_for_projection[d][d2][w1].score));
      }
    }
    return m;
  }

  inline double best_score(int w1, int w2, int w3) const {
    int d = projection[w2];
    int d2 = projection[w3];
    return cur_best_for_projection[d][d2][w1].score;
  }

  int fixed_last_bigram(int w1);

  vector<bool> overridden;

  int projection_dims;

 private:
  // TODO(srush) move to helper
  double word_backoff_two(int i, int j) {
    VocabIndex context[] = {_word_node_cache.store[i],
                            _word_node_cache.store[j], Vocab_None};
    double score = lm->contextBOW(context, 1);
    return score;
  }

  // TODO(srush) move to helper
  double word_prob_bigram_reverse(int i, int j) {
    VocabIndex context[] = {_word_node_cache.store[j],
                            lm->vocab.getIndex(Vocab_Unknown), Vocab_None};
    return lm->wordProb(_word_node_cache.store[i], context);
  }

  // TODO(srush) move to helper
  int word_bow_reverse(int i, int j, int k) {
    VocabIndex context[] = {_word_node_cache.store[j],
                            _word_node_cache.store[k], Vocab_None};
    unsigned int length;
    lm->contextID(_word_node_cache.store[i], context, length);
    return length;
  }

  // TODO(srush) move to helper
  int word_bow_bigram_reverse(int i, int j) {
    VocabIndex context[] = {_word_node_cache.store[j], Vocab_None};
    unsigned int length;
    lm->contextID(_word_node_cache.store[i], context, length);
    return length;
  }

  // TODO(srush) Document
  void initialize_caches();

  // TODO(srush) Document
  void try_set_max(vector <ProjMax> & proj_best,
                   int w1,
                   int w2,
                   int w3,
                   double score,
                   bool is_new) {
    assert(score < 1000);
    if (score < proj_best[w1].score || proj_best[w1].score == INF) {
      proj_best[w1].score = score;
      proj_best[w1].ord_best[0] = w2;
      proj_best[w1].ord_best[1] = w3;
      proj_best[w1].is_new = true;
      assert(proj_best[w1].ord_best[0] != proj_best[w1].ord_best[1]);
    }
  }

  bool first_time;

  // PROBLEMS
  const double _lm_weight;

  vector < vector<float > >  best_lm_score;
  vector < vector<float > >  bigram_score_cache;
  vector < vector<float > >  backoff_score_cache;
  vector < vector < vector<float > > >  bigram_weight_cache;

  vector < vector<bool > >   bigram_in_lm;
  vector <vector <vector <int> *> > forward_trigrams;
  vector < vector< vector <double> * > > forward_trigrams_score;

  const ForestLattice * graph;
  NgramCache * lm;

  const GraphDecompose *gd;
  const Cache<Graphnode, int> _word_node_cache;


  vector<BigramRescore *> bi_rescore;

  bool _first_time_proj[MAX_PROJ][MAX_PROJ];

  // For project move to helper.
  vector<int> projection;
  vector<vector<int> > cur_best_at_bi;
  vector<vector<float> > cur_best_at_bi_score;

  vector<vector<vector<ProjMax> > > cur_best_for_projection;

  void solve_proj(int d2, int d3, bool first_time_proj,
                  vector <ProjMax> & proj_best,
                  bool is_simple);
};

#endif

