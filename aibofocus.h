#ifndef _AIBO_FOCUS_H
#define _AIBO_FOCUS_H


#include "ui_aiboremote.h"

#include <QDebug>
#include <QApplication>
#include <QColor>
#include <QTimer>

#include <libaibo/aibo.h>
#include <libgng/aibosource.h>
#include <libaibo/aibocontrol.h>
#include <libgng/gng.h>

class AiboFocus : QObject {
  Q_OBJECT
  public:
    AiboFocus(GNG::GrowingNeuralGas* gng, Aibo* aibo);
    ~AiboFocus();
    void setColor(QColor focusColor);
    
  public slots:
    void followObject();
    
  private:
    GNG::GrowingNeuralGas* m_gng;
    AiboControl* m_aibo;    
    QColor m_focusColor;
    QTimer m_timer;
    bool m_modifyX;
};

#endif // _AIBO_FOCUS_H