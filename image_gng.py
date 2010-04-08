# -*- coding: utf-8 -*-
from gng import GrowingNeuralGas
from gngdists import Distribution
from PIL import Image
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


gng = GrowingNeuralGas(dim=5, dir=sys.argv[1])

# run the GNG for 300 time steps on the given distribution and
# record data in the directory gng_data
gng.run(10000, reader)
print "Number of units:" + str(len(gng.units))
# create the movie
#gng.saveMovie()
# view the saved data as a 3-D movie at medium speed
#gng.view('3d')
# view the saved data as a 2-D movie at fast speed
#gng.view('2d', speed='fast')


#gng.view('2d3d', frame=250)
#gng.view('2d3d', start=200, end=250, speed='pause')

