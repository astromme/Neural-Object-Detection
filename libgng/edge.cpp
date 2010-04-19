
#include "edge.h"

Edge::Edge(Node* from, Node* to)
{
  m_from = from;
  m_to = to;
  m_age = 0;
  m_totalAge = 0;
  m_lastUpdated = 0;
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

void Edge::resetAge()
{
  m_age = 0;
}

void Edge::incrementAge()
{
  m_age++;
}
void Edge::setAge(int age)
{
  m_age = age;
}


int Edge::totalAge() const
{
  return m_totalAge;
}
void Edge::incrementTotalAge()
{
  m_totalAge++;
}

int Edge::lastUpdated()
{
  return m_lastUpdated;
}
void Edge::setLastUpdated(int time)
{
  m_lastUpdated = time;
}


