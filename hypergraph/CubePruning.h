#ifndef CUBEPRUNING_H_
#define CUBEPRUNING_H_

#include <set>
#include "Hypergraph.h"
#include "EdgeCache.h"
#include <queue>
#include "svector.hpp"
#include "AStar.h"
using namespace Scarab::HG;
using namespace std;
// Some non local feature scorer

#define DEBUG_CUBE 0
typedef vector<int> Sig;

template<class TDerivation>
struct Hyp {
public:
  Hyp(){}

Hyp(double score_in,
    double heuristic_in,
    Sig sig_in,
    TDerivation full_der,
    const vector<int> &edges_)
  : score(score_in),
    total_heuristic(heuristic_in),
    sig(sig_in),
    full_derivation(full_der),
    edges(edges_){

  }

  // The inside score of the hypothesis.
  double score;

  // The inside score + outside heuristic of the hypothesis.
  double total_heuristic;

  // A signature indicating the current fsa states of the hypothesis.
  Sig sig;

  // The representative full derivation of the hypothesis.
  TDerivation full_derivation;

  vector<int> edges;

  // Comparison operator, chooses lower inside score.
  bool operator<(const Hyp & other) const {
    //return total_heuristic < other.total_heuristic;
    return score < other.score;
  }
};

// Interface for non-local scoring functions. Can be thought of
// as moving dual values to primal values.
template<class TDerivation>
class NonLocal  {
 public:

  //virtual ~NonLocal() {};
  // Compute the non-local score by combining the sub_ders.
  virtual bool compute(const Hyperedge &edge,
                       int edge_pos,
                       double bound,
                       const vector<const TDerivation *> &sub_ders,
                       double &score,
                       TDerivation &full_derivation,
                       Sig &sig) const = 0;

  // Initialize a hypothesis for a hypernode.
  virtual Hyp<TDerivation> initialize(const Hypernode &) const =0;
};


// A trivial instantiation of non-local.
template<class TDerivation>
class BlankNonLocal: public NonLocal<TDerivation> {
 public:
  BlankNonLocal() {}

  bool compute(const Hyperedge &edge,
               int edge_pos,
               double,
               const vector<const TDerivation *> &subder,
               double &score,
               vector<int>  &full_derivation,
               Sig &sig
               ) const {
    score = 0.0;
    sig.push_back(edge.id());
  }

  virtual Hyp<TDerivation> initialize(const Hypernode &node) const {
    return Hyp<TDerivation>(0.0, 0.0, vector<int>(), vector<int>(), vector<int>());
  }
};


template <class TDerivation>
class TCubePruning {
  // A candidate.
  struct Candidate {
   Candidate(Hyp<TDerivation> h, const Hyperedge &e,  const vector<int> &v)
   : hyp(h),
      edge(e),
      vec(v) {}

    // The current hypothesis.
    Hyp<TDerivation> hyp;

    // The associated edge.
    const Hyperedge &edge;

    // The back pointers to previous candidates.
    vector <int> vec;

    bool operator<(const Candidate & other) const {
      return hyp < other.hyp;
    }
  };

  struct candidate_compare :
      public std::binary_function<Candidate*, Candidate*, bool> {
    bool operator()(const Candidate * a, const Candidate * b) const {
      return (*b) <  (* a) ;
    }
  };

  typedef priority_queue <Candidate *, vector<Candidate*>, candidate_compare> Candidates;

public:
 TCubePruning(const HGraph & forest,
              const Cache <Hyperedge, double> & weights,
              const NonLocal<TDerivation> & non_local,
              int k,
              int ratio)
     : _forest(forest),
    _weights(weights),
      _non_local(non_local),
      _k(k),
      _ratio(ratio),
      _hypothesis_cache(forest.num_nodes()), _oldvec(forest.num_edges()),
      use_bound_(false),
      use_heuristic_(false),
      fail_(false) {
    check_ = 0;
    check_bounded_ = 0;
    cube_enum_ = false;
  }


  int get_num_derivations() {
    return _hypothesis_cache.store[_forest.root().id()].size();
  }

  void get_derivation(TDerivation &der, int n) {
    der = _hypothesis_cache.store[_forest.root().id()][n].full_derivation;
  }

  void get_derivation(TDerivation &der) { get_derivation(der, 0); }


  bool has_derivation() {
    return _hypothesis_cache.has_value[_forest.root().id()];
  }

  void get_edges(vector<int> &edges, int n) {
    edges = _hypothesis_cache.store[_forest.root().id()][n].edges;
  }

