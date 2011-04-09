#ifndef PARSEMRFLP_H
#define PARSEMRFLP_H

#include "DepParseLP.h"
#include "MRFLP.h"
#include "ParseConstraints.h"

using namespace Scarab::HG;
class ParseMrfLP {
 public:
  static void align_parse_mrf(const vector < MRFLP *> & mrflp, 
                       const vector < DepParserLP *> & parselp, 
                       ParseMrfAligner aligner,
                       GRBModel & model, 
                       int var_type ) {
    for (int i=0; i < parselp.size(); i ++ ) {
      foreach (const Dependency & t, parselp[i]->p.dependencies()) {        
        MrfIndex mrf_ind; 
        ParseIndex parse_ind(i, t.mod, t.head);
        bool is_aligned = aligner.align(parse_ind, mrf_ind);
        if (!is_aligned) continue;
        
        assert(mrf_ind.group < mrflp.size());
        
        GRBLinExpr dep_var;
        if (parselp[i]->p.dep_has_edge(t)) {
          dep_var = parselp[i]->dep_vars.get(t);
        } else {
          dep_var = 0.0;
        }

        GRBVar mrf_node_state_var = mrflp[mrf_ind.group]->node_vars.store[mrf_ind.node]->
          store[mrf_ind.state];
        
        stringstream buf; 
        buf << "parse_mrf_join_" << t;
        model.addConstr(dep_var == mrf_node_state_var, buf.str());
      }
    }
  }

};

#endif
