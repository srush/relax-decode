#include "ForestLattice.h"
#include <iostream>
#include <iomanip>
#include <string>
#include "../common.h"
using namespace std;

ostream& operator<<(ostream& os, const Word& w){ 
   os << w.id() << endl;
  return os;
}

ostream& operator<<(ostream& os, const WordBigram& w){
  os << w.w1 << " " << w.w2 << endl;
  return os;
}

void ForestLattice::make_proper_graph(const Lattice & lat) {
  vector <Graphnode*> nodes;
  Edges all_edges;
  for (int i = 0; i < lat.node_size(); i++) {
    const Lattice_Node & node =  lat.node(i);
    assert(node.id() == nodes.size());
    nodes.push_back(new Graphnode(node.id()));
  }
  

  for (int i = 0; i < lat.node_size(); i++) {
    const Lattice_Node & node =  lat.node(i);

    Edges local_edges;
    for (int j =0; j < node.edge_size(); j++) {
      const Lattice_Edge & edge = node.edge(j);
      
      Graphedge * local_edge = new Graphedge(edge.id(), *nodes[node.id()], *nodes[edge.to_id()]);
      
      all_edges.push_back(local_edge);
      local_edges.push_back(local_edge);

    }
    nodes[i]->set_edges(local_edges);
  }
  
  Nodes final_nodes;
  foreach ( Graphnode *node, nodes) 
    final_nodes.push_back((Node ) node );
  _proper_graph = new Graph(final_nodes, all_edges);

}


