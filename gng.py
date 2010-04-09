# -*- coding: utf-8 -*-
# Graphical support for viewing GNG networks
# Jim Marshall
# Version 7/16/09

# IMPORTANT: to run this code, you must have the Unix program
# "cluster" installed on your system (and visible in your PATH), and
# the Gnuplot python module installed.

import random, math, time, commands, os.path, sys
from gngdists import *

"""
Author: Lisa Meeden
Date: 2/14/2008

Implementation of GNG as described in the paper 'A Growing Neural Gas
Network Learns Topologies' by Bernd Fritzke published in 'Advances in
Neural Information Processing 7', MIT Press, 1995.

GNG is an incremental network model able to learn the topological
relationships in a given set of input vectors using a simple Hebb-like
learning rule.
"""

class Unit:
    """
    Each unit in the GNG maintains a reference vector, an error
    measure, and a list of edges.
    """
    
    def __init__(self, vector = None, dimension=2, minVal=-1, maxVal=1):
        self.dimension = dimension
        self.minVal = minVal
        self.maxVal = maxVal
        if vector:
            self.vector = vector
        else:
            self.vector = self.randomVector()
        self.error = 0
        self.edges = []

    def __str__(self):
        result = "Unit:\nVector: %s\n Error: %s\n" % (self.vectorStr(), self.error)
        for e in self.edges:
            result += str(e)
        return result

    def vectorStr(self):
        result = "[ "
        for i in range(len(self.vector)):
            result += "%.3f " % self.vector[i]
        result += "] "
        return result
    
    def getEdgeTo(self, unit):
        """
        Returns the edge to the given unit or None.
        """
        for edge in self.edges:
            if edge.toUnit == unit:
                return edge
        return None

    def getNeighbors(self):
        """
        Returns a list of its immediate neighboring units. 
        """
        return [edge.toUnit for edge in self.edges]

    def randomVector(self):
        """
        Generats a random reference vector within the appropriate bounds.
        """
        return [random.uniform(self.minVal, self.maxVal) for i in range(self.dimension)]

    def moveVector(self, towardPoint, lrate):
        """
        Moves the reference vector toward the given point based on the
        given learning rate.
        """
        for i in range(len(towardPoint)):
            self.vector[i] += lrate*(towardPoint[i]-self.vector[i])

class Edge:
    """
    Edges in the GNG are undirected.  However for ease of
    implementation, the edges are represented as one-way. For example,
    if unitA and unitB and connected, then unitA maintains an edge to
    unitB and unitB maintains an edge to unitA.  Edges also maintain
    their age.  If an edge becomes too old, it will be removed.
    """
    def __init__(self, fromUnit, toUnit):
        self.fromUnit = fromUnit
        self.toUnit = toUnit
        self.age = 0

    def __str__(self):
        return "Edge to: %s Age: %s\n" % (self.toUnit.vectorStr(), self.age)

