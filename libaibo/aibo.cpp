#include "aibo.h"

#include <QtNetwork/QTcpSocket>
#include <QImage>
#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QMutex>

// Convert from little endian to big endian?
long convert(char *buff) {
  long retval = 0;
  retval += (buff[0] & 0xFF) <<  0;
  retval += (buff[1] & 0xFF) <<  8;
  retval += (buff[2] & 0xFF) << 16;
  retval += (buff[3] & 0xFF) << 24;
  return retval;
}

Aibo::Aibo::Aibo(const QString& hostname, QObject* parent)
  : QObject(parent)
{
  m_dataAccess = new QMutex();
  
  m_hostname = hostname;
  m_mainSocket = new QTcpSocket(this);
  m_cameraSocket = new QTcpSocket(this);
  m_headSocket = new QTcpSocket(this);
  m_walkSocket = new QTcpSocket(this);
  
  // Initialize main
  connect(m_mainSocket, SIGNAL(readyRead()), SLOT(mainSocketReadyRead()));
  connect(m_mainSocket, SIGNAL(error(QAbstractSocket::SocketError)),
          SLOT(mainSocketError(QAbstractSocket::SocketError)));
  
  // Initialize camera
  m_cameraRunning = false;
  m_dataAccess->lock();
  m_currentFrame = QImage(416, 320, QImage::Format_RGB32);
  m_currentFrame.fill(0); // fill with black
  m_dataAccess->unlock();
  connect(m_cameraSocket, SIGNAL(readyRead()), SLOT(cameraSocketReadyRead()));
  connect(m_cameraSocket, SIGNAL(error(QAbstractSocket::SocketError)),
          SLOT(cameraSocketError(QAbstractSocket::SocketError)));
          
  // Initialize head control
  m_headControlRunning = false;
  connect(m_headSocket, SIGNAL(readyRead()), SLOT(headSocketReadyRead()));
  connect(m_headSocket, SIGNAL(error(QAbstractSocket::SocketError)),
          SLOT(headSocketError(QAbstractSocket::SocketError)));
          
  // Initialize walk control
  m_walkControlRunning = false;


  m_mainSocket->connectToHost(m_hostname, (qint16)MainControl);
}  

Aibo::Aibo::~Aibo()
{
  delete m_dataAccess;
}

// Camera Meta Functions
void Aibo::Aibo::startCamera()
{
  // Set camera resolution to maximum (416x320)
  set("vision.rawcam_y_skip", "0");
  set("vision.rawcam_uv_skip", "1");
  
  set("vision.rawcam_encoding", "color");
  set("vision.rawcam_interval", "50"); // Set ms delay to 25. (40fps) //TODO
  set("vision.rawcam_compress_quality", "85");
  set("vision.rawcam_transport", "tcp");
  set("vision.rawcam_compression", "jpeg");
  
  // Turn camera on
  sendCommand("!select \"Raw Cam Server\"");
  m_cameraRunning = true;

  // Wait a second so that the camera server can start up
  QTimer::singleShot(1000, this, SLOT(cameraConnect()));
}
void Aibo::Aibo::cameraConnect()
{
  m_cameraSocket->connectToHost(m_hostname, RawCamServer, QIODevice::ReadOnly);
}
void Aibo::Aibo::stopCamera()
{
  m_cameraSocket->disconnectFromHost();
  sendCommand("!select \"#Raw Cam Server\"");
  m_cameraRunning = false;
}
bool Aibo::Aibo::isCameraRunning() const
{
  return m_cameraRunning;
}
QImage Aibo::Aibo::cameraImage() const
{
  return m_currentFrame;
}

// Head Control Meta Functions
void Aibo::Aibo::startHeadControl()
{
  m_headSocket->disconnectFromHost();
  sendCommand("select \"Head Remote Control\"");
  m_headControlRunning = true;
}
void Aibo::Aibo::headConnect()
{
  m_headSocket->connectToHost(m_hostname, HeadRemoteControl, QIODevice::ReadWrite);
}
void Aibo::Aibo::stopHeadControl()
{
  m_headSocket->disconnectFromHost();
  sendCommand("select \"#Head Remote Control\"");
  m_headControlRunning = false;
}
bool Aibo::Aibo::isHeadControlRunning() const
{
  return m_headControlRunning;
}

