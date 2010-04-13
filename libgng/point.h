
#ifndef _POINT_H
#define _POINT_H

#include <QVector>

class Point : public QVector<qreal> {
  
  public:
    qreal distanceTo(const Point &other); //TODO document
    qreal xyDistanceTo(const Point &other); //TODO document
    
};

#endif // _POINT_H