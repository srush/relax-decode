#include <time.h>
#include "GraphDecompose.h"
#include "dual_subproblem.h"
#include "NGramCache.h"
#include "EdgeCache.h"
#include "../common.h"
//#include <cstdlib>
//#include "GraphColor.h"
//#define INF 100000000
#define DEBUG 0
#define TIMING 0
#define OPTIMIZE 1


#include <bitset>

using namespace std;


Subproblem::Subproblem(const ForestLattice * g, 
                       NgramCache *lm_in, 
                       const GraphDecompose * gd_in, 
                       const Cache<Graphnode, int> & word_node_cache_in) :
  graph(g), lm(lm_in), gd(gd_in), 
  _word_node_cache(word_node_cache_in), 
  bi_rescore(ORDER-1), 
  bigram_weight_cache(ORDER-1), 
  _lm_weight(lm_weight())
{
  
  for (int ord =0; ord < ORDER -1; ord++) {
    bi_rescore[ord] = new BigramRescore(graph, gd_in);
  }

  int num_word_nodes = g->num_word_nodes;
  
  overridden.resize(num_word_nodes);


  cur_best_for_projection.resize(MAX_PROJ);
  for (int d=0; d < MAX_PROJ; d++) {
    cur_best_for_projection[d].resize(MAX_PROJ);
    for (int d2=0; d2 < MAX_PROJ; d2++) {
      cur_best_for_projection[d][d2].resize(num_word_nodes);
      for (int w=0; w < num_word_nodes; w++) {
        cur_best_for_projection[d][d2][w].ord_best.resize(ORDER-1);
      }
    }
  }
  

  best_lm_score.resize(num_word_nodes);
  bigram_score_cache.resize(num_word_nodes);
  backoff_score_cache.resize(num_word_nodes);
  bigram_in_lm.resize(num_word_nodes);
  
  for (int ord=0; ord < ORDER-1; ord++) {
    bigram_weight_cache[ord].resize(num_word_nodes);
  }


  forward_trigrams.resize(num_word_nodes);
  forward_trigrams_score.resize(num_word_nodes);

  for (int i=0; i < num_word_nodes; i ++) {
    best_lm_score[i].resize(num_word_nodes);
    bigram_score_cache[i].resize(num_word_nodes);
    backoff_score_cache[i].resize(num_word_nodes);
    bigram_in_lm[i].resize(num_word_nodes);
    forward_trigrams[i].resize(num_word_nodes);
    forward_trigrams_score[i].resize(num_word_nodes);

    for (int ord=0; ord < ORDER-1; ord++) {
      bigram_weight_cache[ord][i].resize(num_word_nodes);
    }   
  }
  
  for (int ord =0; ord < ORDER -1; ord++) {
    bi_rescore[ord]->recompute_bigram_weights(true);
  }

  first_time = true;
}


//void Subproblem::setup_problems() {
  //gd->decompose(graph);   
//}

void Subproblem::projection_with_constraints(int limit, int & k,  
                                             map<int, set <int> > & constraints, vector < int> & proj) {
  // Use DSATUR
  int most_constrained = 0;
  int most_constraints = -1;
  k = 0;
  for (int w1=0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    // count constraints
    int num_cons = constraints[w1].size();    
    if (num_cons > most_constraints){
      most_constraints =num_cons;
      most_constrained = w1;
    }
  }
  
  int cur = most_constrained; 
  proj.resize(graph->num_word_nodes);  
  int unprocessed = 0;
  for (int w1=0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    proj[w1] = 0;
    unprocessed++;
  }
  
  set <int> done;
  while (unprocessed >0) {
    //cout << "searching "<< cur << endl;
    vector <int> counts(k);
    for (int d =0; d < k; d++) {
      counts[d] = 0;
    }

    for (set<int>::const_iterator iter =constraints[cur].begin(); 
         iter != constraints[cur].end(); iter++) {
      int c = (*iter);
      if (done.find(c) != done.end()) {
        //assert(c < cur);
        counts[proj[c]]++;
      }
    }

    int min = INF;
    int mind;
    for (int d =0; d < k; d++) {
      if (counts[d] < min) {
        min = counts[d];
        mind = d;
      }
    }

    assert (cur < graph->num_word_nodes);
    if (min == 0) {
      proj[cur] = mind;
    } else if (k == limit) {
      //assert(false);
      proj[cur] = mind;
    } else {
      proj[cur] = k;
      k++;
    }


    for (set<int>::const_iterator iter =constraints[cur].begin(); 
         iter != constraints[cur].end(); iter++) {
      int conflict = (*iter);
      if (done.find(conflict) != done.end() && proj[conflict] == proj[cur]) {
        assert(false);
      }
    }

    assert(done.find(cur) == done.end());
    done.insert(cur);
    unprocessed--;
    // get next 

    // pick next to optimize (most saturated)
    int most_saturation = -1;
    for (int w1=0; w1 < graph->num_word_nodes; w1++) {
      if (!graph->is_word(w1)) continue;
      // only consider unassigned words
      if (done.find(w1) != done.end()) continue;

      int sat =0;
      vector <int> counts(k);
      for (int d =0; d < k; d++) {
        counts[d] = 0;
      }
      
      for (set<int>::const_iterator iter =constraints[w1].begin(); 
           iter != constraints[w1].end(); iter++) {

        int c = (*iter);
        if (done.find(c) != done.end()) {
          counts[proj[c]]++;
        }
      }
      for (int d =0; d < k; d++) {
        if (counts[d] > 0) {
          sat++;
        }
      }

      if (sat > most_saturation) {
        most_saturation =sat;
        cur = w1;
      }
    }

  }
}


