#include "gng.h"

#include "node.h"
#include "edge.h"
#include "pointgenerator.h"

#include <math.h>

#include <QDebug>
#include <QHash>

GrowingNeuralGas::GrowingNeuralGas(int dimension, qreal minimum, qreal maximum, int updateInterval)
  : currentCycles(0),
    m_pointGenerator(0)
{
  m_dataAccess = new QMutex();
  
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
  m_targetError = 0.1;
  m_updateInterval = updateInterval;
  
  //The GNG always begins with two randomly placed units.
  m_nodes.append(new Node(Point(), dimension, minimum, maximum));
  m_nodes.append(new Node(Point(), dimension, minimum, maximum));

  m_uniqueEdges = QList<Edge*>();
  
  connectNodes(m_nodes[0], m_nodes[1]);
}

GrowingNeuralGas::~GrowingNeuralGas()
{
  delete m_dataAccess;
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
  }
  qSort(dists.begin(), dists.end(), pairLessThan);
  return QPair<Node*, Node*>(dists[0].second, dists[1].second);
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
  Node* highestError = nodeList.first();
  foreach(Node *node, nodeList) {
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
  if (m_stepCount % 10000 == 0) {
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
  
  if (averageError() > m_targetError && (m_stepsSinceLastInsert > m_stepsToInsert)) {
    qDebug() << "Creating new Node at timestep " << m_stepCount << " and error " << averageError();
    m_stepsSinceLastInsert = 0;
    insertNode();
  }
  
  reduceAllErrors();
  m_stepCount++;
  m_stepsSinceLastInsert++;
  
}

void GrowingNeuralGas::setPointGenerator(PointGenerator* pointGenerator)
{
  m_pointGenerator = pointGenerator;
}

void GrowingNeuralGas::run(int cycles)
{
  currentCycles = cycles;
  start(QThread::LowestPriority);
}

void GrowingNeuralGas::synchronousRun(int cycles)
{
  currentCycles = cycles;
  run();
}

void GrowingNeuralGas::run()
{
  Q_ASSERT(currentCycles > 0);
  Q_ASSERT(m_pointGenerator->dimension() == m_dimension);
  
  if (m_stepCount == 0) {
    qDebug() << "Running the GNG for " << currentCycles << " cycles";
  } else {
    qDebug() << "Running the GNG for " << currentCycles << " additional cycles";
  }
  
  for (int i=0; i<currentCycles; i++) {
    //usleep(1000);
    m_dataAccess->lock();
    if (m_stepCount % m_updateInterval == 0) {
      emit updated();
    }
    Point nextPoint = m_pointGenerator->generatePoint();
    step(nextPoint);
    m_dataAccess->unlock();
  }
}

QList<NodeList> GrowingNeuralGas::getSubgraphs()
{
  QHash<Node*, bool> nodeDict;
  foreach(Node* node, m_nodes){
    nodeDict.insert(node, true);
  }

  QList<NodeList> subgraphList;
  while (!nodeDict.empty()){
    NodeList subgraph;
    NodeList searchList;

    // get node in dictionary
    Node* initNode = nodeDict.begin().key();

    searchList.append(initNode);
    while (!searchList.empty()){
      // remove current node from search list
      Node* searchNode = searchList.takeFirst(); 

      // add node to subgraph since we can reach it
      subgraph.append(searchNode); 

      // by removing node from dictionary we effectively mark it as 'visited'
      nodeDict.remove(searchNode);

      // add all neighbors that can be reached and have not been visited
      QList<Node*> neighbors = searchNode->neighbors();
      foreach (Node* node, neighbors){
        // Breadth First Search
        if (nodeDict.contains(node)){
          searchList.append(node);
          nodeDict.remove(node);
        }
      }
    }
    // searchList now empty, which means subgraph has been populated with all
    // elements reachable in the given subgraph. So add that to subgraphList
    // and clear subgraph
    subgraphList.append(subgraph);
    subgraph.clear();
  }
  return subgraphList;
}

void GrowingNeuralGas::printSubgraphs(
    QList<NodeList> &subgraphs, bool printNodes)
{
  qDebug() << "Printing Subgraphs";
  qDebug() << "------------------";

  int nodesInSubgraph;
  int numSubgraphs = subgraphs.size();
  //foreach (NodeList subgraph, subgraphs){
  for (int i=0; i<numSubgraphs; i++){
    nodesInSubgraph = subgraphs[i].size();
    qDebug() << "==> Subgraph" << i+1 << "of" << numSubgraphs << 
      "contains" << nodesInSubgraph << "nodes";
    
    // only print nodes out if user asks for it
    if (printNodes){
      foreach(Node* node, subgraphs[i]){
        qDebug() << " " << node->toString();
      }
    }
  }
}

QMutex* GrowingNeuralGas::mutex() const
{
  return m_dataAccess;
}


QList< Node* > GrowingNeuralGas::nodes() const
{
  return m_nodes;
}

QList< Edge* > GrowingNeuralGas::uniqueEdges() const
{
  return m_uniqueEdges;
}

