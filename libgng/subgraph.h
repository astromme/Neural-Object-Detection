#ifndef _SUBGRAPH_H
#define _SUBGRAPH_H

#include <QList>

#include "point.h"

class GngNode;

class Subgraph : public QList<GngNode*> {
  public:
    Point center();
        
  private:
    int m_window; // Steps that this is valid for
};

#endif // _SUBGRAPH_H