#ifndef AIBOREMOTE_H
#define AIBOREMOTE_H

#include <QObject>
#include <libaibo/aibo.h>


class AiboControl : public QObject {
  Q_OBJECT
  public:
    AiboControl(Aibo *aibo);
    ~AiboControl() {}
    
  public slots:
    void startHeadControl();
    void stopHeadControl();
    void startWalkControl();
    void stopWalkControl();
    
    void moveFocusLeft(qreal velocity=0.5);
    void moveFocusRight(qreal velocity=0.5);
    void moveFocusCenter(qreal velocity=0.5);
    void moveFocusUp(qreal velocity=0.5);
    void moveFocusDown(qreal velocity=0.5);
    void stopMovingFocus();
    
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
