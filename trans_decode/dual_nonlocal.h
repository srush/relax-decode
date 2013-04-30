#ifndef DUALNONLOCAL_H_
#define DUALNONLOCAL_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <set>

#include "CubePruning.h"
#include "EdgeCache.h"
#include "Hypergraph.h"
#include <Ngram.h>
#include "ForestLattice.h"
#include "LMNonLocal.h"

using namespace std;

#define DEBUG_NONLOCAL 0

class DualNonLocal: public LMNonLocal {
 public:
  DualNonLocal(const HGraph & forest,
              Ngram & lm,
              double lm_weight,
              const Cache <Hypernode, int> & word_cache,
              const Cache <Hypernode, double> & best_trigram,
              const ForestLattice &lattice,
              wvector *duals,
              Subproblem *subproblem,
              HEdges &used_edges)
       : LMNonLocal(forest,
                lm,
                lm_weight,
                word_cache, false),
    _best_trigram(best_trigram),
    lattice_(lattice),
    duals_(duals),
    subproblem_(subproblem),
    used_edges_(used_edges),
    edge_lattice_cache_(forest.edges().size()),
    tmp_derivation_(1000) {
    bigram_score_.resize(lattice_.get_graph().num_edges());
    trigram_score_.resize(lattice_.get_graph().num_edges());
    foreach (HEdge edge, forest.edges()) {
      edge_lattice_cache_.has_value[edge->id()] = true;
      get_lattice(*edge, &edge_lattice_cache_.store[edge->id()]);
    }
  }

  int index_cache(int i) const {
    return index(i);
  }



  void get_lattice(const Hyperedge &edge,
                   vector<vector<int> > *ret) const {
    const vector<int> &all_lat = lattice_.original_edges[edge.id()];
    vector<int> temp;
    int last_lat;
    ret->clear();
    bool last_nt = false;
    for (int i = 0; i < all_lat.size(); ++i) {
      int lat = all_lat[i];
      if (lat >= bigram_score_.size()) {
        bigram_score_.resize(lat + 1);
        trigram_score_.resize(lat + 1);
      }
      bigram_score_[lat] = (*duals_)[lat];
      trigram_score_[lat] = (*duals_)[lat + GRAMSPLIT];

      if (!lattice_.is_word(lat)) {
        if (last_nt) {
          ret->push_back(temp);
          temp.clear();
        }
        temp.push_back(lat);
        last_nt = true;
      } else {
        ret->push_back(temp);

        last_nt = false;
        temp.clear();
      }
    }

    ret->push_back(temp);

    if (DEBUG_NONLOCAL) {
      for (int i = 0; i < ret->size(); ++i) {
        for (int j = 0; j < ret[i].size(); ++j) {
          cerr << lattice_._edge_label_by_nodes[(*ret)[i][j]] << " ";
        }
        cerr << endl;
      }
      cerr << ret->size() << " " << edge.tail_nodes().size() + 1;
    }
    assert(ret->size() == edge.tail_nodes().size() + 1);
  }

