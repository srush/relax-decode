#include <cpptest.h>
#include <string>
#include <cpptest-suite.h>
#include "Forest.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include "hypergraph.pb.h"
using namespace Test;
using namespace std;

class LocalTestSuite : public Test::Suite
{
public:
  
  LocalTestSuite() {
    TEST_ADD(LocalTestSuite::load_test);        
  }
  
private:
  void load_test() {


    GOOGLE_PROTOBUF_VERIFY_VERSION;

    Hypergraph hgraph;
    string forest_file =  "test_data/t2s.forest";
    
    {
      // Read the existing address book.
      fstream input(forest_file.c_str(), ios::in | ios::binary);
      if (!hgraph.ParseFromIstream(&input)) {
        TEST_ASSERT_MSG(false, "Failed to parse");
      }

    }


    Forest f (hgraph);

    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();

    
  }
    
};


int main(int argc, const char * argv[]) {
  Test::TextOutput output(Test::TextOutput::Verbose);
  LocalTestSuite ets;
  return ets.run(output) ;
}
