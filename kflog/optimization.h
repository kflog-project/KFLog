/***********************************************************************
**
**   optimization.h
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

#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include <qthread.h>
#include <qlist.h>
#include <qvaluelist.h>
#include <kprogress.h>
#include "wp.h"
#include "mapcalc.h"

/**This class optimizes a task according to the OLC 2003 rules
  *@author Christof Bodner
  */

#define LEGS 6  // number of legs
  
class Optimization : public QObject{
  Q_OBJECT
public:
/**
  Constructor for the route with the first resp. last point allowed
  an optional progress bar can be specified
  */
  Optimization(unsigned int firstPoint, unsigned int lastPoint, QList<flightPoint> route,KProgress* progressBar=0);
  ~Optimization();
/**
  returns the indizes, the points and the distance of the optimized task
  */
  double optimizationResult(unsigned int pointList[LEGS+3],double *points);
public slots:
  void run();
  void setTimes(unsigned int start_int, unsigned int stop_int);
  void stopRun();
  void enableRun();
private:
  double weight(unsigned int k); // different weight for the legs
  QPtrList<flightPoint> original_route;
  QPtrList<flightPoint> route;
  double distance,points;
  unsigned int pointList[LEGS+1];   // solution points
  unsigned int start;    // first
  unsigned int stop;     // last valid point
  bool  optimized;
  bool  stopit;
  KProgress* progress;
};

#endif
