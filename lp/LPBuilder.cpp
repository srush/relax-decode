#include <cy_svector.hpp>
#include <svector.hpp>
#include "Hypergraph.h"
#include "LPBuilder.h"


#include "HypergraphAlgorithms.h"
#include <sstream>
#include "../common.h"
using namespace std;

namespace Scarab { 
  namespace HG { 
// //#define _var_type GRB_CONTINUOUS
// #define _var_type GRB_BINARY

struct LatticeVars {
  vector < vector < vector < GRBVar > > > all_pairs_vars;
  vector < vector < GRBVar > > all_pairs_exist_vars;
  vector < vector < vector < bool > > > has_all_pairs_var;
  string name;
  int var_type;
  LatticeVars(string n, int var_type):name(n),var_type(var_type) {

  }

  void  initialize_all_pairs(const GraphDecompose & gd,

                             const ForestLattice & _lattice,
                             GRBModel * model);
    void add_all_pairs_constraints(const GraphDecompose & gd,
                                   const ForestLattice & _lattice,
                                   GRBModel * model);

};

void  LatticeVars::initialize_all_pairs(const GraphDecompose & gd,
                                        const ForestLattice & _lattice,
                                        GRBModel * model) {

  int num_nodes = _lattice.get_graph().num_nodes();
  all_pairs_exist_vars.resize(num_nodes); 
  all_pairs_vars.resize(num_nodes); 
  has_all_pairs_var.resize(num_nodes);  

  for (int i = 0; i < num_nodes; i++) {
    all_pairs_vars[i].resize(num_nodes);
    all_pairs_exist_vars[i].resize(num_nodes);
    has_all_pairs_var[i].resize(num_nodes);
    for (int j = 0; j < num_nodes; j++) {
      
        has_all_pairs_var[i][j].resize(num_nodes);        
        for (int k =0; k < num_nodes ; k++) {
          has_all_pairs_var[i][j][k] = false;
        }
        if (!gd.path_exists(i,j)) continue;
        
        all_pairs_vars[i][j].resize(num_nodes);


        const vector <Node> * path = gd.get_path(i, j);
        
        assert (path->size() != 0 || i == j);

        //for (int k = 0; k < path->size(); k++) {
        foreach (Node k , *path) {
          int kid = k->id();
          stringstream buf;
          buf << name <<" SHORTEST " << i << " " << j << " " << kid;
          double obj= 0.0; 
          all_pairs_vars[i][j][kid] = model->addVar(0.0, 1.0, obj, var_type ,  buf.str());
          has_all_pairs_var[i][j][kid] = true;
          
        }
        stringstream buf;
        buf << name <<" EXIST " << i << " " << j;
        
        all_pairs_exist_vars[i][j] = model->addVar(0.0, 1.0, 0.0 , var_type ,  buf.str());
      }
    }
    
    model->update();
}

void LatticeVars::add_all_pairs_constraints(const GraphDecompose & gd,
                                            const ForestLattice & _lattice,
                                            GRBModel * model) {
  int num_nodes = _lattice.get_graph().num_nodes();
  for (int i = 0; i < num_nodes; i++) { 
    for (int j = 0; j < num_nodes; j++) {
      if (!gd.path_exists(i,j)) continue;
      if (i == j) continue; 
      {
        GRBLinExpr sum;
        const vector <Node> * path = gd.get_path(i, j);
        bool has = false;
        foreach (Node k, *path) {
          int last = k->id();
          
          assert(has_all_pairs_var[i][j][last]);
          sum += all_pairs_vars[i][j][last];
          //model->addConstr(all_pairs_exist_vars[i][last] == 1.0);
          //model->addConstr(all_pairs_vars[i][j][last] == all_pairs_path)
          has = true;
        }
        assert (has);
        model->addConstr(all_pairs_exist_vars[i][j] == sum);
      }        
    }
  }
  
    

  // Node constraints (INNER)
  
  for (int i = 0; i < num_nodes; i++) { 
    for (int j = 0; j < num_nodes; j++) {
      if (!gd.path_exists(i,j)) continue;
      
      GRBLinExpr sum;
      bool has = false;
      
      if (!_lattice.is_phrase_node(j)) {
        for (int k = 0; k < num_nodes; k++) {
          if (!has_all_pairs_var[i][k][j]) continue;
          if (k == j) continue;
          
          
          sum += all_pairs_vars[i][k][j];
          has = true;
        }
      }
      if (!_lattice.is_phrase_node(i)) {
        for (int k = 0; k < num_nodes; k++) {
          if (!has_all_pairs_var[k][j][i]) continue;        
          if (i == j) continue;
          
          sum += all_pairs_vars[k][j][i];
          has = true;
        }
      }
      // below 
      
      if  (has) {
        model->addConstr(all_pairs_exist_vars[i][j] == sum);
      }
      
    }
  }
  model->update();
}

void LPBuilder::initialize_word_pairs(Ngram &lm, 
                                      const Cache <Graphnode, int> & word_cache,
                                      const GraphDecompose & gd, 
                                      vector < GRBVar > & word_used_vars,
                                      vector < vector < GRBVar >  > & word_pair_vars,
                                      vector < vector < vector < GRBVar > > > & word_tri_vars) {

  word_used_vars.resize(_lattice.num_word_nodes); 
  word_pair_vars.resize(_lattice.num_word_nodes); 
  //vector < vector < bool > > has_pair_var(_lattice.num_word_nodes);
  word_tri_vars.resize(_lattice.num_word_nodes); 


  for (int i = 0; i < _lattice.num_word_nodes; i++) {
    if (!_lattice.is_word(i)) continue; 
    word_pair_vars[i].resize(_lattice.num_word_nodes);
    word_tri_vars[i].resize(_lattice.num_word_nodes);
    stringstream buf;
    buf << "UNI " << i ;
    word_used_vars[i] = model->addVar(0.0, 1.0, 0.0 /*Obj*/, _var_type /*cont*/,  buf.str()/*names*/);      
  }
  
  //for (unsigned int i=0; i< gd.valid_bigrams.size() ;i++) {
  foreach (const WordBigram & b, gd.valid_bigrams()) {
    //Bigram b = gd.valid_bigrams[i];
    
    //has_pair_var[i].resize(_lattice.num_word_nodes);
    //word_tri_vars[i].resize(_lattice.num_word_nodes);
    stringstream buf;
    buf << "BI " << b;
    //has_pair_var[i][j] = true;
    
    //VocabIndex context [] = {word_cache.store[b.w2], Vocab_None};
    //double prob = lm.wordProb(word_cache.store[b.w1], context);
    //if (b.w1 == 1 && b.w2==0) prob = 0.0;
    //if (isinf(prob)) prob = -1000000.0;
    
    word_pair_vars[b.w1.id()][b.w2.id()] = model->addVar(0.0, 1.0, 0.0 /*Obj*/, _var_type /*cont*/,  buf.str()/*names*/);
    word_tri_vars[b.w1.id()][b.w2.id()].resize(_lattice.num_word_nodes);
    for (int m =0; m < gd.forward_bigrams[b.w2.id()].size(); m++) {
      int k = gd.forward_bigrams[b.w2.id()][m];
      stringstream buf;
      buf << "TRI " << b << " " << k;
      VocabIndex context [] = {word_cache.store[b.w2.id()], word_cache.store[k], Vocab_None};
      double prob = _lm_weight * lm.wordProb(word_cache.store[b.w1.id()], context);
      if (isinf(prob)) prob = 1000000.0;
      
      word_tri_vars[b.w1.id()][b.w2.id()][k] = model->addVar(0.0, 1.0, prob/*Obj*/, _var_type /*cont*/,  buf.str()/*names*/);
    }
  }
  model->update();
}

void LPBuilder::build_all_pairs_lp(Ngram &lm, 
                                   const Cache <Graphnode, int> & word_cache,
                                   vector < GRBVar > & word_used_vars,
                                   vector < vector < vector < GRBVar > > > & word_tri_vars,
                                   LatticeVars & lv,
                                   const GraphDecompose & gd) {
  
  vector < vector < GRBVar > >  word_pair_vars;
  
  try{
    lv.initialize_all_pairs(gd, _lattice, model);
    initialize_word_pairs(lm, word_cache, gd, word_used_vars, word_pair_vars, word_tri_vars);    
    model->update();

    
    // Last must hit first
    //model->addConstr(all_pairs_exist_vars[_lattice.num_nodes-1][0] == 1.0);
    
   
    // Node constraints (Outer)
  
    lv.add_all_pairs_constraints(gd, _lattice, model);
    int num_nodes = _lattice.get_graph().num_nodes();
    for (int i = 0; i < num_nodes; i++) { 
      for (int j = 0; j < num_nodes; j++) {
        if (!gd.path_exists(i,j)) continue;
        if (i == j) continue; 
        if (_lattice.is_phrase_node(i) && _lattice.is_phrase_node(j)) {
          GRBLinExpr sum;
          for (int m =0; m < _lattice.num_last_words(i); m++) {
            
            int lword = _lattice.last_words(i, m);
            assert(_lattice.is_word(lword));
            for (int n =0; n < _lattice.num_first_words(j); n++) {
              int rword = _lattice.first_words(j, n);
              assert(_lattice.is_word(rword));
              //assert(has_pair_var)
              
              sum += word_pair_vars[lword][rword];
            }
          }
          //if (i == j) continue;
          
          model->addConstr(sum == lv.all_pairs_exist_vars[i][j]);
        }
      }
    }

    /*cout << "INNER" << endl;
    // Node constraints (OUTER)
    for (int i = 0; i < _lattice.num_nodes; i++) { 
      if (_lattice.is_phrase_node(i)) continue;
      for (int j = 0; j < _lattice.num_nodes; j++) {
        if (i == j) continue; 
        if (!gd.path_exists(i,j)) continue;
        
        {
          GRBLinExpr sum;
          bool has = false;
          for (int k = 0; k < _lattice.num_nodes; k++) {
            if (!has_all_pairs_var[k][j][i]) continue;        
           
            sum += all_pairs_vars[k][j][i];
            has = true;
          }

          // below 
          if (has) {
            model->addConstr(all_pairs_exist_vars[i][j] == sum);
          }
        }   
      }
      }*/
    


    // Word constraints    
    
    for (int i = 0; i < _lattice.num_word_nodes; i++) {
      if (!_lattice.is_word(i)) continue ;      
      {
        GRBLinExpr sumFor, sumBack;
        for (int j = 0; j < gd.forward_bigrams[i].size(); j++) {
          int next = gd.forward_bigrams[i][j];
          sumFor += word_pair_vars[i][next];
        }
        
        for (int j = 0; j < gd.backward_bigrams[i].size(); j++) {
          int next = gd.backward_bigrams[i][j];
          sumBack += word_pair_vars[next][i];
        }

        if (i != 0) {
          model->addConstr(word_used_vars[i] == sumFor);
        } else {
          model->addConstr(word_used_vars[i] == 1);
        }
      
        if (i != _lattice.num_word_nodes - 1) {
          model->addConstr(word_used_vars[i] == sumBack);
        } else {
          model->addConstr(word_used_vars[i] == 1);
        }
      }

      /*for (int j = 0; j < _lattice.num_word_nodes; j++) {
        if (!_lattice.is_word(j)) continue ;
        
        GRBLinExpr sumFor, sumBack;
        for (int k =0; k < _lattice.num_word_nodes; k++) {
          sumFor += word_tri_vars[i][j][k];
          sumBack += word_tri_vars[k][i][j];
        }
        if (i != 0) {
          model->addConstr(word_pair_vars[i][j] == sumFor);
        }
      
        if (i != _lattice.num_word_nodes - 1) {
          model->addConstr(word_pair_vars[i][j] == sumBack);
        }
        int m = _lattice.lookup_word(i); 
        int n = _lattice.lookup_word(j); 
        }*/
    }

    
    //for (int i = 0; i < gd.valid_bigrams.size(); i++) {
    foreach (WordBigram wb, gd.valid_bigrams()) {
      
      Bigram b; // = gd.valid_bigrams[i];
      b.w1 = wb.w1.id();
      b.w2 = wb.w2.id();
      
      {
        GRBLinExpr sumFor, sumBack;
        for (int j = 0; j < gd.forward_bigrams[b.w2].size(); j++) {
          int next = gd.forward_bigrams[b.w2][j];
          sumFor += word_tri_vars[b.w1][b.w2][next];
        }
        
        for (int j = 0; j < gd.backward_bigrams[b.w1].size(); j++) {
          int next = gd.backward_bigrams[b.w1][j];
          sumBack += word_tri_vars[next][b.w1][b.w2];
        }

        if (b.w2 != 0) {
          model->addConstr(word_pair_vars[b.w1][b.w2] == sumFor);
        } else {
          model->addConstr(word_pair_vars[b.w1][b.w2] == 1);
        }
      
        if (b.w1 != _lattice.num_word_nodes - 1) {
          model->addConstr(word_pair_vars[b.w1][b.w2] == sumBack);
        } else {
          model->addConstr(word_pair_vars[b.w1][b.w2] == 1);
        }
      }
    }
  }
  catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
    return;
  }
  model->update();


}


void LPBuilder::build_all_tri_pairs_lp(Ngram &lm, 
                                       const Cache <Graphnode, int> & word_cache,
                                       vector < GRBVar > & word_used_vars,
                                       vector < vector < vector < GRBVar > > > & word_tri_vars,
                                       LatticeVars & lv,
                                       LatticeVars & lv2,
                                       const GraphDecompose & gd) {
  
  vector < vector < GRBVar > >  word_pair_vars;
  
  try{
    lv.initialize_all_pairs(gd, _lattice, model);
    lv2.initialize_all_pairs(gd, _lattice, model);
    initialize_word_pairs(lm, word_cache, gd, word_used_vars, word_pair_vars, word_tri_vars);    
    model->update();
  
    lv.add_all_pairs_constraints(gd, _lattice, model);
    lv2.add_all_pairs_constraints(gd, _lattice, model);

    // bind word_used_vars to _lattice
    /*for (int i = 0; i < _lattice.num_word_nodes; i++) {
      if (!_lattice.is_word(i)) continue;
      
      int phrase_node = _lattice.lookup_word(i);
      model->addConstr(word_used_node[i] == lattice_node_used[phrase_node]);
    } */




    
    // Word constraints    
    
    for (int i = 0; i < _lattice.num_word_nodes; i++) {
      
      if (!_lattice.is_word(i)) continue ;      

      {
        int w1 = i;
        GRBLinExpr sumFor;
        bool has = false;
        for (int j = 0; j < gd.forward_bigrams[i].size(); j++) {
          int w2 = gd.forward_bigrams[i][j];
          for (int k = 0; k < gd.forward_bigrams[w2].size(); k++) {
            int w3 = gd.forward_bigrams[w2][k];
            sumFor += word_tri_vars[w1][w2][w3];
            has = true;
          }
        }
        
        if (has) {
          stringstream buf;
          buf << "FOR " << i;
          
          model->addConstr(word_used_vars[i] == sumFor, buf.str());
        }

      }
        

      {
        int w2 = i;
        GRBLinExpr sumMid;
        bool has = false;
        for (int j = 0; j < gd.forward_bigrams[i].size(); j++) {
          int w3 = gd.forward_bigrams[i][j];
          for (int k = 0; k < gd.backward_bigrams[w2].size(); k++) {
            int w1 = gd.backward_bigrams[w2][k];
            sumMid += word_tri_vars[w1][w2][w3];
            has = true;
          }
        }
        if (has) {
          stringstream buf;
          buf << "MID " << i;
          
          model->addConstr(word_used_vars[i] == sumMid, buf.str());;
        }
      }


      {
        int w3 = i;
        GRBLinExpr sumBack;
        bool has = false;
        for (int j = 0; j < gd.backward_bigrams[i].size(); j++) {
          int w2 = gd.backward_bigrams[i][j];
          for (int k = 0; k < gd.backward_bigrams[w2].size(); k++) {
            int w1 = gd.backward_bigrams[w2][k];
            
            sumBack += word_tri_vars[w1][w2][w3];
            has = true;
          }
        }
        if (has) {
          stringstream buf;
          buf << "BACK " << i;
          
          model->addConstr(word_used_vars[i] == sumBack, buf.str());
        }
      }
    }
    int num_nodes = _lattice.get_graph().num_nodes();
    vector <vector <GRBLinExpr> >  sums1(num_nodes), sums2(num_nodes);
    vector <vector <bool> >  has_sums1(num_nodes), has_sums2(num_nodes);
    

    for (int i = 0; i < num_nodes; i++) {
      sums1[i].resize(num_nodes);
      sums2[i].resize(num_nodes);
      has_sums1[i].resize(num_nodes);
      has_sums2[i].resize(num_nodes);
      for (int j = 0; j < num_nodes; j++) {
        has_sums1[i][j] = false;
        has_sums2[i][j] = false;
      }
    }

    //for (unsigned int i=0; i< gd.valid_bigrams.size() ;i++) {
    foreach (const WordBigram & b, gd.valid_bigrams()) {
      //Bigram b = gd.valid_bigrams[i];
      int w1 = b.w1.id();
      int w2 = b.w2.id();
      
      Node phrase_node1 = _lattice.lookup_word(w1);
      Node phrase_node2 = _lattice.lookup_word(w2);
      
      for (int m =0; m < gd.forward_bigrams[w2].size(); m++) {
        int w3 = gd.forward_bigrams[w2][m];
        Node phrase_node3 = _lattice.lookup_word(w3);
        
        sums1[phrase_node1->id()][phrase_node2->id()] += word_tri_vars[w1][w2][w3];
        sums2[phrase_node2->id()][phrase_node3->id()] += word_tri_vars[w1][w2][w3];
        has_sums1[phrase_node1->id()][phrase_node2->id()] = true;
        has_sums2[phrase_node2->id()][phrase_node3->id()] = true;
        
        
        //model->addConstr(word_tri_vars[w1][w2][w3] == );
        //model->addConstr(word_tri_vars[w1][w2][w3] == sum);
      }
      
    }


    for (int i = 0; i < num_nodes; i++) {
      for (int j = 0; j < num_nodes; j++) {
        if (!gd.path_exists(i,j)) continue;
        if ( i==j) continue;
        
        if (has_sums1[i][j]) {
          stringstream buf;
          buf << "BILAT  " << i << " " << j ;          
          model->addConstr(sums1[i][j] == lv.all_pairs_exist_vars[i][j], buf.str());
        } 
        if (has_sums2[i][j]) {
          stringstream buf;
          buf << "TRILAT  " << i << " " << j ;          
          model->addConstr(sums2[i][j] == lv2.all_pairs_exist_vars[i][j], buf.str());
        }
        stringstream buf;
        buf << "LATSAME  " << i << " " << j ;          
        
        //model->addConstr(lv.all_pairs_exist_vars[i][j] == 
        //                 lv2.all_pairs_exist_vars[i][j], buf.str());
      }
    }

    /* for (int j = 0; j < gd.backward_bigrams[i].size(); j++) {
          int next = gd.backward_bigrams[i][j];
          sumBack += word_pair_vars[next][i];
        }

        if (i != 0) {
          model->addConstr(word_used_vars[i] == sumFor);
        } else {
          model->addConstr(word_used_vars[i] == 1);
        }
      
        if (i != _lattice.num_word_nodes - 1) {
          model->addConstr(word_used_vars[i] == sumBack);
        } else {
          model->addConstr(word_used_vars[i] == 1);
        }
        }*/

      /*for (int j = 0; j < _lattice.num_word_nodes; j++) {
        if (!_lattice.is_word(j)) continue ;
        
        GRBLinExpr sumFor, sumBack;
        for (int k =0; k < _lattice.num_word_nodes; k++) {
          sumFor += word_tri_vars[i][j][k];
          sumBack += word_tri_vars[k][i][j];
        }
        if (i != 0) {
          model->addConstr(word_pair_vars[i][j] == sumFor);
        }
      
        if (i != _lattice.num_word_nodes - 1) {
          model->addConstr(word_pair_vars[i][j] == sumBack);
        }
        int m = _lattice.lookup_word(i); 
        int n = _lattice.lookup_word(j); 
        }*/
  


  /*for (int i = 0; i < gd.valid_bigrams.size(); i++) {
      Bigram b = gd.valid_bigrams[i];
      {
        GRBLinExpr sumFor, sumBack;
        for (int j = 0; j < gd.forward_bigrams[b.w2].size(); j++) {
          int next = gd.forward_bigrams[b.w2][j];
          sumFor += word_tri_vars[b.w1][b.w2][next];
        }
        
        for (int j = 0; j < gd.backward_bigrams[b.w1].size(); j++) {
          int next = gd.backward_bigrams[b.w1][j];
          sumBack += word_tri_vars[next][b.w1][b.w2];
        }

        if (b.w2 != 0) {
          model->addConstr(word_pair_vars[b.w1][b.w2] == sumFor);
        } else {
          model->addConstr(word_pair_vars[b.w1][b.w2] == 1);
        }
      
        if (b.w1 != _lattice.num_word_nodes - 1) {
          model->addConstr(word_pair_vars[b.w1][b.w2] == sumBack);
        } else {
          model->addConstr(word_pair_vars[b.w1][b.w2] == 1);
        }
      }
      }*/
  }
  catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
    return;
  }
  model->update();


}


