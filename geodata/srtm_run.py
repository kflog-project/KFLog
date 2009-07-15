#! /usr/bin/env python

for LAT in range(-58,62,10):
  for LON in range(-178,182,10):
    S_BBOX=LAT
    N_BBOX=LAT+10
    W_BBOX=LON
    E_BBOX=LON+10

    # SRTM is only available between 56S and 60N:
    if (S_BBOX==-58): S_BBOX=-56
    if (N_BBOX==62):  N_BBOX=60

    # We have to wrap around at 180W/180E:
    if (E_BBOX==182): E_BBOX=-178

    # Call the script that does the magic:
    print './srtm_template.grass %d %d %d %d' %(S_BBOX, N_BBOX, W_BBOX, E_BBOX)

