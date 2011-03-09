#include "MRFLP.h"
#include <sstream>
using namespace std;

#define BLANK 1

void MRFLP::add_node_vars() {
  foreach (Node node, mrf.graph().nodes()) {
    int n_states = mrf.state_max(*node);//mrf.states(*node).size();
    int node_id = node->id();
    Cache <State, GRBVar> * node_state_vars = new Cache<State, GRBVar>(n_states);
    node_vars.set_value(*node, node_state_vars);
    foreach (const State & state, mrf.states(*node)) {
      stringstream buf;
      buf <<  "mrf_node_" << node->id() << "_" << state.id() ;
      double pot = mrf.node_pot(*node, state);
      node_state_vars->set_value(state, lp_conf->addSimpleVar(pot, buf));
    }
  }
}


void MRFLP::add_edge_vars() {
  foreach (Edge edge, mrf.graph().edges()) {
    Cache <State, Cache< State, GRBVar > * > * edge_state_vars = new Cache <State, Cache< State, GRBVar > * > (mrf.state_max(*edge->from_node()));
    edge_vars.set_value(*edge, edge_state_vars);

    foreach (const State & state1, mrf.states(*edge->from_node())) {
      Cache <State, GRBVar> * inner_edge_state_vars = 
        new Cache<State, GRBVar>(mrf.state_max(*edge->to_node()));
      edge_state_vars->set_value(state1, inner_edge_state_vars);
      foreach (const State & state2, mrf.states(*edge->to_node())) {
        if (BLANK) {
          bool has_pot = mrf.has_edge_pot(*edge, state1, state2);
          if (!has_pot) continue;
        }
        stringstream buf;
        int n1 = edge->from_node()->id();
        int n2 = edge->to_node()->id();
        double pot = mrf.edge_pot(*edge, state1, state2);
        buf << "mrf_edge_" << n1 << "_" << state1.id() << "_" << n2 << "_" << state2.id() ;
        inner_edge_state_vars->set_value(state2, lp_conf->addSimpleVar(pot, buf));
      }
    }  
    if (BLANK) {
      // var for blank (zero potential) edge
      Cache <State, GRBVar> * from_blank_vars = 
        new Cache<State, GRBVar>(mrf.state_max(*edge->from_node()));
      from_state_blank_vars.set_value(*edge, from_blank_vars);
      Cache <State, GRBVar> * to_blank_vars = 
        new Cache<State, GRBVar>(mrf.state_max(*edge->to_node()));
      to_state_blank_vars.set_value(*edge, to_blank_vars);
      
      
      int n1 = edge->from_node()->id();
      int n2 = edge->to_node()->id();
          
      // each of these vars says "choose state on one side" and choose some zero potential on the other sied       
      foreach (const State & state, mrf.states(*edge->from_node())) {
        stringstream buf;
        buf << "mrf_from_blank_edge_" << n1 << "_" << n2 << "_" << state.id();
        
        from_blank_vars->set_value(state, 
                                   lp_conf->addSimpleVar(0.0, buf));
      }

      foreach (const State & state, mrf.states(*edge->to_node())) {
        stringstream buf;
        buf << "mrf_to_blank_edge_" << n1 << "_" << n2 << "_" << state.id();
        
        to_blank_vars->set_value(state, 
                                 lp_conf->addSimpleVar(0.0, buf));        
      }
    }  
  }
}


