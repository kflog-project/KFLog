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
#include "wp.h"
#include "mapcalc.h"

/**This class implements a thread which optimizes a task
  *@author Christof Bodner
  */

#define LEGS 6  // number of legs
  
class Optimization : public QObject, public QThread  {
  Q_OBJECT
public:
/**
  Constructor for the route with the first resp. last point allowed
  */
  Optimization(unsigned int firstPoint, unsigned int lastPoint, QList<flightPoint> route);
  ~Optimization();
/**
  returns the indizes, the points and the distance of the optimized task
  */
  double optimizationResult(unsigned int pointList[LEGS+1],double *points);
  virtual void run();
private:
  double weight(unsigned int k); // different weight for the legs
  QList<flightPoint> route;
  double distance,points;
  unsigned int pointList[LEGS+1];
  unsigned int start;    // first
  unsigned int stop;     // last valid point
  bool  optimized;
};

#endif
