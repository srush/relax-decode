#include "ForestAlgorithms.h"

double best_path_helper(const ForestNode & node, const EdgeCache & edge_weights, NodeCache & score_memo_table, NodeBackCache & back_memo_table);
vector <string> construct_best_fringe_help(const ForestNode & node, const NodeBackCache & back_memo_table);
vector <int> construct_best_edges_help(const ForestNode & node, const NodeBackCache & back_memo_table);

EdgeCache * cache_edge_weights(const Forest & forest, const svector<int, double> & weight_vector ) {
  EdgeCache * weights = new EdgeCache();
  for (int i = 0; i < forest.num_edges(); i++) {
    const ForestEdge & edge = forest.get_edge(i);
    weights->set_value(edge, edge.fvector().dot(weight_vector));
  }
  return weights;
}


EdgeCache* combine_edge_weights(const Forest & forest, const EdgeCache & w1, const EdgeCache & w2 ) {
  EdgeCache * combine = new EdgeCache();
  for (int i = 0; i < forest.num_edges(); i++) {
    double val =0; 
    const ForestEdge & edge = forest.get_edge(i);
    val = w1.get_value(edge) + w2.get_value(edge);
    combine->set_value(edge, val);
  }
  return combine;
}

vector <string> construct_best_fringe(const Forest & forest, const NodeBackCache & back_memo_table) {
  return construct_best_fringe_help(forest.root(), back_memo_table);
}

vector <string> construct_best_fringe_help(const ForestNode & node, const NodeBackCache & back_memo_table) {
  vector <string> best; 
  if (node.is_word()) {
    best.push_back(node.word());
    return best;
  }
  
  const ForestEdge * edge = back_memo_table.get_value(node);
  
  for (int i =0; i < edge->num_nodes(); i++)  {
    const ForestNode & bnode = edge->tail_node(i);
    vector <string> b = construct_best_fringe_help(bnode, back_memo_table);
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
  
  const ForestEdge * edge = back_memo_table.get_value(node);
  best.push_back(edge->id());
  
  for (int i =0; i < edge->num_nodes(); i++)  {
    const ForestNode & bnode = edge->tail_node(i);
    vector <int> b = construct_best_edges_help(bnode, back_memo_table);
    for (int j=0; j < b.size(); j++ ) {
      best.push_back(b[j]);
    }
  }
  return best;
}



double best_path(const Forest & forest, const EdgeCache & edge_weights, NodeCache & score_memo_table, NodeBackCache & back_memo_table) {
  return  best_path_helper(forest.root(), edge_weights, score_memo_table, back_memo_table);
}


double best_path_helper(const ForestNode & node, const EdgeCache & edge_weights, NodeCache & score_memo_table, NodeBackCache & back_memo_table) {
  double best_score;
  const ForestEdge * best_edge; 
  if (score_memo_table.has_key(node)) {
    return score_memo_table.get_value(node);
  }

  for (int i=0; i< node.num_edges(); i++) {
    const ForestEdge & edge = node.edge(i);
    double edge_value= edge_weights.get_value(edge);
    for (int j=0; j < edge.num_nodes(); j++ ) {
      edge_value += best_path_helper(edge.tail_node(j), edge_weights, score_memo_table, back_memo_table);
    }

    if (edge_value < best_score) {
      best_score = edge_value;
      best_edge = &edge;
    }
  }
  
  score_memo_table.set_value(node, best_score);
  back_memo_table.set_value(node, best_edge);
  return best_score;
} 





