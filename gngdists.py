import random, operator, time, commands, Gnuplot

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

def savePointsAndProjections(points, name):
    """
    Saves the given data points, which must all be of the same
    dimensionality, in a file called <name>.dat.  Also saves 2D and 3D
    projections of the points in <name>.2d and <name>.3d
    """
    # FIXME: do we need to use eigenbasis when creating 2-D and 3-D points?
    dimension = len(points[0])
    data = '%s.dat' % name
    data2D = '%s.2d' % name
    data3D = '%s.3d' % name
    basis = '%s.basis' % name
    commands.getoutput('rm -f %s %s %s %s' % (data, data2D, data3D, basis))
    f = open(data, 'w')
    for point in points:
        writeVector(f, point)
    f.close()
    if dimension == 1:
        addZeros(data, data2D, '0')
        addZeros(data, data3D, '0 0')
    elif dimension == 2:
        commands.getoutput('cp %s %s' % (data, data2D))
        addZeros(data, data3D, '0')
    elif dimension == 3:
        commands.getoutput('cluster -p -e%s -c1,2 %s > %s' % (basis, data, data2D))
        commands.getoutput('cp %s %s' % (data, data3D))
    else:
        commands.getoutput('cluster -p -e%s -c1,2 %s > %s' % (basis, data, data2D))
        commands.getoutput('cluster -p -e%s -c1,2,3 %s > %s' % (basis, data, data3D))
    #print 'Saved %d points in %s, %s, and %s' % (len(points), data, data2D, data3D)


def addZeros(srcFilename, dstFilename, zeros):
    src = open(srcFilename, 'r')
    dst = open(dstFilename, 'w')
    for point in src:
        dst.write(point.strip() + ' ' + zeros + '\n')
    src.close()
    dst.close()

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

    def view(self, numPoints):
        self.saveSample(numPoints, 'sample')
        self.view2D()
        self.view3D()

    def view2D(self, name='sample'):
        if self.g2:
            del self.g2
        data2D = '%s.2d' % name
        self.g2 = Gnuplot.Gnuplot()
        self.g2('set term x11')
        self.g2('set size ratio -1')
        self.g2('set key off')
        if self.dimension <= 2:
            self.g2('set title "Data points"')
        else:
            self.g2('set title "2-D projection of data points"')
        center, bounds, bbox = getBounds(data2D)
        axes = ''.join(['[%g:%g]' % (m, M) for (m, M) in bbox])
        self.g2('plot %s "%s" with points pt 7 ps 1' % (axes, data2D))

    def view3D(self, name='sample'):
        if self.g3:
            del self.g3
        data3D = '%s.3d' % name
        self.g3 = Gnuplot.Gnuplot()
        self.g3('set term x11')
        self.g3('set size ratio -1')
        self.g3('set ticslevel 0')
        self.g3('set key off')
        if self.dimension <= 3:
            self.g3('set title "Data points"')
        else:
            self.g3('set title "3-D projection of data points"')
        center, bounds, bcube = getBounds(data3D)
        axes = ''.join(['[%g:%g]' % (m, M) for (m, M) in bcube])
        self.g3('splot %s "%s" with points pt 7 ps 1' % (axes, data3D))
        # rotate image back and forth
        for view in range(50) + range(50, 0, -1):
            self.g3('set view 80, %d, 1, 1' % view)
            self.g3('replot')
            time.sleep(0.01)


class HyperSphere(Distribution):
    """
    A continuous N-dimensional hypersphere with a given center and
    radius.  The dimensionality is determined by the center point.
    """
    def __init__(self, center, radius=1):
        dimension = len(center)
        assert dimension > 0, 'dimension must be > 0'
        Distribution.__init__(self, dimension)
        self.center = center
        self.radius = radius
        # distribution boundaries for each dimension
        self.bounds = [(x-radius, x+radius) for x in center]
        # axes to use for each dimension when plotting distribution
        self.axes = self.bounds

    def generateNext(self):
        while True:
            point = [random.uniform(-self.radius, self.radius) for i in range(self.dimension)]
            if sum([x**2 for x in point]) <= self.radius**2:
                return addPoints(point, self.center)
            

class BoundedVolume(Distribution):
    """
    A continuous N-dimensional rectangular volume.  The dimensionality
    is determined by the number of intervals specified.  Examples:

    BoundedVolume([0, 12], [7, 9], [-10, -1]) represents a 3-dimensional
    rectangular solid with bounds 0 <= x < 12, 7 <= y < 9, -10 <= z < -1.

    BoundedVolume([-3, 3], [-3, 3]) represents a 2-dimensional square
    with bounds -3 <= x < 3, -3 <= y < 3.

    BoundedVolume([5, 10]) represents a 1-dimensional line segment with
    bounds 5 <= x < 10.
    """
    def __init__(self, *intervals):
        dimension = len(intervals)
        assert dimension > 0, 'dimension must be > 0'
        for (m, M) in intervals:
            assert m <= M, 'empty interval: [%d, %d]' % (m, M)
        Distribution.__init__(self, dimension)
        # distribution boundaries for each dimension
        self.bounds = intervals
        # center point of distribution
        self.center = [average(m, M) for [m, M] in intervals]
        # radius is half the width of the bounding hypercube
        self.radius = max([(M-m)/2.0 for [m, M] in intervals])
        # axes to use for each dimension when plotting distribution
        self.axes = [(x-self.radius, x+self.radius) for x in self.center]

    def generateNext(self):
        return [random.uniform(m, M) for (m, M) in self.bounds]


