#ifndef DEPPARSELP_H_
#define DEPPARSELP_H_

#include <Hypergraph.h>
#include <gurobi_c++.h>
#include "Tagger.h"
#include "HypergraphLP.h"
#include "../common.h"

namespace Scarab {
  namespace HG {


struct TagLP {

TagLP(const Tagger &parser, const HypergraphLP &hyper_lp) : 
  p(parser), tag_vars(parser.num_tags()), h_lp(hyper_lp) {}
  Cache<Tag, GRBVar>  tag_vars;
  const Tagger & p; 
  const HypergraphLP & h_lp;
};


class TagLPBuilder {  
 public:
  static void show_results(const TagLP & lp_vars); 
  static TagLP * add_tagging(const Tagger & parser, const Cache<Hyperedge, double> & weights,
                             string prefix, GRBModel & model, int var_type);
};
  }
}
#endif
