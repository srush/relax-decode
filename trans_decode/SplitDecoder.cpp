#include "SplitDecoder.h"

bool SplitHeuristic::has_value(const Location & l, const Hypothesis & hyp) const  {
  if (l.location == NODE) {
    int node_id = l.node_id;
    int low_id = lower_id(hyp);
    const BestHyp & bhyp = 
      _outside_scores.store[node_id];
    return bhyp.has_id(low_id);
  } else if (l.location == EDGE)  {
    int edge_id = l.edge_id;
    int low_id = lower_id(hyp);
    //cout << "Looking for " <<  edge_id << " " << low_id << " " << l.edge_pos << endl;
    const BestHyp & bhyp = 
      _outside_edge_scores.store[edge_id][l.edge_pos];
    return bhyp.has_id(low_id);
  }
  assert(false);
  return 0.0;
}

  

double SplitHeuristic::get_value(const Location & l, const Hypothesis & hyp) const {
  if (l.location == NODE) {
    int node_id = l.node_id;
    return _outside_scores.store[node_id].get_score_by_id(lower_id(hyp));
  } else if (l.location == EDGE ) {
    int edge_id = l.edge_id;
    return _outside_edge_scores.store[edge_id][l.edge_pos].get_score_by_id(lower_id(hyp));
  }
  assert(false);
  return 0.0;
}

SplitController::SplitController (const Subproblem &s, const ForestLattice &l, bool two_classes) 
  : _subproblem(s), _lattice(l), _two_classes(two_classes) {
  assert(_subproblem.projection_dims > BACK || !two_classes);
  if (_two_classes) {
    _classes.resize(2);
    //_inner_projection;
    for (int i=0; i < BACK; i++) {
      _classes[0].push_back(i);
      _inner_projection.push_back(0);
    }
    for (int i=BACK; i < _subproblem.projection_dims; i++) {
      _classes[1].push_back(i);
      _inner_projection.push_back(1);
    }
  } else {
    _classes.resize(_subproblem.projection_dims);
    for (int i=0; i < _subproblem.projection_dims; i++) {
      _inner_projection.push_back(i);
      _classes[i].push_back(i);
    }
  }     
}

int SplitController::project_word(int w) const {
  return _inner_projection[_subproblem.project_word(w)];
}

void SplitController::initialize_hypotheses(const Hypernode & node, vector <Hypothesis *> & hyps, vector <double> & scores) const {    
  int graph_id = _lattice.get_word_from_hypergraph_node(node.id());
  //cerr << lattice_.get_word(graph_id) << " ";
  if (_subproblem.is_overridden(graph_id)) {
    // cout << "Overridden " <<  node.label()  << endl;
    for (int d2 = 0; d2 < dim(); d2++) {
      vector <int> hooks(2);
      vector <int> right_side(2);
      
      int w1 = _subproblem.overridden_by(graph_id);
      
      hooks[0] = project_word(w1);
      hooks[1] = d2;

      right_side[0] = project_word(graph_id);
      right_side[1] = hooks[0];

      Hypothesis * h = 
        new Hypothesis(State(hooks, dim()), State(right_side,dim()));
      h->original_value = 0.0;
      hyps.push_back(h);
      scores.push_back(0.0);    
    }

  } else {
    // cout << "Not overridden " <<  node.label()  << endl;
    for (int d = 0; d < dim(); d++) {
      
      assert(node.is_terminal());
        
      assert(_lattice.is_word(graph_id));
      for (int d2 = 0; d2 < dim(); d2++) {
        
        vector <int> hooks(2);
        vector <int> right_side(2);
        hooks[0] = d;
        hooks[1] = d2;
        right_side[0] = project_word(graph_id);
        right_side[1] = d;
        Hypothesis * h = new Hypothesis(State(hooks, dim()), State(right_side,dim()));
        double score = _subproblem.best_score_dim_min(graph_id, 
                                                      _classes[d], 
                                                      _classes[d2]);

        bool has_no_trigram = _subproblem.has_no_trigram(graph_id);
        // if (has_no_trigram) {
        //   cout << "No trigram " <<  node.label()  << endl;
        // }

        // HACK! Words below ORDER have no previous word 
        if (score >=  1000 && !has_no_trigram ) continue;           
        if (has_no_trigram) {
          score = 0.0;
        }
        
        h->original_value = score;
        
        hyps.push_back(h);
        scores.push_back(score);
      }
    }
  }
}

  
void SplitController::initialize_out_root(vector <Hypothesis *> & hyps, 
                           vector <double> & scores)  const {

  
  for (int d=0; d < dim(); d++) {
    for (int d2 = 0; d2 < dim(); d2++) {
      vector <int> hooks(2);
      vector <int> right_side(2);
      // FIXME
      hooks[0] = 0;
      hooks[1] = 0;
      right_side[0] = d2;
      right_side[1] =d;
      
      Hypothesis * h = new Hypothesis(State(hooks,dim()), State(right_side,dim()));
      double my_score = 0.0;
      
      hyps.push_back(h);
      scores.push_back(my_score);
    }
  }
}

  double SplitController::find_best( vector <Hypothesis *> & root_hyps, vector<double > & scores, 
                                     Hypothesis & best_hyp) const {

    // FIXME!!!!
    double best = 1e20;

    // If we did it correctly
    for (uint iter = 0; iter < root_hyps.size(); iter++) {
      const Hypothesis & hyp1 = *root_hyps[iter]; 
      double my_score = scores[iter];
      if (my_score < best) {
        best = my_score;
        best_hyp = hyp1;
      }
    }
    return best;
  }

