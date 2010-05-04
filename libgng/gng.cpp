#include "gng.h"

#include "node.h"
#include "edge.h"
#include "point.h"
#include "pointsource.h"

#include <math.h>

#include <QDebug>
#include <QHash>

// constructor
GrowingNeuralGas::GrowingNeuralGas(int dimension, qreal minimum, qreal maximum)
  : currentCycles(0),
    m_pointGenerator(0),
    m_pickCloseToCountdown(0)
{
  m_dataAccess = new QMutex(QMutex::Recursive);
  
  // Hardcoded values from paper
  m_dimension = dimension;
  setWinnerLearnRate(0.1);
  setNeighborLearnRate(0.01); // TODO was .01
  setMaxEdgeAge(50);
  setMaxEdgeColorDiff(0.1);
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
  
  m_paused = false;
  
  //The GNG always begins with two randomly placed units.
  m_nodes.append(new GngNode(Point(), dimension, minimum, maximum));
  m_nodes.append(new GngNode(Point(), dimension, minimum, maximum));

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

int GrowingNeuralGas::elapsedTime() const
{
  return m_timer.elapsed();
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

void GrowingNeuralGas::setMaxEdgeColorDiff(qreal diff) {
  m_maxEdgeColorDiff = diff;
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
typedef QPair<qreal, GngNode*> DistNodePair;
bool pairLessThan(const DistNodePair &s1, const DistNodePair &s2)
{
    return s1.first < s2.first;
}
   
// see header
QPair< GngNode*, GngNode* > GrowingNeuralGas::computeDistances(const Point& point)
{
  QList<DistNodePair> dists;
  
  foreach(GngNode *node, m_nodes) {
    dists.append(DistNodePair(node->location().distanceTo(point), node));
  }
  qSort(dists.begin(), dists.end(), pairLessThan);

  return QPair<GngNode*, GngNode*>(dists[0].second, dists[1].second);
}

// increments all edges of a node
void GrowingNeuralGas::incrementEdgeAges(GngNode* node)
{
  int currentTime = m_timer.elapsed();
  foreach(Edge* edge, node->edges()) {
    edge->setLastUpdated(currentTime);
    edge->incrementAge();
    edge->to()->getEdgeTo(node)->incrementAge();
  }
}

// Updates the history hash for each edge and removes old edges
void GrowingNeuralGas::incrementEdgeHistory()
{
  int currentTime = m_timer.elapsed();

  bool deletedAnEdge = false; // delete at most one edge per function call

  m_dataAccess->lock();
  foreach(GngNode* node, m_nodes) {
    foreach(Edge* edge, node->edges()) {
      if (!deletedAnEdge && (currentTime - edge->lastUpdated()) > 5000) { // if it's been more than 5 seconds
        qDebug() << "removing edge that has been here for" << (currentTime - edge->lastUpdated()) << "ms" << edge->id();
        GngNode *n1 = edge->to();
        GngNode *n2 = edge->from();
        
        Edge *e1 = n1->getEdgeTo(n2);
        Edge *e2 = n2->getEdgeTo(n1);
        n1->removeEdge(e1);
        n2->removeEdge(e2);
        
        m_uniqueEdges.removeAll(e1);
        m_uniqueEdges.removeAll(e2);
        delete edge;
        edge = 0;

        deletedAnEdge = true;
      } else {
        edge->incrementTotalAge();
        if (edge->from() < edge->to()) {
          NodePair nodes(edge->from(), edge->to());
          int age = m_edgeHistory.value(nodes, 0);
          m_edgeHistory.insert(nodes, age+1);
        }
      }
    }
  }
  m_dataAccess->unlock();
}


/*****************************
 * Function: connectNodes
 * ----------------------
 * Adds edges between two nodes. Since bidirectional edges in graph are
 * represented as two directed edges, we have to connect a-->b and b-->a
 */
void GrowingNeuralGas::connectNodes(GngNode* a, GngNode* b)
{
  Edge* e1 = new Edge(a, b);
  Edge* e2 = new Edge(b, a);
  
  a->appendEdge(e1);
  b->appendEdge(e2);
  
  int currentTime = m_timer.elapsed();
  e1->setLastUpdated(currentTime);
  e2->setLastUpdated(currentTime);
  
  m_uniqueEdges.append(e1);
}

/*****************************
 * Function: disconnectNodes
 * -------------------------
 * Removes edges between two nodes. Since bidirectional edges in graph are
 * represented as two directed edges, we have to remove a-->b and b-->a
 */
void GrowingNeuralGas::disconnectNodes(GngNode* a, GngNode* b)
{
  Edge *e1 = a->getEdgeTo(b);
  Edge *e2 = b->getEdgeTo(a);
  
  a->removeEdge(e1);
  b->removeEdge(e2);
  
  m_uniqueEdges.removeAll(e1);
  m_uniqueEdges.removeAll(e2);
}

/*****************************
 * Function: removeOldEdges
 * --------------------------
 * Removes all edges that are older than m_maxAge. Any nodes without any
 * connecting edges are culled as well
 */
void GrowingNeuralGas::removeOldEdges()
{
  // remove edges
  foreach(GngNode *node, m_nodes) {
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
    GngNode *node = m_nodes[i];
    if (node->edges().isEmpty()) {
      m_nodes.removeAll(node);
      delete node;
    }
  }
}

// get node with the highest error
GngNode* GrowingNeuralGas::maxErrorNode(QList< GngNode* > nodeList)
{
  GngNode* highestError = nodeList.first();
  foreach(GngNode *node, nodeList) {
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
  foreach(GngNode *node, m_nodes) {
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

// see header
void GrowingNeuralGas::insertNode()
{
  GngNode *worst = maxErrorNode(m_nodes);
  GngNode *worstNeighbor = maxErrorNode(worst->neighbors());
  
  Point newPoint = midpoint(worst->location(), worstNeighbor->location());
  GngNode *newNode = new GngNode(newPoint, m_dimension, m_min, m_max);
  m_nodes.append(newNode);

  connectNodes(newNode, worst);
  connectNodes(newNode, worstNeighbor);

  disconnectNodes(worst, worstNeighbor);
  
  worst->setError(worst->error() * m_insertErrorMultiplier);
  worstNeighbor->setError(worstNeighbor->error() * m_insertErrorMultiplier);
  newNode->setError(worst->error());
}

// reduces error for every node. This happens every timestep
// to bias the node insertion towards recently updated points
void GrowingNeuralGas::reduceAllErrors()
{
  foreach(GngNode *node, m_nodes) {
    node->setError(node->error() * m_reduceErrorMultiplier);
  }
}

// Perform one iteration of the GNG with the given source/traning point
// The gng will find the two closest nodes, move them closer to the training
// point and then possibly add new nodes.
void GrowingNeuralGas::step(const Point& trainingPoint)
{
  if (m_stepCount % 10000 == 0) {
    qDebug() << "Step " << m_stepCount;
  }
  QPair<GngNode*, GngNode*> winners = computeDistances(trainingPoint);
  incrementEdgeAges(winners.first);
  
  // if the point is already the right color, don't touch it by moving its xy position all over the place.
  if (false && 0.01 > winners.first->location().colorDistanceTo(trainingPoint)) {
    //qDebug() << winners.first->location() << trainingPoint;
    winners.first->setError(winners.first->error() + 0.1*pow(winners.first->location().distanceTo(trainingPoint), 2));
    winners.first->moveTowards(trainingPoint, 0.1*m_winnerLearnRate);
  } else {
    winners.first->setError(winners.first->error() + pow(winners.first->location().distanceTo(trainingPoint), 2));
    winners.first->moveTowards(trainingPoint, m_winnerLearnRate);
  }
  
  foreach(GngNode *node, winners.first->neighbors()) {
    node->moveTowards(trainingPoint, m_neighborLearnRate);
  }

  // if difference between hues is too great between two nodes, reset ages
  qreal first_hue = winners.first->location()[2];
  qreal second_hue = winners.second->location()[2];

  if (averageError() > m_targetError || fabs(first_hue - second_hue) < m_maxEdgeColorDiff){
    if (winners.first->hasEdgeTo(winners.second)) {
      winners.first->getEdgeTo(winners.second)->resetAge();
      winners.second->getEdgeTo(winners.first)->resetAge();
    } else {
      connectNodes(winners.first, winners.second);
    }
  }
  
  // if GNG has not developed enough subgraphs, try lowering the target errorThreshold
  if (false && m_stepCount > 20000 && m_stepCount % 5000 == 0 && 
      m_targetError > 0.02){ // && m_subgraphs.size() <= 2){
    setTargetError(m_targetError-0.01);
    qDebug() << "It's about that time again...Lowered error to " << m_targetError;
  }
 
  // TODO remove -- this is super hacky
/*
  if (m_stepCount == 10000){
    generateSubgraphs();
    QColor targetColor = QColor::fromRgbF(1,0,0).toHsl();
    //QColor targetColor = QColor::fromRgbF(0,0,1).toHsl();
    //QColor targetColor = QColor::fromRgbF(0,1,0).toHsl();
    assignFollowSubgraph(targetColor);
  }
  
  if (m_stepCount > 10000 && m_stepCount % 10000 == 0){
    generateSubgraphs();
    matchingSubgraph();
    Point center = m_followSubgraph.center();
  }
*/
 
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
  
  incrementEdgeHistory();
  removeOldEdges();
  
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

// start an asynchronous thread running for 'cycles' number of steps
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
  
  m_timer.start(); // time milliseconds since gng was created
  
  // sleep(5); // sleep for 5 seconds to get the aibo going
  
  if (m_stepCount == 0) {
    qDebug() << "Running the GNG for" << currentCycles << "cycles";
  } else {
    qDebug() << "Running the GNG for" << currentCycles << "additional cycles";
  }
  
  for (int i=0; i<currentCycles; i++) {
    if (m_delay != 0) {
      usleep(m_delay*10);
    }
    if (i % 1000 == 0){
      usleep(1000);
    }
    m_dataAccess->lock();
    
    while (m_paused) {
      m_dataAccess->unlock();
      usleep(10000);
      m_dataAccess->lock();
    }
    
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

void GrowingNeuralGas::pause()
{
  m_dataAccess->lock();
  m_paused = true;
  m_dataAccess->unlock();
}
void GrowingNeuralGas::resume()
{
  m_dataAccess->lock();
  m_paused = false;
  m_dataAccess->unlock();
}

void GrowingNeuralGas::togglePause()
{
  m_dataAccess->lock();
  m_paused = !m_paused;
  m_dataAccess->unlock();
}

QList< Subgraph > GrowingNeuralGas::subgraphs() const
{
  return m_subgraphs;
}

/********************************
 * Function: generateSubgraphs
 * ---------------------------
 * After running the GNG, we can calculate which subgraphs are disjoint using
 * this method. It creates list of such subgraphs and sets m_subgraphs to it.
 * Each subgraph is a list of Nodes.
 */
void GrowingNeuralGas::generateSubgraphs()
{
  // TODO lock entire function?
  QHash<GngNode*, bool> nodeDict;

  m_dataAccess->lock();
  foreach(GngNode* node, m_nodes){
    nodeDict.insert(node, true);
  }
  //m_dataAccess->unlock();

  QList<Subgraph> subgraphList;
  while (!nodeDict.empty()){
    Subgraph subgraph;
    Subgraph searchList;

    // get node in dictionary
    GngNode* initNode = nodeDict.begin().key();

    searchList.append(initNode);
    while (!searchList.empty()){
      // remove current node from search list
      GngNode* searchNode = searchList.takeFirst();

      // add node to subgraph since we can reach it
      subgraph.append(searchNode);

      // by removing node from dictionary we effectively mark it as 'visited'
      nodeDict.remove(searchNode);

      // add all neighbors that can be reached and have not been visited
      QList<GngNode*> neighbors = searchNode->neighbors();
      foreach (GngNode* node, neighbors){
        // Breadth First Search
        // Ignore edges less than 2000 steps old
        if (nodeDict.contains(node) && (searchNode->getEdgeTo(node)->totalAge() > 2000)){ //TODO make configurable
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

  //m_dataAccess->lock();
  m_subgraphs = subgraphList;
  m_dataAccess->unlock();
}

/*****************************
 * Function: matchingSubgraph
 * --------------------------
 *  TODO FIXME
 * When we are tracking objects, we want to know which of the possible
 * subgraphs is closest to the one we've chosen to track. Thus we compare the
 * exemplar against the current list of subgraphs (generated by subgraphs())
 * and whichever subgraph shares the most nodes (as defined by pointer mem
 * addr) is designated the closest match
 *
 * Assumes that m_subgraphs and m_followSubgraph have been initialized
 *
 * Inputs: 
 *    exemplar: the subgraph we're trying to match against
 *    subgraphs: the possible subgraphs that can be matched
 *
 * Returns:
 *    the subgraph that most closely matches the exemplar
 */
void GrowingNeuralGas::matchingSubgraph(){
  int count, current_max = 0;
  QList<Subgraph> subgraphs;
  Subgraph exemplar;
  Subgraph best_subgraph; // best match to exemplar

  m_dataAccess->lock();
  exemplar = m_followSubgraph;
  subgraphs = m_subgraphs;
  m_dataAccess->unlock();

  foreach(Subgraph cur_subgraph, subgraphs){
    count = 0;
    foreach(GngNode* check_node, cur_subgraph){
      if (exemplar.contains(check_node)){
        count++;
      }
    }
    // check against current best match and update counts as necessary
    if (count > current_max){
      best_subgraph = cur_subgraph;
      current_max = count;
    }
  }
 
  // set new subgraph to follow 
  m_dataAccess->lock();
  m_followSubgraph = best_subgraph;
  m_dataAccess->unlock();
}

/**********************************
 * Function: assignFollowSubgraph
 * ------------------------------
 * We take the average HSL values of each subgraph and compare the distance
 * from each average to the given HSL values. The subgraph with the smallest
 * distance gets set as the followSubgraph.
 *
 * Inputs: HSL values (self-explanatory)
 */
void GrowingNeuralGas::assignFollowSubgraph(QColor targetColor)
{
  m_dataAccess->lock();
  qreal hue, saturation, lightness;
  targetColor.getHslF(&hue, &saturation, &lightness);
  
  Point exemplar;
  exemplar.resize(5);
  exemplar[0] = 0; exemplar[1] = 0;
  exemplar[2] = hue; exemplar[3] = saturation; exemplar[4] = lightness;
 
  qreal bestColorDistance = 1000;
  qreal curDist;
  int indexOfBestMatch = 0; // index in m_subgraphs of best HSL match
  qreal avgH, avgR, avgL;
  for (int i=0; i<m_subgraphs.size(); i++){
    foreach (GngNode* node, m_subgraphs[i]){
      curDist = node->location().colorDistanceTo(exemplar);
      if (curDist < bestColorDistance){
        indexOfBestMatch = i;
        bestColorDistance = curDist;
      }
    }
  }
  
  // set followSubgraph to subgraph at best indexOfBestMatch
  m_followSubgraph = m_subgraphs[indexOfBestMatch];
    
  m_dataAccess->unlock();
}

// prints subgraphs in a human-readable format
// should be called using result from getSubgraphs
void GrowingNeuralGas::printSubgraphs(bool printNodes) const
{
  m_dataAccess->lock();
  QList<Subgraph> subgraphs = m_subgraphs;
  m_dataAccess->unlock();

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
      foreach(GngNode* node, subgraphs[i]){
        qDebug() << " " << node->toString();
      }
    }
  }
}

Subgraph GrowingNeuralGas::followSubgraph() const
{
  return m_followSubgraph;
}


// accessors
QMutex* GrowingNeuralGas::mutex() const
{
  return m_dataAccess;
}


QList< GngNode* > GrowingNeuralGas::nodes() const
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

int GrowingNeuralGas::edgeHistoryAge(Edge* edge) const
{
  if (edge->from() < edge->to()) {
    return m_edgeHistory.value(NodePair(edge->from(), edge->to()), 0);
  } else {
    return m_edgeHistory.value(NodePair(edge->to(), edge->from()), 0);
  }
}

// mutator
void GrowingNeuralGas::setPointGenerator(PointSource* pointGenerator)
{
  m_pointGenerator = pointGenerator;
}
