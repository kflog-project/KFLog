/***********************************************************************
**
**   configmapelement.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**                   2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "configmapelement.h"
#include "kflogconfig.h"
#include "mapdefaults.h"
#include "mainwindow.h"

extern MainWindow* _mainWindow;
extern QSettings   _settings;

ConfigMapElement::ConfigMapElement( QWidget* parent, bool configMap ) :
  QWidget(parent),
  configureMap(configMap),
  oldElement(-1)
{
  /*
   * Note, this widget is used for two different configurations. The passed
   * flag configMap determines, if the widget is used for map item configuration
   * or print item configuration. In the print item configuration all border2
   * and border3 widgets are not visible.
   */
  QString checkBox1Label;
  QString checkBox2Label;

  if( configureMap )
    {
      itemPrefix = "/";
      checkBox1Label = tr( "Threshold #1" );
      checkBox2Label = tr( "Threshold #2" );
      __readMapItems();
    }
  else
    {
      itemPrefix = "/Print";
      checkBox1Label = "1:500.000";
      checkBox2Label = tr( "Scale limit" );
      __readPrintItems();
    }

  __createBrushStyleIcons();

  border1 = new QCheckBox( checkBox1Label );
  border2 = new QCheckBox( checkBox2Label );
  border3 = new QCheckBox( tr( "Threshold #3" ) );
  border4 = new QCheckBox( tr( "Scale limit" ) );

  connect(border1, SIGNAL(toggled(bool)), SLOT(slotToggleFirst(bool)));
  connect(border2, SIGNAL(toggled(bool)), SLOT(slotToggleSecond(bool)));
  connect(border3, SIGNAL(toggled(bool)), SLOT(slotToggleThird(bool)));
  connect(border4, SIGNAL(toggled(bool)), SLOT(slotToggleForth(bool)));

  QGridLayout* drawLayout = new QGridLayout;
  drawLayout->setMargin( 10 );
  drawLayout->setSpacing( 5 );

  drawLayout->addWidget( border1, 0, 0 );
  drawLayout->addWidget( border2, 1, 0 );
  drawLayout->addWidget( border3, 2, 0 );
  drawLayout->addWidget( border4, 3, 0 );
  drawLayout->setColumnStretch( 1, 10 );

  QGroupBox* drawGroup = new QGroupBox( tr( "Draw up to" ) );
  drawGroup->setLayout( drawLayout );

  //----------------------------------------------------------------------------
  border1ColorButton = new QPushButton(this);
  border1ColorButton->setFixedHeight(24);
  border1ColorButton->setFixedWidth(55);
  border1ColorButton->setToolTip( tr("Select your desired pen color.") );
  connect(border1ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder1Color()));

  border2ColorButton = new QPushButton(this);
  border2ColorButton->setFixedHeight(24);
  border2ColorButton->setFixedWidth(55);
  border2ColorButton->setToolTip( tr("Select your desired pen color.") );
  connect(border2ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder2Color()));

  border3ColorButton = new QPushButton(this);
  border3ColorButton->setFixedHeight(24);
  border3ColorButton->setFixedWidth(55);
  border3ColorButton->setToolTip( tr("Select your desired pen color.") );
  connect(border3ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder3Color()));

  border4ColorButton = new QPushButton(this);
  border4ColorButton->setFixedHeight(24);
  border4ColorButton->setFixedWidth(55);
  border4ColorButton->setToolTip( tr("Select your desired pen color.") );
  connect(border4ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder4Color()));

  border1Pen = new QSpinBox();
  border1Pen->setRange( 1, 9 );
  border1Pen->setMinimumWidth( 35 );
  border1Pen->setButtonSymbols( QAbstractSpinBox::PlusMinus );
  border1Pen->setToolTip( tr("Select your desired pen width.") );

  border2Pen = new QSpinBox();
  border2Pen->setRange( 1, 9 );
  border2Pen->setMinimumWidth( 35 );
  border2Pen->setButtonSymbols( QAbstractSpinBox::PlusMinus );
  border2Pen->setToolTip( tr("Select your desired pen width.") );

  border3Pen = new QSpinBox();
  border3Pen->setRange( 1, 9 );
  border3Pen->setMinimumWidth( 35 );
  border3Pen->setButtonSymbols( QAbstractSpinBox::PlusMinus );
  border3Pen->setToolTip( tr("Select your desired pen width.") );

  border4Pen = new QSpinBox();
  border3Pen->setRange( 1, 9 );
  border4Pen->setMinimumWidth( 35 );
  border4Pen->setButtonSymbols( QAbstractSpinBox::PlusMinus );
  border4Pen->setToolTip( tr("Select your desired pen width.") );

  border1PenStyle = new QComboBox( this );
  border1PenStyle->setMinimumWidth( 60 );
  border1PenStyle->setToolTip( tr("Select your desired pen style.") );
  border1PenStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );

  border2PenStyle = new QComboBox( this );
  border2PenStyle->setMinimumWidth( 35 );
  border2PenStyle->setToolTip( tr("Select your desired pen style.") );
  border2PenStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );

  border3PenStyle = new QComboBox( this );
  border3PenStyle->setMinimumWidth( 35 );
  border3PenStyle->setToolTip( tr("Select your desired pen style.") );
  border3PenStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );

  border4PenStyle = new QComboBox( this );
  border4PenStyle->setMinimumWidth( 35 );
  border4PenStyle->setToolTip( tr("Select your desired pen style.") );
  border4PenStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );

  QGridLayout* penLayout = new QGridLayout;
  penLayout->setMargin( 10 );
  penLayout->setSpacing( 5 );

  penLayout->addWidget(border1ColorButton, 0, 0);
  penLayout->addWidget(border2ColorButton, 1, 0);
  penLayout->addWidget(border3ColorButton, 2, 0);
  penLayout->addWidget(border4ColorButton, 3, 0);
  penLayout->addWidget(border1Pen, 0, 1);
  penLayout->addWidget(border2Pen, 1, 1);
  penLayout->addWidget(border3Pen, 2, 1);
  penLayout->addWidget(border4Pen, 3, 1);
  penLayout->addWidget(border1PenStyle, 0, 2);
  penLayout->addWidget(border2PenStyle, 1, 2);
  penLayout->addWidget(border3PenStyle, 2, 2);
  penLayout->addWidget(border4PenStyle, 3, 2);
  penLayout->setColumnStretch( 4, 10 );

  QGroupBox* penGroup = new QGroupBox( tr("Pen") );
  penGroup->setLayout( penLayout );

  //----------------------------------------------------------------------------
  border1BrushColorButton = new QPushButton(this);
  border1BrushColorButton->setFixedHeight(24);
  border1BrushColorButton->setFixedWidth(55);
  border1BrushColorButton->setToolTip( tr("Select your desired brush color.") );
  connect(border1BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder1BrushColor()));

  border2BrushColorButton = new QPushButton(this);
  border2BrushColorButton->setFixedHeight(24);
  border2BrushColorButton->setFixedWidth(55);
  border2BrushColorButton->setToolTip( tr("Select your desired brush color.") );
  connect(border2BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder2BrushColor()));

  border3BrushColorButton = new QPushButton(this);
  border3BrushColorButton->setFixedHeight(24);
  border3BrushColorButton->setFixedWidth(55);
  border3BrushColorButton->setToolTip( tr("Select your desired brush color.") );
  connect(border3BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder3BrushColor()));

  border4BrushColorButton = new QPushButton(this);
  border4BrushColorButton->setFixedHeight(24);
  border4BrushColorButton->setFixedWidth(55);
  border4BrushColorButton->setToolTip( tr("Select your desired brush color.") );
  connect(border4BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder4BrushColor()));

  border1BrushStyle = new QComboBox(this);
  border1BrushStyle->setMinimumWidth(60);
  border1BrushStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  border1BrushStyle->setToolTip( tr("Select solid pattern brush to enable airspace structure filling.") );
  __fillStyle(border1PenStyle, border1BrushStyle);

  border2BrushStyle = new QComboBox(this);
  border2BrushStyle->setMinimumWidth(35);
  border2BrushStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  border2BrushStyle->setToolTip( tr("Select solid pattern brush to enable airspace structure filling.") );
  __fillStyle(border2PenStyle, border2BrushStyle);

  border3BrushStyle = new QComboBox(this);
  border3BrushStyle->setMinimumWidth(35);
  border3BrushStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  border3BrushStyle->setToolTip( tr("Select solid pattern brush to enable airspace structure filling.") );
  __fillStyle(border3PenStyle, border3BrushStyle);

  border4BrushStyle = new QComboBox(this);
  border4BrushStyle->setMinimumWidth(35);
  border4BrushStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  border4BrushStyle->setToolTip( tr("Select solid pattern brush to enable airspace structure filling.") );
  __fillStyle(border4PenStyle, border4BrushStyle);

  QSpinBox** oSpinArray[4];
  oSpinArray[0] = &opacity1;
  oSpinArray[1] = &opacity2;
  oSpinArray[2] = &opacity3;
  oSpinArray[3] = &opacity4;

  for( int i = 0; i < 4; i++ )
    {
      QSpinBox* sbox = new QSpinBox;
      sbox->setRange( 0, 100 );
      sbox->setSingleStep( 5 );
      sbox->setButtonSymbols( QAbstractSpinBox::PlusMinus );
      sbox->setSuffix( " %" );
      sbox->setEnabled( false );
      sbox->setToolTip( tr("The opacity value should be in the range 0 to 100. "
                           "0 is fully transparent and 100 is fully opaque.") );
      *oSpinArray[i] = sbox;
    }

  QGridLayout* brushLayout = new QGridLayout;
  brushLayout->setMargin( 10 );
  brushLayout->setSpacing( 5 );
  brushLayout->addWidget(border1BrushColorButton, 0, 0);
  brushLayout->addWidget(border2BrushColorButton, 1, 0);
  brushLayout->addWidget(border3BrushColorButton, 2, 0);
  brushLayout->addWidget(border4BrushColorButton, 3, 0);
  brushLayout->addWidget(border1BrushStyle, 0, 1);
  brushLayout->addWidget(border2BrushStyle, 1, 1);
  brushLayout->addWidget(border3BrushStyle, 2, 1);
  brushLayout->addWidget(border4BrushStyle, 3, 1);
  brushLayout->addWidget(opacity1, 0, 2);
  brushLayout->addWidget(opacity2, 1, 2);
  brushLayout->addWidget(opacity3, 2, 2);
  brushLayout->addWidget(opacity4, 3, 2);
  brushLayout->setColumnStretch( 2, 10 );

  QGroupBox* brushGroup = new QGroupBox( tr("Brush") );
  brushGroup->setLayout( brushLayout );

  //----------------------------------------------------------------------------
  border1Button = new QPushButton();
  border1Button->setIcon(_mainWindow->getPixmap("kde_down.png"));
  border1Button->setFixedWidth(30);
  border1Button->setFixedHeight(30);
  connect(border1Button, SIGNAL(clicked()), SLOT(slotSetSecond()));

  border2Button = new QPushButton();
  border2Button->setIcon(_mainWindow->getPixmap("kde_down.png"));
  border2Button->setFixedWidth(30);
  border2Button->setFixedHeight(30);
  connect(border2Button, SIGNAL(clicked()), SLOT(slotSetThird()));

  border3Button = new QPushButton();
  border3Button->setIcon(_mainWindow->getPixmap("kde_down.png"));
  border3Button->setFixedWidth(30);
  border3Button->setFixedHeight(30);
  connect(border3Button, SIGNAL(clicked()), SLOT(slotSetForth()));

  QGridLayout* copyLayout = new QGridLayout;
  copyLayout->setMargin( 10 );
  copyLayout->setSpacing( 5 );
  copyLayout->addWidget( border1Button, 0, 0 );
  copyLayout->addWidget( border2Button, 1, 0 );
  copyLayout->addWidget( border3Button, 2, 0 );
  copyLayout->setRowStretch( 4, 10 );

  QGroupBox* copyGroup = new QGroupBox( tr("Copy") );
  copyGroup->setToolTip( tr("Press this button to copy all content of this line into the line below.") );
  copyGroup->setLayout( copyLayout );

  //----------------------------------------------------------------------------
  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->addWidget( drawGroup );
  hbox->addWidget( penGroup );
  hbox->addWidget( brushGroup );
  hbox->addWidget( copyGroup );
  hbox->addStretch( 10 );
  setLayout( hbox );

  if( configureMap == false )
    {
      // Disable all not needed widgets in print configuration mode.

      // Draw up to group
      border3->setVisible( false );
      border4->setVisible( false );

      // Pen group
      border3ColorButton->setVisible( false );
      border4ColorButton->setVisible( false );
      border3Pen->setVisible( false );
      border4Pen->setVisible( false );
      border3PenStyle->setVisible( false );
      border4PenStyle->setVisible( false );

      // Brush group
      border3BrushColorButton->setVisible( false );
      border4BrushColorButton->setVisible( false );
      border3BrushStyle->setVisible( false );
      border4BrushStyle->setVisible( false );
      opacity3->setVisible( false );
      opacity4->setVisible( false );

      // Copy group
      border2Button->setVisible( false );
      border3Button->setVisible( false );
    }
}

ConfigMapElement::~ConfigMapElement()
{
}

