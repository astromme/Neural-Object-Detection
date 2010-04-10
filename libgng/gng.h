
#ifndef _GROWINGNEURALGAS_H
#define _GROWINGNEURALGAS_H

class Node;
class Edge;

#include "point.h"

#include <QPair>
#include <QList>
#include <QString>

class GrowingNeuralGas {
  public:
    GrowingNeuralGas(int dimension, qreal minimum, qreal maximum);
    ~GrowingNeuralGas();
    
    QString toString();

    bool unitOfInterest(Node *node, qreal cutoff);
    QPair<Node*, Node*> computeDistances(Point &point); // find 2 best nodes
    void incrementEdgeAges(Node *node);    
    void connectNodes(Node *a, Node *b);
    void disconnectNodes(Node *a, Node *b);
    void removeStaleEdges();
    Node* maxErrorNode(QList<Node*> nodeList);
    qreal averageError();
    void insertNode();
    void reduceAllErrors();
    void step(const Point &nextPoint);
    void run(int cycles, void distribution); //FIXME            
    void saveCurrentData(const Point &nextPoint);

    
  private:
    int m_dimension;
    //int m_seed;
    qreal m_winnerLearnRate;
    qreal m_neighborLearnRate;
    int m_maxAge;
    qreal m_reduceError;
    int m_stepsToInsert;
    int m_lastInsertedStep;
    qreal m_insertError;
    int m_stepCount;
    QList<Node*> m_nodes;
    QList<Edge*> m_uniqueEdges;
    void modelVectors;
    void edgeVectors;
    void distPonts;
    
  
};
      




#endif //_GROWINGNEURALGAS_H