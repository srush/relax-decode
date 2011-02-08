#ifndef GRAPH_H_
#define GRAPH_H_
#include <vector>
using namespace std;

namespace Scarab {
namespace Graph {


// Graph interface for internal use. 
// This interface is entirely immutable! Every function is const
// See Cache.h for implementing state on top of graphs.  

class Graphnode;

typedef const Graphnode * Node; 

typedef vector <const Graphnode * > Nodes;  

// Base class for weighted edge 
class Graphedge {
 public:
 Graphedge(uint id, const Graphnode &  from, const Graphnode & to ) : _id(id), _from_node(from), _to_node(to) {}
 virtual ~Graphedge() {}

  /** 
   * Get edge id
   * 
   * @return The id of this edge in a fixed hypergraph
   */
 uint id() const {
   return _id;
 }

 Node from_node() const {
   return &_from_node;
 }

 Node to_node() const {
   return &_to_node;
 }

 private:
 uint _id;
 const Graphnode & _from_node;
 const Graphnode & _to_node;
};


typedef const Graphedge * Edge;
typedef vector <const Graphedge * > Edges;  

class Graphnode {
 public:
 Graphnode(uint id) : _id(id){}
   virtual ~Graphnode() {}
  /** 
   * The unique identifier of the Hypernode 
   * @deprecated
   * 
   * @return The number 
   */
   uint id() const {return _id;};

  // This interface is deprecated, use iterators below instead

  /** 
   * Get the number of edges connected to this node
   * @deprecated
   * 
   * @return The number 
   */
    uint num_edges() const {
     return _edges.size();
   }  
 
  // TODO: These should be (lazy) iterators, figure that part out
  /** 
   * Get all hyperedges with this hypernode as head.
   * WARNING: Treat this as a const iterator.
   * @return Const iterator to edges.
   */
   const Edges & edges() const {
     return _edges;
   } 
   
   void set_edges(Edges edges ) {
     _edges = edges;
   }
 private:
   uint _id;
   Edges _edges;
   
};

class Graph {
 public:
   Graph(Nodes nodes, Edges edges):_nodes(nodes), _edges(edges) {}
  /** 
   * Display the graph for debugging. 
   * 
   */
  //virtual void print() const = 0;
    
  // TODO: remove these 
  uint num_edges() const {return _nodes.size();}
  uint num_nodes() const {return _edges.size();}
    
  
  const Nodes & nodes() const {
    return _nodes;
  } 

  const Edges & edges() const {
    return _edges;
  } 

  const Graphnode & node(uint i) const {
    return *_nodes[i];
  }
 
 private:
  const Nodes & _nodes;
  const Edges & _edges;
  
};
 
}
}
#endif
