#ifndef MRF_H
#define MRF_H

#include <cassert>
#include <string>
#include <Graph.h>
#include <graph.pb.h>
#include <mrf.pb.h>
#include "HypergraphImpl.h"
#include <GraphProtoInterface.h>

#include "EdgeCache.h"
using namespace Scarab::Graph;
using namespace std;
struct State {
  State() {}
State(int id_, string label_):_id(id_), _label(label_) {} 

  int id() const {
    return _id;
  }

  string label() const {
    return _label;
  }

  bool operator==(const State & other) const {
    return _id == other._id; 
  }

  int _id;
  string _label;
};


struct NodeAssignment {
  NodeAssignment(){}
NodeAssignment(int node_id_, const State state_,  int length_): node_id(node_id_), s(state_) , length(length_) {}
  int node_id;
  State s;
  int length;
  
  int id() const {
    return s.id()* length + node_id; 
  }
};

class MRF : public GraphProtoInterface {
 public:
  void process_node(graph::Graph_Node, Graphnode *) ;
  void process_edge(graph::Graph_Edge, Graphedge *) ;
  
  Hypergraph* build_hypergraph();
  
  /** 
   * Solve the potts model with the given node potentials
   * 
   * @param node_potentials 
   */
  //void solve(vector <double> extra_node_potentials) const;

  void set_up(graph::Graph graph, int nodes , int edges);
  const double node_pot(const Graphnode & node, const State & s1) const {
    return _node_potentials->get(node)->get(s1);
  }

  const bool has_edge_pot(const Graphedge & edge, const State & s1, const State & s2) const {
    return _edge_potentials->get(edge).find(pair<int,int>(s1.id(),s2.id())) != _edge_potentials->get(edge).end();
  }

  const double edge_pot(const Graphedge & edge, const State & s1, const State & s2) const {
    if (has_edge_pot(edge, s1, s2)) {
      return  _edge_potentials->get(edge)[pair<int,int>(s1.id(),s2.id())];
    }
    return 0.0;
  }
  

  /* const vector<State>  & states() const { */
  /*   return _states; */
  /* } */

  /* const double potential(int node1, const State & s1, int node2, const State & s2 ) const { */
  /*   assert (node1 != node2); */
  /*   if (s1 == s2) {  */
  /*     return _weight; */
  /*   } */
  /*   return 0.0;   */
  /* } */

  

  const vector <State> & states(const Graphnode & node) const {
    return _node_states->get(node);
  } 

  const int assignments() const{
    return _num_assignments;
  }

  NodeAssignment make_assignment(const Graphnode & n, const State & my_s) const {
    return NodeAssignment(n.id(), my_s, graph().num_nodes());
  }

 private:

  Cache <Graphnode, vector <State> > * _node_states; 
  Cache <Graphnode, Cache <State, double> * > * _node_potentials; 

  // This will be zero on any edge with no potential (makes things way faster)
  Cache <Graphedge, map <pair<int,int>, double> > * _edge_potentials; 

  int _num_assignments;

  /* vector <Graphnode *> nodes; */
  /* vector <Graphedge *> edges; */

  /* string _label; */
  /* int _size; */
  /* vector <State> _states; */
  /* double _weight; */
  //  Graph *  _potts_graph;
};


#endif
