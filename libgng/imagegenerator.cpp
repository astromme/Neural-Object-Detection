
#include "imagegenerator.h"

#include <QDebug>
#include <QList>

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

Point ImageGenerator::generatePoint()
{
  int x, y;
  QRgb colors;
  bool backgroundPoint = true;
  
  QList<int> backgroundColor;
  backgroundColor << 255 << 255 << 255;
  
  // rand points until one isn't a background point
  while (backgroundPoint) {
    x = qrand() % width();
    y = qrand() % height();
    colors = m_image.pixel(x, y);
    
    if ((abs(qRed(colors) - backgroundColor[0]) > 50)
     || (abs(qGreen(colors) - backgroundColor[1]) > 50)
     || (abs(qBlue(colors) - backgroundColor[2]) > 50)) {
      backgroundPoint = false;
    }
  }
   
  Point p;
  p.resize(5);
  p[0] = normalize(x, width());
  p[1] = normalize(y, height());
  
  p[2] = normalize(qRed(colors), 255);
  p[3] = normalize(qGreen(colors), 255);
  p[4] = normalize(qBlue(colors), 255);
  
  return p;
}

qreal ImageGenerator::normalize(qreal value, qreal maxValue)
{
  return 2.0*value/maxValue-1;
}

int ImageGenerator::width() const
{
  return m_image.width();
}
int ImageGenerator::height() const
{
  return m_image.height();
}
