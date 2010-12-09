#include <time.h>
#include "GraphDecompose.h"
#include "dual_subproblem.h"
#include "util.h"
#include "EdgeCache.h"
#include <cstdlib>
#define INF 100000000
#define DEBUG 0
#define TIMING 0
#define OPTIMIZE 1


#include <bitset>

using namespace std;


Subproblem::Subproblem(const ForestLattice * g, NgramCache *lm_in, const SkipTrigram & skip, const GraphDecompose * gd_in, const Cache<LatNode, int> & word_node_cache_in) :
  graph(g), lm(lm_in), gd(gd_in), skip_tri(skip), _word_node_cache(word_node_cache_in){
  //update_filter.reset();
   
  bi_rescore_first = new BigramRescore(graph, gd_in);
  bi_rescore_two = new BigramRescore(graph, gd_in);

  //bigram_cache.resize(graph->num_nodes);
  //for (unsigned int i=0; i < graph->num_nodes; i++) {
  //bigram_cache[i].resize(NUMSTATES);
  //}
  
  //int num_nodes = g->num_word_nodes;
  int num_word_nodes = g->num_word_nodes;
  cur_best_one.resize(num_word_nodes);
  cur_best_two.resize(num_word_nodes);
  cur_best_score.resize(num_word_nodes);
  cur_best_count.resize(num_word_nodes);
  overridden.resize(num_word_nodes);

  if (PROJECT) {
    cur_best_at_bi.resize(num_word_nodes);
    cur_best_at_bi_score.resize(num_word_nodes);
    
    cur_best_bi_projection.resize(num_word_nodes);
    cur_best_bi_projection_first.resize(num_word_nodes);
    cur_best_bi_projection_score.resize(num_word_nodes);
    
  }

  best_lm_score.resize(num_word_nodes);
  bigram_score_cache.resize(num_word_nodes);
  backoff_score_cache.resize(num_word_nodes);
  bigram_in_lm.resize(num_word_nodes);
  bigram_weight_cache_one.resize(num_word_nodes);
  bigram_weight_cache_two.resize(num_word_nodes);

  forward_trigrams.resize(num_word_nodes);
  forward_trigrams_score.resize(num_word_nodes);

  for (int i=0; i < num_word_nodes; i ++) {
    best_lm_score[i].resize(num_word_nodes);
    bigram_score_cache[i].resize(num_word_nodes);
    backoff_score_cache[i].resize(num_word_nodes);
    bigram_in_lm[i].resize(num_word_nodes);
    forward_trigrams[i].resize(num_word_nodes);
    forward_trigrams_score[i].resize(num_word_nodes);

    bigram_weight_cache_one[i].resize(num_word_nodes);
    bigram_weight_cache_two[i].resize(num_word_nodes);
  
    if (PROJECT) {
      cur_best_at_bi[i].resize(num_word_nodes);
      cur_best_at_bi_score[i].resize(num_word_nodes);

      cur_best_bi_projection[i].resize(num_word_nodes);
      cur_best_bi_projection_first[i].resize(num_word_nodes);
      cur_best_bi_projection_score[i].resize(num_word_nodes);

    }
  }

  for (unsigned int i=0; i< gd->valid_bigrams.size() ;i++) {
    Bigram b = gd->valid_bigrams[i];
    //assert(gd->bigram_pairs[b.w1][b.w2].size() > 0);
    //bigram_weights[b.w1][b.w2].resize(gd->bigram_pairs[b.w1][b.w2].size());
    //for (unsigned int path=0; path < gd->bigram_pairs[b.w1][b.w2].size(); path++) {
    //bigram_weights[b.w1][b.w2][path] = 0; 
    //}
    //cache_paths(b.w1, b.w2);
  }
  
  bi_rescore_first->recompute_bigram_weights(true);
  bi_rescore_two->recompute_bigram_weights(true);

  first_time = true;
}


//void Subproblem::setup_problems() {
  //gd->decompose(graph);   
//}


void Subproblem::update_weights(vector <int> u_pos, vector <float> u_values, bool first) {
  assert (u_pos.size() == u_values.size()); 
  if (first) {
    bi_rescore_first->update_weights(u_pos, u_values, u_pos.size());
  } else {
    bi_rescore_two->update_weights(u_pos, u_values, u_pos.size());
  }
}