class GrowingNeuralGas:
    """
    Parameters:

    winnerLearnRate   Used to adjust closest unit towards input point
    neighborLearnRate Used to adjust other neighbors towards input point
    maxAge            Edges older than maxAge are removed
    reduceError       All errors are reduced by this amount each GNG step
    stepsToInsert     Min steps before inserting a new node
    insertError       Error of every new unit is reduced by this amount
    
    NOTE: The default values are taken from the paper.

    The GNG always begins with two randomly placed units.

    """
    def __init__(self, dim=2, dir='gng_data', seed=None, verbose=0, \
                 minimum = -1, maximum = 1):
        self.dimension = dim
        self.dataDir = dir
        # set and store random seed
        if seed is None:
            self.seed = int(time.time())
        else:
            self.seed = seed
        random.seed(self.seed)
        print 'Random seed is', self.seed
        # GNG parameters
        self.winnerLearnRate = 0.3
        self.neighborLearnRate = 0.01
        self.maxAge = 50
        self.reduceError = 0.995
        self.stepsToInsert = 100
        self.lastInsertedStep = 101 # Allow a unit to be inserted immedately
        self.insertError = 0.5
        self.verbose = verbose
        self.stepCount = 0
        self.units = [Unit(dimension=dim, minVal = minimum, maxVal = maximum),\
                      Unit(dimension=dim, minVal = minimum, maxVal = maximum)]
        self.unique_edges = []
        # model_vectors[t] is a list of the model vectors at time t
        self.model_vectors = [[u.vector[:] for u in self.units]]
        # edge_vectors[t] is a list [(u,v), ...] of model vector pairs
        # representing the edges at time t
        self.edge_vectors = [[]]
        # dist_points[t-1] is the distribution point generated at time t
        self.dist_points = []

    def __str__(self):
        result = "GNG step %d\nNumber of units: %d\nAverage error: %s\n" % \
            (self.stepCount, len(self.units), self.averageError())
        if self.verbose > 1:
            for unit in self.units:
                result += str(unit)
        return result

    def distance(self, v1, v2):
        """
        Returns the Euclidean distance between two vectors.
        """
        return math.sqrt(sum([(v1[i]-v2[i])**2 for i in range(len(v1))]))

    def unitOfInterest(self, unit, cutoff):
        """
        Used to focus on particular units when debugging.
        """
        for value in unit.vector:
            if abs(value) > cutoff:
                return True
        return False

    def computeDistances(self, point):
        """
        Computes the distances between the given point and every unit
        in the GNG.  Returns the closest and next closest units.
        """
        dists = []
        for i in range(len(self.units)):
            dists.append((self.distance(self.units[i].vector, point), i))
        dists.sort()
        best = dists[0][1]
        second = dists[1][1]
        if self.verbose > 1:
            print "Processing:", point
            print "Closest:", self.units[best].vectorStr()
            print "Second:", self.units[second].vectorStr()
            print
        return self.units[best], self.units[second]

    def incrementEdgeAges(self, unit):
        """
        Increments the ages of every unit directly connected to the
        given unit.
        """
        for outgoing in unit.edges:
            outgoing.age += 1
            incoming = outgoing.toUnit.getEdgeTo(unit)
            incoming.age += 1

    def connectUnits(self, a, b):
        """
        Adds the appropriate edges to connect units a and b.
        """
        if self.verbose >= 1:
            print "Add edge:", a.vectorStr(), b.vectorStr()
        newEdge1 = Edge(a, b)
        newEdge2 = Edge(b, a)
        a.edges.append(newEdge1)
        b.edges.append(newEdge2)
        # only need to add one edge to unique_edges
        self.unique_edges.append(newEdge1)

    def disconnectUnits(self, a, b):
        """
        Removes the appropriate edges to disconnect units a and b.
        """
        if self.verbose >= 1:
            print "Remove edge:", a.vectorStr(), b.vectorStr()
        edge1 = a.getEdgeTo(b)
        edge2 = b.getEdgeTo(a)
        a.edges.remove(edge1)
        b.edges.remove(edge2)
        # remove whichever one is in self.unique_edges
        if edge1 in self.unique_edges:
            self.unique_edges.remove(edge1)
        else:
            self.unique_edges.remove(edge2)

    def removeStaleEdges(self):
        """
        Checks all edges in the GNG and removes any with an age exceeding
        the maxAge parameter.  Also removes any unit that is completely
        disconnected.
        """
        for unit in self.units:
            for i in range(len(unit.edges)-1, -1, -1):
                edge = unit.edges[i]
                if edge.age > self.maxAge:
                    if self.verbose >= 1:
                        adjacent = edge.toUnit
                        print "Removing stale edge: %s %s" % \
                              (unit.vectorStr(), adjacent.vectorStr())
                    if edge in self.unique_edges:
                        self.unique_edges.remove(edge)
                    unit.edges.pop(i)

        for i in range(len(self.units)-1, -1, -1):
            if len(self.units[i].edges) == 0:
                if self.verbose >= 1:
                    print "Removing disconnected unit:", unit.vectorStr()
                self.units.pop(i)

    def maxErrorUnit(self, unitList):
        """
        Given a list of units, returns the unit with the highest error.
        """
        highest = unitList[0]
        for i in range(1, len(unitList)):
            if unitList[i].error > highest.error:
                highest = unitList[i]
        return highest

    def averageError(self):
        """
        Returns the average error across all units in the GNG.
        """
        return sum([u.error for u in self.units]) / float(len(self.units))

    def insertUnit(self):
        """
        Inserts a new unit into the GNG.  Finds the unit with the highest
        error and then finds its topological neighbor with the highest
        error and inserts the new unit between the two. 
        """
        worst = self.maxErrorUnit(self.units)
        if self.verbose > 1:
            print "Max error %s" % worst
        worstNeighbor = self.maxErrorUnit(worst.getNeighbors())
        newVector = map(average, worst.vector, worstNeighbor.vector)
        newUnit = Unit(newVector)
        self.units.append(newUnit)
        if self.verbose > 0:
            print "Insert unit: %s\nTotal units: %d" % (newUnit.vectorStr(), len(self.units))
        self.connectUnits(newUnit, worst)
        self.connectUnits(newUnit, worstNeighbor)
        self.disconnectUnits(worst, worstNeighbor)
        worst.error *= self.insertError
        worstNeighbor.error *= self.insertError
        newUnit.error = worst.error

    def reduceAllErrors(self):
        """
        Decays the error at all units.
        """
        for unit in self.units:
            unit.error *= self.reduceError
                
    def step(self, nextPoint):
        """
        Processes one input point at a time through the GNG.
        """
        if (self.stepCount % 1000) == 0:
	  print "Step ", self.stepCount
        best, second = self.computeDistances(nextPoint)
        self.incrementEdgeAges(best)
        best.error += self.distance(best.vector, nextPoint)**2
        best.moveVector(nextPoint, self.winnerLearnRate)
        for unit in best.getNeighbors():
            unit.moveVector(nextPoint, self.neighborLearnRate)
        edgeExists = best.getEdgeTo(second)
        if edgeExists:
            edgeExists.age = 0
            second.getEdgeTo(best).age = 0
        else:
            self.connectUnits(best, second)
        self.removeStaleEdges()

        #if self.stepCount % self.stepsToInsert == 0:
        if self.averageError() > 0.05 and self.lastInsertedStep > self.stepsToInsert:
          print "Creating new unit at timestep %d and error %.3f" % (self.stepCount, self.averageError())
          self.lastInsertedStep = 0
          self.insertUnit()
        self.reduceAllErrors()
        self.stepCount += 1
        self.lastInsertedStep += 1

    def run(self, cycles, dist):
        assert cycles > 0
        assert dist.dimension == self.dimension, \
            'distribution dimensionality does not match GNG dimensionality'

        # run the GNG
        if self.stepCount == 0:
            print 'running GNG for %d cycles' % cycles
        else:
            print 'running GNG for %d additional cycles' % cycles
        for i in range(cycles):
            nextPoint = dist.generateNext()
            self.step(nextPoint)
            self.saveCurrentData(nextPoint)
            
    def saveCurrentData(self, nextPoint):
        currentModelVectors = [u.vector[:] for u in self.units]
        currentEdges = [(e.fromUnit.vector[:], e.toUnit.vector[:]) for e in self.unique_edges]
        self.model_vectors.append(currentModelVectors)
        self.edge_vectors.append(currentEdges)
        self.dist_points.append(nextPoint)

