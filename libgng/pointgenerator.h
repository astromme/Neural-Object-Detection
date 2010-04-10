
#ifndef _POINTGENERATOR_H
#define _POINTGENERATOR_H

#include "point.h"

class PointGenerator { 
  public:
    PointGenerator() {}
    virtual ~PointGenerator() {};
    virtual int dimension() = 0;
    virtual Point generatePoint() = 0;
};

#endif // _POINTGENERATOR_H