void LPBuilder::build_hypergraph_lp(vector <GRBVar> & node_vars,  
                                    vector <GRBVar> & edge_vars, 
                                    const Cache<Hyperedge, double> & _weights) {
  try {
    model->set(GRB_StringAttr_ModelName, "Hypergraph");
    
    for(int i =0; i < _forest.num_nodes(); i++) {
      stringstream buf;
      buf << "NODE" << i;
      node_vars[i] = model->addVar(0.0, 1.0, 0.0 /*Obj*/, _var_type /*cont*/,  buf.str()/*names*/);
    }

    for (int i=0; i< _forest.num_edges() ; i++ ) { 

      const Hyperedge & edge = _forest.get_edge(i);
      stringstream buf;
      buf << "EDGE" << i;
      
      //assert (_weights.has_value(edge)); 
      edge_vars[i] = model->addVar(0.0, 1.0, _weights.get_value(edge) /*Obj*/, _var_type /*cont*/,  buf.str()/*names*/);
    }
    
    model->update();
    {

      for(int i =0; i < _forest.num_nodes(); i++) {
        const Hypernode & node = _forest.get_node(i);

        // Downward edges
        {
          GRBLinExpr sum; 
          for (int j=0; j < node.num_edges(); j++) {
            /* nodes_vars[i] = sum node out ;*/
            sum += edge_vars[node.edge(j).id()];
          }
          if (node.num_edges() > 0) { 
            model->addConstr(node_vars[i] == sum);
          }
        }

        // Upward edges
        { 
          GRBLinExpr sum; 
          for (int j=0; j < node.num_in_edges(); j++) {
            sum += edge_vars[node.in_edge(j).id()];

          }
          if (node.num_in_edges() > 0) { 
            model->addConstr(node_vars[i] == sum);
          }
        }
      }
      
    }
    model->addConstr(node_vars[_forest.root().id()] == 1);
    
    
    model->set(GRB_IntAttr_ModelSense, 1);
    model->update();
  
  
  }
  catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
    return;
  }  
}