  double get_score(int n) {
    return _hypothesis_cache.store[_forest.root().id()][n].score;
  }

  double parse(bool *success) {
    run(_forest.root(), _hypothesis_cache.store[_forest.root().id()]);
    if (_hypothesis_cache.store[_forest.root().id()].size() > 0) {
      *success = true;
      return _hypothesis_cache.store[_forest.root().id()][0].score;
    }
    cerr << "FAIL no result" << endl;
    *success = false;
    return 0.0;

  }

  void set_duals(const Cache<Hyperedge, double> *dual_scores) {
    dual_scores_ = dual_scores;
  }

  void set_bound(double bound) {
    use_bound_ = true;
    bound_ = bound;
  }

  void set_heuristic(const Cache<Hypernode, double> *heuristic) {
    use_heuristic_ = true;
    heuristic_ = heuristic;
  }

  void set_edge_heuristic(const Cache <Hyperedge, vector<BestHyp> *> *heuristic) {
    edge_heuristic_ = heuristic;
  }

  void set_cube_enum() {
    cube_enum_ = true;
  }

  bool is_exact() { return !fail_; }

 private:

  /* void run(const Hypernode & cur_node, */
  /*          vector <Hyp> & kbest_hyps); */

  /* void init_cube(const Hypernode & cur_node, */
  /*                Candidates &cands); */

  /* // Implementation of Chiang and Huang, k-best algorithm 2. */
  /* void kbest(Candidates & cands, */
  /*            vector <Hyp> &, */
  /*            bool recombine); */

  /* // @param cedge - the edge that we just took a candidate from */
  /* // @param cvecj - the current position on the cedge cube */
  /* // @param cands - current candidate list */
  /* // for each dimension of the cube */
  /* void next(const Hyperedge & cedge, */
  /*           const vector <int > & cvecj, */
  /*           Candidates & cands); */


  /* // Return the score and signature of the element obtained from combining the */
  /* // vecj-best parses along cedge. Also, apply non-local feature functions (LM) */

  /* bool gethyp(const Hyperedge & cedge, */
  /*             const vector <int> & vecj, */
  /*             Hyp & item, bool, */
  /*             bool *bounded, */
  /*             bool *early_bounded); */

  /* void kbest_enum(const Hypernode &node, */
  /*                 vector <Hyp> &newhypvec); */

  /* bool gethyp_enum(const Hyperedge & cedge, */
  /*                  int pos, */
  /*                  bool unary, */
  /*                  const Hyp &first, */
  /*                  const Hyp &second, */
  /*                  Hyp &item, */
  /*                  bool *bounded, */
  /*                  bool *early_bounded); */

  void run(const Hypernode &cur_node,
           vector<Hyp<TDerivation> > &kbest_hyps) {
    // Compute the k-best list for cur_node.
    foreach (HEdge hedge, cur_node.edges()) {
      foreach (HNode sub, hedge->tail_nodes()) {
        if (!_hypothesis_cache.has_key(*sub)) {
          run(*sub, _hypothesis_cache.store[sub->id()]);
          _hypothesis_cache.has_value[sub->id()] = 1;
        }
      }
    }

    // Create cube.
    if (!cur_node.is_terminal()) {
      Candidates cands;
      if (cube_enum_) {
        kbest_enum(cur_node, kbest_hyps);
      } else {
        init_cube(cur_node, cands);
        if (cur_node.id() == _forest.root().id()) {
          kbest(cands, kbest_hyps, false);
        } else {
          //
          kbest(cands, kbest_hyps, true);
        }
      }
    } else {

      Hyp<TDerivation> hyp = _non_local.initialize(cur_node);
      if (use_heuristic_) {
        double heu = heuristic_->get_default(cur_node, 0.0);
        hyp.total_heuristic = hyp.score + heu;
      }
      kbest_hyps.push_back(hyp);
    }
  }

  void init_cube(const Hypernode &cur_node,
                 Candidates &cands) {
    if (DEBUG_CUBE) {
      cerr << "NODE "
           << cur_node.id() << " " << cur_node.label() << endl;
    }
    foreach (HEdge cedge, cur_node.edges()) {
      if (DEBUG_CUBE) {
        cerr << "initing edge " << cedge->id() << " "
             << cedge->label() << endl;
        foreach (HNode node, cedge->tail_nodes()) {
          cerr << node->id() << " ";
        }
        cerr << endl;
      }

      // Start with derivation vector (0,...0).
      vector<int> newvecj(cedge->num_nodes(), 0);
      set<vector <int> > vecset;
      vecset.insert(newvecj);
      _oldvec.set_value(*cedge, vecset);

      // Add the starting (0,..,0) hypothesis to the heap.
      Hyp<TDerivation> newhyp;
      bool bounded, early_bounded;
      bool b = gethyp(*cedge, newvecj, newhyp, true, &bounded, &early_bounded);
      assert(b || use_bound_);

      if (!early_bounded && b) {
        cands.push(new Candidate(newhyp, *cedge, newvecj));
      }
    }
  }


