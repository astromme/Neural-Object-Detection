#include "aibocam.h"
#include <QPainter>
#include <QApplication>
#include "libaibo/aibo.h"


AiboCam::AiboCam(QWidget* parent)
  : QWidget(parent)
{
  setAttribute(Qt::WA_OpaquePaintEvent);
}
AiboCam::~AiboCam()
{

}

void AiboCam::updateImage(QImage image)
{
  if (image.size() != m_pixmap.size()) {
    resize(image.size());
  }
  m_pixmap = QPixmap::fromImage(image);
  repaint();
}

void AiboCam::paintEvent(QPaintEvent* event)
{
  QPainter p(this);
  p.drawPixmap(rect(), m_pixmap);
}


int main(int argc, char* argv[]) {
  if (argc < 2) {
    qDebug() << "Usage:" << argv[0] << "[hostname]";
    return 1;
  }
  
  QApplication app(argc, argv);
  AiboCam cam;
  Aibo aibo(argv[1]);
  
  aibo.startCamera();
  cam.show();

  QObject::connect(&aibo, SIGNAL(cameraFrame(QImage)), &cam, SLOT(updateImage(QImage)));
  
  app.exec();
};
