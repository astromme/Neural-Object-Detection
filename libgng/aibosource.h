#ifndef GNG_AIBOSOURCE_H
#define GNG_AIBOSOURCE_H

#include <QThread>

#include <libaibo/aibo.h>

#include "pointsource.h"

class QMutex;

class AiboSource : public Aibo, public PointSource {
  Q_OBJECT
  public:
    AiboSource(const QString &hostname, QObject* parent = 0);
    virtual ~AiboSource();
    
    virtual int dimension();
    virtual Point generatePoint();

  private:
    QMutex* m_dataAccess;
    QImage m_tempImage;
    
  private slots:
    void setFrame(QImage image);
};

#endif //GNG_AIBOSOURCE_H