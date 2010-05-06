#ifndef GNG_AIBOSOURCE_H
#define GNG_AIBOSOURCE_H

#include <QThread>

#include <libaibo/aibo.h>

#include "pointsource.h"

namespace GNG {

  class AiboSource : public Aibo, public PointSource {
    Q_OBJECT
    public:
      AiboSource(const QString &hostname, QObject* parent = 0);
      virtual ~AiboSource();
      
      virtual int dimension();
      virtual Point generatePoint();
  };
  
}

#endif //GNG_AIBOSOURCE_H