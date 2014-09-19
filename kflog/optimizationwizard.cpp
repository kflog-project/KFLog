/***********************************************************************
**
**   optimizationwizard.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
** Created: Sam Mär 8 12:18:37 2003
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
****************************************************************************/

#ifdef QT_5
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include "mainwindow.h"
#include "optimizationwizard.h"
#include "wgspoint.h"

extern MapContents* _globalMapContents;
extern MainWindow*  _mainWindow;

/*
 *  Constructs a OptimizationWizard as a child of 'parent'.
 *
 *  The wizard will by default be mode less, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
OptimizationWizard::OptimizationWizard( QWidget* parent ) :
  QWizard( parent )
{
  setObjectName("OptimizationWizard");
  setWindowTitle( tr( "OLC Optimization" ) );
  setModal( false );
  setSizeGripEnabled( true );
  setWizardStyle( QWizard::ModernStyle );
  setPixmap( QWizard::LogoPixmap, _mainWindow->getPixmap( "task_48.png" ) );

  page = new QWizardPage( this );
  page->setTitle( tr( "Task start and end times" ) );
  page->setSubTitle( tr( "Define start and end point of flight by using the mouse buttons." ) );
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  QVBoxLayout* pageLayout_1 = new QVBoxLayout( page );
  pageLayout_1->setMargin(10);
  pageLayout_1->setSpacing(5);

  evaluationDialog = new EvaluationDialog;
  pageLayout_1->addWidget( evaluationDialog );

  QGridLayout* groupBoxLayout = new QGridLayout;

  lblStartHeight = new QLabel;
  lblStartHeight->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

  groupBoxLayout->addWidget( lblStartHeight, 0, 2 );

  lblStopTime = new QLabel;
  lblStopTime->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

  groupBoxLayout->addWidget( lblStopTime, 1, 1 );

  lblDiffHeight = new QLabel;
  lblDiffHeight->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

  groupBoxLayout->addWidget( lblDiffHeight, 2, 2 );

  textLabel1_3_2_2 = new QLabel;

  groupBoxLayout->addWidget( textLabel1_3_2_2, 2, 0 );

  lblStartTime = new QLabel;
  lblStartTime->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

  groupBoxLayout->addWidget( lblStartTime, 0, 1 );

  textLabel1_4_2 = new QLabel;

  groupBoxLayout->addWidget( textLabel1_4_2, 0, 0 );

  textLabel1_2_2_2 = new QLabel;

  groupBoxLayout->addWidget( textLabel1_2_2_2, 1, 0 );

  lblStopHeight = new QLabel;
  lblStopHeight->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

  groupBoxLayout->addWidget( lblStopHeight, 1, 2 );

  lblDiffTime = new QLabel;
  lblDiffTime->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

  groupBoxLayout->addWidget( lblDiffTime, 2, 1 );

  QGroupBox* groupBox = new QGroupBox( page );
  groupBox->setTitle( tr( "Task Start/End Point" ) );
  groupBox->setLayout(groupBoxLayout);

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(groupBox);

  timeButton = new QPushButton;
  timeButton->setText( tr( "Set Times" ) );

  hbox->addStretch( 5 );
  hbox->addWidget(timeButton);
  hbox->addStretch( 5 );

  pageLayout_1->addLayout( hbox );
  addPage(page);

 //-----------------------------------------------------------------------------

  page_2 = new QWizardPage( this );
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  page_2->setTitle(tr("Optimization"));
  page_2->setSubTitle(tr("Use the buttons below to start or stop the OLC optimization."));

  QVBoxLayout* pageLayout_2 = new QVBoxLayout( page_2 );
  pageLayout_2->setMargin(10);
  pageLayout_2->setSpacing(10);

  olcResultBrowser = new QTextBrowser;
  pageLayout_2->addWidget( olcResultBrowser );

  QVBoxLayout* frameLayout = new QVBoxLayout;
  frameLayout->setMargin(10);
  frameLayout->setSpacing(10);

  progress = new QProgressBar;
  frameLayout->addWidget( progress );

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  btnStart = new QPushButton;
  btnStart->setText( tr( "Start Optimization" ) );
  buttonLayout->addWidget( btnStart );
  buttonLayout->addStretch( 5 );

  btnStop = new QPushButton;
  btnStop->setText( tr( "Stop Optimization" ) );
  btnStop->setEnabled( false );
  buttonLayout->addWidget( btnStop );

  frameLayout->addLayout( buttonLayout );

  QFrame* frame = new QFrame( this );
  frame->setFrameShape( QFrame::StyledPanel );
  frame->setFrameShadow( QFrame::Plain );
  frame->setLayout( frameLayout );

  pageLayout_2->addWidget( frame );
  addPage(page_2);
  languageChange();

  // signals and slots connections
  connect( timeButton, SIGNAL( pressed() ), this, SLOT( slotSetTimes() ) );
  connect( btnStop, SIGNAL( pressed() ), this, SLOT( slotStopOptimization() ) );
  connect( btnStart, SIGNAL( pressed() ), this, SLOT( slotStartOptimization() ) );
  init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
OptimizationWizard::~OptimizationWizard()
{
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void OptimizationWizard::languageChange()
{
  lblStartHeight->setText( tr( "1999m" ) );
  lblStopTime->setText( tr( "11:11:11" ) );
  lblDiffHeight->setText( tr( "1999m" ) );
  textLabel1_3_2_2->setText( "<p align=\"right\">" + tr( "Difference" ) + ":</p>" );
  lblStartTime->setText( tr( "11:11:11" ) );
  textLabel1_4_2->setText( "<p align=\"right\">" + tr( "Start of Task" ) + ":</p>" );
  textLabel1_2_2_2->setText( "<p align=\"right\">" + tr( "End of Task" ) + ":</p>" );
  lblStopHeight->setText( tr( "1999m" ) );
  lblDiffTime->setText( tr( "11:11:11" ) );
}

void OptimizationWizard::init()
{
  flight = dynamic_cast<Flight *> (_globalMapContents->getFlight() );

  if( flight == static_cast<Flight *> (0) )
    {
      return;
    }

  route = flight->getRoute();

  optimization = new Optimization( 0, route.count(), route, progress );

  // That loads the current flight in the evaluation dialog.
  evaluationDialog->slotShowFlightData();

  slotSetTimes();

  QString text = "<DIV ALIGN=CENTER>";

  text += tr("The task has not been optimized for the OLC, yet!") +
          "<br><br>" + tr("Press <i>Start Optimization</i> to begin.");

  text += "</DIV>";

  olcResultBrowser->setHtml(text);
}

void OptimizationWizard::slotStartOptimization()
{
  optimization->enableRun();
  btnStart->setEnabled(false);
  btnStop->setEnabled(true);

  optimization->run();

  unsigned int idList[LEGS + 3];
  double points;
  double distance = optimizationResult( idList, &points );

  if( distance < 0.0 ) // optimization was canceled
    {
      return;
    }

  btnStop->setEnabled(false);
  btnStart->setEnabled(true);

  QString text, distText, rawPointText;
  rawPointText.sprintf(" %.2f", points);
  distText.sprintf(" %.2f km  ", distance);

  text = "<div align=\"center\"<caption><b>";
  text += tr("Optimization Result")+"</b></div><table align=\"center\" CELLPADDING=3><thead><tr>";
  text += "<th scope=col></th><th scope=col>" + tr("Time") + "</th><th scope=col>";
  text += tr("Latitude") + "</th><th scope=col>"+tr("Longitude")+"</th><th ALIGN=right scope=col>";
  text += tr("Distance") + "</th></tr>";
  text += "</thead><tbody>";

  text += "<tr><td>" + tr("Begin of Soaring") + "</td><td>"
      + printTime(route.at(idList[0])->time,true) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[0])->origP.lat()) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[0])->origP.lon(), false) + "</td><td></td></tr>";
  text += "<tr><td>" + tr("Begin of Task") + "</td><td>"
      + printTime(route.at(idList[1])->time,true) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[1])->origP.lat()) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[1])->origP.lon(), false) + "</td><td></td></tr>";
  text += "<tr><td>" + tr("1.Turnpoint") + "</td><td>"
      + printTime(route.at(idList[2])->time,true) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[2])->origP.lat()) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[2])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[1]),route.at(idList[2])),0,'f',2);
  text += "<tr><td>" + tr("2.Turnpoint") + "</td><td>"
      + printTime(route.at(idList[3])->time,true) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[3])->origP.lat()) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[3])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[2]),route.at(idList[3])),0,'f',2);
  text += "<tr><td>" + tr("3.Turnpoint") + "</td><td>"
      + printTime(route.at(idList[4])->time,true) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[4])->origP.lat()) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[4])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[3]),route.at(idList[4])),0,'f',2);
  text += "<tr><td>" + tr("4.Turnpoint") + "</td><td>"
      + printTime(route.at(idList[5])->time,true) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[5])->origP.lat()) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[5])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[4]),route.at(idList[5])),0,'f',2);
  text += "<tr><td>" +tr("5.Turnpoint") + "</td><td>"
      + printTime(route.at(idList[6])->time,true) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[6])->origP.lat()) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[6])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[5]),route.at(idList[6])),0,'f',2);
  text += "<tr><td>" + tr("End of Task") + "</td><td>"
      + printTime(route.at(idList[7])->time,true) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[7])->origP.lat()) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[7])->origP.lon(), false) + "</td><td ALIGN=right>"
      + QString("%1km</td></tr>").arg(dist(route.at(idList[6]),route.at(idList[7])),0,'f',2);
  text += "<tr><td>" + tr("End of Soaring") + "</td><td>"
      + printTime(route.at(idList[8])->time,true) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[8])->origP.lat()) + "</td><td>"
      + WGSPoint::printPos(route.at(idList[8])->origP.lon(), false) + "</td><td></td></tr>";
  text += "</tbody></table><th>";

  text += "<br><table align=\"center\">";
  text += "<tr><th>" + tr("Summary:") + " " + tr("Distance:");
  text += distText + " " + tr("Points(raw):");
  text += rawPointText+"</th></tr>";
  text += "</table>";

  int heightDiff = route.at(idList[8])->height - route.at(idList[0])->height;

  if (heightDiff<-1000)
    {
      text += "<div align=\"center\"><br><b>" + tr("WARNING! THE HEIGHT DIFFERENCE IS TOO LARGE!<br>This is not a valid OLC task. Go back and correct the start and/or end times.")+"</b></div>";
    }

  text += "<div align=\"center\"><br>" +
          tr("If you want to use this task and replace the old one, press <i>Finish</i>, otherwise press <i>Cancel</i>")+"</div>";

  olcResultBrowser->setHtml(text);
}

void OptimizationWizard::slotStopOptimization()
{
    btnStart->setEnabled(true);
    btnStop->setEnabled(false);
    optimization->stopRun();
}

void OptimizationWizard::slotSetTimes()
{
  if( ! evaluationDialog && evaluationDialog->getFlight() )
    {
      return;
    }

  unsigned int start = evaluationDialog->getTaskStart();
  unsigned int stop  = evaluationDialog->getTaskEnd();
  FlightPoint startPoint = flight->getPoint( start );
  FlightPoint endPoint   = flight->getPoint( stop );

  lblStartTime->setText(QString("%1").arg(printTime(startPoint.time,true)));
  lblStopTime->setText(QString("%1").arg(printTime(endPoint.time,true)));
  lblDiffTime->setText(QString("%1").arg(printTime(endPoint.time-startPoint.time,true)));
  lblStartHeight->setText(QString("%1m").arg(startPoint.height));
  lblStopHeight->setText(QString("%1m").arg(endPoint.height));
  lblDiffHeight->setText(QString("%1m").arg(endPoint.height-startPoint.height));

  QColor pColor = QColor( 0, 128, 0 );

  if( (endPoint.height - startPoint.height) < -1000 )
    {
      pColor = QColor( 255, 0, 0 );
    }

  QPalette p = lblDiffHeight->palette();
  p.setColor( lblDiffHeight->foregroundRole(), pColor );
  lblDiffHeight->setPalette(p);
  lblDiffHeight->setAutoFillBackground( true );

  optimization->setTimes( start, stop );
}

void OptimizationWizard::setMapContents( Map* _map )
{
  connect( _globalMapContents, SIGNAL(currentFlightChanged()),
           evaluationDialog, SLOT(slotShowFlightData()) );

  connect( evaluationDialog, SIGNAL(showCursor(QPoint, QPoint)),
           _map, SLOT(slotDrawCursor(QPoint, QPoint)) );
}

double OptimizationWizard::optimizationResult( unsigned int* pointList,
                                               double * points )
{
  return optimization->optimizationResult( pointList, points );
}
