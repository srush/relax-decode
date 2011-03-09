#include "CorpusSolver.h"

void CorpusSolver::solve(double & primal, double & dual, wvector & subgrad, int round) {
  cout << "Round " << round;

  dual =0;
  primal = 0;
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
    subgrad += _subgrad_cache[sent_num];
    dual += _dual_cache[sent_num];
    primal += _primal_cache[sent_num];
  }
  cout << "Corpus dual: " << dual << endl;
  cout << "Corpus primal: " << primal << endl;

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