  double internal_score(const vector<const vector<int> *> &subder,
                        int edge_pos,
                        const vector<vector<int> > &lat_ids,
                        int &tmp_derivation_size,
                        double &running_bigram_score,
                        double &running_pretrigram_score,
                        double &score) const {
    int word_number = 0;
    int last_word;
    int last_word2;
    bool last_was_word = false;
    double running_trigram_score = 0.0;
    for (int subder_index = 0; subder_index < subder.size();
         ++subder_index) {
      const vector<int> &sub = *subder[subder_index];
      if (DEBUG_NONLOCAL) {
        cerr << " [ ";
      }
      int inner_word_number = 0;
      for (int s = 0; s < sub.size(); ++s) {
        if (lattice_.is_word(sub[s])) {
          if (DEBUG_NONLOCAL) {
            cerr << lattice_.get_word(sub[s]) << " ";
          }
          word_number++;
          inner_word_number++;
          int node =
            lattice_.get_hypergraph_node_from_word(Word(sub[s]));
          if (word_number > 2 && inner_word_number <= 2) {
            // Fix up.
            double lm_score = trigram(index(node),
                                      last_word,
                                      last_word2);
            if (!subproblem_->is_overridden(sub[s])) {
              score += lm_score;
            }
            if (index(node) != 0 &&
                !subproblem_->is_overridden(sub[s])) {
              double drop = running_bigram_score +
                running_trigram_score + _best_trigram.store[node];

              double a =
                  lm_score - running_bigram_score - running_trigram_score;
              double b =
                  _best_trigram.store[node];

              int w0 = subproblem_->fixed_last_bigram(sub[s]);
              double inner_lm_score = 0.0;
              if (w0 != -1) {
                int next_node =
                    lattice_.get_hypergraph_node_from_word(Word(w0));
                if (index(next_node) != 1) {
                  inner_lm_score = trigram(index(next_node),
                                           index(node),
                                           last_word);
                  a = a + inner_lm_score - running_pretrigram_score;
                  drop += running_pretrigram_score;
                  score += inner_lm_score;
                }
              }
              if (DEBUG_NONLOCAL) {
                cerr << "{TRIGRAMDROP " << running_pretrigram_score << " "
                     << running_bigram_score << " " << running_trigram_score
                     << " " << lm_score << " " << a << " " << b << " " << w0
                     << " " << inner_lm_score << "}";
              }
              score -= drop;
              /* if (!(b <= a + 0.01)) { */
              /*   fail = true; */
              /* } */
            }
          }
          last_word2 = last_word;
          last_word = index(node);
          running_trigram_score = running_pretrigram_score;
          running_bigram_score = 0.0;
          running_pretrigram_score = 0.0;
        } else {
          if (DEBUG_NONLOCAL) {
            cerr << lattice_._edge_label_by_nodes[sub[s]] << " ";
          }
        }

        tmp_derivation_[tmp_derivation_size] = sub[s];
        tmp_derivation_size++;


        running_bigram_score +=  bigram_score_[sub[s]];
        running_pretrigram_score += trigram_score_[sub[s]];
        if (DEBUG_NONLOCAL) {
          cerr << "(" << (*duals_)[sub[s]] << "/"
               << (*duals_)[sub[s] + GRAMSPLIT] << ")";
        }
      }
      if (DEBUG_NONLOCAL) {
        cerr << " ] ";
      }
      if (subder_index < subder.size() - 1) {
        for (int i = 0; i < lat_ids[subder_index + edge_pos].size(); ++i) {
          int lat_id = lat_ids[subder_index + edge_pos][i];
          running_bigram_score += bigram_score_[lat_id];
          running_pretrigram_score += trigram_score_[lat_id];
          if (DEBUG_NONLOCAL) {
            cerr << lattice_._edge_label_by_nodes[lat_id] << " ";
            cerr << "(" << (*duals_)[lat_id] << "/"
                 << (*duals_)[lat_id + GRAMSPLIT] << ")";
          }
          tmp_derivation_[tmp_derivation_size] = lat_id;
          tmp_derivation_size++;
        }
      }
    }
  }

  double score_and_compute(const Hyperedge &edge,
                           int edge_pos,
                           const vector<const vector<int> *> &subder,
                           int &tmp_derivation_size) const {
    double score = 0.0;
    tmp_derivation_size = 0;
    const vector<vector<int> > &lat_ids =
      edge_lattice_cache_.get_value(edge);

    double running_pretrigram_score = 0.0,
      running_bigram_score = 0.0,
      running_trigram_score = 0.0;
    bool fail = false;

    // First edge part.
    if (edge_pos <= 1) {
      foreach (int lat_id, lat_ids[0]) {
        running_bigram_score += bigram_score_[lat_id];
        running_pretrigram_score += trigram_score_[lat_id];
        if (DEBUG_NONLOCAL) {
          cerr << lattice_._edge_label_by_nodes[lat_id] << " ";
          cerr << "(" << (*duals_)[lat_id] << "/"
               << (*duals_)[lat_id + GRAMSPLIT] << ")";
        }
        tmp_derivation_[tmp_derivation_size] = lat_id;
        tmp_derivation_size++;
      }
    }
    internal_score(subder,
                   edge_pos,
                   lat_ids,
                   tmp_derivation_size,
                   running_bigram_score,
                   running_pretrigram_score,
                   score);
    int size = edge.tail_nodes().size();
    if (edge_pos == size - 1) {
      foreach (int lat_id, lat_ids[size]) {
        running_bigram_score += bigram_score_[lat_id];
        running_pretrigram_score += trigram_score_[lat_id];
        if (DEBUG_NONLOCAL) {
          cerr << lattice_._edge_label_by_nodes[lat_id] << " ";
          cerr << "(" << (*duals_)[lat_id] << "/"
               << (*duals_)[lat_id + GRAMSPLIT] << ")";
        }
        tmp_derivation_[tmp_derivation_size] = lat_id;
        tmp_derivation_size++;
      }
    }
    assert(!fail);
    return score;
  }



