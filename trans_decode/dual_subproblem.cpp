#include <map>
#include <string>
#include <set>
#include <vector>
#include <bitset>
#include <time.h>

#include "GraphDecompose.h"
#include "dual_subproblem.h"
#include "NGramCache.h"
#include "EdgeCache.h"
#include "../common.h"

#define DEBUG 0
#define TIMING 0


using namespace std;


Subproblem::Subproblem(const ForestLattice *g,
                       NgramCache *lm_in,
                       const GraphDecompose *gd_in,
                       const Cache<Graphnode, int> &word_node_cache_in) :
  graph(g),
  lm(lm_in),
  gd(gd_in),
  _word_node_cache(word_node_cache_in),
  bi_rescore(ORDER-1),
  bigram_weight_cache(ORDER-1),
  bigram_weight_best(ORDER-1),
  _lm_weight(lm_weight()) {
  _non_exact = false;
  for (int ord =0; ord < ORDER -1; ord++) {
    bi_rescore[ord] = new BigramRescore(graph, gd_in);
  }

  int num_word_nodes = g->num_word_nodes;

  overridden.resize(num_word_nodes);

  cur_best_for_projection.resize(MAX_PROJ);
  for (int d = 0; d < MAX_PROJ; d++) {
    cur_best_for_projection[d].resize(MAX_PROJ);
    for (int d2 = 0; d2 < MAX_PROJ; d2++) {
      cur_best_for_projection[d][d2].resize(num_word_nodes);
      for (int w = 0; w < num_word_nodes; w++) {
        cur_best_for_projection[d][d2][w].ord_best.resize(ORDER-1);
      }
    }
  }

  best_lm_score.resize(num_word_nodes);
  bigram_score_cache.resize(num_word_nodes);
  backoff_score_cache.resize(num_word_nodes);
  bigram_in_lm.resize(num_word_nodes);

  for (int ord = 0; ord < ORDER - 1; ord++) {
    bigram_weight_cache[ord].resize(num_word_nodes);
    bigram_weight_best[ord].resize(num_word_nodes, INF);
  }

  forward_trigrams.resize(num_word_nodes);
  // forward_trigrams_score.resize(num_word_nodes);
  forward_trigrams_score_best.resize(num_word_nodes);
  word_bow_reverse_cache.resize(num_word_nodes);


  for (int i = 0; i < num_word_nodes; i++) {
    best_lm_score[i].resize(num_word_nodes);
    bigram_score_cache[i].resize(num_word_nodes);
    backoff_score_cache[i].resize(num_word_nodes);
    bigram_in_lm[i].resize(num_word_nodes);
    forward_trigrams[i].resize(num_word_nodes);
    // forward_trigrams_score[i].resize(num_word_nodes);
    forward_trigrams_score_best[i].resize(num_word_nodes, INF);
    word_bow_reverse_cache[i].resize(num_word_nodes);

    for (int ord = 0; ord < ORDER - 1; ord++) {
      bigram_weight_cache[ord][i].resize(num_word_nodes);
    }
  }

  for (int ord = 0; ord < ORDER - 1; ord++) {
    bi_rescore[ord]->recompute_bigram_weights(true);
  }

  first_time = true;
}

