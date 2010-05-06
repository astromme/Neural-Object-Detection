
#ifndef _IMAGEGENERATOR_H
#define _IMAGEGENERATOR_H

#include <QString>
#include <QImage>
#include <QThread>

#include "pointsource.h"

class QMutex;

namespace GNG {

  class ImageSource : public QThread, public PointSource { 
    public:
      ImageSource(const QImage &image);
      ~ImageSource();

      void setImage(const QImage &image);
      
      virtual Point generatePoint();
      virtual Point generateNearbyPoint(const Point& nearThisPoint);
      virtual int dimension();

      int width() const;
      int height() const;

    private:
      QMutex *m_dataAccess;
      QImage m_image;
      Point pointFromXY(int x, int y);
  };
  
}

#endif // _IMAGEGENERATOR_H
