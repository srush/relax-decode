#ifndef COMMANDLINE_H_
#define COMMANDLINE_H_

#include <google/gflags.h>
#include <sstream>
#include <iostream>
using namespace std;
using namespace google;

static bool ValidateNum(const char* flagname, double value) {
  if (value !=  0.0)   // value is ok
    return true;
  cout << "Invalid value for --"<<flagname << ": " << value<< endl;
  return false;
}

static bool ValidateReq(const char* flagname, const string & value) {
  if (value !=  "")   // value is ok
    return true;
  cout << "Invalid value for --"<<flagname << ": " << value<< endl;
  return false;
}

static bool ValidateFile(const char* flagname, const string & value) {
  if (value !=  "")   // value is ok
    return true;
  cout << "Invalid value for --"<<flagname << ": " << value<< endl;
  return false;
}

static bool ValidateRange(const char* flagname, const string & value) {
  istringstream range(value);
  int start_range, end_range;
  range >> start_range >> end_range;
  return true;
}

#endif
