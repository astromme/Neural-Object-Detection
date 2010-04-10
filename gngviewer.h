
#ifndef _GNGVIEWER_H
#define _GNGVIEWER_H

#include <QWidget>

class GrowingNeuralGas;

class GngViewer : public QWidget {
  public:
    GngViewer(int width, int height, QWidget *parent = 0);
    ~GngViewer();
    
    void setGng(GrowingNeuralGas *gng);
    void setSource(const QPixmap &background);
    
  protected:
    virtual void paintEvent(QPaintEvent* e);
    
  private:
    qreal unNormalize(qreal value, qreal maxValue);
    int m_width;
    int m_height;
    GrowingNeuralGas *m_gng;
    
    QPixmap m_background;
    bool m_paintBackground;
};


#endif // _GNGVIEWER_H
