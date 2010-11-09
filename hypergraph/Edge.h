#ifndef EDGE_H_
#define EDGE_H_

#include "Node.h"
#include <vector>
using namespace std;

class Edge {
 public: 
  Edge(int id, const char * label);
  int id();
  vector <Node *> subs();
 private:
  int _id;
  const  char * _label;
  vector <Node * >  subs; 
};

#endif
