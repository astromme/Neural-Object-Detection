
#ifndef _NODE_H
#define _NODE_H

#include <QList>
#include <QString>
#include <QPointF>
#include <QVector>

class Edge;

/** 
    Each unit in the GNG maintains a reference vector, an error
    measure, and a list of edges.
 */
class Node {
  
  public:
    Node(QVector<qreal> vector = QVector<qreal>(), int dimension=2, int min=-1, int max=1);
    ~Node();
    
    QString toString() const;
    
    bool hasEdgeTo(const Node *other) const;
    Edge* getEdgeTo(const Node *other) const;
    
    QList<Node*> neighbors() const;
    
    void moveTowards(const QVector<qreal> &point, int learningRate);

  private:
    int m_dimension;
    qreal m_min;
    qreal m_max;
    qreal m_error;
    QVector<qreal> m_vector;
    QList<Edge*> m_edges;
};

#endif // _NODE_H