  void kbest_enum(const Hypernode &node,
                  vector<Hyp<TDerivation> > &newhypvec) {
    if (DEBUG_CUBE) {
      cerr << "kbest_enum" << node.label() << endl;
    }
    foreach (HEdge cedge, node.edges()) {
      int position = -1;
      vector<Hyp<TDerivation> > *lasthypvec, *curhypvec, *nexthypvec;

      foreach (HNode sub, cedge->tail_nodes()) {
        nexthypvec = new vector<Hyp<TDerivation> >();
        if (DEBUG_CUBE) cerr << sub->label() << endl;
        position++;
        if (position == 0) {
          if (cedge->tail_nodes().size() > 1) {
            lasthypvec = &_hypothesis_cache.store[sub->id()];
          } else {
            lasthypvec = &_hypothesis_cache.store[sub->id()];
             if (DEBUG_CUBE) {
              cerr << position << " " << lasthypvec->size() << " "
                   << sub->label() << endl;
             }
            for (int i = 0; i < lasthypvec->size(); ++i) {
              bool bounded, early_bounded;
              bool succeed;
              Hyp<TDerivation> newhyp;
              succeed = gethyp_enum(*cedge, position,
                                    true,
                                    (*lasthypvec)[i],
                                    (*lasthypvec)[i],
                                    newhyp,
                                    &bounded,
                                    &early_bounded);
              if (early_bounded) {
                break;
              } else if (!bounded) {
                nexthypvec->push_back(newhyp);
              }
            }
          }
        } else {
          curhypvec = &_hypothesis_cache.store[sub->id()];
          if (DEBUG_CUBE) {
            cerr << position << " " << lasthypvec->size() << " "
                 << curhypvec->size() << " " << sub->label() << endl;
          }
          for (int i = 0; i < lasthypvec->size(); ++i) {
            int j = 0;
            for (j = 0; j < curhypvec->size(); ++j) {
              if (DEBUG_CUBE) {
                cerr << "Getting hyp: "
                     << i << " " << j << endl;
              }
              bool bounded, early_bounded;
              bool succeed;
              Hyp<TDerivation> newhyp;
              check_++;
              if (check_ % 10000 == 1) {
                /* if (check_ > 5000000) { */
                /*   fail_ = true; */
                /*   return; */
                /* } else if (check_ > 1000000 && fail_ == true) { */
                /*   return; */
                /* } */
                cerr << "SEEN " << check_ << " " << check_bounded_ << endl;
              }
              succeed = gethyp_enum(*cedge, position,
                                    false,
                                    (*lasthypvec)[i], (*curhypvec)[j],
                                    newhyp,
                                    &bounded,
                                    &early_bounded);
              if (bounded) check_bounded_++;
              if (early_bounded) {
                break;
              } else if (!bounded) {
                nexthypvec->push_back(newhyp);
              }
            }
            if (j == 0) break;
          }
          lasthypvec = nexthypvec;
          //curhypvec = nexthypvec;
        }
      }
      foreach(Hyp<TDerivation> &hyp, *nexthypvec) {
        newhypvec.push_back(hyp);
        if (newhypvec.size() == _k) {
          break;
        }
      }
      if (newhypvec.size() == _k) {
        fail_ = true;
        cerr << "FAILED " << newhypvec.size() << " " << _k << endl;
        // cerr << "Node fail " <<  _k << " " << node.label() << endl;
        for (int i = 0; i < newhypvec.size(); ++i) {
          cerr << "Heuristic: " << newhypvec[i].total_heuristic << endl;
          //for (int j = 0; j < newhypvec[i].full_derivation.size(); ++j) {
          //cerr << newhypvec[i].full_derivation[j] << " ";
          //}
          //cerr << endl;
        }
        break;
      }
    }

    sort(newhypvec.begin(), newhypvec.end());
    if (DEBUG_CUBE)
      cerr << "TOTAL: " << newhypvec.size() << endl;
  }

