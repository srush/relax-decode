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
  bitset <10000> * has_value;

  Cache() { 
    store.resize(10000);
    has_value = new bitset<10000>();
  }

  V get_value(const C & edge) const {
    int id = edge.id();
    assert (has_value->test(id));
    return store[id];
  }
  
  void set_value(const C & edge, V val) {
    assert (has_value != NULL);
    int id = edge.id();
    has_value->set(id, true);
    store[id] = val;
  }

  bool has_key(const C & edge) const {
    return has_value->test(edge.id());
  }
};




#endif