void ConfigMapElement::__readMapItems()
{
  __readPen("Trail", trailPenList, TRAIL_COLOR_1, TRAIL_COLOR_2, TRAIL_COLOR_3, TRAIL_COLOR_4,
        TRAIL_PEN_1, TRAIL_PEN_2, TRAIL_PEN_3, TRAIL_PEN_4,
        TRAIL_PEN_STYLE_1, TRAIL_PEN_STYLE_2, TRAIL_PEN_STYLE_3, TRAIL_PEN_STYLE_4);
  __readBorder("Trail", trailBorder);

  __readPen("Road", roadPenList, ROAD_COLOR_1, ROAD_COLOR_2, ROAD_COLOR_3, ROAD_COLOR_4,
        ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4,
        ROAD_PEN_STYLE_1, ROAD_PEN_STYLE_2, ROAD_PEN_STYLE_3, ROAD_PEN_STYLE_4);
  __readBorder("Road", roadBorder);

  __readPen("River", riverPenList, RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_3,
        RIVER_COLOR_4, RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4,
        RIVER_PEN_STYLE_1, RIVER_PEN_STYLE_2, RIVER_PEN_STYLE_3, RIVER_PEN_STYLE_4);
  __readBrush("River", riverBrushList, RIVER_BRUSH_COLOR_1, RIVER_BRUSH_COLOR_2,
        RIVER_BRUSH_COLOR_3, RIVER_BRUSH_COLOR_4, RIVER_BRUSH_STYLE_1,
        RIVER_BRUSH_STYLE_2, RIVER_BRUSH_STYLE_3, RIVER_BRUSH_STYLE_4);
  __readBorder("River", riverBorder);

  __readPen("Canal", canalPenList, CANAL_COLOR_1, CANAL_COLOR_2, CANAL_COLOR_3,
        CANAL_COLOR_4, CANAL_PEN_1, CANAL_PEN_2, CANAL_PEN_3, CANAL_PEN_4,
        CANAL_PEN_STYLE_1, CANAL_PEN_STYLE_2, CANAL_PEN_STYLE_3, CANAL_PEN_STYLE_4);
  __readBorder("Canal", canalBorder);

  __readPen("Rail", railPenList, RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3, RAIL_COLOR_4,
        RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4,
        RAIL_PEN_STYLE_1, RAIL_PEN_STYLE_2, RAIL_PEN_STYLE_3, RAIL_PEN_STYLE_4);
  __readBorder("Rail", railBorder);

  __readPen("Rail_D", rail_dPenList, RAIL_D_COLOR_1, RAIL_D_COLOR_2, RAIL_D_COLOR_3,
        RAIL_D_COLOR_4, RAIL_D_PEN_1, RAIL_D_PEN_2, RAIL_D_PEN_3, RAIL_D_PEN_4,
        RAIL_D_PEN_STYLE_1, RAIL_D_PEN_STYLE_2, RAIL_D_PEN_STYLE_3, RAIL_D_PEN_STYLE_4);
  __readBorder("Rail_D", rail_dBorder);

  __readPen("Aerial Cable", aerialcablePenList, AERIAL_CABLE_COLOR_1, AERIAL_CABLE_COLOR_2,
        AERIAL_CABLE_COLOR_3, AERIAL_CABLE_COLOR_4, AERIAL_CABLE_PEN_1,
        AERIAL_CABLE_PEN_2, AERIAL_CABLE_PEN_3, AERIAL_CABLE_PEN_4,
        AERIAL_CABLE_PEN_STYLE_1, AERIAL_CABLE_PEN_STYLE_2,
        AERIAL_CABLE_PEN_STYLE_3, AERIAL_CABLE_PEN_STYLE_4);
  __readBorder("Aerial Cable", aerialcableBorder);

  __readPen("Highway", motorwayPenList, HIGH_COLOR_1, HIGH_COLOR_2, HIGH_COLOR_3, HIGH_COLOR_4,
        HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4,
        HIGH_PEN_STYLE_1, HIGH_PEN_STYLE_2, HIGH_PEN_STYLE_3, HIGH_PEN_STYLE_4);
  __readBorder("Highway", motorwayBorder);

  // PenStyle and BrushStyle are not used for cities ...
  __readPen("City", cityPenList, CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3,
        CITY_COLOR_4, CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4,
        Qt::SolidLine, Qt::SolidLine, Qt::SolidLine, Qt::SolidLine);
  __readBrush("City", cityBrushList, CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
      CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4, Qt::SolidPattern,
      Qt::SolidPattern, Qt::SolidPattern, Qt::SolidPattern);
  __readBorder("City", cityBorder);

  __readPen("River_T", river_tPenList, RIVER_T_COLOR_1, RIVER_T_COLOR_2, RIVER_T_COLOR_3,
        RIVER_T_COLOR_4, RIVER_T_PEN_1, RIVER_T_PEN_2, RIVER_T_PEN_3, RIVER_T_PEN_4,
        RIVER_T_PEN_STYLE_1, RIVER_T_PEN_STYLE_2, RIVER_T_PEN_STYLE_3, RIVER_T_PEN_STYLE_4);
  __readBrush("River_T", river_tBrushList, RIVER_T_BRUSH_COLOR_1, RIVER_T_BRUSH_COLOR_2,
        RIVER_T_BRUSH_COLOR_3, RIVER_T_BRUSH_COLOR_4, RIVER_T_BRUSH_STYLE_1,
        RIVER_T_BRUSH_STYLE_2, RIVER_T_BRUSH_STYLE_3, RIVER_T_BRUSH_STYLE_4);
  __readBorder("River_T", river_tBorder);

  __readPen("Forest", forestPenList, FRST_COLOR_1, FRST_COLOR_2, FRST_COLOR_3,
        FRST_COLOR_4, FRST_PEN_1, FRST_PEN_2, FRST_PEN_3, FRST_PEN_4,
        FRST_PEN_STYLE_1, FRST_PEN_STYLE_2, FRST_PEN_STYLE_3,
        FRST_PEN_STYLE_4);
  __readBrush("Forest", forestBrushList, FRST_BRUSH_COLOR_1, FRST_BRUSH_COLOR_2,
        FRST_BRUSH_COLOR_3, FRST_BRUSH_COLOR_4, FRST_BRUSH_STYLE_1,
        FRST_BRUSH_STYLE_2, FRST_BRUSH_STYLE_3, FRST_BRUSH_STYLE_4);
  __readBorder("Forest", forestBorder);

  __readPen("Glacier", glacierPenList, GLACIER_COLOR_1, GLACIER_COLOR_2, GLACIER_COLOR_3,
        GLACIER_COLOR_4, GLACIER_PEN_1, GLACIER_PEN_2, GLACIER_PEN_3, GLACIER_PEN_4,
        GLACIER_PEN_STYLE_1, GLACIER_PEN_STYLE_2, GLACIER_PEN_STYLE_3,
        GLACIER_PEN_STYLE_4);
  __readBrush("Glacier", glacierBrushList, GLACIER_BRUSH_COLOR_1, GLACIER_BRUSH_COLOR_2,
        GLACIER_BRUSH_COLOR_3, GLACIER_BRUSH_COLOR_4, GLACIER_BRUSH_STYLE_1,
        GLACIER_BRUSH_STYLE_2, GLACIER_BRUSH_STYLE_3, GLACIER_BRUSH_STYLE_4);
  __readBorder("Glacier", glacierBorder);

  __readPen("Pack Ice", packicePenList, PACK_ICE_COLOR_1, PACK_ICE_COLOR_2, PACK_ICE_COLOR_3,
        PACK_ICE_COLOR_4, PACK_ICE_PEN_1, PACK_ICE_PEN_2, PACK_ICE_PEN_3, PACK_ICE_PEN_4,
        PACK_ICE_PEN_STYLE_1, PACK_ICE_PEN_STYLE_2, PACK_ICE_PEN_STYLE_3,
        PACK_ICE_PEN_STYLE_4);
  __readBrush("Pack Ice", packiceBrushList, PACK_ICE_BRUSH_COLOR_1, PACK_ICE_BRUSH_COLOR_2,
        PACK_ICE_BRUSH_COLOR_3, PACK_ICE_BRUSH_COLOR_4, PACK_ICE_BRUSH_STYLE_1,
        PACK_ICE_BRUSH_STYLE_2, PACK_ICE_BRUSH_STYLE_3, PACK_ICE_BRUSH_STYLE_4);
  __readBorder("Pack Ice", packiceBorder);

  __readPen(AS_A, airAPenList, AIRA_COLOR_1, AIRA_COLOR_2, AIRA_COLOR_3, AIRA_COLOR_4,
        AIRA_PEN_1, AIRA_PEN_2, AIRA_PEN_3, AIRA_PEN_4,
        AIRA_PEN_STYLE_1, AIRA_PEN_STYLE_2, AIRA_PEN_STYLE_3, AIRA_PEN_STYLE_4);
  __readBrush(AS_A, airABrushList, AIRA_BRUSH_COLOR_1, AIRA_BRUSH_COLOR_2,
        AIRA_BRUSH_COLOR_3, AIRA_BRUSH_COLOR_4, AIRA_BRUSH_STYLE_1,
        AIRA_BRUSH_STYLE_2, AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4);
  __readBorder(AS_A, airABorder);
  __readAsOpacity(AS_A, airAOpacityList);

  __readPen(AS_B, airBPenList, AIRB_COLOR_1, AIRB_COLOR_2, AIRB_COLOR_3, AIRB_COLOR_4,
        AIRB_PEN_1, AIRB_PEN_2, AIRB_PEN_3, AIRB_PEN_4,
        AIRB_PEN_STYLE_1, AIRB_PEN_STYLE_2, AIRB_PEN_STYLE_3, AIRB_PEN_STYLE_4);
  __readBrush(AS_B, airBBrushList, AIRB_BRUSH_COLOR_1, AIRB_BRUSH_COLOR_2,
        AIRB_BRUSH_COLOR_3, AIRB_BRUSH_COLOR_4, AIRB_BRUSH_STYLE_1,
        AIRB_BRUSH_STYLE_2, AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4);
  __readBorder(AS_B, airBBorder);
  __readAsOpacity(AS_B, airBOpacityList);

  __readPen(AS_C, airCPenList, AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3, AIRC_COLOR_4,
        AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4,
        AIRC_PEN_STYLE_1, AIRC_PEN_STYLE_2, AIRC_PEN_STYLE_3, AIRC_PEN_STYLE_4);
  __readBrush(AS_C, airCBrushList, AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
        AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4, AIRC_BRUSH_STYLE_1,
        AIRC_BRUSH_STYLE_2, AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4);
  __readBorder(AS_C, airCBorder);
  __readAsOpacity(AS_C, airCOpacityList);

  __readPen(AS_D, airDPenList, AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3, AIRD_COLOR_4,
        AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4,
        AIRD_PEN_STYLE_1, AIRD_PEN_STYLE_2, AIRD_PEN_STYLE_3, AIRD_PEN_STYLE_4);
  __readBrush(AS_D, airDBrushList, AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
        AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4, AIRD_BRUSH_STYLE_1,
        AIRD_BRUSH_STYLE_2, AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4);
  __readBorder(AS_D, airDBorder);
  __readAsOpacity(AS_D, airDOpacityList);

  __readPen(AS_EL, airElPenList, AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3,
        AIREL_COLOR_4, AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4,
        AIREL_PEN_STYLE_1, AIREL_PEN_STYLE_2, AIREL_PEN_STYLE_3, AIREL_PEN_STYLE_4);
  __readBrush(AS_EL, airElBrushList, AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
        AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4, AIREL_BRUSH_STYLE_1,
        AIREL_BRUSH_STYLE_2, AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4);
  __readBorder(AS_EL, airElBorder);
  __readAsOpacity(AS_EL, airElOpacityList);

  __readPen(AS_E, airEhPenList, AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3,
        AIREH_COLOR_4, AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4,
        AIREH_PEN_STYLE_1, AIREH_PEN_STYLE_2, AIREH_PEN_STYLE_3, AIREH_PEN_STYLE_4);

  __readBrush(AS_E, airEhBrushList, AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
        AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4, AIREH_BRUSH_STYLE_1,
        AIREH_BRUSH_STYLE_2, AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4);
  __readBorder(AS_E, airEhBorder);
  __readAsOpacity(AS_E, airEhOpacityList);

  __readPen(AS_F, airFPenList, AIRF_COLOR_1, AIRF_COLOR_2, AIRF_COLOR_3,
        AIRF_COLOR_4, AIRF_PEN_1, AIRF_PEN_2, AIRF_PEN_3, AIRF_PEN_4,
        AIRF_PEN_STYLE_1, AIRF_PEN_STYLE_2, AIRF_PEN_STYLE_3, AIRF_PEN_STYLE_4);
  __readBrush(AS_F, airFBrushList, AIRF_BRUSH_COLOR_1, AIRF_BRUSH_COLOR_2,
        AIRF_BRUSH_COLOR_3, AIRF_BRUSH_COLOR_4, AIRF_BRUSH_STYLE_1,
        AIRF_BRUSH_STYLE_2, AIRF_BRUSH_STYLE_3, AIRF_BRUSH_STYLE_4);
  __readBorder(AS_F, airFBorder);
  __readAsOpacity(AS_F, airFOpacityList);

  __readPen(AS_FIR, airFirPenList, AIRFIR_COLOR_1, AIRFIR_COLOR_2, AIRFIR_COLOR_3,
        AIRFIR_COLOR_4, AIRFIR_PEN_1, AIRFIR_PEN_2, AIRFIR_PEN_3, AIRFIR_PEN_4,
        AIRFIR_PEN_STYLE_1, AIRFIR_PEN_STYLE_2, AIRFIR_PEN_STYLE_3, AIRFIR_PEN_STYLE_4);
  __readBrush(AS_FIR, airFirBrushList, AIRFIR_BRUSH_COLOR_1, AIRFIR_BRUSH_COLOR_2,
        AIRFIR_BRUSH_COLOR_3, AIRFIR_BRUSH_COLOR_4, AIRFIR_BRUSH_STYLE_1,
        AIRFIR_BRUSH_STYLE_2, AIRFIR_BRUSH_STYLE_3, AIRFIR_BRUSH_STYLE_4);
  __readBorder(AS_FIR, airFirBorder);
  __readAsOpacity(AS_FIR, airFirOpacityList);

  __readPen(AS_CTR_C, ctrCPenList, CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3, CTRC_COLOR_4,
        CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4,
        CTRC_PEN_STYLE_1, CTRC_PEN_STYLE_2, CTRC_PEN_STYLE_3, CTRC_PEN_STYLE_4);
  __readBrush(AS_CTR_C, ctrCBrushList, CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
        CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4, CTRC_BRUSH_STYLE_1,
        CTRC_BRUSH_STYLE_2, CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4);
  __readBorder(AS_CTR_C, ctrCBorder);
  __readAsOpacity(AS_CTR_C, ctrCOpacityList);

  __readPen(AS_CTR_D, ctrDPenList, CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3, CTRD_COLOR_4,
        CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4,
        CTRD_PEN_STYLE_1, CTRD_PEN_STYLE_2, CTRD_PEN_STYLE_3, CTRD_PEN_STYLE_4);
  __readBrush(AS_CTR_D, ctrDBrushList, CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
        CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4, CTRD_BRUSH_STYLE_1,
        CTRD_BRUSH_STYLE_2, CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4);
  __readBorder(AS_CTR_D, ctrDBorder);
  __readAsOpacity(AS_CTR_D, ctrDOpacityList);

  __readPen(AS_DANGER, dangerPenList, DNG_COLOR_1, DNG_COLOR_2, DNG_COLOR_3, DNG_COLOR_4,
        DNG_PEN_1, DNG_PEN_2, DNG_PEN_3, DNG_PEN_4,
        DNG_PEN_STYLE_1, DNG_PEN_STYLE_2, DNG_PEN_STYLE_3, DNG_PEN_STYLE_4);
  __readBrush(AS_DANGER, dangerBrushList, DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
        DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4, DNG_BRUSH_STYLE_1,
        DNG_BRUSH_STYLE_2, DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4);
  __readBorder(AS_DANGER, dangerBorder);
  __readAsOpacity(AS_DANGER, dangerOpacityList);

  __readPen(AS_LF, lowFPenList, LOWF_COLOR_1, LOWF_COLOR_2, LOWF_COLOR_3, LOWF_COLOR_4,
        LOWF_PEN_1, LOWF_PEN_2, LOWF_PEN_3, LOWF_PEN_4,
        LOWF_PEN_STYLE_1, LOWF_PEN_STYLE_2, LOWF_PEN_STYLE_3, LOWF_PEN_STYLE_4);
  __readBrush(AS_LF, lowFBrushList, LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
        LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4, LOWF_BRUSH_STYLE_1,
        LOWF_BRUSH_STYLE_2, LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4);
  __readBorder(AS_LF, lowFBorder);
  __readAsOpacity(AS_LF, lowFOpacityList);

  __readPen(AS_RESTRICTED, restrPenList, RES_COLOR_1, RES_COLOR_2, RES_COLOR_3, RES_COLOR_4,
        RES_PEN_1, RES_PEN_2, RES_PEN_3, RES_PEN_4,
        RES_PEN_STYLE_1, RES_PEN_STYLE_2, RES_PEN_STYLE_3, RES_PEN_STYLE_4);
  __readBrush(AS_RESTRICTED, restrBrushList, RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
        RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4, RES_BRUSH_STYLE_1,
        RES_BRUSH_STYLE_2, RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4);
  __readBorder(AS_RESTRICTED, restrBorder);
  __readAsOpacity(AS_RESTRICTED, restrOpacityList);

  __readPen(AS_TMZ, tmzPenList, TMZ_COLOR_1, TMZ_COLOR_2, TMZ_COLOR_3, TMZ_COLOR_4,
        TMZ_PEN_1, TMZ_PEN_2, TMZ_PEN_3, TMZ_PEN_4,
        TMZ_PEN_STYLE_1, TMZ_PEN_STYLE_2, TMZ_PEN_STYLE_3, TMZ_PEN_STYLE_4);
  __readBrush(AS_TMZ, tmzBrushList, TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
        TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4, TMZ_BRUSH_STYLE_1,
        TMZ_BRUSH_STYLE_2, TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4);
  __readBorder(AS_TMZ, tmzBorder);
  __readAsOpacity(AS_TMZ, tmzOpacityList);

  __readPen(AS_GS, gsPenList, GLIDER_SECTOR_COLOR_1, GLIDER_SECTOR_COLOR_2, GLIDER_SECTOR_COLOR_3, GLIDER_SECTOR_COLOR_4,
        GLIDER_SECTOR_PEN_1, GLIDER_SECTOR_PEN_2, GLIDER_SECTOR_PEN_3, GLIDER_SECTOR_PEN_4,
        GLIDER_SECTOR_PEN_STYLE_1, GLIDER_SECTOR_PEN_STYLE_2, GLIDER_SECTOR_PEN_STYLE_3, GLIDER_SECTOR_PEN_STYLE_4);
  __readBrush(AS_GS, gsBrushList, GLIDER_SECTOR_BRUSH_COLOR_1, GLIDER_SECTOR_BRUSH_COLOR_2,
        GLIDER_SECTOR_BRUSH_COLOR_3, GLIDER_SECTOR_BRUSH_COLOR_4, GLIDER_SECTOR_BRUSH_STYLE_1,
        GLIDER_SECTOR_BRUSH_STYLE_2, GLIDER_SECTOR_BRUSH_STYLE_3, GLIDER_SECTOR_BRUSH_STYLE_4);
  __readBorder(AS_GS, gsBorder);
  __readAsOpacity(AS_GS, gsOpacityList);

  __readPen(AS_WW, wwPenList, WAVE_WINDOW_COLOR_1, WAVE_WINDOW_COLOR_2, WAVE_WINDOW_COLOR_3, WAVE_WINDOW_COLOR_4,
        WAVE_WINDOW_PEN_1, WAVE_WINDOW_PEN_2, WAVE_WINDOW_PEN_3, WAVE_WINDOW_PEN_4,
        WAVE_WINDOW_PEN_STYLE_1, WAVE_WINDOW_PEN_STYLE_2, WAVE_WINDOW_PEN_STYLE_3, WAVE_WINDOW_PEN_STYLE_4);
  __readBrush(AS_WW, wwBrushList, WAVE_WINDOW_BRUSH_COLOR_1, WAVE_WINDOW_BRUSH_COLOR_2,
        WAVE_WINDOW_BRUSH_COLOR_3, WAVE_WINDOW_BRUSH_COLOR_4, WAVE_WINDOW_BRUSH_STYLE_1,
        WAVE_WINDOW_BRUSH_STYLE_2, WAVE_WINDOW_BRUSH_STYLE_3, WAVE_WINDOW_BRUSH_STYLE_4);
  __readBorder(AS_WW, wwBorder);
  __readAsOpacity(AS_WW, wwOpacityList);


  __readPen("FAIAreaLow500", faiAreaLow500PenList, FAI_LOW_500_COLOR_1, FAI_LOW_500_COLOR_2, FAI_LOW_500_COLOR_3, FAI_LOW_500_COLOR_4,
        FAI_LOW_500_PEN_1, FAI_LOW_500_PEN_2, FAI_LOW_500_PEN_3, FAI_LOW_500_PEN_4,
        FAI_LOW_500_PEN_STYLE_1, FAI_LOW_500_PEN_STYLE_2, FAI_LOW_500_PEN_STYLE_3, FAI_LOW_500_PEN_STYLE_4);
  __readBrush("FAIAreaLow500", faiAreaLow500BrushList, FAI_LOW_500_BRUSH_COLOR_1, FAI_LOW_500_BRUSH_COLOR_2,
        FAI_LOW_500_BRUSH_COLOR_3, FAI_LOW_500_BRUSH_COLOR_4, FAI_LOW_500_BRUSH_STYLE_1,
        FAI_LOW_500_BRUSH_STYLE_2, FAI_LOW_500_BRUSH_STYLE_3, FAI_LOW_500_BRUSH_STYLE_4);
  __readBorder("FAIAreaLow500", faiAreaLow500Border);

  __readPen("FAIAreaHigh500", faiAreaHigh500PenList, FAI_HIGH_500_COLOR_1, FAI_HIGH_500_COLOR_2, FAI_HIGH_500_COLOR_3, FAI_HIGH_500_COLOR_4,
        FAI_HIGH_500_PEN_1, FAI_HIGH_500_PEN_2, FAI_HIGH_500_PEN_3, FAI_HIGH_500_PEN_4,
        FAI_HIGH_500_PEN_STYLE_1, FAI_HIGH_500_PEN_STYLE_2, FAI_HIGH_500_PEN_STYLE_3, FAI_HIGH_500_PEN_STYLE_4);
  __readBrush("FAIAreaHigh500", faiAreaHigh500BrushList, FAI_HIGH_500_BRUSH_COLOR_1, FAI_HIGH_500_BRUSH_COLOR_2,
        FAI_HIGH_500_BRUSH_COLOR_3, FAI_HIGH_500_BRUSH_COLOR_4, FAI_HIGH_500_BRUSH_STYLE_1,
        FAI_HIGH_500_BRUSH_STYLE_2, FAI_HIGH_500_BRUSH_STYLE_3, FAI_HIGH_500_BRUSH_STYLE_4);
  __readBorder("FAIAreaHigh500", faiAreaHigh500Border);
}

