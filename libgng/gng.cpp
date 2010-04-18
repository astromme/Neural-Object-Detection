#include "gng.h"

#include "node.h"
#include "edge.h"
#include "pointgenerator.h"

#include <math.h>

#include <QDebug>
#include <QHash>

// constructor
GrowingNeuralGas::GrowingNeuralGas(int dimension, qreal minimum, qreal maximum)
  : currentCycles(0),
    m_pointGenerator(0),
    m_pickCloseToCountdown(0)
{
  m_dataAccess = new QMutex();
  
  // Hardcoded values from paper
  m_dimension = dimension;
  setWinnerLearnRate(0.1);
  setNeighborLearnRate(0.01); // TODO was .01
  setMaxEdgeAge(50);
  setErrorReduction(0.1); // TODO was 0.005
  setNodeInsertionDelay(50);
  m_stepsSinceLastInsert = m_minStepsBetweenInsertions + 1;
  setInsertErrorReduction(0.5);
  m_stepCount = 0;
  setTargetError(0.001); // TODO was 0.1
  setDelay(0);
  setUpdateInterval(5000);
  
  m_min = minimum;
  m_max = maximum;
  
  //The GNG always begins with two randomly placed units.
  m_nodes.append(new Node(Point(), dimension, minimum, maximum));
  m_nodes.append(new Node(Point(), dimension, minimum, maximum));

  m_uniqueEdges = QList<Edge*>();
  
  connectNodes(m_nodes[0], m_nodes[1]);
}

// destructor
GrowingNeuralGas::~GrowingNeuralGas()
{
  delete m_dataAccess;
}

// converts gng to a string to print out
QString GrowingNeuralGas::toString()
{
  return QString("GNG step %1\nNumber of units: %2\nAverage error: %3\n")
		.arg(m_stepCount).arg(m_nodes.length()).arg(averageError());
}

// Setting Parameters
void GrowingNeuralGas::setDelay(int milliseconds) {
  m_delay = milliseconds;
}
void GrowingNeuralGas::setUpdateInterval(int steps) {
  m_updateInterval = steps;
}

void GrowingNeuralGas::setWinnerLearnRate(qreal learnRate) {
  m_winnerLearnRate = learnRate;
}
void GrowingNeuralGas::setNeighborLearnRate(qreal learnRate) {
  m_neighborLearnRate = learnRate;
}

void GrowingNeuralGas::setMaxEdgeAge(int steps) {
  m_maxEdgeAge = steps;
}
void GrowingNeuralGas::setNodeInsertionDelay(int minStepsBetweenInsertions) {
  m_minStepsBetweenInsertions = minStepsBetweenInsertions;
}
void GrowingNeuralGas::setTargetError(qreal targetAverageError) {
  m_targetError = targetAverageError;
}

void GrowingNeuralGas::setErrorReduction(qreal reduceErrorBy) {
  m_reduceErrorMultiplier = 1-reduceErrorBy;
}
void GrowingNeuralGas::setInsertErrorReduction(qreal reduceErrorBy) {
  m_insertErrorMultiplier = 1-reduceErrorBy;
}


// sort function used by qSort in gng.cpp 
typedef QPair<qreal, Node*> DistNodePair;
bool pairLessThan(const DistNodePair &s1, const DistNodePair &s2)
{
    return s1.first < s2.first;
}
   
// TODO: no idea what this does
QPair< Node*, Node* > GrowingNeuralGas::computeDistances(const Point& point)
{
  QList<DistNodePair> dists;
  
  foreach(Node *node, m_nodes) {
    dists.append(DistNodePair(node->location().distanceTo(point), node));
  }
  qSort(dists.begin(), dists.end(), pairLessThan);
  return QPair<Node*, Node*>(dists[0].second, dists[1].second);
}

// increments all edges of a node
void GrowingNeuralGas::incrementEdgeAges(Node* node)
{
  foreach(Edge* edge, node->edges()) {
    edge->incrementAge();
    edge->to()->getEdgeTo(node)->incrementAge();
  }
}

/*****************************
 * Function: connectNodes
 * ----------------------
 * Adds edges between two nodes. Since bidirectional edges in graph are
 * represented as two directed edges, we have to connect a-->b and b-->a
 */
void GrowingNeuralGas::connectNodes(Node* a, Node* b)
{
  Edge* e1 = new Edge(a, b);
  Edge* e2 = new Edge(b, a);
  
  a->appendEdge(e1);
  b->appendEdge(e2);
  
  m_uniqueEdges.append(e1);
}

/*****************************
 * Function: disconnectNodes
 * -------------------------
 * Removes edges between two nodes. Since bidirectional edges in graph are
 * represented as two directed edges, we have to remove a-->b and b-->a
 */
void GrowingNeuralGas::disconnectNodes(Node* a, Node* b)
{
  Edge *e1 = a->getEdgeTo(b);
  Edge *e2 = b->getEdgeTo(a);
  
  a->removeEdge(e1);
  b->removeEdge(e2);
  
  m_uniqueEdges.removeAll(e1);
  m_uniqueEdges.removeAll(e2);
}

