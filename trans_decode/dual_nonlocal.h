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

  struct LatticeCache {
   LatticeCache() : bigram_score(0), trigram_score(0) {}
    vector<int> lat_ids;
    double bigram_score;
    double trigram_score;
  };


template <class TDerivation>
class TDualNonLocal: public BaseNonLocal<TDerivation> {
public:
  TDualNonLocal(const HGraph & forest,
               Ngram & lm,
               double lm_weight,
              const Cache <Hypernode, int> & word_cache,
              const Cache <Hypernode, double> & best_trigram,
              const ForestLattice &lattice,
              wvector *duals,
              Subproblem *subproblem,
              HEdges &used_edges)
       : BaseNonLocal<TDerivation >(forest,
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

  void get_lattice(const Hyperedge &edge,
                   vector<LatticeCache> *ret) const {
    const vector<int> &all_lat = lattice_.original_edges[edge.id()];
    LatticeCache temp;
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
          temp = LatticeCache();
        }
        temp.lat_ids.push_back(lat);
        temp.bigram_score += bigram_score_[lat];
        temp.trigram_score += trigram_score_[lat];
        last_nt = true;
      } else {
        ret->push_back(temp);

        last_nt = false;
        temp = LatticeCache();
      }
    }

    ret->push_back(temp);

    if (DEBUG_NONLOCAL) {
      for (int i = 0; i < ret->size(); ++i) {
        for (int j = 0; j < ret[i].size(); ++j) {
          cerr << lattice_._edge_label_by_nodes[(*ret)[i].lat_ids[j]] << " ";
        }
        cerr << endl;
      }
      cerr << ret->size() << " " << edge.tail_nodes().size() + 1;
    }
    assert(ret->size() == edge.tail_nodes().size() + 1);
  }

protected:
  const Cache <Hypernode, double> & _best_trigram;
  const ForestLattice &lattice_;
  wvector *duals_;
  Subproblem *subproblem_;
  HEdges &used_edges_;

  mutable vector<int> tmp_derivation_;

  Cache<Hyperedge, vector<LatticeCache > > edge_lattice_cache_;

  mutable vector<double> bigram_score_;
  mutable vector<double> trigram_score_;
};

class DualNonLocal: public TDualNonLocal<vector<int> > {
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
      : TDualNonLocal<vector<int> >(forest, lm, lm_weight, word_cache,
                                    best_trigram, lattice, duals, subproblem, used_edges) {}





