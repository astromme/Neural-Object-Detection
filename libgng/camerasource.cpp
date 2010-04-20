#include "camerasource.h"

#include <QDebug>
#include <QColor>

static QImage IplImageToQImage(const IplImage  *iplImage, uchar **data , bool mirroRimage = true );

CameraSource::CameraSource()
{
  m_dataAccess = new QMutex();
  m_nextFrameTimer.setInterval(40);
  connect(&m_nextFrameTimer, SIGNAL(timeout()), SLOT(processNextFrame()));
  m_frame = 0;
  m_imageData = 0;
  m_device = cvCreateCameraCapture(1);
  if (!m_device){
    qDebug() << "Camera not found";
    return;
  }
  //m_image = QImage(QSize(width(), height()), QImage::Format_RGB32);
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
}

void CameraSource::processNextFrame()
{
  m_dataAccess->lock();
  cvGrabFrame(m_device);
  m_frame = cvRetrieveFrame(m_device);
  convertFrameToImage();
  m_dataAccess->unlock();
  emit imageUpdated();
}

void CameraSource::convertFrameToImage()
{
  if (m_imageData) {
    delete m_imageData;
  }
  m_image = IplImageToQImage(m_frame, &m_imageData, false);
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
  return m_frame->width; //TODO: Why not working?
  //return cvGetCaptureProperty(m_device, CV_CAP_PROP_FRAME_WIDTH);
}

int CameraSource::height()
{
  return m_frame->height;
  //return cvGetCaptureProperty(m_device, CV_CAP_PROP_FRAME_HEIGHT);
}

Point CameraSource::generatePoint()
{
  int x = qrand() % width();
  int y = qrand() % height();

  return pointFromXY(x, y);
}

Point CameraSource::pointFromXY(int x, int y)
{
  CvScalar s;
  m_dataAccess->lock();
  s=cvGet2D(m_frame, y, x); // get the (row,col) pixel value 
  m_dataAccess->unlock();
  
  // 2 1 0 because data is BGR not RGB
  QColor rgb = QColor::fromRgb(s.val[2], s.val[1], s.val[0]);
  QColor hsl = rgb.toHsl();
  
  Point p;
  p.resize(dimension());
  p[0] = normalize(x, width());
  p[1] = normalize(y, height());

  hsl.getHslF(&p[2], &p[3], &p[4]);
  return p;
}


// Converting a IPL Image to a QImage
// Code from http://www.qtcentre.org/threads/11655-OpenCV-integration?p=72374#post72374
// 

#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <ctype.h>