vector <int> Subproblem::get_best_nodes_between(int w1, int w2, bool first) {
  //assert(path < gd->bigram_pairs[w1][w2].size());
  vector <int> path;
  if (first) { 
    path = bi_rescore_first->get_bigram_path(w1, w2);
  } else {
    path = bi_rescore_two->get_bigram_path(w1, w2);
  }
  //for (int i =0; i< path.size(); i++) {
  //if (!graph->ignore_nodes[path[i]]) {
  //  ret.push_back(path[i]);
  //}
  //} 
  path.push_back(w2);
  return path;
}

float Subproblem::get_best_bigram_weight(int w1, int w2, bool first) {
  if (first) {
    return bi_rescore_first->get_bigram_weight(w1,w2);
  } else {
    return bi_rescore_two->get_bigram_weight(w1,w2);
  }
}

int Subproblem::fixed_last_bigram(int w1) {
  //return -1;
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


void Subproblem::solve(int d2, int d3) {
  // solve (but only in the projected space)

  // faster if we visit in order
  //bitset <NUMSTATES> full_redo;
  //vector <int> redos_one[NUMSTATES];
  //vector <int> redos_two[NUMSTATES];
  //full_redo.reset();
  //cout << graph->num_nodes << endl;
  assert(graph->num_word_nodes > 10);
  for (unsigned int i =0; i< graph->num_word_nodes; i++ ) {
    if (!graph->is_word(i)) continue; 
    
    if (!first_time) {
      //assert (cur_best_one[i] != -1);
      //assert (cur_best_two[i] != -1);
      if (!cur_best_one[i].empty()) {
        int w1 = i;
        int one = cur_best_one[i][0];
        int two = cur_best_two[i][0];
        
        int w0 = fixed_last_bigram(w1);
        

        cur_best_score[i] = bi_rescore_first->get_bigram_weight(i,one) + 
          bi_rescore_two->get_bigram_weight(one,two) +
          (-0.141221) *  word_prob_reverse(i, one, two);
        
        if (w0 != -1) {
          cur_best_score[i] += bi_rescore_two->get_bigram_weight(w1, one) +
            (-0.141221) *  word_prob_reverse(w0, i, one);
        }

        cur_best_one[i].clear();
        cur_best_one[i].push_back(one);
        cur_best_two[i].clear();
        cur_best_two[i].push_back(two);
        cur_best_count[i] = 1;

        if (PROJECT) {
          for (unsigned int j=0; j< graph->num_word_nodes; j++ ) {
            if (!graph->is_word(j)) continue; 
            cur_best_at_bi_score[i][j] = INF;
            cur_best_at_bi[i][j] = -1;            
          }
        }

      }
    } else {
      cur_best_score[i] = INF;
      cur_best_one[i].clear();
      cur_best_two[i].clear();
      cur_best_count[i] = 0;
      if (PROJECT) {
        for (unsigned int j=0; j< graph->num_word_nodes; j++ ) {
          if (!graph->is_word(j)) continue; 
          cur_best_at_bi_score[i][j] = INF;
          cur_best_at_bi[i][j] = -1;
          
        }
      }
    }
  }


  if (first_time) {
    for (int i=0; i < gd->valid_bigrams.size(); i++) {
      Bigram b = gd->valid_bigrams[i];
      int w1 = b.w1;
      int w2 = b.w2;
      bigram_in_lm[b.w1][b.w2] =  word_bow_bigram_reverse(b.w1, b.w2);

      best_lm_score[w1][w2] = INF;
      forward_trigrams[w1][w2] = new vector<int>();
      forward_trigrams_score[w1][w2] = new vector<double>();
      bigram_score_cache[w1][w2] = (-0.141221) *  word_prob_bigram_reverse(w1, w2);
      backoff_score_cache[w1][w2] = (-0.141221) *  word_backoff_two(w1, w2);
    }
  }



  int num_word_nodes = graph->num_word_nodes;
  vector <float> best_bigram(num_word_nodes);
  vector<float> best_bigram_with_backoff(num_word_nodes);
  vector<float> best_backoff(num_word_nodes);
  vector<int> best_bigram_with_backoff_forward(num_word_nodes);
  
  clock_t begin; 
  if (TIMING) 
    begin=clock();


  if (!first_time) {
    int total =0;
    int redo = 0;

    /*for (unsigned int i =0; i< graph->num_nodes; i++ ) {
      if (graph->word_node[i] != -1) { 
        
        // if the best score moved up, need a full_redo 
        full_redo[i] = (bi_rescore_first->move_direction[i][last_best_one[i]]==1) ||
          (bi_rescore_two->move_direction[last_best_one[i]][last_best_two[i]]==1);
        total++;
        if (full_redo[i]) redo++;
      }
     
    }
    cout << "Redoing "<< redo / (float)total<< endl;
    // forward list of nodes to redo
    for (int i=0; i < gd->valid_bigrams.size(); i++) {
      Bigram b = gd->valid_bigrams[i];
      if (bi_rescore_first->move_direction[b.w1][b.w2] == -1) {
        redos_one[b.w1].push_back(b.w2);
      }
      if (bi_rescore_two->move_direction[b.w1][b.w2] == -1) {
        redos_two[b.w1].push_back(b.w2);
      }
      
      }*/
    for (int i=0; i < graph->num_word_nodes; i++) {
      if (!graph->is_word(i)) continue;
      best_bigram[i] = INF;
      best_bigram_with_backoff[i] = INF;
    }
    
    for (int i=0; i < gd->valid_bigrams.size(); i++) {
      Bigram b = gd->valid_bigrams[i];
    
      bigram_weight_cache_one[b.w1][b.w2] = bi_rescore_first->get_bigram_weight(b.w1, b.w2);
      bigram_weight_cache_two[b.w1][b.w2] = bi_rescore_two->get_bigram_weight(b.w1, b.w2);
    
      float score = bigram_weight_cache_two[b.w1][b.w2];
      if (score < best_bigram[b.w1]) {
        best_bigram[b.w1] = score;
      }
      
      double backoff = backoff_score_cache[b.w1][b.w2];
      float score_with_backoff =  backoff + score;
      if (score_with_backoff < best_bigram_with_backoff[b.w1]) {
        best_bigram_with_backoff[b.w1] = score_with_backoff;
        best_backoff[b.w1] = backoff;
        best_bigram_with_backoff_forward[b.w1] = b.w2;
      }
    }
  }
  clock_t end;
  if (TIMING) {

    end=clock();
    cout << "Precompute time: " << double(diffclock(end,begin)) << " ms"<< endl;
  // actual algorithm
    begin=clock();
  }

  // counters
  int zeros =0;
  assert(gd->valid_bigrams.size() > 0);
  int lookups = 0;

  
  // words that are bounded by a later word
  vector <int> word_override;

  for (int w1=0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    overridden[w1] = false;

    int same = graph->get_same(w1);
    
    // Repeat speed optimization - kill this
    // if (same != -1) {
//       assert(same < w1);
//       //cout << same << " " << w1 << endl;
//       cur_best_score[w1] = cur_best_score[same];
//       cur_best_one[w1] = cur_best_one[same];
//       cur_best_two[w1] = cur_best_two[same];
//       cur_best_count[w1] = cur_best_count[same];
//       continue;
//     }
  

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
    //  assert (f1.size() > 0);
    //cout << f1.size() << endl;
    for (unsigned int i =0; i< f1.size(); i++ ) {
      
      int w2 = f1[i];

      //cout << "SEC" << " " << w1 << " " << w2 << endl;
      
      float score1 = bi_rescore_first->get_bigram_weight(w1,w2);

      if (on_edge) {
        score1 += bi_rescore_first->get_bigram_weight(w0,w1) + 
          bi_rescore_two->get_bigram_weight(w1,w2) + (-0.141221) * word_prob_reverse(w0,w1,w2);
        
      }
 
      // check NaN
      assert (score1 == score1);

      //cout << w1 << " " << w2 << " " << score1<< endl;
      const vector <int> * f2;
      //if (is_redo || (bi_rescore_first->move_direction[w1][w2] == -1) ) {
      if (!OPTIMIZE  || first_time) {
        f2 = &gd->forward_bigrams[w2];
      } else {
        // only consider words with full lm context
        f2 = forward_trigrams[w1][w2];
      }
        //} else {
        //f2 = redos_two[w2];
        //}  


      float estimate  = best_lm_score[w1][w2] + best_bigram[w2] + score1;

      if (!PROJECT && OPTIMIZE && !first_time && ( estimate > cur_best_score[w1])) {
        continue;
      }

      if (!first_time && estimate >= INF) {
        continue;
      }
      //if (!first_time) {
        //cout << "EST :"<<best_lm_score[w1][w2] << " " << best_bigram[w2]<< " " <<  estimate << " " << cur_best_score[w1]<< endl;
        //}

      // OPTIMIZATION: predict once for all words that don't have full trigram context

      //float bi_lm_score_test = (-0.141221) *  word_prob_bigram_reverse(w1, w2) + best_backoff[w2];
      //cout << "BI " << bi_lm_score << " " << bi_lm_score_test << endl;


      if (OPTIMIZE && !first_time) {
        float bi_lm_score = bigram_score_cache[w1][w2];
        int w3 = best_bigram_with_backoff_forward[w2];
        float score2 = bi_rescore_two->get_bigram_weight(w2,w3);
        float score  = bi_lm_score + best_backoff[w2] + score1 + score2;

        //cout << w2 << " " << (-0.141221) * word_prob_reverse(w1,w2,w3) << " " << (bi_lm_score + best_backoff[w2]) <<" " << bi_lm_score<<" "<<  best_backoff[w2] << endl;
        if (word_bow_reverse(w1,w2,w3) != 2) {
          double backoff_score = (bi_lm_score + best_backoff[w2]);
          double lm_score = ((-0.141221) * word_prob_reverse(w1,w2,w3));
          //assert (fabs(lm_score - backoff_score) < 1e-4); 
        } else {
          score = (-0.141221) * word_prob_reverse(w1,w2,w3) + score1 + score2;
        }


        //cout << (-0.141221) * word_prob_reverse(w1,w2,w3) + score1 + score2 << " " << bi_lm_score + best_bigram_with_backoff[w2] + score1 << endl;
        assert (score < 1000);
        assert (cur_best_score[w1] <= INF);

        if (cur_best_score[w1] == INF || score < cur_best_score[w1]) {
          cur_best_score[w1] = score;
          cur_best_one[w1].clear();
          cur_best_one[w1].push_back(w2);
          cur_best_two[w1].clear();
          cur_best_two[w1].push_back(w3);
          cur_best_count[w1] =1;
        } 
        if (PROJECT) {
          if (score < cur_best_at_bi_score[w1][w2] || cur_best_at_bi_score[w1][w2] == INF) {
            cur_best_at_bi_score[w1][w2] = score; 
            cur_best_at_bi[w1][w2] =w3;
            assert (cur_best_at_bi_score[w1][w2] >= cur_best_score[w1] - 1e-4);
          }
        }
        
      }
      
      //cout << "F2 " << w1 <<  " " << f2->size() << " "<< cur_best_score[w1] << endl;       


      if (first_time) {
        VocabIndex context [] = {_word_node_cache.store[w2], Vocab_None};        
        lm->wordProbPrimeCache(_word_node_cache.store[w1], context);
      }
      
      for (unsigned int j =0; j < f2->size(); j++) {
        
        int w3 = (*f2)[j];
        //bool _redo = (bi_rescore_first->move_direction[w1][w2] == -1);        
        float score2 =0.0;
        //assert (is_redo || (bi_rescore_first->move_direction[w1][w2] == -1) || (bi_rescore_two->move_direction[w2][w3] == -1));
        double lm_score;

        //int l = word_bow_reverse(w1, w2, w3);
        //cout << lm_score << " " << bi_lm_score<<endl;
        //assert (lm_score <= bi_lm_score_test);
        

        if (first_time) {   
          //cout << word_bow_reverse(w1,w2,w3) << endl; 
          if (bigram_in_lm[w1][w2] && bigram_in_lm[w2][w3] &&  lm->hasNext(_word_node_cache.store[w3])) {         
          //lm_score = (-0.141221) *  lm->wordProbFromCache(_word_node_cache.store[w1], context);
          //if (word_bow_reverse(w1,w2,w3) == 2) {
            VocabIndex context [] = {_word_node_cache.store[w2], _word_node_cache.store[w3], Vocab_None};
            lm_score = (-0.141221) *  lm->wordProbFromCache(_word_node_cache.store[w1], context);
          
            forward_trigrams[w1][w2]->push_back(w3);
            forward_trigrams_score[w1][w2]->push_back(lm_score);
            
          
          } else {
            //cout << graph->get_word(w1)<< _word_node_cache.store[w1] << " " << graph->get_word(w2) << _word_node_cache.store[w2] << " " << graph->get_word(w3) << _word_node_cache.store[w3] << endl;
            //cout <<bigram_in_lm[w1][w2]<< " " << bigram_in_lm[w2][w3] << " " << skip_tri.is_skip(_word_node_cache.store[w3], _word_node_cache.store[w1]) << endl;
            //assert(word_bow_reverse(w1,w2,w3) != 2);
            lm_score = backoff_score_cache[w2][w3] + bigram_score_cache[w1][w2];
            zeros++;
          }

          if (lm_score < best_lm_score[w1][w2]) {
            best_lm_score[w1][w2] = lm_score;
          }
          
        } else if (OPTIMIZE) {
          lookups++;
          lm_score = (*forward_trigrams_score[w1][w2])[j];
          score2= bigram_weight_cache_two[w2][w3];
        }
        


        //     float score = score1 + score2 + lm->all_score[word1][word2][word3];
        
        //     if (score > cur_best_score[b.w1]) {
        //       cur_best_score[b.w1] = score;
        //         cur_best[b.w1] = Bigram(b.w2, w3);
        //     }

        float score = score1 + score2 + lm_score; //(-1.11652878876722) * lm_score;  
        //cout << score1 << " " << score2 << " " << lm_score << endl; 
        //cout << w1 << " " << w2 << " " << w3 << endl;
        //cout << graph->lookup_word(w1) << " " <<graph->lookup_word(w2) << " " << graph->lookup_word(w3) << endl;
        //cout << graph->get_word(w1) << " " <<graph->get_word(w2) << " " << graph->get_word(w3) << endl;
        if (score < cur_best_score[w1] || cur_best_score[w1] == INF) {
          //cout << "blah:" <<  score1 << " " << score2 << " " << lm_score << endl;;
          cur_best_score[w1] = score;
          cur_best_one[w1].clear();
          cur_best_one[w1].push_back(w2);
          cur_best_two[w1].clear();
          cur_best_two[w1].push_back(w3);
          cur_best_count[w1] = 1;
          //cur_best_path_bigram[b.w1] = path;
        } else if (score == cur_best_score[w1]) {
          //cout << "TIE!!!" << endl;
          cur_best_count[w1]++;
          cur_best_one[w1].push_back(w2);
          cur_best_two[w1].push_back(w3);
        }
        
        if (PROJECT) {
          if (score < cur_best_at_bi_score[w1][w2] || cur_best_at_bi_score[w1][w2] == INF) {
            cur_best_at_bi_score[w1][w2] = score; 
            cur_best_at_bi[w1][w2] = w3;
            assert (cur_best_at_bi_score[w1][w2] >= cur_best_score[w1] - 1e-4);
          }
        }
      }
    }
  }

  for (int i=0; i < word_override.size(); i++ ) {
    
    int w0 = word_override[i];
    
    assert(gd->forward_bigrams[w0].size() ==1);
    int w1 = gd->forward_bigrams[w0][0];
    
    overridden[w0] = true;

    if (cur_best_one[w1].size() ==0 ) continue; 

    
    // property w0 trigram must equal w1 bigram
    int w2 = cur_best_one[w1][0];
    int w3 = cur_best_two[w1][0];
    assert(graph->is_word(w1) && graph->is_word(w2));
    //cur_best_score[w0] = (-0.141221) * word_prob_reverse(w0,w1,w2) + bigram_weight_cache_one[w0][w1] + bigram_weight_cache_two[w1][w2];

    double first  = (-0.141221) * word_prob_reverse(w0,w1,w2) + bigram_weight_cache_one[w0][w1] + bigram_weight_cache_two[w1][w2];
    double second = (-0.141221) * word_prob_reverse(w1,w2,w3) + bigram_weight_cache_one[w1][w2] + bigram_weight_cache_two[w2][w3];
 
    
    assert(fabs(first + second - cur_best_score[w1]) < 1e-4);
    
    
    cur_best_score[w0] = 0.0; //first;
    //cur_best_score[w1] = second;
    
    cur_best_one[w0].clear();
    //cur_best_one[w0].push_back(w1);
    cur_best_two[w0].clear();
    //cur_best_two[w0].push_back(w2);
    cur_best_count[w0] = 0;

    if (PROJECT) {
      cur_best_at_bi_score[w0][w1] = 0.0;
      cur_best_at_bi[w0][w1] = -1;
      /*
      // was Wrong!!
      //int w1 = gd->forward_bigrams[w0][0];
      for (int j =0; j < gd->forward_bigrams[w1].size(); j ++) {
        int nw2 = gd->forward_bigrams[w1][j];
        int nw3 = cur_best_at_bi[w1][nw2];
        double nsecond = (-0.141221) * word_prob_reverse(w1,w2,w3) + bigram_weight_cache_one[w1][w2] + bigram_weight_cache_two[w2][w3]; 
        cur_best_at_bi_score[w1][w2] = INF;
        }*/
    }
    //assert (cur_best_at_bi_score[w1][w2] >= cur_best_score[w1] - 1e-4);
    
  }


  for (int w1=0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    if (cur_best_one[w1].size() == 0) continue;
    int w2 = cur_best_one[w1][0];
    int w3 = cur_best_two[w1][0];
    //assert(cur_best_at_bi[w1][w2] == w3);
    assert(fabs(cur_best_at_bi_score[w1][w2] -cur_best_score[w1]) < 1e-4);
  }


  first_time = false;
  
  if (TIMING) {
    clock_t end=clock();
    cout << "TRIGRAM TIME: " << double(diffclock(end,begin)) << " ms"<< endl;
    cout << "Lookups: " << lookups << endl;
    cout << "Zeroes: " << zeros << endl;
  }


}


vector <int> Subproblem::rand_projection( int k) {
  vector<int> proj(graph->num_word_nodes);
  for (int w1=0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    proj[w1] = rand() % k;
    //cout << w1 << " " << proj[w1] << endl;
  }
  return proj;
}

void Subproblem::project(int proj_dim, vector <int> proj ) {
  assert (PROJECT);
  
  //map<int,int>::const_iterator iter;
  projection = proj;

  projection_dims = proj_dim;
  

  for (int w1=0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    if (gd->forward_bigrams[w1].size() == 0) continue;
    for (int d=0; d < projection_dims; d++) {
      cur_best_bi_projection[w1][d] = -1;
      cur_best_bi_projection_first[w1][d] = -1;
      cur_best_bi_projection_score[w1][d] = INF; 
    }

    bool has = false;
    for (int j =0; j < gd->forward_bigrams[w1].size(); j++) {
      int w2 = gd->forward_bigrams[w1][j];
      if (gd->forward_bigrams[w2].size() == 0) continue;

      has = true;
      // project
      int d = projection[w2];
      double score = cur_best_at_bi_score[w1][w2];
      assert (cur_best_at_bi_score[w1][w2] < INF);
      assert (cur_best_at_bi_score[w1][w2] >=  cur_best_score[w1] -1e-4);

      if (cur_best_bi_projection_score[w1][d] == INF || 
          score < cur_best_bi_projection_score[w1][d] ) {
        
        cur_best_bi_projection_score[w1][d] = score;
        cur_best_bi_projection[w1][d] = cur_best_at_bi[w1][w2];
        cur_best_bi_projection_first[w1][d] = w2;
      }      
    }

    bool preserve_best = false;
    if (has) {
      for (int d=0; d < projection_dims; d++) {
        if (fabs (cur_best_bi_projection_score[w1][d] - cur_best_score[w1]) < 1e-4)
          preserve_best = true;
      }
      assert(preserve_best);
    }
    //if (has) assert(cur_best_bi_projection_score[w1][0] != INF);
    
  }



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
  //cout << "BO" << " " << score << " "
  //   << lm->vocab.getWord(_word_node_cache.store[i]) << " "
  //   << lm->vocab.getWord(_word_node_cache.store[j]) << endl;
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
  //cout << length << " " << 
  //  lm->vocab.getWord(_word_node_cache.store[k]) << " " << 
  //  lm->vocab.getWord(_word_node_cache.store[j]) << " " << 
  //  lm->vocab.getWord(_word_node_cache.store[i]) << endl;
  return length;
}



/*
Subproblem * initialize_subproblem(const char* graph_file, const char* word_file, const char * lm_file ) {
  LMCache * lm = new LMCache(lm_file);
  WordHolder * wd = new WordHolder(word_file);
  ForestLattice * g = new Forest(graph_file);
  lm->cache_sentence_probs(*wd);
  GraphDecompose * gd = new GraphDecompose();
  gd->decompose(g);
  
  Subproblem * s = new Subproblem(g, lm, gd);
  return s;
}
*/
