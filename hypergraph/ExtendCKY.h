#ifndef EXTENDCKY_H_
#define EXTENDCKY_H_

#include "EdgeCache.h"
#include "Hypergraph.h"
#include "Hypothesis.h"
#include <assert.h>
#include <map>
#include <queue>
#include <set>
#include <vector>
#include "BestHyp.h"
// #define PDIM 3

using namespace std;

namespace Scarab {
  namespace HG {

    //typedef Cache <Hypernode, const Hyperedge *> NodeBackCache;
//typedef StoreCache <Hypothesis, double> BestHyp;

class ExtendCKY {
 public:
 ExtendCKY(const HGraph & forest, const Cache <Hyperedge, double> & edge_weights,  const Controller & cont)
  : _forest(forest),
    _edge_weights(edge_weights),
    _old_edge_weights(edge_weights),
    _controller(cont),
    _memo_table(forest.num_nodes()),
    _memo_edge_table(forest.num_edges()),
    _memo_edge_back_table(forest.num_edges()),
    _outside_memo_table(forest.num_nodes()),
    _outside_edge_memo_table(forest.num_edges()) {}

    double best_path(NodeBackCache & back_pointers);

    void outside();

    ~ExtendCKY() {


    }

 private:
  const HGraph & _forest;
  double _total_best;
  const Cache <Hyperedge, double>  & _edge_weights;
  const Cache <Hyperedge, double>  & _old_edge_weights;
  const Controller & _controller;

  //Cache <Hypernode, BestHyp > * _old_memo_table;
  Cache <Hypernode, BestHyp *>  _memo_table;
  Cache <Hyperedge, vector<BestHyp> *>  _memo_edge_table;
  Cache <Hyperedge, vector<BestHyp> *>  _memo_edge_back_table;

 public:
    Cache <Hypernode, BestHyp *>  _outside_memo_table;
    Cache <Hyperedge, vector<BestHyp> *>  _outside_edge_memo_table;

 private:
  // outside
  queue <int> _out_queue;
  set <int> _out_done;


  void forward_edge(const Hyperedge & edge,  vector <BestHyp>  & best_edge_hypotheses);
  void backward_edge(const Hyperedge & edge,  vector <BestHyp> & best_edge_hypotheses);

  void node_best_path(const Hypernode & node);
  void node_best_out_path(const Hypernode & node);
  void node_best_out_fast(const Hypernode & node);

  //void extract_back_pointers(const Hypernode & node, const Hypothesis & best_hyp,
  //                                      NodeBackCache & back_pointers);

  vector <BestHyp *> _to_delete;

};



  }}
#endif
