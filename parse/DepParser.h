#ifndef DEPPARSER_H_
#define DEPPARSER_H_

#include "dep.pb.h"
#include "Hypergraph.h"
#include "HypergraphImpl.h"
#include "EdgeCache.h"
using namespace Scarab::HG;

struct Dependency {
  int head;
  int mod;
  int length;
  Dependency(){}
  Dependency(int l, int h, int m): length(l), head(h), mod(m){
    _id= head * l + mod;
  }
  
  int id() const {return _id;}
private:
  int _id;
};


ostream& operator<<(ostream& output, const Dependency& h);




class DepParser : public Scarab::HG::HypergraphImpl {
 public:
  ~DepParser() {
    //delete _edge_map;
  }
  // DepParser(int length, Hypernode)
  void print() const {}
  
  void set_up(const Hypergraph & hgraph) {
    _sent_length = hgraph.GetExtension(len) + 1; 
    //cout << "len " <<   _sent_length << endl;

    int id_size = 0;
    for (int h=0; h < _sent_length; h++) {
      for (int m=0;m < _sent_length; m++) {
        if (h == m) continue;
        if (m == 0) continue;
        Dependency dep = make_dep(h,m);
        _dependencies.push_back(dep);
        //cout << "ID " << make_dep(h,m).id() << endl;
        id_size = max(dep.id(), id_size );
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
    _dep_map =  new Cache <Hyperedge, Dependency>(edge_count);
    _edge_map = new Cache <Dependency, const Hyperedge *>(id_size + 1);
  }

  const Hypergraph & hypergraph() const {
    return *_h;
  }
  
  //const Hypergraph & weights() const {
  //return *_weights;
  //}

  vector <Dependency > dependencies() const {
    return _dependencies;
  }

  uint num_deps() const {
    return _dependencies.size();
  }

  Dependency make_dep(int head, int mod) {
    return Dependency(_sent_length, head, mod);
  }

  const Hyperedge & dep_to_edge(const Dependency & dep) const {
    return *_edge_map->get(dep);
  }

  const Dependency & edge_to_dep(const Hyperedge & edge) const {
    return _dep_map->get(edge);
  }

  bool edge_has_dep(const Hyperedge & edge) const {
    return _dep_map->has_key(edge);
  }
  
 protected:
  void make_edge(const Hypergraph_Edge & edge, const Scarab::HG::Hyperedge * our_edge) {
    //cout << "Make Edge" << edge.HasExtension(has_dep) << endl;
    if ( edge.GetExtension(has_dep)) { 
      const Dep & ret_dep = edge.GetExtension(dep);
      Dependency our_dep = make_dep(ret_dep.head(), ret_dep.mod());
      _dep_map->set_value(*our_edge, our_dep);
      _edge_map->set_value(our_dep, our_edge);
    }
  }

 private:
  // The parse forest in hypergraph form
  int _sent_length;
  Hypergraph * _h;
  Cache <Dependency, const Hyperedge *> *  _edge_map;
  Cache <Hyperedge, Dependency> * _dep_map;
  //Cache <Hyperedge, double> _weights;
  vector <Dependency> _dependencies; 
};


#endif
