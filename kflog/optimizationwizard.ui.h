/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void OptimizationWizard::init()
{
  evaluation =  new EvaluationDialog(this,false);
  setWFlags(getWFlags() | WStyle_StaysOnTop); // doesn't do the trick. Evaluation Dlg is on top
  flight=(Flight*)_globalMapContents.getFlight();
  route=flight->getRoute();
  optimization = new Optimization(0, route.count(), route,progress);
  slotSetTimes();
  QString text = i18n("The task has not been optimized for the OLC, yet");
  kTextBrowser1->setText(text);
}

void OptimizationWizard::slotStartOptimization()
{
  optimization->enableRun();
  btnStart->setEnabled(false);
  btnStop->setEnabled(true);

  optimization->run();

  unsigned int idList[7];
  double points;
  double distance = optimizationResult(idList,&points);

  if (distance<0.0)  // optimization was canceled
    return;

  setFinishEnabled ( page_3, true );
  btnStop->setEnabled(false);
  btnStart->setEnabled(true);

  QString text, distText, rawPointText;
  rawPointText.sprintf(" %.2f", points);
  distText.sprintf(" %.2f km  ", distance);
  text = i18n("The task has been optimized for the OLC.\nThe best task found is:\n\n");
  text = text + "\tBOT:  "
      + printTime(route.at(idList[0])->time,true) + " : \t"
      + printPos(route.at(idList[0])->origP.lat()) + " / "
      + printPos(route.at(idList[0])->origP.lon(), false)
      + QString("\n\t1.TP:  ")
      + printTime(route.at(idList[1])->time,true) + " : \t"
      + printPos(route.at(idList[1])->origP.lat()) + " / "
      + printPos(route.at(idList[1])->origP.lon(), false)
      + QString(" (%1km)\n\t2.TP:  ").arg(dist(route.at(idList[0]),route.at(idList[1])),0,'f',2)
      + printTime(route.at(idList[2])->time,true) + " : \t"
      + printPos(route.at(idList[2])->origP.lat()) + " / "
      + printPos(route.at(idList[2])->origP.lon(), false)
      + QString(" (%1km)\n\t3.TP:  ").arg(dist(route.at(idList[1]),route.at(idList[2])),0,'f',2)
      + printTime(route.at(idList[3])->time,true) + " : \t"
      + printPos(route.at(idList[3])->origP.lat()) + " / "
      + printPos(route.at(idList[3])->origP.lon(), false)
      + QString(" (%1km)\n\t4.TP:  ").arg(dist(route.at(idList[2]),route.at(idList[3])),0,'f',2)
      + printTime(route.at(idList[4])->time,true) + " : \t"
      + printPos(route.at(idList[4])->origP.lat()) + " / "
      + printPos(route.at(idList[4])->origP.lon(), false)
      + QString(" (%1km)\n\t5.TP:  ").arg(dist(route.at(idList[3]),route.at(idList[4])),0,'f',2)
      + printTime(route.at(idList[5])->time,true) + " : \t"
      + printPos(route.at(idList[5])->origP.lat()) + " / "
      + printPos(route.at(idList[5])->origP.lon(), false)
      + QString(" (%1km)\n\tEOT:  ").arg(dist(route.at(idList[4]),route.at(idList[5])),0,'f',2)
      + printTime(route.at(idList[6])->time,true) + " : \t"
      + printPos(route.at(idList[6])->origP.lat()) + " / "
      + printPos(route.at(idList[6])->origP.lon(), false)
      + QString(" (%1km):  ").arg(dist(route.at(idList[5]),route.at(idList[6])),0,'f',2)
      + i18n("\n\nDistance:\t") + distText + "\n" + i18n("Points(raw):\t") + rawPointText + "\n\n"
      + i18n("If you want to use this task and replace the old, press \"Finish\",\nelse press \"Cancel\"");
  kTextBrowser1->setText(text);
  next();
}

void OptimizationWizard::slotStopOptimization()
{
    btnStart->setEnabled(true);
    btnStop->setEnabled(false);
    optimization->stopRun();
}

void OptimizationWizard::slotSetTimes()
{
  if (!evaluation)
    return;
  unsigned int start=evaluation->getTaskStart();
  unsigned int stop=evaluation->getTaskEnd();
  flightPoint startPoint=flight->getPoint(start);
  flightPoint endPoint=flight->getPoint(stop);
  lblStartTime->setText(QString("%1").arg(printTime(startPoint.time,true)));
  lblStopTime->setText(QString("%1").arg(printTime(endPoint.time,true)));
  lblDiffTime->setText(QString("%1").arg(printTime(endPoint.time-startPoint.time,true)));
  lblStartHeight->setText(QString("%1m").arg(startPoint.height));
  lblStopHeight->setText(QString("%1m").arg(endPoint.height));
  lblDiffHeight->setText(QString("%1m").arg(endPoint.height-startPoint.height));
  if (endPoint.height-startPoint.height<-1000)
    lblDiffHeight->setPaletteForegroundColor(QColor(255,0,0));
  else
    lblDiffHeight->setPaletteForegroundColor(QColor(0,128,0));
  optimization->setTimes(start,stop);
}

void OptimizationWizard::setMapContents( Map* _map )
{
    extern MapContents _globalMapContents;
    connect(&_globalMapContents, SIGNAL(currentFlightChanged()), evaluation,
        SLOT(slotShowFlightData()));
    connect(evaluation, SIGNAL(showCursor(QPoint, QPoint)), _map,
        SLOT(slotDrawCursor(QPoint, QPoint)));
}

double OptimizationWizard::optimizationResult( unsigned int pointList[LEGS+1], double * points )
{
    return optimization->optimizationResult( pointList,points );
}

