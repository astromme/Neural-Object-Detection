#ifndef AIBOCAM_H
#define AIBOCAM_H

#include <QWidget>
#include <QPixmap>

class AiboCam : public QWidget {
  Q_OBJECT
  public:
    AiboCam(QWidget* parent=0);
    virtual ~AiboCam();
    
  public slots:
    void updateImage(QImage image);
    
  protected:
    virtual void paintEvent(QPaintEvent* event);
    
  private:
    QPixmap m_pixmap;
};

#endif //AIBOCAM_H