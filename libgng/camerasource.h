#ifndef GNG_CAMERASOURCE_H
#define GNG_CAMERASOURCE_H

#include <QObject>
#include <QImage>
#include <QTimer>

#include "pointsource.h"

#include <cv.h>
#include <highgui.h>

namespace GNG {
  
  class CameraSource : public QObject, public PointSource {
    Q_OBJECT
    public:
      CameraSource();
      ~CameraSource();
      
      virtual int dimension();
      virtual Point generatePoint();
      
      int width();
      int height();
      
      void start();
      void stop();
      
      QImage image() const;
    
    public slots:
      void processNextFrame();
      
    signals:
      void imageUpdated();

    private:
      Point pointFromXY(int x, int y);
      void convertFrameToImage();
      QImage m_image;
      uchar* m_imageData;
      QTimer m_nextFrameTimer;
      CvCapture *m_device;
      IplImage *m_frame;
  };

}

#endif //GNG_CAMERASOURCE_H