void Subproblem::projection_with_constraints(int limit, int & k,
                                             map<int, set <int> > & constraints,
                                             vector < int> & proj) {
  // Use DSATUR
  int most_constrained = 0;
  int most_constraints = -1;
  k = 0;
  for (int w1 = 0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    // count constraints
    int num_cons = constraints[w1].size();
    if (num_cons > most_constraints) {
      most_constraints =num_cons;
      most_constrained = w1;
    }
  }

  int cur = most_constrained;
  proj.resize(graph->num_word_nodes);
  int unprocessed = 0;
  for (int w1 = 0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    proj[w1] = 0;
    unprocessed++;
  }

  set <int> done;
  while (unprocessed >0) {
    vector <int> counts(k);
    for (int d =0; d < k; d++) {
      counts[d] = 0;
    }

    for (set<int>::const_iterator iter =constraints[cur].begin();
         iter != constraints[cur].end(); iter++) {
      int c = (*iter);
      if (done.find(c) != done.end()) {
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

    assert(cur < graph->num_word_nodes);
    if (min == 0) {
      proj[cur] = mind;
    } else if (k == limit) {
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
    for (int w1 = 0; w1 < graph->num_word_nodes; w1++) {
      if (!graph->is_word(w1)) continue;
      // only consider unassigned words
      if (done.find(w1) != done.end()) continue;

      int sat =0;
      vector <int> counts(k);
      for (int d = 0; d < k; d++) {
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

// true - > 0
// false -> 1
void Subproblem::update_weights(vector <int> u_pos,
                                vector <float> u_values,
                                int pos) {
  assert(u_pos.size() == u_values.size());
  bi_rescore[pos]->update_weights(u_pos, u_values, u_pos.size());
}

vector <int> Subproblem::get_best_nodes_between(int w1, int w2, int pos) {
  vector <int> path;
  path = bi_rescore[pos]->get_bigram_path(w1, w2);

  // This is where we add the extra node
  path.push_back(w2);
  return path;
}

float Subproblem::get_best_bigram_weight(int w1, int w2, int pos) {
  return bi_rescore[pos]->get_bigram_weight(w1, w2);
}

int Subproblem::fixed_last_bigram(int w1) {
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
    assert(w0 != -1);
    if (gd->forward_bigrams[w0].size() ==1) {
      return w0;
    } else {
      return -1;
    }
  }
  return -1;
}

void Subproblem::initialize_caches() {
  cout << "Creating cache" << endl;
  for (int w1 = 0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    const vector <int> & f1 = gd->forward_bigrams[w1];
    for (unsigned int i = 0; i < f1.size(); i++) {
      int w2 = f1[i];

      bigram_in_lm[w1][w2] = word_bow_bigram_reverse(w1, w2);
      forward_trigrams[w1][i] = new vector<Trigram>();
      // forward_trigrams_score[w1][i] = new vector<float>();
      word_bow_reverse_cache[w1][w2] = new vector<float>(gd->forward_bigrams[w2].size());
      bigram_score_cache[w1][w2] =
          (_lm_weight) * word_prob_bigram_reverse(w1, w2);
      backoff_score_cache[w1][w2] =
          (_lm_weight) * word_backoff_two(w1, w2);

      best_lm_score[w1][w2] = INF;
    }
  }
  cout << "Done cache" << endl;

  for (int w1 = 0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;

    const vector <int> & f1 = gd->forward_bigrams[w1];

    for (unsigned int i = 0; i< f1.size(); i++) {
      int w2 = f1[i];

      VocabIndex context[] = {_word_node_cache.store[w2], Vocab_None};
      lm->wordProbPrimeCache(_word_node_cache.store[w1], context);

      for (unsigned int j =0; j < gd->forward_bigrams[w2].size(); j++) {
        int w3 = gd->forward_bigrams[w2][j];
        float lm_score;
        if (bigram_in_lm[w1][w2] && bigram_in_lm[w2][w3] &&
            lm->hasNext(_word_node_cache.store[w3])) {
          VocabIndex context[] =
              {_word_node_cache.store[w2],
               _word_node_cache.store[w3],
               Vocab_None};
           lm_score = (_lm_weight) *
               lm->wordProbFromCache(_word_node_cache.store[w1], context);

           forward_trigrams[w1][i]->push_back(Trigram(w3, lm_score));
          // forward_trigrams_score[w1][i]->push_back(lm_score);
          forward_trigrams_score_best[w1][i] = min(forward_trigrams_score_best[w1][i], lm_score);
          (*word_bow_reverse_cache[w1][w2])[j] = lm_score;
        } else {
          lm_score = backoff_score_cache[w2][w3] + bigram_score_cache[w1][w2];
          (*word_bow_reverse_cache[w1][w2])[j] = lm_score;
        }

        if (lm_score < best_lm_score[w1][w2]) {
          best_lm_score[w1][w2] = lm_score;
        }
      }
    }
  }
}


void Subproblem::solve(bool exact) {
  if (first_time) {
    cerr << "caches starting " << endl;
    initialize_caches();
    cerr << "caches inited " << endl;;
    for (int d = 0; d < MAX_PROJ; d++) {
      for (int d2 = 0; d2 < MAX_PROJ; d2++) {
        _first_time_proj[d][d2] = true;
      }
    }
  }
  assert(TRIPROJECT);

  for (int d = 0; d < projection_dims; d++) {
    for (int d2 = 0; d2 < projection_dims; d2++) {
      solve_proj(d, d2, _first_time_proj[d][d2],
                 cur_best_for_projection[d][d2], exact);
      _first_time_proj[d][d2] = false;
    }
  }
  first_time = false;
}

void Subproblem::solve_proj(int d2, int d3,
                            bool first_proj_time,
                            vector<ProjMax> &proj_best,
                            bool exact) {
  // solve (but only in the projected space)
  // unless is_simple
  int num_word_nodes = graph->num_word_nodes;
  vector <float> best_bigram(num_word_nodes);
  vector<float> best_bigram_with_backoff(num_word_nodes);
  vector<float> best_backoff(num_word_nodes);
  vector<int> best_bigram_with_backoff_forward(num_word_nodes);


  clock_t begin;
  if (TIMING) begin=clock();



  {
    for (int i = 0; i < graph->num_word_nodes; i++) {
      if (!graph->is_word(i)) continue;
      best_bigram[i] = INF;
      best_bigram_with_backoff[i] = INF;
      best_bigram_with_backoff_forward[i] = -1;
    }

    for (int w1 = 0; w1 < graph->num_word_nodes; w1++) {
      if (!graph->is_word(w1)) continue;
      const vector <int> & f1 = gd->forward_bigrams[w1];
      for (int ord = 0; ord < ORDER-1; ord++) {
        bigram_weight_best[ord][w1] = INF;
      }
      for (unsigned int i = 0; i< f1.size(); i++) {
        int w2 = f1[i];

        for (int ord = 0; ord < ORDER-1; ord++) {
          float lm_score =
              bi_rescore[ord]->get_bigram_weight(w1, w2);
          bigram_weight_cache[ord][w1][w2] =
              lm_score;
          bigram_weight_best[ord][w1] =
              min(bigram_weight_best[ord][w1], lm_score);
        }

        if (TRIPROJECT && project_word(w2) != d3) {
        } else {
          float score = bigram_weight_cache[1][w1][w2];

          if (score < best_bigram[w1]) {
            best_bigram[w1] = score;
          }


          float backoff = backoff_score_cache[w1][w2];
          float score_with_backoff =  backoff + score;
          if (score_with_backoff < best_bigram_with_backoff[w1]) {
            best_bigram_with_backoff[w1] = score_with_backoff;
            best_backoff[w1] = backoff;
            assert(!TRIPROJECT || project_word(w2) == d3);
            best_bigram_with_backoff_forward[w1] = i;
          }
        }
      }
    }
  }

  assert(graph->num_word_nodes > 10);
  for (int w1 =0; w1 < graph->num_word_nodes; ++w1) {
    if (!graph->is_word(w1)) continue;
    bool reset = false;
    if (!first_proj_time && !(exact && _non_exact)) {
      if (proj_best[w1].ord_best[0] == -1) {
        reset = true;
      } else {
        // int w1 = i;
        int one = proj_best[w1].ord_best[0];
        int two = proj_best[w1].ord_best[1];

        if (project_word(one) != d2 || project_word(two) != d3) {
          reset = true;
        } else {
          int w0 = fixed_last_bigram(w1);

          float old_score = proj_best[w1].score;
          proj_best[w1].score =
            bigram_weight_cache[0][w1][one] +
            bigram_weight_cache[1][one][two] +
              word_prob_reverse(w1, one, two) * (_lm_weight);
               // (*word_bow_reverse_cache[w1][one])[two]; //

          if (w0 != -1) {
            proj_best[w1].score +=
              bigram_weight_cache[0][w0][w1] +
              bigram_weight_cache[1][w1][one] +
                word_prob_reverse(w0, w1, one) * (_lm_weight);
                 // (*word_bow_reverse_cache[w0][w1])[one]; //
          }

          proj_best[w1].ord_best[0] = one;
          proj_best[w1].ord_best[1] = two;
          proj_best[w1].is_new =
              !(fabs(old_score - proj_best[w1].score) < 1e-4);
          assert(proj_best[w1].ord_best[0] != proj_best[w1].ord_best[1]);
          assert(proj_best[w1].score < 1000);
        }
      }
    } else {
      reset = true;
    }
    if (reset) {
      proj_best[w1].score = INF;
      for (int ord =0; ord < ORDER -1; ord++) {
        proj_best[w1].ord_best[ord] = -1;
      }
      proj_best[w1].is_new = true;
    }
  }

  clock_t end;
  if (TIMING) {
    end = clock();
    cout << "Precompute time: "
         << Clock::diffclock(end, begin) << " ms" << endl;
    // actual algorithm
    begin = clock();
  }

  // counters
  assert(gd->valid_bigrams().size() > 0);
  int words = 0, lookups = 0, lookups2 = 0, updates = 0, quick_updates = 0;
  // words that are bounded by a later word
  vector<int> word_override;

  for (int w1 = 0; w1 < graph->num_word_nodes; w1++) {
    if (!graph->is_word(w1)) continue;
    overridden[w1] = false;
    words++;
    // Edge tightness optimization
    bool on_edge = false;
    const vector<float> *on_edge_scores;
    // w0 is the only thing preceding w1
    int w0 = fixed_last_bigram(w1);

    if (w0 != -1) {
      on_edge = true;
      word_override.push_back(w0);
      on_edge_scores = word_bow_reverse_cache[w0][w1];
    }
    assert(w0 == -1 || on_edge ==true);
    const vector<int> &f1 = gd->forward_bigrams[w1];

    float best_score = proj_best[w1].score;

    const vector<float> &best_lm = best_lm_score[w1];
    const vector<float> &score_cache = bigram_score_cache[w1];

    for (int i = 0; i < f1.size(); ++i) {
      int w2 = f1[i];
      const vector<float> &prob_cache = *word_bow_reverse_cache[w1][w2];
      if (project_word(w2) != d2) continue;
      if (gd->forward_bigrams[w2].size() == 0 ||
          best_bigram_with_backoff_forward[w2] == -1) {
        continue;
      }
      float score1 = bigram_weight_cache[0][w1][w2];
      if (on_edge) {
        lookups2++;
        score1 += bigram_weight_cache[0][w0][w1] +
            bigram_weight_cache[1][w1][w2] +
            (*on_edge_scores)[i]; // _lm_weight *
      } else {
        // if (bigram_weight_best[0][w1] + bigram_weight_best[1][w2] + forward_trigrams_score_best[w1][i] >= best_score) {
        //   continue;
        // }
      }

      // check NaN.
      assert(score1 == score1);
      // const vector<Trigram> &trigrams = *forward_trigrams[w1][i];
      const vector<float> &bigram_cache = bigram_weight_cache[1][w2];

      // Only consider words with full lm context.
      //float estimate  = best_lm[w2] + best_bigram[w2] + score1;
      // float bi_lm_score = score_cache[w2];
      int w3_index = best_bigram_with_backoff_forward[w2];

      int w3 = gd->forward_bigrams[w2][w3_index];
      // float score2 = bigram_cache[w3];
      //float score  = bi_lm_score + best_backoff[w2] + score1 + score2;

      //if (prob_cache[w3] != 0.0) {
      float score =  prob_cache[w3_index] + score1 + bigram_cache[w3]; // (_lm_weight) *
        //}



      // reverse
      // VocabIndex context[] = {_word_node_cache.store[w2],
      //                         _word_node_cache.store[w3], Vocab_None};
      // unsigned int length;
      // lm->contextID(_word_node_cache.store[w1], context, length);
      //return length;


      // assert(proj_best[w1].score <= INF);
      // if  (fabs(((_lm_weight) *
      //            word_prob_reverse(w1, w2, w3)
      //            + score1 + score2) - score) > 1e-4) {
      //   cout << "Optimization fail"<< endl;
      //   exit(0);
      // }
      // assert(!TRIPROJECT || project_word(w3) == d3);
      // cerr << score << " " << best_score << endl;
      if (score < best_score || best_score >= INF) {
        quick_updates++;
        best_score = score;
        proj_best[w1].ord_best[0] = w2;
        proj_best[w1].ord_best[1] = w3;
        proj_best[w1].is_new = true;
        assert(proj_best[w1].ord_best[0] != proj_best[w1].ord_best[1]);
      }

      // try_set_max(proj_best, w1, w2, w3, score, true);

      if (exact) {
        const vector<Trigram> &f2 = *forward_trigrams[w1][i];
        for (unsigned int j = 0; j < f2.size(); j++) {
          if (j % 10 == 0 && score1 + bigram_weight_best[1][w2] + forward_trigrams_score_best[w1][i] >= best_score - 1e-4) {
            break;
          }

          int w3 = f2[j].word;
          //cerr << "BEST SCORE " << best_score << " " << score1 << " " << bigram_cache[w3] << " " << trigrams[j] << " " << bigram_weight_best[1][w2] << " " << forward_trigrams_score_best[w1][i] << endl;
          lookups++;
          //float score = score1 + bigram_cache[w3] + trigrams[j];
          if (score1 + bigram_cache[w3] + f2[j].score < best_score) {
            if (project_word(w3) != d3) continue;
            updates++;
            best_score = score1 + bigram_cache[w3] + f2[j].score;
            proj_best[w1].ord_best[0] = w2;
            proj_best[w1].ord_best[1] = w3;
            proj_best[w1].is_new = true;
            assert(proj_best[w1].ord_best[0] != proj_best[w1].ord_best[1]);
          }
          // try_set_max(proj_best, w1, w2, w3, score, true);
        }
      }
    }
    if (best_score < proj_best[w1].score) {
      proj_best[w1].score = best_score;
    }
  }

  if (TIMING) {
    clock_t end = clock();
    cout << "INIT TRIGRAM TIME: "
         << Clock::diffclock(end, begin) << " ms"<< endl;
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
    float first  = (_lm_weight) * word_prob_reverse(w0, w1, w2) +
                    bigram_weight_cache[0][w0][w1] +
                    bigram_weight_cache[1][w1][w2];
    float second = (_lm_weight) * word_prob_reverse(w1, w2, w3) +
                    bigram_weight_cache[0][w1][w2] +
                    bigram_weight_cache[1][w2][w3];

    assert(fabs(first + second - proj_best[w1].score) < 1e-4);

    proj_best[w0].score = 0.0;
    proj_best[w0].ord_best[0] = w1;
    proj_best[w0].ord_best[1] = w2;
    proj_best[w0].is_new = false;
  }

  _non_exact = !exact;
  if (TIMING) {
    clock_t end = clock();
    cout << "TRIGRAM TIME: "
         << Clock::diffclock(end, begin) << " ms"<< endl;
    cout << "Words: " << words << endl;
    cout << "Lookups: " << lookups << endl;
    cout << "Lookups2: " << lookups2 << endl;
    cout << "Updates: " << updates << endl;
    cout << "Quick Updates: " << quick_updates << endl;
    cout << "Override: " << word_override.size() << endl;
  }
}

void Subproblem::project(int proj_dim, vector <int> proj ) {
  assert(PROJECT || TRIPROJECT);
  assert(proj_dim >=1 && proj_dim <= MAX_PROJ);
  projection = proj;
  projection_dims = proj_dim;
}
