#ifndef PARSESOLVERS_H
#define PARSESOLVERS_H


#include "DepParser.h"
#include "ParseConstraints.h"
#include "DualDecomposition.h"
#include "CorpusSolver.h"

class ParserDual:public CorpusSolver {
 public:
 ParserDual(vector <DepParser*> & parsers, 
            const wvector & base_weights,  
            const ParseMrfAligner & consistency): 
  CorpusSolver(parsers.size()),
  _parsers(parsers), 
  _base_weights(base_weights),
    _parse_consistency(consistency),
    best_derivations(parsers.size())
      { }

  vector< HEdges> best_derivations;

 protected:
  const vector <DepParser*> & _parsers;
  const wvector & _base_weights;
  const ParseMrfAligner & _parse_consistency;

  bool dep_to_lag(int sent_num, const Dependency & t, int & lag );

  void solve_one(int sent_num, double & primal, double & dual, wvector & subgrad) ;
  
  int lag_to_sent_num(int lag) ;

  wvector build_parser_subgradient(int sent_num, const DepParser & dep_parser, 
                                   HEdges best_edges);

  EdgeCache build_parser_constraint_vector(int sent_num, const DepParser & dep_parser);
  
};





#endif