void ConfigMapElement::__readPrintItems()
{
  __readPen("Trail", trailPenList, PRINT_TRAIL_COLOR_1, PRINT_TRAIL_COLOR_2, PRINT_TRAIL_COLOR_2, PRINT_TRAIL_COLOR_2,
        PRINT_TRAIL_PEN_1, PRINT_TRAIL_PEN_2, PRINT_TRAIL_PEN_2, PRINT_TRAIL_PEN_2,
        PRINT_TRAIL_PEN_STYLE_1, PRINT_TRAIL_PEN_STYLE_2, PRINT_TRAIL_PEN_STYLE_2, PRINT_TRAIL_PEN_STYLE_2);
  __readBorder("Trail", trailBorder);

  __readPen("Road", roadPenList, PRINT_ROAD_COLOR_1, PRINT_ROAD_COLOR_2, PRINT_ROAD_COLOR_2, PRINT_ROAD_COLOR_2,
        PRINT_ROAD_PEN_1, PRINT_ROAD_PEN_2, PRINT_ROAD_PEN_2, PRINT_ROAD_PEN_2,
        PRINT_ROAD_PEN_STYLE_1, PRINT_ROAD_PEN_STYLE_2, PRINT_ROAD_PEN_STYLE_2, PRINT_ROAD_PEN_STYLE_2);
  __readBorder("Road", roadBorder);

  __readPen("River", riverPenList, PRINT_RIVER_COLOR_1, PRINT_RIVER_COLOR_2, PRINT_RIVER_COLOR_2,
        PRINT_RIVER_COLOR_2, PRINT_RIVER_PEN_1, PRINT_RIVER_PEN_2, PRINT_RIVER_PEN_2, PRINT_RIVER_PEN_2,
        PRINT_RIVER_PEN_STYLE_1, PRINT_RIVER_PEN_STYLE_2, PRINT_RIVER_PEN_STYLE_2, PRINT_RIVER_PEN_STYLE_2);
  __readBrush("River", riverBrushList, PRINT_RIVER_BRUSH_COLOR_1, PRINT_RIVER_BRUSH_COLOR_2,
        PRINT_RIVER_BRUSH_COLOR_2, PRINT_RIVER_BRUSH_COLOR_2, PRINT_RIVER_BRUSH_STYLE_1,
        PRINT_RIVER_BRUSH_STYLE_2, PRINT_RIVER_BRUSH_STYLE_2, PRINT_RIVER_BRUSH_STYLE_2);
  __readBorder("River", riverBorder);

  __readPen("Canal", canalPenList, PRINT_CANAL_COLOR_1, PRINT_CANAL_COLOR_2, PRINT_CANAL_COLOR_2,
        PRINT_CANAL_COLOR_2, PRINT_CANAL_PEN_1, PRINT_CANAL_PEN_2, PRINT_CANAL_PEN_2, PRINT_CANAL_PEN_2,
        PRINT_CANAL_PEN_STYLE_1, PRINT_CANAL_PEN_STYLE_2, PRINT_CANAL_PEN_STYLE_2, PRINT_CANAL_PEN_STYLE_2);
  __readBorder("Canal", canalBorder);

  __readPen("Rail", railPenList, PRINT_RAIL_COLOR_1, PRINT_RAIL_COLOR_2, PRINT_RAIL_COLOR_2, PRINT_RAIL_COLOR_2,
        PRINT_RAIL_PEN_1, PRINT_RAIL_PEN_2, PRINT_RAIL_PEN_2, PRINT_RAIL_PEN_2,
        PRINT_RAIL_PEN_STYLE_1, PRINT_RAIL_PEN_STYLE_2, PRINT_RAIL_PEN_STYLE_2, PRINT_RAIL_PEN_STYLE_2);
  __readBorder("Rail", railBorder);

  __readPen("Rail_D", rail_dPenList, PRINT_RAIL_D_COLOR_1, PRINT_RAIL_D_COLOR_2, PRINT_RAIL_D_COLOR_2,
        PRINT_RAIL_D_COLOR_2, PRINT_RAIL_D_PEN_1, PRINT_RAIL_D_PEN_2, PRINT_RAIL_D_PEN_2, PRINT_RAIL_D_PEN_2,
        PRINT_RAIL_D_PEN_STYLE_1, PRINT_RAIL_D_PEN_STYLE_2, PRINT_RAIL_D_PEN_STYLE_2, PRINT_RAIL_D_PEN_STYLE_2);
  __readBorder("Rail_D", rail_dBorder);

  __readPen("Aerial Cable", aerialcablePenList, PRINT_AERIAL_CABLE_COLOR_1, PRINT_AERIAL_CABLE_COLOR_2,
        PRINT_AERIAL_CABLE_COLOR_2, PRINT_AERIAL_CABLE_COLOR_2, PRINT_AERIAL_CABLE_PEN_1,
        PRINT_AERIAL_CABLE_PEN_2, PRINT_AERIAL_CABLE_PEN_2, PRINT_AERIAL_CABLE_PEN_2,
        PRINT_AERIAL_CABLE_PEN_STYLE_1, PRINT_AERIAL_CABLE_PEN_STYLE_2,
        PRINT_AERIAL_CABLE_PEN_STYLE_2, PRINT_AERIAL_CABLE_PEN_STYLE_2);
  __readBorder("Aerial Cable", aerialcableBorder);

  __readPen("Highway", motorwayPenList, PRINT_HIGH_COLOR_1, PRINT_HIGH_COLOR_2, PRINT_HIGH_COLOR_2, PRINT_HIGH_COLOR_2,
        PRINT_HIGH_PEN_1, PRINT_HIGH_PEN_2, PRINT_HIGH_PEN_2, PRINT_HIGH_PEN_2,
        PRINT_HIGH_PEN_STYLE_1, PRINT_HIGH_PEN_STYLE_2, PRINT_HIGH_PEN_STYLE_2, PRINT_HIGH_PEN_STYLE_2);
  __readBorder("Highway", motorwayBorder);

  // PenStyle and BrushStyle are not used for cities ...
  __readPen("City", cityPenList, PRINT_CITY_COLOR_1, PRINT_CITY_COLOR_2, PRINT_CITY_COLOR_2,
        PRINT_CITY_COLOR_2, PRINT_CITY_PEN_1, PRINT_CITY_PEN_2, PRINT_CITY_PEN_2, PRINT_CITY_PEN_2,
        Qt::SolidLine, Qt::SolidLine, Qt::SolidLine, Qt::SolidLine);
  __readBrush("City", cityBrushList, PRINT_CITY_BRUSH_COLOR_1, PRINT_CITY_BRUSH_COLOR_2,
      PRINT_CITY_BRUSH_COLOR_2, PRINT_CITY_BRUSH_COLOR_2, Qt::SolidPattern,
      Qt::SolidPattern, Qt::SolidPattern, Qt::SolidPattern);
  __readBorder("City", cityBorder);

  __readPen("River_T", river_tPenList, PRINT_RIVER_T_COLOR_1, PRINT_RIVER_T_COLOR_2, PRINT_RIVER_T_COLOR_2,
        PRINT_RIVER_T_COLOR_2, PRINT_RIVER_T_PEN_1, PRINT_RIVER_T_PEN_2, PRINT_RIVER_T_PEN_2, PRINT_RIVER_T_PEN_2,
        PRINT_RIVER_T_PEN_STYLE_1, PRINT_RIVER_T_PEN_STYLE_2, PRINT_RIVER_T_PEN_STYLE_2, PRINT_RIVER_T_PEN_STYLE_2);
  __readBrush("River_T", river_tBrushList, PRINT_RIVER_T_BRUSH_COLOR_1, PRINT_RIVER_T_BRUSH_COLOR_2,
        PRINT_RIVER_T_BRUSH_COLOR_2, PRINT_RIVER_T_BRUSH_COLOR_2, PRINT_RIVER_T_BRUSH_STYLE_1,
        PRINT_RIVER_T_BRUSH_STYLE_2, PRINT_RIVER_T_BRUSH_STYLE_2, PRINT_RIVER_T_BRUSH_STYLE_2);
  __readBorder("River_T", river_tBorder);

  __readPen("Forest", forestPenList, PRINT_FRST_COLOR_1, PRINT_FRST_COLOR_2, PRINT_FRST_COLOR_2,
        PRINT_FRST_COLOR_2, PRINT_FRST_PEN_1, PRINT_FRST_PEN_2, PRINT_FRST_PEN_2, PRINT_FRST_PEN_2,
        PRINT_FRST_PEN_STYLE_1, PRINT_FRST_PEN_STYLE_2, PRINT_FRST_PEN_STYLE_2,
        PRINT_FRST_PEN_STYLE_2);
  __readBrush("Forest", forestBrushList, PRINT_FRST_BRUSH_COLOR_1, PRINT_FRST_BRUSH_COLOR_2,
        PRINT_FRST_BRUSH_COLOR_2, PRINT_FRST_BRUSH_COLOR_2, PRINT_FRST_BRUSH_STYLE_1,
        PRINT_FRST_BRUSH_STYLE_2, PRINT_FRST_BRUSH_STYLE_2, PRINT_FRST_BRUSH_STYLE_2);
  __readBorder("Forest", forestBorder);

  __readPen("Glacier", glacierPenList, PRINT_GLACIER_COLOR_1, PRINT_GLACIER_COLOR_2, PRINT_GLACIER_COLOR_2,
        PRINT_GLACIER_COLOR_2, PRINT_GLACIER_PEN_1, PRINT_GLACIER_PEN_2, PRINT_GLACIER_PEN_2, PRINT_GLACIER_PEN_2,
        PRINT_GLACIER_PEN_STYLE_1, PRINT_GLACIER_PEN_STYLE_2, PRINT_GLACIER_PEN_STYLE_2,
        PRINT_GLACIER_PEN_STYLE_2);
  __readBrush("Glacier", glacierBrushList, PRINT_GLACIER_BRUSH_COLOR_1, PRINT_GLACIER_BRUSH_COLOR_2,
        PRINT_GLACIER_BRUSH_COLOR_2, PRINT_GLACIER_BRUSH_COLOR_2, PRINT_GLACIER_BRUSH_STYLE_1,
        PRINT_GLACIER_BRUSH_STYLE_2, PRINT_GLACIER_BRUSH_STYLE_2, PRINT_GLACIER_BRUSH_STYLE_2);
  __readBorder("Glacier", glacierBorder);

  __readPen("Pack Ice", packicePenList, PRINT_PACK_ICE_COLOR_1, PRINT_PACK_ICE_COLOR_2, PRINT_PACK_ICE_COLOR_2,
        PRINT_PACK_ICE_COLOR_2, PRINT_PACK_ICE_PEN_1, PRINT_PACK_ICE_PEN_2, PRINT_PACK_ICE_PEN_2, PRINT_PACK_ICE_PEN_2,
        PRINT_PACK_ICE_PEN_STYLE_1, PRINT_PACK_ICE_PEN_STYLE_2, PRINT_PACK_ICE_PEN_STYLE_2,
        PRINT_PACK_ICE_PEN_STYLE_2);
  __readBrush("Pack Ice", packiceBrushList, PRINT_PACK_ICE_BRUSH_COLOR_1, PRINT_PACK_ICE_BRUSH_COLOR_2,
        PRINT_PACK_ICE_BRUSH_COLOR_2, PRINT_PACK_ICE_BRUSH_COLOR_2, PRINT_PACK_ICE_BRUSH_STYLE_1,
        PRINT_PACK_ICE_BRUSH_STYLE_2, PRINT_PACK_ICE_BRUSH_STYLE_2, PRINT_PACK_ICE_BRUSH_STYLE_2);
  __readBorder("Pack Ice", packiceBorder);

  __readPen(AS_A, airAPenList, PRINT_AIRA_COLOR_1, PRINT_AIRA_COLOR_2, PRINT_AIRA_COLOR_2, PRINT_AIRA_COLOR_2,
        PRINT_AIRA_PEN_1, PRINT_AIRA_PEN_2, PRINT_AIRA_PEN_2, PRINT_AIRA_PEN_2,
        PRINT_AIRA_PEN_STYLE_1, PRINT_AIRA_PEN_STYLE_2, PRINT_AIRA_PEN_STYLE_2, PRINT_AIRA_PEN_STYLE_2);
  __readBrush(AS_A, airABrushList, PRINT_AIRA_BRUSH_COLOR_1, PRINT_AIRA_BRUSH_COLOR_2,
        PRINT_AIRA_BRUSH_COLOR_2, PRINT_AIRA_BRUSH_COLOR_2, PRINT_AIRA_BRUSH_STYLE_1,
        PRINT_AIRA_BRUSH_STYLE_2, PRINT_AIRA_BRUSH_STYLE_2, PRINT_AIRA_BRUSH_STYLE_2);
  __readBorder(AS_A, airABorder);
  __readAsOpacity(AS_A, airAOpacityList);

  __readPen(AS_B, airBPenList, PRINT_AIRB_COLOR_1, PRINT_AIRB_COLOR_2, PRINT_AIRB_COLOR_2, PRINT_AIRB_COLOR_2,
        PRINT_AIRB_PEN_1, PRINT_AIRB_PEN_2, PRINT_AIRB_PEN_2, PRINT_AIRB_PEN_2,
        PRINT_AIRB_PEN_STYLE_1, PRINT_AIRB_PEN_STYLE_2, PRINT_AIRB_PEN_STYLE_2, PRINT_AIRB_PEN_STYLE_2);
  __readBrush(AS_B, airBBrushList, PRINT_AIRB_BRUSH_COLOR_1, PRINT_AIRB_BRUSH_COLOR_2,
        PRINT_AIRB_BRUSH_COLOR_2, PRINT_AIRB_BRUSH_COLOR_2, PRINT_AIRB_BRUSH_STYLE_1,
        PRINT_AIRB_BRUSH_STYLE_2, PRINT_AIRB_BRUSH_STYLE_2, PRINT_AIRB_BRUSH_STYLE_2);
  __readBorder(AS_B, airBBorder);
  __readAsOpacity(AS_B, airBOpacityList);

  __readPen(AS_C, airCPenList, PRINT_AIRC_COLOR_1, PRINT_AIRC_COLOR_2, PRINT_AIRC_COLOR_2, PRINT_AIRC_COLOR_2,
        PRINT_AIRC_PEN_1, PRINT_AIRC_PEN_2, PRINT_AIRC_PEN_2, PRINT_AIRC_PEN_2,
        PRINT_AIRC_PEN_STYLE_1, PRINT_AIRC_PEN_STYLE_2, PRINT_AIRC_PEN_STYLE_2, PRINT_AIRC_PEN_STYLE_2);
  __readBrush(AS_C, airCBrushList, PRINT_AIRC_BRUSH_COLOR_1, PRINT_AIRC_BRUSH_COLOR_2,
        PRINT_AIRC_BRUSH_COLOR_2, PRINT_AIRC_BRUSH_COLOR_2, PRINT_AIRC_BRUSH_STYLE_1,
        PRINT_AIRC_BRUSH_STYLE_2, PRINT_AIRC_BRUSH_STYLE_2, PRINT_AIRC_BRUSH_STYLE_2);
  __readBorder(AS_C, airCBorder);
  __readAsOpacity(AS_C, airCOpacityList);

  __readPen(AS_D, airDPenList, PRINT_AIRD_COLOR_1, PRINT_AIRD_COLOR_2, PRINT_AIRD_COLOR_2, PRINT_AIRD_COLOR_2,
        PRINT_AIRD_PEN_1, PRINT_AIRD_PEN_2, PRINT_AIRD_PEN_2, PRINT_AIRD_PEN_2,
        PRINT_AIRD_PEN_STYLE_1, PRINT_AIRD_PEN_STYLE_2, PRINT_AIRD_PEN_STYLE_2, PRINT_AIRD_PEN_STYLE_2);
  __readBrush(AS_D, airDBrushList, PRINT_AIRD_BRUSH_COLOR_1, PRINT_AIRD_BRUSH_COLOR_2,
        PRINT_AIRD_BRUSH_COLOR_2, PRINT_AIRD_BRUSH_COLOR_2, PRINT_AIRD_BRUSH_STYLE_1,
        PRINT_AIRD_BRUSH_STYLE_2, PRINT_AIRD_BRUSH_STYLE_2, PRINT_AIRD_BRUSH_STYLE_2);
  __readBorder(AS_D, airDBorder);
  __readAsOpacity(AS_D, airDOpacityList);

  __readPen(AS_EL, airElPenList, PRINT_AIREL_COLOR_1, PRINT_AIREL_COLOR_2, PRINT_AIREL_COLOR_2,
        PRINT_AIREL_COLOR_2, PRINT_AIREL_PEN_1, PRINT_AIREL_PEN_2, PRINT_AIREL_PEN_2, PRINT_AIREL_PEN_2,
        PRINT_AIREL_PEN_STYLE_1, PRINT_AIREL_PEN_STYLE_2, PRINT_AIREL_PEN_STYLE_2, PRINT_AIREL_PEN_STYLE_2);
  __readBrush(AS_EL, airElBrushList, PRINT_AIREL_BRUSH_COLOR_1, PRINT_AIREL_BRUSH_COLOR_2,
        PRINT_AIREL_BRUSH_COLOR_2, PRINT_AIREL_BRUSH_COLOR_2, PRINT_AIREL_BRUSH_STYLE_1,
        PRINT_AIREL_BRUSH_STYLE_2, PRINT_AIREL_BRUSH_STYLE_2, PRINT_AIREL_BRUSH_STYLE_2);
  __readBorder(AS_EL, airElBorder);
  __readAsOpacity(AS_EL, airElOpacityList);

  __readPen(AS_E, airEhPenList, PRINT_AIREH_COLOR_1, PRINT_AIREH_COLOR_2, PRINT_AIREH_COLOR_2,
        PRINT_AIREH_COLOR_2, PRINT_AIREH_PEN_1, PRINT_AIREH_PEN_2, PRINT_AIREH_PEN_2, PRINT_AIREH_PEN_2,
        PRINT_AIREH_PEN_STYLE_1, PRINT_AIREH_PEN_STYLE_2, PRINT_AIREH_PEN_STYLE_2, PRINT_AIREH_PEN_STYLE_2);

  __readBrush(AS_E, airEhBrushList, PRINT_AIREH_BRUSH_COLOR_1, PRINT_AIREH_BRUSH_COLOR_2,
        PRINT_AIREH_BRUSH_COLOR_2, PRINT_AIREH_BRUSH_COLOR_2, PRINT_AIREH_BRUSH_STYLE_1,
        PRINT_AIREH_BRUSH_STYLE_2, PRINT_AIREH_BRUSH_STYLE_2, PRINT_AIREH_BRUSH_STYLE_2);
  __readBorder(AS_E, airEhBorder);
  __readAsOpacity(AS_E, airEhOpacityList);

  __readPen(AS_F, airFPenList, PRINT_AIRF_COLOR_1, PRINT_AIRF_COLOR_2, PRINT_AIRF_COLOR_2,
        PRINT_AIRF_COLOR_2, PRINT_AIRF_PEN_1, PRINT_AIRF_PEN_2, PRINT_AIRF_PEN_2, PRINT_AIRF_PEN_2,
        PRINT_AIRF_PEN_STYLE_1, PRINT_AIRF_PEN_STYLE_2, PRINT_AIRF_PEN_STYLE_2, PRINT_AIRF_PEN_STYLE_2);
  __readBrush(AS_F, airFBrushList, PRINT_AIRF_BRUSH_COLOR_1, PRINT_AIRF_BRUSH_COLOR_2,
        PRINT_AIRF_BRUSH_COLOR_2, PRINT_AIRF_BRUSH_COLOR_2, PRINT_AIRF_BRUSH_STYLE_1,
        PRINT_AIRF_BRUSH_STYLE_2, PRINT_AIRF_BRUSH_STYLE_2, PRINT_AIRF_BRUSH_STYLE_2);
  __readBorder(AS_F, airFBorder);
  __readAsOpacity(AS_F, airFOpacityList);

  __readPen(AS_FIR, airFirPenList, PRINT_AIRFIR_COLOR_1, PRINT_AIRFIR_COLOR_2, PRINT_AIRFIR_COLOR_2,
        PRINT_AIRFIR_COLOR_2, PRINT_AIRFIR_PEN_1, PRINT_AIRFIR_PEN_2, PRINT_AIRFIR_PEN_2, PRINT_AIRFIR_PEN_2,
        PRINT_AIRFIR_PEN_STYLE_1, PRINT_AIRFIR_PEN_STYLE_2, PRINT_AIRFIR_PEN_STYLE_2, PRINT_AIRFIR_PEN_STYLE_2);
  __readBrush(AS_FIR, airFirBrushList, PRINT_AIRFIR_BRUSH_COLOR_1, PRINT_AIRFIR_BRUSH_COLOR_2,
        PRINT_AIRFIR_BRUSH_COLOR_2, PRINT_AIRFIR_BRUSH_COLOR_2, PRINT_AIRFIR_BRUSH_STYLE_1,
        PRINT_AIRFIR_BRUSH_STYLE_2, PRINT_AIRFIR_BRUSH_STYLE_2, PRINT_AIRFIR_BRUSH_STYLE_2);
  __readBorder(AS_FIR, airFirBorder);
  __readAsOpacity(AS_FIR, airFirOpacityList);

  __readPen(AS_CTR_C, ctrCPenList, PRINT_CTRC_COLOR_1, PRINT_CTRC_COLOR_2, PRINT_CTRC_COLOR_2, PRINT_CTRC_COLOR_2,
        PRINT_CTRC_PEN_1, PRINT_CTRC_PEN_2, PRINT_CTRC_PEN_2, PRINT_CTRC_PEN_2,
        PRINT_CTRC_PEN_STYLE_1, PRINT_CTRC_PEN_STYLE_2, PRINT_CTRC_PEN_STYLE_2, PRINT_CTRC_PEN_STYLE_2);
  __readBrush(AS_CTR_C, ctrCBrushList, PRINT_CTRC_BRUSH_COLOR_1, PRINT_CTRC_BRUSH_COLOR_2,
        PRINT_CTRC_BRUSH_COLOR_2, PRINT_CTRC_BRUSH_COLOR_2, PRINT_CTRC_BRUSH_STYLE_1,
        PRINT_CTRC_BRUSH_STYLE_2, PRINT_CTRC_BRUSH_STYLE_2, PRINT_CTRC_BRUSH_STYLE_2);
  __readBorder(AS_CTR_C, ctrCBorder);
  __readAsOpacity(AS_CTR_C, ctrCOpacityList);

  __readPen(AS_CTR_D, ctrDPenList, PRINT_CTRD_COLOR_1, PRINT_CTRD_COLOR_2, PRINT_CTRD_COLOR_2, PRINT_CTRD_COLOR_2,
        PRINT_CTRD_PEN_1, PRINT_CTRD_PEN_2, PRINT_CTRD_PEN_2, PRINT_CTRD_PEN_2,
        PRINT_CTRD_PEN_STYLE_1, PRINT_CTRD_PEN_STYLE_2, PRINT_CTRD_PEN_STYLE_2, PRINT_CTRD_PEN_STYLE_2);
  __readBrush(AS_CTR_D, ctrDBrushList, PRINT_CTRD_BRUSH_COLOR_1, PRINT_CTRD_BRUSH_COLOR_2,
        PRINT_CTRD_BRUSH_COLOR_2, PRINT_CTRD_BRUSH_COLOR_2, PRINT_CTRD_BRUSH_STYLE_1,
        PRINT_CTRD_BRUSH_STYLE_2, PRINT_CTRD_BRUSH_STYLE_2, PRINT_CTRD_BRUSH_STYLE_2);
  __readBorder(AS_CTR_D, ctrDBorder);
  __readAsOpacity(AS_CTR_D, ctrDOpacityList);

  __readPen(AS_DANGER, dangerPenList, PRINT_DNG_COLOR_1, PRINT_DNG_COLOR_2, PRINT_DNG_COLOR_2, PRINT_DNG_COLOR_2,
        PRINT_DNG_PEN_1, PRINT_DNG_PEN_2, PRINT_DNG_PEN_2, PRINT_DNG_PEN_2,
        PRINT_DNG_PEN_STYLE_1, PRINT_DNG_PEN_STYLE_2, PRINT_DNG_PEN_STYLE_2, PRINT_DNG_PEN_STYLE_2);
  __readBrush(AS_DANGER, dangerBrushList, PRINT_DNG_BRUSH_COLOR_1, PRINT_DNG_BRUSH_COLOR_2,
        PRINT_DNG_BRUSH_COLOR_2, PRINT_DNG_BRUSH_COLOR_2, PRINT_DNG_BRUSH_STYLE_1,
        PRINT_DNG_BRUSH_STYLE_2, PRINT_DNG_BRUSH_STYLE_2, PRINT_DNG_BRUSH_STYLE_2);
  __readBorder(AS_DANGER, dangerBorder);
  __readAsOpacity(AS_DANGER, dangerOpacityList);

  __readPen(AS_LF, lowFPenList, PRINT_LOWF_COLOR_1, PRINT_LOWF_COLOR_2, PRINT_LOWF_COLOR_2, PRINT_LOWF_COLOR_2,
        PRINT_LOWF_PEN_1, PRINT_LOWF_PEN_2, PRINT_LOWF_PEN_2, PRINT_LOWF_PEN_2,
        PRINT_LOWF_PEN_STYLE_1, PRINT_LOWF_PEN_STYLE_2, PRINT_LOWF_PEN_STYLE_2, PRINT_LOWF_PEN_STYLE_2);
  __readBrush(AS_LF, lowFBrushList, PRINT_LOWF_BRUSH_COLOR_1, PRINT_LOWF_BRUSH_COLOR_2,
        PRINT_LOWF_BRUSH_COLOR_2, PRINT_LOWF_BRUSH_COLOR_2, PRINT_LOWF_BRUSH_STYLE_1,
        PRINT_LOWF_BRUSH_STYLE_2, PRINT_LOWF_BRUSH_STYLE_2, PRINT_LOWF_BRUSH_STYLE_2);
  __readBorder(AS_LF, lowFBorder);
  __readAsOpacity(AS_LF, lowFOpacityList);

  __readPen(AS_RESTRICTED, restrPenList, PRINT_RES_COLOR_1, PRINT_RES_COLOR_2, PRINT_RES_COLOR_2, PRINT_RES_COLOR_2,
        PRINT_RES_PEN_1, PRINT_RES_PEN_2, PRINT_RES_PEN_2, PRINT_RES_PEN_2,
        PRINT_RES_PEN_STYLE_1, PRINT_RES_PEN_STYLE_2, PRINT_RES_PEN_STYLE_2, PRINT_RES_PEN_STYLE_2);
  __readBrush(AS_RESTRICTED, restrBrushList, PRINT_RES_BRUSH_COLOR_1, PRINT_RES_BRUSH_COLOR_2,
        PRINT_RES_BRUSH_COLOR_2, PRINT_RES_BRUSH_COLOR_2, PRINT_RES_BRUSH_STYLE_1,
        PRINT_RES_BRUSH_STYLE_2, PRINT_RES_BRUSH_STYLE_2, PRINT_RES_BRUSH_STYLE_2);
  __readBorder(AS_RESTRICTED, restrBorder);
  __readAsOpacity(AS_RESTRICTED, restrOpacityList);

  __readPen(AS_TMZ, tmzPenList, PRINT_TMZ_COLOR_1, PRINT_TMZ_COLOR_2, PRINT_TMZ_COLOR_2, PRINT_TMZ_COLOR_2,
        PRINT_TMZ_PEN_1, PRINT_TMZ_PEN_2, PRINT_TMZ_PEN_2, PRINT_TMZ_PEN_2,
        PRINT_TMZ_PEN_STYLE_1, PRINT_TMZ_PEN_STYLE_2, PRINT_TMZ_PEN_STYLE_2, PRINT_TMZ_PEN_STYLE_2);
  __readBrush(AS_TMZ, tmzBrushList, PRINT_TMZ_BRUSH_COLOR_1, PRINT_TMZ_BRUSH_COLOR_2,
        PRINT_TMZ_BRUSH_COLOR_2, PRINT_TMZ_BRUSH_COLOR_2, PRINT_TMZ_BRUSH_STYLE_1,
        PRINT_TMZ_BRUSH_STYLE_2, PRINT_TMZ_BRUSH_STYLE_2, PRINT_TMZ_BRUSH_STYLE_2);
  __readBorder(AS_TMZ, tmzBorder);
  __readAsOpacity(AS_TMZ, tmzOpacityList);

  __readPen(AS_GS, gsPenList, PRINT_GLIDER_SECTOR_COLOR_1, PRINT_GLIDER_SECTOR_COLOR_2, PRINT_GLIDER_SECTOR_COLOR_2, PRINT_GLIDER_SECTOR_COLOR_2,
        PRINT_GLIDER_SECTOR_PEN_1, PRINT_GLIDER_SECTOR_PEN_2, PRINT_GLIDER_SECTOR_PEN_2, PRINT_GLIDER_SECTOR_PEN_2,
        PRINT_GLIDER_SECTOR_PEN_STYLE_1, PRINT_GLIDER_SECTOR_PEN_STYLE_2, PRINT_GLIDER_SECTOR_PEN_STYLE_2, PRINT_GLIDER_SECTOR_PEN_STYLE_2);
  __readBrush(AS_GS, gsBrushList, PRINT_GLIDER_SECTOR_BRUSH_COLOR_1, PRINT_GLIDER_SECTOR_BRUSH_COLOR_2,
        PRINT_GLIDER_SECTOR_BRUSH_COLOR_2, PRINT_GLIDER_SECTOR_BRUSH_COLOR_2, PRINT_GLIDER_SECTOR_BRUSH_STYLE_1,
        PRINT_GLIDER_SECTOR_BRUSH_STYLE_2, PRINT_GLIDER_SECTOR_BRUSH_STYLE_2, PRINT_GLIDER_SECTOR_BRUSH_STYLE_2);
  __readBorder(AS_GS, gsBorder);
  __readAsOpacity(AS_GS, gsOpacityList);

  __readPen(AS_WW, wwPenList, PRINT_WAVE_WINDOW_COLOR_1, PRINT_WAVE_WINDOW_COLOR_2, PRINT_WAVE_WINDOW_COLOR_2, PRINT_WAVE_WINDOW_COLOR_2,
        PRINT_WAVE_WINDOW_PEN_1, PRINT_WAVE_WINDOW_PEN_2, PRINT_WAVE_WINDOW_PEN_2, PRINT_WAVE_WINDOW_PEN_2,
        PRINT_WAVE_WINDOW_PEN_STYLE_1, PRINT_WAVE_WINDOW_PEN_STYLE_2, PRINT_WAVE_WINDOW_PEN_STYLE_2, PRINT_WAVE_WINDOW_PEN_STYLE_2);
  __readBrush(AS_WW, wwBrushList, PRINT_WAVE_WINDOW_BRUSH_COLOR_1, PRINT_WAVE_WINDOW_BRUSH_COLOR_2,
        PRINT_WAVE_WINDOW_BRUSH_COLOR_2, PRINT_WAVE_WINDOW_BRUSH_COLOR_2, PRINT_WAVE_WINDOW_BRUSH_STYLE_1,
        PRINT_WAVE_WINDOW_BRUSH_STYLE_2, PRINT_WAVE_WINDOW_BRUSH_STYLE_2, PRINT_WAVE_WINDOW_BRUSH_STYLE_2);
  __readBorder(AS_WW, wwBorder);
  __readAsOpacity(AS_WW, wwOpacityList);

  __readPen("FAIAreaLow500", faiAreaLow500PenList, PRINT_FAI_LOW_500_COLOR_1, PRINT_FAI_LOW_500_COLOR_2, PRINT_FAI_LOW_500_COLOR_2, PRINT_FAI_LOW_500_COLOR_2,
        PRINT_FAI_LOW_500_PEN_1, PRINT_FAI_LOW_500_PEN_2, PRINT_FAI_LOW_500_PEN_2, PRINT_FAI_LOW_500_PEN_2,
        PRINT_FAI_LOW_500_PEN_STYLE_1, PRINT_FAI_LOW_500_PEN_STYLE_2, PRINT_FAI_LOW_500_PEN_STYLE_2, PRINT_FAI_LOW_500_PEN_STYLE_2);
  __readBrush("FAIAreaLow500", faiAreaLow500BrushList, PRINT_FAI_LOW_500_BRUSH_COLOR_1, PRINT_FAI_LOW_500_BRUSH_COLOR_2,
        PRINT_FAI_LOW_500_BRUSH_COLOR_2, PRINT_FAI_LOW_500_BRUSH_COLOR_2, PRINT_FAI_LOW_500_BRUSH_STYLE_1,
        PRINT_FAI_LOW_500_BRUSH_STYLE_2, PRINT_FAI_LOW_500_BRUSH_STYLE_2, PRINT_FAI_LOW_500_BRUSH_STYLE_2);
  __readBorder("FAIAreaLow500", faiAreaLow500Border);

  __readPen("FAIAreaHigh500", faiAreaHigh500PenList, PRINT_FAI_HIGH_500_COLOR_1, PRINT_FAI_HIGH_500_COLOR_2, PRINT_FAI_HIGH_500_COLOR_2, PRINT_FAI_HIGH_500_COLOR_2,
        PRINT_FAI_HIGH_500_PEN_1, PRINT_FAI_HIGH_500_PEN_2, PRINT_FAI_HIGH_500_PEN_2, PRINT_FAI_HIGH_500_PEN_2,
        PRINT_FAI_HIGH_500_PEN_STYLE_1, PRINT_FAI_HIGH_500_PEN_STYLE_2, PRINT_FAI_HIGH_500_PEN_STYLE_2, PRINT_FAI_HIGH_500_PEN_STYLE_2);
  __readBrush("FAIAreaHigh500", faiAreaHigh500BrushList, PRINT_FAI_HIGH_500_BRUSH_COLOR_1, PRINT_FAI_HIGH_500_BRUSH_COLOR_2,
        PRINT_FAI_HIGH_500_BRUSH_COLOR_2, PRINT_FAI_HIGH_500_BRUSH_COLOR_2, PRINT_FAI_HIGH_500_BRUSH_STYLE_1,
        PRINT_FAI_HIGH_500_BRUSH_STYLE_2, PRINT_FAI_HIGH_500_BRUSH_STYLE_2, PRINT_FAI_HIGH_500_BRUSH_STYLE_2);
  __readBorder("FAIAreaHigh500", faiAreaHigh500Border);
}

