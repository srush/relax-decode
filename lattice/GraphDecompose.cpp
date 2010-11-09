#include "ForestLattice.h"

#include <vector>
#include <bitset>
#include <iostream>
#include <assert.h>
#include "GraphDecompose.h"
using namespace std;

#define INF 1000000


  
GraphDecompose::GraphDecompose() {
    
}
  
  
void GraphDecompose::compute_bigrams() {
  for (int n=0; n < g->num_nodes; n++ ) {
    if (g->word_node[n] == -1) continue; 
    for (int n2=0; n2 < g->num_nodes; n2++ ) {
      if (g->word_node[n2] == -1) continue;
      
      if (!all_pairs_path[n][n2].empty()) { 
        valid_bigrams.push_back(Bigram(n,n2));
        forward_bigrams[n].push_back(n2);        
        //bigram_bitset[n][n2].resize(bigram_pairs[n][n2].size());
        //for (unsigned int i=0; i < bigram_pairs[n][n2].size() ; i ++) {
        //for (int j=0; j < bigram_pairs[n][n2][i].size(); j++) {
        //  bigram_bitset[n][n2][i][bigram_pairs[n][n2][i][j]] = 1;
        //}
        //}
      }
    }   
  }
}

void GraphDecompose::decompose( const ForestLattice * gr) {
  g = gr;
  graph_to_all_pairs();
  //all_pairs_to_bigram();
  compute_bigrams();
}

void GraphDecompose::graph_to_all_pairs() {

  // INITIALIZE DP CHART
  for (int n=0;n < g->num_nodes;n++) {

    // Initialize all INF
    for (int n2=0;n2< g->num_nodes;n2++) {
      //all_pairs_path_length[n][n2] = INF;
    }

    // If path exists set it to 1
    for (int j=0;j < g->node_edges[n]; j++) {
      int n2 = g->graph[n][j];
      //all_pairs_path_length[n][n2].push_back(1);
      // back pointer (split path)
      all_pairs_path[n][n2].push_back(n2);
      //for_updates[n2].push_back( Bigram(n, n2));
      //for_updates[n].push_back( Bigram(n, n2));
    }
  }
  
  

  // RUN Modified FLOYD WARSHALL
  for (int k=0;k < g->num_nodes; k++) {
    
    // lex nodes can't be in the middle of a path
    if (g->word_node[k]!= -1) continue;
    
    for (int n=0; n < g->num_nodes; n++) {
      if (all_pairs_path[n][k].empty()) continue;
      
      for (int n2=0; n2 < g->num_nodes; n2++) {
        if (!all_pairs_path[n][k].empty() &&  !all_pairs_path[k][n2].empty() ) {
          //for_updates[k].push_back( Bigram(n, n2));
          all_pairs_path[n][n2].push_back(k);  
          //assert(all_pairs_path_length[n][k] != INF);
          //assert(all_pairs_path_length[k][n2] != INF);
          
          //all_pairs_path
          //for (int i =0; i < all_pairs_path[n][k].size();i++) {
          //for (int j =0; j < all_pairs_path[k][n2].size();j++) {
              //all_pairs_path_length[n][n2].push_back(all_pairs_path_length[n][k][i] + all_pairs_path_length[k][n2][j] );
        
              
          //assert(k != n);

              //all_pairs_path[n][n2] = k;
          //}
          //}
        }
      }
    }
  }
}
  
  
void GraphDecompose::reconstruct_path(int n, int n2, vector <vector <int> > & array) {
  // find the path between nodes n and n2, and fill in array
  for (int split=0; split < all_pairs_path[n][n2].size(); split++) { 
  
    int k = all_pairs_path[n][n2][split];
    
    vector<vector <int> > one;
    vector<vector <int> > two; 
    
    assert(k != n);
    if (k != n2){
      reconstruct_path(n, k, one);    
      reconstruct_path(k, n2, two);
    

      int p = array.size();
      array.resize(array.size() + one.size()*two.size());
      
      //cout << "Size: " << one.size()*two.size() << endl;
      for (unsigned int i =0; i < one.size();i++) { 
        for (unsigned int j =0; j < two.size();j++) { 
          //cout << p  << endl;
          array[p].insert( array[p].end(), one[i].begin(), one[i].end());
          //array[p].push_back(k);
          array[p].insert( array[p].end(), two[j].begin(), two[j].end());
          p++;
        }
      }  
    } else {
      array.resize(array.size()+ 1);
      array[array.size()-1].push_back(k);
    }
  }
}
  
void GraphDecompose::all_pairs_to_bigram() {
  
  for (int n=0; n < g->num_nodes; n++) {
    if (g->word_node[n]==-1) continue;
    for (int n2=0; n2 < g->num_nodes; n2++) {
      if (g->word_node[n2]==-1) continue;
      
      if (all_pairs_path[n][n2].empty()) continue;
      
      reconstruct_path(n, n2, bigram_pairs[n][n2]);
      //if (bigram_pairs[n][n2].empty()) {
      //bigram_pairs[n][n2].resize(1);
      //}
      //cout << n << " " << n2 << endl;
      for (int i = 0; i < bigram_pairs[n][n2].size(); i++) {
        //bigram_pairs[n][n2][i].push_back(n2);
        //cout << "\t" << i << endl;  
        for (int j = 0; j < bigram_pairs[n][n2][i].size(); j++) {
          //cout << "\t\t" << bigram_pairs[n][n2][i][j] << endl;
        }
      }

      //cerr << bigram_pairs[n][n2].size() << " " << all_pairs_path_length[n][n2] << endl;
      //assert(bigram_pairs[n][n2].size() == all_pairs_path_length[n][n2]);
    }
  }
}

