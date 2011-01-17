#include "EisnerToHypergraph.h"
#include <iostream>
#include <fstream>


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
        // right triangle + left triangle 
        if (j+1 <= k)
        {

          
          // RIGHT TRAP

          EisnerNode n(Span(i,k),  RIGHT, TRAP);
          
          Hypergraph_Node * node = finalize_node(n);
          EisnerNode lchild(Span(i,j), RIGHT, TRI);
          EisnerNode rchild(Span(j+1,k), LEFT, TRI);

          LocalHyperedge edge;
          
          edge.head = _node_to_id[n];
          cout << " Node to id " <<  _node_to_id[lchild] << " " << lchild.name() << endl;
          edge.tail_node_ids.push_back( _node_to_id[lchild]);
          edge.tail_node_ids.push_back( _node_to_id[rchild]);
          edge.weight = get_weight(i, k);
          
          finalize_edge(node, edge);
        
        }
   
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
          edge.tail_node_ids.push_back( _node_to_id[lchild]);
          edge.tail_node_ids.push_back( _node_to_id[rchild]);
          edge.weight = get_weight(k, i);
        
          finalize_edge(node, edge);
        }

        if (j != i)
        {
          // RIGHT TRI

          EisnerNode n(Span(i,k), RIGHT, TRI);
          Hypergraph_Node * node = finalize_node(n);
          
          EisnerNode lchild(Span(i,j), RIGHT, TRAP);
          EisnerNode rchild(Span(j,k), RIGHT, TRI);

          cout << "Right tri debug --  "<< n.name() << " " << lchild.name() << " " << rchild.name() << endl;

          LocalHyperedge edge;
          
          edge.head = _node_to_id[n];
          edge.tail_node_ids.push_back( _node_to_id[lchild]);
          edge.tail_node_ids.push_back( _node_to_id[rchild]);
        
          finalize_edge(node, edge);
        }

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
  vector <int> sent;
  


  fstream in(argv[1], ios::in);

  vector<vector <vector<double > > > weights(MAX_LEN);

  for (int i=0;i < MAX_LEN;i++) {
    weights[i].resize(MAX_LEN);
    for (int j=0;j < MAX_LEN; j++) {
      weights[i][j].resize(2);
    }
  }
  
  int max_pos = 0;
  while(in) {
    double prob;
    int pos1, pos2, head;
    in >> pos1 >> pos2 >> head >> prob; 
    cout << pos1 << " " << pos2 << " " << head << " " << prob<< endl;
    weights[pos1][pos2][head] = prob; 
    max_pos = max(max_pos, pos1); 
  }
  
  
  for (int i=0; i<= max_pos+1; i++ ) {
    sent.push_back(i);
  }

  //WeightVec blank;
  Hypergraph tmp;
  EisnerToHypergraph runner(sent, weights);
  runner.convert(tmp);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    fstream output("/tmp/eisner", ios::out | ios::trunc | ios::binary);
    if (!runner.hgraph.SerializeToOstream(&output)) {
      cerr << "Failed to ." << endl;
      return -1;
    }
  }

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();



  return 0;
}