void ConfigMapElement::slotOk()
{
  // Die aktuell angezeigten Angaben müssen noch gespeichert werden ...
  slotSelectElement(oldElement);

  __writePen("Trail", trailPenList, trailBorder);

  __writePen("Road", roadPenList, roadBorder);

  __writePen("Highway", motorwayPenList, motorwayBorder);

  __writePen("Rail", railPenList, railBorder);

  __writePen("Rail_D", rail_dPenList, rail_dBorder);

  __writePen("Aerial Cable", aerialcablePenList, aerialcableBorder);

  __writePen("Canal", canalPenList, canalBorder);

  __writePen("City", cityPenList, cityBorder);

  __writeBrush(AS_A, airABrushList, airAPenList, airABorder);

  __writeBrush(AS_B, airBBrushList, airBPenList, airBBorder);

  __writeBrush(AS_C, airCBrushList, airCPenList, airCBorder);

  __writeBrush(AS_D, airDBrushList, airDPenList, airDBorder);

  __writeBrush(AS_EL, airElBrushList, airElPenList, airElBorder);

  __writeBrush(AS_E, airEhBrushList, airEhPenList, airEhBorder);

  __writeBrush(AS_F, airFBrushList, airFPenList, airFBorder);

  __writeBrush(AS_FIR, airFirBrushList, airFirPenList, airFirBorder);

  __writeBrush(AS_CTR_C, ctrCBrushList, ctrCPenList, ctrCBorder);

  __writeBrush(AS_CTR_D, ctrDBrushList, ctrDPenList, ctrDBorder);

  __writeBrush(AS_DANGER, dangerBrushList, dangerPenList, dangerBorder);

  __writeBrush(AS_LF, lowFBrushList, lowFPenList, lowFBorder);

  __writeBrush(AS_RESTRICTED, restrBrushList, restrPenList, restrBorder);

  __writeBrush(AS_TMZ, tmzBrushList, tmzPenList, tmzBorder);

  __writeBrush(AS_GS, gsBrushList, gsPenList, gsBorder);

  __writeBrush(AS_WW, wwBrushList, wwPenList, wwBorder);

  __writeBrush("Forest", forestBrushList, forestPenList, forestBorder);

  __writeBrush("River", riverBrushList, riverPenList, riverBorder);

  __writeBrush("River_T", river_tBrushList, river_tPenList, river_tBorder);

  __writeBrush("Glacier", glacierBrushList, glacierPenList, glacierBorder);

  __writeBrush("Pack Ice", packiceBrushList, packicePenList, packiceBorder);

  __writeBrush("FAIAreaLow500", faiAreaLow500BrushList, faiAreaLow500PenList, faiAreaLow500Border);

  __writeBrush("FAIAreaHigh500", faiAreaHigh500BrushList, faiAreaHigh500PenList, faiAreaHigh500Border);

  __writeAsOpacity( AS_A, airAOpacityList );
  __writeAsOpacity( AS_B, airBOpacityList );
  __writeAsOpacity( AS_C, airCOpacityList );
  __writeAsOpacity( AS_D, airDOpacityList );
  __writeAsOpacity( AS_EL, airElOpacityList );
  __writeAsOpacity( AS_E, airEhOpacityList );
  __writeAsOpacity( AS_F, airFOpacityList );
  __writeAsOpacity( AS_FIR, airFirOpacityList );
  __writeAsOpacity( AS_CTR_C, ctrCOpacityList );
  __writeAsOpacity( AS_CTR_D, ctrDOpacityList );
  __writeAsOpacity( AS_LF, lowFOpacityList );
  __writeAsOpacity( AS_DANGER, dangerOpacityList );
  __writeAsOpacity( AS_RESTRICTED, restrOpacityList );
  __writeAsOpacity( AS_TMZ, tmzOpacityList );
  __writeAsOpacity( AS_GS, gsOpacityList );
  __writeAsOpacity( AS_WW, wwOpacityList );
}

void ConfigMapElement::slotDefaultElements()
{
  if( configureMap )
    {
      __readDefaultMapItems();
    }
  else
    {
      __readDefaultPrintItems();
    }

  __defaultAsOpacity( airAOpacityList );
  __defaultAsOpacity( airBOpacityList );
  __defaultAsOpacity( airCOpacityList );
  __defaultAsOpacity( airDOpacityList );
  __defaultAsOpacity( airElOpacityList );
  __defaultAsOpacity( airEhOpacityList );
  __defaultAsOpacity( airFOpacityList );
  __defaultAsOpacity( airFirOpacityList );
  __defaultAsOpacity( ctrCOpacityList );
  __defaultAsOpacity( ctrDOpacityList );
  __defaultAsOpacity( lowFOpacityList );
  __defaultAsOpacity( dangerOpacityList );
  __defaultAsOpacity( restrOpacityList );
  __defaultAsOpacity( tmzOpacityList );
  __defaultAsOpacity( gsOpacityList );
  __defaultAsOpacity( wwOpacityList );

  oldElement = -1;
  slotSelectElement( currentElement );
}

