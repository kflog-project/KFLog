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
**   $Id$
**
***********************************************************************/

#include "optimization.h"
#include <klocale.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>

Optimization::Optimization(QList<flightPoint> ptr_route){
  route = ptr_route;
}

Optimization::~Optimization(){
}

double Optimization::optimizationResult(unsigned int retList[7], double *retPoints){
  for (int i=0;i<6;i++)
    retList[i]=idList[i];
  *retPoints=points;
  return distance;
}

void Optimization::run(){

  unsigned int start=0;
  unsigned int stop=route.count();

  unsigned int delta = (unsigned int)(stop/8);
  for (int i=0;i<7;i++)
    idList[i]=(i+1)*delta;
  
  points=__CalculateOLCPoints(start,stop,idList);

  distance=dist(route.at(idList[0]),route.at(idList[1]))+
    dist(route.at(idList[1]),route.at(idList[2]))+
    dist(route.at(idList[2]),route.at(idList[3]))+
    dist(route.at(idList[3]),route.at(idList[4]))+
    dist(route.at(idList[4]),route.at(idList[5]))+
    dist(route.at(idList[5]),route.at(idList[6]));

}

double Optimization::__CalculateOLCPoints(unsigned int start,unsigned int stop,unsigned int idList[7]){
  if (idList[0]<start || idList[1]<start || idList[2]<start || idList[3]<start || idList[4]<start || idList[5]<start || idList[6]<start ||
      idList[0]<start || idList[1]>=stop || idList[2]>=stop || idList[3]>=stop || idList[4]>=stop || idList[5]>=stop || idList[6]>=stop ||
      idList[1]<idList[0] || idList[2]<idList[1] ||idList[3]<idList[2] || idList[4]<idList[3] || idList[5]<idList[4] || idList[6]<idList[5] )
    return 0.0;

  return dist(route.at(idList[0]),route.at(idList[1]))+
   dist(route.at(idList[1]),route.at(idList[2]))+
   dist(route.at(idList[2]),route.at(idList[3]))+
   dist(route.at(idList[3]),route.at(idList[4]))+
   dist(route.at(idList[4]),route.at(idList[5]))*0.8+
   dist(route.at(idList[5]),route.at(idList[6]))*0.6;
}
