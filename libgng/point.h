
#ifndef _POINT_H
#define _POINT_H

#include <QVector>
#include <QPointF>

namespace GNG {
  class Point : public QVector<qreal> {
    
    public:
      // Warning, use with care. To put any data in you must call resize()
      Point();
      
      Point(int dimension);
      
      qreal distanceTo(const Point &other) const; //TODO document
      qreal xyDistanceTo(const Point &other) const; //TODO document
      qreal colorDistanceTo(const Point &other) const;
      
      QPointF xyLocation() const;
  };
}

#endif // _POINT_H