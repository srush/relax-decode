#ifndef NODE_H_
#define NODE_H_
#include "Edge.h"
#include <vector>
using namespace std;


class Node {
 public: 
  Node(int id, const char * label, vector <Edge> edges);
  
  vector <Edge> edges();
  int id();

 private:
  int _id;
  const char * _label;
  vector <Edge>  _edges; 

};

#endif
