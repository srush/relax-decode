#include "CubePruning.h"
#include <iostream>
#include <algorithm>
#include "../common.h"

using namespace std;


void CubePruning::run(const Hypernode &cur_node, 
                      vector<Hyp> &kbest_hyps) {
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
    kbest_hyps.push_back(_non_local.initialize(cur_node));
  }
}

void CubePruning::init_cube(const Hypernode &cur_node, 
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
    Hyp newhyp;
    bool bounded, early_bounded;
    bool b = gethyp(*cedge, newvecj, newhyp, true, &bounded, &early_bounded);
    assert(b || use_bound_);
    
    if (!early_bounded && b) {
      cands.push(new Candidate(newhyp, *cedge, newvecj));
    }
  }
}


void CubePruning::kbest_enum(const Hypernode &node,
                             vector<Hyp> &newhypvec) {
  if (DEBUG_CUBE) {
    cerr << "kbest_enum" << node.label() << endl;
  }
  foreach (HEdge cedge, node.edges()) {
    int position = -1;
    vector<Hyp> *lasthypvec, *curhypvec, *nexthypvec;

    foreach (HNode sub, cedge->tail_nodes()) {
      nexthypvec = new vector<Hyp>();
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
            Hyp newhyp;
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
            Hyp newhyp;
            check_++;
            if (check_ % 10000 == 1) {
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
    foreach(Hyp &hyp, *nexthypvec) {
      newhypvec.push_back(hyp);
      if (newhypvec.size() == _k) {
        break;
      }
    }
    if (newhypvec.size() == _k) {
      fail_ = true;
      // cerr << "Node fail " <<  _k << " " << node.label() << endl;
      // for (int i = 0; i < newhypvec.size(); ++i) {
      //   cerr << "Heuristic: " << newhypvec[i].total_heuristic << endl;
      //   for (int j = 0; j < newhypvec[i].full_derivation.size(); ++j) {
      //     cerr << newhypvec[i].full_derivation[j] << " ";
               
      //   }
      //   cerr << endl;
      // }
      break;
    }
  }

  sort(newhypvec.begin(), newhypvec.end());
  if (DEBUG_CUBE) cerr << "TOTAL: " << newhypvec.size() << endl;
}

void CubePruning::kbest(Candidates &cands, 
                        vector<Hyp> &newhypvec, 
                        bool recombine) {
       
  // The buffer of solutions.
  vector <Hyp> hypvec;
  uint cur_kbest = 0;
    
  // Keep tracks of signatures seen.
  set <Sig> sigs;

  // Overfill the buffer for recombination.
  uint buf_limit = _ratio * _k;
  
  while (cur_kbest < _k &&                       
         !(cands.empty() || hypvec.size() >= buf_limit)) {
    Candidate * cand = cands.top();
    cands.pop();
    const Hyp & chyp  = cand->hyp;
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
    Hyp &item = hypvec[i]; 
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

void CubePruning::next(const Hyperedge &cedge, 
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
        Hyp newhyp;
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


bool CubePruning::gethyp(const Hyperedge &cedge, 
                         const vector<int> &vecj, 
                         Hyp &item, 
                         bool keep_if_bounded, 
                         bool *bounded, 
                         bool *early_bounded) {

  double score = _weights.get_value(cedge);  
  vector<const vector<int> *> subders;
  vector<int> edges;  
  double worst_heuristic = 0.0;

  // Grab the jth best hypothesis at each node of the hyperedge.
  for (uint i=0; i < cedge.num_nodes(); i++) {
    const Hypernode &sub = cedge.tail_node(i); 
    if (vecj[i] >= (int)_hypothesis_cache.get_value(sub).size()) {
      return false;
    }
    Hyp *item = &_hypothesis_cache.get(sub)[vecj[i]];
    if (item->full_derivation.size() == 0) {
      return false;
    }
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
  vector <int> full_derivation;
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
    item = Hyp(score, heuristic_score, sig, full_derivation, edges);
    assert(item.full_derivation.size()!=0);
    (*bounded) = false;
  } else {
    (*bounded) = true;
  }
  return true;
}

bool CubePruning::gethyp_enum(const Hyperedge & cedge, 
                              int pos,
                              bool unary,
                              const Hyp &first,
                              const Hyp &second,
                              Hyp &item, 
                              bool *bounded, 
                              bool *early_bounded) {
  /*
    Return the score and signature of the element obtained from combining the
    vecj-best parses along cedge. Also, apply non-local feature functions (LM)
  */

  double score = 0.0; //_weights.get_value(cedge);  
  double dual_score = 0.0; //dual_scores_->get_value(cedge);  
  double heuristic =
    (*edge_heuristic_->get_value(cedge))[pos].get_score_by_id(0); 
  if (pos == cedge.tail_nodes().size() - 1) {
    score = _weights.get_value(cedge);  
    dual_score = dual_scores_->get_value(cedge);
    heuristic = heuristic_->get_value(cedge.head_node());
  }


  vector<const vector<int> *> subders;
  vector<int> edges;  // grab the jth best hypothesis at each node of the hyperedge
  double worst_heuristic = 0.0;

  if (!unary) {
    subders.push_back(&first.full_derivation);
    subders.push_back(&second.full_derivation);
    
    // Generic times (eventually)
    score += first.score + second.score;
    dual_score += first.score + second.score;
    worst_heuristic = max(first.total_heuristic, second.total_heuristic);
  } else {
    subders.push_back(&first.full_derivation);
    
    // Generic times (eventually)
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
  vector<int> full_derivation;
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
    item = Hyp(score, heuristic_score, sig, full_derivation, edges);
    assert(item.full_derivation.size()!=0);
    (*bounded) = false;
  } else {
    (*bounded) = true;
  }
  return true;
}


