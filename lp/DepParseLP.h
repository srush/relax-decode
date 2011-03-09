#ifndef DEPPARSELP_H_
#define DEPPARSELP_H_

#include <Hypergraph.h>
#include <gurobi_c++.h>
#include "DepParser.h"
#include "HypergraphLP.h"
#include "../common.h"
#include "LPCommon.h"
namespace Scarab {
  namespace HG {

class DepParserLP:public LPBuilder {
 public:
DepParserLP(const DepParser &parser, const Cache <Hyperedge, double> & weights) : 
  p(parser), dep_vars(parser.num_deps()), h_lp(parser, weights) {}
  Cache<Dependency, GRBVar>  dep_vars;
  const DepParser & p; 
   HypergraphLP h_lp;

  void set_lp_conf(LPConfig * configuration) {
    lp_conf = configuration;
    h_lp.set_lp_conf(configuration);
  }


  void add_vars();
  void add_constraints();
  void show() const; 
};


  }
}
#endif
