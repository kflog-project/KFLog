#! /usr/bin/env python

import sys

TILE = int(sys.argv[1])

print "LAT =", 89-((int(TILE)/180)*2)
print "LON =", ((TILE%180)*2)-179
