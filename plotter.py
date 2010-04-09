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
          
        for edge in self.gng.unique_edges:
          x1, y1, b, c, d = edge.fromUnit.vector
          x2, y2, b, c, d = edge.toUnit.vector
          
          x1 = self.unNormalize(x1, self.w)
          y1 = self.unNormalize(y1, self.h)
          x2 = self.unNormalize(x2, self.w)
          y2 = self.unNormalize(y2, self.h)
          
          c = QColor(0, 0, 0)
          p.setBrush(c)
          p.drawLine(x1, y1, x2, y2)
          
        for unit in self.gng.units:
          x, y, r, g, b = unit.vector
          x = self.unNormalize(x, self.w)
          y = self.unNormalize(y, self.h)
          r = self.unNormalize(r, 255)
          g = self.unNormalize(g, 255)
          b = self.unNormalize(b, 255)
          
          c = QColor(r, g, b)
          p.setBrush(c)
          p.drawEllipse(x-5, y-5, 10, 10)
                    
          
        
        

#if __name__ == '__main__':
    #a = QApplication(sys.argv)
    #QObject.connect(a,SIGNAL('lastWindowClosed()'),a,SLOT('quit()'))
    #w = TestWidget()
    #a.setMainWidget(w)
    #w.show()
    #a.exec_loop()
      