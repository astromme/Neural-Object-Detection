
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

Node::Node(Point location, int dimension, qreal min, qreal max)
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
    
Node::~Node()
{
}

Point Node::location()
{
  return m_location;
}

QString Node::toString() const
{
  QString string = "[";
  foreach(qreal part, m_location) {
    string.append(QString::number(part) + ", ");
  }
  string.append("]\n");
  return string;
}

bool Node::hasEdgeTo(const Node* other) const
{
  foreach(Node *neighbor, neighbors()) {
    if (other == neighbor) {
      return true;
    }
  }
  return false;
}

Edge* Node::getEdgeTo(const Node* other) const
{
  foreach(Edge* edge, m_edges) {
    if (edge->to() == other) {
      return edge;
    }
  }
  return 0;
}

void Node::appendEdge(Edge* edge)
{
  m_edges.append(edge);
}

void Node::removeEdge(Edge* edge)
{
  m_edges.removeAll(edge);
}


QList<Node*> Node::neighbors() const
{
  QList<Node*> neighbors;
  foreach(Edge* edge, m_edges) {
    neighbors.append(edge->to());
  }
  return neighbors;
}

QList< Edge* > Node::edges() const
{
  return m_edges;
}

qreal Node::error() const
{
  return m_error;
}

void Node::setError(qreal error)
{
  m_error = error;
}

void Node::moveTowards(const Point& point, qreal learningRate)
{
  for (int i=0; i<point.size(); i++) {
    m_location[i] += learningRate*(point[i]-m_location[i]);
  }
}
