/****************************************************************************
** Form implementation generated from reading ui file 'optimizationwizard.ui'
**
** Created: Sam Mär 8 12:18:54 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include <QColor>
#include <QImage>
#include <QPushButton>
#include <QToolTip>

#include "optimizationwizard.h"
#include "wgspoint.h"

static const char* const image0_data[] = {
"22 22 114 2",
"Qt c None",
".# c #000000",
".J c #000008",
".g c #000400",
".2 c #000408",
"#J c #080408",
"#y c #080808",
".P c #080c10",
"#c c #101010",
"#z c #101408",
"#h c #101410",
"#V c #101800",
"## c #101810",
"#M c #101c08",
"#p c #101c10",
"#q c #181810",
".S c #181c10",
"#r c #182010",
"#L c #182410",
"#v c #202410",
".K c #202810",
".5 c #292c10",
"#R c #293000",
"#U c #293008",
".I c #293010",
"#e c #293018",
".Q c #293418",
".R c #313818",
".s c #314008",
".f c #393839",
"#t c #393c18",
".W c #414c18",
".F c #4a5908",
".T c #4a5918",
"#I c #525918",
"#S c #525d08",
"#j c #525d18",
".N c #526108",
"#T c #526110",
".9 c #526118",
"#n c #526120",
"#b c #526508",
"#F c #526520",
".e c #5a5d5a",
"#g c #5a6908",
"#k c #5a6910",
".O c #5a6918",
".1 c #5a6920",
"#A c #5a6d08",
"#o c #5a6d10",
"#K c #627110",
".3 c #627120",
".z c #627518",
".A c #627520",
".L c #627920",
".d c #6a6d6a",
"#B c #6a7520",
".H c #6a7d20",
"#P c #738529",
".y c #738920",
"#u c #738929",
".a c #7b797b",
"#x c #7b8920",
".X c #8b9d20",
".G c #8ba110",
".v c #8ba118",
".u c #8ba120",
"#E c #8ba131",
".x c #8ba518",
".8 c #8ba520",
"#s c #8ba531",
".c c #949194",
"#l c #94a529",
".r c #94a539",
".w c #94aa10",
".B c #94aa18",
"#d c #94aa20",
"#N c #94aa29",
"#Q c #94ae18",
".D c #94b218",
".0 c #94b229",
".E c #9cb218",
".Y c #9cb220",
"#a c #9cb229",
".t c #9cb241",
".6 c #9cb610",
".C c #9cb618",
".7 c #9cba10",
".Z c #9cba18",
".M c #9cba20",
"#w c #9cba29",
".b c #a4a1a4",
"#H c #a4b620",
".V c #a4ba18",
".U c #a4ba20",
"#. c #a4ba29",
"#f c #a4be29",
".k c #acbe62",
"#O c #acc220",
".4 c #acc629",
"#C c #acca20",
"#i c #acce20",
".l c #b4be62",
".m c #b4c262",
".j c #b4c273",
".i c #b4c28b",
".q c #b4c65a",
".n c #b4c662",
".p c #b4ca62",
"#D c #b4d618",
"#m c #b4d620",
".h c #bdbea4",
".o c #bdca62",
"#G c #bdde20",
"QtQtQt.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#Qt",
"QtQt.a.b.c.a.d.d.d.d.d.d.d.d.d.d.e.f.g.#.#.#",
"Qt.#.b.h.i.j.k.l.m.n.n.o.o.n.n.p.q.r.s.#.#.#",
"Qt.#.c.i.t.u.v.w.x.y.z.A.y.B.C.D.D.E.F.#.#.#",
"Qt.#.a.j.u.G.B.H.I.J.#.#.J.K.L.M.C.C.N.#.#.#",
"Qt.#.d.k.G.B.O.J.J.P.Q.R.S.J.J.T.U.V.N.#.#.#",
"Qt.#.d.l.w.H.J.#.W.X.Y.Z.0.1.2.#.3.4.N.#.#.#",
"Qt.#.d.m.x.5.#.W.Y.6.7.8.9#..3.J###a#b.#.#.#",
"Qt.#.d.n.y.J#c.X.6.7#d#e.J.W#f.5.#.y#g.#.#.#",
"Qt.#.d.n.z.##e.Y.7#d.I.J#h.y#i#j.#.O#k.#.#.#",
"Qt.#.d.o.A.#.R.Z.u#e.J.#.S#l#m#n.#.1#o.#.#.#",
"Qt.#.d.o.y.J#p.0#n.J#c#q.##r#s#t.##u#o.#.#.#",
"Qt.#.d.n.B#v.J.1#w.W#x#l#r.##c#y#z.U#A.#.#.#",
"Qt.#.d.n.C#B.J.2.A#f#C#D#E##.#.g#F#G#A.#.#.#",
"Qt.#.d.n.D#H#I.#.J.I#I.9#t#J.g.g#v#a#K.#.#.#",
"Qt.#.e.q.D.C.U.3.S.#.#.#.##z#F#L#M#N.N.#.#.#",
"Qt.#.f.r.D.C.Z#O#a.y.1.1#P.U#G.0#N#Q#R.#.#.#",
"Qt.#.g.s.F.N#S.N#b#g#k#o#o#o#A#K#T#U#V.#.#.#",
"Qt.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
"Qt.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
"QtQt.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
"QtQtQt.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#Qt"};


/*
 *  Constructs a OptimizationWizard as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The wizard will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
OptimizationWizard::OptimizationWizard(QWidget* parent, Qt::WFlags fl)
    : QWizard(parent, fl)
,
      image0( (const char **) image0_data )
{
    setName("Optimization for OLC");

    page = new QWizardPage(this);
    page->setTitle(tr("Task start and end times"));
    pageLayout = new QVBoxLayout( page, 11, 6, "pageLayout");

    layout14 = new QVBoxLayout( 0, 0, 6, "layout14");

    evaluation = new EvaluationDialog( page );
    evaluation->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, evaluation->sizePolicy().hasHeightForWidth() ) );
    evaluation->setMinimumSize( QSize( 100, 100 ) );
    layout14->addWidget( evaluation );

    layout12 = new QHBoxLayout( 0, 0, 6, "layout12");

    groupBox1 = new QGroupBox( page, "groupBox1" );
    groupBox1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, groupBox1->sizePolicy().hasHeightForWidth() ) );

    QWidget* privateLayoutWidget = new QWidget( groupBox1, "layout3" );
    privateLayoutWidget->setGeometry( QRect( 10, 20, 290, 80 ) );
    layout3 = new QGridLayout( privateLayoutWidget, 1, 1, 11, 6, "layout3");

    lblStartHeight = new QLabel( privateLayoutWidget, "lblStartHeight" );
    lblStartHeight->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );

    layout3->addWidget( lblStartHeight, 0, 2 );

    lblStopTime = new QLabel( privateLayoutWidget, "lblStopTime" );
    lblStopTime->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );

    layout3->addWidget( lblStopTime, 1, 1 );

    lblDiffHeight = new QLabel( privateLayoutWidget, "lblDiffHeight" );
    lblDiffHeight->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );

    layout3->addWidget( lblDiffHeight, 2, 2 );

    textLabel1_3_2_2 = new QLabel( privateLayoutWidget, "textLabel1_3_2_2" );

    layout3->addWidget( textLabel1_3_2_2, 2, 0 );

    lblStartTime = new QLabel( privateLayoutWidget, "lblStartTime" );
    lblStartTime->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );

    layout3->addWidget( lblStartTime, 0, 1 );

    textLabel1_4_2 = new QLabel( privateLayoutWidget, "textLabel1_4_2" );

    layout3->addWidget( textLabel1_4_2, 0, 0 );

    textLabel1_2_2_2 = new QLabel( privateLayoutWidget, "textLabel1_2_2_2" );

    layout3->addWidget( textLabel1_2_2_2, 1, 0 );

    lblStopHeight = new QLabel( privateLayoutWidget, "lblStopHeight" );
    lblStopHeight->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );

    layout3->addWidget( lblStopHeight, 1, 2 );

    lblDiffTime = new QLabel( privateLayoutWidget, "lblDiffTime" );
    lblDiffTime->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );

    layout3->addWidget( lblDiffTime, 2, 1 );
    layout12->addWidget( groupBox1 );

    kPushButton2 = new QPushButton( page, "kPushButton2" );
    kPushButton2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, kPushButton2->sizePolicy().hasHeightForWidth() ) );
    layout12->addWidget( kPushButton2 );
    layout14->addLayout( layout12 );
    pageLayout->addLayout( layout14 );
    addPage(page);

    page_2 = new QWizardPage(this);
    page_2->setTitle(tr("Optimization"));
    pageLayout_2 = new QVBoxLayout( page_2, 11, 6, "pageLayout_2");

    layout13 = new QVBoxLayout( 0, 0, 6, "layout13");

    kTextBrowser1 = new QTextBrowser( page_2, "kTextBrowser1" );
    layout13->addWidget( kTextBrowser1 );

    layout12_2 = new QHBoxLayout( 0, 0, 6, "layout12_2");
    QSpacerItem* spacer = new QSpacerItem( 21, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout12_2->addItem( spacer );

    frame3 = new QFrame( page_2, "frame3" );
    frame3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, frame3->sizePolicy().hasHeightForWidth() ) );
    frame3->setFrameShape( Q3Frame::StyledPanel );
    frame3->setFrameShadow( Q3Frame::Plain );
    frame3Layout = new QVBoxLayout( frame3, 11, 6, "frame3Layout");

    layout5 = new QVBoxLayout( 0, 0, 6, "layout5");

    progress = new QProgressBar(frame3);
    progress->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, progress->sizePolicy().hasHeightForWidth() ) );
    layout5->addWidget( progress );

    layout2 = new QHBoxLayout( 0, 0, 6, "layout2");

    btnStart = new QPushButton( frame3, "btnStart" );
    btnStart->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, btnStart->sizePolicy().hasHeightForWidth() ) );
    layout2->addWidget( btnStart );

    btnStop = new QPushButton( frame3, "btnStop" );
    btnStop->setEnabled( FALSE );
    btnStop->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, btnStop->sizePolicy().hasHeightForWidth() ) );
    layout2->addWidget( btnStop );
    layout5->addLayout( layout2 );
    frame3Layout->addLayout( layout5 );
    layout12_2->addWidget( frame3 );
    QSpacerItem* spacer_2 = new QSpacerItem( 51, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout12_2->addItem( spacer_2 );
    layout13->addLayout( layout12_2 );
    pageLayout_2->addLayout( layout13 );
    addPage(page_2);
    languageChange();
    resize( QSize(446, 329).expandedTo(minimumSizeHint()) );

    // signals and slots connections
    connect( kPushButton2, SIGNAL( pressed() ), this, SLOT( slotSetTimes() ) );
    connect( btnStop, SIGNAL( pressed() ), this, SLOT( slotStopOptimization() ) );
    connect( btnStart, SIGNAL( pressed() ), this, SLOT( slotStartOptimization() ) );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
OptimizationWizard::~OptimizationWizard()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void OptimizationWizard::languageChange()
{
    setCaption( tr( "OLC Optimization" ) );
    groupBox1->setTitle( tr( "Task Start/End Point" ) );
    lblStartHeight->setText( tr( "1999m" ) );
    lblStopTime->setText( tr( "11:11:11" ) );
    lblDiffHeight->setText( tr( "1999m" ) );
    textLabel1_3_2_2->setText( "<p align=\"right\">" + tr( "Difference" ) + ":</p>" );
    lblStartTime->setText( tr( "11:11:11" ) );
    textLabel1_4_2->setText( "<p align=\"right\">" + tr( "Start of Task" ) + ":</p>" );
    textLabel1_2_2_2->setText( "<p align=\"right\">" + tr( "End of Task" ) + ":</p>" );
    lblStopHeight->setText( tr( "1999m" ) );
    lblDiffTime->setText( tr( "11:11:11" ) );
    kPushButton2->setText( tr( "Set Times" ) );
    btnStart->setText( tr( "Start Optimization" ) );
    btnStop->setText( tr( "Stop Optimization" ) );
}

void OptimizationWizard::init()
{
   extern MapContents *_globalMapContents;
//  next two lines are a hack to fix initial evaluation widget size bug
  showMaximized();
  showNormal();

  resize(640,520);

  flight=(Flight*)_globalMapContents->getFlight();
  route=flight->getRoute();
  optimization = new Optimization(0, route.count(), route,progress);
  slotSetTimes();
  QString text = "<DIV ALIGN=CENTER>";
  text += tr("The task has not been optimized for the OLC, yet") +
      "<br>" + tr("Press \"Start Optimization\" to begin");
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

//  setFinishEnabled ( page_2, true );
  btnStop->setEnabled(false);
  btnStart->setEnabled(true);

  QString text, distText, rawPointText;
  rawPointText.sprintf(" %.2f", points);
  distText.sprintf(" %.2f km  ", distance);
//  text = tr("The task has been optimized for the OLC.\nThe best task found is:\n\n");
//  text = text + "\tBOT:  "
//      + printTime(route.at(idList[0])->time,true) + " : \t"
//      + WGSPoint::printPos(route.at(idList[0])->origP.lat()) + " / "
//      + WGSPoint::printPos(route.at(idList[0])->origP.lon(), false)
//      + QString("\n\t1.TP:  ")
//      + printTime(route.at(idList[1])->time,true) + " : \t"
//      + WGSPoint::printPos(route.at(idList[1])->origP.lat()) + " / "
//      + WGSPoint::printPos(route.at(idList[1])->origP.lon(), false)
//      + QString(" (%1km)\n\t2.TP:  ").arg(dist(route.at(idList[0]),route.at(idList[1])),0,'f',2)
//      + printTime(route.at(idList[2])->time,true) + " : \t"
//      + WGSPoint::printPos(route.at(idList[2])->origP.lat()) + " / "
//      + WGSPoint::printPos(route.at(idList[2])->origP.lon(), false)
//      + QString(" (%1km)\n\t3.TP:  ").arg(dist(route.at(idList[1]),route.at(idList[2])),0,'f',2)
//      + printTime(route.at(idList[3])->time,true) + " : \t"
//      + WGSPoint::printPos(route.at(idList[3])->origP.lat()) + " / "
//      + WGSPoint::printPos(route.at(idList[3])->origP.lon(), false)
//      + QString(" (%1km)\n\t4.TP:  ").arg(dist(route.at(idList[2]),route.at(idList[3])),0,'f',2)
//      + printTime(route.at(idList[4])->time,true) + " : \t"
//      + WGSPoint::printPos(route.at(idList[4])->origP.lat()) + " / "
//      + WGSPoint::printPos(route.at(idList[4])->origP.lon(), false)
//      + QString(" (%1km)\n\t5.TP:  ").arg(dist(route.at(idList[3]),route.at(idList[4])),0,'f',2)
//      + printTime(route.at(idList[5])->time,true) + " : \t"
//      + WGSPoint::printPos(route.at(idList[5])->origP.lat()) + " / "
//      + WGSPoint::printPos(route.at(idList[5])->origP.lon(), false)
//      + QString(" (%1km)\n\tEOT:  ").arg(dist(route.at(idList[4]),route.at(idList[5])),0,'f',2)
//      + printTime(route.at(idList[6])->time,true) + " : \t"
//      + WGSPoint::printPos(route.at(idList[6])->origP.lat()) + " / "
//      + WGSPoint::printPos(route.at(idList[6])->origP.lon(), false)
//      + QString(" (%1km):  ").arg(dist(route.at(idList[5]),route.at(idList[6])),0,'f',2)
//      + tr("\n\nDistance:\t") + distText + "\n" + tr("Points(raw):\t") + rawPointText + "\n\n"
//      + tr("If you want to use this task and replace the old, press \"Finish\",\nelse press \"Cancel\"");
  text = "<div align=\"center\"<caption><b>";
  text += tr("Optimization Result")+"</b></div><table align=\"center\"><thead><tr>";
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

  text += "<table align=\"center\">";
  text += "<tr><td><b>" + tr("Summary:") + " " + tr("Distance:");
  text += distText + " " + tr("Points(raw):");
  text += rawPointText+"</b></td></tr>";
  text += "</table>";

  int heightDiff = route.at(idList[8])->height - route.at(idList[0])->height;
  if (heightDiff<-1000)
    text += "<div align=\"center\"><b>" + tr("WARNING! THE HEIGHT DIFFERENCE IS TOO LARGE!<br>This is not a valid OLC task. Go back and correct the start and/or end times.")+"</b></div>";

  text += "<div align=\"center\">"+tr("If you want to use this task and replace the old, press \"Finish\", else press \"Cancel\"")+"</div>";

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
  FlightPoint startPoint=flight->getPoint(start);
  FlightPoint endPoint=flight->getPoint(stop);
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
    extern MapContents *_globalMapContents;
    connect(_globalMapContents, SIGNAL(currentFlightChanged()), evaluation,
        SLOT(slotShowFlightData()));
    connect(evaluation, SIGNAL(showCursor(QPoint, QPoint)), _map,
        SLOT(slotDrawCursor(QPoint, QPoint)));
}

double OptimizationWizard::optimizationResult( unsigned int* pointList, double * points )
{
    return optimization->optimizationResult( pointList,points );
}