void LPBuilder::solve_hypergraph(const Cache<Hyperedge, double> & _weights) {
  GRBEnv env = GRBEnv();
  model = new GRBModel(env);
  vector <GRBVar> node_vars(_forest.num_nodes());
  vector <GRBVar> edge_vars(_forest.num_edges());
  build_hypergraph_lp( node_vars, edge_vars, _weights);

  model->optimize();

  try {
    for (int i=0; i < _forest.num_nodes(); i++) {
      
      const Hypernode & node = _forest.get_node(i);    
      for (int j=0; j < node.num_edges(); j++) {
        int edge_id = node.edge(j).id();
      
      }
    }
  }
  catch (GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
    return;
  }

}

/*void LPBuilder::solve_full(const Cache<ForestEdge, double> & _weights, const ForestLattice & _lattice, Ngram &lm, const Cache <Graphnode, int> & word_cache) {  
  GRBEnv env = GRBEnv();
  GRBModel model = GRBModel(env);
  //vector <GRBVar> node_vars(_forest.num_nodes());
  //vector <GRBVar> edge_vars(_forest.num_edges());
  build_all_pairs_lp(_lattice, model, lm, word_cache);

  // Now add in LM
  }*/


void LPBuilder::solve_full(int run_num, const Cache<Hyperedge, double> & _weights, 
                           Ngram &lm, double lm_weight, 
                           const Cache <Graphnode, int> & word_cache) {  
  GraphDecompose gd(_lattice);
  LatticeVars lv("Bi", _var_type),lv2("Tri", _var_type);
  _lm_weight = lm_weight;
  gd.decompose();

  GRBEnv env = GRBEnv();
  env.set(GRB_StringParam_LogFile, "/tmp/log");
  env.set(GRB_DoubleParam_TimeLimit, 100);
  env.set(GRB_IntParam_OutputFlag, 0);
  model = new GRBModel(env);
  
  //vector <GRBVar> node_vars(_forest.num_nodes());
  //vector <GRBVar> edge_vars(_forest.num_edges());

  vector <GRBVar> node_vars(_forest.num_nodes());
  vector <GRBVar> edge_vars(_forest.num_edges());
  vector < vector < vector < GRBVar > > > word_tri_vars;
  //vector < vector < GRBVar > > all_pairs_exist_vars;
  vector < GRBVar > word_used_vars;

  build_hypergraph_lp(node_vars, edge_vars, _weights);
  //build_all_pairs_lp(lm, word_cache, word_used_vars, word_tri_vars,lv,  gd);
  build_all_tri_pairs_lp(lm, word_cache, word_used_vars, word_tri_vars, lv, lv2, gd);

  // Now add constraints to join the two models
  for (int i =0; i < _forest.num_edges(); i++) {
    vector <int> all = _lattice.original_edges[i];
    for (int j=0; j < all.size();j++) {
      if (_lattice.is_word(all[j])) {
        model->addConstr(word_used_vars[all[j]] == edge_vars[i]);
      } 
    }
  }
  
  for (int i =0; i < _lattice.num_word_nodes; i++) {
    if (_lattice.is_word(i)) continue;
    Bigram b = _lattice.get_nodes_by_labels(i);
    GRBLinExpr sum;
    for (int j=0; j < _lattice.edges_original[i].size(); j++) {
      sum += edge_vars[_lattice.edges_original[i][j]];
    }
    model->addConstr(lv.all_pairs_exist_vars[b.w1][b.w2] == sum);
    model->addConstr(lv2.all_pairs_exist_vars[b.w1][b.w2] == sum);
  }

  int num_nodes = _lattice.get_graph().num_nodes();
  // extra trick constraint
  for (int i =0; i < num_nodes; i++) {
  //for (int i =0; i < num_nodes; i++) {
    if (!_lattice.is_phrase_node(i)) continue;
    for (int j=0; j < _lattice.num_last_bigrams(i); j++) {
      Bigram b = _lattice.last_bigrams(i,j);
      
      for (int w3 = 0; w3 < _lattice.num_word_nodes; w3++) {
        if (!_lattice.is_word(w3)) continue;
        Node n1 = _lattice.lookup_word(b.w2);
        Node n2 = _lattice.lookup_word(w3);
        if (gd.path_exists(*n1, *n2)) {
          model->addConstr(lv.all_pairs_exist_vars[n1->id()][n2->id()] == lv2.all_pairs_exist_vars[n1->id()][n2->id()]);
        }

      }
    }
  }

  // Now add in LM
  model->update();
  model->write("/tmp/graph.lp");
  model->set(GRB_IntAttr_ModelSense, 1);

  model->optimize();
  

  int exact = 0;
  
  for (int i = 0; i < _lattice.num_word_nodes; i++) {
    if (!_lattice.is_word(i)) continue; 
    double xval = word_used_vars[i].get(GRB_DoubleAttr_X);
    
  

    if (xval != 0.0 && xval!=1.0) {
      exact +=1;
    }
  }

  //for (unsigned int i=0; i< gd.valid_bigrams.size() ;i++) {
  foreach (const WordBigram & wb, gd.valid_bigrams()) {
    Bigram b;
    b.w1 = wb.w1.id();
    b.w2 = wb.w2.id();

    for (int j =0; j < gd.forward_bigrams[b.w2].size(); j++) {
      int w3 = gd.forward_bigrams[b.w2][j];
    
      double xval =  word_tri_vars[b.w1][b.w2][w3].get(GRB_DoubleAttr_X);

      if (word_tri_vars[b.w1][b.w2][w3].get(GRB_DoubleAttr_X)) {
        //cout << b.w1 << " " << b.w2 << " " << w3 << " " 
        //   << _lattice.lookup_word(b.w1) << " " << _lattice.lookup_word(b.w2) << " " << _lattice.lookup_word(w3) << " " 
        //   << word_tri_vars[b.w1][b.w2][w3].get(GRB_DoubleAttr_X) << " " << word_tri_vars[b.w1][b.w2][w3].get(GRB_DoubleAttr_Obj) << endl;
      
      }
      if ( !(xval == 0.0 || xval == 1.0)) {
        //cerr << b.w1 << " " << b.w2 << " " << w3 << " " << word_tri_vars[b.w1][b.w2][w3].get(GRB_DoubleAttr_X) << " " << word_tri_vars[b.w1][b.w2][w3].get(GRB_DoubleAttr_Obj) << endl;
        //cerr << i << " " << j << " " << all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_X) << " " << all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_Obj) << endl;
        exact +=1;
      }
    }
  }


  for (int i = 0; i < num_nodes; i++) {
    for (int j = 0; j < num_nodes; j++) {
      if (!gd.path_exists(i,j)) continue; 
      
      double x1 =  lv.all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_X);
      double x2 =  lv2.all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_X);
      if (fabs(x1 -x2) >0.001) {
        //cout << "DIFFER " << i << " " << _lattice.is_phrase_node(i) << " " << j << " " <<_lattice.is_phrase_node(j)  <<" "<< x1<< " "<< x2 << endl;
      }
      if (lv.all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_X)) {
        //cout << i << " " << j << " " << all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_X) << " " << all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_Obj) << endl;
      }
      double xval =  lv.all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_X);
      if ( !(xval == 0.0 || xval == 1.0)) {
        //cerr << i << " " << j << " " << all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_X) << " " << all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_Obj) << endl;
        exact += 1;
      }
    }
  }

  /*
  for (int i = 0; i < num_nodes; i++) {
    for (int j = 0; j < num_nodes; j++) {
      if (!gd.path_exists(i,j)) continue; 
      //cout << i << " " << j << endl;
      if (lv2.all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_X)) {
        //cout << i << " " << j << " " << all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_X) << " " << all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_Obj) << endl;
      }
      double xval =  lv2.all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_X);
      if ( !(xval == 0.0 || xval == 1.0)) {
        //cerr << i << " " << j << " " << all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_X) << " " << all_pairs_exist_vars[i][j].get(GRB_DoubleAttr_Obj) << endl;
        exact += 1;
      }
    }
    }*/

  cout << "*END* " << run_num<< " "<<model->get(GRB_DoubleAttr_ObjVal) << " " << model->get(GRB_DoubleAttr_Runtime) << " "<<  (exact == 0) << " " << exact << endl;
}





