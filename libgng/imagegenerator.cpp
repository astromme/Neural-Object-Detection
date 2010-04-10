
#include "imagegenerator.h"

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
  p[0] = x;
  p[1] = y;
  
  QRgb colors = m_image.pixel(x, y);
  p[2] = qRed(colors);
  p[3] = qGreen(colors);
  p[4] = qBlue(colors);
  
  return p;
}

