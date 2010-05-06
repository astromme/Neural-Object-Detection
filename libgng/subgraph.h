#ifndef _SUBGRAPH_H
#define _SUBGRAPH_H

#include <QList>

#include "point.h"

namespace GNG {
  class Node;

  class Subgraph : public QList<GNG::Node*> {
    public:
      Point center();
          
    private:
      int m_window; // Steps that this is valid for
  };
}

#endif // _SUBGRAPH_H