const Cache <Graphnode, int> * sync_lattice_lm(const ForestLattice  &_lattice, Ngram & lm) {
  Cache <Graphnode, int> *  _cached_words = new Cache <Graphnode, int> (_lattice.num_word_nodes);
  int max = lm.vocab.numWords();
  int unk = lm.vocab.getIndex(Vocab_Unknown);
  //assert(false);
  for (int n=0; n < _lattice.num_word_nodes; n++ ) {
    if (!_lattice.is_word(n)) continue;
    
    //const Graphnode & node = _lattice.node(n); 
    //assert (node.id() == n);
    string str = _lattice.get_word(n);
    int ind = lm.vocab.getIndex(str.c_str());
    if (ind == -1 || ind > max) { 
      _cached_words->store[n] = unk;
    } else {
      _cached_words->store[n] = ind;
    }
  }
  return _cached_words;
}

// int main(int argc, char ** argv) {
//   GOOGLE_PROTOBUF_VERIFY_VERSION;
  
//   svector<int, double> * weight;

//   {
//     // Read the existing address book.
//     fstream input(argv[3], ios::in );
//     char buf[1000];
//     input.getline(buf, 100000);
//     string s (buf);
//     weight = svector_from_str<int, double>(s);
//   }

  
//   Vocab * all = new Vocab();
//   all->unkIsWord() = true;
//   Ngram * lm = new Ngram(*all, 3);