class DiscreteLattice(Distribution):
    """
    A discrete N-dimensional rectangular volume of regularly-spaced
    lattice points.  The dimensionality is determined by the number of
    ranges specified.  Examples:

    DiscreteLattice(range(2), range(5), range(7, 10)) represents a
    3-dimensional rectangular lattice of points with bounds 0 <= x <= 1,
    0 <= y <= 4, 7 <= z <= 9.
    """
    def __init__(self, *ranges):
        dimension = len(ranges)
        assert dimension > 0, 'dimension must be > 0'
        Distribution.__init__(self, dimension)
        self.allPoints = self.enumerate(*ranges)
        minima = [min(r) for r in ranges]
        maxima = [max(r) for r in ranges]
        # distribution boundaries for each dimension
        self.bounds = zip(minima, maxima)
        # center point of distribution
        self.center = map(average, minima, maxima)
        # radius is half the width of the bounding hypercube
        self.radius = max(map(operator.sub, maxima, minima))/2.0
        # axes to use for each dimension when plotting distribution
        self.axes = [(x-self.radius, x+self.radius) for x in self.center]

    def enumerate(self, *ranges):
        if len(ranges) == 0:
            return [[]]
        else:
            points = self.enumerate(*ranges[:-1])
            return reduce(operator.add, [[p+[i] for p in points] for i in ranges[-1]])

    def generateNext(self):
        return random.choice(self.allPoints)


class CompositeDistribution(Distribution):
    """
    A composite distribution is formed by the union of one or more
    sub-distributions, which do not have to be of the same
    dimensionality.  The composite dimensionality is the maximum of
    the sub-distribution dimensionalities.
    """
    def __init__(self, distributions):
        assert len(distributions) > 0, 'requires at least one distribution'
        dimension = max([dist.dimension for dist in distributions])
        Distribution.__init__(self, dimension)
        self.distributions = distributions
        self.zero = [0] * self.dimension
        # merge boundaries of all sub-distributions
        minima = []
        maxima = []
        for dist in distributions:
            for i in range(len(dist.bounds)):
                m, M = dist.bounds[i]
                if i < len(minima):
                    minima[i] = min(m, minima[i])
                    maxima[i] = max(M, maxima[i])
                else:
                    minima.append(m)
                    maxima.append(M)
        self.bounds = zip(minima, maxima)
        # center point of distribution
        self.center = map(average, minima, maxima)
        # radius is half the width of the bounding hypercube
        self.radius = max(map(operator.sub, maxima, minima))/2.0
        # axes to use for each dimension when plotting distribution
        self.axes = [(x-self.radius, x+self.radius) for x in self.center]

    def generateNext(self):
        point = random.choice(self.distributions).generateNext()
        if len(point) < self.dimension:
            # adjust dimensionality of point
            return addPoints(point, self.zero)
        else:
            return point


#------------------------------------------------------------------------------
# example distributions

# 3-dimensional spheres
largeSphere = HyperSphere(center=(0,0,0), radius=1)
smallSphere = HyperSphere(center=(0,0,0), radius=0.5)

# 2-dimensional circles
largeCircle = HyperSphere(center=(0,0), radius=1)
smallCircle = HyperSphere(center=(0,0), radius=0.5)

# 3-dimensional rectangular solid
rect = BoundedVolume([-5,8], [7,9], [-10,-1])

galaxy = CompositeDistribution([
        HyperSphere(center=(0,0,0), radius=0.3),
        HyperSphere(center=(0,0), radius=1),
        ])

blobs = CompositeDistribution([
        HyperSphere(center=(-0.2,0,0), radius=0.8),
        HyperSphere(center=(0.2,0.7,-0.6), radius=0.1),
        HyperSphere(center=(0,0.5,-0.6), radius=0.6),
        HyperSphere(center=(0,0.5,0), radius=0.1),
        HyperSphere(center=(0,0.5), radius=0.4),
        ])

clover = CompositeDistribution([
        HyperSphere(center=(0,0,-0.5), radius=0.3),
        HyperSphere(center=(0,0,0), radius=0.3),
        HyperSphere(center=(0,0,0.5), radius=0.3),
        HyperSphere(center=(0,-0.5,0), radius=0.3),
        HyperSphere(center=(0,0.5,0), radius=0.3),
        HyperSphere(center=(0.7,0,0), radius=0.1),
        HyperSphere(center=(0,0,0), radius=0.8),
        ])

latt = DiscreteLattice(range(-5,9), range(7,10), range(-10,0))

mixed = CompositeDistribution([
        DiscreteLattice(range(-5,9), range(7,10), range(-10,0)),
        HyperSphere(center=(0,0,0), radius=3),
        ])

planes = CompositeDistribution([
        BoundedVolume([-1, 1], [-1, 1]),
        BoundedVolume([0, 0], [-1, 1], [-1, 1]),
        BoundedVolume([-1, 1], [0, 0], [-1, 1]),
        ])

dplanes = CompositeDistribution([
        DiscreteLattice(range(-10, 11), range(-10, 11), [0]),
        DiscreteLattice([0], range(-10, 11), range(-10, 11)),
        DiscreteLattice(range(-10, 11), [0], range(-10, 11)),
        ])