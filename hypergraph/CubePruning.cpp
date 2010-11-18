#include <priority_queue>

using namespace std;



struct Candidate {
  Candidate(const Hyp &h, const ForestEdge &  e, const vector <int> & v) 
    : hyp(h), edge(e), vec(v){}
  const Hyp & hyp;
  const ForestEdge & edge;
  const vector <int> & vec;
}

typedef priority_queue <Candidate> Candidates;


void CubePruningrun(const & ForestNode cur_node) {
  //compute the k-'best' list for cur_node 
  for (int i =0; i < cur_node.num_edges(); i++) {
    const ForestEdge & hedge = cur_node.edge(i); 
    for (int j=0; j < hedge.num_tail_nodes();j++) {
      const ForestNode & sub = hedge.tail_node(j);
      if (!_hypothesis_cache.has_key(sub)){
        run(sub);
      }
    }
  }

  //create cube
  Candidate cands;
  init_cube(cur_node, cands);
  //heapq.heapify(cands);
    
  // gen kbest
  _hypothesis_cache.set_value(cur_node, kbest(cands));
  //print cur_node
  //print map(str,self.hypothesis_cache[cur_node])

  return _hypothesis_cache.get_value(cur_node);       
}

void init_cube(const ForestNode & cur_node, Candidates & cands) {
  for (int i=0; i < cur_node.num_edges(); i++) {
    const ForestEdge & cedge = cur_node.edge(i);
    
    cedge.oldvecs = set();
              
    // start with (0,...0)
    vector <int> newvecj(cedge.num_tail_nodes());
    for (int j=0; j < newvecj.size();j++) {
      newvecj[j] = 0;
    }
    

    cedge.oldvecs.add(newvecj);
    
    // add the starting (0,..,0) hypothesis to the heap
    newhyp = gethyp(cedge, newvecj);
    cands.add(Candidate(newhyp, cedge, newvecj));
  }
}


def kbest(Candidates cands) {
  // Algorithm 2, kbest 
       
  // list of best hypvectors (buf)
  vector <Hyp> hypvec;
  
  // number of hypotheses found 
  int cur_kbest = 0;
    
  // keep tracks of sigs in buffer (don't count multiples twice, since they will be recombined)
  set <> sigs;

  //overfill the buffer since we assume there will be some reordering
  int buf_limit = _ratio * _k;

  while (cur_kbest < _k &&                       
         ! (cands.empty() ||                          
            hypvec.size() >= buf_limit)) {
    
    Candidate cand = cands.pop();
    const Hyp & chyp  = cand.hyp;
    const ForestEdge & cedge = cand.edge;
    const vector <int> & cvecj = cand.vec; 

    //TODO: duplicate management

    if (sigs.find(chyp.sig) == sigs.end()) {
      sigs.add(chyp.sig);
      cur_kbest += 1;
    }
    
    // add hypothesis to buffer
    hypvec.push_back(chyp);
      
    // expand next hypotheses
    next(cedge, cvecj, cands);
  }  
      
  // RECOMBINATION (shrink buf to actual k-best list)
  
  // sort and combine hypevec
  hypvec.sort();
  
  keylist = {};
  
  vector <> newhypvec;
  
  for (item in hypvec) {
    if (!keylist.has_key(item.sig)) {
      keylist[item.sig] = len(newhypvec);
      newhypvec.append(item);
      
      if (newhypvec.size() >= _k) {
        break;
      }
    }
    else {
      pos = keylist[item.sig];
      //semiring plus
      newhypvec[pos].add(item);
    }
  }     
  return newhypvec;
}

void next(const ForestEdge & cedge, const vector <int > cvecj, Candidates cands){
  /*
    @param cedge - the edge that we just took a candidate from
    @param cvecj - the current position on the cedge cube
    @param cands - current candidate list 
  */
  // for each dimension of the cube
  for (int i=0; i < cedge.tail_nodes.size(); i++) {
    // vecj' = vecj + b^i (just change the i^th dimension
    vector <int> newvecj = cvecj;
    newvecj[i] += 1;
    //newvecj = cvecj[:i] + (cvecj[i]+1,) + cvecj[i+1:];
   
    if (newvecj not in cedge.oldvecs) {
      Item newhyp;
      if (gethyp(cedge, newvecj, newhyp)){
        // add j'th dimension to the cube
        cedge.oldvecs.add(newvecj);
        cands.add(Candidate(newhyp, cedge, newvecj));
      }
    }
  }
}


bool gethyp(const ForestEdge & cedge, const vector <int> & vecj, Item & item) {
  /*
    Return the score and signature of the element obtained from combining the
    vecj-best parses along cedge. Also, apply non-local feature functions (LM)
  */
  score = self.scorer.from_edge(cedge);
  
  subders = [];

  // grab the jth best hypothesis at each node of the hyperedge
  for (i, sub in enumerate(cedge.subs) ) {
  
    if (vecj[i] >= len(self.hypothesis_cache[sub])) {
      return false;
    }
  
    item = self.hypothesis_cache[sub][vecj[i]];
    subders.append(item.full_derivation);
    score = self.scorer.times(score, item.score);
  }

  // Get the non-local feature and signature information
  (non_local_score, full_derivation, sig) = self.non_local_feature_function(cedge, subders);
  
  score = self.scorer.times(score, non_local_score);
  item = Item(score, full_derivation, (cedge, vecj), sig, self.scorer, self.find_min);
  return true;
}
