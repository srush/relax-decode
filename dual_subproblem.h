#ifndef DUALSUB_H_
#define DUALSUB_H_

//#include "Bigram.h"
#include "ForestLattice.h"
#include "EdgeCache.h"
#include "GraphDecompose.h"
#include "BigramRescore.h"
#include "NGramCache.h"
//#define NUMWORDS 300
#include "common.h"
#include<fstream>
#include<set>
#define MAX_PROJ 30
#define PROJECT 0
#define TRIPROJECT 1

using namespace std;
class SkipTrigram;

class Subproblem {
 private:


  bool _first_time_proj[MAX_PROJ][MAX_PROJ];
  vector <int> cur_best_count;

  // For project
  vector <int> projection;
  vector <vector<int> > cur_best_at_bi;
  vector <vector <float> > cur_best_at_bi_score;

  vector <vector<int> > cur_best_bi_projection;
  vector <vector<int> > cur_best_bi_projection_first;
  vector <vector <float> > cur_best_bi_projection_score;

  vector <vector <vector<int> > > cur_best_tri_projection;
  vector <vector <vector<int> > > cur_best_tri_projection_first;
  vector <vector <vector <float> > > cur_best_tri_projection_score;
  
  // has the best changed
  vector <vector <vector<bool> > > cur_best_tri_projection_is_new;

  // make private (eventually)
  vector <float> cur_best_score;
  vector<int>  cur_best_one;
  vector<int>  cur_best_two;

  void solve_proj(int d2, int d3, bool first_time_proj, 
                            vector <int> & proj_best_one,
                            vector <int> & proj_best_two,
                  vector <float> & proj_best_score,
                  vector <bool> & proj_best_is_new,
                  bool is_simple
                  ) ;

 public: 

  vector <bool > overridden;

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
    if (PROJECT) {
      int d = projection[w2];
      int proj = cur_best_bi_projection_first[d][w1];
      assert (proj != -1);
      return cur_best_bi_projection_first[d][w1];
    } else if (TRIPROJECT) {
      int d = projection[w2];
      int d2 = projection[w3];
      return cur_best_tri_projection_first[d][d2][w1];
    } else {
      return cur_best_one[w1];
    }
  }

  inline int best_two(int w1, int w2, int w3) const {
    if (PROJECT) {
      int d = projection[w2];
      int proj = cur_best_bi_projection[d][w1];
      assert (proj != -1);
      return cur_best_bi_projection[d][w1];
    } else if (TRIPROJECT) {
      int d = projection[w2];
      int d2 = projection[w3];
      return cur_best_tri_projection[d][d2][w1];
    } else {
      return cur_best_two[w1];
    }
  }

  inline double best_score_dim(int w1, int d, int d2) const {
    if (PROJECT) {
      return cur_best_bi_projection_score[d][w1];
    } else if (TRIPROJECT) {
      return cur_best_tri_projection_score[d][d2][w1];
    }
  }

  inline double best_score_dim_min(int w1, vector<int> ds, vector<int> ds2) const {
    double m = INF;
    foreach (int d, ds) {
      foreach (int d2, ds2) {
        m = min(m, (double)cur_best_tri_projection_score[d][d2][w1]);
      }
    }
    return m;
  }


  inline bool is_new_dim(int w1, int d, int d2) const {
    assert(TRIPROJECT);
    return cur_best_tri_projection_is_new[d][d2][w1];
  }



  

  inline double best_score(int w1, int w2, int w3) const {
    if (PROJECT) {
      int d = projection[w2];
      return cur_best_bi_projection_score[d][w1];
    } else if (TRIPROJECT) {
      int d = projection[w2];
      int d2 = projection[w3];
      return cur_best_tri_projection_score[d][d2][w1];
    } else {
      return cur_best_score[w1];
    }
  }



  Subproblem(const ForestLattice *g, NgramCache * lm_in, const SkipTrigram & skip, const GraphDecompose * gd_in, const Cache<LatNode, int> & word_node_cache_in);
  void update_weights(vector <int> u_pos, vector <float> u_values, bool first);
  void solve();
  void initialize_caches();
  //void solve_bigram();
  vector <int> get_best_nodes_between(int w1, int w2, bool first);
  float get_best_bigram_weight(int w1, int w2 , bool first);
  float primal_score(int word[], int l);
  
  double word_prob(int, int, int );
  double word_backoff(int );
  double word_backoff_two(int i, int j);
  double word_prob_reverse(int, int, int);
  double word_prob_bigram_reverse(int i, int j);
  int word_bow_reverse(int i, int j, int k);
  int word_bow_bigram_reverse(int i, int j);
 private:
  
  bool first_time;

  int fixed_last_bigram(int w1);  
  // PROBLEMS
  
  // The lagragian score associated with a bigram 
  //vector<float> bigram_weights[NUMSTATES][NUMSTATES];

  // current best weight associated with a 
  //float bigram_weights[NUMSTATES][NUMSTATES];
  //vector <int> bigram_path[NUMSTATES][NUMSTATES];



  // BIG
  vector < vector<float > >  best_lm_score;
  // BIG
  vector < vector<float > >  bigram_score_cache;

  vector < vector<float > >  backoff_score_cache;
  
  vector < vector<float > >  bigram_weight_cache_one;
  vector < vector<float > >  bigram_weight_cache_two;

  vector < vector<bool > >   bigram_in_lm;
  // BIG
  vector <vector <vector <int> *> > forward_trigrams;
  //BIG
  vector < vector< vector <double> * > > forward_trigrams_score;
  //Bigram valid_bigrams[NUMSTATES*NUMSTATES];

  const SkipTrigram & skip_tri;
  
  const ForestLattice * graph;
  NgramCache * lm;
  
  const GraphDecompose * gd;
  const Cache<LatNode, int> _word_node_cache;

  BigramRescore * bi_rescore_first;
  BigramRescore * bi_rescore_two;
};

class SkipTrigram {
 public:
  void initialize(const char * filename, NgramCache & lm) {
    int max_size = lm.vocab.numWords();
    _skip_tri.resize(max_size);
    cout << "Start " << max_size << endl;
    
    cout << "Start" << endl;
    ifstream fin(filename ,ios::in);
    while (!fin.eof()) {
      string word1, word2;
      int w1, w2;
      fin >> word1 >> word2;
      if (fin.eof()) break;
      w1 = lm.vocab.getIndex(word1.c_str()); 
      w2 = lm.vocab.getIndex(word2.c_str());
      //cout << w1 << " " << w2 << endl;
      //cout << word1 << " " << word2 << endl;
      assert (w1 != -1 && w1 < max_size);
      assert (w2 != -1 && w2 < max_size);
      _skip_tri[w1][w2] = true; 
    }
    cout << "end" << endl;
    fin.close();
  }

  inline bool is_skip(int i, int j) const  {
    assert(i != -1 );
    assert(j != -1 );
    //return _skip_tri[i].find(j) != _skip_tri[i].end();
    return _skip_tri[i][j];
  }

 private:
  vector < bitset<100000> > _skip_tri;
};

//Subproblem * initialize_subproblem(const char* graph_file, const char* word_file, const char * lm_file );

#endif

