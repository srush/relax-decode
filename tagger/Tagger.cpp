#include "Tagger.h"

ostream& operator<<(ostream& output, const Tag& h) {
  output << h.ind << "/" << h.tag;
  return output;
} 


  void Tagger::set_up(const Hypergraph & hgraph) {
    //_sent_length = hgraph.GetExtension(len) + 1; 
    _sent_length = 150; 
    //cout << "len " <<   _sent_length << endl;

    int id_size = 0;
    for (int i=0; i < _sent_length; i++) {
      for (int t=0; t < num_tag; t++) {
        Tag tag = make_tag(i, t);
        _tags.push_back(tag);

        id_size = max(tag.id(), id_size );
      }
    }


    int edge_count = 0;
    for (int i = 0; i < hgraph.node_size(); i++) {
      const Hypergraph_Node & node = hgraph.node(i);
      for (int j=0; j < node.edge_size(); j++) {
        const Hypergraph_Edge& edge = node.edge(j);
        edge_count++;
      }
    }
    _tag_length = id_size + 1;
    _tag_map =  new Cache <Hypernode, Tag>(hgraph.node_size());
    _node_map = new Cache <Tag, HNodes  >(_tag_length);
  }


Hypernode * Tagger::make_node(const Hypergraph_Node & node, wvector * features) {
  Hypernode * our_node = new HypernodeImpl(node.label(), node.id(), features); 
  if ( node.GetExtension(has_tagging)) { 
    const Tagging & ret_tag = 
      node.GetExtension(tagging);
    
    Tag our_tag = make_tag(ret_tag.ind(), 
                           ret_tag.tag_id());
    //cout << our_tag << endl;
    _tag_map->set_value(*our_node, our_tag);

    cout << our_tag << endl;
    _node_map->get_no_check(our_tag).push_back( our_node);
  }
  return our_node;
}