void MRFLP::show() const {
  cout << mrf.label() <<endl;
  foreach (Node node, mrf.graph().nodes()) {
    foreach (const State & state, mrf.states(*node)) {
      GRBVar var = node_vars.get(*node)->get(state);
      if (var.get(GRB_DoubleAttr_X)) {
        cout << (GRBLinExpr)var<< " "<< node->id() << " " << node->label() << " " << state.id() << " " << state.label() << " " << var.get(GRB_DoubleAttr_X) << " "<< endl;
      }

      foreach (Edge edge, node->edges()) {
        foreach (const State & other_state, mrf.states(*edge->to_node())) {
          bool has_pot = mrf.has_edge_pot(*edge, state, other_state);
          if (!has_pot) continue;
          GRBVar var = edge_vars.get(*edge)->get(state)->get(other_state);
          if (var.get(GRB_DoubleAttr_X)) {
            cout << (GRBLinExpr)var<< " "<< edge->to_node()->id() << " " << state.id() << " " << other_state.id() << " " << var.get(GRB_DoubleAttr_X) << " "<< var.get(GRB_DoubleAttr_Obj) <<  endl;
          }
        }
      }
    }
  }
  
}

void MRFLP::add_node_constraints() {
  // Each node has exactly one state
  foreach (Node node, mrf.graph().nodes()) {
    GRBLinExpr sum;
    foreach (const State & state, mrf.states(*node)) {
      sum += node_vars.get(*node)->get(state);
    }
    stringstream buf;            
    buf << "mrf_node_constr_" << node->id();
    lp_conf->addSimpleConstr(sum == 1, buf);
  }
}

void MRFLP::add_edge_constraints() {
  // If a node is on, at least one of the equivalent edge state vars must be on
  clock_t s=clock();      
  foreach (Node node, mrf.graph().nodes()) {
    GRBLinExpr sum;
    foreach (const State & my_state, mrf.states(*node)) {
      
      foreach (Edge edge, node->edges()) {
        GRBLinExpr sum;
        foreach (const State & other_state, mrf.states(*edge->to_node())) {
          if (BLANK) {
            bool has_pot = mrf.has_edge_pot(*edge, my_state, other_state);
            if (!has_pot) continue;
          }
          sum += edge_vars.get(*edge)->get(my_state)->get(other_state);
        }
        // or it could be blank
        if (BLANK) {
          sum += from_state_blank_vars.get(*edge)->get(my_state); 
        }                                
        
        stringstream buf;
        buf << "mrf_node_edge_constr_" << node->id()  << "_" << my_state.id();
        lp_conf->addSimpleConstr(sum == node_vars.get(*node)->get(my_state), buf);
        
      }

      foreach (Edge edge, node->in_edges()) {
        GRBLinExpr sum;
        foreach (const State & other_state, mrf.states(*edge->from_node())) {
          if (BLANK) {
            bool has_pot = mrf.has_edge_pot(*edge, other_state, my_state);
            
            if (!has_pot)continue;
          }
          sum += edge_vars.get(*edge)->get(other_state)->get(my_state);
        }
        // or it could be blank
        if (BLANK) {
          sum += to_state_blank_vars.get(*edge)->get(my_state); 
        }
        stringstream buf;
        buf << "mrf_node_edge_constr_" << node->id()  << "_" << my_state.id();
        lp_conf->addSimpleConstr(sum == node_vars.get(*node)->get(my_state), buf);
        
      }
    }
  }
      

  //cout << "consist "<< double(Clock::diffclock(clock(),s)) << endl;
  // finally make sure that if we pick a zero on one side we do it on the other
  if (BLANK) {
    foreach (Edge edge, mrf.graph().edges()) {
      GRBLinExpr sum_blank_from;
      foreach (const State & from_state, mrf.states(*edge->from_node())) {
        sum_blank_from += from_state_blank_vars.get(*edge)->get(from_state);
      }
      
      GRBLinExpr sum_blank_to;
      foreach (const State & to_state, mrf.states(*edge->to_node())) {
        sum_blank_to += to_state_blank_vars.get(*edge)->get(to_state);
      }
      stringstream buf;
      int n1 = edge->from_node()->id();
      int n2 = edge->to_node()->id();
      
      buf << "mrf_blank_" << n1 << "_"<<n2;
      lp_conf->addSimpleConstr(sum_blank_from == sum_blank_to, buf);
    }
  }
}



