
#include "point.h"

#include "math.h"

using namespace GNG;

Point::Point()
{

}

Point::Point(int dimension)
  : QVector<qreal>()
{
  resize(dimension);
}

qreal Point::distanceTo(const Point &other) const {
  qreal dist = 0;
  
  // Color wraps around. HACK: Specific to HSL/HSV
  bool otherIsSmaller = other[2] < at(2) ? true : false;
  if (otherIsSmaller) {
    // Calculate distance both ways and take shorter amount.
    qreal clockwise = at(2) - other[2];
    qreal counterclockwise = other[2] - at(2) + 1;
    dist += pow(qMin(clockwise, counterclockwise), 2);
  } else {
    qreal clockwise = other[2] - at(2);
    qreal counterclockwise = at(2) - other[2] + 1;
    dist += pow(qMin(clockwise, counterclockwise), 2);
  }
  
  // Calculate rest of distances
  for (int i=0; i<size(); i++) {
    if (i == 2) {
      continue;
    }
    dist += pow(other[i]-at(i), 2);
  }
  
  return sqrt(dist) + 2*xyDistanceTo(other);
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
