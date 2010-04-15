
#include "imagegenerator.h"

#include <math.h>

#include <QDebug>
#include <QList>
#include <QColor>

ImageGenerator::ImageGenerator(const QString& filePath)
: PointGenerator(),
  m_image(filePath)
{
}

ImageGenerator::~ImageGenerator()
{
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

  QColor colors = m_image.pixel(x, y);
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
  return m_image.width();
}
int ImageGenerator::height() const
{
  return m_image.height();
}