// MRFLP * MRFBuilderLP::add_mrf(const MRF & mrf, string prefix, GRBModel & model, int var_type) {
//     /* try{ */
//   clock_t s=clock();
//       MRFLP * mrf_lp = new MRFLP(mrf);    
//       foreach (Node node, mrf.graph().nodes()) {
//         int n_states = mrf.states(*node).size();
//         int node_id = node->id();
//         Cache <State, GRBVar> * node_state_vars = new Cache<State, GRBVar>(n_states);
//         mrf_lp->node_vars.set_value(*node, node_state_vars);
//         foreach (const State & state, mrf.states(*node)) {
//           stringstream buf;
//           buf << prefix << "_mrf_node_" << node->id() << "_" << state.id() ;
//           double pot = mrf.node_pot(*node, state);
//           node_state_vars->set_value(state, model.addVar(0.0, 1.0, 
//                                                         pot, var_type, buf.str()));
//         }
//       }
//       model.update();
      
//       cout << "First vars "<< double(Clock::diffclock(clock(),s)) << endl;

//        s=clock();
//       // Each node has exactly one state
//       foreach (Node node, mrf.graph().nodes()) {
//         GRBLinExpr sum;
//         foreach (const State & state, mrf.states(*node)) {
//           sum += mrf_lp->node_vars.get(*node)->get(state);
//         }
//         stringstream buf;            
//         buf << prefix << "_mrf_node_constr_" << node->id();// << "_" << node->label();
//         model.addConstr(sum == 1, buf.str());
//       }

//       model.update();


//       cout << "Nodes "<< double(Clock::diffclock(clock(),s)) << endl;
//        s=clock();
//       foreach (Edge edge, mrf.graph().edges()) {
//         Cache <State, Cache< State, GRBVar > * > * edge_state_vars = new Cache <State, Cache< State, GRBVar > * > (mrf.states(*edge->from_node()).size());
//         mrf_lp->edge_vars.set_value(*edge, edge_state_vars);

//         foreach (const State & state1, mrf.states(*edge->from_node())) {
//           Cache <State, GRBVar> * inner_edge_state_vars = 
//             new Cache<State, GRBVar>(mrf.states(*edge->to_node()).size());
//           edge_state_vars->set_value(state1, inner_edge_state_vars);
//           foreach (const State & state2, mrf.states(*edge->to_node())) {
//             if (BLANK) {
//               bool has_pot = mrf.has_edge_pot(*edge, state1, state2);
//               if (!has_pot) continue;
//             }
//             stringstream buf;
//             int n1 = edge->from_node()->id();
//             int n2 = edge->to_node()->id();
//             double pot = mrf.edge_pot(*edge, state1, state2);
//             buf << prefix << "_mrf_edge_" << n1 << "_" << state1.id() << "_" << n2 << "_" << state2.id() ;
//             inner_edge_state_vars->set_value(state2, model.addVar(0.0, 1.0, 
//                                                            pot, var_type, buf.str()));
//           }
//         }  
//         if (BLANK) {
//           // var for blank (zero potential) edge
//           Cache <State, GRBVar> * from_blank_vars = 
//             new Cache<State, GRBVar>(mrf.states(*edge->from_node()).size());
//           mrf_lp->from_state_blank_vars.set_value(*edge, from_blank_vars);
//           Cache <State, GRBVar> * to_blank_vars = 
//             new Cache<State, GRBVar>(mrf.states(*edge->to_node()).size());
//           mrf_lp->to_state_blank_vars.set_value(*edge, to_blank_vars);

          
//           int n1 = edge->from_node()->id();
//           int n2 = edge->to_node()->id();
          
//           // each of these vars says "choose state on one side" and choose some zero potential on the other sied 
          
//           foreach (const State & state, mrf.states(*edge->from_node())) {
//             stringstream buf;
//             buf << prefix << "_mrf_from_blank_edge_" << n1 << "_" << n2 << "_" << state.id();
  
