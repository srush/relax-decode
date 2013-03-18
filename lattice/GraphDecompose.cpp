#include "ForestLattice.h"

#include <vector>
#include <bitset>
#include <iostream>
#include <assert.h>
#include "GraphDecompose.h"
using namespace std;


    
void GraphDecompose::compute_bigrams() {
  // Simple bigram collecting
  foreach (Node n, _lat.phrase_nodes() ) {
    // first add bigrams within n
    const vector<WordBigram> &bigrams = _lat.get_bigrams_at_node(*n);
    if (bigrams.size() > 0) {
    foreach (const WordBigram &b, bigrams) {
      _valid_bigrams.push_back(WordBigram(b.w1, b.w2));
      forward_bigrams[b.w1.id()].push_back(b.w2.id());     
      backward_bigrams[b.w2.id()].push_back(b.w1.id());     
    }
    }
  }

  foreach (Node n, _lat.phrase_nodes()) {
    foreach (Node n2, _lat.phrase_nodes()) {
      if (n->id() == n2->id()) continue;
      if (!all_pairs_path_exist[n->id()][n2->id()]) continue; 
      foreach (const Word & w1, _lat.last_words(*n)) {
        foreach (const Word & w2, _lat.first_words(*n2)) {
          _valid_bigrams.push_back(WordBigram(w1, w2));
          forward_bigrams[w1.id()].push_back(w2.id());        
          backward_bigrams[w2.id()].push_back(w1.id());        
        }
      }   
    }
  }
}

void GraphDecompose::decompose() {
  int num_nodes = _lat.get_graph().num_nodes();
  forward_bigrams.resize(_lat.num_word_nodes);
  backward_bigrams.resize(_lat.num_word_nodes);
  
  for (int w1 =0; w1 < _lat.num_word_nodes; w1++) {
    backward_bigrams[w1].clear();
    forward_bigrams[w1].clear();
  }

  all_pairs_path.resize(num_nodes);
  all_pairs_path_exist.resize(num_nodes);

  for (int i =0; i < num_nodes; i++ ) {
    all_pairs_path_exist[i].resize(num_nodes);
    all_pairs_path[i].resize(num_nodes);
  }
  //g = gr;
  graph_to_all_pairs();
  //all_pairs_to_bigram();
  compute_bigrams();
}

void GraphDecompose::graph_to_all_pairs() {

  // INITIALIZE DP CHART
  //for (int n=0;n < _lat.num_nodes;n++) {
  foreach (Node n, _lat.get_graph().nodes()) {

    // Initialize all INF
    foreach (Node n2, _lat.get_graph().nodes()) {
      all_pairs_path_exist[n->id()][n2->id()] =0;
    }
    
    
    // If path exists set it to 1
    foreach (Edge e, n->edges()) {
      Node n2 = e->to_node();
      all_pairs_path_exist[n->id()][n2->id()] =1;
      all_pairs_path[n->id()][n2->id()] = new vector <Node>();
      all_pairs_path[n->id()][n2->id()]->push_back(n2);      
    }
    all_pairs_path_exist[n->id()][n->id()] =1;
    all_pairs_path[n->id()][n->id()] = new vector<Node>();
  }
  
  

  // RUN Modified FLOYD WARSHALL
  //for (int k=0;k < _lat.get_graph().num_nodes(); k++) {
  foreach (Node k, _lat.get_graph().nodes()) {
    // lex nodes can't be in the middle of a path
    if (_lat.is_phrase_node(*k)) continue;
    
    //for (int n=0; n < _lat.get_graph().num_nodes(); n++) {
    foreach(Node n, _lat.get_graph().nodes()) {
      if (!path_exists(*n,*k)) continue;
      if (n->id() == k->id()) continue;
      
      foreach(Node n2, _lat.get_graph().nodes()) {
        if (n2->id() == n->id()) continue;
        if (k->id() == n2->id()) continue;
        if (path_exists(*n, *k) &&  path_exists(*k, *n2) ) {
 
          if (!path_exists(*n, *n2)) {
            all_pairs_path_exist[n->id()][n2->id()] =1;
            all_pairs_path[n->id()][n2->id()] = new vector<Node>();
          }
          all_pairs_path[n->id()][n2->id()]->push_back(k);  
        }
      }
    }
  }
}
  
  
// void GraphDecompose::reconstruct_path(const Graphnode & n, const Graphnode & n2, vector <vector <int> > & array) {
//   // find the path between nodes n and n2, and fill in array
//   foreach (Node k,  *(all_pairs_path[n][n2])) {    
//     vector<vector <int> > one;
//     vector<vector <int> > two; 
    
//     assert(k->id() != n);
//     if (k->id() != n2){
//       reconstruct_path(n, *k, one);    
//       reconstruct_path(*k, n2, two);
    

//       int p = array.size();
//       array.resize(array.size() + one.size()*two.size());
      
//       //cout << "Size: " << one.size()*two.size() << endl;
//       for (unsigned int i =0; i < one.size();i++) { 
//         for (unsigned int j =0; j < two.size();j++) { 
//           //cout << p  << endl;
//           array[p].insert( array[p].end(), one[i].begin(), one[i].end());
//           //array[p].push_back(k);
//           array[p].insert( array[p].end(), two[j].begin(), two[j].end());
//           p++;
//         }
//       }  
//     } else {
//       array.resize(array.size()+ 1);
//       array[array.size()-1].push_back(k->id());
//     }
//   }
// }
   

