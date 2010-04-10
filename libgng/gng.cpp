#include "gng.h"

#include "node.h"
#include "edge.h"
#include "pointgenerator.h"

#include <math.h>

#include <QDebug>

GrowingNeuralGas::GrowingNeuralGas(int dimension, qreal minimum, qreal maximum)
{
  // Hardcoded values from paper
  m_dimension = dimension;
  m_winnerLearnRate = 0.3;
  m_neighborLearnRate = 0.01;
  m_maxAge = 50;
  m_reduceError = 0.995;
  m_stepsToInsert = 100;
  m_stepsSinceLastInsert = m_stepsToInsert + 1;
  m_insertError = 0.5;
  m_stepCount = 0;
  
  //The GNG always begins with two randomly placed units.
  m_nodes.append(new Node(Point(), dimension, minimum, maximum));
  m_nodes.append(new Node(Point(), dimension, minimum, maximum));
  
  m_uniqueEdges = QList<Edge*>();
}

GrowingNeuralGas::~GrowingNeuralGas()
{

}

QString GrowingNeuralGas::toString()
{
  return QString("GNG step %1\nNumber of units: %2\nAverage error: %3\n")
		.arg(m_stepCount).arg(m_nodes.length()).arg(averageError());
}

bool GrowingNeuralGas::unitOfInterest(Node* node, qreal cutoff)
{
  foreach(qreal part, node->location()) {
    if (abs(part) > cutoff) {
      return true;
    }
  }
  return false;
}

typedef QPair<qreal, Node*> DistNodePair;
  
bool pairLessThan(const DistNodePair &s1, const DistNodePair &s2)
{
    return s1.first < s2.first;
}
    
QPair< Node*, Node* > GrowingNeuralGas::computeDistances(const Point& point)
{
  QList<DistNodePair> dists;
  foreach(Node *node, m_nodes) {
    dists.append(DistNodePair(node->location().distanceTo(point), node));
    qSort(dists.begin(), dists.end(), pairLessThan);
    return QPair<Node*, Node*>(dists[0].second, dists[1].second);
  }
}

void GrowingNeuralGas::incrementEdgeAges(Node* node)
{
  foreach(Edge* edge, node->edges()) {
    edge->incrementAge();
    edge->to()->getEdgeTo(node)->incrementAge();
  }
}

void GrowingNeuralGas::connectNodes(Node* a, Node* b)
{
  Edge* e1 = new Edge(a, b);
  Edge* e2 = new Edge(b, a);
  
  a->appendEdge(e1);
  b->appendEdge(e2);
  
  m_uniqueEdges.append(e1);
}

void GrowingNeuralGas::disconnectNodes(Node* a, Node* b)
{
  Edge *e1 = a->getEdgeTo(b);
  Edge *e2 = b->getEdgeTo(a);
  
  a->removeEdge(e1);
  b->removeEdge(e2);
  
  m_uniqueEdges.removeAll(e1);
  m_uniqueEdges.removeAll(e2);
}

void GrowingNeuralGas::removeStaleEdges()
{
  foreach(Node *node, m_nodes) {
    foreach(Edge *edge, node->edges()) {
      if (edge->age() > m_maxAge) {
	m_uniqueEdges.removeAll(edge);
	node->removeEdge(edge);
	delete edge;
      }
    }
  }
  
  for (int i=m_nodes.length()-1; i>=0; i--) {
    Node *node = m_nodes[i];
    if (node->edges().isEmpty()) {
      m_nodes.removeAll(node);
      delete node;
    }
  }
}

Node* GrowingNeuralGas::maxErrorNode(QList< Node* > nodeList)
{
  Node* highestError = m_nodes.first();
  foreach(Node *node, m_nodes) {
    if (node->error() > highestError->error()) {
      highestError = node;
    }
  }
  return highestError;
}

qreal GrowingNeuralGas::averageError()
{
  qreal error = 0;
  foreach(Node *node, m_nodes) {
    error += node->error();
  }
  return error/m_nodes.length();
}

Point midpoint(Point *p1, Point *p2) {
  Point p3;
  p3.resize(p1->size());
  
  for (int i=0; i<p1->size(); i++) {
    p3[i] = (p1->at(i) + p2->at(i))/2;
  }
  return p3;
}

void GrowingNeuralGas::insertNode()
{
  Node *worst = maxErrorNode(m_nodes);
  Node *worstNeighbor = maxErrorNode(worst->neighbors());
  
  Point newPoint = midpoint(&worst->location(), &worstNeighbor->location());
  Node *newNode = new Node(newPoint);
  m_nodes.append(newNode);

  connectNodes(newNode, worst);
  connectNodes(newNode, worstNeighbor);

  disconnectNodes(worst, worstNeighbor);
  
  worst->setError(worst->error() * m_insertError);
  worstNeighbor->setError(worstNeighbor->error() * m_insertError);
  newNode->setError(worst->error());
}

void GrowingNeuralGas::reduceAllErrors()
{
  foreach(Node *node, m_nodes) {
    node->setError(node->error() * m_reduceError);
  }
}

void GrowingNeuralGas::step(const Point& nextPoint)
{
  if (m_stepCount % 1000) {
    qDebug() << "Step " << m_stepCount;
  }
  QPair<Node*, Node*> winner = computeDistances(nextPoint);
  incrementEdgeAges(winner.first);
  winner.first->setError(winner.first->error() + pow(winner.first->location().distanceTo(nextPoint), 2));
  winner.first->moveTowards(nextPoint, m_winnerLearnRate);
  
  foreach(Node *node, winner.first->neighbors()) {
    node->moveTowards(nextPoint, m_neighborLearnRate);
  }
  
  if (winner.first->hasEdgeTo(winner.second)) {
    winner.first->getEdgeTo(winner.second)->resetAge();
    winner.second->getEdgeTo(winner.first)->resetAge();
  } else {
    connectNodes(winner.first, winner.second);
  }
  
  removeStaleEdges();;
  
  if (averageError() > 0.05 && (m_stepsSinceLastInsert > m_stepsToInsert)) {
    qDebug() << QString("Creating new Node at timestep %1 and error %2").arg(m_stepCount).arg(averageError());
    m_stepsSinceLastInsert = 0;
    insertNode();
  }
  
  reduceAllErrors();
  m_stepCount++;
  m_stepsSinceLastInsert++;
  
}

void GrowingNeuralGas::run(int cycles, PointGenerator* pointGenerator)
{
  Q_ASSERT(cycles > 0);
  Q_ASSERT(pointGenerator->dimension() == m_dimension);
  
  if (m_stepCount == 0) {
    qDebug() << "Running the GNG for " << cycles << " cycles";
  } else {
    qDebug() << "Running the GNG for " << cycles << " additional cycles";
  }
  
  for (int i=0; i<cycles; i++) {
    Point nextPoint = pointGenerator->generatePoint();
    step(nextPoint);
  }
}
