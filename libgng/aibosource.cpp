#include "aibosource.h"

#include <QColor>
#include <QtNetwork/QTcpSocket>
#include <QMutex>

AiboSource::AiboSource(const QString& hostname, QObject* parent)
  : Aibo(hostname, parent)
{
  m_dataAccess = new QMutex();
  connect(this, SIGNAL(cameraFrame(QImage)), SLOT(setFrame(QImage)));
}

AiboSource::~AiboSource()
{

}

int AiboSource::dimension()
{
  return 5;
}

void AiboSource::setFrame(QImage frame)
{
  m_dataAccess->lock();
  m_tempImage = frame;
  m_dataAccess->unlock();
}

Point AiboSource::generatePoint()
{
  m_dataAccess->lock();
  
  if (!isCameraRunning()) {
    Point p;
    p.resize(dimension());
    p.fill(0);
    return p;
  }
  int x = qrand() % cameraImage().width();
  int y = qrand() % cameraImage().height();
  
  QColor rgb = cameraImage().pixel(x, y);
  QColor hsl = rgb.toHsl();
  
  Point p;
  p.resize(dimension());
  p[0] = normalize(x, cameraImage().width());
  p[1] = normalize(y, cameraImage().height());
  
  hsl.getHslF(&p[2], &p[3], &p[4]);
  
  m_dataAccess->unlock();
  return p;
}

