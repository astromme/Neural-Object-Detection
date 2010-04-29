#ifndef AIBO_AIBO_H
#define AIBO_AIBO_H

#include <QObject>
#include <QImage>
#include <QtNetwork/QAbstractSocket>

class QTcpSocket;
class QMutex;

class Aibo : public QObject {
  Q_OBJECT
  Q_PROPERTY(qreal tilt READ tilt WRITE setTilt)
  Q_PROPERTY(qreal pan READ pan WRITE setPan)
  Q_PROPERTY(qreal roll READ roll WRITE setRoll)
  //Q_PROPERTY(qreal translation READ translation WRITE setTranslation)
  //Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)
  //Q_PROPERTY(qreal strafing READ strafing WRITE setStrafing)
  public:
    enum Port {
      // Numbers are port numbers
      HeadRemoteControl = 10052,
      WalkRemoteControl = 10050,
      EStopRemoteControl = 10053,
      WorldStateSerializer = 10031,
      RawCamServer = 10011,
      SegCamServer = 10012,
      MainControl = 10020,
      JointWriter = 10051
    };
    
    enum Control {
      HeadTilt,
      HeadPan,
      HeadRoll,
      BodyTranslate,
      BodyRotate,
      BodyStrafe
    };
    
    enum Joint {
      //TODO Enumerate joints
    };

    Aibo(const QString &hostname, QObject *parent = 0);
    ~Aibo();
    
    void startCamera();
    void stopCamera();
    bool isCameraRunning() const;
    QImage cameraImage() const;
    
    void startHeadControl();
    void stopHeadControl();
    bool isHeadControlRunning() const;
    
    void startWalkControl();
    void stopWalkControl();
    bool isWalkControlRunning() const;
    
    // Head Controls. Requries the head control to be running
    /** 0 to -1 (straight ahead to down) */
    void setTilt(qreal tilt);
    /** -1 to 1 (right to left) */
    void setPan(qreal pan);
    /** 0 to 1 (straight ahead, to up (stretched)) */
    void setRoll(qreal roll);
    
    void setHeadOrientation(qreal tilt, qreal pan, qreal roll);
    
    qreal tilt() const;
    qreal pan() const;
    qreal roll() const;
    
    // Body Controls. Requires Walk Remote Control
    void setTranslation(qreal velocity);
    void setRotation(qreal velocity);
    void setStrafing(qreal velocity);
    
    void setMovement(qreal translationVelocity, qreal rotationVelocity);
    
    // Joint Controls
    void setJoint(Joint joint, qreal x, qreal y, qreal z);
    
  signals:
    void cameraFrame(QImage frame);
    
  private slots:
    void mainSocketReadyRead();
    void mainSocketError(QAbstractSocket::SocketError error);
    
    void cameraConnect();
    void cameraSocketReadyRead();
    void cameraSocketError(QAbstractSocket::SocketError error);
    
    void headConnect();
    void headSocketReadyRead();
    void headSocketError(QAbstractSocket::SocketError error);
    
  protected:
    QMutex *m_dataAccess;
    
  private:
    char* readUntil(QTcpSocket* socket, char stop);
    void sendCommand(const QString &command, QTcpSocket *socket=0);
    void sendControl(Control control, qreal amount);
    void set(const QString &property, const QString &value);
  
    QString m_hostname;
    QTcpSocket *m_mainSocket;
    QTcpSocket *m_cameraSocket;
    QTcpSocket *m_headSocket;
    QTcpSocket *m_walkSocket;
    
    bool m_cameraRunning;
    bool m_headControlRunning;
    bool m_walkControlRunning;
    
    QImage m_currentFrame;

};

// Python Definitions
//     def setRemoteControl(self, item, state):
//     def readWorldState(self, socket):
//     def getJoint(self, query):
//     def getButton(self, query):
//     def getSensor(self, query):
//     def startDeviceBuiltin(self, item):
//     def connect(self):
//     def disconnect(self):
//     def rotate(self, amount):
//     def translate(self, amount):
//     def strafe(self, amount):
//     def move(self, translate, rotate):
//     def destroy(self):
//     def playSound(self, file):
//     def runMotion(self, file):
//     def setWalk(self, file):
//     def setPose(self, joint, amtx, amty=None, amtz=None):
//     def getPose(self, joint):


#endif //AIBO_AIBO_H