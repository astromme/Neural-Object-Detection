#include "aibofocus.h"

// constructor
AiboFocus::AiboFocus(GrowingNeuralGas* gng, Aibo* aibo) 
  : m_gng(gng)
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
  m_gng->generateSubgraphs();
  m_gng->assignFollowSubgraph(m_focusColor);
  m_timer.start(5000);
}

void AiboFocus::followObject()
{
  m_gng->generateSubgraphs();
  m_gng->matchingSubgraph();
  Point center = m_gng->followSubgraph().center();
  qDebug() << "*********\nCenter: " << center[0] << ", " <<
    center[1] << ", " << center[2] << ", " << center[3] <<
    ", " << center[4] << "\n************";
}
