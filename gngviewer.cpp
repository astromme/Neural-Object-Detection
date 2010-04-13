
#include "gngviewer.h"

#include <libgng/gng.h>
#include <libgng/node.h>
#include <libgng/edge.h>

#include <QPainter>
#include <QDebug>
#include <QApplication>

GngViewer::GngViewer(int width, int height, QWidget* parent)
  : QWidget(parent)
{
  resize(width, height);
  setMinimumSize(width, height);
  setMaximumSize(width, height);
  
  m_width = width;
  m_height = height;
  m_gng = 0;
  m_paintBackground = false;
}

GngViewer::~GngViewer()
{

}

void GngViewer::setGng(GrowingNeuralGas* gng)
{
  m_gng = gng;
}

void GngViewer::setSource(const QPixmap& background)
{
  setAttribute(Qt::WA_OpaquePaintEvent);
  m_background = background;
  m_paintBackground = true;
  
//   QPalette palette;
//   palette.setBrush(backgroundRole(), QBrush(background));
//   //	setPalette(palette);
}

qreal GngViewer::unNormalize(qreal value, qreal maxValue)
{
  return maxValue*value;
}

void GngViewer::paintEvent(QPaintEvent* e)
{ 
  //QWidget::paintEvent(e);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  
  if (m_paintBackground) {
    painter.drawPixmap(rect(), m_background);
  }
  
  if (!m_gng) {
    return;
  }
  
  m_gng->mutex()->lock();
  foreach(Edge *edge, m_gng->uniqueEdges()) {
    Point p1 = edge->from()->location();
    Point p2 = edge->to()->location();
    
    int x1 = unNormalize(p1[0], m_width);
    int y1 = unNormalize(p1[1], m_height);
    int x2 = unNormalize(p2[0], m_width);
    int y2 = unNormalize(p2[1], m_height);
    
    QColor c = QColor(0, 0, 0);
    painter.setBrush(c);
    painter.drawLine(x1, y1, x2, y2);
  }
  foreach(Node *node, m_gng->nodes()) {
    Point p = node->location();
    
    p[0] = unNormalize(p[0], m_width);
    p[1] = unNormalize(p[1], m_height);
    
    QColor c = QColor::fromHslF(p[2], p[3], p[4]);
    painter.setBrush(c);
    painter.drawEllipse(p[0]-5, p[1]-5, 10, 10);
  }
  
  // Draw focus area
  if (m_gng->focusing()) {
    Point focus = m_gng->focusPoint();
    
    painter.setPen(Qt::black);
    QColor transparentGray(Qt::gray);
    transparentGray.setAlphaF(0.5);
    painter.setBrush(transparentGray);
    int x = unNormalize(focus[0], m_width) - 0.05*width();
    int y = unNormalize(focus[1], m_height) - 0.05*height();
    painter.drawEllipse(x, y, 0.1*width(), 0.1*height());
  }
  
  // Draw framecount
  QColor white(Qt::white);
  white.setAlphaF(0.6);
  painter.setBrush(white);
  painter.setPen(Qt::black);
  QString stepString = QString("Frame %1").arg(m_gng->step());
  int width = QApplication::fontMetrics().width(stepString);
  int height = QApplication::fontMetrics().height();
  painter.drawRect(5, 5, 10+width, 10+height);
  painter.drawText(QPoint(10, 10+height), stepString);
  
  m_gng->mutex()->unlock();
}

          
        