#include "aibofocus.h"

using namespace GNG;

// constructor
AiboFocus::AiboFocus(GrowingNeuralGas* gng, Aibo* aibo) 
  : m_gng(gng),
    m_modifyX(true)
{ 
  m_aibo = new AiboControl(aibo);
  connect(&m_timer, SIGNAL(timeout()), SLOT(followObject()));
}

AiboFocus::~AiboFocus()
{
  delete m_aibo;
}

void AiboFocus::setColor(QColor focusColor)
{
  m_focusColor = focusColor.toHsl();
 
  m_aibo->startHeadControl();
  m_gng->generateSubgraphs();
  m_gng->assignFollowSubgraph(m_focusColor);
  m_timer.start(1000);
}

void AiboFocus::followObject()
{
  m_gng->generateSubgraphs();
  m_gng->matchingSubgraph();
  Point center = m_gng->followSubgraph().center();

  qDebug() << "*********\nCenter: " << center[0] << ", " <<
    center[1] << ", " << center[2] << ", " << center[3] <<
    ", " << center[4] << "\n************";
  
  // get x and y coordinates of center
  qreal x = center[0];
  qreal y = center[1];
  
 
  if (m_modifyX){
    if (x < 0.33){
      qDebug() << "Aibo Move Left";
      m_aibo->moveFocusLeft();
    } else if (x > 0.66){
      qDebug() << "Aibo Move Right";
      m_aibo->moveFocusRight();
    } else {
      qDebug() << "Aibo Stop Moving";
      m_aibo->moveFocusCenter();
    }
  } else {
    if (y < 0.66){
      m_aibo->moveFocusUp();
    } else {
      m_aibo->moveFocusDown();
    }
  }
  
  m_modifyX = !m_modifyX;
 
}
