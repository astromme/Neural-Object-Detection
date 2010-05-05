
#include "point.h"

#include "math.h"

using namespace GNG;

qreal Point::distanceTo(const Point &other) const {
  qreal dist = 0;
  
  for (int i=0; i<size(); i++) {
    dist += pow(other[i]-at(i), 2);
  }
  
  return sqrt(dist) + 1.5*xyDistanceTo(other);
  //return dist;
}

qreal Point::xyDistanceTo(const Point &other) const {
  Q_ASSERT(size() > 1);
  return sqrt(pow(other[0]-at(0),2) + pow(other[1]-at(1),2));
}

qreal Point::colorDistanceTo(const Point& other) const {
  qreal dist = 0;
  
  // Start at 2 for just colors
  for (int i=2; i<size(); i++) {
    dist += pow(other[i]-at(i), 2);
  }
  
  return sqrt(dist);
}

QPointF Point::xyLocation() const {
  return QPointF(at(0), at(1));
}
