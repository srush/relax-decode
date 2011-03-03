#ifndef TAGLP_H_
#define TAGLP_H_

#include <Hypergraph.h>
#include "Tagger.h"
#include "HypergraphLP.h"
#include "../common.h"
#include "LPCommon.h"
#include <gurobi_c++.h>
namespace Scarab {
  namespace HG {


    class TagLP : public LPBuilder {
    public:
TagLP(const Tagger &tagger,const Cache <Hyperedge, double> & weights ) : 
  p(tagger), tag_vars(tagger.num_tags()), h_lp(tagger, weights) {}
  Cache<Tag, GRBVar>  tag_vars;
  const Tagger & p; 
  HypergraphLP  h_lp;

  void set_lp_conf(LPConfig * configuration) {
    lp_conf = configuration;
    h_lp.set_lp_conf(configuration);
  }
  
  void add_vars();
  void add_constraints();
  void show() const; 

};


  } }
#endif
