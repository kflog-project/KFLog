/***********************************************************************
**
**   optimization.cpp
**
**   This file is part of KFLog2.
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
**
**   $Id$
**
***********************************************************************/

#include "optimization.h"
#include <klocale.h>
#include <kmessagebox.h>
#include <qprogressdialog.h>
#include <stdlib.h>

#define PROGESSDLG

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

Optimization::Optimization(unsigned int firstPoint, unsigned int lastPoint, QList<flightPoint> ptr_route){
  route = ptr_route;
  start = firstPoint;
  stop  = lastPoint;
  optimized=false;
}

Optimization::~Optimization(){
}

double Optimization::optimizationResult(unsigned int retList[LEGS+1], double *retPoints){
  if (!optimized)
    return -1.0;
  for (int i=0;i<=LEGS;i++)
    retList[i]=pointList[i];
  *retPoints=points;
  return distance;
}

void Optimization::run(){

  double *L;                        // length values
  unsigned int *w;                           // waypoints
  double length;                    // solution length

  unsigned int i,j,k;               // loop variables
  unsigned int n;                   // number of points
  double c;                         // temp variable

  n=(stop-start)+1;

#ifdef PROGESSDLG
  QProgressDialog importProgress(0,0,true);

  importProgress.setCaption(i18n("Optimizing flight ..."));
  importProgress.setLabelText(
      i18n("Please wait while optimizing flight for OLC"));
  importProgress.setMinimumWidth(importProgress.sizeHint().width() + 45);
  importProgress.setTotalSteps(7*n);
  importProgress.setMinimumDuration(0);
  importProgress.setProgress(0);
  importProgress.show();
  if (importProgress.wasCancelled()) return;
#endif
  
  // allocate memory
  L=(double *) malloc((n+1)*(LEGS+1)*sizeof(double));
  w=(unsigned int *) malloc((n+1)*(LEGS+1)*sizeof(unsigned int));
  
  for (i=0;i<=n-1;i++){
    L[i+0*n]=0;
  }
  for (k=1;k<=LEGS;k++){
    for (i=0;i<n-1;i++){
#ifdef PROGESSDLG
      if (importProgress.wasCancelled()) return;
      importProgress.setProgress(i+k*n);
#endif
      L[i+k*n]=0;
      c=0;
      for (j=0;j<i;j++){
        c=L[j+(k-1)*n]+weight(k)*dist(route.at(j+start),route.at(i+start));
        if (c>L[i+k*n]){
          L[i+k*n]=c;
          w[i+k*n]=j;
        }
      }
    }
  }
#ifdef PROGESSDLG
  importProgress.close();
#endif

  // find maximal length
  points=0;
  for (i=0;i<n-1;i++){
    if(L[i+LEGS*n]>points){
      points=L[i+LEGS*n];
      pointList[LEGS]=i;
    }
  }

  // find waypoints
  for (long k=LEGS-1;k>=0;k--){
      qWarning(QString("  k:%1\tstart:%2\t\tpointList[k+1]:%3").arg(k).arg(start).arg(pointList[k+1]));
//      pointList[k]=w[pointList[k+1]+k*n];  // if this was the bug, we can delete this line
      pointList[k]=w[pointList[k+1]+(k+1)*n];
      qWarning(QString("->k:%1\tstart:%2\t\tpointList[k]:%3").arg(k).arg(start).arg(pointList[k]));
  }
  // correct waypoints
  for (long k=LEGS;k>=0;k--){
      pointList[k]+=start;
      if (pointList[k]<start || pointList[k]>stop){
        qWarning(QString("##k:%1\tstart:%2\t\tpointList[k]:%3").arg(k).arg(start).arg(pointList[k]));
        KMessageBox::error(0,"Sorry optimization fault. Report error (including IGC-File) to <christof.bodner@gmx.net>");
        return;
      }
        
  }

  distance=dist(route.at(pointList[0]),route.at(pointList[1]))+
    dist(route.at(pointList[1]),route.at(pointList[2]))+
    dist(route.at(pointList[2]),route.at(pointList[3]))+
    dist(route.at(pointList[3]),route.at(pointList[4]))+
    dist(route.at(pointList[4]),route.at(pointList[5]))+
    dist(route.at(pointList[5]),route.at(pointList[6]));

    // free memory
    free(L);
    free(w);

    optimized=true;
}
