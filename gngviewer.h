
#ifndef _GNGVIEWER_H
#define _GNGVIEWER_H

#include <QWidget>
#include <QTimer>

class CameraSource;
class GrowingNeuralGas;

class GngViewer : public QWidget {
  public:
    GngViewer(QWidget *parent = 0);
    ~GngViewer();
    
    void setGng(GrowingNeuralGas *gng);
    void setSource(CameraSource *source);
    void setSource(const QPixmap &background);
    void setSize(int width, int height);
    
  protected:
    virtual void paintEvent(QPaintEvent* e);
    
  private:
    qreal unNormalize(qreal value, qreal maxValue);
    void drawTextInFrame(QPainter *painter, const QPoint &topLeft, const QString &text);
    QTimer m_repaintTimer;
    int m_width;
    int m_height;
    GrowingNeuralGas *m_gng;
    
    CameraSource *m_cameraSource;
    QPixmap m_background;
    bool m_paintBackground;
};


#endif // _GNGVIEWER_H
