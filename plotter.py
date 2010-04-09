# -*- coding: utf-8 -*-
import numpy as np
from PyQt4 import *
from PyQt4.QtGui import *
from gng import *

class GNGPlotter(QWidget):
    def __init__(self, w, h, *args):
        apply(QWidget.__init__, (self,) + args)

        self.resize(w, h)
        self.setMinimumSize(w,h)
        self.setMaximumSize(w,h)

        self.h = h
        self.w = w
        
        self.gng = None

    def unNormalize(self, value, maxValue):
       return maxValue*(value + 1.0)/2.0

    def paintEvent(self, ev):
        p = QPainter(self)
        if not self.gng:
          return
          
        for unit in self.gng.units:
          x, y, r, g, b = unit.vector
          x = self.unNormalize(x, self.w)
          y = self.unNormalize(y, self.h)
          r = self.unNormalize(r, 255)
          g = self.unNormalize(g, 255)
          b = self.unNormalize(b, 255)
          
          c = QColor(r, g, b)
          p.setBrush(c)
          p.drawEllipse(x, y, 10, 10)
        
        

#if __name__ == '__main__':
    #a = QApplication(sys.argv)
    #QObject.connect(a,SIGNAL('lastWindowClosed()'),a,SLOT('quit()'))
    #w = TestWidget()
    #a.setMainWidget(w)
    #w.show()
    #a.exec_loop()
      