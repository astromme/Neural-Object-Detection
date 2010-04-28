#ifndef AIBO_AIBO_H
#define AIBO_AIBO_H

#include <QObject>
#include <QImage>
#include <QtNetwork/QAbstractSocket>

class QTcpSocket;

class Aibo : public QObject {
  Q_OBJECT
  public:
    enum Control {
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


    Aibo(const QString &hostname, QObject *parent = 0);
    ~Aibo();
    
    void startCamera();
    void stopCamera();
    bool isCameraRunning() const;
    QImage cameraImage() const;
    
    void startHeadControl();
    void stopHeadControl();
    bool isHeadControlRunning() const;
    
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
    
  private:
    // private methods
    char* readUntil(QTcpSocket* socket, char stop);
    void sendCommand(const QString &command);
    void set(const QString &property, const QString &value);
  
    //private variables
    QString m_hostname;
    QTcpSocket *m_mainSocket;
    QTcpSocket *m_cameraSocket;
    QTcpSocket *m_headSocket;
    
    bool m_cameraRunning;
    bool m_headControlRunning;
    
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