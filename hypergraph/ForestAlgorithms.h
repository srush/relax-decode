#ifndef FORESTALGORITHMS_H_
#define FORESTALGORITHMS_H_

#include "svector.hpp"
#include "EdgeCache.h"
#include "Forest.h"
typedef Cache <ForestEdge, double> EdgeCache;
typedef Cache <ForestNode, double> NodeCache;
typedef Cache <ForestNode, const ForestEdge *> NodeBackCache;

EdgeCache * cache_edge_weights(const Forest & forest, const svector <int, double> & weight_vector );
EdgeCache* combine_edge_weights(const Forest & forest, const EdgeCache & w1, const EdgeCache & w2 );
vector <const ForestNode *> construct_best_fringe(const Forest & forest, const NodeBackCache & back_memo_table);
vector <int> construct_best_edges(const Forest & forest, const NodeBackCache & back_memo_table);
double best_path(const Forest & forest, const EdgeCache & edge_weights, NodeCache & score_memo_table, NodeBackCache & back_memo_table);

#endif 
