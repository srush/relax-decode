#include <time.h>
#include "GraphDecompose.h"
#include "dual_subproblem.h"
#include "util.h"
#include "EdgeCache.h"
#define INF 1000000
#define DEBUG 0
#define TIMING 0

#include <bitset>

using namespace std;


Subproblem::Subproblem(const ForestLattice * g, LM *lm_in, const GraphDecompose * gd_in, const Cache<LatNode, int> & word_node_cache_in) :
  graph(g), lm(lm_in), gd(gd_in), _word_node_cache(word_node_cache_in){
  //update_filter.reset();
    
  bi_rescore_first = new BigramRescore(graph, gd_in);
  bi_rescore_two = new BigramRescore(graph, gd_in);
  //bigram_cache.resize(graph->num_nodes);
  //for (unsigned int i=0; i < graph->num_nodes; i++) {
  //bigram_cache[i].resize(NUMSTATES);
  //}
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
  if (first) { 
    return bi_rescore_first->bigram_path[w1][w2];
  } else {
    return bi_rescore_two->bigram_path[w1][w2];
  }
}

float Subproblem::get_best_bigram_weight(int w1, int w2, bool first) {
  if (first) {
    return bi_rescore_first->bigram_weights[w1][w2];
  } else {
    return bi_rescore_two->bigram_weights[w1][w2];
  }
}

void Subproblem::solve() {

  // faster if we visit in order
  bitset <NUMSTATES> full_redo;
  vector <int> redos_one[NUMSTATES];
  vector <int> redos_two[NUMSTATES];
  full_redo.reset();

  assert(graph->num_nodes > 10);
  for (unsigned int i =0; i< graph->num_nodes; i++ ) {

    if (graph->word_node[i] == -1) continue; 

    last_best_score[i] = cur_best_score[i];
    last_best_one[i] = cur_best_one[i];
    last_best_two[i] = cur_best_two[i];
    
    if (!first_time) {
      //assert (cur_best_one[i] != -1);
      //assert (cur_best_two[i] != -1);
      if (cur_best_one[i] != -1) {
        cur_best_score[i] = bi_rescore_first->bigram_weights[i][cur_best_one[i]] + 
          bi_rescore_two->bigram_weights[cur_best_one[i]][cur_best_two[i]] +
          (-0.141221) *  word_prob_reverse(graph->word_node[i], graph->word_node[cur_best_one[i]], graph->word_node[cur_best_two[i]]);
        
        cur_best_one[i] = cur_best_one[i];
        cur_best_two[i] = cur_best_two[i];
      }
    } else {
      cur_best_score[i] = INF;
      cur_best_one[i] = -1;
      cur_best_two[i] = -1;
    }
  }

  float best_bigram[NUMSTATES];
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
    }
    
    for (int i=0; i < gd->valid_bigrams.size(); i++) {
      Bigram b = gd->valid_bigrams[i];
      float score = bi_rescore_two->bigram_weights[b.w1][b.w2];

      if (score < best_bigram[b.w1]) {
        best_bigram[b.w1] = score;
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
      
      int word1 = graph->word_node[w1];
      int word2 = graph->word_node[w2];
      
      float score1 = bi_rescore_first->bigram_weights[w1][w2]; 
      
      vector <int> f2;
      //if (is_redo || (bi_rescore_first->move_direction[w1][w2] == -1) ) {
      f2 = gd->forward_bigrams[w2];
        //} else {
        //f2 = redos_two[w2];
        //}  

      if (first_time) {
        best_lm_score[w1][w2] = INF;
      }
      float estimate  = best_lm_score[w1][w2] + best_bigram[w2] + score1;
      if (!first_time && ( estimate> cur_best_score[w1])) {
        continue;
      }
      //if (!first_time) {
        //cout << "EST :"<<best_lm_score[w1][w2] << " " << best_bigram[w2]<< " " <<  estimate << " " << cur_best_score[w1]<< endl;
        //}
      for (unsigned int j =0; j < f2.size(); j++) {
        int w3 = f2[j];
        //bool _redo = (bi_rescore_first->move_direction[w1][w2] == -1);

        int word3 = graph->word_node[w3];
        float score2 = bi_rescore_two->bigram_weights[w2][w3];

        //assert (is_redo || (bi_rescore_first->move_direction[w1][w2] == -1) || (bi_rescore_two->move_direction[w2][w3] == -1));
        float lm_score = (-0.141221) *  word_prob_reverse(word1, word2, word3);
        
        if (first_time) {
          if (lm_score < best_lm_score[w1][w2]) {
            best_lm_score[w1][w2] = lm_score;
          }
        }
        
        lookups ++;
        //     float score = score1 + score2 + lm->all_score[word1][word2][word3];
        
        //     if (score > cur_best_score[b.w1]) {
        //       cur_best_score[b.w1] = score;
        //         cur_best[b.w1] = Bigram(b.w2, w3);
        //     }

        float score = score1 + score2 + lm_score; //(-1.11652878876722) * lm_score;  
      
      
        if (cur_best_score[w1] == INF || score < cur_best_score[w1]) {
          cur_best_score[w1] = score;
          cur_best_one[w1] = w2;
          cur_best_two[w1] = w3;
          //cur_best_path_bigram[b.w1] = path;
        }
      }
      //cout << w1 << endl; 
      //assert(cur_best_one[w1] != -1);
      //assert(cur_best_two[w1] != -1);
    }
  }

  first_time = false;
  
  if (TIMING) {
    clock_t end=clock();
    cout << "TRIGRAM TIME: " << double(diffclock(end,begin)) << " ms"<< endl;
    cout << "Lookups: " << lookups << endl;
  }

  // for (unsigned int i =0; i< graph->num_nodes; i++ ) {
  //   if (graph->word_node[i]!= -1 && graph->final[i]!= 1) {
  //     assert(cur_best_score[i] != INF);
  //     assert(cur_best_one[i] != -1);
  //     assert(cur_best_two[i] != -1);
  //     assert(graph->word_node[cur_best_one[i]] != -1);
  //     assert(graph->word_node[cur_best_two[i]] != -1);
  //   }
  // }
}


float Subproblem::primal_score(int word[NUMWORDS], int l) {
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

double Subproblem::word_prob_reverse(int i, int j, int k) {
  VocabIndex context [] = {_word_node_cache.store[j], _word_node_cache.store[k], Vocab_None};
  return lm->wordProb(_word_node_cache.store[i], context);
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
