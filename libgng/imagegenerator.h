
#ifndef _IMAGEGENERATOR_H
#define _IMAGEGENERATOR_H

#include <QString>
#include <QImage>

#include "pointgenerator.h"

class ImageGenerator : public PointGenerator { 
  public:
    ImageGenerator(const QString &filePath);
    ~ImageGenerator();
    
    virtual Point generatePoint();
    virtual int dimension();
    
    int width() const;
    int height() const;
    
  private:
    QImage m_image;
    qreal normalize(qreal value, qreal maxValue);
};

#endif // _IMAGEGENERATOR_H