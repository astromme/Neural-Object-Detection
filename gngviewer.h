
#ifndef _GNGVIEWER_H
#define _GNGVIEWER_H

#include <QWidget>
#include <QTimer>

namespace GNG {
  class CameraSource;
  class GrowingNeuralGas;
}
  
class GngViewer : public QWidget {
  Q_OBJECT
  public:
    GngViewer(QWidget *parent = 0);
    ~GngViewer();
    
    void setGng(GNG::GrowingNeuralGas *gng);
    void setSource(GNG::CameraSource *source);
    void setSource(const QPixmap &background);
    void setSize(int width, int height);
    
  public slots:
    void setImage(QImage image);
    
  protected:
    virtual void paintEvent(QPaintEvent* e);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void keyReleaseEvent(QKeyEvent* e);
    
  private:
    qreal unNormalize(qreal value, qreal maxValue);
    void drawTextInFrame(QPainter *painter, const QPoint &topLeft, const QString &text);
    QTimer m_repaintTimer;
    int m_width;
    int m_height;
    GNG::GrowingNeuralGas *m_gng;
    
    GNG::CameraSource *m_cameraSource;
    QPixmap m_background;
    bool m_paintBackground;
};


#endif // _GNGVIEWER_H
