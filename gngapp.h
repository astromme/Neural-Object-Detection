#ifndef _GNGAPP_H
#define _GNGAPP_H

#include <QApplication>
#include <QImage>
#include <QTimer>

class ImageGenerator;
class GngViewer;

class GngApp : public QApplication {
  Q_OBJECT
  
  public:
    GngApp(int& argc, char** argv);
    void setGenerator(ImageGenerator *gen);
    void setViewer(GngViewer *view);
    void addImage(const QImage &image);
    void runMovie();
    
  private:
    QTimer m_switchImageTimeout;
    QList<QImage> m_images;
    ImageGenerator *m_generator;
    GngViewer *m_viewer;
    int m_currentImage;
    
  private slots:
    void switchToNextImage();
};

#endif //_GNGAPP_H