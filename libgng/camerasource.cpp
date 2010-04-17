#include "camerasource.h"

#include <QDebug>

CameraSource::CameraSource()
{
  m_nextFrameTimer.setInterval(1000);
  connect(&m_nextFrameTimer, SIGNAL(timeout()), SLOT(processNextFrame()));
  m_frame = 0;
  m_device = cvCreateCameraCapture(-1);
  if (!m_device){
    qDebug() << "Camera not found";
    return;
  }
  processNextFrame();
}
CameraSource::~CameraSource()
{
//   delete m_device; //TODO: Memleaks
//   delete m_frame;
}

void CameraSource::run()
{
  m_nextFrameTimer.start();
  QThread::run();
}

void CameraSource::processNextFrame()
{
  cvGrabFrame(m_device);
  m_frame = cvRetrieveFrame(m_device);
  convertFrameToImage();
}

void CameraSource::convertFrameToImage()
{
  if (m_frame && cvGetSize(m_frame).width>0) {
    int x;
    int y;
    char* data = m_frame->imageData;
    for( y = 0; y < m_frame->height; y++, data += m_frame->widthStep ) {
      for( x = 0; x < m_frame->width; x++) {
        uint *p = (uint*)m_image.scanLine(y) + x;
        *p = qRgb(data[x * m_frame->nChannels+2], data[x * m_frame->nChannels+1], data[x * m_frame->nChannels]);
      }
    }
  }
  emit imageUpdated();
}

int CameraSource::dimension()
{
  return 5;
}

QImage CameraSource::image() const
{
  return m_image;
}

int CameraSource::width()
{
  return m_frame->width;
}

int CameraSource::height()
{
  return m_frame->height;
}

Point CameraSource::generatePoint()
{
  int x = qrand() % m_frame->width;
  int y = qrand() % m_frame->height;

  return pointFromXY(x, y);
}

Point CameraSource::pointFromXY(int x, int y)
{
  CvScalar s;
  s=cvGet2D(m_frame, y, x); // get the (row,col) pixel value 
  Point p;
  p[0] = normalize(x, 255);
  p[1] = normalize(y, 255);
  p[2] = s.val[2]; // bgr not rgb
  p[3] = s.val[1];
  p[4] = s.val[0];
  return p;
}

