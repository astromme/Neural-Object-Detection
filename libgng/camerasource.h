#ifndef GNG_CAMERASOURCE_H
#define GNG_CAMERASOURCE_H

#include <QThread>
#include <QMutex>
#include <QImage>
#include <QTimer>

#include "pointgenerator.h"

#include <cv.h>
#include <highgui.h>

class CameraSource : public QThread, public PointGenerator {
  Q_OBJECT
  public:
    CameraSource();
    ~CameraSource();
    
    virtual int dimension();
    virtual Point generatePoint();
    
    int width();
    int height();
   
    virtual void run();
    
    QImage image() const;
  
  public slots:
    void processNextFrame();
    
  signals:
    void imageUpdated();

  private:
    Point pointFromXY(int x, int y);
    void convertFrameToImage();
    QMutex *m_dataAccess;
    QImage m_image;
    QTimer m_nextFrameTimer;
    CvCapture *m_device;
    IplImage *m_frame;
};

#endif //GNG_CAMERASOURCE_H