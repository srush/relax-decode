#include "CorpusSolver.h"

void CorpusSolver::solve(const SubgradState & info,
                         SubgradResult & result) {
  
  cout << "Round " << info.round;

  result.dual =0;
  result.primal = 0;
  for (int sent_num =0; sent_num < _corpus_size;sent_num++) {
    if (_dirty_cache[sent_num]) {
      double local_dual, local_primal;
      wvector  local_subgrad;
      solve_one(sent_num, local_dual, local_primal, local_subgrad);
      _subgrad_cache[sent_num] = local_subgrad;
      _primal_cache[sent_num] = local_primal;
      _dual_cache[sent_num] = local_dual;
      _dirty_cache[sent_num] = false;
    }
    result.subgrad += _subgrad_cache[sent_num];
    result.dual += _dual_cache[sent_num];
    result.primal += _primal_cache[sent_num];
  }
  cout << "Corpus dual: " << result.dual << endl;
  cout << "Corpus primal: " << result.primal << endl;

}


//       if (round == 100) {
//         cout << endl;
//         vector <Tag> res;
//         foreach (HEdge edge, best_edges) {
//           if (tagger.edge_has_tag(*edge)) {
//             Tag d = tagger.edge_to_tag(*edge);
//             res.push_back(d);
//           }
//         }
//         sort(res.begin(), res.end());
//         cout << "SENT: "; 
//         foreach (Tag d, res) {
//           cout << d << " ";
//         }
//         cout << endl;
//       }
