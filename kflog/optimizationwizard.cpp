#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './optimizationwizard.ui'
**
** Created: Don Mär 6 14:44:02 2003
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
#include <qgroupbox.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "./optimizationwizard.ui.h"

/* 
 *  Constructs a OptimizationWizard as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The wizard will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
OptimizationWizard::OptimizationWizard( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QWizard( parent, name, modal, fl )

{
    if ( !name )
	setName( "OLC_Optimization" );

    page = new QWidget( this, "page" );

    kPushButton2 = new KPushButton( page, "kPushButton2" );
    kPushButton2->setGeometry( QRect( 170, 200, 140, 30 ) );

    groupBox1 = new QGroupBox( page, "groupBox1" );
    groupBox1->setGeometry( QRect( 90, 70, 310, 120 ) );

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
    addPage( page, "" );

    page_2 = new QWidget( this, "page_2" );

    QWidget* privateLayoutWidget_2 = new QWidget( page_2, "layout5" );
    privateLayoutWidget_2->setGeometry( QRect( 100, 100, 280, 70 ) );
    layout5 = new QVBoxLayout( privateLayoutWidget_2, 11, 6, "layout5"); 

    progress = new KProgress( privateLayoutWidget_2, "progress" );
    layout5->addWidget( progress );

    layout2 = new QHBoxLayout( 0, 0, 6, "layout2"); 

    btnStart = new KPushButton( privateLayoutWidget_2, "btnStart" );
    layout2->addWidget( btnStart );

    btnStop = new KPushButton( privateLayoutWidget_2, "btnStop" );
    btnStop->setEnabled( FALSE );
    layout2->addWidget( btnStop );
    layout5->addLayout( layout2 );
    addPage( page_2, "" );

    page_3 = new QWidget( this, "page_3" );

    kTextBrowser1 = new KTextBrowser( page_3, "kTextBrowser1" );
    kTextBrowser1->setGeometry( QRect( 14, 8, 460, 260 ) );
    kTextBrowser1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, kTextBrowser1->sizePolicy().hasHeightForWidth() ) );
    addPage( page_3, "" );
    languageChange();
    resize( QSize(500, 353).expandedTo(minimumSizeHint()) );

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
    setCaption( tr2i18n( "OLC Optimization" ) );
    kPushButton2->setText( tr2i18n( "Set Times" ) );
    groupBox1->setTitle( tr2i18n( "Task Start/End Point" ) );
    lblStartHeight->setText( tr2i18n( "1999m" ) );
    lblStopTime->setText( tr2i18n( "11:11:11" ) );
    lblDiffHeight->setText( tr2i18n( "1999m" ) );
    textLabel1_3_2_2->setText( tr2i18n( "<p align=\"right\">Difference:</p>" ) );
    lblStartTime->setText( tr2i18n( "11:11:11" ) );
    textLabel1_4_2->setText( tr2i18n( "<p align=\"right\">Start of Task:</p>" ) );
    textLabel1_2_2_2->setText( tr2i18n( "<p align=\"right\">End of Task:</p>" ) );
    lblStopHeight->setText( tr2i18n( "1999m" ) );
    lblDiffTime->setText( tr2i18n( "11:11:11" ) );
    setTitle( page, tr2i18n( "Task start and end times" ) );
    btnStart->setText( tr2i18n( "Start Optimization" ) );
    btnStop->setText( tr2i18n( "Stop Optimization" ) );
    setTitle( page_2, tr2i18n( "Optimization" ) );
    setTitle( page_3, tr2i18n( "Pilot information" ) );
}

#include "optimizationwizard.moc"
