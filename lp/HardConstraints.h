#ifndef HARDCONSTRAINTS_H
#define HARDCONSTRAINTS_H

#include <fstream>
#include "DepParseLP.h"

using namespace std;

struct PossibleDep {
  int ind;
  int group;
  string group_name;
  vector <int> head_inds;
  int sent_num;
};

class HardConstraints {
 public:
  vector < vector <PossibleDep> > _constraint_struct;
  vector < GRBVar > group_used;

  void read_from_file(string file_name) {
    int num_groups;
    fstream input(file_name.c_str(), ios::in );
    input >> num_groups;
    _constraint_struct.resize(num_groups);
    while (input) {

      PossibleDep dep;
      //int group, sent_num, ind, len;
      int len;
      vector <int> head_inds;
      //, 2 74 7 1 19
      input >> dep.group_name >> dep.group >> dep.sent_num >> dep.ind >> len;
      for (int i =0 ; i < len; i++) {
        int head_ind;
        input >> head_ind;
        head_inds.push_back(head_ind);
      }
      dep.head_inds = head_inds;
      _constraint_struct[dep.group].push_back(dep);
    }
  }

  GRBLinExpr make_lin_expr(const vector <DepParserLP*> & lp_vars, const PossibleDep & dep) {
    // the some of all the possible deps
    GRBLinExpr lin;
    foreach (int head_ind, dep.head_inds) {
      cout << dep.sent_num << endl;
      if (head_ind == dep.ind) continue;
      Dependency d = lp_vars[dep.sent_num]->p.make_dep(head_ind, dep.ind);
      cout << dep.sent_num << " " << head_ind << " " << d << endl;
      GRBVar dep_lp_var = lp_vars[dep.sent_num]->dep_vars.get(d);
      cout << (GRBLinExpr)dep_lp_var << endl;
      lin += dep_lp_var;
      
    }
    return lin;
  }
  void show_results() { 
    for (int group_num=0; group_num < _constraint_struct.size(); group_num++) { 
      if (group_used[group_num].get(GRB_DoubleAttr_X)) {
        cout << "Used " << (GRBLinExpr)group_used[group_num] << endl;
      }
    }
  }

  void add_to_lp(const vector <DepParserLP * > & lp_vars, GRBModel & model) {
    try{
      for (int group_num=0; group_num < _constraint_struct.size(); group_num++) { 
        stringstream buf; 
        buf << "group_" << group_num;
        group_used.push_back(model.addVar(0.0, 1.0, 
                                          0.0, GRB_CONTINUOUS, buf.str()));
      }
      model.update();

      for (int group_num=0; group_num < _constraint_struct.size(); group_num++) { 
        PossibleDep first_pdep = _constraint_struct[group_num][0];
        GRBLinExpr base = make_lin_expr(lp_vars, first_pdep);
        model.addConstr(base == group_used[group_num]);
        foreach (PossibleDep pdep, _constraint_struct[group_num]) {
          stringstream buf; 
          buf << "group_const_" <<  pdep.group_name;
          model.addConstr(base == make_lin_expr(lp_vars, pdep), buf.str());
        }
      }
    } catch (GRBException e) {
      cout << "Error code = " << e.getErrorCode() << endl;
      cout << e.getMessage() << endl;
      assert(false);
    }

    model.update();

  }

};

#endif