/*****************************
 * Function: removeStaleEdges
 * --------------------------
 * Removes all edges that are older than m_maxAge. Any nodes without any
 * connecting edges are culled as well
 */
void GrowingNeuralGas::removeStaleEdges()
{
  // remove edges
  foreach(Node *node, m_nodes) {
    foreach(Edge *edge, node->edges()) {
      if (edge->age() > m_maxEdgeAge) {
        m_uniqueEdges.removeAll(edge);
        node->removeEdge(edge);
        delete edge;
      }
    }
  }

  // remove nodes
  for (int i=m_nodes.length()-1; i>=0; i--) {
    Node *node = m_nodes[i];
    if (node->edges().isEmpty()) {
      m_nodes.removeAll(node);
      delete node;
    }
  }
}

// get node with the highest error
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

// get the average error over all nodes
qreal GrowingNeuralGas::averageError()
{
  qreal error = 0;
  foreach(Node *node, m_nodes) {
    error += node->error();
  }
  return error/m_nodes.length();
}

// returns a point object located at the midpoint between two points
Point midpoint(const Point &p1, const Point &p2) {
  Point p3;
  p3.resize(p1.size());
  
  for (int i=0; i<p1.size(); i++) {
    p3[i] = (p1.at(i) + p2.at(i))/2;
  }
  return p3;
}

// TODO: describe this shit
void GrowingNeuralGas::insertNode()
{
  Node *worst = maxErrorNode(m_nodes);
  Node *worstNeighbor = maxErrorNode(worst->neighbors());
  
  Point newPoint = midpoint(worst->location(), worstNeighbor->location());
  Node *newNode = new Node(newPoint, m_dimension, m_min, m_max);
  m_nodes.append(newNode);

  connectNodes(newNode, worst);
  connectNodes(newNode, worstNeighbor);

  disconnectNodes(worst, worstNeighbor);
  
  worst->setError(worst->error() * m_insertErrorMultiplier);
  worstNeighbor->setError(worstNeighbor->error() * m_insertErrorMultiplier);
  newNode->setError(worst->error());
}

// reduces error for every node. TODO: why do we want this?
void GrowingNeuralGas::reduceAllErrors()
{
  foreach(Node *node, m_nodes) {
    node->setError(node->error() * m_reduceErrorMultiplier);
  }
}

// umm... steps TODO: wtf?
void GrowingNeuralGas::step(const Point& trainingPoint)
{
  if (m_stepCount % 10000 == 0) {
    qDebug() << "Step " << m_stepCount;
  }
  QPair<Node*, Node*> winners = computeDistances(trainingPoint);
  incrementEdgeAges(winners.first);
  
  // if the point is already the right color, don't touch it by moving its xy position all over the place.
  if (0.01 > winners.first->location().colorDistanceTo(trainingPoint)) {
    //qDebug() << winners.first->location() << trainingPoint;
    winners.first->setError(winners.first->error() + 0.1*pow(winners.first->location().distanceTo(trainingPoint), 2));
    winners.first->moveTowards(trainingPoint, 0.1*m_winnerLearnRate);
  } else {
    winners.first->setError(winners.first->error() + pow(winners.first->location().distanceTo(trainingPoint), 2));
    winners.first->moveTowards(trainingPoint, m_winnerLearnRate);
  }
  
  foreach(Node *node, winners.first->neighbors()) {
    node->moveTowards(trainingPoint, m_neighborLearnRate);
  }
  
  if (winners.first->hasEdgeTo(winners.second)) {
    winners.first->getEdgeTo(winners.second)->resetAge();
    winners.second->getEdgeTo(winners.first)->resetAge();
  } else {
    connectNodes(winners.first, winners.second);
  }
  
    
//   qreal dist = winners.first->location().distanceTo(winners.second->location());
//   qreal xyDist = winners.first->location().xyDistanceTo(winners.second->location());
//   if ((averageError() < m_targetError) && (nodes().size() > 4) && (xyDist > 0.2*sqrt(2)*m_max)) {
//     Point p1 = winners.first->location();
//     Point p2 = winners.second->location();
//     qDebug() << "xydist:" << xyDist << "(" << p1[0] << p1[1] << ") (" << p2[0] << p2[1] << ")";
//     
//     Edge *e1 = winners.first->getEdgeTo(winners.second);
//     Edge *e2 = winners.second->getEdgeTo(winners.first);
//     
//     if (e1) { e1->setAge(100); }
//     if (e2) { e2->setAge(100); }
//   }
  
  removeStaleEdges();
  
  if (averageError() > m_targetError && (m_stepsSinceLastInsert > m_minStepsBetweenInsertions)) {
    qDebug() << "Creating new Node at timestep " << m_stepCount << " and error " << averageError();
    m_stepsSinceLastInsert = 0;
    insertNode();
  }
  
  // TODO Make less hacky
  //qDebug() << "Errors: First:" << winners.first->error() << " and Second:" << winners.second->error();
//   qreal errorThreshold = 10*m_targetError;
//   if (m_pickCloseToCountdown > 0) {
//     m_pickCloseToCountdown--;
//   } else if ((averageError() < m_targetError) && (winners.second->error() > errorThreshold)) {
//     //qDebug() << "Picking close to " << trainingPoint;
//     m_pickCloseTo = trainingPoint;
//     m_pickCloseToCountdown = 500;
//   }
  
  reduceAllErrors();
  m_stepCount++;
  m_stepsSinceLastInsert++;
  
}

