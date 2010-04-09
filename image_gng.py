# -*- coding: utf-8 -*-
from gng import GrowingNeuralGas
from gngdists import Distribution
from plotter import GNGPlotter
from PIL import Image
from PyQt4.Qt import *
import random, sys


class ImageReader(Distribution):
  """
  """
  def __init__(self, imagefile):
    self.image = Image.open(imagefile)
    self.width, self.height = self.image.size
    self.pixels = self.image.load()
    dimension = 5
    Distribution.__init__(self, dimension)
 
  def normalize(self, value, maxvalue):
    return 2.0*value/maxvalue - 1

  def generateNext(self):
    x = random.randint(0, self.width-1)
    y = random.randint(0, self.height-1)

    colors = list(self.pixels[x, y])
    
    for i in range(len(colors)):
      colors[i] = self.normalize(colors[i], 255)

    point = [self.normalize(x, self.width), self.normalize(y, self.width)] + colors

    return point

reader = ImageReader("images/rgb.png")
reader.generateNext()


gng = GrowingNeuralGas(dim=5)

gng.run(10000, reader)
print "Number of units:" + str(len(gng.units))

application = QApplication([])
view = GNGPlotter(200, 200)
view.gng = gng
view.show()
application.exec_()