//             from_blank_vars->set_value(state, 
//                                        model.addVar(0.0, 1.0, 0.0, var_type, buf.str()));
//           }
//           foreach (const State & state, mrf.states(*edge->to_node())) {
//             stringstream buf;
//             buf << prefix << "_mrf_to_blank_edge_" << n1 << "_" << n2 << "_" << state.id();
  
//             to_blank_vars->set_value(state, 
//                                       model.addVar(0.0, 1.0, 0.0, var_type, buf.str()));
          
//           }
//         }  
//       }
//       model.update();

//       cout << "edge "<< double(Clock::diffclock(clock(),s)) << endl;
//       // If a node is on, at least one of the equivalent edge state vars must be on
//        s=clock();      
//       foreach (Node node, mrf.graph().nodes()) {
//         GRBLinExpr sum;
//         foreach (const State & my_state, mrf.states(*node)) {
          
//           foreach (Edge edge, node->edges()) {
//             GRBLinExpr sum;
//             foreach (const State & other_state, mrf.states(*edge->to_node())) {
//               if (BLANK) {
//                 bool has_pot = mrf.has_edge_pot(*edge, my_state, other_state);
//                 if (!has_pot) continue;
//               }
//               sum += mrf_lp->edge_vars.get(*edge)->get(my_state)->get(other_state);
//             }
//             // or it could be blank
//             if (BLANK) {
//               sum += mrf_lp->from_state_blank_vars.get(*edge)->get(my_state); 
//             }                                

//             stringstream buf;
//             buf << prefix << "_mrf_node_edge_constr_" << node->id()  << "_" << my_state.id();
//             model.addConstr(sum == mrf_lp->node_vars.get(*node)->get(my_state), buf.str());

//           }

//           foreach (Edge edge, node->in_edges()) {
//             GRBLinExpr sum;
//             foreach (const State & other_state, mrf.states(*edge->from_node())) {
//               if (BLANK) {
//                 bool has_pot = mrf.has_edge_pot(*edge, my_state, other_state);
                
//                 if (!has_pot)continue;
//               }
//               sum += mrf_lp->edge_vars.get(*edge)->get(other_state)->get(my_state);
//             }
//             // or it could be blank
//             if (BLANK) {
//               sum += mrf_lp->to_state_blank_vars.get(*edge)->get(my_state); 
//             }
//             stringstream buf;
//             buf << prefix << "_mrf_node_edge_constr_" << node->id()  << "_" << my_state.id();
//             model.addConstr(sum == mrf_lp->node_vars.get(*node)->get(my_state), buf.str());

//           }
//         }
//       }
      

//       cout << "consist "<< double(Clock::diffclock(clock(),s)) << endl;
//       // finally make sure that if we pick a zero on one side we do it on the other
//       if (BLANK) {
//         foreach (Edge edge, mrf.graph().edges()) {
//           GRBLinExpr sum_blank_from;
//           foreach (const State & from_state, mrf.states(*edge->from_node())) {
//             sum_blank_from += mrf_lp->from_state_blank_vars.get(*edge)->get(from_state);
//           }

//           GRBLinExpr sum_blank_to;
//           foreach (const State & to_state, mrf.states(*edge->to_node())) {
//             sum_blank_to += mrf_lp->to_state_blank_vars.get(*edge)->get(to_state);
//           }
//           stringstream buf;
//           int n1 = edge->from_node()->id();
//           int n2 = edge->to_node()->id();

//           buf << prefix << "_mrf_blank_" << n1 << "_"<<n2;
//           model.addConstr(sum_blank_from == sum_blank_to, buf.str());
//         }

//       }
//       model.update();

//     /* } catch (GRBException e) { */
//     /*   cout << "Error code = " << e.getErrorCode() << endl; */
//     /*   cout << e.getMessage() << endl; */
//     /*   assert(false); */
//     /* } */

//     model.update();
//     return mrf_lp;
//   }
