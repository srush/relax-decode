#include "ParseConstraints.h"


void ParseMrfAligner::build_from_constraints(string file_name) {
  int positions;
  fstream input(file_name.c_str(), ios::in );
  int consistency_id = 0;
  while (input) {
    //PossibleTag tag;
    int constraint_group; 
    int constraint_node; 
    ParseIndex parse_index;
    string group_name;
    input >> constraint_group >>  constraint_node >> parse_index.sent_num >> parse_index.ind;// >> tag.deviance_penalty;
    
    //tag_constraints[constraint_group].push_back(tag_index);    
    for (int s=0; s < 200; s++) {
      parse_index.head = s;
      MrfIndex mrf_index(constraint_group, constraint_node, s);
      alignment[parse_index] =  mrf_index;

      other_id_map[parse_index] = consistency_id;
      cons_id_map[mrf_index] = consistency_id;
      id_other_map[consistency_id] = parse_index;
      id_cons_map[consistency_id] = mrf_index;
      consistency_id++;
    }
  }
  input.close();

}
