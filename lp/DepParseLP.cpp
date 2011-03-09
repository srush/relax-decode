#include "DepParseLP.h"

namespace Scarab {
  namespace HG {


void DepParserLP::show() const {
  foreach (Dependency d, p.dependencies()) {     
    GRBVar var = dep_vars.get(d);
      
    if (var.get(GRB_DoubleAttr_X) != 0.0) {
      cout << d << " ";
    }        
  }
  cout << endl;
}

void DepParserLP::add_vars() {
  h_lp.add_vars();
  
  foreach (Dependency d, p.dependencies()) {
    stringstream buf;
    buf << "dep_" << d;
    // add a variable for each dependency
    dep_vars.set_value(d, lp_conf->addSimpleVar(0.0, buf));
  }
}    

void DepParserLP::add_constraints() {
  h_lp.add_constraints();
  foreach (Dependency d, p.dependencies()) {
    // constrain it to be equal to the hypergraph edge
    const vector<const Hyperedge *> & edges = p.dep_to_edge(d);
    
    GRBLinExpr sum;
    foreach(HEdge edge, edges) {
      sum += h_lp.edge_vars.get(*edge);
    } 
    
    stringstream buf;
    buf << "dep_is_edge" << d;
   
    lp_conf->addSimpleConstr(sum == dep_vars.get(d), buf);
  }
}

  } 
}
   //  // First build the hypergraph for the parser 
//     HypergraphLP * h_lp = HypergraphLPBuilder::add_hypergraph(parser,
//                                                             weights, 
//                                                             prefix, model, var_type);
      
//     DepParserLP * p_lp = new DepParserLP(parser, *h_lp);    
//     foreach (Dependency d, parser.dependencies()) {
//       stringstream buf;
//       buf << prefix << "_dep_" << d;
      
//       // add a variable for each dependency
//       p_lp->dep_vars.set_value(d, model.addVar(0.0, 1.0, 
//                                               0.0, var_type, buf.str()));
      
//     }
//     model.update();

//     foreach (Dependency d, parser.dependencies()) {
//       // constrain it to be equal to the hypergraph edge
//       const vector<const Hyperedge *> & edges = parser.dep_to_edge(d);
      
//       GRBLinExpr sum;
//       foreach(HEdge edge, edges) {

//         //cout << (GRBLinExpr) h_lp->edge_vars.get(edge) << " == " << (GRBLinExpr)  p_lp->dep_vars.get(d) << endl;
//         //cout << h_lp->edge_vars.get(edge).get(GRB_DoubleAttr_Obj);
//         sum += h_lp->edge_vars.get(*edge);

//       } 
      
//       stringstream buf;
//       buf << prefix << "_dep_is_edge" << d;

//       model.addConstr(sum == p_lp->dep_vars.get(d), buf.str());
//     }
        
//     model.update();
      
//     return p_lp;
//   }
//   }
// }
