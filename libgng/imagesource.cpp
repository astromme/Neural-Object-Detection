
#include "imagesource.h"

#include <math.h>

#include <QDebug>
#include <QList>
#include <QColor>
#include <QMutex>

using namespace GNG;

ImageSource::ImageSource(const QImage& image)
: PointSource(),
  m_image(image)
{
  m_dataAccess = new QMutex();
}

ImageSource::~ImageSource()
{
  delete m_dataAccess;
}

void ImageSource::setImage(const QImage& image)
{
  m_dataAccess->lock();
  m_image = image;
  m_dataAccess->unlock();
}

int ImageSource::dimension()
{
  return 5;
}

Point ImageSource::pointFromXY(int x, int y)
{
  Point p(dimension());
  p[0] = normalize(x, width());
  p[1] = normalize(y, height());

  m_dataAccess->lock();
  QColor colors = m_image.pixel(x, y);
  m_dataAccess->unlock();
  //colors = colors.toHsv();
  colors = colors.toHsl();
  //colors.getHsvF(&p[2], &p[3], &p[4]);
  colors.getHslF(&p[2], &p[3], &p[4]);
  
  if (p[2] < 0) {
    p[2] = 0; // qt returns hue == -1 if the color is a gray/black/white
  }

  return p;
}

Point ImageSource::generatePoint()
{
  int x = qrand() % width();
  int y = qrand() % height();
      
    //     if ((abs(colors.hue() - backgroundColor[0]) > 50)
    //      || (abs(colors.saturation() - backgroundColor[1]) > 50)
    //      || (abs(colors.value() - backgroundColor[2]) > 50)) {
    //if ((abs(colors.saturation() - backgroundColor[0]) > 10)) {
    //  backgroundPoint = false;
    //}

  return pointFromXY(x, y);
}
  
Point ImageSource::generateNearbyPoint(const Point& nearThisPoint)
{
//   // Create gaussian distribution
//   qreal x1, x2, w, y1, y2;
// 
//   do {
//     x1 = 2.0*qrand()-1.0;
//     x2 = 2.0*qrand()-1.0;
//     w = x1*x1+x2*x2;
//   } while (w >= 1.0);
// 
//   w = sqrt((-2.0*log(w))/w);
//   y1 = x1*w;
//   y2 = x2*w;

  // Unnormalize
  int x = nearThisPoint[0]*width();
  int y = nearThisPoint[1]*height();
  
  int xrange = 0.1*width();
  int yrange = 0.1*height();
  
  int closeX = qrand() % xrange;
  int closeY = qrand() % yrange;
  
  closeX -= xrange/2;
  closeY -= yrange/2;

  //qDebug() << "Picking close to (" << x << "," << y << ")" << x+closeX << y+closeY;
  closeX = qBound(0, x+closeX, width()-1);
  closeY = qBound(0, y+closeY, height()-1);
  return pointFromXY(closeX, closeY);
}

int ImageSource::width() const
{
  m_dataAccess->lock();
  int width = m_image.width();
  m_dataAccess->unlock();
  return width;
}
int ImageSource::height() const
{
  m_dataAccess->lock();
  int height = m_image.height();
  m_dataAccess->unlock();
  return height;
}
