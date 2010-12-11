/***********************************************************************
**
**   optimization.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by Christof Bodner
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   The optimization procedure is developped by
**   Oswin Aichholzer (oaich@igi.tu-graz.ac.at) and implemented by
**   Ch. Bodner (christof.bodner@gmx.net)
**   It is based upon the principle of dynamic programming.
**   The time consumption is O(n^2) and the memory consumption O(n),
**   if n is the number of points in the route
**
**   $Id$
**
***********************************************************************/

#include <cstdlib>

#include <QApplication>
#include <QMessageBox>

#include "optimization.h"

// different weight for last two legs
double Optimization::weight(unsigned int k){
  switch (k){
    case LEGS:
      return 0.6;
    case LEGS-1:
      return 0.8;
    default:
      return 1.0;
  }
}

Optimization::Optimization(unsigned int /*firstPoint*/, unsigned int /*lastPoint*/, QList<flightPoint*> ptr_route, QProgressBar *progressBar){
  original_route = ptr_route;
  setTimes(0,original_route.count());
  optimized=false;
  progress = progressBar;
  stopit=false;
  start=0;
  stop=original_route.count();
}

Optimization::~Optimization(){
}

double Optimization::optimizationResult(unsigned int* retList, double *retPoints){
  if (!optimized)
    return -1.0;
  int i=0;
  int j=0;
  retList[i++]=start;
  for(;i<=LEGS+1;i++){
    retList[i]=original_route.indexOf(route.at(pointList[j]));
    if (pointList[j]>original_route.count()){
      qWarning("##k:%d\tstart:%d\t\tpointList[k]:%d", i, start, pointList[i]);
      QMessageBox::warning(0, "Optimization fault", "Sorry optimization fault. Report error (including IGC-File) to <christof.bodner@gmx.net>", QMessageBox::Ok, 0);
      return -1.0;
    }
    j++;
  }
  retList[i]=stop;
  *retPoints=points;
  return distance;
}

void Optimization::setTimes(unsigned int start_int, unsigned int stop_int){
  start=start_int;
  stop=stop_int;
  // delete actual route
  while ( route.count() != 0 ) {
    route.removeLast();
  }
  qWarning("Items in list:%d", original_route.count());
  // construct route
  for ( unsigned int i=start_int; i<=stop_int; i++)
    route.append(original_route.at(i));
  qWarning("Number of points for optimization:%d", route.count());
}

void Optimization::stopRun(){
  stopit=true;
}
void Optimization::enableRun(){
  stopit=false;
}

void Optimization::run(){
  double *L;                        // length values
  unsigned int *w;                  // waypoints
//  double length;                    // solution length

  int ii;                           // loop variables
  unsigned int n;                   // number of points
  double c;                         // temp variables
  unsigned int index;
  double wLeg;
  flightPoint **rp;
  
  n=route.count()+1;
  qWarning("Number of points to optimize: %d", n);
  if(progress){
    progress->setMinimumWidth(progress->sizeHint().width() + 45);
    progress->setRange(0, 7*n);
    progress->setValue(0);
  }
  
  // allocate memory
  L=(double *) malloc((n+1)*(LEGS+1)*sizeof(double));
  w=(unsigned int *) malloc((n+1)*(LEGS+1)*sizeof(unsigned int));
  rp = (flightPoint **) malloc(route.count() * sizeof(flightPoint *));
  
  Q_CHECK_PTR(L);
  Q_CHECK_PTR(w);
  Q_CHECK_PTR(rp);

  for(int i=0;i<=n-1;i++){
    L[i+0*n]=0;
  }
  
  for(int i = 0; i < route.count(); i++){
    rp[i] = route.at(i);
  }
  
  for(int k=1;k<=LEGS;k++){
    ii = (k-1)*n;
    wLeg = weight(k);
    
    for(int i=0;i<n-1;i++){
      qApp->processEvents();
      if (stopit){
          free(L);
          free(w);
          free(rp);
          progress->setValue(0);
          optimized=false;
          return;
      }
      index=i+k*n;

      if(progress) {
        progress->setValue(index);
      }
      
      L[index]=0;
      c=0;
      for(int j=0;j<i;j++){
        c=L[j+ii]+wLeg*dist(rp[j], rp[i]);
        if (c>L[index]){
          L[index]=c;
          w[index]=j;
        }
      }
    }
  }

  // find maximal length i.e. points
  points=0;
  for(int i=0;i<n-1;i++){
    if(L[i+LEGS*n]>points){
      points=L[i+LEGS*n];
      pointList[LEGS]=i;
    }
  }

  // find waypoints
  for(int k=LEGS-1;k>=0;k--){
      qWarning("  k:%d\tpointList[k+1]:%d", k, (int)pointList[k+1]);
      pointList[k]=w[pointList[k+1]+(k+1)*n];
      qWarning("->k:%d\tpointList[k]:%d", k, (int)pointList[k]);
  }

  distance=dist(route.at(pointList[0]),route.at(pointList[1]))+
    dist(route.at(pointList[1]),route.at(pointList[2]))+
    dist(route.at(pointList[2]),route.at(pointList[3]))+
    dist(route.at(pointList[3]),route.at(pointList[4]))+
    dist(route.at(pointList[4]),route.at(pointList[5]))+
    dist(route.at(pointList[5]),route.at(pointList[6]));
  qWarning("Distance:%f\nPoints:%f", distance, points);

    // free memory
    free(L);
    free(w);
    free(rp);

    if(progress)
      progress->setValue(0);
    optimized=true;
}
