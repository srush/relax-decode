#ifndef HARDPOSCONSTRAINTS_H
#define HARDPOSCONSTRAINTS_H


#include "TagConstraints.h"
#include "TagLP.h"


using namespace std;

class HardPosConstraintsLP {
 public:
  vector <vector <GRBVar > > group_used;
  vector <vector <vector <GRBVar > > > slacks_pos;
  vector <vector <vector <GRBVar > > > slacks_neg;
  set<int> groups;

 HardPosConstraintsLP(const TagConstraints & constraints, double penalty) : _constraints(constraints), _penalty(penalty) {}

  GRBLinExpr make_lin_expr(const vector <TagLP*> & lp_vars, const PossibleTag & tag, POS restricted_to) {
    // the syn of all the possible deps
    GRBLinExpr lin;
    
    Tag d = lp_vars[tag.sent_num]->p.make_tag(tag.ind, restricted_to);
    if (lp_vars[tag.sent_num]->tag_vars.has_key(d)) {
      GRBVar tag_lp_var = lp_vars[tag.sent_num]->tag_vars.get(d);
      lin = tag_lp_var;
    } else {
      lin = 0;
    }
    return lin;
  }

  void add_to_lp(const vector <TagLP * > & lp_vars, GRBModel & model) {
    try{
      // iterate over unknown words
      group_used.resize(_constraints._constraint_struct.size());      
      slacks_pos.resize(_constraints._constraint_struct.size());      
      slacks_neg.resize(_constraints._constraint_struct.size());  
      
      for (int word_num=0; word_num < _constraints._constraint_struct.size(); word_num++) { 
        //cout << _constraints.groups.count(word_num) << endl;
        if (_constraints.groups.count(word_num) == 0 ) continue; 
        // iterate over possible part of speech
        //group_used.push_back(vector<GRBVar > ());

        slacks_pos[word_num].resize(_constraints._num_tags);
        slacks_neg[word_num].resize(_constraints._num_tags);
        for (POS pos=0; pos < _constraints._num_tags; pos++) {
          stringstream buf; 
          buf << "unknown_" << word_num << "_" << pos;
          group_used[word_num].push_back(model.addVar(0.0, 1.0, 
                                                      0.0, GRB_CONTINUOUS, buf.str()));
        
          slacks_pos[word_num][pos].resize(_constraints._constraint_struct[word_num].group.size());
          slacks_neg[word_num][pos].resize(_constraints._constraint_struct[word_num].group.size());
          
          for (int i =0; i < _constraints._constraint_struct[word_num].group.size() ; i++) {

            PossibleTag first_ptag = _constraints._constraint_struct[word_num].group[i];
            double final_penalty = min(_penalty * (20*first_ptag.test_count  / (float)(20* first_ptag.test_count + first_ptag.training_count + 1)), 1.0);

            stringstream slack_pos_str; 
            slack_pos_str << "slackpos_" << word_num << "_" << pos << "_" << i;

            slacks_pos[word_num][pos][i] =model.addVar(0.0, 1.0, final_penalty, GRB_CONTINUOUS, slack_pos_str.str());
            stringstream slack_neg_str; 
            slack_neg_str << "slackneg_" << word_num << "_" << pos << "_" << i;

            slacks_neg[word_num][pos][i] =model.addVar(0.0, 1.0, final_penalty, GRB_CONTINUOUS, slack_neg_str.str());
        
          }
        }
      }
      model.update();

      for (int group_num=0; group_num < _constraints._constraint_struct.size(); group_num++) { 
        if (_constraints.groups.count(group_num) == 0) continue; 
        for (POS pos =0; pos < _constraints._num_tags; pos++) { 
          
          PossibleTag first_ptag = _constraints._constraint_struct[group_num].group[0];
          GRBLinExpr base = make_lin_expr(lp_vars, first_ptag, pos);
          
          model.addConstr(base == group_used[group_num][pos]);
          for (int i =1; i < _constraints._constraint_struct[group_num].group.size() ; i++) {
            PossibleTag ptag = _constraints._constraint_struct[group_num].group[i];
            stringstream buf; 
            buf << "group_const_" << pos << "_" <<ptag.group_name << "_" << i;
            model.addConstr(base == make_lin_expr(lp_vars, ptag, pos) 
                            - slacks_pos[group_num][pos][i] + slacks_neg[group_num][pos][i], 
                          buf.str());
          }
        }
      }
    } catch (GRBException e) {
      cout << "Error code = " << e.getErrorCode() << endl;
      cout << e.getMessage() << endl;
      assert(false);
    }

    model.update();

  }

  void show_results() { 
    for (int group_num=0; group_num < _constraints._constraint_struct.size(); group_num++) { 
      if (_constraints.groups.find(group_num) == _constraints.groups.end() ) continue; 
      for (POS pos = 0; pos < _constraints._num_tags; pos++) { 
        if (group_used[group_num][pos].get(GRB_DoubleAttr_X)) {
          cout << "Used " << (GRBLinExpr)group_used[group_num][pos] << group_used[group_num][pos].get(GRB_DoubleAttr_X) << endl;
        }
        for (int i =1; i < _constraints._constraint_struct[group_num].group.size() ; i++) {
          if (slacks_pos[group_num][pos][i].get(GRB_DoubleAttr_X)) {
            cout << "Slack " << (GRBLinExpr) slacks_pos[group_num][pos][i] << endl;

          }
          if (slacks_neg[group_num][pos][i].get(GRB_DoubleAttr_X)) { 
            cout << "Slack " << (GRBLinExpr) slacks_neg[group_num][pos][i] << endl;

          }
        }
        
      }
    }
  }


 private:
  const TagConstraints & _constraints; 
  double _penalty;
};

#endif