void Aibo::startWalkControl()
{
  m_walkSocket->disconnectFromHost();
  sendCommand("select \"Walk Remote Control\"");
  m_walkControlRunning = true;
}
void Aibo::stopWalkControl()
{
  m_walkSocket->disconnectFromHost();
  sendCommand("select \"#Walk Remote Control\"");
  m_walkControlRunning = false;
}
bool Aibo::isWalkControlRunning() const
{
  return m_walkControlRunning;
}


// Main Socket Read/Write
void Aibo::Aibo::mainSocketReadyRead()
{

}
void Aibo::Aibo::mainSocketError(QAbstractSocket::SocketError error )
{
  qDebug() << "Main Socket Error! Yikes:" << m_mainSocket->errorString();
}


// Camera Socket Read/Write
void Aibo::Aibo::cameraSocketReadyRead()
{ 
  if (m_cameraSocket->bytesAvailable() < 35000) {
    return;
  }
  
  // Types taken from aibolib.cpp
  char *header, *type, *creator, *fmt, *image_buffer;
  long format, compression, newWidth, newHeight, timeStamp, frameNum, unknown1;
  long chanWidth, chanHeight, layer, chanID, unknown2, size;
  
  header = m_cameraSocket->read(4).data();  // \r\0\0\0
  type = readUntil(m_cameraSocket, (char)0); // "TekkotsuImage"
  format = convert(m_cameraSocket->read(4).data());
  compression = convert(m_cameraSocket->read(4).data());
  newWidth = convert(m_cameraSocket->read(4).data());
  newHeight = convert(m_cameraSocket->read(4).data());
  timeStamp = convert(m_cameraSocket->read(4).data());
  frameNum = convert(m_cameraSocket->read(4).data());
  unknown1 = convert(m_cameraSocket->read(4).data());
  //printf("unknown1: %ld\n", unknown1);
  //// Got creator=FbkImage
  //// Got chanwidth=104
  //// Got chanheight=80
  //// Got layer=3
  //// Got chan_id=0
  //// Got fmt=JPEGColor
  //// read JPEG: len=2547
  creator = readUntil(m_cameraSocket, (char)0); // creator
  chanWidth = convert(m_cameraSocket->read(4).data());
  chanHeight = convert(m_cameraSocket->read(4).data());
  layer = convert(m_cameraSocket->read(4).data());
  chanID = convert(m_cameraSocket->read(4).data());
  unknown2 = convert(m_cameraSocket->read(4).data());
  fmt = readUntil(m_cameraSocket, (char)0); // fmt
  size = convert(m_cameraSocket->read(4).data());
  image_buffer = m_cameraSocket->read(size).data();
  
  //qDebug() << "Image of size" << newWidth << newHeight << "with data size" << size;
  //   qDebug() << type << creator << fmt;
  //   qDebug() << "frame" << frameNum << "chan w/h" << chanWidth << chanHeight <<chanID;
  m_dataAccess->lock();
  m_currentFrame = QImage(newWidth, newHeight, QImage::Format_RGB32);
  m_currentFrame.loadFromData((uchar*)image_buffer, size, "JPG");
  m_dataAccess->unlock();
  
  emit cameraFrame(m_currentFrame);
}
void Aibo::Aibo::cameraSocketError(QAbstractSocket::SocketError error)
{
  qDebug() << "Camera Socket Error:" << m_cameraSocket->errorString();
}


// Head Socket Read/Write
void Aibo::Aibo::headSocketReadyRead()
{

}
void Aibo::Aibo::headSocketError(QAbstractSocket::SocketError error )
{
  qDebug() << "Head Socket Error:" << m_headSocket->errorString();
}

