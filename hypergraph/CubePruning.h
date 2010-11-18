#include "Forest.h"
#include "EdgeCache.h"

// Some non local feature scorer
class NonLocal {
  
}

class CubePruning {
 public:
  CubePruning(const Forest & forest, const NonLocal & non_local, int k, int ratio):
    _forest(forest), _non_local(non_local), _k(k), _ratio(ratio)
    {}

  void parse();

  

 private:
  void run(const & ForestNode cur_node);

  const Forest & _forest;
  const NonLocal & _non_local;
  const Cache<ForestNode, Float> & _hypothesis_cache;
  //const PriorityQueue _candidates;
  const int _k;
  const int _ratio;
}