void ConfigMapElement::__readDefaultMapItems()
{
  __defaultPen( trailPenList, trailBorder, TRAIL_COLOR_1, TRAIL_COLOR_2,
      TRAIL_COLOR_3, TRAIL_COLOR_4,
      TRAIL_PEN_1, TRAIL_PEN_2, TRAIL_PEN_3, TRAIL_PEN_4 );

  __defaultPen( roadPenList, roadBorder, ROAD_COLOR_1, ROAD_COLOR_2,
      ROAD_COLOR_3, ROAD_COLOR_4,
      ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4 );

  __defaultPen( motorwayPenList, motorwayBorder, HIGH_COLOR_1, HIGH_COLOR_2,
      HIGH_COLOR_3, HIGH_COLOR_4,
      HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4 );

  __defaultPen( canalPenList, canalBorder, CANAL_COLOR_1, CANAL_COLOR_2,
      CANAL_COLOR_3, CANAL_COLOR_4,
      CANAL_PEN_1, CANAL_PEN_2, CANAL_PEN_3, CANAL_PEN_4 );

  __defaultPen( railPenList, railBorder, RAIL_COLOR_1, RAIL_COLOR_2,
      RAIL_COLOR_3, RAIL_COLOR_4,
      RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4 );

  __defaultPen( rail_dPenList, rail_dBorder, RAIL_D_COLOR_1, RAIL_D_COLOR_2,
      RAIL_D_COLOR_3, RAIL_D_COLOR_4,
      RAIL_D_PEN_1, RAIL_D_PEN_2, RAIL_D_PEN_3, RAIL_D_PEN_4 );

  __defaultPen( aerialcablePenList, aerialcableBorder, AERIAL_CABLE_COLOR_1, AERIAL_CABLE_COLOR_2,
      AERIAL_CABLE_COLOR_3, AERIAL_CABLE_COLOR_4,
      AERIAL_CABLE_PEN_1, AERIAL_CABLE_PEN_2, AERIAL_CABLE_PEN_3, AERIAL_CABLE_PEN_4 );

  __defaultPenBrush( riverPenList, riverBorder, riverBrushList,
      RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_3, RIVER_COLOR_4,
      RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4,
      RIVER_BRUSH_COLOR_1, RIVER_BRUSH_COLOR_2,
      RIVER_BRUSH_COLOR_3, RIVER_BRUSH_COLOR_4,
      RIVER_BRUSH_STYLE_1, RIVER_BRUSH_STYLE_2,
      RIVER_BRUSH_STYLE_3, RIVER_BRUSH_STYLE_4);

  __defaultPenBrush( river_tPenList, river_tBorder, river_tBrushList,
      RIVER_T_COLOR_1, RIVER_T_COLOR_2, RIVER_T_COLOR_3, RIVER_T_COLOR_4,
      RIVER_T_PEN_1, RIVER_T_PEN_2, RIVER_T_PEN_3, RIVER_T_PEN_4,
      RIVER_T_BRUSH_COLOR_1, RIVER_T_BRUSH_COLOR_2,
      RIVER_T_BRUSH_COLOR_3, RIVER_T_BRUSH_COLOR_4,
      RIVER_T_BRUSH_STYLE_1, RIVER_T_BRUSH_STYLE_2,
      RIVER_T_BRUSH_STYLE_3, RIVER_T_BRUSH_STYLE_4);

  __defaultPenBrush( cityPenList, cityBorder, cityBrushList,
      CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3, CITY_COLOR_4,
      CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4,
      CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
      CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4,
      CITY_BRUSH_STYLE_1, CITY_BRUSH_STYLE_2,
      CITY_BRUSH_STYLE_3, CITY_BRUSH_STYLE_4 );

  __defaultPenBrush( forestPenList, forestBorder, forestBrushList,
      FRST_COLOR_1, FRST_COLOR_2, FRST_COLOR_3, FRST_COLOR_4,
      FRST_PEN_1, FRST_PEN_2, FRST_PEN_3, FRST_PEN_4,
      FRST_BRUSH_COLOR_1, FRST_BRUSH_COLOR_2,
      FRST_BRUSH_COLOR_3, FRST_BRUSH_COLOR_4,
      FRST_BRUSH_STYLE_1, FRST_BRUSH_STYLE_2,
      FRST_BRUSH_STYLE_3, FRST_BRUSH_STYLE_4 );

  __defaultPenBrush( glacierPenList, glacierBorder, glacierBrushList,
      GLACIER_COLOR_1, GLACIER_COLOR_2, GLACIER_COLOR_3, GLACIER_COLOR_4,
      GLACIER_PEN_1, GLACIER_PEN_2, GLACIER_PEN_3, GLACIER_PEN_4,
      GLACIER_BRUSH_COLOR_1, GLACIER_BRUSH_COLOR_2,
      GLACIER_BRUSH_COLOR_3, GLACIER_BRUSH_COLOR_4,
      GLACIER_BRUSH_STYLE_1, GLACIER_BRUSH_STYLE_2,
      GLACIER_BRUSH_STYLE_3, GLACIER_BRUSH_STYLE_4 );

  __defaultPenBrush(packicePenList, packiceBorder, packiceBrushList,
      PACK_ICE_COLOR_1, PACK_ICE_COLOR_2, PACK_ICE_COLOR_3, PACK_ICE_COLOR_4,
      PACK_ICE_PEN_1, PACK_ICE_PEN_2, PACK_ICE_PEN_3, PACK_ICE_PEN_4,
      PACK_ICE_BRUSH_COLOR_1, PACK_ICE_BRUSH_COLOR_2,
      PACK_ICE_BRUSH_COLOR_3, PACK_ICE_BRUSH_COLOR_4,
      PACK_ICE_BRUSH_STYLE_1, PACK_ICE_BRUSH_STYLE_2,
      PACK_ICE_BRUSH_STYLE_3, PACK_ICE_BRUSH_STYLE_4);

  __defaultPenBrush(airAPenList, airABorder, airABrushList,
      AIRA_COLOR_1, AIRA_COLOR_2, AIRA_COLOR_3, AIRA_COLOR_4,
      AIRA_PEN_1, AIRA_PEN_2, AIRA_PEN_3, AIRA_PEN_4,
      AIRA_BRUSH_COLOR_1, AIRA_BRUSH_COLOR_2,
      AIRA_BRUSH_COLOR_3, AIRA_BRUSH_COLOR_4,
      AIRA_BRUSH_STYLE_1, AIRA_BRUSH_STYLE_2,
      AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4);

  __defaultPenBrush(airBPenList, airBBorder, airBBrushList,
      AIRB_COLOR_1, AIRB_COLOR_2, AIRB_COLOR_3, AIRB_COLOR_4,
      AIRB_PEN_1, AIRB_PEN_2, AIRB_PEN_3, AIRB_PEN_4,
      AIRB_BRUSH_COLOR_1, AIRB_BRUSH_COLOR_2,
      AIRB_BRUSH_COLOR_3, AIRB_BRUSH_COLOR_4,
      AIRB_BRUSH_STYLE_1, AIRB_BRUSH_STYLE_2,
      AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4);

  __defaultPenBrush(airCPenList, airCBorder, airCBrushList,
      AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3, AIRC_COLOR_4,
      AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4,
      AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
      AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4,
      AIRC_BRUSH_STYLE_1, AIRC_BRUSH_STYLE_2,
      AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4);

  __defaultPenBrush(airDPenList, airDBorder, airDBrushList,
      AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3, AIRD_COLOR_4,
      AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4,
      AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
      AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4,
      AIRD_BRUSH_STYLE_1, AIRD_BRUSH_STYLE_2,
      AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4);

  __defaultPenBrush(airElPenList, airElBorder, airElBrushList,
      AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3, AIREL_COLOR_4,
      AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4,
      AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
      AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4,
      AIREL_BRUSH_STYLE_1, AIREL_BRUSH_STYLE_2,
      AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4);

  __defaultPenBrush(airEhPenList, airEhBorder, airEhBrushList,
      AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3, AIREH_COLOR_4,
      AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4,
      AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
      AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4,
      AIREH_BRUSH_STYLE_1, AIREH_BRUSH_STYLE_2,
      AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4);

  __defaultPenBrush(airFPenList, airFBorder, airFBrushList,
      AIRF_COLOR_1, AIRF_COLOR_2, AIRF_COLOR_2, AIRF_COLOR_2,
      AIRF_PEN_1, AIRF_PEN_2, AIRF_PEN_2, AIRF_PEN_2,
      AIRF_BRUSH_COLOR_1, AIRF_BRUSH_COLOR_2,
      AIRF_BRUSH_COLOR_2, AIRF_BRUSH_COLOR_2,
      AIRF_BRUSH_STYLE_1, AIRF_BRUSH_STYLE_2,
      AIRF_BRUSH_STYLE_2, AIRF_BRUSH_STYLE_2);

  __defaultPenBrush(airFirPenList, airFirBorder, airFirBrushList,
      AIRFIR_COLOR_1, AIRFIR_COLOR_2, AIRFIR_COLOR_2, AIRFIR_COLOR_2,
      AIRFIR_PEN_1, AIRFIR_PEN_2, AIRFIR_PEN_2, AIRFIR_PEN_2,
      AIRFIR_BRUSH_COLOR_1, AIRFIR_BRUSH_COLOR_2,
      AIRFIR_BRUSH_COLOR_2, AIRFIR_BRUSH_COLOR_2,
      AIRFIR_BRUSH_STYLE_1, AIRFIR_BRUSH_STYLE_2,
      AIRFIR_BRUSH_STYLE_2, AIRFIR_BRUSH_STYLE_2);

  __defaultPenBrush(ctrCPenList, ctrCBorder, ctrCBrushList,
      CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3, CTRC_COLOR_4,
      CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4,
      CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
      CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4,
      CTRC_BRUSH_STYLE_1, CTRC_BRUSH_STYLE_2,
      CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4);

  __defaultPenBrush(ctrDPenList, ctrDBorder, ctrDBrushList,
      CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3, CTRD_COLOR_4,
      CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4,
      CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
      CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4,
      CTRD_BRUSH_STYLE_1, CTRD_BRUSH_STYLE_2,
      CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4);

  __defaultPenBrush(dangerPenList, dangerBorder, dangerBrushList,
      DNG_COLOR_1, DNG_COLOR_2, DNG_COLOR_3, DNG_COLOR_4,
      DNG_PEN_1, DNG_PEN_2, DNG_PEN_3, DNG_PEN_4,
      DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
      DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4,
      DNG_BRUSH_STYLE_1, DNG_BRUSH_STYLE_2,
      DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4);

  __defaultPenBrush(restrPenList, restrBorder, restrBrushList,
      RES_COLOR_1, RES_COLOR_2, RES_COLOR_3, RES_COLOR_4,
      RES_PEN_1, RES_PEN_2, RES_PEN_3, RES_PEN_4,
      RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
      RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4,
      RES_BRUSH_STYLE_1, RES_BRUSH_STYLE_2,
      RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4);

  __defaultPenBrush(lowFPenList, lowFBorder, lowFBrushList,
      LOWF_COLOR_1, LOWF_COLOR_2, LOWF_COLOR_3, LOWF_COLOR_4,
      LOWF_PEN_1, LOWF_PEN_2, LOWF_PEN_3, LOWF_PEN_4,
      LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
      LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4,
      LOWF_BRUSH_STYLE_1, LOWF_BRUSH_STYLE_2,
      LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4);

  __defaultPenBrush(tmzPenList, tmzBorder, tmzBrushList,
      TMZ_COLOR_1, TMZ_COLOR_2, TMZ_COLOR_3, TMZ_COLOR_4,
      TMZ_PEN_1, TMZ_PEN_2, TMZ_PEN_3, TMZ_PEN_4,
      TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
      TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4,
      TMZ_BRUSH_STYLE_1, TMZ_BRUSH_STYLE_2,
      TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4);

  __defaultPenBrush(gsPenList, gsBorder, gsBrushList,
      GLIDER_SECTOR_COLOR_1, GLIDER_SECTOR_COLOR_2, GLIDER_SECTOR_COLOR_3, GLIDER_SECTOR_COLOR_4,
      GLIDER_SECTOR_PEN_1, GLIDER_SECTOR_PEN_2, GLIDER_SECTOR_PEN_3, GLIDER_SECTOR_PEN_4,
      GLIDER_SECTOR_BRUSH_COLOR_1, GLIDER_SECTOR_BRUSH_COLOR_2,
      GLIDER_SECTOR_BRUSH_COLOR_3, GLIDER_SECTOR_BRUSH_COLOR_4,
      GLIDER_SECTOR_BRUSH_STYLE_1, GLIDER_SECTOR_BRUSH_STYLE_2,
      GLIDER_SECTOR_BRUSH_STYLE_3, GLIDER_SECTOR_BRUSH_STYLE_4);

  __defaultPenBrush(wwPenList, wwBorder, wwBrushList,
      WAVE_WINDOW_COLOR_1, WAVE_WINDOW_COLOR_2, WAVE_WINDOW_COLOR_3, WAVE_WINDOW_COLOR_4,
      WAVE_WINDOW_PEN_1, WAVE_WINDOW_PEN_2, WAVE_WINDOW_PEN_3, WAVE_WINDOW_PEN_4,
      WAVE_WINDOW_BRUSH_COLOR_1, WAVE_WINDOW_BRUSH_COLOR_2,
      WAVE_WINDOW_BRUSH_COLOR_3, WAVE_WINDOW_BRUSH_COLOR_4,
      WAVE_WINDOW_BRUSH_STYLE_1, WAVE_WINDOW_BRUSH_STYLE_2,
      WAVE_WINDOW_BRUSH_STYLE_3, WAVE_WINDOW_BRUSH_STYLE_4);

  __defaultPenBrush(faiAreaLow500PenList, faiAreaLow500Border, faiAreaLow500BrushList,
      FAI_LOW_500_COLOR_1, FAI_LOW_500_COLOR_2, FAI_LOW_500_COLOR_3, FAI_LOW_500_COLOR_4,
      FAI_LOW_500_PEN_1, FAI_LOW_500_PEN_2, FAI_LOW_500_PEN_3, FAI_LOW_500_PEN_4,
      FAI_LOW_500_BRUSH_COLOR_1, FAI_LOW_500_BRUSH_COLOR_2,
      FAI_LOW_500_BRUSH_COLOR_3, FAI_LOW_500_BRUSH_COLOR_4,
      FAI_LOW_500_BRUSH_STYLE_1, FAI_LOW_500_BRUSH_STYLE_2,
      FAI_LOW_500_BRUSH_STYLE_3, FAI_LOW_500_BRUSH_STYLE_4);

  __defaultPenBrush(faiAreaHigh500PenList, faiAreaHigh500Border, faiAreaHigh500BrushList,
      FAI_HIGH_500_COLOR_1, FAI_HIGH_500_COLOR_2, FAI_HIGH_500_COLOR_3, FAI_HIGH_500_COLOR_4,
      FAI_HIGH_500_PEN_1, FAI_HIGH_500_PEN_2, FAI_HIGH_500_PEN_3, FAI_HIGH_500_PEN_4,
      FAI_HIGH_500_BRUSH_COLOR_1, FAI_HIGH_500_BRUSH_COLOR_2,
      FAI_HIGH_500_BRUSH_COLOR_3, FAI_HIGH_500_BRUSH_COLOR_4,
      FAI_HIGH_500_BRUSH_STYLE_1, FAI_HIGH_500_BRUSH_STYLE_2,
      FAI_HIGH_500_BRUSH_STYLE_3, FAI_HIGH_500_BRUSH_STYLE_4);
}

