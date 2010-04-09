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
  def __init__(self, imagefile, background=[255, 255, 255]):
    self.image = Image.open(imagefile)
    self.background = background	
    self.width, self.height = self.image.size
    self.pixels = self.image.load()
    dimension = 5
    Distribution.__init__(self, dimension)
 
  def normalize(self, value, maxvalue):
    return 2.0*value/maxvalue - 1

  def generateNext(self):
    x, y, colors = 0, 0, [0]
    backgroundPoint = True
    
    # rand points until one isn't a background point
    while backgroundPoint:
      x = random.randint(0, self.width-1)
      y = random.randint(0, self.height-1)

      colors = list(self.pixels[x, y])
      
      for i in range(len(self.background)):
	if abs(colors[i] - self.background[i]) > 50:
	  backgroundPoint = False
    
    for i in range(len(colors)):
      colors[i] = self.normalize(colors[i], 255)
      
    # Remove alpha channel + others if we find them.
    while len(colors) > 3:
      colors.pop(len(colors)-1)

    point = [self.normalize(x, self.width), self.normalize(y, self.width)] + colors

    return point

gng = GrowingNeuralGas(dim=5)

for i in range(1, 12):
  reader = ImageReader("images/rgb/rgb%s.png" % i)
  gng.run(2000, reader)
  print "Number of units:" + str(len(gng.units))  

application = QApplication([])
view = GNGPlotter(reader.width, reader.height)
view.gng = gng
view.show()
application.exec_()
