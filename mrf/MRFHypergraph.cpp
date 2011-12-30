#include "MRFHypergraph.h"

// Turn the mrf into a hypergraph
// This only works if it is not loopy
MRFHypergraph *  MRFHypergraph::from_mrf(const MRF & mrf) {
  // Basically we will treat the directionality of the graph 
  // as an elimination ordering. This is a bit hacky, but general
  // In edges have already been marginalized out.
  MRFHypergraph *mrf_hyp = new MRFHypergraph(mrf);
  
  const Nodes &nodes = mrf.graph().nodes();
  assert(nodes.size() != 0);

  mrf_hyp->_canonical_hnode = new Cache <NodeAssignment, Hypernode * > (mrf.assignments());
  mrf_hyp->_canonical_assignment = new Cache<Hypernode, NodeAssignment >(mrf.assignments()*10);
  cerr << "big array " << mrf.assignments()*10;
  Cache <Graphnode, Cache <Graphnode, Cache <State, Hypernode * > *> * > middle_nodes (nodes.size());
  int hnode_id = 0;
  int hedge_id = 0;
  
  foreach (Node n, nodes) {
    Cache <Graphnode, Cache <State, Hypernode * > *> * c = 
      new Cache <Graphnode, Cache <State, Hypernode * > *> (nodes.size());
    middle_nodes.set_value(*n, c);
    
    // out edges
    foreach (Edge e, n->edges()) {
      Node to_node = e->to_node();
      Cache <State, Hypernode * > * c2 = new Cache <State, Hypernode * >(mrf.state_max(*to_node));
      c->set_value(*to_node, c2);

      foreach (const State & s, mrf.states(*to_node)) {
        // one node for each "message"
        stringstream buf;
        buf << n->id() << "_" << to_node->id() << "_" << s.id(); 
        HypernodeImpl * node = new HypernodeImpl(buf.str(), hnode_id,  new wvector());

        hnode_id++;
        mrf_hyp->_nodes.push_back(node);
        // TODO 
        c2->set_value(s, node); 
      }
    }

    // One hypergraph canonical node for each hypergraph state 
    foreach (const State & my_s, mrf.states(*n) ) {
      stringstream buf;
      buf << n->id() << "_" << my_s.id(); 
      stringstream wstr;
      double node_pot = -mrf.node_pot(*n, my_s);
      HypernodeImpl * base_hnode = new HypernodeImpl(buf.str(), hnode_id,  
                                                     svector_from_str<int, double>(wstr.str()));
      NodeAssignment assign = mrf.make_assignment(*n, my_s);
      int id = assign.id();
      mrf_hyp->_canonical_hnode->set_value(assign, base_hnode);
      mrf_hyp->_canonical_assignment->set_value(*base_hnode, mrf.make_assignment(*n, my_s));
      hnode_id++;
      mrf_hyp->_nodes.push_back(base_hnode);

      // outgoing edges
      foreach (Edge e, n->edges()) {
        Node to_node = e->to_node();
        foreach (const State & to_s, mrf.states(*to_node)) {
          Hypernode * to_hnode = middle_nodes.get(*n)->get( *to_node)->get(to_s);
          vector <Hypernode *> tail_node;
          tail_node.push_back( base_hnode);
          stringstream wstr;
          wstr << "value="<< node_pot + -mrf.edge_pot(*e, my_s, to_s);
          HyperedgeImpl * edge = new HyperedgeImpl("", 
                                                   svector_from_str<int, double>(wstr.str()),
                                                   hedge_id, tail_node, to_hnode);
          hedge_id++;
          mrf_hyp->_edges.push_back(edge);
          ((HypernodeImpl*)to_hnode)->add_edge(edge);
          ((HypernodeImpl*)base_hnode)->add_in_edge(edge);
        }
      }

      // Incoming edges
      vector <Hypernode *> tail_nodes;
      foreach (Edge e, n->in_edges()) {
        Node from_node = e->from_node();
        Hypernode * mid_node = middle_nodes.get(*from_node)->get( *n)->get(my_s);
        tail_nodes.push_back(mid_node);
      }
      HyperedgeImpl * edge = new HyperedgeImpl("", new wvector(), hedge_id, tail_nodes, base_hnode);
      hedge_id++;
      mrf_hyp->_edges.push_back(edge);

      base_hnode->add_edge(edge);
      
      foreach (Hypernode * tail_node, tail_nodes) {
        ((HypernodeImpl*)tail_node)->add_in_edge(edge);
      }
    }
  }

  // last node is root
  assert(nodes.size() != 0);
  Node root = nodes[nodes.size()-1];
  mrf_hyp->_root = new HypernodeImpl("", hnode_id, new wvector());
  hnode_id++;
  mrf_hyp->_nodes.push_back(mrf_hyp->_root);

  foreach (const State & last_state, mrf.states(*root) ) {
    vector <Hypernode *> tail_nodes;
    Hypernode * last_node = mrf_hyp->_canonical_hnode->get(mrf.make_assignment(*root, last_state));
    tail_nodes.push_back(last_node);
    double node_pot = -mrf.node_pot(*root, last_state);
    stringstream wstr;
    wstr << "value="<< node_pot;
    HyperedgeImpl * edge = new HyperedgeImpl("",svector_from_str<int, double>(wstr.str()), hedge_id, tail_nodes, mrf_hyp->_root);
    hedge_id++;
    mrf_hyp->_edges.push_back(edge);
    ((HypernodeImpl*)mrf_hyp->_root)->add_edge(edge);
    ((HypernodeImpl*)last_node)->add_in_edge(edge);
  }
  cerr << "size: " << mrf_hyp->num_nodes() << " " << mrf_hyp->num_edges() << endl;
  return mrf_hyp;
}

