#! /usr/bin/env python
# coding=latin1

# ATTENTION: Coordinates are specified as x,y or lon,lat

import os,sys
from math import sqrt
import Polygon

class Coordinates:
    def __init__(self, lon, lat):
        self.p = (lon,lat)
    def __repr__(self):
        return '  %s  %s' %(self.p[0],self.p[1])
    def __eq__(self,other):
        if (self.p[0]==other.p[0] and self.p[1]==other.p[1]):
            return True
        else:
            return False
    def __ne__(self,other):
        return not self.__eq__(other)
    def lon(self):
        return self.p[0]
    def lat(self):
        return self.p[1]
    def set_lon(self,lon):
        self.p[0] = lon
    def set_lat(self,lat):
        self.p[1] = lat

class Isoline:
    def __init__(self,elevation):
        self.points = []
        self.elevation = elevation
    def __getitem__(self,i):
        return self.points[i]
    def __delitem__(self,i):
        del self.points[i]
    def __len__(self):
        return len(self.points)
    def __repr__(self):
        ret = ''
        for i in self.points:
            ret += '%s   %s\n' %(i,self.elevation)
        return ret.strip('\n')
    def clear(self):
        self.points = []
    def isclosed(self):
        if (len(self) > 2 and self.points[0]==self.points[-1]):
            return True
        else:
            return False
    def append(self,x):
        if (x.__class__.__name__=='Coordinates'):
            self.points.append(x)
        elif (x.__class__.__name__=='Isoline'):
            self.points += x.points
    def prepend(self,x):
        if (x.__class__.__name__=='Coordinates'):
            self.points.insert(0,x)
        elif (x.__class__.__name__=='Isoline'):
            self.points = x.points + self.points
    def reverse(self):
        return self.points.reverse()
    def remove_duplicates(self):
        removallist = []
        for i in range(len(self)-1):
            if (self[i]==self[i+1]):
                removallist.insert(0,i)
        for i in removallist:
            del self[i]
    def as_polygon(self):
        pointlist = []
        for i in self.points:
            pointlist.append((i.lon(), i.lat()))
        return Polygon.Polygon(pointlist)

def write_grassfile(file,isolines,order):
    outfile = open('%s' %(file.split('/')[-1]), 'w')

    outfile.write('ORGANIZATION: \n')
    outfile.write('DIGIT DATE:   \n')
    outfile.write('DIGIT NAME:   hoeth\n')
    outfile.write('MAP NAME:     \n')
    outfile.write('MAP DATE:     Thu Jan 01 00:00:00 2009\n')
    outfile.write('MAP SCALE:    1\n')
    outfile.write('OTHER INFO:   \n')
    outfile.write('ZONE:         0\n')
    outfile.write('MAP THRESH:   0.000000\n')
    outfile.write('VERTI:\n')
    for i in order:
        outfile.write('L  %d 1\n' %len(isolines[i]))
        outfile.write('%s\n' %(isolines[i]))
        outfile.write(' 1      1\n')
        
    outfile.close()

def read_grassfile(file):
    isolines = []
    infile = open('%s' %file, 'r')
    lines = infile.readlines()
    infile.close()

    # reverse ordering to increase speed (pop(0) is sloooooow!)
    lines.reverse()
    for i in range(10):
        lines.pop()

    while (len(lines) > 0):
        loclength = int(lines.pop().split()[1])
        for i in range(loclength):
            lon,lat,elev = lines.pop().split()
            if (i==0):
                isoline = Isoline(int(elev))
            isoline.append(Coordinates(float(lon),float(lat)))
        lines.pop()
        isolines.append(isoline)
        if (len(isolines)%200 == 0):
            print "   ... %d isolines read" %len(isolines)
    return isolines


file = sys.argv[1]

print "reading isolines ..."
isolines = read_grassfile(file)
print "finished reading %d isolines" %len(isolines)

print "creating polygons"
polygons = []
for i,isoline in enumerate(isolines):
    polygons.append(isoline.as_polygon())

print "%d polygons created" % len(polygons)

print "calculating dependencies ..."
deps = {}
independent = []
for i,isoline in enumerate(isolines):
    deps[i] = []
    if (i%200 == 0 and i!=0):
        print "   ... %d polygons processed" %i
    for j in range(len(isolines)):
        if (i==j): continue
        # If i is inside j, then i depends on j
        if polygons[j].isInside(isoline[0].lon(),isoline[0].lat()):
            # If j also claims to be inside i, the smaller one wins.
            # This can happen if they share the first point.
            if polygons[i].isInside(isolines[j][0].lon(),isolines[j][0].lat()):
                if (polygons[j].area() > polygons[i].area()):
                    deps[i].append(j)
            else:
                deps[i].append(j)
    if (len(deps[i])==0):
        independent.append(i)
        del deps[i]
print "finished calculating dependencies"

print "solving the dependency tree"
sorted = []
while (len(independent)>0):
    line = independent.pop()
    sorted.append(line)
    for i in deps.keys():
        if (deps[i].count(line)>0):
            deps[i].remove(line)
        if (len(deps[i])==0):
            independent.append(i)
            del deps[i]

print "%d sorted isolines will be written" % len(sorted)
if (len(deps.keys())!=0):
    print "ERROR: %d lines still have dependencies" % len(deps.keys())

write_grassfile(file, isolines, sorted)


