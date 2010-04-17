
#ifndef _IMAGEGENERATOR_H
#define _IMAGEGENERATOR_H

#include <QString>
#include <QImage>
#include <QThread>

#include "pointgenerator.h"

class QMutex;

class ImageGenerator : public PointGenerator, QThread { 
  public:
    ImageGenerator(const QImage &image);
    ~ImageGenerator();

    void setImage(const QImage &image);
    
    virtual Point generatePoint();
    virtual Point generateNearbyPoint(const Point& nearThisPoint);
    virtual int dimension();

    int width() const;
    int height() const;

  private:
    QMutex *m_dataAccess;
    QImage m_image;
    qreal normalize(qreal value, qreal maxValue);
    Point pointFromXY(int x, int y);
};

#endif // _IMAGEGENERATOR_H
