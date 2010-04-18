#include "gngapp.h"
#include "libgng/imagesource.h"
#include "gngviewer.h"


GngApp::GngApp(int& argc, char** argv)
  : QApplication(argc, argv)
{
  m_generator = 0;
  m_viewer = 0;
  m_currentImage = 0;
  m_switchImageTimeout.setInterval(1000); // 4 seconds //TODO: UnHardcode
  connect(&m_switchImageTimeout, SIGNAL(timeout()), SLOT(switchToNextImage()));
}

void GngApp::setGenerator(ImageSource* gen)
{
  m_generator = gen;
}

void GngApp::setViewer(GngViewer* view)
{
  m_viewer = view;
}

void GngApp::addImage(const QImage& image)
{
  m_images.append(image);
}

void GngApp::runMovie()
{
  m_switchImageTimeout.start();
  exec();
}

void GngApp::switchToNextImage()
{
  m_generator->setImage(m_images[m_currentImage]);
  m_viewer->setSource(QPixmap::fromImage(m_images[m_currentImage]));
  m_currentImage = (m_currentImage + 1) % m_images.length();
}
