#ifndef POTTSMODELLP_H
#define POTTSMODELLP_H

#include "PottsModel.h"
#include "Graph.h"

//#include "TagConstraints.h"
//#include "TagLP.h"

using namespace std;

struct PottsModelLP {
PottsModelLP(const PottsModel &p) : potts(p), node_vars(potts.graph().num_nodes()), edge_vars(potts.graph().num_edges()) {}
  Cache<Graphnode, Cache<State, GRBVar> * >  node_vars;
  Cache<Graphedge, Cache<State, Cache <State , GRBVar > * > *> edge_vars;
  const PottsModel & potts; 
};


class PottsModelBuilderLP {
 public:

  static PottsModelLP * add_potts(const PottsModel & potts, GRBModel & model, int var_type) {
    try{
      PottsModelLP * potts_lp = new PottsModelLP(potts);    
      foreach (Node node, potts.graph().nodes()) {
        Cache <State, GRBVar> * node_state_vars = new Cache<State, GRBVar>(potts.states().size());
        potts_lp->node_vars.set_value(*node, node_state_vars);
        foreach (const State & state, potts.states()) {
          stringstream buf;
          buf << "potts_node_" << node->id() << "_" << state.label() ;
          node_state_vars->set_value(state, model.addVar(0.0, 1.0, 
                                                        0.0, var_type, buf.str()));
        }
      }
      model.update();
      

      // Each node has exactly one state
      foreach (Node node, potts.graph().nodes()) {
        GRBLinExpr sum;
        foreach (const State & state, potts.states()) {
          sum += potts_lp->node_vars.get(*node)->get(state);
        }
        stringstream buf;
        buf << "potts_node_constr_" << node->id();// << "_" << node->label();
        model.addConstr(sum == 1, buf.str());
      }

      model.update();

      foreach (Edge edge, potts.graph().edges()) {
        Cache <State, Cache< State, GRBVar > * > * edge_state_vars = new Cache <State, Cache< State, GRBVar > * >(potts.states().size());
        potts_lp->edge_vars.set_value(*edge, edge_state_vars);

        foreach (const State & state1, potts.states()) {
          Cache <State, GRBVar> * inner_edge_state_vars = new Cache<State, GRBVar>(potts.states().size());
          edge_state_vars->set_value(state1, inner_edge_state_vars);
          foreach (const State & state2, potts.states()) {
            stringstream buf;
            int n1 = edge->from_node()->id();
            int n2 = edge->to_node()->id();
            buf << "potts_edge_" << n1 << "_" << state1.label() << "_" << n2 << "_" << state2.label() ;
            inner_edge_state_vars->set_value(state2, model.addVar(0.0, 1.0, 
                                                           potts.potential(n1, state1, n2, state2), var_type, buf.str()));
          }
        }
      }
      model.update();

      // If a node is on, at least one of the equivalent edge state vars must be on
      
      foreach (Node node, potts.graph().nodes()) {
        GRBLinExpr sum;
        foreach (const State & my_state, potts.states()) {
          GRBLinExpr sum;
          foreach (Edge edge, node->edges()) {
            foreach (const State & other_state, potts.states()) {
              sum += potts_lp->edge_vars.get(*edge)->get(my_state)->get(other_state);
            }
          }
          stringstream buf;
          buf << "potts_node_edge_constr_" << node->id()  << "_" << my_state.id();
          model.addConstr(sum == potts_lp->node_vars.get(*node)->get(my_state), buf.str());
        }
      }
      model.update();

    } catch (GRBException e) {
      cout << "Error code = " << e.getErrorCode() << endl;
      cout << e.getMessage() << endl;
      assert(false);
    }

    model.update();

  }

  /* void show_results() {  */
  /*   for (int group_num=0; group_num < _constraints._constraint_struct.size(); group_num++) {  */
  /*     if (_constraints.groups.find(group_num) == _constraints.groups.end() ) continue;  */
  /*     for (POS pos = 0; pos < _constraints._num_tags; pos++) {  */
  /*       if (group_used[group_num][pos].get(GRB_DoubleAttr_X)) { */
  /*         cout << "Used " << (GRBLinExpr)group_used[group_num][pos] << group_used[group_num][pos].get(GRB_DoubleAttr_X) << endl; */
  /*       } */
  /*       for (int i =1; i < _constraints._constraint_struct[group_num].group.size() ; i++) { */
  /*         if (slacks_pos[group_num][pos][i].get(GRB_DoubleAttr_X)) { */
  /*           cout << "Slack " << (GRBLinExpr) slacks_pos[group_num][pos][i] << endl; */

  /*         } */
  /*         if (slacks_neg[group_num][pos][i].get(GRB_DoubleAttr_X)) {  */
  /*           cout << "Slack " << (GRBLinExpr) slacks_neg[group_num][pos][i] << endl; */

  /*         } */
  /*       } */
        
  /*     } */
  /*   } */
  /* } */


 private:
  const PottsModel & _potts; 
  //double _penalty;
};

#endif
