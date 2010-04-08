# Graphical support for viewing GNG networks
# Jim Marshall
# Version 7/16/09

# IMPORTANT: to run this code, you must have the Unix program
# "cluster" installed on your system (and visible in your PATH), and
# the Gnuplot python module installed.

import random, math, time, commands, os.path, Gnuplot, sys
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

    Every GNG object has an associated directory where data files for
    plotting the network in 2-D and 3-D are stored, as well as the
    random seed value used to create the data.
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
        self.winnerLearnRate = 0.2
        self.neighborLearnRate = 0.006
        self.maxAge = 50
        self.reduceError = 0.995
        self.stepsToInsert = 300
        self.lastInsertedStep = 0
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
        # 2-D plot
        self.gplot2 = Gnuplot.Gnuplot()
        self.gplot2('set term x11')
        self.gplot2('set size ratio -1')
        self.gplot2('set key off')
        # 3-D plot
        self.gplot3 = Gnuplot.Gnuplot()
        self.gplot3('set term x11')
        self.gplot3('set size ratio -1')
        self.gplot3('set key off')
        self.gplot3('set ticslevel 0')
        self.plot_edges = []
        self.plot_units = []

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
        self.addPlotEdge(newEdge1)

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
                self.removePlotUnit(self.units[i])
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
        self.addPlotUnit(newUnit)
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
        if self.averageError() > 0.5 and self.lastInsertedStep > self.stepsToInsert:
          print "Creating new unit at timestep %d and error %.3f" % (self.stepCount, self.averageError())
          self.lastInsertedStep = 0
          self.insertUnit()
        self.reduceAllErrors()
        self.stepCount += 1
        self.lastInsertedStep += 1

    #----------------------------------------------------------------------
    # graphics support

    def addPlotUnit(self, unit):
        self.plot_units.append(unit)
        if self.verbose > 0:
            print 'Added plot unit (now %d units)' % len(self.plot_units)

    def addPlotEdge(self, edge):
        self.plot_edges.append(edge)
        if self.verbose > 0:
            print 'Added plot edge (now %d edges)' % len(self.plot_edges)

    def removePlotUnit(self, unit):
        for i in range(len(self.plot_units)):
            if self.plot_units[i] is unit:
                self.plot_units.pop(i)
                if self.verbose > 0:
                    print 'Removed plot unit (now %d units)' % len(self.plot_units)
                return

    def removeUniqueEdge(self, edge):
        for i in range(len(self.unique_edges)):
            if self.unique_edges[i] is edge:
                self.unique_edges.pop(i)
                if self.verbose > 0:
                    print 'Removed plot edge (now %d edges)' % len(self.plot_edges)
                return True
        return False

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

    def clearDirectory(self, dir):
        if os.path.exists(dir):
            answer = raw_input('Clear directory %s? ' % dir)
            if answer not in ('y', 'Y'):
                print 'Aborted - no files deleted'
                return False
            print 'Clearing directory %s' % dir
            commands.getoutput('rm -f %s/basis' % dir)
            if os.path.exists('%s/dist/' % dir):
                commands.getoutput('rm -f %s/dist/*' % dir)
            else:
                commands.getoutput('mkdir %s/dist/' % dir)
            if os.path.exists('%s/frames/' % dir):
                commands.getoutput('rm -f %s/frames/*' % dir)
            else:
                commands.getoutput('mkdir %s/frames/' % dir)
            if not os.path.exists('%s/empty' % dir):
                commands.getoutput('touch %s/empty' % dir) # gnuplot hack. see plotFrame3D
        else:
            print 'Creating directory %s to store movie frames' % dir
            commands.getoutput('mkdir %s/' % dir)
            commands.getoutput('mkdir %s/frames/' % dir)
            commands.getoutput('mkdir %s/dist/' % dir)
            commands.getoutput('touch %s/empty' % dir) # gnuplot hack. see plotFrame3D
        return True

    def saveGNGData(self, dir, timeStep):
        filename = '%s/frames/gng_%05d.dat' % (dir, timeStep)
        f = open(filename, 'w')
        # to plot the model vectors and edges correctly in 3-D in
        # Gnuplot using linespoints, each pair of points representing
        # an edge must be separated from the others by two blank lines
        for v in self.model_vectors[timeStep]:
            writeVector(f, v)
            writeSeparator(f)
        for (v1, v2) in self.edge_vectors[timeStep]:
            writeVector(f, v1)
            writeVector(f, v2)
            writeSeparator(f)
        f.close()

    def saveGNGProjections(self, dimension, frame, basis):
        if dimension == 2:
            commands.getoutput('cluster -p -e%s -c1,2 %s.dat > %s.tmp' % (basis, frame, frame))
        else:
            commands.getoutput('cluster -p -e%s -c1,2,3 %s.dat > %s.tmp' % (basis, frame, frame))
        # cluster does not preserve blank lines, so we must first
        # create a .tmp file of the transformed vectors and then
        # create the final .2d or .3d file from the .tmp file by
        # manually adding blank lines to match the original file
        src = open('%s.dat' % frame)
        tmp = open('%s.tmp' % frame)
        dst = open('%s.%dd' % (frame, dimension), 'w')
        for line in src:
            if line == '\n':
                dst.write('\n')
            else:
                dst.write(tmp.readline())
        src.close()
        tmp.close()
        dst.close()
        commands.getoutput('rm -f %s.tmp' % frame)

    def saveDistProjections(self, dimension, frame, basis):
        # assert cluster program exists
        if dimension == 2:
            commands.getoutput('cluster -p -e%s -c1,2 %s.dat > %s.2d' % (basis, frame, frame))
        else:
            commands.getoutput('cluster -p -e%s -c1,2,3 %s.dat > %s.3d' % (basis, frame, frame))

    def saveMovie(self, dir=None, stepSize=1):
        # assert cluster program exists
        # initialize data directory to store movie frames
        if dir is None:
            dir = self.dataDir
        if not self.clearDirectory(dir):
            return

        print 'saving GNG data'
        for timeStep in range(self.stepCount+1):
            self.saveGNGData(dir, timeStep)
        print 'saving distribution data'
        for timeStep in range(self.stepCount+1):
            dataFile = '%s/dist/dist_%05d.dat' % (dir, timeStep)
            savePoints(dataFile, self.dist_points[:timeStep])
            
        # create eigenbasis using model vectors from last time step
        basis = '%s/basis' % dir
        lastFrame = '%s/frames/gng_%05d.dat' % (dir, self.stepCount)
        commands.getoutput('cluster -p -e%s -c1,2 %s > /dev/null' % (basis, lastFrame))

        print 'creating projections of GNG vectors/edges'
        for timeStep in range(self.stepCount+1):
            frame = '%s/frames/gng_%05d' % (dir, timeStep)
            self.saveGNGProjections(2, frame, basis)
            self.saveGNGProjections(3, frame, basis)
            if timeStep > 0 and timeStep % 100 == 0 or timeStep == self.stepCount:
                print '  %d/%d complete' % (timeStep, self.stepCount)

        print 'creating projections of distribution points'
        for timeStep in range(self.stepCount+1):
            frame = '%s/dist/dist_%05d' % (dir, timeStep)
            self.saveDistProjections(2, frame, basis)
            self.saveDistProjections(3, frame, basis)
            commands.getoutput('tail -1 %s.2d > %s.last.2d' % (frame, frame))
            commands.getoutput('tail -1 %s.3d > %s.last.3d' % (frame, frame))
            if timeStep > 0 and timeStep % 100 == 0 or timeStep == self.stepCount:
                print '  %d/%d complete' % (timeStep, self.stepCount)

