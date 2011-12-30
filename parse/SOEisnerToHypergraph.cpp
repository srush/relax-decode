#include "Weights.h"
#include "Forest.h"
#include "../common.h"
#include "SOEisnerToHypergraph.h"
#include <iostream>
#include <fstream>
#include <strstream>
#include "../hypergraph/HypergraphAlgorithms.h"


void EisnerToHypergraph::convert(Hypergraph & _forest) {
  for (int i =0; i < length(); i++) {
    {
      EisnerNode n(Span(i,i), LEFT, TRI);    
      finalize_node(n);
    }

    {
      EisnerNode n(Span(i,i), RIGHT, TRI);    
      finalize_node(n);
    }

  }

  for (int spansize = 1 ; spansize < length(); spansize++) {
    for (int i =0; i < length(); i++) {
      int k = i + spansize;
      if (k >= length()) continue;

      for (int j =i; j <= k ; j++) {
        // 5 cases 
     
        // triangle rule
        if (j+1 <= k )
        {          
          // right triangle + left triangle [Creates dep] 
          // Only when right tri is size 1. 

          if (i == j) {
            // RIGHT TRAP 

            EisnerNode lchild(Span(i,j), RIGHT, TRI);
            EisnerNode rchild(Span(j+1,k), LEFT, TRI);
            LocalHyperedge edge;
            
            edge.tail_node_ids.push_back( _node_to_id[lchild]);
            edge.tail_node_ids.push_back( _node_to_id[rchild]);
            Hypergraph_Node * node;

            EisnerNode n(Span(i,k),  RIGHT, TRAP);
            node = finalize_node(n);          
            edge.head = _node_to_id[n];


            // Creates a dependency
            edge.weight = get_weight(i, i, k);          

            Hypergraph_Edge & pedge =finalize_edge(node, edge);
            Dep * my_dep =  pedge.MutableExtension(dep);                  
            my_dep->set_head(i);
            my_dep->set_mod(k);
            pedge.SetExtension(has_dep, true);                  
            //cerr << "Dep "<<  i << " " << k << endl; 
          } 
          {
            EisnerNode lchild(Span(i,j), RIGHT, TRI);
            EisnerNode rchild(Span(j+1,k), LEFT, TRI);
            LocalHyperedge edge;
            
            edge.tail_node_ids.push_back( _node_to_id[lchild]);
            edge.tail_node_ids.push_back( _node_to_id[rchild]);
            Hypergraph_Node * node;

            // right triangle + left triangle [Creates BOX] 
            // BOX
            // Does not! create a dependency
            assert(i!=k);
            EisnerNode n(Span(i,k),  NODIR, BOX);
            node = finalize_node(n);
            edge.head = _node_to_id[n];
            Hypergraph_Edge & pedge =finalize_edge(node, edge);
          }
        }


        // right triangle + left triangle [Creates dep] When l tri is one word
        if (j+1 <= k)
        {

          if (j+1 == k) {
            // LEFT TRAP
            Hypergraph_Node * node;
            EisnerNode lchild(Span(i,j), RIGHT, TRI);
            EisnerNode rchild(Span(j+1,k), LEFT, TRI);
            LocalHyperedge edge;
            edge.tail_node_ids.push_back( _node_to_id[lchild]);
            edge.tail_node_ids.push_back( _node_to_id[rchild]);

            EisnerNode n(Span(i,k),  LEFT, TRAP);
            node = finalize_node(n);                     
            edge.weight = get_weight(k, k, i);
            edge.head = _node_to_id[n];            
            Hypergraph_Edge & pedge = finalize_edge(node, edge);
            
            Dep * my_dep =  pedge.MutableExtension(dep);                  
            my_dep->set_head(k);
            my_dep->set_mod(i);
            pedge.SetExtension(has_dep, true);
            //cerr << "Dep "<<  k << " " << i << endl; 
          }
          {
            // right triangle + left triangle [Creates BOX] 
            // BOX
            Hypergraph_Node * node;
            EisnerNode lchild(Span(i,j), RIGHT, TRI);
            EisnerNode rchild(Span(j+1,k), LEFT, TRI);
            LocalHyperedge edge;
            edge.tail_node_ids.push_back( _node_to_id[lchild]);
            edge.tail_node_ids.push_back( _node_to_id[rchild]);

            assert(i!=k);
            EisnerNode n(Span(i,k),  NODIR, BOX);
            node = finalize_node(n);
            edge.head = _node_to_id[n];
            Hypergraph_Edge & pedge = finalize_edge(node, edge);
          }
        }
      }
      for (int j =i; j <= k ; j++) {
        // right trap + BOX 
        if (i <j && j < k)
        {
          // RIGHT TRAP
          
          EisnerNode n(Span(i,k), RIGHT, TRAP);
          Hypergraph_Node * node = finalize_node(n);
          
          EisnerNode lchild(Span(i,j), RIGHT, TRAP);
          assert(j != k) ;
          EisnerNode rchild(Span(j,k), NODIR, BOX);

          //cout << "Right tri debug --  "<< n.name() << " " << lchild.name() << " " << rchild.name() << endl;

          LocalHyperedge edge;
          assert(_node_to_id.find(lchild) != _node_to_id.end()); 
          assert(_node_to_id.find(rchild) != _node_to_id.end()); 
          edge.head = _node_to_id[n];
          edge.tail_node_ids.push_back( _node_to_id[lchild]);
          edge.tail_node_ids.push_back( _node_to_id[rchild]);

          edge.weight = get_weight(i,j,k);
          edge.head = _node_to_id[n];            
          Hypergraph_Edge & pedge = finalize_edge(node, edge);
          
          Dep * my_dep =  pedge.MutableExtension(dep);                  
          my_dep->set_head(i);
          my_dep->set_mod(k);
          //cerr << "Dep "<<  i << " " << k << endl; 
          pedge.SetExtension(has_dep, true);
        }

        // BOX+ left trap
        if (j < k && i < j)
        {
          // LEFT TRAP

          EisnerNode n(Span(i,k), LEFT, TRAP);
          Hypergraph_Node * node =finalize_node(n);
          
          EisnerNode lchild(Span(i,j), NODIR, BOX);
          EisnerNode rchild(Span(j,k), LEFT, TRAP);
          assert(i != j) ;
          
          LocalHyperedge edge;
          
          edge.head = _node_to_id[n];
          edge.tail_node_ids.push_back( _node_to_id[lchild]);
          edge.tail_node_ids.push_back( _node_to_id[rchild]);
          edge.weight = get_weight(k,j,i);
          Hypergraph_Edge & pedge = finalize_edge(node, edge);
          
          Dep * my_dep =  pedge.MutableExtension(dep);                  
          my_dep->set_head(k);
          my_dep->set_mod(i);
          pedge.SetExtension(has_dep, true);
          //cerr << "Dep "<<  k << " " << i << endl; 
        }

        // right trap + right triangle 
        if (j > i )
        {
          // RIGHT TRI

          EisnerNode n(Span(i,k), RIGHT, TRI);
          Hypergraph_Node * node = finalize_node(n);
          assert(j != i);
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
        if (j < k)
        {
          // LEFT TRI

          EisnerNode n(Span(i,k), LEFT, TRI);
          Hypergraph_Node * node =finalize_node(n);
          assert(j!=k);
          EisnerNode lchild(Span(i,j), LEFT, TRI);
          EisnerNode rchild(Span(j,k), LEFT, TRAP);

          
          LocalHyperedge edge;

          assert(_node_to_id.find(lchild) != _node_to_id.end()); 
          assert(_node_to_id.find(rchild) != _node_to_id.end()); 
          
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

vector<DepParser *> SecondOrderConverter::convert_file(const char *file) {
  stringstream buf;
  vector<DepParser *> ret;
  int sent_num = -1;
  fstream input(file, ios::in);
  while (input) {
    sent_num++;
    vector <int> sent;  
    vector<vector <vector<double > > > weights(MAX_LEN);
    
    for (int i=0;i < MAX_LEN;i++) {
      weights[i].resize(MAX_LEN);
      for (int j=0;j < MAX_LEN; j++) {
        weights[i][j].resize(MAX_LEN);
      }
    }
    
    int max_pos = 0;
    while(input) {
      double prob;
      int pos1, pos2, head;
      string ignore;
      int snum;
      // format is (PROB|END): {sentnum} {i} {j} {dir} {prob}  
      input >> ignore;
      if (ignore == "DONE:") break; 
      input >> snum >> head >> pos1 >> pos2 >>  prob; 
      weights[head][pos1][pos2] = prob; 
      max_pos = max(max_pos, pos1); 
    }
    
    for (int i=0; i<= max_pos; i++ ) {
      sent.push_back(i);
    }
    
    Hypergraph tmp;
    EisnerToHypergraph runner(sent, weights);
    runner.convert(tmp);
    runner.hgraph.SetExtension(len, max_pos);
   
    // Turn the hypergraph into a parser.
    DepParser *parser = new DepParser();
    parser->build_from_proto(&runner.hgraph);  
    HypergraphAlgorithms algorithms(*parser);

    wvector * simple = svector_from_str<int, double>("value=-1");
    EdgeCache *edge_weights = algorithms.cache_edge_weights(*simple);    
    NodeCache inside_memo(parser->num_nodes()), outside_memo(parser->num_nodes());
    double best = algorithms.inside_scores(true, *edge_weights, inside_memo);
    algorithms.outside_scores(true, *edge_weights, inside_memo, outside_memo);
    HypergraphPrune prune = 
      algorithms.pretty_good_pruning(*edge_weights, inside_memo, outside_memo, 0.7 * best);
    Hypergraph to_write = 
      parser->write_to_proto(prune);
    to_write.SetExtension(len, max_pos);
    DepParser *parser2 = new DepParser();
    parser2->build_from_proto(&to_write);  
    ret.push_back(parser2);
  }
  input.close();
  return ret;
}
/*
int main(int argc, char ** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  stringstream buf;

  int sent_num = -1;
  while(cin) {
    sent_num++;
    vector <int> sent;  
    
    vector<vector <vector<double > > > weights(MAX_LEN);
    
    for (int i=0;i < MAX_LEN;i++) {
      weights[i].resize(MAX_LEN);
      for (int j=0;j < MAX_LEN; j++) {
        weights[i][j].resize(MAX_LEN);
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
      cin >> snum >> head >> pos1 >> pos2 >>  prob; 
      //cout << sent_num << " " << snum;
      //assert(sent_num+1 == snum);
      //cout << pos1 << " " << pos2 << " " << head << " " << prob<< endl;
      weights[head][pos1][pos2] = prob; 
      max_pos = max(max_pos, pos1); 
    }
    
    //cout << "Sent " << sent_num << " is " << max_pos << endl;
    for (int i=0; i<= max_pos; i++ ) {
      sent.push_back(i);
    }
    
    //WeightVec blank;
    Hypergraph tmp;
    EisnerToHypergraph runner(sent, weights);
    runner.convert(tmp);

    //cout << "extension set " << max_pos << endl;
    runner.hgraph.SetExtension(len, max_pos);
    //cout << "extension get " << tmp.GetExtension(len) << endl;
   
    // Turn the hypergraph into a parser.
    DepParser *parser = new DepParser();
    parser->build_from_proto(&runner.hgraph);  
    HypergraphAlgorithms algorithms(*parser);
    //wvector *simple = load_weights_from_file("config.ini");
    wvector * simple = svector_from_str<int, double>("value=-1");
  //wvector *simple = load_weights_from_file("config.ini");
    EdgeCache *edge_weights = algorithms.cache_edge_weights(*simple);    
    NodeCache inside_memo(parser->num_nodes()), outside_memo(parser->num_nodes());
    double best = algorithms.inside_scores(true, *edge_weights, inside_memo);
    algorithms.outside_scores(true, *edge_weights, inside_memo, outside_memo);
    HypergraphPrune prune = 
      algorithms.pretty_good_pruning(*edge_weights, inside_memo, outside_memo, 0.7 * best);
    //parser->prune(prune);
    Hypergraph to_write = 
      parser->write_to_proto(prune);
    to_write.SetExtension(len, max_pos);
    // Test
    DepParser *parser2 = new DepParser();
    parser2->build_from_proto(&to_write);  
    HypergraphAlgorithms algorithms2(*parser2);
    EdgeCache *edge_weights2 = algorithms2.cache_edge_weights(*simple);    
    NodeCache inside_memo2(parser2->num_nodes()), outside_memo2(parser2->num_nodes());
    double best2 = algorithms2.inside_scores(true, *edge_weights2, inside_memo2);
    algorithms2.outside_scores(true, *edge_weights2, inside_memo2, outside_memo2);
    algorithms2.pretty_good_pruning(*edge_weights2, inside_memo2, outside_memo2, 0.7 * best);
    cerr << best << " " << best2 << endl;
    {
      stringstream buf;
      buf << argv[1] << sent_num;
      cerr << max_pos << endl;
      fstream output(buf.str().c_str(), ios::out | ios::trunc | ios::binary);
      if (!to_write.SerializeToOstream(&output)) {
        cerr << "Failed to ." << endl;
        return -1;
      }
      //const char * file_name,
    }
  }
  cout << (sent_num -1 ) << endl;
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
*/
