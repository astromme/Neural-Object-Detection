
#ifndef _GROWINGNEURALGAS_H
#define _GROWINGNEURALGAS_H


#include "point.h"
#include "subgraph.h"

#include <QPair>
#include <QList>
#include <QHash>
#include <QString>
#include <QObject>
#include <QDateTime>
#include <QColor>
#include <QTimer>

namespace GNG {
  class Node;
  class Edge;
  class PointSource;

  typedef QPair<GNG::Node*, GNG::Node*> NodePair;

  class GrowingNeuralGas : public QObject {
    Q_OBJECT
    Q_PROPERTY(int delay READ delay WRITE setDelay);
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval);
    Q_PROPERTY(qreal winnerLearnRate READ winnerLearnRate WRITE setWinnerLearnRate);
    Q_PROPERTY(qreal neighborLearnRate READ neighborLearnRate WRITE setNeighborLearnRate);
    Q_PROPERTY(int maxEdgeAge READ maxEdgeAge WRITE setMaxEdgeAge);
    Q_PROPERTY(int maxEdgeColorDiff READ maxEdgeColorDiff WRITE setMaxEdgeColorDiff);
    Q_PROPERTY(int targetError READ targetError WRITE setTargetError);
    Q_PROPERTY(qreal errorReduction READ errorReduction WRITE setErrorReduction);
    Q_PROPERTY(qreal insertErrorReduction READ insertErrorReduction WRITE setInsertErrorReduction);
    
    public:
      GrowingNeuralGas(int dimension, qreal minimum = 0, qreal maximum = 1);
      ~GrowingNeuralGas();
      
      QString toString();
      int elapsedTime() const;
      
      void setPointGenerator(PointSource *pointGenerator);

      QList<Subgraph> subgraphs() const;
      void generateSubgraphs();
      void matchingSubgraph();
      void assignFollowSubgraph(QColor targetColor);
      void printSubgraphs(bool printNodes=false) const;
      Subgraph followSubgraph() const;
      
      int edgeHistoryAge(Edge *edge) const;
      
      QList<GNG::Node*> nodes() const;
      QList<Edge*> uniqueEdges() const;
      
      int currentStep() const; /**< Returns the current step of the computation. Reset when run() or runSynchronous() is called */
      
      void stopAt(int step);

      Point focusPoint() const;
      bool focusing() const;
      
      
      
      int delay() const;
      int updateInterval() const;
      
      qreal winnerLearnRate() const;
      qreal neighborLearnRate() const;
      
      int maxEdgeAge() const;
      qreal maxEdgeColorDiff() const;
      int nodeInsertionDelay() const;
      qreal targetError() const;
      
      qreal errorReduction() const;
      qreal insertErrorReduction() const;

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
      
      
    signals:
      void updated();    
      
    public slots:
      void start();
      void stop();
      void togglePause();
      
    private slots:
      void runManySteps(int steps=1);
      void runSingleStep();
      
    private:
      PointSource *m_pointGenerator;
      
      /** Computes the distances between the given point and every unit
          in the GNG.  Returns the closest and next closest units. */
      QPair<GNG::Node*, GNG::Node*> computeDistances(const Point& point); // find 2 best nodes
      
      /** Increments the ages of every unit directly connected to the given unit. */
      void incrementEdgeAges(GNG::Node *node);
      
      /** Adds the appropriate edges to connect units a and b. */
      void connectNodes(GNG::Node *a, GNG::Node *b);
      
      /** Removes the appropriate edges to disconnect units a and b. */
      void disconnectNodes(GNG::Node *a, GNG::Node *b);
      
      /** Checks all edges in the GNG and removes any with an age exceeding
          the maxAge parameter.  Also removes any unit that is completely
          disconnected. */
      void removeOldEdges();
      
      /** Given a list of units, returns the unit with the highest error. */
      GNG::Node* maxErrorNode(QList<GNG::Node*> nodeList);
      
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
      
      int m_currentStep;
      int m_stopAtStep;
      bool m_running;
      QTimer m_idleTimer;
      
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
      
      QList<GNG::Node*> m_nodes;
      QList<Edge*> m_uniqueEdges;
      
      QHash<NodePair, int> m_edgeHistory;
      
      QTime m_currentRuntime;
      int m_pastRuntime;

      QList<Subgraph> m_subgraphs;
      Subgraph m_followSubgraph;
  };
      
}

#endif //_GROWINGNEURALGAS_H
