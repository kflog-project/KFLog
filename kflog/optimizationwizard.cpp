/****************************************************************************
** Form implementation generated from reading ui file 'optimizationwizard.ui'
**
** Created: Sam Mär 8 12:18:54 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "optimizationwizard.h"

#include <qvariant.h>
#include <kprogress.h>
#include <kpushbutton.h>
#include <ktextbrowser.h>
#include <qcolor.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include "evaluationdialog.h"
#include "optimizationwizard.ui.h"
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
OptimizationWizard::OptimizationWizard( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QWizard( parent, name, modal, fl )
,
      image0( (const char **) image0_data )
{
    if ( !name )
	setName( "OLC_Optimization" );

    page = new QWidget( this, "page" );
    pageLayout = new QVBoxLayout( page, 11, 6, "pageLayout"); 

    layout14 = new QVBoxLayout( 0, 0, 6, "layout14"); 

    evaluation = new EvaluationDialog( page, "evaluation" );
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
    lblStartHeight->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    layout3->addWidget( lblStartHeight, 0, 2 );

    lblStopTime = new QLabel( privateLayoutWidget, "lblStopTime" );
    lblStopTime->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    layout3->addWidget( lblStopTime, 1, 1 );

    lblDiffHeight = new QLabel( privateLayoutWidget, "lblDiffHeight" );
    lblDiffHeight->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    layout3->addWidget( lblDiffHeight, 2, 2 );

    textLabel1_3_2_2 = new QLabel( privateLayoutWidget, "textLabel1_3_2_2" );

    layout3->addWidget( textLabel1_3_2_2, 2, 0 );

    lblStartTime = new QLabel( privateLayoutWidget, "lblStartTime" );
    lblStartTime->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    layout3->addWidget( lblStartTime, 0, 1 );

    textLabel1_4_2 = new QLabel( privateLayoutWidget, "textLabel1_4_2" );

    layout3->addWidget( textLabel1_4_2, 0, 0 );

    textLabel1_2_2_2 = new QLabel( privateLayoutWidget, "textLabel1_2_2_2" );

    layout3->addWidget( textLabel1_2_2_2, 1, 0 );

    lblStopHeight = new QLabel( privateLayoutWidget, "lblStopHeight" );
    lblStopHeight->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    layout3->addWidget( lblStopHeight, 1, 2 );

    lblDiffTime = new QLabel( privateLayoutWidget, "lblDiffTime" );
    lblDiffTime->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    layout3->addWidget( lblDiffTime, 2, 1 );
    layout12->addWidget( groupBox1 );

    kPushButton2 = new KPushButton( page, "kPushButton2" );
    kPushButton2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, kPushButton2->sizePolicy().hasHeightForWidth() ) );
    layout12->addWidget( kPushButton2 );
    layout14->addLayout( layout12 );
    pageLayout->addLayout( layout14 );
    addPage( page, "" );

    page_2 = new QWidget( this, "page_2" );
    pageLayout_2 = new QVBoxLayout( page_2, 11, 6, "pageLayout_2"); 

    layout13 = new QVBoxLayout( 0, 0, 6, "layout13"); 

    kTextBrowser1 = new KTextBrowser( page_2, "kTextBrowser1" );
    layout13->addWidget( kTextBrowser1 );

    layout12_2 = new QHBoxLayout( 0, 0, 6, "layout12_2"); 
    QSpacerItem* spacer = new QSpacerItem( 21, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout12_2->addItem( spacer );

    frame3 = new QFrame( page_2, "frame3" );
    frame3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, frame3->sizePolicy().hasHeightForWidth() ) );
    frame3->setFrameShape( QFrame::StyledPanel );
    frame3->setFrameShadow( QFrame::Plain );
    frame3Layout = new QVBoxLayout( frame3, 11, 6, "frame3Layout"); 

    layout5 = new QVBoxLayout( 0, 0, 6, "layout5"); 

    progress = new KProgress( frame3, "progress" );
    progress->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, progress->sizePolicy().hasHeightForWidth() ) );
    layout5->addWidget( progress );

    layout2 = new QHBoxLayout( 0, 0, 6, "layout2"); 

    btnStart = new KPushButton( frame3, "btnStart" );
    btnStart->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, btnStart->sizePolicy().hasHeightForWidth() ) );
    layout2->addWidget( btnStart );

    btnStop = new KPushButton( frame3, "btnStop" );
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
    addPage( page_2, "" );
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
    setCaption( i18n( "OLC Optimization" ) );
    groupBox1->setTitle( i18n( "Task Start/End Point" ) );
    lblStartHeight->setText( tr( "1999m" ) );
    lblStopTime->setText( tr( "11:11:11" ) );
    lblDiffHeight->setText( tr( "1999m" ) );
    textLabel1_3_2_2->setText( "<p align=\"right\">" + i18n( "Difference" ) + ":</p>" );
    lblStartTime->setText( tr( "11:11:11" ) );
    textLabel1_4_2->setText( "<p align=\"right\">" + i18n( "Start of Task" ) + ":</p>" );
    textLabel1_2_2_2->setText( "<p align=\"right\">" + i18n( "End of Task" ) + ":</p>" );
    lblStopHeight->setText( tr( "1999m" ) );
    lblDiffTime->setText( tr( "11:11:11" ) );
    kPushButton2->setText( i18n( "Set Times" ) );
    setTitle( page, i18n( "Task start and end times" ) );
    btnStart->setText( i18n( "Start Optimization" ) );
    btnStop->setText( i18n( "Stop Optimization" ) );
    setTitle( page_2, i18n( "Optimization" ) );
}

