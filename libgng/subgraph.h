#ifndef _SUBGRAPH_H
#define _SUBGRAPH_H

#include <QList>

class Node;

class Subgraph : public QList<Node*> {
  public:
        
  private:
    int m_window; // Steps that this is valid for
};

#endif // _SUBGRAPH_H