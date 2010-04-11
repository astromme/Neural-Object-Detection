
#include "imagegenerator.h"

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

void ImageGenerator::nextPixel(int* x, int* y)
{
  if (*x < width()-1) {
    (*x)++;
    return;
  } else {
    *x = 0;
    if (*y < height()-1) {
      (*y)++;
      return;
    } else {
      *y = 0;
    }
  }
}

Point ImageGenerator::generatePoint()
{
  static int lastX = 1;
  static int lastY = 1;
  
  int x, y;
  QColor colors;
  bool backgroundPoint = true;
  
  QList<int> backgroundColor;
  backgroundColor << 0 << 0 << 0;
  
  // rand points until one isn't a background point
  while (backgroundPoint) {
    x = qrand() % width();
    y = qrand() % height();
    //qDebug() << lastX << lastY;	
    colors = m_image.pixel(x, y);
    
//     if ((abs(colors.hue() - backgroundColor[0]) > 50)
//      || (abs(colors.saturation() - backgroundColor[1]) > 50)
//      || (abs(colors.value() - backgroundColor[2]) > 50)) {
      if ((abs(colors.saturation() - backgroundColor[0]) > 10)) {
	backgroundPoint = false;
    }
  }
   
  Point p;
  p.resize(5);
  p[0] = normalize(x, width());
  p[1] = normalize(y, height());
  
  p[2] = normalize(colors.hue(), 255);
  p[3] = normalize(colors.saturation(), 255);
  p[4] = normalize(colors.value(), 255);
  
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
