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

class Optimization : public QObject, public QThread  {
  Q_OBJECT
public:
  Optimization(QList<flightPoint> route);
  ~Optimization();
/**
  returns the indizes, the points and the distance of the optimized task
  */
  double optimizationResult(unsigned int idList[7],double *points);
  virtual void run();
private:
double __CalculateOLCPoints(unsigned int start,unsigned int stop,unsigned int idList[7]);
  QList<flightPoint> route;
  double distance,points;
  unsigned int idList[7];
};

#endif