  void kbest(Candidates &cands,
             vector<Hyp<TDerivation> > &newhypvec,
             bool recombine) {

    // The buffer of solutions.
    vector<Hyp<TDerivation> > hypvec;
    uint cur_kbest = 0;

    // Keep tracks of signatures seen.
    set <Sig> sigs;

    // Overfill the buffer for recombination.
    uint buf_limit = _ratio * _k;

    while (cur_kbest < _k &&
           !(cands.empty() || hypvec.size() >= buf_limit)) {
      Candidate * cand = cands.top();
      cands.pop();
      const Hyp<TDerivation> & chyp  = cand->hyp;
      const Hyperedge & cedge = cand->edge;
      const vector <int> & cvecj = cand->vec;

      //TODO: duplicate management
      if (!recombine || sigs.find(chyp.sig) == sigs.end()) {
        if (recombine) sigs.insert(chyp.sig);
        cur_kbest += 1;
      }
      hypvec.push_back(chyp);

      // Expand the next hypotheses.
      next(cedge, cvecj, cands);
    }

    // RECOMBINATION (shrink buf to actual k-best list)

    // Sort and combine hypvec.
    assert(cur_kbest);
    assert(hypvec.size());

    sort(hypvec.begin(), hypvec.end());

    map <Sig, int> keylist;
    for (uint i=0; i < hypvec.size(); i++) {
      Hyp<TDerivation> &item = hypvec[i];
      assert(i == 0 || item.score >= hypvec[i-1].score);

      map<Sig, int>::iterator f = keylist.find(item.sig);
      if (!recombine || f == keylist.end()) {
        if (DEBUG_CUBE) {
          cerr << "Solution: " << " " << i
               << " " << item.score << " " << item.total_heuristic << endl;
        }

        keylist[item.sig] = newhypvec.size();
        newhypvec.push_back(item);
        if (newhypvec.size() >= _k) {
          break;
        }
      }
    }
    assert(newhypvec.size());
  }

  void next(const Hyperedge &cedge,
            const vector<int> &cvecj,
            Candidates &cands) {
    assert(cvecj.size() == cedge.num_nodes());

    for (uint i=0; i < cedge.num_nodes(); i++) {
      // vecj' = vecj + b^i (just change the i^th dimension
      vector <int> newvecj(cvecj);

      set <vector <int> > & vecs = _oldvec.store[cedge.id()];

      while (true) {
        newvecj[i] += 1;
        // cerr << "vec" << endl;
        // for (int k = 0; k < newvecj.size(); ++k) {
        //   cerr << newvecj[k] << " ";
        // }
        // cerr << endl;
        if (vecs.find(newvecj) == vecs.end()) {
          Hyp<TDerivation> newhyp;
          bool bounded, early_bounded;
          bool succeed = gethyp(cedge, newvecj, newhyp, true, &bounded, &early_bounded);
          if (succeed) {
            // Add j'th dimension to the cube
            _oldvec.store[cedge.id()].insert(newvecj);
            if (early_bounded) {
              break;
            }
            //if (!bounded) {
            cands.push(new Candidate(newhyp, cedge, newvecj));
            break;
            //}
          } else {
            break;
          }
        } else {
          break;
        }
      }
    }
  }

  bool gethyp(const Hyperedge &cedge,
              const vector<int> &vecj,
              Hyp<TDerivation> &item,
              bool keep_if_bounded,
              bool *bounded,
              bool *early_bounded) {

    double score = _weights.get_value(cedge);
    vector<const TDerivation *> subders;
    vector<int> edges;
    double worst_heuristic = 0.0;

    // Grab the jth best hypothesis at each node of the hyperedge.
    for (uint i=0; i < cedge.num_nodes(); i++) {
      const Hypernode &sub = cedge.tail_node(i);
      if (vecj[i] >= (int)_hypothesis_cache.get_value(sub).size()) {
        return false;
      }
      Hyp<TDerivation> *item = &_hypothesis_cache.get(sub)[vecj[i]];
      /* if (item->full_derivation.size() == 0) { */
      /*   return false; */
      /* } */
      //assert (item.full_derivation.size() != 0);
      subders.push_back(&item->full_derivation);
      for (uint j = 0; j < item->edges.size(); ++j) {
        edges.push_back(item->edges[j]);
      }
      // "Times"
      score = score + item->score;
      if (DEBUG_CUBE) cerr << item->score << " " << vecj[i]  << endl;
      //dual_score += item.score;
      worst_heuristic = max(item->total_heuristic, worst_heuristic);
    }

    double heuristic = 0.0;
    (*early_bounded) = false;

    // Get the non-local feature and signature information
    TDerivation full_derivation;
    Sig sig;
    double non_local_score;
    _non_local.compute(cedge, 0, 1e8, subders, non_local_score,
                       full_derivation, sig);
    score = score + non_local_score;

    if (use_heuristic_) {
      heuristic = heuristic_->get_default(cedge.head_node(), 0.0);
    }
    double heuristic_score = score + heuristic;
    if (!use_bound_ || heuristic_score <= bound_ || keep_if_bounded) {
      edges.push_back(cedge.id());
      item = Hyp<TDerivation>(score, heuristic_score, sig, full_derivation, edges);
      //assert(item.full_derivation.size()!=0);
      (*bounded) = false;
    } else {
      (*bounded) = true;
    }
    return true;
  }

