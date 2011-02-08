#include "HypergraphImpl.h"
#include "hypergraph.pb.h"
#include "features.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
using namespace google::protobuf::io;

namespace Scarab {
namespace HG {


void HypergraphImpl::prune(const HypergraphPrune & prune) {
  vector <Hypernode *> new_nodes;
  vector <Hyperedge *> new_edges; 

  int node_count = 0;
  foreach (Hypernode * tmp_node, nodes()) {
    HypernodeImpl * node = (HypernodeImpl *)tmp_node;
    if (prune.nodes.find(node->id()) != prune.nodes.end()) {
      new_nodes.push_back((Hypernode*)node);
      node->prune_edges(prune.edges);
      node->reid(node_count);
      node_count++;
    }
  }

  int edge_count = 0;
  foreach (Hyperedge * tmp_edge, edges()) {
    HyperedgeImpl * edge = (HyperedgeImpl *)tmp_edge;
    if (prune.edges.find(edge->id()) != prune.edges.end()) {
      new_edges.push_back((Hyperedge*)edge);
      edge->reid(edge_count);
      edge_count++;
    }
  }

  _nodes = new_nodes;
  _edges = new_edges;
  
} 


void HypergraphImpl::write_to_file(const char * file_name) {
  Hypergraph hgraph;


  foreach (HNode my_node, nodes()) {
    Hypergraph_Node * node = hgraph.add_node();
    
    //string feat_str = svector_str<int, double>(my_node->fvector()); 
    //node->SetExtension(node_fv, feat_str);
    
    convert_node(my_node, node);
    //str_vector * features = svector_from_str<int, double>(feat_str);
    
    // 
    //Hypernode * forest_node = make_node(node, features);

    //assert (forest_node->
    
    foreach (HEdge my_edge, my_node->edges()) {
      Hypergraph_Edge *edge = node->add_edge();
      str_vector * features;
      string feat_str = svector_str<int, double>(my_edge->fvector()); 
      edge->SetExtension(edge_fv, feat_str);

      foreach (HNode sub_node, my_edge->tail_nodes()) {
        //int id = edge.tail_node_ids(k); 
        edge->add_tail_node_ids(sub_node->id()); //push_back( _nodes[id]);
      }             
      convert_edge(my_edge, edge);
    }
  }
  //assert (_nodes.size() == (uint)hgraph->node_size());
  hgraph.set_root(hgraph.root());
 

  {
    fstream output(file_name, ios::out | ios::binary);
    if (!hgraph.SerializeToOstream(&output)) {
      assert (false);
    } 
  }
}

void HypergraphImpl::build_from_file(const char * file_name) { 
  hgraph = new ::Hypergraph();     
  {
    //int fd = open(file_name, O_RDONLY);
    fstream input(file_name, ios::in | ios::binary);
    google::protobuf::io::IstreamInputStream fs(&input);
    
    google::protobuf::io::CodedInputStream coded_fs(&fs);
    coded_fs.SetTotalBytesLimit(1000000000, -1);
    hgraph->ParseFromCodedStream(&coded_fs);
   
    //if (!hgraph->ParseFromIstream(&input)) {
    //assert (false);
    //} 
  }

  set_up(*hgraph);

  assert (hgraph->node_size() > 0);
  for (int i = 0; i < hgraph->node_size(); i++) {
    const Hypergraph_Node & node = hgraph->node(i);
    
    string feat_str = node.GetExtension(node_fv);
    str_vector * features = svector_from_str<int, double>(feat_str);
    
    // 
    Hypernode * forest_node = make_node(node, features);

    //assert (forest_node->
    assert (_nodes.size() == (uint)node.id());
    _nodes.push_back(forest_node);
    assert(_nodes[forest_node->id()]->id() == forest_node->id());
  }

  int edge_id = 0;
  for (int i = 0; i < hgraph->node_size(); i++) {
    const Hypergraph_Node& node = hgraph->node(i);
    assert (node.id()  == i);

    for (int j=0; j < node.edge_size(); j++) {
      const Hypergraph_Edge& edge = node.edge(j);
      str_vector * features;
      if (edge.HasExtension(edge_fv)) { 
        const string & feat_str = edge.GetExtension(edge_fv);  
        features = svector_from_str<int, double>(feat_str);
      } else {
        features = new svector<int, double>();
      }

      vector <Scarab::HG::Hypernode *> tail_nodes;
      for (int k =0; k < edge.tail_node_ids_size(); k++ ){
        int id = edge.tail_node_ids(k); 
        tail_nodes.push_back( _nodes[id]);
      } 
      

      
      Scarab::HG::Hyperedge * forest_edge = new Scarab::HG::HyperedgeImpl(edge.label(), 
                                                                          features, 
                                                                          edge_id, 
                                                                          tail_nodes, 
                                                                          _nodes[node.id()]);
      
      make_edge(edge, forest_edge);      
      for (int k =0; k < edge.tail_node_ids_size(); k++ ){
        int id = edge.tail_node_ids(k);
        ((HypernodeImpl*)_nodes[id])->add_in_edge(forest_edge);
      }
      ((HypernodeImpl*)_nodes[node.id()])->add_edge(forest_edge);

      edge_id++;
      //int for_edge_id = forest_edge->id();
      _edges.push_back(forest_edge);//[for_edge_id] = forest_edge;
    }
    //cout << node.id() << " "<<  _nodes[node.id()]->num_edges() << " " << node.edge_size() << " " << _nodes[node.id()]->is_word() << endl;
    assert (_nodes[node.id()]->num_edges() == (uint)node.edge_size() );
  }
  assert (_nodes.size() == (uint)hgraph->node_size());
  int root_num = hgraph->root();
  _root = _nodes[hgraph->root()];//_nodes[_nodes.size()-1];
  delete hgraph;
}
}
}
