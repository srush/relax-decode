#include <iostream>
#include <iomanip>
#include "common.h"
#include "Rates.h"
#include "DualDecomposition.h"
#include "tagger/TagConstraints.h"
#include "optimization/CorpusSolver.h"
#include "mrf/MRF.h"
#include "mrf/MRFConstraints.h"
#include "mrf/MRFSolvers.h"
#include "tagger/TagConstraints.h"
#include "tagger/TagSolvers.h"

using namespace std;

/**
 * This is an example showing how to use the MRF code to tie together multiple sentences. 
 * 
 * We'll have three sentences. One word in each is tied together with an MRF constraint. 
 *
 * "A B C"
 * "D A E"
 * "F G A"
 * 
 * We'll have a binary tage for each word (0 or 1) and the constraint
 * that the A's choose the same label.
 *
 * Our model will strongly favor labelings of "0 1 1", which means it
 * will want to give A label 1 in the first sentence and 0, 0 in the
 * next two. But we will also add a constraint that says the A's
 * should be the same.
 *
 */
 
// This is a place holder class that implements a basic sentence level model. 
// Feel free to replace this with whatever model makes sense. 
// 
// optimization/CorpusSolver has the base class. It handles not redecoding sentence.
class CustomTaggerDual : public CorpusSolver {
public:
  
  CustomTaggerDual(vector<vector<string> > &sentences,
                   wvector &weights,
                   TagMrfAligner &alignment):
    _tag_consistency(alignment),
    _base_weights(weights),
    _sentences(sentences),
    CorpusSolver(sentences.size()){}
  
  // This code should solve a single sentence with the lagrangian vector _cur_weights. 
  // dual should get the dual value and subgrad the result. Ignore primal for now. 
  void solve_one(int sent_num, double & dual, double & primal, wvector & subgrad) {
    cout << endl << "Sent: " << sent_num << endl;
    
    // For each word in the sentence. 
    for (int i = 0; i < _sentences[sent_num].size(); i++) {

      // Score for label 0.
      double score_0 = 0.0;
      TagIndex tag_index_0(sent_num, i, 0);
      // Does it have an associated constraint?
      bool has_constraint_0 = _tag_consistency.other_aligned(tag_index_0);
      int lagrangian_index_0;
      if (has_constraint_0) {
        lagrangian_index_0 = _tag_consistency.other_id(tag_index_0);
        // Add the lagrangian penalty.
        score_0 += (*_cur_weights)[lagrangian_index_0]; 
      }

      // Score for label 1.
      // base weights is the on score. 
      double score_1 = _base_weights[i];
      TagIndex tag_index_1(sent_num, i, 1);
      bool has_constraint_1 = _tag_consistency.other_aligned(tag_index_1);
      int lagrangian_index_1;
      if (has_constraint_1) {
        lagrangian_index_1 = _tag_consistency.other_id(tag_index_1);
        score_1 += (*_cur_weights)[lagrangian_index_1]; 
      }

      // Take best (min) assignment.
      if (score_0 < score_1) {
        dual += score_0;
        if (has_constraint_0) {
          // If there is a constraint here, update the vector.
          subgrad[lagrangian_index_0] += 1;
        }
        cout << "0 ";
      } else {
        dual += score_1;
        if (has_constraint_1) {
          subgrad[lagrangian_index_1] += 1;
        }
        cout << "1 ";
      }
    }
    cout << endl;
  } 


protected:
  // Required method. Given an index in the lagrangian vector, return
  // the sentence number. (handles dirtying.
  int lag_to_sent_num(int lag) {
    return _tag_consistency.id_other(lag).sent_num;
  }

  // The corpus. 
  const vector<vector<string> > &_sentences;

  // The initial weight vector \theta.
  wvector & _base_weights;

  // The constraint alignment information.
  const TagMrfAligner & _tag_consistency;

};