#-------------------------------------------------------------------------------------------

    def view(self, mode='2d', speed='medium', dir=None, frame=None, start=0, end=None):
        # verify the arguments
        assert mode in ('2d', '3d', '2d3d', '3d2d'), 'bad mode argument: %s' % mode
        delay = {'fast': 0.001, 'medium': 0.05, 'slow': 0.3, 'pause': 0}
        assert speed in delay, 'bad speed argument: %s' % speed
        if dir is None:
            dir = self.dataDir
        if not os.path.exists(dir):
            print 'directory does not exist: %s' % dir
            return
        assert frame is None or type(frame) is int, 'bad frame argument: %s' % frame
        assert type(start) is int, 'bad start argument: %s' % start
        assert end is None or type(end) is int, 'bad end argument: %s' % end
        # figure out last frame
        gngFrames = commands.getoutput('ls %s/frames/gng_*.dat' % dir).split()
        distFrames = commands.getoutput('ls %s/dist/dist_*.dat' % dir).split()
        gngFrames.sort()
        distFrames.sort()
        lastFrame = int(gngFrames[-1][-9:-4])

        if frame is not None:
            if not 0 <= frame <= lastFrame:
                print 'no such frame: %d' % frame
                return
            start = frame
            end = frame
        elif not 0 <= start <= lastFrame:
            print 'no such frame: %d' % start
            return
        elif end is None:
            end = lastFrame
        elif not 0 <= end <= lastFrame:
            print 'no such frame: %d' % end
            return
        elif end < start:
            end = start

        # plot the frames
        for timeStep in range(start, end+1):
            if '2d' in mode:
                self.plotFrame2D(dir, timeStep)
            if '3d' in mode:
                self.plotFrame3D(dir, timeStep)
            time.sleep(delay[speed])
            if speed == 'pause' and timeStep != end:
                if raw_input('Press RETURN to continue (q to quit)...') in ('q', 'Q'):
                    return

    def plotFrame2D(self, dir, timeStep):
        gng_2d = '%s/frames/gng_%05d.2d' % (dir, timeStep)
        dist_2d = '%s/dist/dist_%05d.2d' % (dir, timeStep)
        last_2d = '%s/dist/dist_%05d.last.2d' % (dir, timeStep)
        if not os.path.exists(gng_2d):
            print 'WARNING: %s does not exist' % gng_2d
            return
        cmd = 'plot [-1:2][-1:2] '
        # plot complains about plotting empty files, so we need to
        # avoid plotting the distribution files on time step 0, which
        # contain no points.  splot, however, is fine with empty files.
        if timeStep == 0:
            cmd += '"%s" with linespoints lc 3 pt 7 ps 2' % gng_2d   # model vectors/edges (lc 3=blue)
        else:
            cmd += '"%s" with points lc 1 pt 6 ps 2' % last_2d       # current sample point (lc 1=red)
            cmd += ',"%s" with points lc 2 pt 7 ps 1' % dist_2d      # distribution points (lc 2=green)
            cmd += ',"%s" with linespoints lc 3 pt 7 ps 2' % gng_2d  # model vectors/edges (lc 3=blue)
        self.gplot2('set title "GNG: time step %d' % timeStep)
        self.gplot2(cmd)
            
    def plotFrame3D(self, dir, timeStep):
        gng_3d = '%s/frames/gng_%05d.3d' % (dir, timeStep)
        dist_3d = '%s/dist/dist_%05d.3d' % (dir, timeStep)
        last_3d = '%s/dist/dist_%05d.last.3d' % (dir, timeStep)
        if not os.path.exists(gng_3d):
            print 'WARNING: %s does not exist' % gng_3d
            return
        cmd = 'splot [-1:2][-1:2][-1:2] '
        cmd += '"%s" with points pt 6 ps 1.75' % last_3d       # current sample point (red)
        cmd += ',"%s" with points pt 7 ps 0.75' % dist_3d      # distribution points (green)
        cmd += ',"%s" with linespoints pt 7 ps 1' % gng_3d     # model vectors/edges (blue)
        self.gplot3('set title "GNG: time step %d' % timeStep)
        self.gplot3(cmd)

#-------------------------------------------------------------------
# Example distributions (defined in gngdists.py)
#
# All Distribution objects have a generateNext() method that returns a
# new point chosen at random from the distribution, represented as a
# list of numbers.
#
# continuous:
#   largeSphere, smallSphere, largeCircle, smallCircle, rect, galaxy,
#   blobs, clover, planes
#
# discrete:
#   latt, dplanes
#
# discrete/continuous:
#   mixed

# To view distributions with a sample of N points:
# % python -i gng.py
# >>> largeSphere.view(2000)
# >>> galaxy.view(2000)
# >>> latt.view(10000)
# >>> mixed.view(10000)
# etc.

#-------------------------------------------------------------------
# GNG example

# To run:
# % python -i gng.py
# >>> main()
# >>> show()
# >>> sstep()

