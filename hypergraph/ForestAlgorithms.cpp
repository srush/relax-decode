#include "ForestAlgorithms.h"

#include <iostream>
#include <iomanip>
#include <set>
#include <vector>
#include <cy_svector.hpp>

using namespace std;
#define INF 1e20


double best_path_helper(const ForestNode & node, const EdgeCache & edge_weights, NodeCache & score_memo_table, NodeBackCache & back_memo_table);
vector <const ForestNode *> construct_best_fringe_help(const ForestNode & node, const NodeBackCache & back_memo_table);
vector <int> construct_best_edges_help(const ForestNode & node, const NodeBackCache & back_memo_table);

void topological_sort(const Forest & forest, vector <int> & top_sort) {
  set <int> s;
  s.insert(forest.root().id());
  top_sort.clear();
  Cache <ForestEdge, bool> removed(forest.num_edges());
  while (!s.empty()) {
    int n = (*s.begin());
    s.erase(n);
    top_sort.push_back(n);
    
    const ForestNode & node = forest.get_node(n);
    for (int i=0; i< node.num_edges(); i++) {
      const ForestEdge & edge = node.edge(i); 
      removed.set_value(edge, 1);

      for (int j=0; j < edge.num_nodes(); j++) {
        const ForestNode & sub_node = edge.tail_node(j);
        bool no_edges = true; 

        // have all the above edges been removed
        for (int k=0; k < sub_node.num_in_edges(); k++) {
          const ForestEdge & in_edge = sub_node.in_edge(k); 
          no_edges &= (removed.has_key(in_edge) && removed.get_value(in_edge));  
        }
        if (no_edges) {
          s.insert(sub_node.id());
        }
      }
    }
  }
  
} 



EdgeCache * cache_edge_weights(const Forest & forest, const svector<int, double> & weight_vector ) {
  EdgeCache * weights = new EdgeCache(forest.num_edges());
  
  for (int i = 0; i < forest.num_edges(); i++) {
    const ForestEdge & edge =forest.get_edge(i);
    double dot = edge.fvector().dot(weight_vector);
    //cout << svector_str(weight_vector) << endl;
    //cout << svector_str(edge.fvector()) << endl;
    //cout << dot<< endl;
    //cout << "Norm " << edge.fvector().normsquared() << " " << weight_vector.normsquared() << endl;
    //cout << "Dot " << edge.id() << " " << dot<< endl; 
    weights->set_value(edge, dot);
    
  }
  return weights;
}


EdgeCache* combine_edge_weights(const Forest & forest, const EdgeCache & w1, const EdgeCache & w2 ) {
  EdgeCache * combine = new EdgeCache(forest.num_edges());
  for (int i = 0; i < forest.num_edges(); i++) {
    const ForestEdge & edge = forest.get_edge(i);
    double v1 = w1.get_value(edge);
    double v2 = w2.get_value(edge);
    combine->set_value(edge, v1 + v2);
  }
  return combine;
}



vector <const ForestNode *> construct_best_fringe(const Forest & forest, const NodeBackCache & back_memo_table) {
  return construct_best_fringe_help(forest.root(), back_memo_table);
}

vector <const ForestNode *> construct_best_fringe_help(const ForestNode & node, const NodeBackCache & back_memo_table) {
  vector <const ForestNode *> best; 
  if (node.is_word()) {
    best.push_back(&node);
    return best;
  }
  
  const ForestEdge * edge = back_memo_table.get_value(node);
  
  for (int i =0; i < edge->num_nodes(); i++)  {
    const ForestNode & bnode = edge->tail_node(i);
    vector <const ForestNode *> b = construct_best_fringe_help(bnode, back_memo_table);
    for (int j=0; j < b.size(); j++ ) {
      best.push_back(b[j]);
    }
  }
  return best;
}

vector <int> construct_best_edges(const Forest & forest, const NodeBackCache & back_memo_table) {
  return construct_best_edges_help(forest.root(), back_memo_table);
}

vector <int> construct_best_edges_help(const ForestNode & node, const NodeBackCache & back_memo_table) {
  vector <int> best; 
  
  if (node.num_edges() == 0) {
    //assert (node.is_word());
    return best;
  } else {
    const ForestEdge * edge = back_memo_table.get_value(node);
    best.push_back(edge->id());
  
    for (int i =0; i < edge->num_nodes(); i++)  {
      const ForestNode & bnode = edge->tail_node(i);
      vector <int> b = construct_best_edges_help(bnode, back_memo_table);
      for (int j=0; j < b.size(); j++ ) {
        best.push_back(b[j]);
      }
    }
  }
  return best;
}


vector <int> construct_best_node_order_help(const ForestNode & node, const NodeBackCache & back_memo_table) {
  vector <int> best; 

  best.push_back(node.id());

  if (node.num_edges() == 0) {
    //assert (node.is_word());
    cout << "w ";
  } else {
    cout << node.id() << "D ";
    const ForestEdge * edge = back_memo_table.get_value(node);  
    for (int i =0; i < edge->num_nodes(); i++)  {
      const ForestNode & bnode = edge->tail_node(i);
      vector <int> b = construct_best_node_order_help(bnode, back_memo_table);
      for (int j=0; j < b.size(); j++ ) {
        best.push_back(b[j]);
      }
    }
    cout << node.id() << "U ";
  }
  
  return best;
}

vector <int> construct_best_node_order(const Forest & forest, const NodeBackCache & back_memo_table) {
  return construct_best_node_order_help(forest.root(), back_memo_table);
}


double best_path(const Forest & forest, const EdgeCache & edge_weights, NodeCache & score_memo_table, NodeBackCache & back_memo_table) {
  return  best_path_helper(forest.root(), edge_weights, score_memo_table, back_memo_table);
}



// find the best path through a hypergraph
double best_path_helper(const ForestNode & node, const EdgeCache & edge_weights, 
                        NodeCache & score_memo_table, NodeBackCache & back_memo_table) {
  double best_score = INF;
  int id = node.id();

  const ForestEdge * best_edge = NULL; 
  if (score_memo_table.has_key(node)) {
    return score_memo_table.get_value(node);
  }
  
  //cout << "EDGES: "<< node.num_edges() <<endl; 
  if (node.num_edges() == 0) {
    
    //assert (node.is_word());
    best_score = 0.0;
    best_edge = NULL;
  } else {
    for (int i=0; i< node.num_edges(); i++) {
      const ForestEdge & edge = node.edge(i);
      double edge_value= edge_weights.get_value(edge);
      for (int j=0; j < edge.num_nodes(); j++ ) {
        edge_value += best_path_helper(edge.tail_node(j), edge_weights, score_memo_table, back_memo_table);
      }
      //cout << edge_value << endl;
      if (edge_value < best_score) {
        best_score = edge_value;
        best_edge = &edge;
      }
    }
  }
  //cout << "EDGES: "<< node.num_edges() <<endl; 
  assert (best_score != INF);
  //assert (best_edge != NULL || node.is_word());
  
  score_memo_table.set_value(node, best_score);
  back_memo_table.set_value(node, best_edge);
  return best_score;
} 






