#ifndef GRAPHPROTOINTERFACE
#define GRAPHPROTOINTERFACE
#include "graph.pb.h"
#include "Graph.h"
using namespace Scarab::Graph;
class GraphProtoInterface {
 public:
  void build_from_file(const char * file_name) ;

  
  virtual void process_node(graph::Graph_Node, Graphnode *) = 0;
  virtual void process_edge(graph::Graph_Edge, Graphedge *) = 0;
  virtual void set_up(graph::Graph, int, int) { }     
  const Graph & graph() const  {
    return *_graph;
  } 
  Graph * _graph;

 protected:


};

#endif
