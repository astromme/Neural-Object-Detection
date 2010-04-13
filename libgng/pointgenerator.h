
#ifndef _POINTGENERATOR_H
#define _POINTGENERATOR_H

#include "point.h"

class PointGenerator { 
  public:
    PointGenerator() {}
    virtual ~PointGenerator() {};
    /** The dimension of the point generator. All points returned should be of this dimension */
    virtual int dimension() = 0;
    /** Generate a point within the distribution. Reimplement */
    virtual Point generatePoint() = 0;
    /** If the Generator supports it, generate a point nearby to the given point.
        The default implementation simply calls generatePoint() */
    virtual Point generateNearbyPoint(const Point &nearThisPoint) { return generatePoint(); }
};

#endif // _POINTGENERATOR_H