  // Compute takes the hyperedge and sub-derivations to combine.
  // Returns the new score, derivation and signature.
  bool compute(const Hyperedge &edge,
               int edge_pos,
               double bound,
               const vector<const vector<int> *> &subder,
               double &score,
               vector <int> &full_derivation,
               vector <int> &signature) const {
    if (DEBUG_NONLOCAL) cerr << "Computing "
                             << edge_pos << " " << edge.label() << endl;
    int tmp_derivation_size = 0;
    score = score_and_compute(edge, edge_pos, subder, tmp_derivation_size);
    if (score > bound) return false;

    // Construct dervations.
    full_derivation.clear();
    signature.clear();

    int first = tmp_derivation_size, second = 0;

    // Only keep up to and including the second word.
    int words_seen = 0;
    for (int i = 0; i < tmp_derivation_size; ++i) {
      int lat_id = tmp_derivation_[i];
      if (lattice_.is_word(lat_id)) {
        words_seen++;
      }
      if (words_seen == 2) {
        first = i;
        break;
      }
    }
    words_seen = 0;
    for (int i = tmp_derivation_size - 1; i >= 0; --i) {
      int lat_id = tmp_derivation_[i];
      if (lattice_.is_word(lat_id)) {
        words_seen++;
      }
      if (words_seen == 2) {
        second = i;
        break;
      }
    }

    if (second <= first) {
      full_derivation.resize(tmp_derivation_size);
      for (int i = 0; i < tmp_derivation_size; ++i) {
        full_derivation[i] = tmp_derivation_[i];
      }
    } else {
      for (int i = 0; i <= first; ++i) {
        full_derivation.push_back(tmp_derivation_[i]);
      }
      for (int i = second; i < tmp_derivation_size; ++i) {
        full_derivation.push_back(tmp_derivation_[i]);
      }
    }

    assert(score < 10000);
    int full_size = full_derivation.size();
    assert(full_size > 0);

    // New signature is w_0 w_n w_1 w_{n-1}.
    signature.push_back(full_derivation[0]);
    signature.push_back(full_derivation[full_size - 1]);
    if (full_size != 1) {
      signature.push_back(full_derivation[1]);
      signature.push_back(full_derivation[full_size - 2]);
    }
    return true;
  }

  // Initialize the hypothesis for leaf nodes.
  Hyp initialize(const Hypernode &node) const {
    assert(node.is_terminal());

    // Get the index of the word at this node
    int lat_id = lattice_.get_word_from_hypergraph_node(node.id());
    int node_index = node.id();
    double score = 0.0;

    // If this is not a special node, add in the unigram score.
    if (index(node_index) != 0 && !subproblem_->is_overridden(lat_id)) {
      score += _best_trigram.get_default(node, 0.0);
      if (score > 100000) score = 0.0;
    }

    // The signature (left and right words).
    vector<int> signature;
    signature.push_back(node_index);
    signature.push_back(node_index);

    // So far dervation is just this node
    vector<int> derivation;
    derivation.push_back(lat_id);

    // There are no edges.
    vector<int> edges;

    return Hyp(score, score, signature, derivation, edges);
  }

  protected:
  const Cache <Hypernode, double> & _best_trigram;
  const ForestLattice &lattice_;
  wvector *duals_;
  Subproblem *subproblem_;
  HEdges &used_edges_;

  mutable vector<int> tmp_derivation_;
  Cache<Hyperedge, vector<vector<int > > > edge_lattice_cache_;

  mutable vector<double> bigram_score_;
  mutable vector<double> trigram_score_;
};

#endif
