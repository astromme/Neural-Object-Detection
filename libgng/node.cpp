
#include "node.h"

#include "edge.h"

Node::Node(QVector<qreal> vector, int dimension, int min, int max)
{
  m_dimension = dimension;
  m_min = min;
  m_max = max;
  m_vector = vector;
  m_error = 0;
  
  if (m_vector.isEmpty()) {
    m_vector.resize(dimension);
    for (int i=0; i<dimension; i++) {
      m_vector[i] = min + ((qrand()-min) % max);
    }
  }
}
    
Node::~Node()
{
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

QList<Node*> Node::neighbors() const
{
  QList<Node*> neighbors;
  foreach(Edge* edge, m_edges) {
    neighbors.append(edge->to());
  }
  return neighbors;
}

void Node::moveTowards(const QVector<qreal> &point, int learningRate)
{
  for (int i=0; i<point.size(); i++) {
    m_vector[i] += learningRate*(point[i]-m_vector[i]);
  }
}