void ConfigMapElement::__readDefaultPrintItems()
{
  __defaultPen( trailPenList, trailBorder, PRINT_TRAIL_COLOR_1, PRINT_TRAIL_COLOR_2,
      PRINT_TRAIL_COLOR_2, PRINT_TRAIL_COLOR_2,
      PRINT_TRAIL_PEN_1, PRINT_TRAIL_PEN_2, PRINT_TRAIL_PEN_2, PRINT_TRAIL_PEN_2 );

  __defaultPen( roadPenList, roadBorder, PRINT_ROAD_COLOR_1, PRINT_ROAD_COLOR_2,
      PRINT_ROAD_COLOR_2, PRINT_ROAD_COLOR_2,
      PRINT_ROAD_PEN_1, PRINT_ROAD_PEN_2, PRINT_ROAD_PEN_2, PRINT_ROAD_PEN_2 );

  __defaultPen( motorwayPenList, motorwayBorder, PRINT_HIGH_COLOR_1, PRINT_HIGH_COLOR_2,
      PRINT_HIGH_COLOR_2, PRINT_HIGH_COLOR_2,
      PRINT_HIGH_PEN_1, PRINT_HIGH_PEN_2, PRINT_HIGH_PEN_2, PRINT_HIGH_PEN_2 );

  __defaultPen( riverPenList, riverBorder, PRINT_RIVER_COLOR_1, PRINT_RIVER_COLOR_2,
      PRINT_RIVER_COLOR_2, PRINT_RIVER_COLOR_2,
      PRINT_RIVER_PEN_1, PRINT_RIVER_PEN_2, PRINT_RIVER_PEN_2, PRINT_RIVER_PEN_2 );

  __defaultPen( canalPenList, canalBorder, PRINT_CANAL_COLOR_1, PRINT_CANAL_COLOR_2,
      PRINT_CANAL_COLOR_2, PRINT_CANAL_COLOR_2,
      PRINT_CANAL_PEN_1, PRINT_CANAL_PEN_2, PRINT_CANAL_PEN_2, PRINT_CANAL_PEN_2 );

  __defaultPen( railPenList, railBorder, PRINT_RAIL_COLOR_1, PRINT_RAIL_COLOR_2,
      PRINT_RAIL_COLOR_2, PRINT_RAIL_COLOR_2,
      PRINT_RAIL_PEN_1, PRINT_RAIL_PEN_2, PRINT_RAIL_PEN_2, PRINT_RAIL_PEN_2 );

  __defaultPen( rail_dPenList, rail_dBorder, PRINT_RAIL_D_COLOR_1, PRINT_RAIL_D_COLOR_2,
      PRINT_RAIL_D_COLOR_2, PRINT_RAIL_D_COLOR_2,
      PRINT_RAIL_D_PEN_1, PRINT_RAIL_D_PEN_2, PRINT_RAIL_D_PEN_2, PRINT_RAIL_D_PEN_2 );

  __defaultPen( aerialcablePenList, aerialcableBorder, PRINT_AERIAL_CABLE_COLOR_1, PRINT_AERIAL_CABLE_COLOR_2,
      PRINT_AERIAL_CABLE_COLOR_2, PRINT_AERIAL_CABLE_COLOR_2,
      PRINT_AERIAL_CABLE_PEN_1, PRINT_AERIAL_CABLE_PEN_2, PRINT_AERIAL_CABLE_PEN_2, PRINT_AERIAL_CABLE_PEN_2 );

  __defaultPenBrush( river_tPenList, river_tBorder, river_tBrushList,
      PRINT_RIVER_T_COLOR_1, PRINT_RIVER_T_COLOR_2, PRINT_RIVER_T_COLOR_2, PRINT_RIVER_T_COLOR_2,
      PRINT_RIVER_T_PEN_1, PRINT_RIVER_T_PEN_2, PRINT_RIVER_T_PEN_2, PRINT_RIVER_T_PEN_2,
      PRINT_RIVER_T_BRUSH_COLOR_1, PRINT_RIVER_T_BRUSH_COLOR_2,
      PRINT_RIVER_T_BRUSH_COLOR_2, PRINT_RIVER_T_BRUSH_COLOR_2,
      PRINT_RIVER_T_BRUSH_STYLE_1, PRINT_RIVER_T_BRUSH_STYLE_2,
      PRINT_RIVER_T_BRUSH_STYLE_2, PRINT_RIVER_T_BRUSH_STYLE_2);

  __defaultPenBrush( cityPenList, cityBorder, cityBrushList,
      PRINT_CITY_COLOR_1, PRINT_CITY_COLOR_2, PRINT_CITY_COLOR_2, PRINT_CITY_COLOR_2,
      PRINT_CITY_PEN_1, PRINT_CITY_PEN_2, PRINT_CITY_PEN_2, PRINT_CITY_PEN_2,
      PRINT_CITY_BRUSH_COLOR_1, PRINT_CITY_BRUSH_COLOR_2,
      PRINT_CITY_BRUSH_COLOR_2, PRINT_CITY_BRUSH_COLOR_2,
      PRINT_CITY_BRUSH_STYLE_1, PRINT_CITY_BRUSH_STYLE_2,
      PRINT_CITY_BRUSH_STYLE_2, PRINT_CITY_BRUSH_STYLE_2 );

  __defaultPenBrush( forestPenList, forestBorder, forestBrushList,
      PRINT_FRST_COLOR_1, PRINT_FRST_COLOR_2, PRINT_FRST_COLOR_2, PRINT_FRST_COLOR_2,
      PRINT_FRST_PEN_1, PRINT_FRST_PEN_2, PRINT_FRST_PEN_2, PRINT_FRST_PEN_2,
      PRINT_FRST_BRUSH_COLOR_1, PRINT_FRST_BRUSH_COLOR_2,
      PRINT_FRST_BRUSH_COLOR_2, PRINT_FRST_BRUSH_COLOR_2,
      PRINT_FRST_BRUSH_STYLE_1, PRINT_FRST_BRUSH_STYLE_2,
      PRINT_FRST_BRUSH_STYLE_2, PRINT_FRST_BRUSH_STYLE_2 );

  __defaultPenBrush( glacierPenList, glacierBorder, glacierBrushList,
      PRINT_GLACIER_COLOR_1, PRINT_GLACIER_COLOR_2, PRINT_GLACIER_COLOR_2, PRINT_GLACIER_COLOR_2,
      PRINT_GLACIER_PEN_1, PRINT_GLACIER_PEN_2, PRINT_GLACIER_PEN_2, PRINT_GLACIER_PEN_2,
      PRINT_GLACIER_BRUSH_COLOR_1, PRINT_GLACIER_BRUSH_COLOR_2,
      PRINT_GLACIER_BRUSH_COLOR_2, PRINT_GLACIER_BRUSH_COLOR_2,
      PRINT_GLACIER_BRUSH_STYLE_1, PRINT_GLACIER_BRUSH_STYLE_2,
      PRINT_GLACIER_BRUSH_STYLE_2, PRINT_GLACIER_BRUSH_STYLE_2 );

  __defaultPenBrush(packicePenList, packiceBorder, packiceBrushList,
      PRINT_PACK_ICE_COLOR_1, PRINT_PACK_ICE_COLOR_2, PRINT_PACK_ICE_COLOR_2, PRINT_PACK_ICE_COLOR_2,
      PRINT_PACK_ICE_PEN_1, PRINT_PACK_ICE_PEN_2, PRINT_PACK_ICE_PEN_2, PRINT_PACK_ICE_PEN_2,
      PRINT_PACK_ICE_BRUSH_COLOR_1, PRINT_PACK_ICE_BRUSH_COLOR_2,
      PRINT_PACK_ICE_BRUSH_COLOR_2, PRINT_PACK_ICE_BRUSH_COLOR_2,
      PRINT_PACK_ICE_BRUSH_STYLE_1, PRINT_PACK_ICE_BRUSH_STYLE_2,
      PRINT_PACK_ICE_BRUSH_STYLE_2, PRINT_PACK_ICE_BRUSH_STYLE_2);

  __defaultPenBrush(airAPenList, airABorder, airABrushList,
      PRINT_AIRA_COLOR_1, PRINT_AIRA_COLOR_2, PRINT_AIRA_COLOR_2, PRINT_AIRA_COLOR_2,
      PRINT_AIRA_PEN_1, PRINT_AIRA_PEN_2, PRINT_AIRA_PEN_2, PRINT_AIRA_PEN_2,
      PRINT_AIRA_BRUSH_COLOR_1, PRINT_AIRA_BRUSH_COLOR_2,
      PRINT_AIRA_BRUSH_COLOR_2, PRINT_AIRA_BRUSH_COLOR_2,
      PRINT_AIRA_BRUSH_STYLE_1, PRINT_AIRA_BRUSH_STYLE_2,
      PRINT_AIRA_BRUSH_STYLE_2, PRINT_AIRA_BRUSH_STYLE_2);

  __defaultPenBrush(airBPenList, airBBorder, airBBrushList,
      PRINT_AIRB_COLOR_1, PRINT_AIRB_COLOR_2, PRINT_AIRB_COLOR_2, PRINT_AIRB_COLOR_2,
      PRINT_AIRB_PEN_1, PRINT_AIRB_PEN_2, PRINT_AIRB_PEN_2, PRINT_AIRB_PEN_2,
      PRINT_AIRB_BRUSH_COLOR_1, PRINT_AIRB_BRUSH_COLOR_2,
      PRINT_AIRB_BRUSH_COLOR_2, PRINT_AIRB_BRUSH_COLOR_2,
      PRINT_AIRB_BRUSH_STYLE_1, PRINT_AIRB_BRUSH_STYLE_2,
      PRINT_AIRB_BRUSH_STYLE_2, PRINT_AIRB_BRUSH_STYLE_2);

  __defaultPenBrush(airCPenList, airCBorder, airCBrushList,
      PRINT_AIRC_COLOR_1, PRINT_AIRC_COLOR_2, PRINT_AIRC_COLOR_2, PRINT_AIRC_COLOR_2,
      PRINT_AIRC_PEN_1, PRINT_AIRC_PEN_2, PRINT_AIRC_PEN_2, PRINT_AIRC_PEN_2,
      PRINT_AIRC_BRUSH_COLOR_1, PRINT_AIRC_BRUSH_COLOR_2,
      PRINT_AIRC_BRUSH_COLOR_2, PRINT_AIRC_BRUSH_COLOR_2,
      PRINT_AIRC_BRUSH_STYLE_1, PRINT_AIRC_BRUSH_STYLE_2,
      PRINT_AIRC_BRUSH_STYLE_2, PRINT_AIRC_BRUSH_STYLE_2);

  __defaultPenBrush(airDPenList, airDBorder, airDBrushList,
      PRINT_AIRD_COLOR_1, PRINT_AIRD_COLOR_2, PRINT_AIRD_COLOR_2, PRINT_AIRD_COLOR_2,
      PRINT_AIRD_PEN_1, PRINT_AIRD_PEN_2, PRINT_AIRD_PEN_2, PRINT_AIRD_PEN_2,
      PRINT_AIRD_BRUSH_COLOR_1, PRINT_AIRD_BRUSH_COLOR_2,
      PRINT_AIRD_BRUSH_COLOR_2, PRINT_AIRD_BRUSH_COLOR_2,
      PRINT_AIRD_BRUSH_STYLE_1, PRINT_AIRD_BRUSH_STYLE_2,
      PRINT_AIRD_BRUSH_STYLE_2, PRINT_AIRD_BRUSH_STYLE_2);

  __defaultPenBrush(airElPenList, airElBorder, airElBrushList,
      PRINT_AIREL_COLOR_1, PRINT_AIREL_COLOR_2, PRINT_AIREL_COLOR_2, PRINT_AIREL_COLOR_2,
      PRINT_AIREL_PEN_1, PRINT_AIREL_PEN_2, PRINT_AIREL_PEN_2, PRINT_AIREL_PEN_2,
      PRINT_AIREL_BRUSH_COLOR_1, PRINT_AIREL_BRUSH_COLOR_2,
      PRINT_AIREL_BRUSH_COLOR_2, PRINT_AIREL_BRUSH_COLOR_2,
      PRINT_AIREL_BRUSH_STYLE_1, PRINT_AIREL_BRUSH_STYLE_2,
      PRINT_AIREL_BRUSH_STYLE_2, PRINT_AIREL_BRUSH_STYLE_2);

  __defaultPenBrush(airEhPenList, airEhBorder, airEhBrushList,
      PRINT_AIREH_COLOR_1, PRINT_AIREH_COLOR_2, PRINT_AIREH_COLOR_2, PRINT_AIREH_COLOR_2,
      PRINT_AIREH_PEN_1, PRINT_AIREH_PEN_2, PRINT_AIREH_PEN_2, PRINT_AIREH_PEN_2,
      PRINT_AIREH_BRUSH_COLOR_1, PRINT_AIREH_BRUSH_COLOR_2,
      PRINT_AIREH_BRUSH_COLOR_2, PRINT_AIREH_BRUSH_COLOR_2,
      PRINT_AIREH_BRUSH_STYLE_1, PRINT_AIREH_BRUSH_STYLE_2,
      PRINT_AIREH_BRUSH_STYLE_2, PRINT_AIREH_BRUSH_STYLE_2);

  __defaultPenBrush(airFPenList, airFBorder, airFBrushList,
      PRINT_AIRF_COLOR_1, PRINT_AIRF_COLOR_2, PRINT_AIRF_COLOR_2, PRINT_AIRF_COLOR_2,
      PRINT_AIRF_PEN_1, PRINT_AIRF_PEN_2, PRINT_AIRF_PEN_2, PRINT_AIRF_PEN_2,
      PRINT_AIRF_BRUSH_COLOR_1, PRINT_AIRF_BRUSH_COLOR_2,
      PRINT_AIRF_BRUSH_COLOR_2, PRINT_AIRF_BRUSH_COLOR_2,
      PRINT_AIRF_BRUSH_STYLE_1, PRINT_AIRF_BRUSH_STYLE_2,
      PRINT_AIRF_BRUSH_STYLE_2, PRINT_AIRF_BRUSH_STYLE_2);

  __defaultPenBrush(airFirPenList, airFirBorder, airFirBrushList,
      PRINT_AIRFIR_COLOR_1, PRINT_AIRFIR_COLOR_2, PRINT_AIRFIR_COLOR_2, PRINT_AIRFIR_COLOR_2,
      PRINT_AIRFIR_PEN_1, PRINT_AIRFIR_PEN_2, PRINT_AIRFIR_PEN_2, PRINT_AIRFIR_PEN_2,
      PRINT_AIRFIR_BRUSH_COLOR_1, PRINT_AIRFIR_BRUSH_COLOR_2,
      PRINT_AIRFIR_BRUSH_COLOR_2, PRINT_AIRFIR_BRUSH_COLOR_2,
      PRINT_AIRFIR_BRUSH_STYLE_1, PRINT_AIRFIR_BRUSH_STYLE_2,
      PRINT_AIRFIR_BRUSH_STYLE_2, PRINT_AIRFIR_BRUSH_STYLE_2);

  __defaultPenBrush(ctrCPenList, ctrCBorder, ctrCBrushList,
      PRINT_CTRC_COLOR_1, PRINT_CTRC_COLOR_2, PRINT_CTRC_COLOR_2, PRINT_CTRC_COLOR_2,
      PRINT_CTRC_PEN_1, PRINT_CTRC_PEN_2, PRINT_CTRC_PEN_2, PRINT_CTRC_PEN_2,
      PRINT_CTRC_BRUSH_COLOR_1, PRINT_CTRC_BRUSH_COLOR_2,
      PRINT_CTRC_BRUSH_COLOR_2, PRINT_CTRC_BRUSH_COLOR_2,
      PRINT_CTRC_BRUSH_STYLE_1, PRINT_CTRC_BRUSH_STYLE_2,
      PRINT_CTRC_BRUSH_STYLE_2, PRINT_CTRC_BRUSH_STYLE_2);

  __defaultPenBrush(ctrDPenList, ctrDBorder, ctrDBrushList,
      PRINT_CTRD_COLOR_1, PRINT_CTRD_COLOR_2, PRINT_CTRD_COLOR_2, PRINT_CTRD_COLOR_2,
      PRINT_CTRD_PEN_1, PRINT_CTRD_PEN_2, PRINT_CTRD_PEN_2, PRINT_CTRD_PEN_2,
      PRINT_CTRD_BRUSH_COLOR_1, PRINT_CTRD_BRUSH_COLOR_2,
      PRINT_CTRD_BRUSH_COLOR_2, PRINT_CTRD_BRUSH_COLOR_2,
      PRINT_CTRD_BRUSH_STYLE_1, PRINT_CTRD_BRUSH_STYLE_2,
      PRINT_CTRD_BRUSH_STYLE_2, PRINT_CTRD_BRUSH_STYLE_2);

  __defaultPenBrush(dangerPenList, dangerBorder, dangerBrushList,
      PRINT_DNG_COLOR_1, PRINT_DNG_COLOR_2, PRINT_DNG_COLOR_2, PRINT_DNG_COLOR_2,
      PRINT_DNG_PEN_1, PRINT_DNG_PEN_2, PRINT_DNG_PEN_2, PRINT_DNG_PEN_2,
      PRINT_DNG_BRUSH_COLOR_1, PRINT_DNG_BRUSH_COLOR_2,
      PRINT_DNG_BRUSH_COLOR_2, PRINT_DNG_BRUSH_COLOR_2,
      PRINT_DNG_BRUSH_STYLE_1, PRINT_DNG_BRUSH_STYLE_2,
      PRINT_DNG_BRUSH_STYLE_2, PRINT_DNG_BRUSH_STYLE_2);

  __defaultPenBrush(restrPenList, restrBorder, restrBrushList,
      PRINT_RES_COLOR_1, PRINT_RES_COLOR_2, PRINT_RES_COLOR_2, PRINT_RES_COLOR_2,
      PRINT_RES_PEN_1, PRINT_RES_PEN_2, PRINT_RES_PEN_2, PRINT_RES_PEN_2,
      PRINT_RES_BRUSH_COLOR_1, PRINT_RES_BRUSH_COLOR_2,
      PRINT_RES_BRUSH_COLOR_2, PRINT_RES_BRUSH_COLOR_2,
      PRINT_RES_BRUSH_STYLE_1, PRINT_RES_BRUSH_STYLE_2,
      PRINT_RES_BRUSH_STYLE_2, PRINT_RES_BRUSH_STYLE_2);

  __defaultPenBrush(lowFPenList, lowFBorder, lowFBrushList,
      PRINT_LOWF_COLOR_1, PRINT_LOWF_COLOR_2, PRINT_LOWF_COLOR_2, PRINT_LOWF_COLOR_2,
      PRINT_LOWF_PEN_1, PRINT_LOWF_PEN_2, PRINT_LOWF_PEN_2, PRINT_LOWF_PEN_2,
      PRINT_LOWF_BRUSH_COLOR_1, PRINT_LOWF_BRUSH_COLOR_2,
      PRINT_LOWF_BRUSH_COLOR_2, PRINT_LOWF_BRUSH_COLOR_2,
      PRINT_LOWF_BRUSH_STYLE_1, PRINT_LOWF_BRUSH_STYLE_2,
      PRINT_LOWF_BRUSH_STYLE_2, PRINT_LOWF_BRUSH_STYLE_2);

  __defaultPenBrush(tmzPenList, tmzBorder, tmzBrushList,
      PRINT_TMZ_COLOR_1, PRINT_TMZ_COLOR_2, PRINT_TMZ_COLOR_2, PRINT_TMZ_COLOR_2,
      PRINT_TMZ_PEN_1, PRINT_TMZ_PEN_2, PRINT_TMZ_PEN_2, PRINT_TMZ_PEN_2,
      PRINT_TMZ_BRUSH_COLOR_1, PRINT_TMZ_BRUSH_COLOR_2,
      PRINT_TMZ_BRUSH_COLOR_2, PRINT_TMZ_BRUSH_COLOR_2,
      PRINT_TMZ_BRUSH_STYLE_1, PRINT_TMZ_BRUSH_STYLE_2,
      PRINT_TMZ_BRUSH_STYLE_2, PRINT_TMZ_BRUSH_STYLE_2);

  __defaultPenBrush(gsPenList, gsBorder, gsBrushList,
      PRINT_GLIDER_SECTOR_COLOR_1, PRINT_GLIDER_SECTOR_COLOR_2, PRINT_GLIDER_SECTOR_COLOR_2, PRINT_GLIDER_SECTOR_COLOR_2,
      PRINT_GLIDER_SECTOR_PEN_1, PRINT_GLIDER_SECTOR_PEN_2, PRINT_GLIDER_SECTOR_PEN_2, PRINT_GLIDER_SECTOR_PEN_2,
      PRINT_GLIDER_SECTOR_BRUSH_COLOR_1, PRINT_GLIDER_SECTOR_BRUSH_COLOR_2,
      PRINT_GLIDER_SECTOR_BRUSH_COLOR_2, PRINT_GLIDER_SECTOR_BRUSH_COLOR_2,
      PRINT_GLIDER_SECTOR_BRUSH_STYLE_1, PRINT_GLIDER_SECTOR_BRUSH_STYLE_2,
      PRINT_GLIDER_SECTOR_BRUSH_STYLE_2, PRINT_GLIDER_SECTOR_BRUSH_STYLE_2);

  __defaultPenBrush(wwPenList, wwBorder, wwBrushList,
      PRINT_WAVE_WINDOW_COLOR_1, PRINT_WAVE_WINDOW_COLOR_2, PRINT_WAVE_WINDOW_COLOR_2, PRINT_WAVE_WINDOW_COLOR_2,
      PRINT_WAVE_WINDOW_PEN_1, PRINT_WAVE_WINDOW_PEN_2, PRINT_WAVE_WINDOW_PEN_2, PRINT_WAVE_WINDOW_PEN_2,
      PRINT_WAVE_WINDOW_BRUSH_COLOR_1, PRINT_WAVE_WINDOW_BRUSH_COLOR_2,
      PRINT_WAVE_WINDOW_BRUSH_COLOR_2, PRINT_WAVE_WINDOW_BRUSH_COLOR_2,
      PRINT_WAVE_WINDOW_BRUSH_STYLE_1, PRINT_WAVE_WINDOW_BRUSH_STYLE_2,
      PRINT_WAVE_WINDOW_BRUSH_STYLE_2, PRINT_WAVE_WINDOW_BRUSH_STYLE_2);

  __defaultPenBrush(faiAreaLow500PenList, faiAreaLow500Border, faiAreaLow500BrushList,
      PRINT_FAI_LOW_500_COLOR_1, PRINT_FAI_LOW_500_COLOR_2, PRINT_FAI_LOW_500_COLOR_2, PRINT_FAI_LOW_500_COLOR_2,
      PRINT_FAI_LOW_500_PEN_1, PRINT_FAI_LOW_500_PEN_2, PRINT_FAI_LOW_500_PEN_2, PRINT_FAI_LOW_500_PEN_2,
      PRINT_FAI_LOW_500_BRUSH_COLOR_1, PRINT_FAI_LOW_500_BRUSH_COLOR_2,
      PRINT_FAI_LOW_500_BRUSH_COLOR_2, PRINT_FAI_LOW_500_BRUSH_COLOR_2,
      PRINT_FAI_LOW_500_BRUSH_STYLE_1, PRINT_FAI_LOW_500_BRUSH_STYLE_2,
      PRINT_FAI_LOW_500_BRUSH_STYLE_2, PRINT_FAI_LOW_500_BRUSH_STYLE_2);

  __defaultPenBrush(faiAreaHigh500PenList, faiAreaHigh500Border, faiAreaHigh500BrushList,
      PRINT_FAI_HIGH_500_COLOR_1, PRINT_FAI_HIGH_500_COLOR_2, PRINT_FAI_HIGH_500_COLOR_2, PRINT_FAI_HIGH_500_COLOR_2,
      PRINT_FAI_HIGH_500_PEN_1, PRINT_FAI_HIGH_500_PEN_2, PRINT_FAI_HIGH_500_PEN_2, PRINT_FAI_HIGH_500_PEN_2,
      PRINT_FAI_HIGH_500_BRUSH_COLOR_1, PRINT_FAI_HIGH_500_BRUSH_COLOR_2,
      PRINT_FAI_HIGH_500_BRUSH_COLOR_2, PRINT_FAI_HIGH_500_BRUSH_COLOR_2,
      PRINT_FAI_HIGH_500_BRUSH_STYLE_1, PRINT_FAI_HIGH_500_BRUSH_STYLE_2,
      PRINT_FAI_HIGH_500_BRUSH_STYLE_2, PRINT_FAI_HIGH_500_BRUSH_STYLE_2);
}

void ConfigMapElement::slotSelectBorder1Color()
{
  QColor newColor = QColorDialog::getColor( border1Color, this );

  if( newColor.isValid() )
    {
      border1Color = newColor;
      QPixmap buttonPixmap(45, 14);
      buttonPixmap.fill( newColor );
      border1ColorButton->setIcon( buttonPixmap );
      border1ColorButton->setIconSize( buttonPixmap.size() );
    }
}

void ConfigMapElement::slotSelectBorder2Color()
{
  QColor newColor = QColorDialog::getColor( border2Color, this );

  if( newColor.isValid() )
    {
      border2Color = newColor;
      QPixmap buttonPixmap(45, 14);
      buttonPixmap.fill( newColor );
      border2ColorButton->setIcon( buttonPixmap );
      border2ColorButton->setIconSize( buttonPixmap.size() );
    }
}

void ConfigMapElement::slotSelectBorder3Color()
{
  QColor newColor = QColorDialog::getColor( border3Color, this );

  if( newColor.isValid() )
    {
      border3Color = newColor;
      QPixmap buttonPixmap(45, 14);
      buttonPixmap.fill( newColor );
      border3ColorButton->setIcon( buttonPixmap );
      border3ColorButton->setIconSize( buttonPixmap.size() );
    }
}

void ConfigMapElement::slotSelectBorder4Color()
{
  QColor newColor = QColorDialog::getColor( border4Color, this );

  if( newColor.isValid() )
    {
      border4Color = newColor;
      QPixmap buttonPixmap(45, 14);
      buttonPixmap.fill( newColor );
      border4ColorButton->setIcon( buttonPixmap );
      border4ColorButton->setIconSize( buttonPixmap.size() );
    }
}

void ConfigMapElement::slotCheckOpacity1( int index )
{
  bool enable = border1BrushStyle->isEnabled();

  opacity1->setEnabled( enable );

  if( ! enable )
    {
      return;
    }

  enum Qt::BrushStyle pattern =
      static_cast <enum Qt::BrushStyle> (border1BrushStyle->itemData( index ).toInt());

  opacity1->setEnabled( pattern == Qt::SolidPattern );
}

void ConfigMapElement::slotCheckOpacity2( int index )
{
  bool enable = border2BrushStyle->isEnabled();

  opacity2->setEnabled( enable );

  if( ! enable )
    {
      return;
    }

  enum Qt::BrushStyle pattern =
      static_cast <enum Qt::BrushStyle> (border2BrushStyle->itemData( index ).toInt());

  opacity2->setEnabled( pattern == Qt::SolidPattern );
}

void ConfigMapElement::slotCheckOpacity3( int index )
{
  bool enable = border3BrushStyle->isEnabled();

  opacity3->setEnabled( enable );

  if( ! enable )
    {
      return;
    }

  enum Qt::BrushStyle pattern =
      static_cast <enum Qt::BrushStyle> (border3BrushStyle->itemData( index ).toInt());

  opacity3->setEnabled( pattern == Qt::SolidPattern );
}

void ConfigMapElement::slotCheckOpacity4( int index )
{
  bool enable = border4BrushStyle->isEnabled();

  opacity4->setEnabled( enable );

  if( ! enable )
    {
      return;
    }

  enum Qt::BrushStyle pattern =
      static_cast <enum Qt::BrushStyle> (border4BrushStyle->itemData( index ).toInt());

  opacity4->setEnabled( pattern == Qt::SolidPattern );
}


void ConfigMapElement::slotSelectBorder1BrushColor()
{
  QColor newColor = QColorDialog::getColor( border1BrushColor, this );

  if( newColor.isValid() )
    {
      border1BrushColor = newColor;
      QPixmap buttonPixmap(45, 14);
      buttonPixmap.fill( newColor );
      border1BrushColorButton->setIcon( buttonPixmap );
      border1BrushColorButton->setIconSize( buttonPixmap.size() );
    }
}

void ConfigMapElement::slotSelectBorder2BrushColor()
{
  QColor newColor = QColorDialog::getColor( border2BrushColor, this );

  if( newColor.isValid() )
    {
      border2BrushColor = newColor;
      QPixmap buttonPixmap(45, 14);
      buttonPixmap.fill( newColor );
      border2BrushColorButton->setIcon( buttonPixmap );
      border2BrushColorButton->setIconSize( buttonPixmap.size() );
    }
}

void ConfigMapElement::slotSelectBorder3BrushColor()
{
  QColor newColor = QColorDialog::getColor( border3BrushColor, this );

  if( newColor.isValid() )
    {
      border3BrushColor = newColor;
      QPixmap buttonPixmap(45, 14);
      buttonPixmap.fill( newColor );
      border3BrushColorButton->setIcon( buttonPixmap );
      border3BrushColorButton->setIconSize( buttonPixmap.size() );
    }
}

void ConfigMapElement::slotSelectBorder4BrushColor()
{
  QColor newColor = QColorDialog::getColor( border4BrushColor, this );

  if( newColor.isValid() )
    {
      border4BrushColor = newColor;
      QPixmap buttonPixmap(45, 14);
      buttonPixmap.fill( newColor );
      border4BrushColorButton->setIcon( buttonPixmap );
      border4BrushColorButton->setIconSize( buttonPixmap.size() );
    }
}

