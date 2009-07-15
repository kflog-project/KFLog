#! /usr/bin/env python

def write_grassfile(file):
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

    outfile.close()

for LON in range(-179,181,2):     # latitude from -179 to 179
    for LAT in range(-55,61,2):   # longitude from -55 to 59
        for TYPE in ['T', 'G']:
            write_grassfile('%s_%05d' %(TYPE, 180*int((LAT-90)/-2) + int((LON+180)/2)))
