
#include "HypergraphLP.h"
#include "../common.h"
namespace Scarab {
  namespace HG {

void HypergraphLP::show() const {
  foreach (HEdge edge, _h.edges()) {
    GRBVar var = edge_vars.get(*edge);
    if (var.get(GRB_DoubleAttr_X)) {
      cout << (GRBLinExpr)var<< " "<< edge->label() << " " << var.get(GRB_DoubleAttr_X) << " "<< endl;
    }
  }
}

// Add an LP var for each node and edge in the hypergraph
void HypergraphLP::add_vars() {
  assert(_initialized);
  
  foreach(HNode node, _h.nodes()) {
    stringstream buf;
    buf << "node_" << node->id();
    node_vars.set_value(*node, lp_conf->addSimpleVar(0.0, buf));
  }

  foreach (HEdge edge, _h.edges()) {
    stringstream buf;
    buf << "edge_" << edge->id() << "_"<<edge->label();
    
    edge_vars.set_value(*edge, lp_conf->addSimpleVar(
                                            _weights.get_value(*edge) /*Obj*/, 
                                            buf));
  }     
  _vars_initialized = true;
}

void HypergraphLP::add_constraints( ) {
  assert(_initialized && _vars_initialized);
  
  foreach(HNode node, _h.nodes()) { 
    // Downward edges
    if (node->edges().size() > 0) {
      GRBLinExpr sum; 
      foreach (HEdge edge, node->edges()) {
        /* hyperedges[i] = sum node out ;*/
        sum += edge_vars.get(*edge);
      }
      stringstream buf;
      buf << "Downward_edge";
    
      lp_conf->addSimpleConstr(node_vars.get(*node) == sum, buf);            
    }
        
    // Upward edges
    if (node->in_edges().size() > 0) {
      GRBLinExpr sum; 
      foreach (HEdge edge, node->in_edges()) {
        sum += edge_vars.get(*edge);
      }
      stringstream buf;
      buf << "Upward_edge";

      lp_conf->addSimpleConstr(node_vars.get(*node) == sum, buf);
    }
  }

  stringstream buf;
  buf << "Root";
  lp_conf->addSimpleConstr(node_vars.get(_h.root()) == 1, buf);
}
  }  }

// static HypergraphLP * add_hypergraph(const HGraph & h, const Cache<Hyperedge, double> & _weights, 
//                                      string prefix, GRBModel & model, int var_type) {
//     HypergraphLP *  hypergraph_vars= new HypergraphLP(h);

      
//     //for(int i =0; i < _forest.num_nodes(); i++) {
//     foreach(HNode node, h.nodes()) {
//       stringstream buf;
//       //buf << prefix << "_node_" << node->id() << "_"<< node->label();
//       hypergraph_vars->node_vars.set_value(*node, model.addVar(0.0, 1.0, 0.0 /*Obj*/, 
//                                                                var_type /*cont*/,  
//                                                                buf.str()/*names*/));
//     }

//     foreach (HEdge edge, h.edges()) {
//       stringstream buf;
//       buf << prefix << "_edge_" << edge->id() << "_"<<edge->label();
      
//       //assert (_weights.has_value(edge)); 
//       hypergraph_vars->edge_vars.set_value(*edge, model.addVar(0.0, 1.0, 
//                                                               _weights.get_value(*edge) /*Obj*/, 
//                                                               var_type /*cont*/,  
//                                                               buf.str()/*names*/));
//     }
    
//     model.update();
//     {
      
//       foreach(HNode node, h.nodes()) { 
//         // Downward edges
//         if (node->edges().size() > 0) {
          
//           GRBLinExpr sum; 
//           foreach (HEdge edge, node->edges()) {
//             /* hyperedges[i] = sum node out ;*/
//             sum += hypergraph_vars->edge_vars.get(*edge);
//           }
//           model.addConstr(hypergraph_vars->node_vars.get(*node) == sum, "Downward_edge");            
//         }
        
//         // Upward edges
//         if (node->in_edges().size() > 0) {
//           GRBLinExpr sum; 
//           foreach (HEdge edge, node->in_edges()) {
//             sum += hypergraph_vars->edge_vars.get(*edge);
//           }
//           model.addConstr(hypergraph_vars->node_vars.get(*node) == sum, "Upward_edge");
//         }
//       }
      
//     }
//     model.addConstr(hypergraph_vars->node_vars.get(h.root()) == 1, "Root");
  
//     model.update();  
  
//     return hypergraph_vars;
//   }


