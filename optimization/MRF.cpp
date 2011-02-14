#include "MRF.h"
#include "HypergraphImpl.h"


void MRF::process_node(graph::Graph_Node proto_node, Graphnode * internal_node) {
  const graph::MRFNode & mrfnode = proto_node.GetExtension(graph::mrf_node); 
  int num_states = mrfnode.node_potentials_size();
  Cache <State, double> * node_potentials = new Cache <State, double> (num_states);
  _node_potentials->set_value(*internal_node, node_potentials);
  
  for (int i =0; i < mrfnode.node_potentials_size(); i++ ) {
    const graph::NodeStatePotential & node_poten = mrfnode.node_potentials(i);
    const graph::State & state = node_poten.state();
    float weight = node_poten.node_potential();
    State s(state.id(), state.label());
    _node_states->get_no_check(*internal_node).push_back(s);
    _num_assignments++;
    node_potentials->set_value(s, weight);
  }
  
}

void MRF::process_edge(graph::Graph_Edge proto_edge, Graphedge * internal_edge) {
  const graph::MRFEdge & mrf_edge = proto_edge.GetExtension(graph::mrf_edge);
  map <pair<int,int>,  double > & edge_weights = _edge_potentials->get_no_check(*internal_edge);

  int n_from_states = states(*internal_edge->from_node()).size();
  int n_to_states = states(*internal_edge->to_node()).size();
  
  // edge_weights.resize(n_from_states);
  // for (int s=0;s < n_from_states; s++ ) {
  //   edge_weights[s].resize(n_to_states);
  //   for (int s2=0;s2 < n_to_states; s2++ ) {
  //     edge_weights[s][s2] = 0.0;
  //   }
  // }


  for (int i =0; i < mrf_edge.edge_potentials_size(); i++ ) {
    const graph::EdgeStatePotential & edge_poten = mrf_edge.edge_potentials(i);
    int from = edge_poten.from_state_id();
    int to = edge_poten.to_state_id();
    
    edge_weights[pair<int,int>(from,to)] = edge_poten.edge_potential();
  }
}


void MRF::set_up(graph::Graph graph, int nodes , int edges) {
  _node_states = new Cache <Graphnode, vector <State> >(nodes);
  _node_potentials = new Cache <Graphnode, Cache <State, double> * >(nodes);
  _edge_potentials = new Cache <Graphedge, map<pair<int,int>, double> >(edges);
}
