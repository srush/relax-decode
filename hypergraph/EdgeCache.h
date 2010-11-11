#ifndef EDGECACHE_H_
#define EDGECACHE_H_
#include <vector>
#include <bitset>
using namespace std;

template <class C, class V>
class Cache {
 public:
  // can hit directly if need be
  vector <V> store;
  vector <bool> has_value;

  Cache(int size) { 
    store.resize(size);
    has_value.resize(size);
  }

  V get_value(const C & edge) const {
    int id = edge.id();
    assert (has_value[id]);
    return store[id];
  }
  
  void set_value(const C & edge, V val) {
    int id = edge.id();
    has_value[id]= true;
    store[id] = val;
  }

  bool has_key(const C & edge) const {
    return has_value[edge.id()];
  }
};




#endif
