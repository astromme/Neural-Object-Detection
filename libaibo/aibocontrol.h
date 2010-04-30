#ifndef AIBOREMOTE_H
#define AIBOREMOTE_H

#include <QObject>
#include <libaibo/aibo.h>


class AiboRemote : public QObject {
  Q_OBJECT
  public:
    AiboRemote(Aibo *aibo);
    ~AiboRemote() {}
    
  public slots:
    void headLeft();
    void headRight();
    void headUp();
    void headDown();
    void headCenter();
    
    void stop();
    
    void moveForward();
    void moveBackward();
    void moveLeft();
    void moveRight();
    void turnLeft();
    void turnRight();
      
  private:
    Aibo *m_aibo;    
};

#endif //AIBOREMOTE_H
