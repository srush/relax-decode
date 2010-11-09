#ifndef NODEEDGE_H_
#define NODEEDGE_H_

#include <vector>
#include <string>
using namespace std;

class Edge;

class Node {
 public: 
  Node(int id, string label, vector <Edge *> edges);
  
  vector <Edge *> edges();
  int id();

 private:
  int _id;
  const string _label;
  vector <Edge *>  _edges; 

};

class Edge {
 public: 
  Edge(int id, string label);
  int id();
  vector <Node *> subs();
 private:
  const int _id;
  const string _label;
  const vector <Node * >  _subs; 
};


#endif
