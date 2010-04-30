#ifndef _AIBOREMOTE_H
#define _AIBOREMOTE_H

#include <QObject>
#include <libaibo/aibo.h>


class AiboRemote : public QObject {
  Q_OBJECT
  public:
    AiboRemote(Aibo *aibo) { m_aibo = aibo; }
    ~AiboRemote() {}
    
  public slots:
    void headLeft() { m_aibo->setPan(1); }
    void headRight() { m_aibo->setPan(-1); }
    void headUp() { m_aibo->setTilt(0); }
    void headDown() { m_aibo->setTilt(-1); }
    void headCenter() { m_aibo->setTilt(0); m_aibo->setPan(0); }
    
    void stop() { m_aibo->setMovement(0, 0); m_aibo->setStrafing(0); }
    
    void moveForward() { m_aibo->setTranslation(0.5); }
    void moveBackward() { m_aibo->setTranslation(-0.5); }
    void moveLeft() { m_aibo->setStrafing(0.5); }
    void moveRight() { m_aibo->setStrafing(-0.5); }
    void turnLeft() { m_aibo->setRotation(0.5); }
    void turnRight() { m_aibo->setRotation(-0.5); }
      
  private:
    Aibo *m_aibo;    
};

#endif //_AIBOREMOTE_H