int main(int argc, char ** argv) {
  
  // First setup some simple data.
  vector<vector<string> > sents(3);  
  sents[0].push_back("A");
  sents[0].push_back("B");
  sents[0].push_back("C");

  vector<string> sent2;
  sents[1].push_back("D");
  sents[1].push_back("A");
  sents[1].push_back("E");

  vector<string> sent3;
  sents[2].push_back("F");
  sents[2].push_back("G");
  sents[2].push_back("A");

  // Initial model
  wvector weights;
  // Score for choosing 1 and not 0 at each position.
  // Favor 0 1 1 "tagging" 
  weights[0] = 5;
  weights[1] = -5;
  weights[2] = -5;

  // Declare consistency constraints. 
  vector<TagIndex> tag_indices(6);
  // Each position and tag of the letter A. 
  tag_indices[0] = TagIndex(0, 0, 0);
  tag_indices[1] = TagIndex(0, 0, 1);
  tag_indices[2] = TagIndex(1, 1, 0);
  tag_indices[3] = TagIndex(1, 1, 1);
  tag_indices[4] = TagIndex(2, 2, 0);
  tag_indices[5] = TagIndex(2, 2, 1);  

  // Now build the MRF Graph (using protobuf interface)
  cout << "Building Graph" << endl;
  graph::Graph *mrf_graph = new graph::Graph(); 
  vector<graph::Graph::Node *> nodes(3);

  // Graph looks like-
  //      c
  //      o 
  //     /|\
  //    / | \
  //   o  o  o
  //   0  1  2 
  //
  //  Each node has label 0 and 1.
  for (int i = 0; i < 3; i++) {
    nodes[i] = mrf_graph->add_node();
    nodes[i]->set_id(i);
    graph::MRFNode *mrf_node_info = nodes[i]->MutableExtension(graph::mrf_node);
    
    // Each tag (0 or 1).
    for (int j = 0; j <= 1; j++) {
      graph::NodeStatePotential *pot = mrf_node_info->add_node_potentials();
      graph::State * state = pot->mutable_state(); 
      state->set_id(j);
    }
  }
  graph::Graph::Node *consensus =  mrf_graph->add_node();
  consensus->set_id(3);
  graph::MRFNode *mrf_node_info = consensus->MutableExtension(graph::mrf_node);
  for (int j = 0; j <= 1; j++) {
    graph::NodeStatePotential *pot = mrf_node_info->add_node_potentials();
    graph::State * state = pot->mutable_state(); 
    state->set_id(j);
  }


  // Add edges. 
  cout << "Adding Edges" << endl;
  vector<graph::Graph::Edge *> edges(3);
  for (int i = 0; i < 3; i++) {
    edges[i] = nodes[i]->add_edge();
    edges[i]->set_id(i);
    edges[i]->set_to_node(consensus->id());  
    graph::MRFEdge *mrf_edge_info = edges[i]->MutableExtension(graph::mrf_edge);

    // Each tag pair (0 or 1).
    for (int j = 0; j <= 1; j++) {
      for (int k = 0; k <= 1; k++) {
        graph::EdgeStatePotential *pot = mrf_edge_info->add_edge_potentials();
        pot->set_from_state_id(j);
        pot->set_to_state_id(k);
        // IMPORTANT
        // How strongly should we encourage agreement. 
        pot->set_edge_potential( (j==k) ? -1 : 0 );
      }
    }
  }

  // Load in the mrf.
  cout << "Loading MRF" << endl;
  vector<MRF *> mrfs;
  MRF *mrf = new MRF();
  mrf->build_from_proto(mrf_graph);
  mrfs.push_back(mrf);

  // Align MRF indices (constraint, node, label)
  cout << "Mrf Indices";

  vector<MrfIndex> mrf_indices(6);
  // Parameters are (constraing index, node index, label index)
  // match with tag_indices aboce.
  mrf_indices[0] = MrfIndex(0, 0, 0);
  mrf_indices[1] = MrfIndex(0, 0, 1);
  mrf_indices[2] = MrfIndex(0, 1, 0);
  mrf_indices[3] = MrfIndex(0, 1, 1);
  mrf_indices[4] = MrfIndex(0, 2, 0);
  mrf_indices[5] = MrfIndex(0, 2, 1);  

  
  // MRF part of dual.
  cout << "Constrainer dual" << endl;
  TagMrfAligner tag_align;
  // Ignore
  wvector * simple = svector_from_str<int, double>("value=-1");
  tag_align.build_from_vectors(mrf_indices, tag_indices);
  ConstrainerDual<TagIndex> mrf_dual(mrfs, *simple, tag_align);

  // Custom tagger part of dual.
  CustomTaggerDual tagger_dual(sents, weights, tag_align);
  FallingRate rate;
  DualDecomposition dd(tagger_dual, mrf_dual, rate);
  dd.subgradsolver.set_debug();  
  dd.solve(1);
}