void ConfigMapElement::slotSelectElement(int elementID)
{
  currentElement = elementID;

  // Disconnect signals from brush style combo boxes.
  border1BrushStyle->disconnect(SIGNAL(currentIndexChanged(int)));
  border2BrushStyle->disconnect(SIGNAL(currentIndexChanged(int)));
  border3BrushStyle->disconnect(SIGNAL(currentIndexChanged(int)));
  border4BrushStyle->disconnect(SIGNAL(currentIndexChanged(int)));

  // Reset access to opacity spin boxes. They are only usable for airspaces.
  opacity1->setEnabled( false );
  opacity2->setEnabled( false );
  opacity3->setEnabled( false );
  opacity4->setEnabled( false );

  switch(oldElement)
    {
      case KFLogConfig::Trail:
        __savePen(trailPenList, trailBorder);
        break;
      case KFLogConfig::Road:
        __savePen(roadPenList, roadBorder);
        break;
      case KFLogConfig::Motorway:
        __savePen(motorwayPenList, motorwayBorder);
        break;
      case KFLogConfig::Railway:
        __savePen(railPenList, railBorder);
        break;
      case KFLogConfig::Railway_D:
        __savePen(rail_dPenList, rail_dBorder);
        break;
      case KFLogConfig::Aerial_Cable:
        __savePen(aerialcablePenList, aerialcableBorder);
        break;
      case KFLogConfig::River:
        __savePen(riverPenList, riverBorder);
        __saveBrush(riverBrushList);
        break;
      case KFLogConfig::Canal:
        __savePen(canalPenList, canalBorder);
        break;
      case KFLogConfig::River_T:
        __savePen(river_tPenList, river_tBorder);
        __saveBrush(river_tBrushList);
        break;
      case KFLogConfig::City:
        __savePen(cityPenList, cityBorder);
        __saveBrush(cityBrushList);
        break;

      case KFLogConfig::AirA:
        __savePen(airAPenList, airABorder);
        __saveBrush(airABrushList);
        __saveAsOpacity( airAOpacityList );
        break;
      case KFLogConfig::AirB:
        __savePen(airBPenList, airBBorder);
        __saveBrush(airBBrushList);
        __saveAsOpacity( airBOpacityList );
        break;
      case KFLogConfig::AirC:
        __savePen(airCPenList, airCBorder);
        __saveBrush(airCBrushList);
        __saveAsOpacity( airCOpacityList );
        break;
      case KFLogConfig::AirD:
        __savePen(airDPenList, airDBorder);
        __saveBrush(airDBrushList);
        __saveAsOpacity( airDOpacityList );
        break;
      case KFLogConfig::AirElow:
        __savePen(airElPenList, airElBorder);
        __saveBrush(airElBrushList);
        __saveAsOpacity( airElOpacityList );
        break;
      case KFLogConfig::AirE:
        __savePen(airEhPenList, airEhBorder);
        __saveBrush(airEhBrushList);
        __saveAsOpacity( airEhOpacityList );
        break;
      case KFLogConfig::AirF:
        __savePen(airFPenList, airFBorder);
        __saveBrush(airFBrushList);
        __saveAsOpacity( airFOpacityList );
        break;
      case KFLogConfig::AirFir:
        __savePen(airFirPenList, airFirBorder);
        __saveBrush(airFirBrushList);
        __saveAsOpacity( airFirOpacityList );
        break;
      case KFLogConfig::ControlC:
        __savePen(ctrCPenList, ctrCBorder);
        __saveBrush(ctrCBrushList);
        __saveAsOpacity( ctrCOpacityList );
        break;
      case KFLogConfig::ControlD:
        __savePen(ctrDPenList, ctrDBorder);
        __saveBrush(ctrDBrushList);
        __saveAsOpacity( ctrDOpacityList );
        break;
      case KFLogConfig::Danger:
        __savePen(dangerPenList, dangerBorder);
        __saveBrush(dangerBrushList);
        __saveAsOpacity( dangerOpacityList );
        break;
      case KFLogConfig::LowFlight:
        __savePen(lowFPenList, lowFBorder);
        __saveBrush(lowFBrushList);
        __saveAsOpacity( lowFOpacityList );
        break;
      case KFLogConfig::Restricted:
        __savePen(restrPenList, restrBorder);
        __saveBrush(restrBrushList);
        __saveAsOpacity( restrOpacityList );
        break;
      case KFLogConfig::Tmz:
        __savePen(tmzPenList, tmzBorder);
        __saveBrush(tmzBrushList);
        __saveAsOpacity( tmzOpacityList );
        break;
      case KFLogConfig::GliderSector:
        __savePen(gsPenList, gsBorder);
        __saveBrush(gsBrushList);
        __saveAsOpacity( gsOpacityList );
        break;
      case KFLogConfig::WaveWindow:
        __savePen(wwPenList, wwBorder);
        __saveBrush(wwBrushList);
        __saveAsOpacity( wwOpacityList );
        break;

      case KFLogConfig::Forest:
        __savePen(forestPenList, forestBorder);
        __saveBrush(forestBrushList);
        break;
      case KFLogConfig::Glacier:
        __savePen(glacierPenList, glacierBorder);
        __saveBrush(glacierBrushList);
        break;
      case KFLogConfig::PackIce:
        __savePen(packicePenList, packiceBorder);
        __saveBrush(packiceBrushList);
        break;
      case KFLogConfig::FAIAreaLow500:
        __savePen(faiAreaLow500PenList, faiAreaLow500Border);
        __saveBrush(faiAreaLow500BrushList);
        break;
      case KFLogConfig::FAIAreaHigh500:
        __savePen(faiAreaHigh500PenList, faiAreaHigh500Border);
        __saveBrush(faiAreaHigh500BrushList);
        break;
      default:
        break;
    }

  switch(elementID)
    {
      case KFLogConfig::Trail:
        __showPen(trailPenList, trailBorder);
        break;
      case KFLogConfig::Road:
        __showPen(roadPenList, roadBorder);
        break;
      case KFLogConfig::Motorway:
        __showPen(motorwayPenList, motorwayBorder);
        break;
      case KFLogConfig::Railway:
        __showPen(railPenList, railBorder);
        break;
      case KFLogConfig::Railway_D:
        __showPen(rail_dPenList, rail_dBorder);
        break;
      case KFLogConfig::Aerial_Cable:
        __showPen(aerialcablePenList, aerialcableBorder);
        break;
      case KFLogConfig::River:
        __showPen(riverPenList, riverBorder);
        __showBrush(riverBrushList);
        break;
      case KFLogConfig::Canal:
        __showPen(canalPenList, canalBorder);
        break;
      case KFLogConfig::River_T:
        __showPen(river_tPenList, river_tBorder);
        __showBrush(river_tBrushList);
        break;
      case KFLogConfig::City:
        __showPen(cityPenList, cityBorder);
        __showBrush(cityBrushList);
        break;

      case KFLogConfig::AirA:
        __showPen(airAPenList, airABorder);
        __showBrush(airABrushList);
        __showAsOpacity(airAOpacityList);
        break;
      case KFLogConfig::AirB:
        __showPen(airBPenList, airBBorder);
        __showBrush(airBBrushList);
        __showAsOpacity(airBOpacityList);
        break;
      case KFLogConfig::AirC:
        __showPen(airCPenList, airCBorder);
        __showBrush(airCBrushList);
        __showAsOpacity(airCOpacityList);
        break;
      case KFLogConfig::AirD:
        __showPen(airDPenList, airDBorder);
        __showBrush(airDBrushList);
        __showAsOpacity(airDOpacityList);
        break;
      case KFLogConfig::AirElow:
        __showPen(airElPenList, airElBorder);
        __showBrush(airElBrushList);
        __showAsOpacity(airElOpacityList);
        break;
      case KFLogConfig::AirE:
        __showPen(airEhPenList, airEhBorder);
        __showBrush(airEhBrushList);
        __showAsOpacity(airEhOpacityList);
         break;
      case KFLogConfig::AirF:
        __showPen(airFPenList, airFBorder);
        __showBrush(airFBrushList);
        __showAsOpacity(airFOpacityList);
        break;
      case KFLogConfig::AirFir:
        __showPen(airFirPenList, airFirBorder);
        __showBrush(airFirBrushList);
        __showAsOpacity(airFirOpacityList);
        break;
      case KFLogConfig::ControlC:
         __showPen(ctrCPenList, ctrCBorder);
         __showBrush(ctrCBrushList);
         __showAsOpacity(ctrCOpacityList);
        break;
      case KFLogConfig::ControlD:
        __showPen(ctrDPenList, ctrDBorder);
        __showBrush(ctrDBrushList);
        __showAsOpacity(ctrDOpacityList);
        break;
      case KFLogConfig::Danger:
        __showPen(dangerPenList, dangerBorder);
        __showBrush(dangerBrushList);
        break;
      case KFLogConfig::LowFlight:
        __showPen(lowFPenList, lowFBorder);
        __showBrush(lowFBrushList);
        __showAsOpacity(lowFOpacityList);
        break;
      case KFLogConfig::Restricted:
        __showPen(restrPenList, restrBorder);
        __showBrush(restrBrushList);
        __showAsOpacity(restrOpacityList);
        break;
      case KFLogConfig::Tmz:
        __showPen(tmzPenList, tmzBorder);
        __showBrush(tmzBrushList);
        __showAsOpacity(tmzOpacityList);
        break;
      case KFLogConfig::GliderSector:
        __showPen(gsPenList, gsBorder);
        __showBrush(gsBrushList);
        __showAsOpacity(gsOpacityList);
        break;
      case KFLogConfig::WaveWindow:
        __showPen(wwPenList, wwBorder);
        __showBrush(wwBrushList);
        __showAsOpacity(wwOpacityList);
        break;

      case KFLogConfig::Forest:
        __showPen(forestPenList, forestBorder);
        __showBrush(forestBrushList);
        break;
      case KFLogConfig::Glacier:
        __showPen(glacierPenList, glacierBorder);
        __showBrush(glacierBrushList);
        break;
      case KFLogConfig::PackIce:
        __showPen(packicePenList, packiceBorder);
        __showBrush(packiceBrushList);
        break;
      case KFLogConfig::FAIAreaLow500:
        __showPen(faiAreaLow500PenList, faiAreaLow500Border);
        __showBrush(faiAreaLow500BrushList);
        break;
      case KFLogConfig::FAIAreaHigh500:
        __showPen(faiAreaHigh500PenList, faiAreaHigh500Border);
        __showBrush(faiAreaHigh500BrushList);
        break;
      default:
        qWarning() << "ConfigDrawElement::slotSelectElement: Can not handle Id="
                   << elementID;
    }

  oldElement = elementID;
  slotToggleFirst(border1->isChecked());
}

void ConfigMapElement::slotToggleFirst(bool toggle)
{
  border1Button->setEnabled(toggle);
  border1Pen->setEnabled(toggle);
  border1ColorButton->setEnabled(toggle);
  border2->setEnabled(toggle);

  switch(currentElement)
    {
      case KFLogConfig::City:
        border1PenStyle->setEnabled(false);
        border1BrushColorButton->setEnabled(toggle);
        border1BrushStyle->setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirE:
      case KFLogConfig::AirF:
      case KFLogConfig::AirFir:
      case KFLogConfig::ControlC:
      case KFLogConfig::ControlD:
      case KFLogConfig::LowFlight:
      case KFLogConfig::Danger:
      case KFLogConfig::Restricted:
      case KFLogConfig::Tmz:
      case KFLogConfig::GliderSector:
      case KFLogConfig::WaveWindow:
      case KFLogConfig::Prohibited:

      case KFLogConfig::Glacier:
      case KFLogConfig::PackIce:
      case KFLogConfig::River:
      case KFLogConfig::River_T:
      case KFLogConfig::FAIAreaLow500:
      case KFLogConfig::FAIAreaHigh500:
        border1PenStyle->setEnabled(toggle);
        border1BrushColorButton->setEnabled(toggle);
        border1BrushStyle->setEnabled(toggle);
        slotCheckOpacity1( border1BrushStyle->currentIndex() );
        connect( border1BrushStyle, SIGNAL(currentIndexChanged(int)),
                 this, SLOT(slotCheckOpacity1(int)) );
        break;
      case KFLogConfig::Forest:
        //border1ColorButton->setEnabled(false);
        //border1Pen->setEnabled(false);
        border1PenStyle->setEnabled(false);
        border1BrushColorButton->setEnabled(toggle);
        border1BrushStyle->setEnabled(toggle);
        break;
      default:
        border1PenStyle->setEnabled(toggle);
        border1BrushColorButton->setEnabled(false);
        border1BrushStyle->setEnabled(false);
    }

  if( ! toggle )
    {
      slotToggleSecond( false );
    }
  else
    {
      slotToggleSecond( border2->isChecked() );
    }
}

void ConfigMapElement::slotToggleSecond( bool toggle )
{
  border2Button->setEnabled( toggle );
  border2Pen->setEnabled( toggle );
  border2ColorButton->setEnabled( toggle );
  border3->setEnabled( toggle );

  switch( currentElement )
    {
      case KFLogConfig::City:
        border2PenStyle->setEnabled(false);
        border2BrushColorButton->setEnabled(toggle);
        border2BrushStyle->setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirE:
      case KFLogConfig::AirF:
      case KFLogConfig::AirFir:
      case KFLogConfig::ControlC:
      case KFLogConfig::ControlD:
      case KFLogConfig::LowFlight:
      case KFLogConfig::Danger:
      case KFLogConfig::Restricted:
      case KFLogConfig::Tmz:
      case KFLogConfig::GliderSector:
      case KFLogConfig::WaveWindow:
      case KFLogConfig::Prohibited:

      case KFLogConfig::Glacier:
      case KFLogConfig::PackIce:
      case KFLogConfig::River:
      case KFLogConfig::River_T:
      case KFLogConfig::FAIAreaLow500:
      case KFLogConfig::FAIAreaHigh500:
        border2PenStyle->setEnabled(toggle);
        border2BrushColorButton->setEnabled(toggle);
        border2BrushStyle->setEnabled(toggle);
        slotCheckOpacity2( border2BrushStyle->currentIndex() );
        connect( border2BrushStyle, SIGNAL(currentIndexChanged(int)),
                 this, SLOT(slotCheckOpacity2(int)) );
        break;
      case KFLogConfig::Forest:
        //border2ColorButton->setEnabled(false);
        //border2Pen->setEnabled(false);
        border2PenStyle->setEnabled(false);
        border2BrushColorButton->setEnabled(toggle);
        border2BrushStyle->setEnabled(toggle);
        break;
      default:
        border2PenStyle->setEnabled(toggle);
        border2BrushColorButton->setEnabled(false);
        border2BrushStyle->setEnabled(false);
    }

  if( !toggle )
    {
      slotToggleThird(false);
    }
  else
    {
      slotToggleThird(border3->isChecked());
    }
}

void ConfigMapElement::slotToggleThird(bool toggle)
{
  border3Button->setEnabled( toggle );
  border3Pen->setEnabled( toggle );
  border3ColorButton->setEnabled( toggle );
  border4->setEnabled( toggle );

  switch( currentElement )
    {
      case KFLogConfig::City:
        border3PenStyle->setEnabled(false);
        border3BrushColorButton->setEnabled(toggle);
        border3BrushStyle->setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirE:
      case KFLogConfig::AirF:
      case KFLogConfig::AirFir:
      case KFLogConfig::ControlC:
      case KFLogConfig::ControlD:
      case KFLogConfig::LowFlight:
      case KFLogConfig::Danger:
      case KFLogConfig::Restricted:
      case KFLogConfig::Tmz:
      case KFLogConfig::GliderSector:
      case KFLogConfig::WaveWindow:
      case KFLogConfig::Prohibited:

      case KFLogConfig::Glacier:
      case KFLogConfig::PackIce:
      case KFLogConfig::River:
      case KFLogConfig::River_T:
      case KFLogConfig::FAIAreaLow500:
      case KFLogConfig::FAIAreaHigh500:
        border3PenStyle->setEnabled(toggle);
        border3BrushColorButton->setEnabled(toggle);
        border3BrushStyle->setEnabled(toggle);
        slotCheckOpacity3( border3BrushStyle->currentIndex() );
        connect( border3BrushStyle, SIGNAL(currentIndexChanged(int)),
                 this, SLOT(slotCheckOpacity3(int)) );
        break;
      case KFLogConfig::Forest:
        //border3ColorButton->setEnabled(false);
        //border3Pen->setEnabled(false);
        border3PenStyle->setEnabled(false);
        border3BrushColorButton->setEnabled(toggle);
        border3BrushStyle->setEnabled(toggle);
        break;
      default:
        border3PenStyle->setEnabled(toggle);
        border3BrushColorButton->setEnabled(false);
        border3BrushStyle->setEnabled(false);
    }

  if(!toggle)
    {
      slotToggleForth(false);
    }
  else
    {
      slotToggleForth(border4->isChecked());
    }
}

void ConfigMapElement::slotToggleForth(bool toggle)
{
  border4ColorButton->setEnabled(toggle);
  border4Pen->setEnabled(toggle);

  switch(currentElement)
    {
      case KFLogConfig::City:
        border4PenStyle->setEnabled(false);
        border4BrushColorButton->setEnabled(toggle);
        border4BrushStyle->setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirE:
      case KFLogConfig::AirF:
      case KFLogConfig::AirFir:
      case KFLogConfig::ControlC:
      case KFLogConfig::ControlD:
      case KFLogConfig::LowFlight:
      case KFLogConfig::Danger:
      case KFLogConfig::Restricted:
      case KFLogConfig::Tmz:
      case KFLogConfig::GliderSector:
      case KFLogConfig::WaveWindow:
      case KFLogConfig::Prohibited:

      case KFLogConfig::Glacier:
      case KFLogConfig::PackIce:
      case KFLogConfig::River:
      case KFLogConfig::River_T:
      case KFLogConfig::FAIAreaLow500:
      case KFLogConfig::FAIAreaHigh500:
        border4PenStyle->setEnabled(toggle);
        border4BrushColorButton->setEnabled(toggle);
        border4BrushStyle->setEnabled(toggle);
        slotCheckOpacity4( border4BrushStyle->currentIndex() );
        connect( border4BrushStyle, SIGNAL(currentIndexChanged(int)),
                 this, SLOT(slotCheckOpacity4(int)) );
        break;
      case KFLogConfig::Forest:
        //border4ColorButton->setEnabled(false);
        //border4Pen->setEnabled(false);
        border4PenStyle->setEnabled(false);
        border4BrushColorButton->setEnabled(toggle);
        border4BrushStyle->setEnabled(toggle);
        break;
      default:
        border4PenStyle->setEnabled(toggle);
        border4BrushColorButton->setEnabled(false);
        border4BrushStyle->setEnabled(false);
    }
}

void ConfigMapElement::slotSetSecond()
{
  QPixmap buttonPixmap(45, 14);

  border2Color = border1Color;
  buttonPixmap.fill(border2Color);
  border2ColorButton->setIcon(buttonPixmap);
  border2ColorButton->setIconSize(buttonPixmap.size());
  border2Pen->setValue(border1Pen->value());
  border2PenStyle->setCurrentIndex(border1PenStyle->currentIndex());

  border2BrushColor = border1BrushColor;
  buttonPixmap.fill(border2BrushColor);
  border2BrushColorButton->setIcon(buttonPixmap);
  border2BrushColorButton->setIconSize(buttonPixmap.size());
  border2BrushStyle->setCurrentIndex(border1BrushStyle->currentIndex());

  opacity2->setValue( opacity1->value() );
}

void ConfigMapElement::slotSetThird()
{
  QPixmap buttonPixmap(45, 14);

  border3Color = border2Color;
  buttonPixmap.fill(border3Color);
  border3ColorButton->setIcon(buttonPixmap);
  border3ColorButton->setIconSize(buttonPixmap.size());
  border3Pen->setValue(border2Pen->value());
  border3PenStyle->setCurrentIndex(border2PenStyle->currentIndex());

  border3BrushColor = border2BrushColor;
  buttonPixmap.fill(border3BrushColor);
  border3BrushColorButton->setIcon(buttonPixmap);
  border3BrushColorButton->setIconSize(buttonPixmap.size());
  border3BrushStyle->setCurrentIndex(border2BrushStyle->currentIndex());

  opacity3->setValue( opacity2->value() );
}

void ConfigMapElement::slotSetForth()
{
  QPixmap buttonPixmap(45, 14);

  border4Color = border3Color;
  buttonPixmap.fill(border4Color);
  border4ColorButton->setIcon(buttonPixmap);
  border4ColorButton->setIconSize(buttonPixmap.size());
  border4Pen->setValue(border3Pen->value());
  border4PenStyle->setCurrentIndex(border3PenStyle->currentIndex());

  border4BrushColor = border3BrushColor;
  buttonPixmap.fill(border4BrushColor);
  border4BrushColorButton->setIcon(buttonPixmap);
  border4BrushColorButton->setIconSize(buttonPixmap.size());
  border4BrushStyle->setCurrentIndex(border3BrushStyle->currentIndex());

  opacity4->setValue( opacity3->value() );
}

