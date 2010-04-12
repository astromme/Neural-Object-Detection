
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

Point ImageGenerator::generatePoint()
{
  int x, y;
  QColor colors;
  bool backgroundPoint = true;

  QList<int> backgroundColor;
  backgroundColor << 0 << 0 << 0;

  // rand points until one isn't a background point
  while (backgroundPoint) {
    x = qrand() % width();
    y = qrand() % height();
    colors = m_image.pixel(x, y);
    colors = colors.toHsv();

    backgroundPoint = false;
    
    //     if ((abs(colors.hue() - backgroundColor[0]) > 50)
    //      || (abs(colors.saturation() - backgroundColor[1]) > 50)
    //      || (abs(colors.value() - backgroundColor[2]) > 50)) {
    //if ((abs(colors.saturation() - backgroundColor[0]) > 10)) {
    //  backgroundPoint = false;
    //}
  }

  Point p;
  p.resize(5);
  p[0] = normalize(x, width());
  p[1] = normalize(y, height());

  colors.getHsvF(&p[2], &p[3], &p[4]);
  
  if (p[2] < 0) {
    p[2] = 0; // qt returns hue == -1 if the color is a gray/black/white
  }

  return p;
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
