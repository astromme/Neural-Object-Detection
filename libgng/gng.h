
#ifndef _GROWINGNEURALGAS_H
#define _GROWINGNEURALGAS_H

class Node;
class Edge;
class PointGenerator;

#include "point.h"

#include <QPair>
#include <QList>
#include <QString>
#include <QThread>
#include <QMutex>

typedef QList<Node*> NodeList;

class GrowingNeuralGas : public QThread {
  Q_OBJECT
 
  public:
    GrowingNeuralGas(int dimension, qreal minimum = 0, qreal maximum = 1);
    ~GrowingNeuralGas();
    
    QString toString();
    
    void setPointGenerator(PointGenerator *pointGenerator);
    void run(int cycles);
    void synchronousRun(int cycles);

    QList<NodeList> subgraphs();
    void printSubgraphs(QList<NodeList> &subgraphs, bool printNodes=false);
    
    QList<Node*> nodes() const;
    QList<Edge*> uniqueEdges() const;
    
    int step() const; /**< Returns the current step of the computation. Reset when run() or runSynchronous() is called */
    
    void setUpdateInterval(int steps); /**< Emit signal updated() once per this number of steps */
    
    void setWinnerLearnRate(qreal learnRate); /**< Used to adjust closest unit towards input point */
    void setNeighborLearnRate(qreal learnRate); /**< Used to adjust other neighbors towards input point */
    
    void setMaxEdgeAge(int steps); /**< Edges older than maxAge are removed */
    void setNodeInsertionDelay(int minStepsBetweenInsertions); /**< Min steps before inserting a new node */
    void setTargetError(qreal targetAverageError); /**< Continue inserting nodes until the average error has reached this threshold */
    
    void setErrorReduction(qreal reduceErrorBy); /**< All errors are reduced by this amount each GNG step */
    void setInsertErrorReduction(qreal reduceErrorBy); /**< Reduce new unit's error by this much */
    
    QMutex* mutex() const;
    
  signals:
    void updated();    
    
  private:
    virtual void run();
    int currentCycles;
    PointGenerator *m_pointGenerator;
    QMutex *m_dataAccess;
    
    /** Computes the distances between the given point and every unit
        in the GNG.  Returns the closest and next closest units. */
    QPair<Node*, Node*> computeDistances(const Point& point); // find 2 best nodes
    
    /** Increments the ages of every unit directly connected to the given unit. */
    void incrementEdgeAges(Node *node);
    
    /** Adds the appropriate edges to connect units a and b. */
    void connectNodes(Node *a, Node *b);
    
    /** Removes the appropriate edges to disconnect units a and b. */
    void disconnectNodes(Node *a, Node *b);
    
    /** Checks all edges in the GNG and removes any with an age exceeding
        the maxAge parameter.  Also removes any unit that is completely
        disconnected. */
    void removeStaleEdges();
    
    /** Given a list of units, returns the unit with the highest error. */
    Node* maxErrorNode(QList<Node*> nodeList);
    
    /** Returns the average error across all units in the GNG. */
    qreal averageError();
    
    /** Inserts a new unit into the GNG.  Finds the unit with the highest
        error and then finds its topological neighbor with the highest
        error and inserts the new unit between the two. */
    void insertNode();
    
    /** Decays the error at all units. */
    void reduceAllErrors();
    
    /** Processes one input point at a time through the GNG. */
    void step(const Point& trainingPoint);
    
  private:
    int m_dimension;
    int m_min;
    int m_max;
    int m_stepCount;
    
    int m_updateInterval;
    
    qreal m_winnerLearnRate;
    qreal m_neighborLearnRate;
    
    int m_maxEdgeAge;
    int m_minStepsBetweenInsertions;
    int m_stepsSinceLastInsert;
    
    qreal m_reduceErrorMultiplier;
    qreal m_insertErrorMultiplier;
    qreal m_targetError;
    
    QList<Node*> m_nodes;
    QList<Edge*> m_uniqueEdges;
    
    
};
      




#endif //_GROWINGNEURALGAS_H
