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
    _tag_map =  new Cache <Hyperedge, Tag>(edge_count);
    _edge_map = new Cache <Tag, vector<const Hyperedge *> >(_tag_length);
  }

void Tagger::make_edge(const Hypergraph_Edge & edge, const Scarab::HG::Hyperedge * our_edge) {
  //cout << "Make Edge" << edge.HasExtension(has_dep) << endl;
  if ( edge.GetExtension(has_tagging)) { 
    const Tagging & ret_tag = 
      edge.GetExtension(tagging);
    
    Tag our_tag = make_tag(ret_tag.ind(), 
                           ret_tag.tag_id());
    //cout << our_tag << endl;
    _tag_map->set_value(*our_edge, our_tag);
    
    _edge_map->get_no_check(our_tag).push_back(our_edge);
  }
}
