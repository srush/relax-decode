#ifndef LPCOMMON_H_
#define LPCOMMON_H_
#include "gurobi_c++.h"
#include <sstream>
#include <../common.h>


struct LPConfig {

  LPConfig( string prefix, GRBModel & model, int var_type) : _prefix(prefix), _model(model), _var_type(var_type){
  }

  int var_type() const {
    return _var_type;
  }

  GRBModel & model() {
    return _model;
  }

  GRBVar  addSimpleVar(double score, stringstream & name) {
    stringstream buf;
    buf << _prefix << "_" << name.str();
    return _model.addVar(0.0, 1.0, score, _var_type, buf.str());
 
  }

  GRBConstr  addSimpleConstr(GRBTempConstr lin_expr, stringstream & name ) {
    stringstream buf;
    buf << _prefix << "_" << name;
    return _model.addConstr(lin_expr, buf.str());
  }

  string _prefix;
  GRBModel & _model;
  int _var_type;
};


class LPBuilder {
 public:
  void set_lp_conf(LPConfig * configuration) {
    lp_conf = configuration;
    _initialized = true;
  }
  virtual void show() const {}
  virtual void add_vars() = 0 ;
  virtual void add_constraints() = 0;
  

  static void build_list(GRBModel & model, vector<LPBuilder *> builders) {
    foreach (LPBuilder * builder, builders) {
      builder->add_vars();
    }
    model.update();
    foreach (LPBuilder * builder, builders) {
      builder->add_constraints();
    }
    model.update();
  }

 protected:
  LPConfig * lp_conf;
  bool _initialized;
  bool _vars_initialized;
  

};

#endif
