#ifndef HYPERGRAPHALGORITHMS_H_
#define HYPERGRAPHALGORITHMS_H_

#include "svector.hpp"
#include "EdgeCache.h"
#include "Hypergraph.h"

namespace Scarab {
  namespace HG {


typedef Cache <Hyperedge, double> EdgeCache;
typedef Cache <Hypernode, double> NodeCache;
typedef Cache <Hypernode, const Hyperedge *> NodeBackCache;

class HypergraphAlgorithms {
 public:
 HypergraphAlgorithms(const Hypergraph & hypergraph): _forest(hypergraph) {}

/** Associate a weight which each edge in the hypergraph
 *  @param weight_vector A weight vector
 *  @return A cache associated a weight with each edge
 */
EdgeCache * cache_edge_weights(const svector <int, double> & weight_vector ) const;

/** Combine two weight vectors 
 * fix this!
 */
EdgeCache* combine_edge_weights(const EdgeCache & w1, 
                                const EdgeCache & w2 ) const;

/** Given a hypergraph and back pointers, produces the left-to-right fringe
 *  @param forest The hypergraph 
 *  @param back_memo_table The associated back pointers (possibly obtained through best_path) 
 *  @return A const iterator of hypernodes in "inorder" order
 */
HNodes construct_best_fringe(const NodeBackCache & back_memo_table) const ;


/** Given a hypergraph and back pointers, produces the best edges used in the path
 *  @param back_memo_table The associated back pointers (possibly obtained through best_path) 
 *  @return A vector of const edges
 */
HEdges construct_best_edges(const NodeBackCache & back_memo_table) const;

/** Given a hypergraph and back pointers, produces the best nodes used in the path (in inorder order)
 *  
 *  @param back_memo_table The associated back pointers (possibly obtained through best_path) 
 *  @return A vector of const hypernodes
 */
HNodes construct_best_node_order(const NodeBackCache & back_memo_table) const;


/** Find the best path, lowest weight, through a weighted hypergraph
 *  @param edge_weights The cached edge weights associated with the graph
 *  @param score_memo_table The shortest path to each node
 *  @param back_memo_table The back pointers.
 *  @return Weight of shortest path
 */
double best_path(const EdgeCache & edge_weights, 
                 NodeCache & score_memo_table, 
                 NodeBackCache & back_memo_table) const;

/** Topologically sort the given hypergraph (immutable) 
 *  @return The ids of the hypergraph in topological order
 */
 HNodes topological_sort() const;
 private:
 const Hypergraph & _forest;

};

  }}
#endif 
