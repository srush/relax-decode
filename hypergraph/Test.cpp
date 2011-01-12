/*#include <google/protobuf/stubs/common.h>
#include <cpptest.h>
#include <string>
#include <cpptest-suite.h>
#include "Forest.h"
#include "CubePruning.h"
#include "HypergraphAlgorithms.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <cy_svector.hpp>
#include <svector.hpp>
#include "EdgeCache.h"
#include "ExtendCKY.h"
using namespace Test;
using namespace std;



class LocalTestSuite : public Test::Suite
{
public:
  
  LocalTestSuite() {
    // Forest tests 
    TEST_ADD(LocalTestSuite::load_forest_test);        
   
    TEST_ADD(LocalTestSuite::load_test);        
  }
  
  
  void setup() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
  }

  void teardown() {
    google::protobuf::ShutdownProtobufLibrary();
  }

  void load_forest_test() {
    // Load in forests and check that they are valid

    
    string forest_file =  "test_data/t2s.forest";
    
    Forest f = Forest::from_file(forest_file.c_str());
    


  }


  // Check  basic properties of the forest 
  void check_forest(const Forest & forest) {
    TEST_ASSERT(forest.num_nodes() > 0);
    TEST_ASSERT(forest.num_edges() > 0);

    int edges = 0;
    for (int i=0; i < forest.num_nodes(); i++ ) {
      const ForestNode &  node = forest.get_node(i);
      for (int j=0; j <  node.num_edges(); j++) {
        const ForestEdge & edge  = node.edge(j);
        edges ++;
      }
    }
    assert(edges == forest.num_edges());   
  }

  void load_test() {




    
    string forest_file =  "test_data/t2s.forest";
    
    Forest f = Forest::from_file (forest_file.c_str());

    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();

    svector<int, double> * weight;
    {
      fstream input("test_data/config.ini", ios::in );
      char buf[1000];
      input.getline(buf, 100000);
      string s (buf);
      weight = svector_from_str<int, double>(s);
    }
    EdgeCache * w = cache_edge_weights(f, *weight);
    NodeBackCache bcache(f.num_nodes()),  bcache2(f.num_nodes());
    NodeCache ncache(f.num_nodes());

    //CubePruning p(f, *w, BlankNonLocal(), 100, 5);
    vector <Hyp> kbest;
    //p.run(f.root(), kbest);

    //p.parse();
    double best = best_path(f, *w, ncache, bcache);
    cout << best << endl;
    
    TrivialController c;
    ExtendCKY ecky(f);
    ecky.set_params(w, &c);
    best = ecky.best_path(bcache2);
    double total =0.0;
    for (int i =0; i < f.num_nodes(); i++) {
      const ForestNode & node = f.get_node(i);
      
      if (!node.is_word() && bcache2.has_key(node)) {
        //assert(bcache.get_value(node) == bcache2.get_value(node));
        total += w->get_value(*(bcache2.get_value(node)));
      }
    }
    cout << best << endl;
    cout << total << endl;
  }
    
};
*/

int main(int argc, const char * argv[]) {
  //Test::TextOutput output(Test::TextOutput::Verbose);
  //LocalTestSuite ets;
  //ets.load_test();
   return 0;
}
