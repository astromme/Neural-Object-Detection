# -*- coding: utf-8 -*-
import random, operator, time, commands

#------------------------------------------------------------------------
# utilities

def average(x, y):
    return (x + y) / 2.0

def writeVector(f, vector):
    for v in vector:
        f.write('%.3f ' % v)
    f.write('\n')

def savePoints(filename, points):
    f = open(filename, 'w')
    for p in points:
        writeVector(f, p)
    f.close()

def writeSeparator(f):
    f.write('\n\n')

def addPoints(p1, p2):
    """
    Returns p1 + p2, adding extra dimensions as necessary if the points
    differ in dimensionality.  Example: [1,2] + [3,4,5] => [4,6,5]
    """
    if len(p1) > len(p2):
        p2 += [0] * (len(p1) - len(p2))
    elif len(p2) > len(p1):
        p1 += [0] * (len(p2) - len(p1))
    return map(operator.add, p1, p2)

def readData(filename):
    f = open(filename)
    data = [[float(v) for v in line.split()] for line in f]
    f.close()
    return data

def getBounds(filename):
    data = readData(filename)
    transpose = zip(*data)
    minima = map(min, transpose)
    maxima = map(max, transpose)
    bounds = zip(minima, maxima)
    center = map(average, minima, maxima)
    radius = max([(M-m)/2.0 for (m, M) in bounds])
    boundingCube = [(x-radius, x+radius) for x in center]
    return center, bounds, boundingCube



#------------------------------------------------------------------------
# distributions

class Distribution(object):
    """
    Base class for distributions
    """
    def __init__(self, dimension):
        self.dimension = dimension
        self.g2 = None
        self.g3 = None

    def generateNext(self):
        """
        This method should be overridden by the user.
        """
        raise Exception("generateNext method is undefined")

    def saveSample(self, numPoints, name):
        """
        Generates a sample of points and saves them in <name>.dat,
        <name>.2d, and <name>.3d
        """
        assert type(numPoints) is int, 'first argument must be the number of points'
        samplePoints = self.generateSample(numPoints)
        savePointsAndProjections(samplePoints, name)

    def generateSample(self, numPoints):
        """
        Generate a number of points and return them in a list.
        """
        return [self.generateNext() for i in range(numPoints)]

