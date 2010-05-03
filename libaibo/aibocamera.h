#ifndef AIBOCAM_H
#define AIBOCAM_H

#include <QWidget>
#include <QPixmap>

class AiboCamera : public QWidget {
  Q_OBJECT
  public:
    AiboCamera(QWidget* parent=0);
    virtual ~AiboCamera();
    
  public slots:
    void updateImage(QImage image);
    
  protected:
    virtual void paintEvent(QPaintEvent* event);
    
  private:
    QPixmap m_pixmap;
};

#endif //AIBOCAM_H