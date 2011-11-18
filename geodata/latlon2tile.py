#! /usr/bin/env python

import sys

LAT = int(sys.argv[1])
LON = int(sys.argv[2])

print "LAT=%s"  %LAT
print "LON=%s"  %LON
print "TILE=%s" %(180*int((LAT-90)/-2) + int((LON+180)/2))
