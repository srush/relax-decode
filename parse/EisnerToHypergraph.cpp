#include "EisnerToHypergraph.h"
#include <iostream>
#include <fstream>
#include <strstream>


Direction num_dir = (Direction)2;

void EisnerToHypergraph::convert(Hypergraph & _forest) {
  for (int i =0; i < length(); i++) {
    //for (Direction d =0; d < num_dir; d++) {
//_node_to_id[n] = get_id();    

    {
      EisnerNode n(Span(i,i), LEFT, TRI);    
      finalize_node(n);
    }


    {
      EisnerNode n(Span(i,i), RIGHT, TRI);    
      finalize_node(n);
    }


    //}
  }

  for (int spansize = 1 ; spansize < length(); spansize++) {
    for (int i =0; i < length(); i++) {
      int k = i + spansize;
      if (k >= length()) continue;

      for (int j =i; j <= k ; j++) {
        // 4 cases 
     
        // triangle rule
        // right triangle + left triangle [Creates dep] 
        if (j+1 <= k)
        {

          
          // RIGHT TRAP 

          EisnerNode n(Span(i,k),  RIGHT, TRAP);
          
          Hypergraph_Node * node = finalize_node(n);
          EisnerNode lchild(Span(i,j), RIGHT, TRI);
          EisnerNode rchild(Span(j+1,k), LEFT, TRI);

          LocalHyperedge edge;
          
          edge.head = _node_to_id[n];
          stringstream buf;
          
          //edge.dep = buf.str();
          //cout << " Node to id " <<  _node_to_id[lchild] << " " << lchild.name() << endl;
          edge.tail_node_ids.push_back( _node_to_id[lchild]);
          edge.tail_node_ids.push_back( _node_to_id[rchild]);
          edge.weight = get_weight(i, k);
          
          Hypergraph_Edge & pedge =finalize_edge(node, edge);
          
          Dep * my_dep =  pedge.MutableExtension(dep);                  
          my_dep->set_head(i);
          my_dep->set_mod(k);
          pedge.SetExtension(has_dep, true);                  
          cout << "RL:" << i << " " << k << " " << pedge.id() << endl;
        }
        // right triangle + left triangle [Creates dep]
        if (j+1 <= k)
        {
          // LEFT TRAP
          
          if (j+1 > k) continue; 
          EisnerNode n(Span(i,k),  LEFT, TRAP);
          Hypergraph_Node * node = finalize_node(n);
          
          EisnerNode lchild(Span(i,j), RIGHT, TRI);
          EisnerNode rchild(Span(j+1,k), LEFT, TRI);

          LocalHyperedge edge;
          
          edge.head = _node_to_id[n];
          //stringstream buf;
          //buf << k << " " << i;
          

          edge.tail_node_ids.push_back( _node_to_id[lchild]);
          edge.tail_node_ids.push_back( _node_to_id[rchild]);
          edge.weight = get_weight(k, i);
        
          Hypergraph_Edge & pedge = finalize_edge(node, edge);
          Dep * my_dep =  pedge.MutableExtension(dep);                  
          my_dep->set_head(k);
          my_dep->set_mod(i);
          pedge.SetExtension(has_dep, true);
          cout << "LR:" <<  k << " " << i << " " << pedge.id() << endl;
          
        }

        // right trap + right triangle 
        if (j != i)
        {
          // RIGHT TRI

          EisnerNode n(Span(i,k), RIGHT, TRI);
          Hypergraph_Node * node = finalize_node(n);
          
          EisnerNode lchild(Span(i,j), RIGHT, TRAP);
          EisnerNode rchild(Span(j,k), RIGHT, TRI);

          //cout << "Right tri debug --  "<< n.name() << " " << lchild.name() << " " << rchild.name() << endl;

          LocalHyperedge edge;
          
          edge.head = _node_to_id[n];
          edge.tail_node_ids.push_back( _node_to_id[lchild]);
          edge.tail_node_ids.push_back( _node_to_id[rchild]);
        
          finalize_edge(node, edge);
        }

        // left tri + left trap
        if (j != k)
        {
          // LEFT TRI

          EisnerNode n(Span(i,k), LEFT, TRI);
          Hypergraph_Node * node =finalize_node(n);
          
          EisnerNode lchild(Span(i,j), LEFT, TRI);
          EisnerNode rchild(Span(j,k), LEFT, TRAP);

          
          LocalHyperedge edge;
          
          edge.head = _node_to_id[n];
          edge.tail_node_ids.push_back( _node_to_id[lchild]);
          edge.tail_node_ids.push_back( _node_to_id[rchild]);
          
          finalize_edge(node, edge);
        }      
      }    
    }
  }
  finalize_root();
}



int main(int argc, char ** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  stringstream buf;
  //fstream in(argv[1], ios::in);
  for (int sent_num=1; sent_num <= 10 ;sent_num++ ) {
 
    vector <int> sent;  
    
    vector<vector <vector<double > > > weights(MAX_LEN);
    
    for (int i=0;i < MAX_LEN;i++) {
      weights[i].resize(MAX_LEN);
      for (int j=0;j < MAX_LEN; j++) {
        weights[i][j].resize(2);
      }
    }
    
    int max_pos = 0;
    while(cin) {
      double prob;
      int pos1, pos2, head;
      string ignore;
      int snum;
      // format is (PROB|END): {sentnum} {i} {j} {dir} {prob}  
      cin >> ignore;
      if (ignore == "DONE:") break; 
      cin >> snum >> pos1 >> pos2 >> head >> prob; 
      //cout << sent_num << " " << snum;
      assert(sent_num == snum);
      //cout << pos1 << " " << pos2 << " " << head << " " << prob<< endl;
      weights[pos1][pos2][head] = prob; 
      max_pos = max(max_pos, pos1); 
    }
    
    cout << "Sent " << sent_num << " is " << max_pos << endl;
    for (int i=0; i<= max_pos+1; i++ ) {
      sent.push_back(i);
    }
    
    //WeightVec blank;
    Hypergraph tmp;
    EisnerToHypergraph runner(sent, weights);
    runner.convert(tmp);

    cout << "extension set " << max_pos << endl;
    runner.hgraph.SetExtension(len, max_pos+1);
    cout << "extension get " << tmp.GetExtension(len) << endl;
   

    {
      stringstream buf;
      buf << "/tmp/eisner" << sent_num;
      fstream output(buf.str().c_str() , ios::out | ios::trunc | ios::binary);
      if (!runner.hgraph.SerializeToOstream(&output)) {
        cerr << "Failed to ." << endl;
        return -1;
      }
    }
  }

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();



  return 0;
}
