/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <klocale.h>

void OptimizationWizard::init()
{
//  next two lines are a hack to fix initial evaluation widget size bug
  showMaximized();
  showNormal();

  resize(640,520);
  
  flight=(Flight*)_globalMapContents.getFlight();
  route=flight->getRoute();
  optimization = new Optimization(0, route.count(), route,progress);
  slotSetTimes();
  QString text = "<DIV ALIGN=CENTER>";
  text += i18n("The task has not been optimized for the OLC, yet") +
      "<br>" + i18n("Press \"Start Optimization\" to begin");
  text += "</DIV>";
  kTextBrowser1->setText(text);
}

void OptimizationWizard::slotStartOptimization()
{
  optimization->enableRun();
  btnStart->setEnabled(false);
  btnStop->setEnabled(true);

  optimization->run();

  unsigned int idList[LEGS+3];
  double points;
  double distance = optimizationResult(idList,&points);

  if (distance<0.0)  // optimization was canceled
    return;

  setFinishEnabled ( page_2, true );
  btnStop->setEnabled(false);
  btnStart->setEnabled(true);

  QString text, distText, rawPointText;
  rawPointText.sprintf(" %.2f", points);
  distText.sprintf(" %.2f km  ", distance);
//  text = i18n("The task has been optimized for the OLC.\nThe best task found is:\n\n");
//  text = text + "\tBOT:  "
//      + printTime(route.at(idList[0])->time,true) + " : \t"
//      + printPos(route.at(idList[0])->origP.lat()) + " / "
//      + printPos(route.at(idList[0])->origP.lon(), false)
//      + QString("\n\t1.TP:  ")
//      + printTime(route.at(idList[1])->time,true) + " : \t"
//      + printPos(route.at(idList[1])->origP.lat()) + " / "
//      + printPos(route.at(idList[1])->origP.lon(), false)
//      + QString(" (%1km)\n\t2.TP:  ").arg(dist(route.at(idList[0]),route.at(idList[1])),0,'f',2)
//      + printTime(route.at(idList[2])->time,true) + " : \t"
//      + printPos(route.at(idList[2])->origP.lat()) + " / "
//      + printPos(route.at(idList[2])->origP.lon(), false)
//      + QString(" (%1km)\n\t3.TP:  ").arg(dist(route.at(idList[1]),route.at(idList[2])),0,'f',2)
//      + printTime(route.at(idList[3])->time,true) + " : \t"
//      + printPos(route.at(idList[3])->origP.lat()) + " / "
//      + printPos(route.at(idList[3])->origP.lon(), false)
//      + QString(" (%1km)\n\t4.TP:  ").arg(dist(route.at(idList[2]),route.at(idList[3])),0,'f',2)
//      + printTime(route.at(idList[4])->time,true) + " : \t"
//      + printPos(route.at(idList[4])->origP.lat()) + " / "
//      + printPos(route.at(idList[4])->origP.lon(), false)
//      + QString(" (%1km)\n\t5.TP:  ").arg(dist(route.at(idList[3]),route.at(idList[4])),0,'f',2)
//      + printTime(route.at(idList[5])->time,true) + " : \t"
//      + printPos(route.at(idList[5])->origP.lat()) + " / "
//      + printPos(route.at(idList[5])->origP.lon(), false)
//      + QString(" (%1km)\n\tEOT:  ").arg(dist(route.at(idList[4]),route.at(idList[5])),0,'f',2)
//      + printTime(route.at(idList[6])->time,true) + " : \t"
//      + printPos(route.at(idList[6])->origP.lat()) + " / "
//      + printPos(route.at(idList[6])->origP.lon(), false)
//      + QString(" (%1km):  ").arg(dist(route.at(idList[5]),route.at(idList[6])),0,'f',2)
//      + i18n("\n\nDistance:\t") + distText + "\n" + i18n("Points(raw):\t") + rawPointText + "\n\n"
//      + i18n("If you want to use this task and replace the old, press \"Finish\",\nelse press \"Cancel\"");
  text = "<div align=\"center\"<caption><b>";
  text += i18n("Optimization Result")+"</b></div><table align=\"center\"><thead><tr>";
  text += i18n("<th scope=col></th><th scope=col>Time</th><th scope=col>Latitude</th><th scope=col>Longitude</th><th ALIGN=right scope=col>Distance</th></tr>");
  text += i18n("</thead><tbody>");

  text += i18n("<tr><td>Begin of Soaring</td><td>")
      + printTime(route.at(idList[0])->time,true) + "</td><td>"
      + printPos(route.at(idList[0])->origP.lat()) + "</td><td>"
      + printPos(route.at(idList[0])->origP.lon(), false) + "</td><td></td></tr>";
  text += i18n("<tr><td>Begin of Task</td><td>")
      + printTime(route.at(idList[1])->time,true) + "</td><td>"
      + printPos(route.at(idList[1])->origP.lat()) + "</td><td>"
      + printPos(route.at(idList[1])->origP.lon(), false) + "</td><td></td></tr>";
  text += i18n("<tr><td>1.Turnpoint</td><td>")
      + printTime(route.at(idList[2])->time,true) + "</td><td>"
      + printPos(route.at(idList[2])->origP.lat()) + "</td><td>"
      + printPos(route.at(idList[2])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[1]),route.at(idList[2])),0,'f',2);
  text += i18n("<tr><td>2.Turnpoint</td><td>")
      + printTime(route.at(idList[3])->time,true) + "</td><td>"
      + printPos(route.at(idList[3])->origP.lat()) + "</td><td>"
      + printPos(route.at(idList[3])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[2]),route.at(idList[3])),0,'f',2);
  text += i18n("<tr><td>3.Turnpoint</td><td>")
      + printTime(route.at(idList[4])->time,true) + "</td><td>"
      + printPos(route.at(idList[4])->origP.lat()) + "</td><td>"
      + printPos(route.at(idList[4])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[3]),route.at(idList[4])),0,'f',2);
  text += i18n("<tr><td>4.Turnpoint</td><td>")
      + printTime(route.at(idList[5])->time,true) + "</td><td>"
      + printPos(route.at(idList[5])->origP.lat()) + "</td><td>"
      + printPos(route.at(idList[5])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[4]),route.at(idList[5])),0,'f',2);
  text += i18n("<tr><td>5.Turnpoint</td><td>")
      + printTime(route.at(idList[6])->time,true) + "</td><td>"
      + printPos(route.at(idList[6])->origP.lat()) + "</td><td>"
      + printPos(route.at(idList[6])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[5]),route.at(idList[6])),0,'f',2);
  text += i18n("<tr><td>End of Task</td><td>")
      + printTime(route.at(idList[7])->time,true) + "</td><td>"
      + printPos(route.at(idList[7])->origP.lat()) + "</td><td>"
      + printPos(route.at(idList[7])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[6]),route.at(idList[7])),0,'f',2);
  text += i18n("<tr><td>End of Soaring</td><td>")
      + printTime(route.at(idList[8])->time,true) + "</td><td>"
      + printPos(route.at(idList[8])->origP.lat()) + "</td><td>"
      + printPos(route.at(idList[8])->origP.lon(), false) + "</td><td></td></tr>";
  text += "</tbody></table><th>";

  text += "<table align=\"center\">";
  text += "<tr><td><b>" + i18n("Summary: Distance:");
  text += distText + i18n(" Points(raw):");
  text += rawPointText+"</b></td></tr>";
  text += "</table>";

  int heightDiff = route.at(idList[8])->height - route.at(idList[0])->height;
  if (heightDiff<-1000)
    text += "<div align=\"center\"><b>" + i18n("WARNING! THE HEIGHT DIFFERENCE IS TOO LARGE!<br>This is not a valid OLC task. Go back and correct the start and/or end times.")+"</b></div>";
  
  text += "<div align=\"center\">"+i18n("If you want to use this task and replace the old, press \"Finish\", else press \"Cancel\"")+"</div>";

  kTextBrowser1->setText(text);
//  next();
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

double OptimizationWizard::optimizationResult( unsigned int* pointList, double * points )
{
    return optimization->optimizationResult( pointList,points );
}

