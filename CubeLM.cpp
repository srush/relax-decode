
#include "CubePruning.h"
#include "CubeLM.h"
#include "ForestAlgorithms.h"
#include <cy_svector.hpp>
#include <svector.hpp>
#include "Forest.h"
#include <fstream>
#include <iostream>
#include <Vocab.h>
#include <Ngram.h>
#include <File.h>
#include <iomanip>
#include <sstream>
#include <time.h>
#include "util.h"
using namespace std;

Cache <ForestNode, int > * cache_word_nodes(Ngram lm, const Forest & forest) {
  int max = lm.vocab.numWords();
  int unk = lm.vocab.getIndex(Vocab_Unknown);
  
  Cache <ForestNode, int > * words = new Cache <ForestNode, int >(forest.num_nodes());
  for (int i=0; i< forest.num_nodes(); i++ ) {
    const ForestNode & node = forest.get_node(i);
    if (node.is_word()) {
      string str = node.word();
      int ind = lm.vocab.getIndex(str.c_str());
      //cout << node.id() << endl;
      if (ind == -1 || ind > max) { 
        words->set_value(node, unk);
        //cout << "Word " << unk;
      } else {
        words->set_value(node, ind);
        //cout << "Word " << ind;
      }   
    }
  }
  return words;
}


int main(int argc, char ** argv) {
  cout << argc << endl;

  
  svector<int, double> * weight;

  {
    // Read the existing address book.
    fstream input(argv[2], ios::in );
    char buf[1000];
    input.getline(buf, 100000);
    string s (buf);
    weight = svector_from_str<int, double>(s);
  }

  
  Vocab * all = new Vocab();
  all->unkIsWord() = true;
  Ngram * lm = new Ngram(*all, 3);

  File file(argv[3], "r", 0);    
  if (!lm->read(file, false)) {
    cerr << "READ FAILURE\n";
  }
  
  

  cout << "START!!!!" << endl;

  for (int i=1; i <= 10; i++) {
    
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    Hypergraph hgraph;
    
    {
      stringstream fname;
      fname <<argv[1] << i;
      //cout << fname.str() << endl;
      fstream input(fname.str().c_str(), ios::in | ios::binary);
      if (!hgraph.ParseFromIstream(&input)) {
        assert (false);
      } 
    }
    
    Forest f (hgraph);
    
    // Optional:  Delete all global objects allocated by libprotobuf.
    //google::protobuf::ShutdownProtobufLibrary();
    

    f.append_end_nodes();

    Cache<ForestEdge, double> * w = cache_edge_weights(f, *weight);
    Cache<ForestNode, int> * words = cache_word_nodes(*lm, f);
    
    clock_t begin=clock();    
    CubePruning p(f, *w, LMNonLocal(f, *lm, *words), 100, 3);
    double v =p.parse();    
    clock_t end=clock();
    cout << v << " " <<  (double)diffclock(end,begin) << endl;
  }
  return 0;
}

