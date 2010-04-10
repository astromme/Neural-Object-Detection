
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
  int x = qrand() % m_image.width();
  int y = qrand() % m_image.height();
  
  Point p;
  p.resize(5);
  p[0] = x;
  p[1] = y;
  
  QRgb colors = m_image.pixel(x, y);
  p[2] = qRed(colors);
  p[3] = qGreen(colors);
  p[4] = qBlue(colors);
  
  qDebug() << p[0] << p[1];
  return p;
}

int ImageGenerator::width() const
{
  return m_image.width();
}
int ImageGenerator::height() const
{
  return m_image.height();
}