//   File file(argv[4], "r", 0);    
//   if (!lm->read(file, false)) {
//     cerr << "READ FAILURE\n";
//   }

//   for(int i=atoi(argv[5]);i<=atoi(argv[6]); i++) {

//       Hypergraph hgraph;
      
//       {
//         stringstream fname;
//         fname <<argv[1] << i;
//         fstream input(fname.str().c_str(), ios::in | ios::binary);
//         if (!hgraph.ParseFromIstream(&input)) {
//           assert (false);
//         } 
//       }
      
//       Forest f (hgraph);
      
//       Lattice lat;

//       {
//         stringstream fname;
//         fname <<argv[2] << i;

//         fstream input(fname.str().c_str(), ios::in | ios::binary);
//         if (!lat.ParseFromIstream(&input)) {
//           assert (false);
//         }
        
//       }

//       ForestLattice graph (lat);

      
//       LPBuilder lp(f, graph);
      

//       const Cache <Graphnode, int> * word_cache = sync_lattice_lm(graph, *lm); 
      

//       Cache<ForestEdge, double> * w = cache_edge_weights(f, *weight);
      
//       try {
//         lp.solve_full(i, *w,  *lm, *word_cache);
//       } 
//       catch (GRBException e) {
//         cerr << "Error code = " << e.getErrorCode() << endl;
//         cerr << e.getMessage() << endl;
//         cout << "*END* " << i<< " "<<0 << " " << 200 << " "<<  0 << " " << 0 << endl;
//       }
 


//       NodeBackCache bcache(f.num_nodes());     
      
//       NodeCache ncache(f.num_nodes());
//       //double best = best_path(f, *w, ncache, bcache);
//       //cout << best << endl;
//   }  
//   return 1;
// }
  }
}
