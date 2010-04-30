
#ifndef _NODE_H
#define _NODE_H

#include <QList>
#include <QString>
#include <QPointF>

#include "point.h"

class Edge;

/** 
    Each unit in the GNG maintains a reference vector, an error
    measure, and a list of edges.
 */
class GngNode {
  
  public:
    GngNode(Point location = Point(), int dimension=2, qreal min=0, qreal max=1);
    ~GngNode();
    
    Point location();
    
    QString toString() const; 
    
    bool hasEdgeTo(const GngNode *other) const;
    Edge* getEdgeTo(const GngNode *other) const;
    void appendEdge(Edge *edge);
    void removeEdge(Edge *edge);
    
    QList<GngNode*> neighbors() const;
    QList<Edge*> edges() const;
    
    qreal error() const;
    void setError(qreal error);
    
    void moveTowards(const Point &point, qreal learningRate);

  private:
    int m_dimension;
    qreal m_min;
    qreal m_max;
    qreal m_error;
    Point m_location;
    QList<Edge*> m_edges;
};

#endif // _NODE_H
