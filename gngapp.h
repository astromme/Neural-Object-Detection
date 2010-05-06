#ifndef _GNGAPP_H
#define _GNGAPP_H

#include <QApplication>
#include <QImage>
#include <QTimer>

namespace GNG {
  class ImageSource;
}

class GngViewer;

class GngApp : public QApplication {
  Q_OBJECT
  
  public:
    GngApp(int& argc, char** argv);
    void setGenerator(GNG::ImageSource *gen);
    void setViewer(GngViewer *view);
    void addImage(const QImage &image);
    void runMovie();
    
  private:
    QTimer m_switchImageTimeout;
    QList<QImage> m_images;
    GNG::ImageSource *m_generator;
    GngViewer *m_viewer;
    int m_currentImage;
    
  private slots:
    void switchToNextImage();
};

#endif //_GNGAPP_H