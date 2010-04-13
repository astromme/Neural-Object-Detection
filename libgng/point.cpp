
#include "point.h"

#include "math.h"

qreal Point::distanceTo(const Point &other) {
  qreal dist = 0;
  
  for (int i=0; i<size(); i++) {
    dist += pow(other[i]-at(i), 2);
  }
  
  return sqrt(dist) + 1.5*xyDistanceTo(other);
  //return dist;
}

qreal Point::xyDistanceTo(const Point &other)
{
  Q_ASSERT(size() > 1);
  return sqrt(pow(other[0]-at(0),2) + pow(other[1]-at(1),2));
}