void Subproblem::projection_with_constraints_str(int limit, int & k,  
                                                 map<string, set <string> > & constraints, vector <int> & ret_proj) {
  // Use DSATUR
  map <string, int> proj;
  int most_constrained = 0;
  int most_constraints = -1;
  k = 0;
  for (int w1=0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    // count constraints
    string word  = graph->get_word(w1);

    int num_cons = constraints[word].size();    
    if (num_cons > most_constraints){
      most_constraints =num_cons;
      most_constrained = w1;
    }
  }
  
  int cur = most_constrained; 
  string cur_str = graph->get_word(most_constrained);
  //proj.resize(graph->num_word_nodes);  
  int unprocessed = 0;
  for (int w1=0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    //proj[w1] = 0;
    unprocessed++;
  }
  
  set <string> done;
  while (unprocessed >0) { 
    if (done.find(cur_str) == done.end()) {

      //cout << "searching "<< cur << endl;
      vector <int> counts(k);
      for (int d =0; d < k; d++) {
        counts[d] = 0;
      }

      for (set<string>::const_iterator iter =constraints[cur_str].begin(); 
           iter != constraints[cur_str].end(); iter++) {
        string c = (*iter);
        if (done.find(c) != done.end()) {
          //assert(c < cur);
          counts[proj[c]]++;
        }
      }

      int min = INF;
      int mind;
      for (int d =0; d < k; d++) {
        if (counts[d] < min) {
          min = counts[d];
          mind = d;
        }
      }

      assert (cur < graph->num_word_nodes);
      if (min == 0) {
        proj[cur_str] = mind;
      } else if (k == limit) {
        //assert(false);
        proj[cur_str] = mind;
      } else {
        proj[cur_str] = k;
        k++;
      }


      for (set<string>::const_iterator iter =constraints[cur_str].begin(); 
           iter != constraints[cur_str].end(); iter++) {
        string conflict = (*iter);
        if (done.find(conflict) != done.end() && proj[conflict] == proj[cur_str]) {
          assert(false);
        }
      }

      assert(done.find(cur_str) == done.end());
      done.insert(cur_str);
    }


    unprocessed--;
    // get next 

    // pick next to optimize (most saturated)
    int most_saturation = -1;
    for (int w1=0; w1 < graph->num_word_nodes; w1++) {
      if (!graph->is_word(w1)) continue;
      //string cur_str = graph->get_word(w1);
      string w1_str = graph->get_word(w1);
      // only consider unassigned words
      if (done.find(w1_str) != done.end()) continue;

      int sat =0;
      vector <int> counts(k);
      for (int d =0; d < k; d++) {
        counts[d] = 0;
      }
      
      for (set<string>::const_iterator iter =constraints[w1_str].begin(); 
           iter != constraints[w1_str].end(); iter++) {

        string c = (*iter);
        if (done.find(c) != done.end()) {
          counts[proj[c]]++;
        }
      }
      for (int d =0; d < k; d++) {
        if (counts[d] > 0) {
          sat++;
        }
      }

      if (sat > most_saturation) {
        most_saturation =sat;
        cur = w1;
        cur_str = graph->get_word(w1);
      }
    }

  }
  // map to old style;
  ret_proj.resize(graph->num_word_nodes);  
  for (int w1=0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    ret_proj[w1] = proj[graph->get_word(w1)];
  }
}


