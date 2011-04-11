
//#define LM -1.11652878876722


//#define LM -0.157627

//-0.141221


#ifndef COMMON_H
#define COMMON_H

#include <boost/foreach.hpp> 
//#define LMWEIGHT -0.141221
#define foreach BOOST_FOREACH
#define INF 1e8


typedef unsigned int uint;
class Clock {
 public:
  static double diffclock(clock_t clock1,clock_t clock2){
    double diffticks=clock1-clock2;
    double diffms=(diffticks*1000)/CLOCKS_PER_SEC;
    return diffms;
  }
};

#endif
