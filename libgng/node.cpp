
#include "node.h"

#include "edge.h"

#include <QDebug>
#include <cstdlib>

qreal realRand(qreal minimum, qreal maximum) {
  qreal rand = ((float)qrand())/RAND_MAX;
  rand *= maximum-minimum;
  rand += minimum;
  
  return rand;
}

GngNode::GngNode(Point location, int dimension, qreal min, qreal max)
{
  m_dimension = dimension;
  m_min = min;
  m_max = max;
  m_location = location;
  m_error = 0;
  

  if (m_location.isEmpty()) {
    m_location.resize(dimension);
    for (int i=0; i<dimension; i++) {
      m_location[i] = realRand(min, max);
    }
  }
}

GngNode::~GngNode()
{
}

Point GngNode::location()
{
  return m_location;
}

QString GngNode::toString() const
{
  QString string = "[";
  foreach(qreal part, m_location) {
    string.append(QString::number(part) + ", ");
  }
  string.append("]");
  return string;
}

bool GngNode::hasEdgeTo(const GngNode* other) const
{
  foreach(GngNode *neighbor, neighbors()) {
    if (other == neighbor) {
      return true;
    }
  }
  return false;
}

Edge* GngNode::getEdgeTo(const GngNode* other) const
{
  foreach(Edge* edge, m_edges) {
    if (edge->to() == other) {
      return edge;
    }
  }
  return 0;
}

void GngNode::appendEdge(Edge* edge)
{
  m_edges.append(edge);
}

void GngNode::removeEdge(Edge* edge)
{
  m_edges.removeAll(edge);
}

QList<GngNode*> GngNode::neighbors() const
{
  QList<GngNode*> neighbors;
  foreach(Edge* edge, m_edges) {
    neighbors.append(edge->to());
  }
  return neighbors;
}

QList< Edge* > GngNode::edges() const
{
  return m_edges;
}

qreal GngNode::error() const
{
  return m_error;
}

void GngNode::setError(qreal error)
{
  m_error = error;
}

// adjust position of node towards a given point and a learningRate
void GngNode::moveTowards(const Point& point, qreal learningRate)
{
  for (int i=0; i<point.size(); i++) {
    m_location[i] += learningRate*(point[i]-m_location[i]);
  }
}
