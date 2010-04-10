
#ifndef _POINT_H
#define _POINT_H

#include <QVector>

class Point : QVector<qreal> {
  
  public:
    distanceTo(const Point &other);
    
};

#endif // _POINT_H