// single threaded run TODO more
void GrowingNeuralGas::synchronousRun(int cycles)
{
  currentCycles = cycles;
  run();
}

// TODO not really sure what's going on here
void GrowingNeuralGas::run(int cycles)
{
  currentCycles = cycles;
  start(QThread::LowestPriority);
}

// multithreaded run TODO more
void GrowingNeuralGas::run()
{
  Q_ASSERT(currentCycles > 0);
  Q_ASSERT(m_pointGenerator->dimension() == m_dimension);
  
  if (m_stepCount == 0) {
    qDebug() << "Running the GNG for" << currentCycles << "cycles";
  } else {
    qDebug() << "Running the GNG for" << currentCycles << "additional cycles";
  }
  
  for (int i=0; i<currentCycles; i++) {
    if (m_delay != 0) {
      usleep(m_delay*10);
    }
    m_dataAccess->lock();
    m_stepCount++;
    if (m_stepCount % m_updateInterval == 0) {
      emit updated();
    }
    
    Point nextPoint;
//     if (false && m_pickCloseToCountdown > 0) {
//       nextPoint = m_pointGenerator->generateNearbyPoint(m_pickCloseTo);
//     } else {
      nextPoint = m_pointGenerator->generatePoint();
//     }
    step(nextPoint);
    
    m_dataAccess->unlock();
  }
}

/*****************************
 * Function: subgraphs
 * -------------------
 * After running the GNG, we can calculate which subgraphs are disjoint using
 * this method. It returns a list of such subgraphs. Each subgraph is a list
 * of points.
 */
QList<Subgraph> GrowingNeuralGas::subgraphs()
{
  QHash<Node*, bool> nodeDict;
  foreach(Node* node, m_nodes){
    nodeDict.insert(node, true);
  }

  QList<Subgraph> subgraphList;
  while (!nodeDict.empty()){
    Subgraph subgraph;
    Subgraph searchList;

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

/*****************************
 * Function: matchingSubgraph
 * --------------------------
 * When we are tracking objects, we want to know which of the possible
 * subgraphs is closest to the one we've chosen to track. Thus we compare the
 * exemplar against the current list of subgraphs (generated by subgraphs())
 * and whichever subgraph shares the most nodes (as defined by pointer mem
 * addr) is designated the closest match
 *
 * Inputs: 
 *    exemplar: the subgraph we're trying to match against
 *    subgraphs: the possible subgraphs that can be matched
 *
 * Returns:
 *    the subgraph that most closely matches the exemplar
 */
Subgraph GrowingNeuralGas::matchingSubgraph(Subgraph exemplar, QList<Subgraph> subgraphs){
  int count, current_max = 0;
  Subgraph best_subgraph; // best match to exemplar

  foreach(Subgraph cur_subgraph, subgraphs){
    count = 0;
    foreach(Node* exemplar_node, exemplar){
      foreach(Node* check_node, cur_subgraph){
        if (exemplar_node == check_node){
          count++;
          break;
        }
      }
    }
    // check against current best match and update counts as necessary
    if (count > current_max){
      best_subgraph = cur_subgraph;
      current_max = count;
    }
  }
  return best_subgraph;
}

// prints subgraphs in a human-readable format
// should be called using result from getSubgraphs
void GrowingNeuralGas::printSubgraphs(
    QList<Subgraph> &subgraphs, bool printNodes)
{
  qDebug() << "Printing Subgraphs";
  qDebug() << "------------------";

  int nodesInSubgraph;
  int numSubgraphs = subgraphs.size();
  for (int i=0; i<numSubgraphs; i++){
    nodesInSubgraph = subgraphs[i].size();
    qDebug() << "==> Subgraph" << i+1 << "of" << numSubgraphs << 
      "contains" << nodesInSubgraph << "nodes";
    
    // only print node details out if user asks for it
    if (printNodes){
      foreach(Node* node, subgraphs[i]){
        qDebug() << " " << node->toString();
      }
    }
  }
}

// accessors
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

int GrowingNeuralGas::step() const
{
  return m_stepCount;
}

bool GrowingNeuralGas::focusing() const
{
  return m_pickCloseToCountdown != 0;
}
Point GrowingNeuralGas::focusPoint() const
{
  return m_pickCloseTo;
}


// mutator
void GrowingNeuralGas::setPointGenerator(PointSource* pointGenerator)
{
  m_pointGenerator = pointGenerator;
}


