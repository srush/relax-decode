#include "GraphProtoInterface.h"

#include <fstream>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

using namespace std;

void GraphProtoInterface::build_from_file(const char * file_name) { 
  graph::Graph * graph = new graph::Graph();     
  {
    fstream input(file_name, ios::in | ios::binary);
    google::protobuf::io::IstreamInputStream fs(&input);
    
    google::protobuf::io::CodedInputStream coded_fs(&fs);
    coded_fs.SetTotalBytesLimit(1000000000, -1);
    graph->ParseFromCodedStream(&coded_fs);
  }

  vector <Graphnode *> nodes;
  vector <Graphedge *> edges;


  int edge_count = 0;
  for (int i = 0; i < graph->node_size(); i++) {
    const graph::Graph_Node & node = graph->node(i);
    for (int j=0; j < node.edge_size(); j++) {
      const graph::Graph_Edge& edge = node.edge(j);
      edge_count++;
    }
  }

  set_up(*graph, graph->node_size(), edge_count);


  for (int i = 0; i < graph->node_size(); i++) {
    const graph::Graph_Node & node = graph->node(i);

    Graphnode * my_node = new Graphnode(node.id());
    my_node->set_label(node.label());
    process_node(node, my_node);
    nodes.push_back(my_node);
  }

  uint edge_id = 0;
  for (int i = 0; i < graph->node_size(); i++) {
    const graph::Graph_Node& node = graph->node(i);

    for (int j=0; j < node.edge_size(); j++) {
      const graph::Graph_Edge& edge = node.edge(j);
      int to_node = edge.to_node();

      
      Graphedge * my_edge = new Graphedge(edge_id, *nodes[node.id()], *nodes[to_node]);
      process_edge(edge, my_edge);      

      //((HypernodeImpl*)_nodes[to_node])->add_edge(forest_edge);
      nodes[node.id()]->add_edge(my_edge);
      nodes[my_edge->to_node()->id()]->add_in_edge(my_edge);
      edge_id++;
      edges.push_back(my_edge);
    }
  }

  Nodes * ns  = new Nodes ();
  Edges * es = new Edges ();
  
  foreach (Graphnode * n, nodes) {
    ns->push_back((Node) n);
  }
  foreach (Graphedge * e, edges) {
    es->push_back((Edge)e);
  }
  _graph = new Graph(*ns, *es);

  delete graph;
 
  
}



