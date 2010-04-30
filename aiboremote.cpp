#include "aiboremote.h"

#include "ui_aiboremote.h"

#include <QDebug>
#include <QApplication>

#include <libaibo/aibo.h>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    qDebug() << "Usage:" << argv[0] << "[hostname]";
    return 1;
  }
  
  QApplication app(argc, argv);
  
  Ui::Remote ui;
  QWidget remote;
  ui.setupUi(&remote);
  remote.show();
  
  Aibo aibo(argv[1]);
  AiboRemote control(&aibo);
  

  QObject::connect(ui.cameraOn, SIGNAL(clicked(bool)), &aibo, SLOT(startCamera()));
  QObject::connect(ui.cameraOff, SIGNAL(clicked(bool)), &aibo, SLOT(stopCamera()));
  
  QObject::connect(ui.headControlOn, SIGNAL(clicked(bool)), &aibo, SLOT(startHeadControl()));
  QObject::connect(ui.headControlOff, SIGNAL(clicked(bool)), &aibo, SLOT(stopHeadControl()));
  
  QObject::connect(ui.bodyControlOn, SIGNAL(clicked(bool)), &aibo, SLOT(startWalkControl()));
  QObject::connect(ui.bodyControlOff, SIGNAL(clicked(bool)), &aibo, SLOT(stopWalkControl()));
  
  QObject::connect(ui.headLeft, SIGNAL(clicked(bool)), &control, SLOT(headLeft()));
  QObject::connect(ui.headRight, SIGNAL(clicked(bool)), &control, SLOT(headRight()));
  QObject::connect(ui.headCenter, SIGNAL(clicked(bool)), &control, SLOT(headCenter()));
  QObject::connect(ui.headUp, SIGNAL(clicked(bool)), &control, SLOT(headUp()));
  QObject::connect(ui.headDown, SIGNAL(clicked(bool)), &control, SLOT(headDown()));
  
  QObject::connect(ui.forwards, SIGNAL(clicked(bool)), &control, SLOT(moveForward()));
  QObject::connect(ui.backwards, SIGNAL(clicked(bool)), &control, SLOT(moveBackward()));
  QObject::connect(ui.stop, SIGNAL(clicked(bool)), &control, SLOT(stop()));
  QObject::connect(ui.bodyLeft, SIGNAL(clicked(bool)), &control, SLOT(moveLeft()));
  QObject::connect(ui.bodyRight, SIGNAL(clicked(bool)), &control, SLOT(moveRight()));
  
  
  app.exec();
};