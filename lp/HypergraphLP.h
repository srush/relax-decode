
#include <Hypergraph.h>
#include "../common.h"

struct HypergraphLP {
  HypergraphLP(const Hypergraph & h) _h(h), node_vars(h.num_nodes()), edge_vars(h.num_edges()) {}

  Cache<Hypernode, GRBVar>  node_vars;
  Cache<Hyperedge, GRBVar>  edge_vars;
  const Hypergraph & _h; 
}




class HypergraphLPBuilder {  
  static HypergraphLP add_hypergraph(const Hypergraph & h, const Cache<ForestEdge, double> & _weights, 
                                     string prefix, GRBModel & model, int var_type) {
    HypergraphLP hypergraph_vars;

      
    //for(int i =0; i < _forest.num_nodes(); i++) {
    foreach(HNode node, h.nodes()) {
      stringstream buf;
      buf << prefix << "_node_" << node->id();
      hypergraph_vars.node_vars.set(*node, model->addVar(0.0, 1.0, 0.0 /*Obj*/, 
                                                         var_type /*cont*/,  
                                                         buf.str()/*names*/));
    }

    foreach (HEdge edge, h.edges()) {
      stringstream buf;
      buf << prefix << "_edge_" << edge->id();
      
      //assert (_weights.has_value(edge)); 
      hypergraph_vars.edge_vars.set(*edge, model->addVar(0.0, 1.0, 
                                                         _weights.get_value(edge) /*Obj*/, 
                                                         var_type /*cont*/,  
                                                         buf.str()/*names*/));
    }
    
    model->update();
    {
      
      foreach(HNode node, _forest.nodes()) { 
        // Downward edges
        if (!node->is_terminal()) {
          
          GRBLinExpr sum; 
          foreach (HEdge edge, nodes.edges()) {
            /* hyperedges[i] = sum node out ;*/
            sum += hypergraph_lp.edge_vars.get(*edge);
          }
          model->addConstr(node_vars[i] == sum);            
        }
        
        // Upward edges
        if (!node->is_terminal()) {
          GRBLinExpr sum; 
          foreach (HEdge edge, nodes.in_edges()) {
            sum += hypergraph_lp.edge_vars.get(*edge);
          }
          model->addConstr(node_vars[i] == sum);
        }
      }
      
    }
    model->addConstr(hypergraph_lp.node_vars.get(_forest.root()) == 1);
  
    model->update();  
  
    return hypergraph_vars;
  }
}