// true - > 0
// false -> 1
void Subproblem::update_weights(vector <int> u_pos, vector <float> u_values, int pos) {
  assert (u_pos.size() == u_values.size()); 
  bi_rescore[pos]->update_weights(u_pos, u_values, u_pos.size());
}

// true - > 0
// false -> 1
vector <int> Subproblem::get_best_nodes_between(int w1, int w2, int pos) {
  //assert(path < gd->bigram_pairs[w1][w2].size());
  vector <int> path;
  path = bi_rescore[pos]->get_bigram_path(w1, w2);
  
  // This is where we add the extra node
  path.push_back(w2);
  return path;
}

float Subproblem::get_best_bigram_weight(int w1, int w2, int pos) {
  return bi_rescore[pos]->get_bigram_weight(w1,w2);
}

int Subproblem::fixed_last_bigram(int w1) {
  //return -1;
  assert(graph->is_word(w1));
  // if I am forward bound, return -1
  if (gd->forward_bigrams[w1].size() == 1) {
    int w2 = gd->forward_bigrams[w1][0];
    if (gd->backward_bigrams[w2].size() ==1) {
      return -1;
    }
  }

  if (gd->backward_bigrams[w1].size() == 1) {
    int w0 = gd->backward_bigrams[w1][0];
    assert (w0 != -1);
    if (gd->forward_bigrams[w0].size() ==1) {
      return w0;
    } else {
      return -1;
    }
  }
  return -1;
}

void Subproblem::initialize_caches() {

  foreach (const WordBigram & b, gd->valid_bigrams()) { 
    int w1 = b.w1.id();
    int w2 = b.w2.id();
    bigram_in_lm[w1][w2] =  word_bow_bigram_reverse(w1, w2);
    forward_trigrams[w1][w2] = new vector<int>();
    forward_trigrams_score[w1][w2] = new vector<double>();
    bigram_score_cache[w1][w2] = (_lm_weight) *  word_prob_bigram_reverse(w1, w2);
    backoff_score_cache[w1][w2] = (_lm_weight) *  word_backoff_two(w1, w2);

    best_lm_score[w1][w2] = INF;
  }
  
  for (int w1=0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    
    const vector <int> & f1 = gd->forward_bigrams[w1];
    
    for (unsigned int i =0; i< f1.size(); i++ ) {
      int w2 = f1[i];
      
      VocabIndex context [] = {_word_node_cache.store[w2], Vocab_None};        
      lm->wordProbPrimeCache(_word_node_cache.store[w1], context);
      
      for (unsigned int j =0; j < gd->forward_bigrams[w2].size(); j++) {
        int w3 = gd->forward_bigrams[w2][j];
        double lm_score;
        if (bigram_in_lm[w1][w2] && bigram_in_lm[w2][w3] &&  lm->hasNext(_word_node_cache.store[w3])) {         
          
          VocabIndex context [] = {_word_node_cache.store[w2], _word_node_cache.store[w3], Vocab_None};
           lm_score = (_lm_weight) *  lm->wordProbFromCache(_word_node_cache.store[w1], context);
          
          forward_trigrams[w1][w2]->push_back(w3);
          forward_trigrams_score[w1][w2]->push_back(lm_score);
          //cout << w1 << " " << w2 << " " << w3;
          
   
        } else {
            lm_score = backoff_score_cache[w2][w3] + bigram_score_cache[w1][w2];
            //cout << graph->get_word(w1)<< _word_node_cache.store[w1] << " " << graph->get_word(w2) << _word_node_cache.store[w2] << " " << graph->get_word(w3) << _word_node_cache.store[w3] << endl;
          //cout <<bigram_in_lm[w1][w2]<< " " << bigram_in_lm[w2][w3] << " " << skip_tri.is_skip(_word_node_cache.store[w3], _word_node_cache.store[w1]) << endl;
          //assert(word_bow_reverse(w1,w2,w3) != 2);
            //zeros++;
        }

        if (lm_score < best_lm_score[w1][w2]) {
          best_lm_score[w1][w2] = lm_score;
        }
      }
    }
  }  
}


