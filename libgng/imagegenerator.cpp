
#include "imagegenerator.h"

#include <math.h>

#include <QDebug>
#include <QList>
#include <QColor>
#include <QMutex>

ImageGenerator::ImageGenerator(const QImage& image)
: PointGenerator(),
  m_image(image)
{
  m_dataAccess = new QMutex();
}

ImageGenerator::~ImageGenerator()
{
  delete m_dataAccess;
}

void ImageGenerator::setImage(const QImage& image)
{
  m_dataAccess->lock();
  m_image = image;
  m_dataAccess->unlock();
}

int ImageGenerator::dimension()
{
  return 5;
}

Point ImageGenerator::pointFromXY(int x, int y)
{
  Point p;
  p.resize(5);
  p[0] = normalize(x, width());
  p[1] = normalize(y, height());

  m_dataAccess->lock();
  QColor colors = m_image.pixel(x, y);
  m_dataAccess->unlock();
  colors = colors.toHsv();
  colors.getHsvF(&p[2], &p[3], &p[4]);
  //colors.getHslF(&p[2], &p[3], &p[4]);
  
  if (p[2] < 0) {
    p[2] = 0; // qt returns hue == -1 if the color is a gray/black/white
  }

  return p;
}

Point ImageGenerator::generatePoint()
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
  
Point ImageGenerator::generateNearbyPoint(const Point& nearThisPoint)
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

qreal ImageGenerator::normalize(qreal value, qreal maxValue)
{
  return value/maxValue;
}

int ImageGenerator::width() const
{
  m_dataAccess->lock();
  int width = m_image.width();
  m_dataAccess->unlock();
  return width;
}
int ImageGenerator::height() const
{
  m_dataAccess->lock();
  int height = m_image.height();
  m_dataAccess->unlock();
  return height;
}