  double internal_score(const vector<const vector<int> *> &subder,
                        int edge_pos,
                        const vector<LatticeCache > &lat_ids,
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
        for (int i = 0; i < lat_ids[subder_index + edge_pos].lat_ids.size(); ++i) {
          int lat_id = lat_ids[subder_index + edge_pos].lat_ids[i];
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
    const vector<LatticeCache > &lat_ids =
        edge_lattice_cache_.store[edge.id()];

    double running_pretrigram_score = 0.0,
      running_bigram_score = 0.0,
      running_trigram_score = 0.0;
    bool fail = false;

    // First edge part.
    if (edge_pos <= 1) {
      foreach (int lat_id, lat_ids[0].lat_ids) {
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
      foreach (int lat_id, lat_ids[size].lat_ids) {
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
    if (full_size != 1) {
      signature.resize(4);
    } else {
      signature.resize(2);
    }
    signature[0] = full_derivation[0];
    signature[1] = full_derivation[full_size - 1];
    if (full_size != 1) {
      signature[2] = full_derivation[1];
      signature[3] = full_derivation[full_size - 2];
    }
    return true;
  }

  // Initialize the hypothesis for leaf nodes.
  Hyp<vector<int> > initialize(const Hypernode &node) const {
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
    vector<int> signature(2);
    signature[0] = node_index;
    signature[1] = node_index;

    // So far dervation is just this node
    vector<int> derivation;
    derivation.push_back(lat_id);

    // There are no edges.
    vector<int> edges;

    return Hyp<vector<int> >(score, score, signature, derivation, edges);
  }
};



struct Derivation {
  Derivation() {
    start_bi_penalty[0] = 0;
    start_bi_penalty[1] = 0;
    start_tri_penalty[0] = 0;
    start_tri_penalty[1] = 0;

    end_bi_penalty[0] = 0;
    end_bi_penalty[1] = 0;
    end_tri_penalty[0] = 0;
    end_tri_penalty[1] = 0;

    end_word[0] = -1;
    start_word[1] = -1;

    two_words = false;
  }
  int end_word[2];
  int start_word[2];
  double start_bi_penalty[2];
  double start_tri_penalty[2];
  double end_bi_penalty[2];
  double end_tri_penalty[2];

  bool two_words;

  void show() const {
    cerr << start_word[0] << " " << start_word[1] << " " << end_word[0] << " " << end_word[1] << " " << two_words << endl;
  }
};


class DerDualNonLocal: public TDualNonLocal<Derivation> {
 public:
  DerDualNonLocal(const HGraph & forest,
                  Ngram & lm,
                  double lm_weight,
                  const Cache <Hypernode, int> & word_cache,
                  const Cache <Hypernode, double> & best_trigram,
                  const ForestLattice &lattice,
                  wvector *duals,
                  Subproblem *subproblem,
                  HEdges &used_edges)
      : TDualNonLocal<Derivation >(forest, lm, lm_weight, word_cache,
                                    best_trigram, lattice, duals, subproblem, used_edges) {}


  double score_trigram(int s1, int s2, int s3, double penalty, double pre) const {
    double score = 0;
    int n1 = lattice_.get_hypergraph_node_from_word(Word(s1));
    int n2 = lattice_.get_hypergraph_node_from_word(Word(s2));
    int n3 = lattice_.get_hypergraph_node_from_word(Word(s3));
    double lm_score = trigram(index(n1), index(n2), index(n3));
    if (!subproblem_->is_overridden(s1)) {
      score += lm_score;
    }

    if (index(n1) != 0 && !subproblem_->is_overridden(s1)) {
      double drop = penalty + _best_trigram.store[n1] + bigram_score_[s2] + trigram_score_[s3];

      int w0 = subproblem_->fixed_last_bigram(s1);
      double inner_lm_score = 0.0;
      if (w0 != -1) {
        int next_node = lattice_.get_hypergraph_node_from_word(Word(w0));
        if (index(next_node) != 1) {
          inner_lm_score = trigram(index(next_node),
                                   index(n1),
                                   index(n2));

          drop += pre + trigram_score_[s2];
          score += inner_lm_score;
        }
      }
      score -= drop;
    }
    return score;
  }

  double internal_score(const vector<const Derivation *> &subder,
                        const LatticeCache &lat_cache) const {
    double score = 0;
    double between_bigram = lat_cache.bigram_score;
    double between_trigram = lat_cache.trigram_score;

    if (subder[1]->two_words) {
      int s1 = subder[1]->start_word[1];
      int s2 = subder[1]->start_word[0];
      int s3 = subder[0]->end_word[1];
      double penalty = subder[0]->end_tri_penalty[1] + between_trigram +
          subder[1]->start_tri_penalty[0] + subder[1]->start_bi_penalty[1];
      double pre = subder[1]->start_tri_penalty[1];
      score += score_trigram(s1, s2, s3, penalty, pre);
    }

    if (subder[0]->two_words) {
      int s1 = subder[1]->start_word[0];
      int s2 = subder[0]->end_word[1];
      int s3 = subder[0]->end_word[0];
      double penalty = subder[0]->end_tri_penalty[0] + between_bigram +
          subder[0]->end_bi_penalty[1] + subder[1]->start_bi_penalty[0];
      double pre = subder[0]->end_tri_penalty[1] + subder[1]->start_tri_penalty[0] + between_trigram;
      score += score_trigram(s1, s2, s3, penalty, pre);
    }
    return score;
  }

  bool compute(const Hyperedge &edge,
               int edge_pos,
               double bound,
               const vector<const Derivation *> &subder,
               double &score,
               Derivation &der,
               vector <int> &signature) const {
    const vector<LatticeCache> &lat_cache =
        edge_lattice_cache_.store[edge.id()];

    // First edge part.
    int size = edge.tail_nodes().size();
    bool start = (edge_pos <= 1);
    bool end = (edge_pos == size - 1);
    bool bleft = (subder.size() > 1 && !subder[0]->two_words);
    bool bright = (subder.size() > 1 && !subder[1]->two_words);

    double start_bigram =  start ? lat_cache[0].bigram_score : 0.0;
    double start_trigram = start ? lat_cache[0].trigram_score : 0.0;
    double between_bigram =  lat_cache[edge_pos].bigram_score;
    double between_trigram = lat_cache[edge_pos].trigram_score;
    double end_bigram =  end ? lat_cache[size].bigram_score : 0.0;
    double end_trigram = end ? lat_cache[size].trigram_score : 0.0;

    assert(subder.size() == 2 || subder.size() == 1);
    if (subder.size() > 1) {
      score = internal_score(subder, lat_cache[edge_pos]);
      if (score > bound) return false;
    }

    int o = subder.size() - 1;
    assert(subder[0]->start_word[0] != subder[0]->start_word[1]);
    assert(subder[0]->end_word[0] != subder[0]->end_word[1]);
    assert(subder[o]->start_word[0] != subder[o]->start_word[1]);
    assert(subder[o]->end_word[0] != subder[o]->end_word[1]);


    der.start_word[0] = subder[0]->start_word[0];
    der.start_word[1] = (subder[0]->two_words) ? subder[0]->start_word[1] : subder[o]->start_word[0];
    der.end_word[0] =   (subder[o]->two_words) ? subder[o]->end_word[0] : subder[0]->end_word[1];
    der.end_word[1] =   subder[o]->end_word[1];

    if (der.start_word[0] == der.start_word[1]) der.start_word[1] = -1;
    if (der.end_word[0]   == der.end_word[1]) der.end_word[0] = -1;

    der.start_bi_penalty[0] =  subder[0]->start_bi_penalty[0] + start_bigram;
    der.start_tri_penalty[0] = subder[0]->start_tri_penalty[0] + start_trigram;

    if (o == 0 || subder[0]->two_words) {
      der.start_bi_penalty[1] = subder[0]->start_bi_penalty[1];
      der.start_tri_penalty[1] = subder[0]->start_tri_penalty[1];
    } else { //if (o == 1 && !subder[0]->two_words && subder[1]->two_words) {
      der.start_bi_penalty[1] =  subder[0]->end_bi_penalty[1] + between_bigram +
          subder[1]->start_bi_penalty[0];
      der.start_tri_penalty[1] = subder[0]->end_tri_penalty[1] + between_trigram +
          subder[1]->start_tri_penalty[0];
    }

    if (o == 0) {
      der.end_bi_penalty[0] =  subder[0]->end_bi_penalty[0];
      der.end_tri_penalty[0] = subder[0]->end_tri_penalty[0];
    } else if (subder[1]->two_words) {
      der.end_bi_penalty[0] =  subder[1]->end_bi_penalty[0];
      der.end_tri_penalty[0] = subder[1]->end_tri_penalty[0];
    } else {
      der.end_bi_penalty[0]  = subder[0]->end_bi_penalty[1] + subder[1]->start_bi_penalty[0] + between_bigram;
      der.end_tri_penalty[0] = subder[0]->end_tri_penalty[1] + subder[1]->start_tri_penalty[0] + between_trigram;
    }

    der.end_bi_penalty[1] =  subder[o]->end_bi_penalty[1] + end_bigram;
    der.end_tri_penalty[1] = subder[o]->end_tri_penalty[1] + end_trigram;
    der.two_words = subder.size() == 2 || subder[0]->two_words;

    /* cerr << "COMBINE" << endl; */
    /* subder[0]->show(); */
    /* subder[o]->show(); */
    /* der.show(); */
    /* cerr << endl << endl; */
    if (der.two_words) {
      signature.resize(4);
      signature[0] = subder[0]->start_word[0];
      signature[1] = subder[o]->end_word[1];
      signature[2] = subder[0]->start_word[1];
      signature[3] = subder[o]->end_word[0];
    } else {
      signature.resize(2);
      signature[0] = subder[0]->start_word[0];
      signature[1] = subder[o]->end_word[1];
    }

    //assert(!fail);
    return true;
  }

    // Initialize the hypothesis for leaf nodes.
  Hyp<Derivation> initialize(const Hypernode &node) const {
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
    vector<int> signature(2);
    signature[0] = node_index;
    signature[1] = node_index;

    // So far dervation is just this node
    Derivation der;
    der.end_word[1] = lat_id;
    der.start_word[0] = lat_id;

    // There are no edges.
    vector<int> edges;

    return Hyp<Derivation >(score, score, signature, der, edges);
  }
};

#endif
