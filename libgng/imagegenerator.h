
#ifndef _IMAGEGENERATOR_H
#define _IMAGEGENERATOR_H

#include <QString>
#include <QImage>

#include "pointgenerator.h"

class ImageGenerator : PointGenerator { 
  public:
    ImageGenerator(const QString &filePath);
    virtual ~ImageGenerator();
    
    virtual Point generatePoint();
    virtual int dimension();
    
  private:
    QImage m_image;
};

#endif // _IMAGEGENERATOR_H