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

/**
  * This class optimizes a task according to the OLC 2003 rules
  * @author Christof Bodner
  * @version $Id$
  */

#define LEGS 6  // number of legs
  
class Optimization : public QObject{
  Q_OBJECT
public:
 /**
  * Constructor for the route with the first resp. last point allowed
  * an optional progress bar can be specified, since optimizing can take
  * a long time.
  *
  * @param firstPoint Index of first point in the @ref route ?
  * @param lastPoint Index of last point in the @ref route ?
  * @param route List of flightpoints that together consitute the route this flight used.
  * @param progressBar optional reference to a progressbar to indicate optimisation progress.
  */
  Optimization(unsigned int firstPoint, unsigned int lastPoint, QList<flightPoint> route,KProgress* progressBar=0);
 /**
  * Destructor
  */ 
  ~Optimization();
 /**
  * @return the indices, the points awarded and the distance of the optimized task
  */
  double optimizationResult(unsigned int* pointList,double *points);  //where is the distance returned?
public slots:
 /**
  * Starts optimisation of the given route
  */
  void run();
 /**
  * ?
  */
  void setTimes(unsigned int start_int, unsigned int stop_int);
 /**
  * Cancels optimisation
  */ 
  void stopRun();
 /**
  * ?
  */ 
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
