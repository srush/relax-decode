#include "CubePruning.h"
#include <iostream>
#include <algorithm>
using namespace std;



//typedef priority_queue< const Candidate * > Candidates;

double CubePruning::parse() {
  run(_forest.root(),  _hypothesis_cache.store[_forest.root().id()]);
  return _hypothesis_cache.store[_forest.root().id()][0].score;
  //cout << _hypothesis_cache.store[_forest.root().id()][0].score << endl;
  //cout << _hypothesis_cache.store[_forest.root().id()][1].score << endl;
  //cout << _hypothesis_cache.store[_forest.root().id()][2].score << endl;
}

void CubePruning::get_derivation(vector <int> & der) {
  der = _hypothesis_cache.store[_forest.root().id()][0].full_derivation;
}

void CubePruning::run(const ForestNode & cur_node, vector <Hyp> & kbest_hyps) {
  //compute the k-'best' list for cur_node 
  for (int i =0; i < cur_node.num_edges(); i++) {
    const ForestEdge & hedge = cur_node.edge(i); 
    for (int j=0; j < hedge.num_nodes();j++) {
      const ForestNode & sub = hedge.tail_node(j);
      if (!_hypothesis_cache.has_key(sub)) {
        run(sub, _hypothesis_cache.store[sub.id()]);
        _hypothesis_cache.has_value[sub.id()] = 1;
      }
    }
  }

  //create cube
  if (!cur_node.is_word()) {
    Candidates cands;
    //cout << "Starting cube" << endl;
    init_cube(cur_node, cands);
  
    //heapq.heapify(cands);
    
    // gen kbest
    //vector<Hyp> kbest_hyp;
    //cout << "DOING NODE: " << cur_node.id() <<endl; 

    kbest(cands, kbest_hyps);
    //cout << kbest_hyps.size() << endl;
    //cout << "SIZE " << cur_node.id() << " " << kbest_hyps.size() << endl;
  } else {
    //vector <int> * p = new vector <int> ();
    //vector<int> n;
    //n.push_back(_non_local.initialize(cur_node));
    //vector <int> sig;
    //sig.push_back(cur_node.id());
    kbest_hyps.push_back(_non_local.initialize(cur_node));
    //cout << "Word " << endl;
  }
 
  //print cur_node
  //print map(str,self.hypothesis_cache[cur_node])

  //return kbest_hyps;       
}

void CubePruning::init_cube(const ForestNode & cur_node, Candidates & cands) {
  for (int i=0; i < cur_node.num_edges(); i++) {
    const ForestEdge & cedge = cur_node.edge(i);
                  
    // start with (0,...0)
    vector <int> newvecj(cedge.num_nodes());
    for (int j=0; j < newvecj.size();j++) {
      newvecj[j] = 0;
    }
    
    set <vector <int > > vecset;
    vecset.insert(newvecj);
    //_oldvec.
    _oldvec.set_value(cedge, vecset);
    
    // add the starting (0,..,0) hypothesis to the heap
    Hyp newhyp;
    bool b = gethyp(cedge, newvecj, newhyp);
    //cout << "Get hyp " << newhyp.score << endl;
    assert(b);
    cands.push(new Candidate(newhyp, cedge, newvecj));
  }
}


