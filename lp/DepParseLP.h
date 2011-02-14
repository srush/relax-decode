#ifndef DEPPARSELP_H_
#define DEPPARSELP_H_

#include <Hypergraph.h>
#include <gurobi_c++.h>
#include "DepParser.h"
#include "HypergraphLP.h"
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
  static void show_results(const DepParserLP & lp_vars);

  static DepParserLP * add_parse(const DepParser & parser, const Cache<Hyperedge, double> & weights,
                                 string prefix, GRBModel & model, int var_type);
};
  }
}
#endif
