#ifndef WEIGHTS_H_
#define WEIGHTS_H_

// Weights.h - helper methods for using David Chiang's sparse vector 
#include <iostream>
#include <fstream>
#include <cy_svector.hpp>
#include <svector.hpp>
using namespace std;

// Weight vector
typedef svector<int, double> wvector;


/**
 * @param file Name of file to read from.
 * @return Weight Vector from the file 
 */ 
wvector * load_weights_from_file(const char * file);



#endif
