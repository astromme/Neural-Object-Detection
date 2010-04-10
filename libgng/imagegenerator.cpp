
#include "imagegenerator.h"

#include <QDebug>

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
  int x = qrand() % width();
  int y = qrand() % height();
  
  Point p;
  p.resize(5);
  p[0] = normalize(x, width());
  p[1] = normalize(y, height());
  
  QRgb colors = m_image.pixel(x, y);
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
