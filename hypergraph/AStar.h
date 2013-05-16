

#ifndef ASTAR_H_
#define ASTAR_H_

#include <assert.h>
#include <map>
#include <vector>
#include <queue>
#include "Hypothesis.h"
#include "Hypergraph.h"
#include "BestHyp.h"
#include <iostream>
#include <limits>
#include "../common.h"
using namespace std;
namespace Scarab{
  namespace HG{


// numeric_limits<double>::max()
//typedef Cache <Hypernode, const Hyperedge *> NodeBackCache;
enum loc { NODE, EDGE, TOP};

struct Location {
  loc location;
  int node_id;
  int edge_id;
  int edge_pos;
  void show() {
    if (location == NODE) {
      cout << "NODE " << node_id << endl;
    } else if (location == EDGE){
      cout << "EDGE " << edge_id << " " << edge_pos << endl;
    }
  }
};

class Heuristic {
 public:
  virtual bool has_value(const Location & loc,
                         const Hypothesis & hyp) const = 0;
  virtual double get_value(const Location & loc,
                           const Hypothesis & hyp) const = 0;
};

struct QueueHyp{
  Hypothesis * h;
  double score;
  Location * where;

  QueueHyp(){}
  QueueHyp(Hypothesis * hyp, double score_in, Location * w):
  h(hyp), score(score_in), where(w) {  }
  bool operator<( const QueueHyp & other) const  {
    return score > other.score;
  }
};

class AStar {
 public:
 AStar(const HGraph & f,
       const Controller & cont,
       const Cache <Hyperedge, double> & edge_weights,
       const Heuristic & heu
       ) :
  _forest(f), _controller(cont), _memo_table(_forest.num_nodes()),
    _memo_edge_table(_forest.num_edges()),
    _edge_weights(edge_weights),
    _heuristic(heu), _best_so_far(-INF),
    _num_pops(0),
    _num_pushes(0),
    _num_recompute(0){}
  double best_path(NodeBackCache & back_pointers);

  ~AStar() {
    for (uint i=0;i< _locs.size(); i++) {
      delete _locs[i];
    }

    for (uint i=0;i< _hyps.size(); i++) {
      delete _hyps[i];
    }
  }
private:
  vector <Hypothesis *> _hyps;
  vector <Location *> _locs;
  Hypothesis * alloc_hyp() {
    _hyps.push_back(new Hypothesis());
    return _hyps[_hyps.size()-1];
  }
  Hypothesis * alloc_hyp(const State & h, const State & r,
                         const Hyperedge * be) {

    _hyps.push_back(new Hypothesis(h, r, be));
    return _hyps[_hyps.size()-1];
  }



  Location * alloc_loc() {
    _locs.push_back(new Location());
    return _locs[_locs.size()-1];
  }

  void get_next(Hypothesis *& hyp, double & score, Location *&);
  void add_to_queue( Hypothesis * hyp, double score, Location * );
  void initialize_queue();
  void main_loop(Hypothesis * & best, double & best_score );
  void recompute_edge(const Hyperedge & edge,
                             int pos,
                             const Hypothesis & h,
                             double original_score);
  void recompute_node(const Hypernode & node, const Hypothesis & h, double original_score);
  const HGraph & _forest;
  const Controller & _controller;
  //void forward_edge(const Hyperedge & edge,  vector <BestHyp> & best_edge_hypotheses);
  void forward_edge(const Hyperedge & edge,  vector <BestHyp> & best_edge_hypotheses,  int pos_changed, int id);
  Cache <Hypernode, BestHyp *> _memo_table;
  Cache <Hyperedge, vector<BestHyp> *> _memo_edge_table;

  const Cache <Hyperedge, double>  & _edge_weights;
  const Heuristic  & _heuristic;
  priority_queue <QueueHyp> _queue;
  double _best_so_far;
  // Stats for optimization
  int _num_pops;
  int _num_pushes;
  int _num_recompute;
};

  }}
#endif

