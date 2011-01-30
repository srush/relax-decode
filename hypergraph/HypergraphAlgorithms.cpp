#include "HypergraphAlgorithms.h"

#include <iostream>
#include <iomanip>
#include <set>
#include <vector>
#include <cy_svector.hpp>
#include "../common.h"
using namespace std;

namespace Scarab{
  namespace HG{



double best_path_helper(const Hypernode & node, const EdgeCache & edge_weights, NodeCache & score_memo_table, NodeBackCache & back_memo_table);
vector <const Hypernode *> construct_best_fringe_help(const Hypernode & node, const NodeBackCache & back_memo_table);
HEdges construct_best_edges_help(const Hypernode & node, const NodeBackCache & back_memo_table);

void best_outside_path_helper(const Hypernode & node, 
                              const EdgeCache & edge_weights, 
                              const NodeCache & score_memo_table,
                              NodeCache & outside_memo_table);


vector <const Hypernode *>  HypergraphAlgorithms::topological_sort() const {
  vector <const Hypernode * > top_sort;
  set <int> s;
  s.insert(_forest.root().id());
  top_sort.clear();
  Cache <Hyperedge, bool> removed(_forest.num_edges());
  while (!s.empty()) {
    int n = (*s.begin());
    s.erase(n);
    top_sort.push_back(&_forest.get_node(n));
    
    const Hypernode & node = _forest.get_node(n);
    foreach (const Hyperedge *edge, node.edges()) {
      //int i=0; i< node.num_edges(); i++) {
      //const Hyperedge & edge = node.edge(i); 
      removed.set_value(*edge, 1);

      //for (int j=0; j < edge.num_nodes(); j++) {
      foreach (const Hypernode * sub_node, edge->tail_nodes()) {  
        bool no_edges = true; 

        // have all the above edges been removed
        foreach (const Hyperedge * in_edge, sub_node->in_edges()) {
          //const Hyperedge & in_edge = sub_node.in_edge(k); 
          no_edges &= (removed.has_key(*in_edge) && removed.get_value(*in_edge));  
        }
        if (no_edges) {
          s.insert(sub_node->id());
        }
      }
    }
  }
  return top_sort;
} 


EdgeCache * HypergraphAlgorithms::cache_edge_weights(const svector<int, double> & weight_vector ) const {
  EdgeCache * weights = new EdgeCache(_forest.num_edges());
  
  foreach (const Hyperedge *edge, _forest.edges()) {
    double dot = edge->fvector().dot(weight_vector);
    //cout << svector_str(weight_vector) << endl;
    //cout << svector_str(edge.fvector()) << endl;
    //cout << dot<< endl;
    //cout << "Norm " << edge.fvector().normsquared() << " " << weight_vector.normsquared() << endl;
    //cout << "Dot " << edge.id() << " " << dot<< endl; 
    weights->set_value(*edge, dot);
  }
  return weights;
}


EdgeCache* HypergraphAlgorithms::combine_edge_weights( const EdgeCache & w1, const EdgeCache & w2 )  const {
  EdgeCache * combine = new EdgeCache(_forest.num_edges());
  foreach (const Hyperedge * edge, _forest.edges()) {
    double v1 = w1.get_value(*edge);
    double v2 = w2.get_value(*edge);
    combine->set_value(*edge, v1 + v2);
  }
  return combine;
}



vector <const Hypernode *> HypergraphAlgorithms::construct_best_fringe( const NodeBackCache & back_memo_table) const {
  return construct_best_fringe_help(_forest.root(), back_memo_table);
}

vector <const Hypernode *> construct_best_fringe_help(const Hypernode & node, const NodeBackCache & back_memo_table) {
  vector <const Hypernode *> best; 
  if (node.is_terminal()) {
    best.push_back(&node);
    return best;
  }
  
  const Hyperedge * edge = back_memo_table.get_value(node);
  
  foreach(const Hypernode * bnode, edge->tail_nodes()) { 
    vector <const Hypernode *> b = construct_best_fringe_help(*bnode, back_memo_table);
    best.insert(best.end(), b.begin(), b.end());
    
  }
  return best;
}

HEdges HypergraphAlgorithms::construct_best_edges(const NodeBackCache & back_memo_table) const {
  return construct_best_edges_help(_forest.root(), back_memo_table);
}

HEdges construct_best_edges_help(const Hypernode & node, const NodeBackCache & back_memo_table) {
  HEdges best; 
  
  if (node.num_edges() == 0) {
    //assert (node.is_word());
    return best;
  } else {
    const Hyperedge * edge = back_memo_table.get_value(node);
    best.push_back(edge);
  
    foreach (const Hypernode * bnode, edge->tail_nodes()) {
      vector <const Hyperedge *> b = construct_best_edges_help(*bnode, back_memo_table);
      foreach (const Hyperedge *  in_b, b) {
        best.push_back(in_b);
      }
    }
  }
  return best;
}


vector <const Hypernode *> construct_best_node_order_help(const Hypernode & node, const NodeBackCache & back_memo_table) {
  vector <const Hypernode * > best; 

  best.push_back(&node);

  if (node.num_edges() == 0) {
    //assert (node.is_word());
    cout << "w ";
  } else {
    cout << node.id() << "D ";
    const Hyperedge * edge = back_memo_table.get_value(node);  
    //for (int i =0; i < edge->num_nodes(); i++)  {
    foreach (const Hypernode * bnode, edge->tail_nodes() ) {
      //const Hypernode & bnode = edge->tail_node(i);
      vector <const Hypernode *> b = construct_best_node_order_help(*bnode, back_memo_table);
      foreach (const Hypernode * in_b, b) {
        best.push_back(in_b);
      }
    }
    cout << node.id() << "U ";
  }
  
  return best;
}

vector <const Hypernode * > HypergraphAlgorithms::construct_best_node_order(const NodeBackCache & back_memo_table) const {
  return construct_best_node_order_help(_forest.root(), back_memo_table);
}

HypergraphPrune HypergraphAlgorithms::pretty_good_pruning(const EdgeCache & edge_weights,
                                                          const NodeCache & score_memo_table, 
                                                          const NodeCache & outside_memo_table,
                                                          double cutoff) {
  HypergraphPrune prune(_forest);


  foreach (HNode node, _forest.nodes()) { 
    double node_outside = outside_memo_table.get(*node);
    double marginal = score_memo_table.get(*node) + node_outside;
    
    if ( marginal  <  cutoff ) { 
      prune.nodes.insert(node->id());
    }

    foreach (HEdge edge, node->edges()) {
      double total = 0.0;
      foreach (HNode sub_node, edge->tail_nodes()) {
        total += score_memo_table.get(*sub_node);
      }
      
      double edge_marginal = total + node_outside + edge_weights.get_value(*edge);
      if (edge_marginal < cutoff) {
        prune.edges.insert(edge->id());
      } 
    }
  }
  return prune;
}



double HypergraphAlgorithms::best_outside_path(const EdgeCache & edge_weights, 
                                               const NodeCache & score_memo_table, 
                                               NodeCache & outside_score_table) const {
  vector <const Hypernode *> node_order =
    HypergraphAlgorithms(_forest).topological_sort();

  foreach(HNode node, node_order) {
    outside_score_table.set_value(*node, INF);
  }

  foreach (HNode node, node_order) { 
    int id = node->id(); 
    //if (_out_done.find(id) == _out_done.end()) {
    best_outside_path_helper(*node, edge_weights, score_memo_table, outside_score_table);
      //}
  }
}

void best_outside_path_helper(const Hypernode & node, 
                              const EdgeCache & edge_weights, 
                              const NodeCache & score_memo_table,
                              NodeCache & outside_memo_table) {
  // when you get to a node it is done already 
  assert (outside_memo_table.has_key(node));
  //assert(_out_done.find(node.id()) == _out_done.end());
  double above_score = outside_memo_table.get_value(node);

  foreach (HEdge edge, node.edges()) {
    double edge_value= edge_weights.get_value(*edge);        
    double total = 0.0;
    foreach (HNode node, edge->tail_nodes()) {
      double node_inside = score_memo_table.get_value(*node); 
      total += node_inside;
    }

    foreach (HNode node, edge->tail_nodes()) {
      double node_inside = score_memo_table.get_value(*node); 
      double outside_score = edge_value + above_score + total - node_inside;
      double best_score = outside_memo_table.get(*node);
      if (outside_score < best_score) {
        //best_score = outside_score;
        //best_edge = edge;
        outside_memo_table.set_value(*node, outside_score);
      }
    }
  }
}


double HypergraphAlgorithms::best_path( const EdgeCache & edge_weights, NodeCache & score_memo_table, NodeBackCache & back_memo_table) const {
  return  best_path_helper(_forest.root(), edge_weights, score_memo_table, back_memo_table);
}

// find the best path through a hypergraph
double best_path_helper(const Hypernode & node, const EdgeCache & edge_weights, 
                        NodeCache & score_memo_table, NodeBackCache & back_memo_table) {
  double best_score = INF;
  //int id = node.id();

  const Hyperedge * best_edge = NULL; 
  if (score_memo_table.has_key(node)) {
    return score_memo_table.get_value(node);
  }
  
  //cout << "EDGES: "<< node.num_edges() <<endl; 
  if (node.num_edges() == 0) {
    
    //assert (node.is_word());
    best_score = 0.0;
    best_edge = NULL;
  } else {
    foreach (const Hyperedge * edge, node.edges()) { 
      
      double edge_value= edge_weights.get_value(*edge);
      foreach ( const Hypernode * tail_node, edge->tail_nodes()) {
        edge_value += best_path_helper(*tail_node, edge_weights, score_memo_table, back_memo_table);
      }
      //cout << edge_value << endl;
      if (edge_value < best_score) {
        best_score = edge_value;
        best_edge = edge;
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


  }}



