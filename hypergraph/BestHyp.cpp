#include "BestHyp.h"


void extract_back_pointers(const ForestNode & node, 
                           const Hypothesis & best_hyp, 
                           const Cache <ForestNode, BestHyp> & memo_table, 
                           NodeBackCache & back_pointers) {
  if (back_pointers.has_key(node)) {
    assert(false);
    return;
  } else if (node.num_edges() == 0) {
    assert(node.is_word());
    //cout << best_hyp.hook << " " << best_hyp.original_value << " " << node.word();
    
    return;
  } else {
    back_pointers.set_value(node, best_hyp.back_edge);
    
    const ForestEdge & edge = *best_hyp.back_edge;
    //cout << node.id() << " " << edge.id() << endl;
    int prev = best_hyp.prev_hyp.size();
    int num_nodes = edge.num_nodes();
    assert(best_hyp.prev_hyp.size() == edge.num_nodes());
    
    
    for (int j=0; j < edge.num_nodes(); j++ ) {
      const ForestNode & sub_node = edge.tail_node(j);
      
      extract_back_pointers(sub_node, 
                            memo_table.get_value(sub_node).get_hyp_by_id(best_hyp.prev_hyp[j]), 
                            memo_table,
                            back_pointers);
    }
  }
  return;
}

