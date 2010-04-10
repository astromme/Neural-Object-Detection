
#include "edge.h"

Edge::Edge(Node* from, Node* to)
{
  m_from = from;
  m_to = to;
  m_age = 0;
}

Edge::~Edge()
{
}

Node* Edge::from() const
{
  return m_from;
}

Node* Edge::to() const
{
  return m_to;
}

int Edge::age() const
{
  return m_age;
}

void Edge::incrementAge()
{
  m_age++;
}


