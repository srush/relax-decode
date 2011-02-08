#ifndef DEPPARSELP_H_
#define DEPPARSELP_H_

#include <Hypergraph.h>
#include <gurobi_c++.h>
#include "DepParser.h"
#include "../common.h"

namespace Scarab {
  namespace HG {


struct DepParserLP {

DepParserLP(const DepParser &parser, const HypergraphLP &hyper_lp) : 
  p(parser), dep_vars(parser.num_deps()), h_lp(hyper_lp) {}
  Cache<Dependency, GRBVar>  dep_vars;
  const DepParser & p; 
  const HypergraphLP & h_lp;
};


class DepParserLPBuilder {  
 public:
  static void show_results(const DepParserLP & lp_vars) {
    //h_lp.edge_vars.get(edge)
    //HypergraphLPBuilder::show_hypergraph(lp_vars.h_lp);

    foreach (Dependency d, lp_vars.p.dependencies()) {
      //if (lp_vars.dep_vars.has_key(d)) {
      GRBVar var = lp_vars.dep_vars.get(d);
      //GRBVar var = lp_vars..get(d);
      //const vector <const Hyperedge *>  & e  = lp_vars.p.dep_to_edge(d);
      //GRBVar edge_var = lp_vars.h_lp.edge_vars.get(e);

        if (var.get(GRB_DoubleAttr_X) != 0.0) {
          cout << d << " ";
        }
        
      //cout << d << " " << (GRBLinExpr)var<< " "<< var.get(GRB_DoubleAttr_X) << " "<< endl;
      //cout << e.id() << (GRBLinExpr)edge_var <<" " << edge_var.get(GRB_DoubleAttr_X) << " "<< endl;

          //}
      //}
    }
    cout << endl;
  }

  static DepParserLP * add_parse(const DepParser & parser, const Cache<Hyperedge, double> & weights,
                               string prefix, GRBModel & model, int var_type) {

    
    // First build the hypergraph for the parser 
    HypergraphLP * h_lp = HypergraphLPBuilder::add_hypergraph(parser,
                                                            weights, 
                                                            prefix, model, var_type);
      
    DepParserLP * p_lp = new DepParserLP(parser, *h_lp);    
    foreach (Dependency d, parser.dependencies()) {
      stringstream buf;
      buf << prefix << "_dep_" << d;
      
      // add a variable for each dependency
      p_lp->dep_vars.set_value(d, model.addVar(0.0, 1.0, 
                                              0.0, var_type, buf.str()));
      
    }
    model.update();

    foreach (Dependency d, parser.dependencies()) {
      // constrain it to be equal to the hypergraph edge
      const vector<const Hyperedge *> & edges = parser.dep_to_edge(d);
      
      GRBLinExpr sum;
      foreach(HEdge edge, edges) {

        //cout << (GRBLinExpr) h_lp->edge_vars.get(edge) << " == " << (GRBLinExpr)  p_lp->dep_vars.get(d) << endl;
        //cout << h_lp->edge_vars.get(edge).get(GRB_DoubleAttr_Obj);
        sum += h_lp->edge_vars.get(*edge);

      } 
      
      stringstream buf;
      buf << prefix << "_dep_is_edge" << d;

      model.addConstr(sum == p_lp->dep_vars.get(d), buf.str());
    }
        
    model.update();
      
    return p_lp;
  }
};
  }
}
#endif