ForestLattice::ForestLattice(const Lattice & lat):
  num_word_nodes(lat.GetExtension(num_original_ids)),
  _first_words(lat.node_size()), 
  _last_words(lat.node_size()),
  bigrams_at_node(lat.node_size()),
  _words(num_word_nodes),
  _lat_word_to_hyp_node(num_word_nodes),
  _words_lookup(num_word_nodes)
 {
  int num_nodes = lat.node_size();
  int num_hyper_edges = lat.GetExtension(num_hypergraph_edges);
  _is_word.resize(num_word_nodes);
  original_edges.resize(num_hyper_edges);
  //original_edges_position.resize(num_hyper_edges);
  edges_original.resize(num_word_nodes);
  
  word_node.resize(num_nodes);
  edge_node.resize(num_nodes);
  ignore_nodes.resize(num_nodes);
  final.resize(num_nodes);
  node_edges.resize(num_nodes);
  //graph.resize(num_nodes);
  _edge_by_nodes.resize(num_nodes);
  _edge_label_by_nodes.resize(num_word_nodes);
  //_words_lookup.resize(lat.GetExtension(num_original_ids));
  
  _last_bigrams.resize(num_nodes);
  _last_same.resize(num_word_nodes);

  //_lat_word_to_hyp_node.resize(num_word_nodes);
  _hyp_node_to_lat_word.resize(num_word_nodes);

  make_proper_graph(lat);

  for (int i =0; i< num_word_nodes; i++) {
  
    _last_same[i] = -1;
  }
 
 _original_id_to_edge.resize(num_word_nodes);
 //int same =0;
  for (int i = 0; i < lat.node_size(); i++) {
    const Lattice_Node & node =  lat.node(i);
    const Graphnode & gnode = _proper_graph->node(node.id());

    //cout << node.id()<<endl;
    //assert ((int)_nodes.size() == node.id());
    
    
    node_edges[node.id()] = node.edge_size();
    //graph[node.id()].resize(node.edge_size());
    _edge_by_nodes[node.id()].resize(num_nodes);
    //_edge_label_by_nodes[node.id()].resize(num_nodes);


    for (int j =0; j < lat.node_size(); j++) {
      _edge_by_nodes[node.id()][j] = -1;
    }

    if (node.GetExtension(has_phrases)) {
      const Phraselets & plets  = node.GetExtension(phraselets);
      int size = plets.phraselet_size();

      for (int i =0; i < size; i ++) {
        const Phraselet & plet= plets.phraselet(i);
        for (int j=0; j < plet.word_size(); j++) {
          const Subword & word = plet.word(j);
          int hyper_edge = plet.phraselet_hypergraph_edge();
          Word our_word = Word(word.subword_original_id());

          if (word.subword_hypergraph_node_id()!= -1) {
            _lat_word_to_hyp_node.set_value(our_word,  word.subword_hypergraph_node_id());
            _hyp_node_to_lat_word[word.subword_hypergraph_node_id()] = word.subword_original_id();
          }
          _words_lookup.set_value(our_word, &gnode);
          _words.set_value(our_word, word.word());

          _is_word[word.subword_original_id()] = 1;
          if (hyper_edge != -1) {
            int hypergraph_edge_position = plet.hypergraph_edge_position() - (j + 1);
            if (original_edges[hyper_edge].size() <= hypergraph_edge_position) {
              original_edges[hyper_edge].resize(hypergraph_edge_position + 1);
            }

            original_edges[hyper_edge][hypergraph_edge_position] = word.subword_original_id();
            // Assuming the lattice is reversed.
            //original_edges_position[hyper_edge].push_back(plet.hypergraph_edge_position() - (j + 1));
          }
          if (j < plet.word_size() -1) {
            bigrams_at_node.get_no_check(gnode).push_back(WordBigram(our_word,
                                                                     Word(plet.word(j+1).subword_original_id())));
          }
          
        }
        assert(plet.word_size() > 0); 
        int last = plet.word_size()-1;
        assert(plet.word(0).subword_original_id() != 0);
        
        _first_words.get_no_check(gnode).push_back(Word(plet.word(0).subword_original_id()));


        //for (uint i =0 ; i < _last_words.get(node).size(); i++) {
        foreach (const Word & w, _last_words.get_default(gnode, vector<Word>())) { 
          if (plet.word(last).word() == _words.get(w)) {
            // this position is the same as some previous
            _last_same[plet.word(last).subword_original_id()] = w.id();
            break;
          }
        }

        _last_words.get_no_check(gnode).push_back(Word(plet.word(last).subword_original_id()));
        
        if (plet.word_size() >= 2) {
          Bigram b(plet.word(last-1).subword_original_id(), 
                   plet.word(last).subword_original_id());
          
          _last_bigrams[node.id()].push_back(b);
        }
      }
    }
    

    for (int j =0; j < node.edge_size(); j++) {
      const Lattice_Edge & edge = node.edge(j);
      
      //graph[node.id()][j] = edge.to_id();
      
      string label = edge.label();

      const Origin & orig = edge.GetExtension(origin);
      if (orig.has_origin()) {
        int original_id = orig.original_id();
        for (int k =0; k < orig.hypergraph_edge_size(); k++) {
          assert(orig.hypergraph_edge_size() == orig.hypergraph_edge_position_size());
          int hypergraph_edge = orig.hypergraph_edge(k);
          int hypergraph_edge_position = orig.hypergraph_edge_position(k);
          if (original_edges[hypergraph_edge].size() <= hypergraph_edge_position) {
            original_edges[hypergraph_edge].resize(hypergraph_edge_position + 1);
          }
          original_edges[hypergraph_edge][hypergraph_edge_position] = original_id;
          //original_edges_position[hypergraph_edge][hypergraph_edge_position];
          edges_original[original_id].push_back(hypergraph_edge);
         
        }
        //edges[]
        _edge_by_nodes[node.id()][edge.to_id()] = original_id;
        _edge_label_by_nodes[original_id] = label;
        //cout << "EDGE " << original_id << " " << node.id() << " " << edge.to_id() << endl;
        _original_id_to_edge[original_id] = Bigram(node.id(), edge.to_id());
      } else {
        //cout << "NO EDGE " << node.id() << " " << edge.to_id() << " " << endl;
        _edge_by_nodes[node.id()][edge.to_id()] = -1;
      }

    }


    if (node.GetExtension(has_phrases)) {
      word_node[node.id()] = 1; //node.getExtension(word);
      edge_node[node.id()] = -1;
      //cout << node.GetExtension(word) << endl;
      //cout <<node.id() << endl;
      //_words[node.id()]= node.GetExtension(word);
      _phrase_nodes.push_back(&_proper_graph->node(node.id()));
    } else {
      word_node[node.id()] = -1;
      edge_node[node.id()] = 1;
    }
  }

  start = lat.start();
  for (int i=0; i < lat.final_size(); i++) {
    final[lat.final(i)] = 1;
  }  
} 


