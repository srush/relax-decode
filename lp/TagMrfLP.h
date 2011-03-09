#ifndef TAGMRFLP_H
#define TAGMRFLP_H

#include "TagLP.h"
#include "MRFLP.h"
#include "TagConstraints.h"

using namespace Scarab::HG;

class TagMrfLP {
 public:
  static void align_tag_mrf(const vector <const MRFLP *> & mrflp, 
                            const vector <const TagLP *> & taglp, 
                            TagMrfAligner aligner,
                            GRBModel & model, 
                            int var_type ) {
    for (int i=0; i < taglp.size(); i ++ ) {
      foreach (const Tag & t, taglp[i]->p.tags()) {        
        MrfIndex mrf_ind; 
        TagIndex tag_ind(i, t.ind, t.tag);
        bool is_aligned = aligner.align(tag_ind, mrf_ind);
        if (!is_aligned) continue;
        
        assert(mrf_ind.group < mrflp.size());
        
        GRBLinExpr tag_var;
        if (taglp[i]->p.tag_has_node(t)) {
          tag_var = taglp[i]->tag_vars.get(t);
        } else {
          tag_var = 0.0;
        }
        GRBVar mrf_node_state_var = mrflp[mrf_ind.group]->node_vars.store[mrf_ind.node]->
          store[mrf_ind.state];
        
        stringstream buf; 
        buf << "tag_mrf_join_" << t;
        model.addConstr(tag_var == mrf_node_state_var, buf.str());
      }
    }
    model.update();
  }

};

#endif