void Subproblem::solve() {
  if (first_time) {
    initialize_caches();
    
    for (int d =0; d < MAX_PROJ; d++ ) {
      for (int d2 =0; d2 < MAX_PROJ; d2++ ) {
        _first_time_proj[d][d2] = true;
      }
    }
  }
  assert(TRIPROJECT);

  for (int d =0; d < projection_dims; d++ ) {
    for (int d2 =0; d2 < projection_dims; d2++ ) {
      solve_proj(d, d2, _first_time_proj[d][d2], 
                 cur_best_for_projection[d][d2],
                 projection_dims==1);
      _first_time_proj[d][d2] = false;
    }
  }
  first_time = false;
}

void Subproblem::solve_proj(int d2, int d3, 
                            bool first_proj_time, 
                            vector <ProjMax > & proj_best,  
                            bool is_simple) {


  // solve (but only in the projected space)
  // unless is_simple
  //cout << "Start solve" << endl;

  int num_word_nodes = graph->num_word_nodes;
  vector <float> best_bigram(num_word_nodes);
  vector<float> best_bigram_with_backoff(num_word_nodes);
  vector<float> best_backoff(num_word_nodes);
  vector<int> best_bigram_with_backoff_forward(num_word_nodes);
  
  clock_t begin; 
  if (TIMING) begin=clock();



  {  
    for (int i=0; i < graph->num_word_nodes; i++) {
      if (!graph->is_word(i)) continue;
      best_bigram[i] = INF;
      best_bigram_with_backoff[i] = INF;
      best_bigram_with_backoff_forward[i] = -1;
    }
    
    foreach (const WordBigram & b, gd->valid_bigrams()) { 
      int w1 = b.w1.id();
      int w2 = b.w2.id();
      for (int ord=0; ord < ORDER-1; ord++) {
        bigram_weight_cache[ord][w1][w2] = bi_rescore[ord]->get_bigram_weight(w1, w2);
      }
    
    
      if (TRIPROJECT && project_word(w2) != d3) {
        
      } else {
    
        float score = bigram_weight_cache[1][w1][w2];
      
    
        if (score < best_bigram[w1]) {
          best_bigram[w1] = score;
        }
      
      
        double backoff = backoff_score_cache[w1][w2];
        float score_with_backoff =  backoff + score;
        if (score_with_backoff < best_bigram_with_backoff[w1]) {
          best_bigram_with_backoff[w1] = score_with_backoff;
          best_backoff[w1] = backoff;
          assert(!TRIPROJECT || project_word(w2) == d3);
          best_bigram_with_backoff_forward[w1] = w2;
        }
      }
    }
  }


  assert(graph->num_word_nodes > 10);
  for ( int i =0; i< graph->num_word_nodes; i++ ) {
    if (!graph->is_word(i)) continue; 
    bool reset = false;
    if (!first_proj_time) {
      
      if (proj_best[i].ord_best[0] == -1) {
        reset = true;
      } else {
        int w1 = i;
        int one = proj_best[w1].ord_best[0];
        int two = proj_best[w1].ord_best[1];
        
        if (project_word(one) != d2 || project_word(two) != d3 ) {
          reset = true;
        } else {

          int w0 = fixed_last_bigram(w1);
        
          double old_score = proj_best[i].score;
          proj_best[i].score = 
            bigram_weight_cache[0][i][one] + 
            bigram_weight_cache[1][one][two] +
            (_lm_weight) *  word_prob_reverse(i, one, two);
          
          if (w0 != -1) {
            proj_best[i].score += 
              bigram_weight_cache[0][w0][i] + 
              bigram_weight_cache[1][i][one] +
              (_lm_weight) *  word_prob_reverse(w0, i, one);
          }


          proj_best[i].ord_best[0] = one;
          proj_best[i].ord_best[1] = two;
          proj_best[i].is_new = !(fabs(old_score -proj_best[i].score) < 1e-4);
          //if (!proj_best_is_new[i]) 
          //cout << "POSSIBLY " <<  i << " " << old_score << " " << proj_best_score[i] << endl;
          assert(proj_best[i].ord_best[0] != proj_best[i].ord_best[1]);
          assert(proj_best[i].score < 1000); 

        }
      }
    } else {
      reset = true;
    }
    if (reset) {
      proj_best[i].score = INF;
      for (int ord =0; ord < ORDER -1; ord++) {
        proj_best[i].ord_best[ord] = -1;
      }
      proj_best[i].is_new = true;
    }
  }

  clock_t end;
  if (TIMING) {
    end=clock();
    cout << "Precompute time: " 
         << double(Clock::diffclock(end,begin)) << " ms"<< endl;
    // actual algorithm
    begin=clock();
  }

  // counters
  int zeros =0;
  assert(gd->valid_bigrams().size() > 0);
  int lookups = 0;
  
  // words that are bounded by a later word
  vector <int> word_override;

  for (int w1 = 0; w1 < graph->num_word_nodes; w1++) {
    
    if (!graph->is_word(w1)) continue;
    overridden[w1] = false;  

    //cout << "TRYING " << w1 << endl; 
    // Edge tightness optimization
  
    bool on_edge = false;

    // w0 is the only thing preceding w1
    int w0 = fixed_last_bigram(w1);

    if (w0 != -1) {
      on_edge = true;
      word_override.push_back(w0);
    }

    assert (w0 == -1 || on_edge ==true);
 
    //bool is_redo = first_time || full_redo[w1];
    
    vector <int> f1 = gd->forward_bigrams[w1];
   
    foreach(uint w2, f1) { 
   
      if (project_word(w2) != d2 ) continue;
            
      float score1 = bigram_weight_cache[0][w1][w2];

      if (on_edge) {
        double internal = bigram_weight_cache[0][w0][w1] + 
                          bigram_weight_cache[1][w1][w2] + 
                          (_lm_weight) * word_prob_reverse(w0,w1,w2);        
        score1 += internal;
      }
 
      // check NaN
      assert (score1 == score1);

      const vector <int> * f2;

      if (gd->forward_bigrams[w2].size() == 0 || 
          best_bigram_with_backoff_forward[w2] == -1) continue;
      if (!OPTIMIZE) {
        f2 = &gd->forward_bigrams[w2];
      } else {
        // only consider words with full lm context
        f2 = forward_trigrams[w1][w2];
      }


      float estimate  = best_lm_score[w1][w2] + best_bigram[w2] + score1;

      if ( OPTIMIZE && is_simple && ( estimate > proj_best[w1].score)) {
        //cout << "killed" << endl;
        //continue;
      }

      
      if (OPTIMIZE) {
        float bi_lm_score = bigram_score_cache[w1][w2];
        int w3 = best_bigram_with_backoff_forward[w2];
        float score2 = bigram_weight_cache[1][w2][w3];
        float score  = bi_lm_score + best_backoff[w2] + score1 + score2;

        if (word_bow_reverse(w1,w2,w3) != 2) {

        } else {
          score = (_lm_weight) * word_prob_reverse(w1,w2,w3) + score1 + score2;
        }
        
        assert (proj_best[w1].score <= INF);
        if  (fabs(((_lm_weight) * word_prob_reverse(w1,w2,w3) + score1 + score2) - score) > 1e-4) {
          cout << "Optimization fail"<<endl; 
          exit(0);
        }
        assert(!TRIPROJECT || project_word(w3) == d3 ); 

        //cout << "\t QUICK SETTING " << w1 << " " << w2 << " " << w3 << " " << score<< endl; 
        try_set_max(proj_best, w1, w2, w3, score, true);       
      }      

      for (unsigned int j =0; j < f2->size(); j++) {
        
        int w3 = (*f2)[j];
        if (project_word(w3) != d3 ) continue;
        
        float score2 =0.0;
        //assert (is_redo || (bi_rescore_first->move_direction[w1][w2] == -1) || (bi_rescore_two->move_direction[w2][w3] == -1));
        double lm_score;

        //int l = word_bow_reverse(w1, w2, w3);
        //cout << lm_score << " " << bi_lm_score<<endl;
        //assert (lm_score <= bi_lm_score_test);
        
        if (OPTIMIZE) {
          lookups++;
          lm_score = (*forward_trigrams_score[w1][w2])[j];
          score2= bigram_weight_cache[1][w2][w3];
        }
        
        float score = score1 + score2 + lm_score; 

        //cout << "\t SETTING " << w1 << " " << w2 << " " << w3 << " " << score<< endl; 
        try_set_max(proj_best, w1, w2, w3, score, true);
        
      }
    }
  }

  foreach (int w0, word_override) { 
    
    assert(gd->forward_bigrams[w0].size() ==1);
    int w1 = gd->forward_bigrams[w0][0];
    
    overridden[w0] = true;

    if (proj_best[w1].ord_best[0] ==-1 ) continue; 
    
    // property w0 trigram must equal w1 bigram
    int w2 = proj_best[w1].ord_best[0];
    int w3 = proj_best[w1].ord_best[1];
    assert(graph->is_word(w1) && graph->is_word(w2));

    // the lm score at w1 needs to include the previous trigram score
    double first  = (_lm_weight) * word_prob_reverse(w0,w1,w2) + 
                    bigram_weight_cache[0][w0][w1] + 
                    bigram_weight_cache[1][w1][w2];
    double second = (_lm_weight) * word_prob_reverse(w1,w2,w3) + 
                    bigram_weight_cache[0][w1][w2] + 
                    bigram_weight_cache[1][w2][w3];
 
    //cout << w0 << " " << w1 << " " << " " << w2<< " " << w3 << " " << first << " " << second << " " << proj_best[w1].score << endl;
    assert(fabs(first + second - proj_best[w1].score) < 1e-4);
    
    proj_best[w0].score = 0.0;  
    proj_best[w0].ord_best[0] = w1;
    proj_best[w0].ord_best[1] = w2;
    proj_best[w0].is_new = false;   
  }

  
  if (TIMING) {
    clock_t end=clock();
    cout << "TRIGRAM TIME: " << double(Clock::diffclock(end,begin)) << " ms"<< endl;
    cout << "Lookups: " << lookups << endl;
    cout << "Zeroes: " << zeros << endl;
  }

}


