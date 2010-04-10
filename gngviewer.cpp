
#include "gngviewer.h"

#include <libgng/gng.h>
#include <libgng/node.h>
#include <libgng/edge.h>

#include <QPainter>
#include <QDebug>

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
  return maxValue*(value + 1.0)/2.0;
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
    p[2] = unNormalize(p[2], 255);
    p[3] = unNormalize(p[3], 255);
    p[4] = unNormalize(p[4], 255);
    
    QColor c = QColor(p[2], p[3], p[4]);
    painter.setBrush(c);
    painter.drawEllipse(p[0]-5, p[1]-5, 10, 10);
  }
  
  m_gng->mutex()->unlock();
}

          
        