
#ifndef _GROWINGNEURALGAS_H
#define _GROWINGNEURALGAS_H

class GngNode;
class Edge;
class PointSource;

#include "point.h"
#include "subgraph.h"

#include <QPair>
#include <QList>
#include <QHash>
#include <QString>
#include <QThread>
#include <QMutex>
#include <QDateTime>
#include <QColor>

typedef QPair<GngNode*, GngNode*> NodePair;

class GrowingNeuralGas : public QThread {
  Q_OBJECT
 
  public:
    GrowingNeuralGas(int dimension, qreal minimum = 0, qreal maximum = 1);
    ~GrowingNeuralGas();
    
    QString toString();
    int elapsedTime() const;
    
    void setPointGenerator(PointSource *pointGenerator);
    void run(int cycles);
    void synchronousRun(int cycles);

    QList<Subgraph> subgraphs() const;
    void generateSubgraphs();
    void matchingSubgraph();
    void assignFollowSubgraph(QColor targetColor);
    void printSubgraphs(bool printNodes=false) const;
    Subgraph followSubgraph() const;
    
    int edgeHistoryAge(Edge *edge) const;
    
    QList<GngNode*> nodes() const;
    QList<Edge*> uniqueEdges() const;
    
    int step() const; /**< Returns the current step of the computation. Reset when run() or runSynchronous() is called */

    Point focusPoint() const;
    bool focusing() const;

    void setDelay(int milliseconds);
    void setUpdateInterval(int steps); /**< Emit signal updated() once per this number of steps */
    
    void setWinnerLearnRate(qreal learnRate); /**< Used to adjust closest unit towards input point */
    void setNeighborLearnRate(qreal learnRate); /**< Used to adjust other neighbors towards input point */
    
    void setMaxEdgeAge(int steps); /**< Edges older than maxAge are removed */
    void setMaxEdgeColorDiff(qreal diff); /**< Edges not created if color difference between nodes is above value */
    void setNodeInsertionDelay(int minStepsBetweenInsertions); /**< Min steps before inserting a new node */
    void setTargetError(qreal targetAverageError); /**< Continue inserting nodes until the average error has reached this threshold */
    
    void setErrorReduction(qreal reduceErrorBy); /**< All errors are reduced by this amount each GNG step */
    void setInsertErrorReduction(qreal reduceErrorBy); /**< Reduce new unit's error by this much */
    
    QMutex* mutex() const;
    
  signals:
    void updated();    
    
  public slots:
    void pause();
    void resume();
    void togglePause();
    
  private:
    virtual void run();
    int currentCycles;
    PointSource *m_pointGenerator;
    QMutex *m_dataAccess;
    
    /** Computes the distances between the given point and every unit
        in the GNG.  Returns the closest and next closest units. */
    QPair<GngNode*, GngNode*> computeDistances(const Point& point); // find 2 best nodes
    
    /** Increments the ages of every unit directly connected to the given unit. */
    void incrementEdgeAges(GngNode *node);
    
    /** Adds the appropriate edges to connect units a and b. */
    void connectNodes(GngNode *a, GngNode *b);
    
    /** Removes the appropriate edges to disconnect units a and b. */
    void disconnectNodes(GngNode *a, GngNode *b);
    
    /** Checks all edges in the GNG and removes any with an age exceeding
        the maxAge parameter.  Also removes any unit that is completely
        disconnected. */
    void removeOldEdges();
    
    /** Given a list of units, returns the unit with the highest error. */
    GngNode* maxErrorNode(QList<GngNode*> nodeList);
    
    /** Returns the average error across all units in the GNG. */
    qreal averageError();
    
    /** Inserts a new unit into the GNG.  Finds the unit with the highest
        error and then finds its topological neighbor with the highest
        error and inserts the new unit between the two. */
    void insertNode();
    
    /** Decays the error at all units. */
    void reduceAllErrors();
    
    void incrementEdgeHistory();
    
    /** Processes one input point at a time through the GNG. */
    void step(const Point& trainingPoint);
    
  private:
    int m_dimension;
    int m_min;
    int m_max;
    int m_stepCount;
    bool m_paused;
    
    int m_delay;
    int m_updateInterval;
    
    qreal m_winnerLearnRate;
    qreal m_neighborLearnRate;
    
    int m_maxEdgeAge;

    int m_minStepsBetweenInsertions;
    int m_stepsSinceLastInsert;
    
    qreal m_reduceErrorMultiplier;
    qreal m_insertErrorMultiplier;
    qreal m_targetError;
    qreal m_maxEdgeColorDiff;
    
    Point m_pickCloseTo;
    int m_pickCloseToCountdown;
    
    QList<GngNode*> m_nodes;
    QList<Edge*> m_uniqueEdges;
    
    QHash<NodePair, int> m_edgeHistory;
    
    QTime m_timer;

    QList<Subgraph> m_subgraphs;
    Subgraph m_followSubgraph;
};
      




#endif //_GROWINGNEURALGAS_H
