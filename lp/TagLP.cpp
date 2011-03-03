
#include "TagLP.h"
namespace Scarab {
  namespace HG {

void TagLP::show() const {
  //h_lp.edge_vars.get(edge)
  //HypergraphLPBuilder::show_hypergraph(lp_vars.h_lp);

  foreach (Tag d, p.tags()) {
    if (p.tag_has_node(d)) {     
      GRBVar var = tag_vars.get(d);
      
      if (var.get(GRB_DoubleAttr_X) != 0.0) {
        cout << d << " ";
      }
    }
  }
  cout << endl;
  foreach (Tag d, p.tags()) {
    if (p.tag_has_node(d)) {     
      GRBVar var = tag_vars.get(d);
      
      if (var.get(GRB_DoubleAttr_X) != 0.0) {
        cout << d << " " << var.get(GRB_DoubleAttr_X) << " " ;
      }
    }
  }

  cout << endl;
}


void TagLP::add_vars( ) {
  h_lp.add_vars();
  //TagLP * p_lp = new TagLP(parser, *h_lp);
  foreach (Tag d, p.tags()) {
    if (p.tag_has_node(d)) {
      stringstream buf;
      buf << "tag_" << d;
      
      // add a variable for each dependency
      tag_vars.set_value(d, lp_conf->addSimpleVar(0.0, buf));
    }
  }
}

void TagLP::add_constraints() {
  h_lp.add_constraints();
  foreach (Tag d, p.tags()) {
    // constrain it to be equal to the hypergraph edge
    if (p.tag_has_node(d)) { 
      HNodes nodes = p.tag_to_node(d);
      
      GRBLinExpr sum;
      //assert(node.size() == 1;
      foreach(HNode node, nodes) {
        sum += h_lp.node_vars.get(*node);
      } 
      
      stringstream buf;
      buf << "tag_is_node" << d;

      lp_conf->addSimpleConstr(sum == tag_vars.get(d), buf);
    }
  }
}
  }
} 







// TagLP * TagLPBuilder::add_tagging(const Tagger & parser, const Cache<Hyperedge, double> & weights,
//                                  string prefix, GRBModel & model, int var_type) {

    
//     // First build the hypergraph for the parser 
//     HypergraphLP * h_lp = HypergraphLPBuilder::add_hypergraph(parser,
//                                                               weights, 
//                                                               prefix, model, var_type);
      
   
//     model.update();

//     foreach (Tag d, parser.tags()) {
//       // constrain it to be equal to the hypergraph edge
//       if (parser.tag_has_node(d)) { 
//         HNodes nodes = parser.tag_to_node(d);
      
//         GRBLinExpr sum;
//         //assert(node.size() == 1;
//         foreach(HNode node, nodes) {
//           sum += h_lp->node_vars.get(*node);
//         } 
      
//         stringstream buf;
//         buf << prefix << "_tag_is_node" << d;

//         model.addConstr(sum == p_lp->tag_vars.get(d), buf.str());
//       }
//     }
        
//     model.update();
      
//     return p_lp;
//   }
//   }
// }
