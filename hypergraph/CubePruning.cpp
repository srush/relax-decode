#include "CubePruning.h"
#include <iostream>
#include <algorithm>
#include "../common.h"

using namespace std;

double CubePruning::parse() {
  run(_forest.root(), _hypothesis_cache.store[_forest.root().id()]);
  return _hypothesis_cache.store[_forest.root().id()][0].score;
}

bool CubePruning::has_derivation() {
  return _hypothesis_cache.has_value[_forest.root().id()];
}

void CubePruning::get_derivation(vector<int> &der) {
  der = _hypothesis_cache.store[_forest.root().id()][0].full_derivation;
}

void CubePruning::get_derivation(vector<int> &der, int n) {
  der = _hypothesis_cache.store[_forest.root().id()][n].full_derivation;
}

int CubePruning::get_num_derivations() {
  return _hypothesis_cache.store[_forest.root().id()].size();
}

void CubePruning::get_edges(vector<int> &edges, int n) {
  edges = _hypothesis_cache.store[_forest.root().id()][n].edges;
}

double CubePruning::get_score(int n) {
  return _hypothesis_cache.store[_forest.root().id()][n].score;
}

void CubePruning::run(const Hypernode & cur_node, vector <Hyp> & kbest_hyps) {
  //compute the k-'best' list for cur_node 
  foreach (HEdge hedge, cur_node.edges()) { 
    foreach (HNode sub, hedge->tail_nodes()) {
      if (!_hypothesis_cache.has_key(*sub)) {
        run(*sub, _hypothesis_cache.store[sub->id()]);
        _hypothesis_cache.has_value[sub->id()] = 1;
      }
    }
  }

  //create cube
  if (!cur_node.is_terminal()) {
    Candidates cands;
    init_cube(cur_node, cands);
    if (cur_node.id() == _forest.root().id()) {
      kbest(cands, kbest_hyps, false);
    } else {
      kbest(cands, kbest_hyps, true);
    }
  } else {
    kbest_hyps.push_back(_non_local.initialize(cur_node));
  } 
}

void CubePruning::init_cube(const Hypernode & cur_node, Candidates & cands) {
  cerr << cur_node.label() << endl;
  foreach (HEdge cedge, cur_node.edges()) { 
    
    // start with (0,...0)
    vector <int> newvecj(cedge->num_nodes());
    for (uint j=0; j < newvecj.size(); j++) {
      newvecj[j] = 0;
    }
    
    set <vector <int > > vecset;
    vecset.insert(newvecj);
    //_oldvec.
    _oldvec.set_value(*cedge, vecset);
    
    // add the starting (0,..,0) hypothesis to the heap
    Hyp newhyp;
    bool bounded, early_bounded;
    bool b = gethyp(*cedge, newvecj, newhyp, true, &bounded, &early_bounded);
    //cout << "Get hyp " << newhyp.score << endl;
    assert(b || use_bound_);
    
    if (!bounded) {
      cands.push(new Candidate(newhyp, *cedge, newvecj));
    }
  }
}


void CubePruning::kbest(Candidates & cands, vector <Hyp> & newhypvec, bool recombine) {
  // Algorithm 2, kbest 
       
  // list of best hypvectors (buf)
  vector <Hyp> hypvec;
  
  // number of hypotheses found 
  uint cur_kbest = 0;
    
  // keep tracks of sigs in buffer (don't count multiples twice, since they will be recombined)
  set <Sig> sigs;

  //overfill the buffer since we assume there will be some reordering
  uint buf_limit = _ratio * _k;
  
  while (cur_kbest < _k &&                       
         !(cands.empty() ||                          
           hypvec.size() >= buf_limit)) {
    Candidate * cand = cands.top();
    cands.pop();
    const Hyp & chyp  = cand->hyp;
    const Hyperedge & cedge = cand->edge;
    const vector <int> & cvecj = cand->vec; 
    
    bool bounded = false;
    if (use_bound_ && chyp.total_heuristic > bound_) {
      bounded = true;
    }
    //TODO: duplicate management
    if (!bounded && 
        (!recombine || sigs.find(chyp.sig) == sigs.end())) {
      sigs.insert(chyp.sig);
      cur_kbest += 1;
      cerr << "KBest: " << cur_kbest << endl;
    } else {
      
    }
    
    // add hypothesis to buffer
    if (!bounded) { 
      hypvec.push_back(chyp);
    }
      
    // expand next hypotheses
    next(cedge, cvecj, cands);
    //cout << "CUR CANDIDATES " << cands.size() << endl;
  }  
  //cerr << hypvec.size() << " " << cur_kbest << " " << endl;
  if (!cands.empty()) {
    cout << "====================Fail=====================" << endl;
    fail_ = true;
  }

  /*
  if (cands.empty()) {
    cout << "Out of candidates" << endl;
  }
  if ( cur_kbest >= _k) {
    cout << "Cur best limit" << endl;
  }
  if (hypvec.size() >= buf_limit) {
    
    cout << "Buf limit " << hypvec.size() <<" " << buf_limit <<  endl; 
  }

  cout << hypvec.size() << endl;
  */  
  // RECOMBINATION (shrink buf to actual k-best list)
  
  // Sort and combine hypevec  
  assert(cur_kbest || use_bound_);
  assert(hypvec.size() || use_bound_);
  sort(hypvec.begin(), hypvec.end());
  
  map <Sig, int> keylist;
  
  for (uint i=0; i < hypvec.size(); i++) {
    Hyp item = hypvec[i]; 
    assert(i == 0 || item.score >= hypvec[i-1].score); 

    map<Sig, int>::iterator f = keylist.find(item.sig);
    if (!recombine || f == keylist.end()) {
      //cout << "miss" << endl;
      keylist[item.sig] = newhypvec.size();
      newhypvec.push_back(item);
      
      if (newhypvec.size() >= _k) {
        break;
      }
    }
  }    
  assert(newhypvec.size() || use_bound_);
}