  bool gethyp_enum(const Hyperedge & cedge,
                   int pos,
                   bool unary,
                   const Hyp<TDerivation> &first,
                   const Hyp<TDerivation> &second,
                   Hyp<TDerivation> &item,
                   bool *bounded,
                   bool *early_bounded) {
    double score = 0.0;
    double dual_score = 0.0;
    double heuristic =
        (*edge_heuristic_->get_value(cedge))[pos].get_score_by_id(0);
    if (pos == cedge.tail_nodes().size() - 1) {
      score = _weights.get_value(cedge);
      dual_score = dual_scores_->get_value(cedge);
      heuristic = heuristic_->get_value(cedge.head_node());
    }

    vector<const TDerivation *> subders(2);
    vector<int> edges;
    double worst_heuristic = 0.0;

    if (!unary) {
      subders[0] = &first.full_derivation;
      subders[1] = &second.full_derivation;

      score += first.score + second.score;
      dual_score += first.score + second.score;
      //cerr << first.total_heuristic << " " << second.total_heuristic << endl;
      worst_heuristic = max(first.total_heuristic, second.total_heuristic);
    } else {
      subders.resize(1);
      subders[0] = &first.full_derivation;
      score += first.score;
      dual_score += first.score;
      worst_heuristic = first.total_heuristic;
    }

    (*early_bounded) = false;
    if (use_bound_ && use_heuristic_) {
      double early_heuristic_score = heuristic + dual_score;
      if (DEBUG_CUBE) {
        cerr << "EarlyHeuristic: " <<  dual_score << " " << cedge.id() << " " <<
            heuristic << " " << early_heuristic_score << " " <<
            worst_heuristic << " " << bound_ << endl;
      }
      assert(early_heuristic_score >= worst_heuristic - 0.01);
      if (early_heuristic_score > bound_) {
        (*bounded) = true;
        (*early_bounded) = true;
        return true;
      }
    }

    // Get the non-local feature and signature information
    TDerivation full_derivation;
    Sig sig;
    double non_local_score;
    if (DEBUG_CUBE) cerr << "Non Local " << pos << " " << endl;
    double score_bound = bound_ - heuristic - score;
    bool not_bounded = _non_local.compute(cedge, pos, score_bound, subders, non_local_score, full_derivation, sig);
    score = score + non_local_score;

    double heuristic_score = score + heuristic;
    if (DEBUG_CUBE) {
      cerr << "Heuristic: " << score << " " << heuristic_score << " " << bound_ << endl;
    }
    if (!use_bound_ || (heuristic_score <= bound_ && not_bounded)) {
      edges.push_back(cedge.id());
      item = Hyp<TDerivation>(score, heuristic_score, sig, full_derivation, edges);
      //assert(item.full_derivation.size()!=0);
      (*bounded) = false;
    } else {
      (*bounded) = true;
    }
    return true;
  }


  double bound_;
  bool use_bound_;

  const Cache<Hypernode, double>  *heuristic_;
  const Cache <Hyperedge, vector<BestHyp> * > *edge_heuristic_;
  bool use_heuristic_;

  const Cache <Hyperedge, double>  *dual_scores_;

  const HGraph & _forest;
  const Cache <Hyperedge, double>  & _weights;
  const NonLocal<TDerivation> & _non_local;
  const uint _k;
  const uint _ratio;

  int check_;
  int check_bounded_;

  Cache<Hypernode, vector<Hyp<TDerivation> > > _hypothesis_cache;
  Cache<Hyperedge, set<vector <int> > > _oldvec;

  bool fail_;
  bool cube_enum_;
};



#endif