// Templated qBound() doesn't like mixed ints and reals
// so we fix it here instead of cluttering code below;
qreal qBound(int min, qreal value, int max) {
  return qBound((qreal)min, value, (qreal)max);
}

qreal Aibo::pan() const
{
  qWarning() << "pan() unimplemented";
  return 0;
}
qreal Aibo::tilt() const
{
  qWarning() << "tilt() unimplemented";
  return 0;
}
qreal Aibo::roll() const
{
  qWarning() << "roll() unimplemented";
  return 0;
}
void Aibo::setTilt(qreal tilt)
{
  tilt = qBound(-1, tilt, 0);
  sendControl(HeadTilt, tilt);
}
void Aibo::setPan(qreal pan)
{
  pan = qBound(-1, pan, 1);
  sendControl(HeadPan, pan);
}
void Aibo::setRoll(qreal roll)
{
  roll = qBound(0, roll, 1);
  sendControl(HeadRoll, roll);
}
void Aibo::setHeadOrientation(qreal tilt, qreal pan, qreal roll)
{
  setTilt(tilt);
  setPan(pan);
  setRoll(roll);
}

void Aibo::setTranslation(qreal velocity)
{
  qBound(-1, velocity, 1);
  sendControl(BodyTranslate, velocity);
}
void Aibo::setRotation(qreal velocity)
{
  qBound(-1, velocity, 1);
  sendControl(BodyRotate, velocity);
}
void Aibo::setStrafing(qreal velocity)
{
  qBound(-1, velocity, 1);
  sendControl(BodyStrafe, velocity);
}
void Aibo::setMovement(qreal translationVelocity, qreal rotationVelocity)
{
  setTranslation(translationVelocity);
  setRotation(rotationVelocity);
}

void Aibo::setJoint(Aibo::Joint joint, qreal x, qreal y, qreal z)
{
  x = qBound(-1, x, 1);
  y = qBound(-1, y, 1);
  z = qBound(-1, z, 1);
  qWarning() << "setJoint() unimplemented";
}



// Utilities
char* Aibo::Aibo::readUntil(QTcpSocket *socket, char stop)
{
  // Fuck. C programming at its finest (worst)
  static char retval[100]; // max buffer? 50?
  int numbytes = 0;
  char character[5];
  int pos = 0;
  numbytes = socket->read((char*)&character, 1);
  
  while (character[0] != stop && numbytes == 1 && pos < 50) { // no text is > 50
    retval[pos++] = character[0];
    numbytes = socket->read((char*)&character, 1);
  }
  retval[pos] = 0; // end of string
  //printf("readUntil: read %d chars\n", pos);
  return retval;
}

void Aibo::Aibo::sendCommand(const QString& command, QTcpSocket *socket)
{
  if (socket == 0) {
    socket = m_mainSocket;
  }
  socket->write(QString("%1\n").arg(command).toAscii());
}
void Aibo::sendControl(Control control, qreal amount)
{
  QTcpSocket *socket;
  QChar controlChar;
  switch (control) {
    case HeadTilt:
      controlChar = 't';
      socket = m_headSocket;
      break;
    case HeadPan:
      controlChar = 'p';
      socket = m_headSocket;
      break;
    case HeadRoll:
      controlChar = 'r';
      socket = m_headSocket;
      break;
    case BodyTranslate:
      controlChar = 'f';
      socket = m_walkSocket;
      break;
    case BodyRotate:
      controlChar = 'r';
      socket = m_walkSocket;
      break;
    case BodyStrafe:
      controlChar = 's';
      socket = m_walkSocket;
      break;
    default:
      qDebug() << "Unknown Control" << control;
      return;
      break;
  }
  
  //TODO: Verify Accuracy
  char command[5];
  command[0] = controlChar.toAscii();
  command[1] = (unsigned char)amount;
  
  sendCommand(command);
}
void Aibo::Aibo::set(const QString& property, const QString& value)
{
  sendCommand(QString("!set %1=%2").arg(property).arg(value));
}