void ConfigMapElement::__defaultPen( QList<QPen> &penList,
                                      bool *b,
                                      QColor defaultColor1,
                                      QColor defaultColor2,
                                      QColor defaultColor3,
                                      QColor defaultColor4,
                                      int defaultPenSize1,
                                      int defaultPenSize2,
                                      int defaultPenSize3,
                                      int defaultPenSize4 )
{
  penList[0].setColor( defaultColor1 );
  penList[0].setWidth( defaultPenSize1 );
  penList[1].setColor( defaultColor2 );
  penList[1].setWidth( defaultPenSize2 );
  penList[2].setColor( defaultColor3 );
  penList[2].setWidth( defaultPenSize3 );
  penList[3].setColor( defaultColor4 );
  penList[3].setWidth( defaultPenSize4 );

  b[0] = true;
  b[1] = true;
  b[2] = true;
  b[3] = true;
}

void ConfigMapElement::__defaultPenBrush( QList<QPen> &penList,
                                           bool *b,
                                           QList<QBrush> &brushList,
                                           QColor defaultColor1,
                                           QColor defaultColor2,
                                           QColor defaultColor3,
                                           QColor defaultColor4,
                                           int defaultPenSize1,
                                           int defaultPenSize2,
                                           int defaultPenSize3,
                                           int defaultPenSize4,
                                           QColor defaultBrushColor1,
                                           QColor defaultBrushColor2,
                                           QColor defaultBrushColor3,
                                           QColor defaultBrushColor4,
                                           Qt::BrushStyle defaultBrushStyle1,
                                           Qt::BrushStyle defaultBrushStyle2,
                                           Qt::BrushStyle defaultBrushStyle3,
                                           Qt::BrushStyle defaultBrushStyle4 )
{
  __defaultPen( penList, b, defaultColor1, defaultColor2, defaultColor3, defaultColor4,
                defaultPenSize1, defaultPenSize2, defaultPenSize3, defaultPenSize4);

  brushList[0].setColor( defaultBrushColor1 );
  brushList[0].setStyle( defaultBrushStyle1 );
  brushList[1].setColor( defaultBrushColor2 );
  brushList[1].setStyle( defaultBrushStyle2 );
  brushList[2].setColor( defaultBrushColor3 );
  brushList[2].setStyle( defaultBrushStyle3 );
  brushList[3].setColor( defaultBrushColor4 );
  brushList[3].setStyle( defaultBrushStyle4 );
}

void ConfigMapElement::__fillStyle( QComboBox *pen, QComboBox *brush )
{
  QPixmap pixmap = _mainWindow->getPixmap("solid.png");
  pen->setIconSize( pixmap.size() );

  pen->addItem(_mainWindow->getPixmap("solid.png"), "", Qt::SolidLine);
  pen->addItem(_mainWindow->getPixmap("dashed.png"), "", Qt::DashLine);
  pen->addItem(_mainWindow->getPixmap("dotted.png"), "", Qt::DotLine);
  pen->addItem(_mainWindow->getPixmap("dashdot.png"), "", Qt::DashDotLine);
  pen->addItem(_mainWindow->getPixmap("dashdotdot.png"), "", Qt::DashDotDotLine);

  brush->setIconSize( brushStyleIcons[0].size() );

  for( int i = 0; i < 15; i++ )
    {
      brush->addItem( brushStyleIcons[i], "", brushStyles[i] );
    }
}

void ConfigMapElement::__readBorder( QString group, bool *array )
{
  array[0] = _settings.value( "/Map/" + group + itemPrefix + "Border1", true ).toBool();
  array[1] = _settings.value( "/Map/" + group + itemPrefix + "Border2", true ).toBool();
  array[2] = _settings.value( "/Map/" + group + itemPrefix + "Border3", true ).toBool();
  array[3] = _settings.value( "/Map/" + group + itemPrefix + "Border4", true ).toBool();
}

void ConfigMapElement::__readAsOpacity( QString group, QList<int> &opacityList )
{
  int o1 = _settings.value( "/Map/" + group + itemPrefix + "Opacity1", AS_OPACITY_1 ).toInt();
  int o2 = _settings.value( "/Map/" + group + itemPrefix + "Opacity2", AS_OPACITY_2 ).toInt();
  int o3 = _settings.value( "/Map/" + group + itemPrefix + "Opacity3", AS_OPACITY_3 ).toInt();
  int o4 = _settings.value( "/Map/" + group + itemPrefix + "Opacity4", AS_OPACITY_4 ).toInt();

  opacityList << o1 << o2 << o3 << o4;
}

void ConfigMapElement::__writeAsOpacity( QString group, QList<int> &opacityList )
{
  _settings.setValue( "/Map/" + group + itemPrefix + "Opacity1", opacityList.at(0) );
  _settings.setValue( "/Map/" + group + itemPrefix + "Opacity2", opacityList.at(1) );

  if( configureMap )
    {
      _settings.setValue( "/Map/" + group + itemPrefix + "Opacity3", opacityList.at(2) );
      _settings.setValue( "/Map/" + group + itemPrefix + "Opacity4", opacityList.at(3) );
    }
}

void ConfigMapElement::__saveAsOpacity( QList<int> &opacityList )
{
  opacityList.clear();

  opacityList << opacity1->value()
              << opacity2->value()
              << opacity3->value()
              << opacity4->value();
}

void ConfigMapElement::__showAsOpacity( QList<int> &opacityList )
{
  opacity1->setValue( opacityList.at(0) );
  opacity2->setValue( opacityList.at(1) );
  opacity3->setValue( opacityList.at(2) );
  opacity4->setValue( opacityList.at(3) );
}

void ConfigMapElement::__defaultAsOpacity( QList<int> &opacityList )
{
  opacityList.clear();
  opacityList << AS_OPACITY_1 << AS_OPACITY_2 << AS_OPACITY_3 << AS_OPACITY_4;
}

void ConfigMapElement::__readPen( QString group,
                                   QList<QPen> &penList,
                                   QColor defaultColor1,
                                   QColor defaultColor2,
                                   QColor defaultColor3,
                                   QColor defaultColor4,
                                   int defaultPenSize1,
                                   int defaultPenSize2,
                                   int defaultPenSize3,
                                   int defaultPenSize4,
                                   Qt::PenStyle defaultPenStyle1,
                                   Qt::PenStyle defaultPenStyle2,
                                   Qt::PenStyle defaultPenStyle3,
                                   Qt::PenStyle defaultPenStyle4 )
{
  Q_UNUSED( defaultPenSize3 )
  Q_UNUSED( defaultPenSize4 )

  QColor c1( _settings.value( "/Map/" + group + itemPrefix + "Color1", defaultColor1.name() ).value<QColor>() );
  QColor c2( _settings.value( "/Map/" + group + itemPrefix + "Color2", defaultColor2.name() ).value<QColor>() );
  QColor c3( _settings.value( "/Map/" + group + itemPrefix + "Color3", defaultColor3.name() ).value<QColor>() );
  QColor c4( _settings.value( "/Map/" + group + itemPrefix + "Color4", defaultColor4.name() ).value<QColor>() );

  int ps1 = _settings.value( "/Map/" + group + itemPrefix + "PenSize1", defaultPenSize1 ).toInt();
  int ps2 = _settings.value( "/Map/" + group + itemPrefix + "PenSize2", defaultPenSize2 ).toInt();
  int ps3 = _settings.value( "/Map/" + group + itemPrefix + "PenSize3", defaultPenSize2 ).toInt();
  int ps4 = _settings.value( "/Map/" + group + itemPrefix + "PenSize4", defaultPenSize2 ).toInt();

  Qt::PenStyle pst1 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + itemPrefix + "PenStyle1", defaultPenStyle1).toInt());
  Qt::PenStyle pst2 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + itemPrefix + "PenStyle2", defaultPenStyle2).toInt());
  Qt::PenStyle pst3 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + itemPrefix + "PenStyle3", defaultPenStyle3).toInt());
  Qt::PenStyle pst4 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + itemPrefix + "PenStyle4", defaultPenStyle4).toInt());

  QPen pen1(c1);
  QPen pen2(c2);
  QPen pen3(c3);
  QPen pen4(c4);

  pen1.setWidth( ps1 );
  pen2.setWidth( ps2 );
  pen3.setWidth( ps3 );
  pen4.setWidth( ps4 );

  pen1.setStyle( pst1 );
  pen2.setStyle( pst2 );
  pen3.setStyle( pst3 );
  pen4.setStyle( pst4 );

  penList.append( pen1 );
  penList.append( pen2 );
  penList.append( pen3 );
  penList.append( pen4 );
}

void ConfigMapElement::__readBrush( QString group,
                                     QList<QBrush> &brushList,
                                     QColor defaultBrushColor1,
                                     QColor defaultBrushColor2,
                                     QColor defaultBrushColor3,
                                     QColor defaultBrushColor4,
                                     Qt::BrushStyle defaultBrushStyle1,
                                     Qt::BrushStyle defaultBrushStyle2,
                                     Qt::BrushStyle defaultBrushStyle3,
                                     Qt::BrushStyle defaultBrushStyle4 )
{
  QColor c1( _settings.value( "/Map/" + group + itemPrefix + "BrushColor1", defaultBrushColor1.name() ).value<QColor>() );
  QColor c2( _settings.value( "/Map/" + group + itemPrefix + "BrushColor2", defaultBrushColor2.name() ).value<QColor>() );
  QColor c3( _settings.value( "/Map/" + group + itemPrefix + "BrushColor3", defaultBrushColor3.name() ).value<QColor>() );
  QColor c4( _settings.value( "/Map/" + group + itemPrefix + "BrushColor4", defaultBrushColor4.name() ).value<QColor>() );

  Qt::BrushStyle bs1 = (Qt::BrushStyle) _settings.value( "/Map/" + group + itemPrefix + "BrushStyle1", defaultBrushStyle1 ).toInt();
  Qt::BrushStyle bs2 = (Qt::BrushStyle) _settings.value( "/Map/" + group + itemPrefix + "BrushStyle2", defaultBrushStyle2 ).toInt();
  Qt::BrushStyle bs3 = (Qt::BrushStyle) _settings.value( "/Map/" + group + itemPrefix + "BrushStyle3", defaultBrushStyle3 ).toInt();
  Qt::BrushStyle bs4 = (Qt::BrushStyle) _settings.value( "/Map/" + group + itemPrefix + "BrushStyle4", defaultBrushStyle4 ).toInt();

  brushList.append( QBrush( c1, bs1 ) );
  brushList.append( QBrush( c2, bs2 ) );
  brushList.append( QBrush( c3, bs3 ) );
  brushList.append( QBrush( c4, bs4 ) );
}

void ConfigMapElement::__saveBrush(QList<QBrush> &brushList)
{
  int ix1 = border1BrushStyle->currentIndex();
  int ix2 = border2BrushStyle->currentIndex();
  int ix3 = border3BrushStyle->currentIndex();
  int ix4 = border4BrushStyle->currentIndex();

  brushList[0].setColor(border1BrushColor);
  brushList[0].setStyle((Qt::BrushStyle) border1BrushStyle->itemData(ix1).toInt());
  brushList[1].setColor(border2BrushColor);
  brushList[1].setStyle((Qt::BrushStyle) border2BrushStyle->itemData(ix2).toInt());
  brushList[2].setColor(border3BrushColor);
  brushList[2].setStyle((Qt::BrushStyle) border3BrushStyle->itemData(ix3).toInt());
  brushList[3].setColor(border4BrushColor);
  brushList[3].setStyle((Qt::BrushStyle) border4BrushStyle->itemData(ix4).toInt());
}

void ConfigMapElement::__savePen(QList<QPen> &penList, bool *b)
{
  int ix1 = border1PenStyle->currentIndex();
  int ix2 = border2PenStyle->currentIndex();
  int ix3 = border3PenStyle->currentIndex();
  int ix4 = border4PenStyle->currentIndex();

  b[0] = border1->isChecked();
  penList[0].setColor(border1Color);
  penList[0].setWidth(border1Pen->value());
  penList[0].setStyle((Qt::PenStyle) border1PenStyle->itemData(ix1).toInt());

  b[1] = border2->isChecked();
  penList[1].setColor(border2Color);
  penList[1].setWidth(border2Pen->value());
  penList[1].setStyle((Qt::PenStyle) border2PenStyle->itemData(ix2).toInt());

  b[2] = border3->isChecked();
  penList[2].setColor(border3Color);
  penList[2].setWidth(border3Pen->value());
  penList[2].setStyle((Qt::PenStyle) border3PenStyle->itemData(ix3).toInt());

  b[3] = border4->isChecked();
  penList[3].setColor(border4Color);
  penList[3].setWidth(border4Pen->value());
  penList[3].setStyle((Qt::PenStyle) border4PenStyle->itemData(ix4).toInt());
}

void ConfigMapElement::__showBrush( QList<QBrush> &brushList )
{
  QPixmap buttonPixmap(45, 14);

  border1BrushColor = brushList[0].color();
  buttonPixmap.fill(border1BrushColor);
  border1BrushColorButton->setIcon(buttonPixmap);
  border1BrushColorButton->setIconSize( buttonPixmap.size() );
  border1BrushStyle->setCurrentIndex(border1BrushStyle->findData(brushList[0].style()));

  border2BrushColor = brushList[1].color();
  buttonPixmap.fill(border1BrushColor);
  border2BrushColorButton->setIcon(buttonPixmap);
  border2BrushColorButton->setIconSize( buttonPixmap.size() );
  border2BrushStyle->setCurrentIndex(border2BrushStyle->findData(brushList[1].style()));

  border3BrushColor = brushList[2].color();
  buttonPixmap.fill(border3BrushColor);
  border3BrushColorButton->setIcon(buttonPixmap);
  border3BrushColorButton->setIconSize( buttonPixmap.size() );
  border3BrushStyle->setCurrentIndex(border3BrushStyle->findData(brushList[2].style()));

  border4BrushColor = brushList[3].color();
  buttonPixmap.fill(border4BrushColor);
  border4BrushColorButton->setIcon(buttonPixmap);
  border4BrushColorButton->setIconSize( buttonPixmap.size() );
  border4BrushStyle->setCurrentIndex(border4BrushStyle->findData(brushList[3].style()));
}

void ConfigMapElement::__showPen(QList<QPen> &penList, bool *b)
{
  QPixmap buttonPixmap(45, 14);

  border1->setChecked(b[0]);
  border1Color = penList[0].color();
  buttonPixmap.fill(border1Color);
  border1ColorButton->setIcon(buttonPixmap);
  border1ColorButton->setIconSize( buttonPixmap.size() );
  border1Pen->setValue(penList[0].width());
  border1PenStyle->setCurrentIndex(border1PenStyle->findData(penList[0].style()));

  border2->setChecked(b[1]);
  border2Color = penList[1].color();
  buttonPixmap.fill(border2Color);
  border2ColorButton->setIcon(buttonPixmap);
  border2ColorButton->setIconSize( buttonPixmap.size() );
  border2Pen->setValue(penList[1].width());
  border2PenStyle->setCurrentIndex(border2PenStyle->findData(penList[1].style()));

  border3->setChecked(b[2]);
  border3Color = penList[2].color();
  buttonPixmap.fill(border3Color);
  border3ColorButton->setIcon(buttonPixmap);
  border3ColorButton->setIconSize( buttonPixmap.size() );
  border3Pen->setValue(penList[2].width());
  border3PenStyle->setCurrentIndex(border3PenStyle->findData(penList[2].style()));

  border4->setChecked(b[3]);
  border4Color = penList[3].color();
  buttonPixmap.fill(border4Color);
  border4ColorButton->setIcon(buttonPixmap);
  border4ColorButton->setIconSize( buttonPixmap.size() );
  border4Pen->setValue(penList[3].width());
  border4PenStyle->setCurrentIndex(border4PenStyle->findData(penList[3].style()));
}

void ConfigMapElement::__writeBrush(QString group, QList<QBrush> &brushList, QList<QPen> &penList, bool *b)
{
  __writePen(group, penList, b);

  _settings.setValue("/Map/" + group + itemPrefix + "BrushColor1", brushList.at(0).color().name());
  _settings.setValue("/Map/" + group + itemPrefix + "BrushColor2", brushList.at(1).color().name());
  _settings.setValue("/Map/" + group + itemPrefix + "BrushStyle1", brushList.at(0).style());
  _settings.setValue("/Map/" + group + itemPrefix + "BrushStyle2", brushList.at(1).style());

  if( configureMap )
    {
      _settings.setValue("/Map/" + group + itemPrefix + "BrushColor3", brushList.at(2).color().name());
      _settings.setValue("/Map/" + group + itemPrefix + "BrushColor4", brushList.at(3).color().name());
      _settings.setValue("/Map/" + group + itemPrefix + "BrushStyle3", brushList.at(2).style());
      _settings.setValue("/Map/" + group + itemPrefix + "BrushStyle4", brushList.at(3).style());
    }
}

void ConfigMapElement::__writePen(QString group, QList<QPen> &penList, bool *b)
{
  _settings.setValue("/Map/" + group + itemPrefix + "Color1", penList.at(0).color().name());
  _settings.setValue("/Map/" + group + itemPrefix + "Color2", penList.at(1).color().name());
  _settings.setValue("/Map/" + group + itemPrefix + "PenSize1", (int) penList.at(0).width());
  _settings.setValue("/Map/" + group + itemPrefix + "PenSize2", (int) penList.at(1).width());
  _settings.setValue("/Map/" + group + itemPrefix + "PenStyle1", penList.at(0).style());
  _settings.setValue("/Map/" + group + itemPrefix + "PenStyle2", penList.at(1).style());
  _settings.setValue("/Map/" + group + itemPrefix + "Border1", b[0]);
  _settings.setValue("/Map/" + group + itemPrefix + "Border2", ( b[1] && b[0] ) );

  if( configureMap )
    {
      _settings.setValue("/Map/" + group + itemPrefix + "Color3", penList.at(2).color().name());
      _settings.setValue("/Map/" + group + itemPrefix + "Color4", penList.at(3).color().name());
      _settings.setValue("/Map/" + group + itemPrefix + "PenSize3", (int) penList.at(2).width());
      _settings.setValue("/Map/" + group + itemPrefix + "PenSize4", (int) penList.at(3).width());
      _settings.setValue("/Map/" + group + itemPrefix + "PenStyle3", penList.at(2).style());
      _settings.setValue("/Map/" + group + itemPrefix + "PenStyle4", penList.at(3).style());
      _settings.setValue("/Map/" + group + itemPrefix + "Border3", ( b[2] && b[1] && b[0] ) );
      _settings.setValue("/Map/" + group + itemPrefix + "Border4", ( b[3] && b[2] && b[1] && b[0] ) );
    }
}

void ConfigMapElement::__createBrushStyleIcons()
{
  brushStyles[0] = Qt::NoBrush;
  brushStyles[1] = Qt::SolidPattern;
  brushStyles[2] = Qt::Dense1Pattern;
  brushStyles[3] = Qt::Dense2Pattern;
  brushStyles[4] = Qt::Dense3Pattern;
  brushStyles[5] = Qt::Dense4Pattern;
  brushStyles[6] = Qt::Dense5Pattern;
  brushStyles[7] = Qt::Dense6Pattern;
  brushStyles[8] = Qt::Dense7Pattern;
  brushStyles[9] = Qt::HorPattern;
  brushStyles[10] = Qt::VerPattern;
  brushStyles[11] = Qt::CrossPattern;
  brushStyles[12] = Qt::BDiagPattern;
  brushStyles[13] = Qt::FDiagPattern;
  brushStyles[14] = Qt::DiagCrossPattern;

  const int iconSize = 22;
  const int penWidth = 2;

  for( int i = 0; i < 15; i++ )
    {
      brushStyleIcons[i] = QPixmap( iconSize, iconSize );
      brushStyleIcons[i].fill( Qt::white );

      QPainter painter( &brushStyleIcons[i] );

      QPen pen( Qt::black );
      pen.setWidth( penWidth );

      painter.setPen( pen );
      painter.setBrush( QBrush( Qt::black, brushStyles[i] ));
      painter.drawRect( 2, 2, iconSize-4, iconSize-4 );
    }
}
