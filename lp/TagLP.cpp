
#include "TagLP.h"
namespace Scarab {
  namespace HG {

void TagLPBuilder::show_results(const TagLP & lp_vars) {
  //h_lp.edge_vars.get(edge)
  //HypergraphLPBuilder::show_hypergraph(lp_vars.h_lp);

  foreach (Tag d, lp_vars.p.tags()) {
    if (lp_vars.p.tag_has_node(d)) {     
      GRBVar var = lp_vars.tag_vars.get(d);
      
      if (var.get(GRB_DoubleAttr_X) != 0.0) {
        cout << d << " ";
      }
    }
  }
  cout << endl;
  foreach (Tag d, lp_vars.p.tags()) {
    if (lp_vars.p.tag_has_node(d)) {     
      GRBVar var = lp_vars.tag_vars.get(d);
      
      if (var.get(GRB_DoubleAttr_X) != 0.0) {
        cout << d << " " << var.get(GRB_DoubleAttr_X) << " " ;
      }
    }
  }

  cout << endl;
}

TagLP * TagLPBuilder::add_tagging(const Tagger & parser, const Cache<Hyperedge, double> & weights,
                                 string prefix, GRBModel & model, int var_type) {

    
    // First build the hypergraph for the parser 
    HypergraphLP * h_lp = HypergraphLPBuilder::add_hypergraph(parser,
                                                              weights, 
                                                              prefix, model, var_type);
      
    TagLP * p_lp = new TagLP(parser, *h_lp);
    foreach (Tag d, parser.tags()) {
      if (parser.tag_has_node(d)) {
        stringstream buf;
        buf << prefix << "_tag_" << d;
        
        // add a variable for each dependency
        p_lp->tag_vars.set_value(d, model.addVar(0.0, 1.0, 
                                                 0.0, var_type, buf.str()));
      }
    }
    model.update();

    foreach (Tag d, parser.tags()) {
      // constrain it to be equal to the hypergraph edge
      if (parser.tag_has_node(d)) { 
        const Hypernode & node = parser.tag_to_node(d);
      
        //GRBLinExpr sum;
        //assert edge.size() == 1;
        // foreach(HEdge edge, edges) {
        //   sum += h_lp->edge_vars.get(*edge);
        // } 
      
        stringstream buf;
        buf << prefix << "_tag_is_node" << d;

        model.addConstr(h_lp->node_vars.get(node) == p_lp->tag_vars.get(d), buf.str());
      }
    }
        
    model.update();
      
    return p_lp;
  }
  }
}