void CubePruning::kbest(Candidates & cands, vector <Hyp> & newhypvec) {
  // Algorithm 2, kbest 
       
  // list of best hypvectors (buf)
  vector <Hyp> hypvec;
  
  // number of hypotheses found 
  int cur_kbest = 0;
    
  // keep tracks of sigs in buffer (don't count multiples twice, since they will be recombined)
  set <Sig> sigs;

  //overfill the buffer since we assume there will be some reordering
  int buf_limit = _ratio * _k;
  
  while (cur_kbest < _k &&                       
         ! (cands.empty() ||                          
            hypvec.size() >= buf_limit)) {
    //cout << buf_limit << " " << cands.size() << endl;
    Candidate * cand = cands.top();
    cands.pop();
    const Hyp & chyp  = cand->hyp;
    const ForestEdge & cedge = cand->edge;
    const vector <int> & cvecj = cand->vec; 


    //cout << "Init vec ";
    //for (int p=0; p < cvecj.size();p++) {
    //cout << cvecj[p] << " ";
      //assert(!cvecj[p]);
    //}
    //cout << endl;


    //TODO: duplicate management
    //cout << "SIG: ";
    //for (int p=0; p < chyp.sig.size();p++)
      //cout << chyp.sig[p] << " ";
      //cout << endl;
    if (sigs.find(chyp.sig) == sigs.end()) {
      sigs.insert(chyp.sig);
      cur_kbest += 1;
      
      //cout << cur_kbest << endl;
    } else {
      
    }
    
    //cout << chyp.sig << " " << chyp.score << endl;
    // add hypothesis to buffer
    hypvec.push_back(chyp);
      
    // expand next hypotheses
    next(cedge, cvecj, cands);
    //cout << "CUR CANDIDATES " << cands.size() << endl;
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
  
  // sort and combine hypevec
  
  assert(cur_kbest);
  assert(hypvec.size());
  sort(hypvec.begin(), hypvec.end());
  
  map <Sig, int> keylist;
  
  //vector <Hyp> newhypvec;
  
  for (int i=0; i < hypvec.size(); i++) {
    Hyp item = hypvec[i]; 
    assert(i == 0 || item.score >= hypvec[i-1].score); 
    //cout << item.score << " " << endl; 
    
    //for (int p=0; p < item.sig.size();p++) {
    //cout << item.sig[p] << " ";
    //}
    //cout << endl;

    map<Sig, int>::iterator f = keylist.find(item.sig);
    if (f == keylist.end()) {
      //cout << "miss" << endl;
      keylist[item.sig] = newhypvec.size();
      
      //for (int p=0; p < item.full_derivation.size();p++) {
      //cout << item.full_derivation[p] << " ";
      //}
      //cout << item.score;
      //cout << endl;

      newhypvec.push_back(item);
      
      if (newhypvec.size() >= _k) {
        break;
      }
    }
    else {
      int pos = keylist[item.sig];
      //semiring plus
      //newhypvec[pos].add(item);
    }
  }     
  assert(newhypvec.size());


  
  //return newhypvec;
}

void CubePruning::next(const ForestEdge & cedge, const vector <int > & cvecj, Candidates & cands){
  /*
    @param cedge - the edge that we just took a candidate from
    @param cvecj - the current position on the cedge cube
    @param cands - current candidate list 
  */
  // for each dimension of the cube
  //cout << "Cur vec ";
  
  assert(cvecj.size() == cedge.num_nodes());

  for (int i=0; i < cedge.num_nodes(); i++) {
    // vecj' = vecj + b^i (just change the i^th dimension
    vector <int> newvecj(cvecj);
    newvecj[i] += 1;
  
    //for (int p=0; p < cvecj.size();p++) {
    //cout << newvecj[p] << " ";
    //}
    //cout << endl;

    //newvecj = cvecj[:i] + (cvecj[i]+1,) + cvecj[i+1:];
    
    set <vector <int> > & vecs = _oldvec.store[cedge.id()];
    if (vecs.find(newvecj)==vecs.end()) {
      Hyp newhyp;
      if (gethyp(cedge, newvecj, newhyp)){
        // add j'th dimension to the cube
        _oldvec.store[cedge.id()].insert(newvecj);
        //cout << "INSERTING NEW" << endl;
        int orig = cands.size();
        
        cands.push(new Candidate(newhyp, cedge, newvecj));
        assert(cands.size() != orig);
      } else {
        //cout << "no get" << endl;
      }
      } 
    //else {
      //cout << "seen" << endl;
      //}
  }
}


bool CubePruning::gethyp(const ForestEdge & cedge, const vector <int> & vecj, Hyp & item) {
  /*
    Return the score and signature of the element obtained from combining the
    vecj-best parses along cedge. Also, apply non-local feature functions (LM)
  */

  //cout << "ENTER " << endl;
  double score = _weights.get_value(cedge);
  
  vector <vector <int> > subders;

  // grab the jth best hypothesis at each node of the hyperedge
  //cout << cedge.num_nodes() << endl;
  for (int i=0; i < cedge.num_nodes(); i++) {
    const ForestNode & sub = cedge.tail_node(i);
 
    if (vecj[i] >= _hypothesis_cache.get_value(sub).size()) {
      //cout << "FAIL for size " << _hypothesis_cache.get_value(sub).size();
      return false;
    }


  
    Hyp item = _hypothesis_cache.get_value(sub)[vecj[i]];

    //cout << "ITEM FULL DER: ";
    assert (item.full_derivation.size() != 0);
    //for (int p=0; p < item.full_derivation.size();p++)
      //cout << item.full_derivation[p] << " ";
      //cout << endl;
   
    subders.push_back(item.full_derivation);
    // generic times (eventually)
    score = score + item.score;
  }

  // Get the non-local feature and signature information
  vector <int> full_derivation;
  Sig sig; 
  double non_local_score;
  _non_local.compute(cedge, subders, non_local_score, full_derivation, sig);
  //cout << " NON LOCAL SCORE " << non_local_score << endl;;
  score = score + non_local_score;

  //cout << "LOWER SIG: ";
  //for (int p=0; p < sig.size();p++)
    //cout << sig[p] << " ";
    //cout << endl;

    //cout << "FULL DER: ";
    //for (int p=0; p < full_derivation.size();p++)
    //cout << full_derivation[p] << " ";
    //cout << endl;

  //item = Hyp(score, full_derivation, cedge, vecj, sig);

  item = Hyp(score,  sig, full_derivation);
  /*for (int p=0; p < item.full_derivation.size();p++) {
    cout << item.full_derivation[p] << " ";
  }
  cout << item.score;
  cout << endl;
  */
  assert(item.full_derivation.size()!=0);
  return true;
}


