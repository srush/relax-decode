#ifndef EXTENDCKY_H_
#define EXTENDCKY_H_

#include "EdgeCache.h"
#include "Forest.h"
#include "Hypothesis.h"
#include <assert.h>
#include <map>
#include <queue>
#include <set>
#include <vector>
#include "BestHyp.h"
// #define PDIM 3

using namespace std;




typedef Cache <ForestNode, const ForestEdge *> NodeBackCache;
//typedef StoreCache <Hypothesis, double> BestHyp;

class ExtendCKY {
 public:
  ExtendCKY(const Forest & forest):
  _forest(forest), _memo_table(forest.num_nodes()), 
    _outside_memo_table(forest.num_nodes()),
    _outside_edge_memo_table(forest.num_edges()),
    _memo_edge_table(forest.num_edges()),
    _memo_edge_back_table(forest.num_edges())
      {
        _is_first = true;
      }
    
    double best_path(NodeBackCache & back_pointers);
    void set_params(Cache <ForestEdge, double> * edge_weights,  Controller * cont) {
      _old_edge_weights = _edge_weights;
      _edge_weights = edge_weights;
      _controller = cont;
    }
    void outside();
  Cache <ForestNode, BestHyp >  _outside_memo_table;
  Cache <ForestEdge, vector<BestHyp> >  _outside_edge_memo_table;

 private:
  const Forest & _forest;
  double _total_best;
  Cache <ForestEdge, double>  * _edge_weights;
  Cache <ForestEdge, double>  * _old_edge_weights;
  Controller * _controller;
  //Cache <ForestNode, BestHyp > * _old_memo_table;
  Cache <ForestNode, BestHyp >  _memo_table;
  Cache <ForestEdge, vector<BestHyp> >  _memo_edge_table;
  Cache <ForestEdge, vector<BestHyp> >  _memo_edge_back_table;

  // outside
  queue <int> _out_queue;
  set <int> _out_done;


  void forward_edge(const ForestEdge & edge,  vector <BestHyp>  & best_edge_hypotheses);
  void backward_edge(const ForestEdge & edge,  vector <BestHyp> & best_edge_hypotheses);

  void node_best_path(const ForestNode & node); 
  void node_best_out_path(const ForestNode & node);
  void node_best_out_fast(const ForestNode & node);
  
  //void extract_back_pointers(const ForestNode & node, const Hypothesis & best_hyp, 
  //                                      NodeBackCache & back_pointers);

  vector <BestHyp *> _to_delete;
  bool _is_first;
};




#endif
