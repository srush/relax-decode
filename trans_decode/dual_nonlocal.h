#ifndef DUALNONLOCAL_H_
#define DUALNONLOCAL_H_

#include "CubePruning.h"
#include "EdgeCache.h"
#include "Hypergraph.h"
#include <Ngram.h>
#include "ForestLattice.h"
#include "LMNonLocal.h"
#include <iostream>
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
    used_edges_(used_edges)
    //edge_lattice_cache_(forest.edges().size())
      {
        /* foreach (HEdge edge, forest.edges()) { */
        /*   vector<vector<vector<int > > > &lats =  */
        /*     edge_lattice_cache_.get(*edge); */
        /*   lats.resize(edge->tail_nodes().size() + 1); */
        /*   for (int i =0; i < lats.size(); ++i) { */
        /*     lats[i] = get_lattice(*edge, i); */
        /*   } */
        /* } */
      }

  int index_cache(int i) const {
    return index(i);
  }
  
  vector<vector<int> > get_lattice(const Hyperedge &edge,
                                   int edge_pos) const {
    
    const vector<int> &all_lat = lattice_.original_edges[edge.id()];
    vector<int> temp;
    vector<vector<int> > ret;
    int last_lat;
    /* if (lattice_.is_word(all_lat[0])) { */
    /*   ret.push_back(temp); */
    /* } */
    bool last_nt = false;
    for (int i = 0; i < all_lat.size(); ++i) {
      int lat = all_lat[i];
      if (!lattice_.is_word(lat)) {
        if (last_nt) {
          ret.push_back(temp);
          temp.clear();
        }
        temp.push_back(lat);
        last_nt = true;
      } else {
        ret.push_back(temp);
         
        last_nt = false;
        temp.clear();
      }
    }
    
    ret.push_back(temp);
    
    if (DEBUG_NONLOCAL) {
      for (int i = 0; i < ret.size(); ++i) {
        for (int j = 0; j < ret[i].size(); ++j) {
          cerr << lattice_._edge_label_by_nodes[ret[i][j]] << " ";
        }
        cerr << endl;
      }
      cerr << ret.size() << " " << edge.tail_nodes().size() + 1;
    }
    assert(ret.size() == edge.tail_nodes().size() + 1);
    
    return ret;
  }

  // Compute takes the hyperedge and sub-derivations to combine.
  // Returns the new score, derivation and signature.
  bool compute(const Hyperedge &edge,
               int edge_pos,
               double bound,
               const vector<vector <int> > &subder,
               double &score,
               vector <int> &full_derivation,
               vector <int> &signature) const {
    if (DEBUG_NONLOCAL) cerr << "Computing " << edge_pos << " " << edge.label() << endl;
    vector<int> tmp_derivation; 
    full_derivation.clear();
    signature.clear();
    score = 0.0;
 
    vector<vector<int> > lat_ids = get_lattice(edge, edge_pos);
    //edge_lattice_cache_.get(edge)[edge_pos];

    int word_number = 0;
    int last_word;
    int last_word2;
    bool last_was_word = false;
    double running_pretrigram_score = 0.0, 
      running_bigram_score = 0.0, 
      running_trigram_score = 0.0;
    bool fail = false;
    
    // First edge part. 
    if (edge_pos <= 1) {
      foreach (int lat_id, lat_ids[0]) {
        running_bigram_score += (*duals_)[lat_id];
        running_pretrigram_score += (*duals_)[GRAMSPLIT + lat_id];
        if (DEBUG_NONLOCAL) {
          cerr << lattice_._edge_label_by_nodes[lat_id] << " ";
          cerr << "(" << (*duals_)[lat_id] << "/" 
               << (*duals_)[lat_id + GRAMSPLIT] << ")";
        }
        tmp_derivation.push_back(lat_id);
      }      
    }
    for (int subder_index = 0; subder_index < subder.size(); 
         ++subder_index) {

      const vector<int> &sub = subder[subder_index];
      if (DEBUG_NONLOCAL) {
        cerr << " [ " ;
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
            double lm_score = trigram(index_cache(node), last_word, last_word2); 
            if (!subproblem_->is_overridden(sub[s])) {
              score += lm_score;
            }
            if (index(node) != 0 && !subproblem_->is_overridden(sub[s])) {      
              double drop = running_bigram_score + running_trigram_score + _best_trigram.store[node];

              double a = lm_score - running_bigram_score - running_trigram_score;
              double b = _best_trigram.store[node];

              int w0 = subproblem_->fixed_last_bigram(sub[s]);
              double inner_lm_score = 0.0;
              if (w0 != -1) {
                int next_node = lattice_.get_hypergraph_node_from_word(Word(w0));
                if (index_cache(next_node) != 1) {
                  inner_lm_score = trigram(index_cache(next_node), index_cache(node), last_word); 
                  a = a + inner_lm_score - running_pretrigram_score;
                  drop += running_pretrigram_score;
                  score += inner_lm_score;
                }
              }
              if (DEBUG_NONLOCAL) {
                cerr << "{TRIGRAMDROP " << running_pretrigram_score << " " << running_bigram_score << " " << running_trigram_score << " " << lm_score << " " << a << " " << b << " " << w0 << " " << inner_lm_score << "}";
              }
              score -= drop;       
              if (!(b <= a + 0.01)) {
                fail = true;
              }
            }
          }
          last_word2 = last_word;
          last_word = index_cache(node);
          running_trigram_score = running_pretrigram_score;
          running_bigram_score = 0.0;
          running_pretrigram_score = 0.0;
        } else {
          if (DEBUG_NONLOCAL) {
            cerr << lattice_._edge_label_by_nodes[sub[s]] << " ";
          }
        }
        tmp_derivation.push_back(sub[s]);
        running_bigram_score += (*duals_)[sub[s]];
        running_pretrigram_score += (*duals_)[sub[s] + GRAMSPLIT];
        if (DEBUG_NONLOCAL) {
          cerr << "(" << (*duals_)[sub[s]] << "/" << (*duals_)[sub[s] + GRAMSPLIT] << ")";
        }
      }
      if (DEBUG_NONLOCAL) {
        cerr << " ] ";
      }
      if (subder_index < subder.size() - 1) { 
        for (int i = 0; i < lat_ids[subder_index + edge_pos].size(); ++i) {
          int lat_id = lat_ids[subder_index + edge_pos][i];
          running_bigram_score += (*duals_)[lat_id];
          running_pretrigram_score += (*duals_)[GRAMSPLIT + lat_id];
          if (DEBUG_NONLOCAL) {
            cerr << lattice_._edge_label_by_nodes[lat_id] << " ";
            cerr << "(" << (*duals_)[lat_id] << "/" 
                 << (*duals_)[lat_id + GRAMSPLIT] << ")";
          }
          tmp_derivation.push_back(lat_id);
        }
      }
    }
    int size = edge.tail_nodes().size();
    if (edge_pos == size - 1) {
      foreach (int lat_id, lat_ids[size]) {
        running_bigram_score += (*duals_)[lat_id];
        running_pretrigram_score += (*duals_)[GRAMSPLIT + lat_id];
        if (DEBUG_NONLOCAL) {
          cerr << lattice_._edge_label_by_nodes[lat_id] << " ";
          cerr << "(" << (*duals_)[lat_id] << "/" 
               << (*duals_)[lat_id + GRAMSPLIT] << ")";
        }
        tmp_derivation.push_back(lat_id);
      }      
    }

    //full_derivation = tmp_derivation;
    
    int first = tmp_derivation.size(), second = 0;

    // Only keep up to and including the second word.
    int words_seen = 0;
    for (int i = 0; i < tmp_derivation.size(); ++i) {
      int lat_id = tmp_derivation[i];
      if (lattice_.is_word(lat_id)) {
        words_seen++;
      }
      if (words_seen == 2) {
        first = i;
        break;
      }
    }
    words_seen = 0;
    for (int i = tmp_derivation.size() - 1; i >= 0; --i) {
      int lat_id = tmp_derivation[i];
      if (lattice_.is_word(lat_id)) {
        words_seen++;
      }
      if (words_seen == 2) {
        second = i;
        break;
      }
    }
    if (second <= first) {
      full_derivation = tmp_derivation;
    } else {
      for (int i = 0; i <= first; ++i) {
        full_derivation.push_back(tmp_derivation[i]);
      }
      for (int i = second; i < tmp_derivation.size(); ++i) {
        full_derivation.push_back(tmp_derivation[i]);
      }
    }

    assert(!fail);
    //cerr << "score " << score << endl;
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
    //cerr << endl << score << " done " <<  endl;
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
      //assert(score < 100000);
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

  //mutable Cache<Hyperedge, vector<vector<vector<int> > > > edge_lattice_cache_;
};

#endif

    /* bool used = false; */
    /* int t[] = {241, 240, 230, 87, 78, 77, 72, 60, 57, 56, 45, 25, 0, 23, 11, 2, 5, 93, 216, 100, 97, 125, 105, 214, 206, 202, 168, 182, 186, 189, 201, 197, 153, 150, 149, 140, 229}; */


    /* for (int i = 0; i < 100; ++i) { */
    /*   if (edge.id() == t[i]) { */
    /*     cerr << "USED" << endl; */
    /*     used = true; */
    /*   } */
    /*   if (t[i] == 229) { */
    /*     break; */
    /*   }  */
    /* } */
    /* if (!used) { */
    /*   full_derivation.push_back(1); */
    /*   signature.push_back(1); */
    /*   score = 1e80; */
    /*   return; */
    /* } */
