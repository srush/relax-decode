#ifndef HYPERGRAPHLP_H_
#define HYPERGRAPHLP_H_
#include <Hypergraph.h>
#include <gurobi_c++.h>

#include "../common.h"

namespace Scarab {
  namespace HG {

struct HypergraphLP {
  HypergraphLP(const HGraph & h):  
    _h(h), node_vars(h.num_nodes()), 
    edge_vars(h.num_edges()) {}

  Cache<Hypernode, GRBVar>  node_vars;
  Cache<Hyperedge, GRBVar>  edge_vars;
  const HGraph & _h; 
};




class HypergraphLPBuilder {  
 public:
  static void show_hypergraph(const HypergraphLP & h_lp) {
    foreach (HEdge edge, h_lp._h.edges()) {
      GRBVar var = h_lp.edge_vars.get(*edge);
      if (var.get(GRB_DoubleAttr_X)) {
        cout << (GRBLinExpr)var<< " "<< edge->label() << " " << var.get(GRB_DoubleAttr_X) << " "<< endl;
      }
    }
  }

  static HypergraphLP * add_hypergraph(const HGraph & h, const Cache<Hyperedge, double> & _weights, 
                                     string prefix, GRBModel & model, int var_type) {
    HypergraphLP *  hypergraph_vars= new HypergraphLP(h);

      
    //for(int i =0; i < _forest.num_nodes(); i++) {
    foreach(HNode node, h.nodes()) {
      stringstream buf;
      //buf << prefix << "_node_" << node->id() << "_"<< node->label();
      hypergraph_vars->node_vars.set_value(*node, model.addVar(0.0, 1.0, 0.0 /*Obj*/, 
                                                               var_type /*cont*/,  
                                                               buf.str()/*names*/));
    }

    foreach (HEdge edge, h.edges()) {
      stringstream buf;
      buf << prefix << "_edge_" << edge->id() << "_"<<edge->label();
      
      //assert (_weights.has_value(edge)); 
      hypergraph_vars->edge_vars.set_value(*edge, model.addVar(0.0, 1.0, 
                                                              _weights.get_value(*edge) /*Obj*/, 
                                                              var_type /*cont*/,  
                                                              buf.str()/*names*/));
    }
    
    model.update();
    {
      
      foreach(HNode node, h.nodes()) { 
        // Downward edges
        if (node->edges().size() > 0) {
          
          GRBLinExpr sum; 
          foreach (HEdge edge, node->edges()) {
            /* hyperedges[i] = sum node out ;*/
            sum += hypergraph_vars->edge_vars.get(*edge);
          }
          model.addConstr(hypergraph_vars->node_vars.get(*node) == sum, "Downward_edge");            
        }
        
        // Upward edges
        if (node->in_edges().size() > 0) {
          GRBLinExpr sum; 
          foreach (HEdge edge, node->in_edges()) {
            sum += hypergraph_vars->edge_vars.get(*edge);
          }
          model.addConstr(hypergraph_vars->node_vars.get(*node) == sum, "Upward_edge");
        }
      }
      
    }
    model.addConstr(hypergraph_vars->node_vars.get(h.root()) == 1, "Root");
  
    model.update();  
  
    return hypergraph_vars;
  }
};
  }
}

#endif