static QImage IplImageToQImage(const IplImage  *iplImage, uchar **data , bool mirroRimage )
{
  uchar *qImageBuffer = NULL;
  int    width        = iplImage->width;

  /*
   * Note here that OpenCV image is stored so that each lined is 32-bits aligned thus
   * explaining the necessity to "skip" the few last bytes of each line of OpenCV image buffer.
   */
  int widthStep = iplImage->widthStep;
  int height    = iplImage->height;

  switch (iplImage->depth)
  {
    case IPL_DEPTH_8U:
      if (iplImage->nChannels == 1)
      {
        /* OpenCV image is stored with one byte grey pixel. We convert it to an 8 bit depth QImage. */
        qImageBuffer = (uchar *)malloc(width * height * sizeof(uchar));
        uchar *QImagePtr = qImageBuffer;
        const uchar *iplImagePtr = (const uchar *)iplImage->imageData;
        for (int y = 0; y < height; ++y)
        {
          // Copy line by line
          memcpy(QImagePtr, iplImagePtr, width);
          QImagePtr += width;
          iplImagePtr += widthStep;
        }
      }
      else if (iplImage->nChannels == 3)
      {
        /* OpenCV image is stored with 3 byte color pixels (3 channels). We convert it to a 32 bit depth QImage. */
        qImageBuffer = (uchar *)malloc(width * height * 4 * sizeof(uchar));
        uchar *QImagePtr = qImageBuffer;
        const uchar *iplImagePtr = (const uchar *)iplImage->imageData;

        for (int y = 0; y < height; ++y)
        {
          for (int x = 0; x < width; ++x)
          {
            // We cannot help but copy manually.
            QImagePtr[0] = iplImagePtr[0];
            QImagePtr[1] = iplImagePtr[1];
            QImagePtr[2] = iplImagePtr[2];
            QImagePtr[3] = 0;

            QImagePtr += 4;
            iplImagePtr += 3;
          }
          iplImagePtr += widthStep - 3 * width;
        }
      }
      else
        qDebug("IplImageToQImage: image format is not supported : depth=8U and %d channels\n", iplImage->nChannels);

      break;

    case IPL_DEPTH_16U:
      if (iplImage->nChannels == 1)
      {
        /* OpenCV image is stored with 2 bytes grey pixel. We convert it to an 8 bit depth QImage. */
        qImageBuffer = (uchar *)malloc(width * height * sizeof(uchar));
        uchar *QImagePtr = qImageBuffer;
        const uint16_t *iplImagePtr = (const uint16_t *)iplImage->imageData;

        for (int y = 0; y < height; ++y)
        {
          for (int x = 0; x < width; ++x)
            *QImagePtr++ = ((*iplImagePtr++) >> 8); // We take only the highest part of the 16 bit value. It is similar to dividing by 256.
          iplImagePtr += widthStep / sizeof(uint16_t) - width;
        }
      }
      else
        qDebug("IplImageToQImage: image format is not supported : depth=16U and %d channels\n", iplImage->nChannels);

      break;

    case IPL_DEPTH_32F:
      if (iplImage->nChannels == 1)
      {
        /* OpenCV image is stored with float (4 bytes) grey pixel. We convert it to an 8 bit depth QImage. */
        qImageBuffer = (uchar *)malloc(width * height * sizeof(uchar));
        uchar *QImagePtr = qImageBuffer;
        const float *iplImagePtr = (const float *)iplImage->imageData;

        for (int y = 0; y < height; ++y)
        {
          for (int x = 0; x < width; ++x)
            *QImagePtr++ = (uchar)(255 * ((*iplImagePtr++)));
          iplImagePtr += widthStep / sizeof(float) - width;
        }
      }
      else
        qDebug("IplImageToQImage: image format is not supported : depth=32F and %d channels\n", iplImage->nChannels);

      break;

    case IPL_DEPTH_64F:
      if (iplImage->nChannels == 1)
      {
        /* OpenCV image is stored with double (8 bytes) grey pixel. We convert it to an 8 bit depth QImage. */
        qImageBuffer = (uchar *) malloc(width * height * sizeof(uchar));
        uchar *QImagePtr = qImageBuffer;
        const double *iplImagePtr = (const double *) iplImage->imageData;

        for (int y = 0; y < height; ++y)
        {
          for (int x = 0; x < width; ++x)
            *QImagePtr++ = (uchar)(255 * ((*iplImagePtr++)));
          iplImagePtr += widthStep / sizeof(double) - width;
        }
      }
      else
        qDebug("IplImageToQImage: image format is not supported : depth=64F and %d channels\n", iplImage->nChannels);

      break;

    default:
      qDebug("IplImageToQImage: image format is not supported : depth=%d and %d channels\n", iplImage->depth, iplImage->nChannels);
  }

  QImage *qImage;
  if (iplImage->nChannels == 1)
  {
    QVector<QRgb> colorTable;
    for (int i = 0; i < 256; i++)
      colorTable.push_back(qRgb(i, i, i));

    qImage = new QImage(qImageBuffer, width, height, QImage::Format_Indexed8);
    qImage->setColorTable(colorTable);
  }
  else
    qImage = new QImage(qImageBuffer, width, height, QImage::Format_RGB32);
  
  QImage gd0 = qImage->mirrored(false,mirroRimage);
  *data = qImageBuffer;
  
  return gd0;
}