void CubePruning::next(const Hyperedge & cedge, const vector <int > & cvecj, Candidates & cands){
  /*
    @param cedge - the edge that we just took a candidate from
    @param cvecj - the current position on the cedge cube
    @param cands - current candidate list 
  */
  // for each dimension of the cube
  
 
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
        bool succeed = gethyp(cedge, newvecj, newhyp, false, &bounded, &early_bounded);
        if (succeed) {
          // Add j'th dimension to the cube
          _oldvec.store[cedge.id()].insert(newvecj);
          if (early_bounded) {
            break;
          }
          if (!bounded) {
            cands.push(new Candidate(newhyp, cedge, newvecj));
            break;
          }
        } else {
          break;
        }
      } else {
        break;
      } 
    }
  }
}


bool CubePruning::gethyp(const Hyperedge & cedge, const vector <int> & vecj, Hyp & item, bool keep_if_bounded, bool *bounded, bool *early_bounded) {
  /*
    Return the score and signature of the element obtained from combining the
    vecj-best parses along cedge. Also, apply non-local feature functions (LM)
  */

  double score = _weights.get_value(cedge);  
  double dual_score = dual_scores_->get_value(cedge);  
  vector<vector <int> > subders;
  vector<int> edges;  // grab the jth best hypothesis at each node of the hyperedge
  double worst_heuristic = 0.0;
  for (uint i=0; i < cedge.num_nodes(); i++) {
    const Hypernode & sub = cedge.tail_node(i); 
    if (vecj[i] >= (int)_hypothesis_cache.get_value(sub).size()) {
      return false;
    }
    Hyp item = _hypothesis_cache.get_value(sub)[vecj[i]];
    if (item.full_derivation.size() == 0) {
      return false;
    }
    //assert (item.full_derivation.size() != 0);   
    subders.push_back(item.full_derivation);
    for (uint j = 0; j < item.edges.size(); ++j) {
      edges.push_back(item.edges[j]);
    }
    
    // Generic times (eventually)
    score = score + item.score;
    dual_score += item.score;
    worst_heuristic = max(item.total_heuristic, worst_heuristic);
  }

  double heuristic = 0.0;
  (*early_bounded) = false; 
  if (use_bound_ && use_heuristic_ && !keep_if_bounded) {
    heuristic = heuristic_->get_default(cedge.head_node(), 0.0);
    double early_heuristic_score = heuristic + dual_score;
    cerr << "EarlyHeuristic: " <<  cedge.id() << " " << heuristic << " " << early_heuristic_score << " " << worst_heuristic << " " << bound_ << endl;
    if (early_heuristic_score > bound_) {
      (*bounded) = true;
      (*early_bounded) = true; 
      return true;
    }
  }

  // Get the non-local feature and signature information
  vector <int> full_derivation;
  Sig sig; 
  double non_local_score;
  _non_local.compute(cedge, subders, non_local_score, full_derivation, sig);
  score = score + non_local_score;

  if (use_heuristic_) {
    heuristic = heuristic_->get_default(cedge.head_node(), 0.0);
  }
  double heuristic_score = score + heuristic;
  cerr << "Heuristic: " <<  heuristic_score << " " << bound_ << endl;
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