void Subproblem::project(int proj_dim, vector <int> proj ) {
  assert (PROJECT || TRIPROJECT);
  assert (proj_dim >=1 && proj_dim <= MAX_PROJ);
  //map<int,int>::const_iterator iter;
  projection = proj;

  projection_dims = proj_dim;
  
}

float Subproblem::primal_score(int word[], int l) {
  float total = 0.0;
  for (int i=0; i < l-2; i++) {
    total += word_prob(word[i],word[i+1],word[i+2]);
  }
  return total;
}

double Subproblem::word_prob(int i, int j, int k) {
  VocabIndex context [] = {_word_node_cache.store[j], _word_node_cache.store[i], Vocab_None};
  return lm->wordProb(_word_node_cache.store[k], context);
}

double Subproblem::word_backoff(int i) {
  VocabIndex context [] = {_word_node_cache.store[i], Vocab_None};
  
  double score = lm->contextBOW(context, 0);
  //cout << "BO" << " " << score << " "
  //   << lm->vocab.getWord(_word_node_cache.store[i]) << endl;
  return score;
}

double Subproblem::word_backoff_two(int i, int j) {
  VocabIndex context [] = {_word_node_cache.store[i], _word_node_cache.store[j], Vocab_None};  
  double score = lm->contextBOW(context, 1);
  return score;
}

double Subproblem::word_prob_reverse(int i, int j, int k) {
  VocabIndex context [] = {_word_node_cache.store[j], _word_node_cache.store[k], Vocab_None};
  return lm->wordProb(_word_node_cache.store[i], context);
}

double Subproblem::word_prob_bigram_reverse(int i, int j) {
  VocabIndex context [] = {_word_node_cache.store[j], lm->vocab.getIndex(Vocab_Unknown), Vocab_None};
  return lm->wordProb(_word_node_cache.store[i], context);
}


int Subproblem::word_bow_bigram_reverse(int i, int j) {
  VocabIndex context [] = {_word_node_cache.store[j], Vocab_None};
  unsigned int length;
  lm->contextID(_word_node_cache.store[i], context, length);
  return length;
}

int Subproblem::word_bow_reverse(int i, int j, int k) {
  VocabIndex context [] = {_word_node_cache.store[j], _word_node_cache.store[k], Vocab_None};
  unsigned int length;
  lm->contextID(_word_node_cache.store[i], context, length);
  return length;
}



