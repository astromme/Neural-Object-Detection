
#include "gngviewer.h"

#include <libgng/gng.h>
#include <libgng/node.h>
#include <libgng/edge.h>
#include <libgng/camerasource.h>

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QKeyEvent>


GngViewer::GngViewer(QWidget* parent)
  : QWidget(parent)
{
  setSize(200, 200);
  m_gng = 0;
  m_cameraSource = 0;
  m_paintBackground = false;
  
  // Set widget to repaint 25 times per second
  m_repaintTimer.setInterval(40);
  connect(&m_repaintTimer, SIGNAL(timeout()), SLOT(update()));
}

GngViewer::~GngViewer()
{

}

void GngViewer::setSize(int width, int height)
{
  resize(width, height);
  setMinimumSize(width, height);
  setMaximumSize(width, height);
  
  m_width = width;
  m_height = height;
}

void GngViewer::setGng(GrowingNeuralGas* gng)
{
  m_gng = gng;
  update();
  m_repaintTimer.start();
}

void GngViewer::setSource(const QPixmap& background)
{
  setAttribute(Qt::WA_OpaquePaintEvent);
  m_background = background;
  m_paintBackground = true;
  m_repaintTimer.start();
  
//   QPalette palette;
//   palette.setBrush(backgroundRole(), QBrush(background));
//   //	setPalette(palette);
}

void GngViewer::setSource(CameraSource* source)
{
  setAttribute(Qt::WA_OpaquePaintEvent);
  m_cameraSource = source;
  m_paintBackground = true;
}

void GngViewer::setImage(QImage image)
{
  setSource(QPixmap::fromImage(image));
}

qreal GngViewer::unNormalize(qreal value, qreal maxValue)
{
  return maxValue*value;
}

void GngViewer::drawTextInFrame(QPainter *painter, const QPoint& topLeft, const QString& text)
{
  QColor white(Qt::white);
  white.setAlphaF(0.6);
  painter->setBrush(white);
  painter->setPen(Qt::black);
  int width = QApplication::fontMetrics().width(text);
  int height = QApplication::fontMetrics().height();
  painter->drawRect(topLeft.x(), topLeft.y(), 10+width, 10+height);
  painter->drawText(QPoint(topLeft.x()+5, topLeft.y()+5+height), text);
}

void GngViewer::paintEvent(QPaintEvent* e)
{ 
  //QWidget::paintEvent(e);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  
  if (m_paintBackground) {
    if (m_cameraSource) {
      m_background = QPixmap::fromImage(m_cameraSource->image());
    }
    painter.drawPixmap(rect(), m_background);
  }
  
  if (!m_gng) {
    return;
  }
  
  painter.save();
  painter.setPen(Qt::NoPen);
  
  // make sure we have up to date subgraphs (so no nodes have been deleted)
  m_gng->generateSubgraphs();
  
  // Draw subgraph colors behind edges
  foreach(Subgraph s, m_gng->subgraphs()) {
    qreal hue = 0;
    foreach(GngNode *n, s) {
      hue += n->location()[2];
    }
    //qreal hue = s.first()->location()[2]; //hue of first point
    hue /= s.length();
    qreal sat = s.first()->location()[3];
    qreal light = s.first()->location()[4];
    QColor color = QColor::fromHslF(hue, 0.5, 0.5, 0.3);
    QPen pen(color);
    pen.setWidth(5);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
        
    foreach(GngNode *node, s) {
      foreach(GngNode *neighbor, node->neighbors()) {
        Point p1 = node->location();
        Point p2 = neighbor->location();
        
        int x1 = unNormalize(p1[0], m_width);
        int y1 = unNormalize(p1[1], m_height);
        int x2 = unNormalize(p2[0], m_width);
        int y2 = unNormalize(p2[1], m_height);
        
        painter.drawLine(x1, y1, x2, y2);
      }
    }
  }
  painter.restore();
  
  // Draw the actual edges
  foreach(Edge *edge, m_gng->uniqueEdges()) {
    Point p1 = edge->from()->location();
    Point p2 = edge->to()->location();
    
    int x1 = unNormalize(p1[0], m_width);
    int y1 = unNormalize(p1[1], m_height);
    int x2 = unNormalize(p2[0], m_width);
    int y2 = unNormalize(p2[1], m_height);
    
    QColor c = QColor(0, 0, 0);
    painter.setPen(c);
    painter.drawLine(x1, y1, x2, y2);
    
    int age = m_gng->edgeHistoryAge(edge);
    int edgeX = abs(x2 - x1)/2 + qMin(x1, x2);
    int edgeY = abs(y2 - y1)/2 + qMin(y1, y2);
   
    if (edge->totalAge() < 2000)
      drawTextInFrame(&painter, QPoint(edgeX, edgeY), QString::number(edge->totalAge()));// QString("%1 : %2").arg(age).arg(edge->totalAge()));
    //drawTextInFrame(&painter, QPoint(edgeX, edgeY), QString::number(edge->lastUpdated()));// QString("%1 : %2").arg(age).arg(edge->totalAge()));
  }
  
  // Draw the nodes
  foreach(GngNode *node, m_gng->nodes()) {
    Point p = node->location();
    
    p[0] = unNormalize(p[0], m_width);
    p[1] = unNormalize(p[1], m_height);
    
    //QColor c = QColor::fromHsvF(p[2], p[3], p[4]);
    QColor c = QColor::fromHslF(p[2], p[3], p[4]);
    //QColor c = QColor::fromHslF(p[2], 1, .5);
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
    painter.drawEllipse(x, y, 0.1*width(), 0.1*height()); //TODO Don't hardcode
  }
  
  // Draw stepcount
  QString stepString = QString("Step %L1").arg(m_gng->currentStep());
  drawTextInFrame(&painter, QPoint(5, 5), stepString);
  drawTextInFrame(&painter, QPoint(5, 34), QString("%L1s").arg((qreal)m_gng->elapsedTime()/1000, 0, 'f', 2));
  
}

void GngViewer::keyPressEvent(QKeyEvent* e)
{
  if (e->key() == Qt::Key_Space) {
    e->accept();
  }
}
void GngViewer::keyReleaseEvent(QKeyEvent* e)
{
  if (e->key() == Qt::Key_Space) {
    if (m_gng) {
      qDebug() << "toggle playpause";
      m_gng->togglePause();
    }
  }
}

          
        
