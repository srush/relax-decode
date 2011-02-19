#include "TagConstraints.h"

ostream& operator<<(ostream& output, const PossibleTag& ptag) {
  output << ptag.id << " " <<ptag.sent_num << " " <<ptag.ind << " " << ptag.group_name;
  return output;
}

// wvector TagConstraints::solve_hard( wvector & weights) const {
//   wvector ret;
//   foreach (const ConstraintGroup & cg , _constraint_struct) {
//     ret += cg.solve_hard(weights);
//   }
//   return ret;
// }

// wvector ConstraintGroup::solve_hard( wvector & weights) const {
//   // try each hard tag assignment, pick the one with the best score 
//   double min_score = INF;
//   POS best;
//   for (POS tag =0; tag < Tag::MAX_TAG; tag++) {
//     double score = 0.0;

//     foreach (const PossibleTag & p, group) {
//       double local_score =weights[p.weight_id(tag)];
//       //cout<< "CONS: " << p << " " <<tag << " " << local_score << " "  << endl;
//       score += local_score;
//     }

//      if (score < min_score) {
//       best = tag;
//       min_score = score;
//     }
//   }
//   assert (min_score != INF); 

//   // build up the vector for this choice
//   wvector ret;
//   foreach (const PossibleTag & p, group) {
//     //cout<< "CONS: " << p << " " <<best << endl;
//     ret[p.weight_id(best)] = 1.0;
//   }
//   return ret;
// }


// void TagConstraints::read_from_file(string file_name) {
//     int num_unknown_words;
//     fstream input(file_name.c_str(), ios::in );
//     input >> num_unknown_words;
//     _constraint_struct.resize(num_unknown_words);
//     int id = 0;
    
//     while (input) {
//       PossibleTag tag;
      
//       input >> tag.group >> tag.group_name >> tag.sent_num >> tag.ind >> tag.training_count >> tag.test_count;
//       tag.id = id;
//       id++;
//       _constraint_struct[tag.group].group.push_back(tag);
      
//       if (tag.sent_num >= _constrained_words.size()) {
//         _constrained_words.resize(tag.sent_num+1);
//       }

//       _constrained_words[tag.sent_num].push_back(tag);
//       _all_constraints.push_back(tag);
//       groups.insert(tag.group);
//       //cout << groups.count(tag.group) << endl;
//     }
// }
// NodeCache TagConstraints::build_tagger_constraint_vector(int sent_num, const Tagger & tagger, 
//                                                         wvector & orig_weights ) const {
//   NodeCache ret(tagger.num_edges());
//     if (sent_num > _constrained_words.size()) return ret;

//     const vector <PossibleTag> & sent_constraints =  _constrained_words[sent_num];

//     foreach ( const PossibleTag &  ptag, sent_constraints) {
//       for (POS p =0; p < Tag::MAX_TAG; p++ ) {
//         Tag tag = tagger.make_tag(ptag.ind, p);
//         if (tagger.tag_has_node(tag)) {
//           const Hypernode & node = tagger.tag_to_node(tag);
//           int old_id =ptag.weight_id(p);
//           double old_val = orig_weights[old_id];
//           //cout << "ADDING WEIGHT: " <<ptag << " " <<p << old_val <<endl; 
//           ret.set_value(node,  old_val);
          
//         }
//       }
//     }
//     return ret;
//   }

// wvector TagConstraints::build_tagger_subgradient(int sent_num, const Tagger & tagger, const vector<const Hyperedge *> used_edges) const {
//     wvector ret;
//     if (sent_num > _constrained_words.size()) return ret;

//     const vector <PossibleTag> & sent_constraints =  _constrained_words[sent_num];

//     foreach (HEdge edge, used_edges) {   
//       if (tagger.edge_has_tag(*edge)) {
//         const Tag & tag = tagger.edge_to_tag(*edge);
//         foreach ( const PossibleTag &  ptag, sent_constraints) {
//           if (tag.ind == ptag.ind) {
//             ret[ptag.weight_id(tag.tag)] = 1.0;
//             //cout << "TAG: " <<  ptag << " " << tag.tag<< endl;
//           }
//         }
//       }
//     }
//     return ret;
//   }



void TagMrfAligner::build_from_constraints(string file_name) {
  int positions;
  fstream input(file_name.c_str(), ios::in );
  while (input) {
    //PossibleTag tag;
    int constraint_group; 
    int constraint_node; 
    TagIndex tag_index;
    string group_name;
    input >> constraint_group >>  constraint_node >> tag_index.sent_num >> tag_index.ind;// >> tag.deviance_penalty;
    
    //tag_constraints[constraint_group].push_back(tag_index);    
    for (int s=0; s < Tag::MAX_TAG; s++) {
      tag_index.tag = s;
      alignment[tag_index] = MrfIndex(constraint_group, constraint_node, s);
    }
  }
  input.close();

}

