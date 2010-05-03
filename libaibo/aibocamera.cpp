#include "aibocamera.h"
#include <QPainter>
#include <QApplication>
#include "libaibo/aibo.h"


AiboCamera::AiboCamera(QWidget* parent)
  : QWidget(parent)
{
  setAttribute(Qt::WA_OpaquePaintEvent);
}
AiboCamera::~AiboCamera()
{

}

void AiboCamera::updateImage(QImage image)
{
  if (image.size() != m_pixmap.size()) {
    resize(image.size());
  }
  m_pixmap = QPixmap::fromImage(image);
  repaint();
}

void AiboCamera::paintEvent(QPaintEvent* event)
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
  AiboCamera cam;
  Aibo aibo(argv[1]);
  
  aibo.startCamera();
  cam.show();

  QObject::connect(&aibo, SIGNAL(cameraFrame(QImage)), &cam, SLOT(updateImage(QImage)));
  
  app.exec();
};
