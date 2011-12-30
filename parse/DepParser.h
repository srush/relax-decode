#ifndef DEPPARSER_H_
#define DEPPARSER_H_

#include "dep.pb.h"
#include "Hypergraph.h"
#include "HypergraphImpl.h"
#include "EdgeCache.h"
#include <algorithm>
using namespace Scarab::HG;

struct Dependency {
  int head;
  int mod;
  int length;
  Dependency(){}
  Dependency(int l, int h, int m): length(l), head(h), mod(m){
    _id= head * l + mod;
  }
  
  const bool operator<(const Dependency & other) const {
    if (mod != other.mod) 
      return mod < other.mod;
    else 
      return head < other.head;
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
    for (int m=0;m < _sent_length; m++) {
      for (int h=0; h < _sent_length; h++) {
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
    _dep_length = id_size + 5;
    _dep_map =  new Cache <Hyperedge, Dependency>(edge_count);
    _edge_map = new Cache <Dependency, vector<const Hyperedge *> >(_dep_length);
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
    return _dep_length;
  }

  uint sent_length() const {
    return _sent_length;
  } 

  Dependency make_dep(int head, int mod) const {
    return Dependency(_sent_length, head, mod);
  }

  const vector<const Hyperedge*> & dep_to_edge(const Dependency & dep) const {
    return _edge_map->get(dep);
  }

  bool dep_has_edge(const Dependency & dep) const {
    return _edge_map->has_key(dep);
  }

  const Dependency & edge_to_dep(const Hyperedge & edge) const {
    return _dep_map->get(edge);
  }

  bool edge_has_dep(const Hyperedge & edge) const {
    return _dep_map->has_key(edge);
  }

  void show_derivation( HEdges best_edges) const {
    vector <Dependency> res;

    foreach (HEdge edge, best_edges) {
      if (edge_has_dep(*edge)) {
        Dependency d = edge_to_dep(*edge);
        res.push_back(d);
      }
    }
    sort(res.begin(), res.end());
    foreach(Dependency d, res) {
      cout << d << " ";
    }
    cout << endl;
  }
  
 protected:
  void make_edge(const Hypergraph_Edge & edge, const Scarab::HG::Hyperedge * our_edge) {
    //cout << "Make Edge" << edge.HasExtension(has_dep) << endl;
    if ( edge.GetExtension(has_dep)) { 
      const Dep & ret_dep = edge.GetExtension(dep);
      Dependency our_dep = make_dep(ret_dep.head(), ret_dep.mod());
      _dep_map->set_value(*our_edge, our_dep);
      _edge_map->get_no_check(our_dep).push_back(our_edge);
    }
  }

  virtual void convert_edge(const Hyperedge * our_edge, Hypergraph_Edge * edge, int id) {
    edge->set_id(id);
    edge->set_label(our_edge->label());
    //edge->SetExtension(edge_fv, svector_str<int, double>(our_edge->fvector()));
    if (_dep_map->has_key(*our_edge)) {
      edge->SetExtension(has_dep, true);                  
      Dep *mut_dep = edge->MutableExtension(dep);
      const Dependency &dep = _dep_map->get(*our_edge);
      mut_dep->set_head(dep.head);
      mut_dep->set_mod(dep.mod);
    } else {
      edge->SetExtension(has_dep, false);
    }
  }

 private:
  // The parse forest in hypergraph form
  int _sent_length;
  int _dep_length;
  Hypergraph * _h;
  Cache <Dependency, vector <const Hyperedge *> > *  _edge_map;
  Cache <Hyperedge, Dependency> * _dep_map;
  //Cache <Hyperedge, double> _weights;
  vector <Dependency> _dependencies; 
};


#endif
