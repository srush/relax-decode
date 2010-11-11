#include <time.h>
#include "GraphDecompose.h"
#include "dual_subproblem.h"
#include "util.h"
#include "EdgeCache.h"
#define INF 100000000
#define DEBUG 0
#define TIMING 1
#define OPTIMIZE 1
#include <bitset>

using namespace std;


Subproblem::Subproblem(const ForestLattice * g, Ngram *lm_in, const GraphDecompose * gd_in, const Cache<LatNode, int> & word_node_cache_in) :
  graph(g), lm(lm_in), gd(gd_in), _word_node_cache(word_node_cache_in){
  //update_filter.reset();
    
  bi_rescore_first = new BigramRescore(graph, gd_in);
  bi_rescore_two = new BigramRescore(graph, gd_in);
  //bigram_cache.resize(graph->num_nodes);
  //for (unsigned int i=0; i < graph->num_nodes; i++) {
  //bigram_cache[i].resize(NUMSTATES);
  //}
  
  int num_nodes = g->num_nodes;
  cur_best_one.resize(num_nodes);
  cur_best_two.resize(num_nodes);
  cur_best_score.resize(num_nodes);
  cur_best_count.resize(num_nodes);
  best_lm_score.resize(num_nodes);
  bigram_score_cache.resize(num_nodes);

  forward_trigrams.resize(num_nodes);
  forward_trigrams_score.resize(num_nodes);

  for (int i=0; i < num_nodes; i ++) {
    best_lm_score[i].resize(num_nodes);
    bigram_score_cache[i].resize(num_nodes);
    forward_trigrams[i].resize(num_nodes);
    forward_trigrams_score[i].resize(num_nodes);
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
  vector <int> path, ret; 
  if (first) { 
    path = bi_rescore_first->get_bigram_path(w1, w2);
  } else {
    path = bi_rescore_two->get_bigram_path(w1, w2);
  }
  for (int i =0; i< path.size(); i++) {
    if (!graph->ignore_nodes[path[i]]) {
      ret.push_back(path[i]);
    }
  } 
  return ret;
}

float Subproblem::get_best_bigram_weight(int w1, int w2, bool first) {
  if (first) {
    return bi_rescore_first->get_bigram_weight(w1,w2);
  } else {
    return bi_rescore_two->get_bigram_weight(w1,w2);
  }
}

void Subproblem::solve() {

  // faster if we visit in order
  //bitset <NUMSTATES> full_redo;
  //vector <int> redos_one[NUMSTATES];
  //vector <int> redos_two[NUMSTATES];
  //full_redo.reset();
  //cout << graph->num_nodes << endl;
  assert(graph->num_nodes > 10);
  for (unsigned int i =0; i< graph->num_nodes; i++ ) {
    if (graph->word_node[i] == -1) continue; 

    
    if (!first_time) {
      //assert (cur_best_one[i] != -1);
      //assert (cur_best_two[i] != -1);
      if (!cur_best_one[i].empty()) {
        int one = cur_best_one[i][0];
        int two = cur_best_two[i][0];
        cur_best_score[i] = bi_rescore_first->get_bigram_weight(i,one) + 
          bi_rescore_two->get_bigram_weight(one,two) +
          (-0.141221) *  word_prob_reverse(i, one, two);
        
        
        cur_best_one[i].clear();
        cur_best_one[i].push_back(one);
        cur_best_two[i].clear();
        cur_best_two[i].push_back(two);
        cur_best_count[i] = 1;
      }
    } else {
      cur_best_score[i] = INF;
      cur_best_one[i].clear();
      cur_best_two[i].clear();
      cur_best_count[i] = 0;
    }
  }
  int num_nodes = graph->num_nodes;
  vector <float> best_bigram(num_nodes);
  vector<float> best_bigram_with_backoff(num_nodes);
  vector<float> best_backoff(num_nodes);
  vector<int> best_bigram_with_backoff_forward(num_nodes);
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
    

    for (int i=0; i < graph->num_nodes; i++) {
      best_bigram[i] = INF;
      best_bigram_with_backoff[i] = INF;
    }
    
    for (int i=0; i < gd->valid_bigrams.size(); i++) {
      Bigram b = gd->valid_bigrams[i];
      float score = bi_rescore_two->get_bigram_weight(b.w1,b.w2);

      if (score < best_bigram[b.w1]) {
        best_bigram[b.w1] = score;
       }
      
      double backoff = word_backoff_two(b.w1, b.w2);
      float score_with_backoff = (-0.141221) * backoff + score;
      if (score_with_backoff < best_bigram_with_backoff[b.w1]) {
        best_bigram_with_backoff[b.w1] = score_with_backoff;
        best_backoff[b.w1] = (-0.141221) * backoff;
        best_bigram_with_backoff_forward[b.w1] = b.w2;
      }
    }

  }


  // actual algorithm
  clock_t begin=clock();
  assert(gd->valid_bigrams.size() > 0);
  int lookups = 0;
  for (int w1=0; w1 < graph->num_nodes; w1++) {
    if (graph->word_node[w1] == -1) continue;
    
    //bool is_redo = first_time || full_redo[w1];
    
    vector <int> f1 = gd->forward_bigrams[w1];
    //cout << f1.size() << endl;
    for (unsigned int i =0; i< f1.size(); i++ ) {
      
      int w2 = f1[i];
      
      
      //cout << "SEC" << " " << w1 << " " << w2 << endl;
      
      float score1 = bi_rescore_first->get_bigram_weight(w1,w2); 
      
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

      if (first_time) {
        best_lm_score[w1][w2] = INF;
        forward_trigrams[w1][w2] = new vector<int>();
        forward_trigrams_score[w1][w2] = new vector<double>();
        bigram_score_cache[w1][w2] = (-0.141221) *  word_prob_bigram_reverse(w1, w2);
      }

      float estimate  = best_lm_score[w1][w2] + best_bigram[w2] + score1;
      if (OPTIMIZE && !first_time && ( estimate > cur_best_score[w1])) {
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
          assert (fabs(((-0.141221) * word_prob_reverse(w1,w2,w3)) - (bi_lm_score + best_backoff[w2])) < 1e-4); 
        } else {
          score = (-0.141221) * word_prob_reverse(w1,w2,w3) + score1 + score2;
        }

        //cout << (-0.141221) * word_prob_reverse(w1,w2,w3) + score1 + score2 << " " << bi_lm_score + best_bigram_with_backoff[w2] + score1 << endl;
        if (cur_best_score[w1] == INF || score < cur_best_score[w1]) {
          cur_best_score[w1] = score;
          cur_best_one[w1].clear();
          cur_best_one[w1].push_back(w2);
          cur_best_two[w1].clear();
          cur_best_two[w1].push_back(best_bigram_with_backoff_forward[w2]);
          cur_best_count[w1] =1;
        }
      }
      
      

      for (unsigned int j =0; j < f2->size(); j++) {
        int w3 = (*f2)[j];
        //bool _redo = (bi_rescore_first->move_direction[w1][w2] == -1);

        float score2 = bi_rescore_two->get_bigram_weight(w2,w3);

        //assert (is_redo || (bi_rescore_first->move_direction[w1][w2] == -1) || (bi_rescore_two->move_direction[w2][w3] == -1));
        double lm_score;

        //int l = word_bow_reverse(w1, w2, w3);
        //cout << lm_score << " " << bi_lm_score<<endl;
        //assert (lm_score <= bi_lm_score_test);
        if (first_time || !OPTIMIZE) {
          lm_score = (-0.141221) *  word_prob_reverse(w1, w2, w3);
        }

        if (first_time) {
          if (lm_score < best_lm_score[w1][w2]) {
            best_lm_score[w1][w2] = lm_score;
          }
   
          if (word_bow_reverse(w1,w2,w3) == 2) {
            forward_trigrams[w1][w2]->push_back(w3);
            forward_trigrams_score[w1][w2]->push_back(lm_score);
          }
        } else if (OPTIMIZE) {
          lm_score = (*forward_trigrams_score[w1][w2])[j];
        }
        
        lookups++;

        //     float score = score1 + score2 + lm->all_score[word1][word2][word3];
        
        //     if (score > cur_best_score[b.w1]) {
        //       cur_best_score[b.w1] = score;
        //         cur_best[b.w1] = Bigram(b.w2, w3);
        //     }

        float score = score1 + score2 + lm_score; //(-1.11652878876722) * lm_score;  
        //cout << score1 << " " << score2 << " " << lm_score << endl; 
        //cout << w1 << " " << w2 << " " << w3 << endl;
        if (cur_best_score[w1] == INF || score < cur_best_score[w1]) {
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
      }
    }
  }
  first_time = false;
  
  if (TIMING) {
    clock_t end=clock();
    cout << "TRIGRAM TIME: " << double(diffclock(end,begin)) << " ms"<< endl;
    cout << "Lookups: " << lookups << endl;
  }

  //  for (unsigned int i =0; i< graph->num_nodes; i++ ) {
  //if (graph->word_node[i]!= -1 && graph->final[i]!= 1) {
  //   assert(cur_best_score[i] != INF);
  //}
  // }
       //     assert(cur_best_one[i] != -1);
  //     assert(cur_best_two[i] != -1);
  //     assert(graph->word_node[cur_best_one[i]] != -1);
  //     assert(graph->word_node[cur_best_two[i]] != -1);
  //   }
  // }
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
