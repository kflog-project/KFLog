/***********************************************************************
**
**   kflogconfig.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2010 by Constantijn Neeteson, Heiner Lamprecht, Florian Ehinger
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class KFLogConfig
 *
 * \short Configuration dialog widget
 *
 * This class provides the configuration dialog for KFLog.
 *
 * \author Heiner Lamprecht, Florian Ehinger, Constantijn Neeteson, Axel Pauli
 *
 * \date 2000-2011
 *
 * @version $Id$
 */

#ifndef KFLOG_CONFIG_H
#define KFLOG_CONFIG_H

#include <QButtonGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QDialog>
#include <QFrame>
#include <QLayout>
#include <QLCDNumber>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTreeWidget>
#include <QWidget>

#include "coordedit.h"

class ConfigMapElement;
class ConfigPrintElement;

class KFLogConfig : public QDialog
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( KFLogConfig )

 public:

  KFLogConfig( QWidget* parent=0 );

  virtual ~KFLogConfig();

  /** */
  enum ElementType { Road = 0, Highway, Railway, River, Canal, City,
      AirA, AirB, AirC, AirD, AirElow, AirE, AirF, ControlC, ControlD, Danger,
      LowFlight, Restricted, Prohibited, Tmz, GliderSector, WaveWindow,
      Forest, Trail, Railway_D, Aerial_Cable, River_T,
      Glacier, PackIce, FAIAreaLow500, FAIAreaHigh500, Separator };

  enum DefaultWaypointCatalog { Empty = 0, LastUsed, Specific };

 signals:

  /** */
  void scaleChanged(int min, int max);

  /** Emitted, if the ok button was pressed and all new data have been saved. */
  void configOk();

  /** Emitted to subwidgets that their configuration should be saved. */
  void saveConfig();

  /** */
  void newDrawType(int type);

  /**
   * Emitted to request a Welt2000 file download.
   */
  void downloadWelt2000();

 public slots:
  /** */
  void slotOk();

  void slotPageClicked( QTreeWidgetItem * item, int column );
  /** */
  void slotDefaultPath();
  /** */
  void slotDefaultProjection();
  /** */
  void slotDefaultScale();
  /** */
  void slotSearchFlightPath();
  /** */
  void slotSearchMapPath();
  /** */
  void slotSearchTaskPath();
  /** */
  void slotSearchWaypointPath();
  /** */
  void slotSelectProjection(int);
  /** */
  void slotShowLowerLimit(int value);
  /** */
  void slotShowUpperLimit(int value);
  /** */
  void slotShowSwitchScale(int value);
  /** */
  void slotShowWpLabel(int value);
  /** */
  void slotShowReduceScaleA(int value);
  /** */
  void slotShowReduceScaleB(int value);
  /** */
  void slotShowReduceScaleC(int value);
  /** */
  void slotSelectDefaultCatalog(int item);
  /** */
  void slotDefaultWaypoint();
  /** */
  void slotSearchDefaultWaypoint();
  /**
   * Open a color dialog and let the user define a new color.
   *
   * \param buttonIdentifier Identifier of pressed button.
   */
  void slotSelectFlightTypeColor( int buttonIdentifier );

  /**
   * Reset all flight path line widths to their default values.
   */
  void slotDefaultFlightPathLines();

  /**
   * Resets all flight path colors to their default values.
   */
  void slotDefaultFlightPathColors();

  /**
   * Selects the indexed item in the ConfigDrawElement class.
   */
  void slotSelectDrawElement( int index );

  /**
   * Selects the indexed item in the ConfigPrintElement class.
   */
  void slotSelectPrintElement( int index );

  /**
   * Called if a Welt2000 file shall be downloaded.
   */
  void slotDownloadWelt2000();

 private slots:

 /**
  * Called to make all text to upper cases.
  */
 void slotTextEditedCountry( const QString& text );

 private:

 /** */
  void __addPersonalTab();
  /** */
  void __addMapTab();
  /** */
  void __addFlightTab();
  /** */
  void __addPathTab();
  /** */
  void __addProjectionTab();
  /** */
  void __addScaleTab();
  /** */
  int __setScaleValue(int value);
  /** */
  int __getScaleValue(double value);
  /** */
  void __addAirfieldTab();
  /** */
  void __addWaypointTab();

  /** Adds a tab for unit configuration. */
  void __addUnitTab();

  QGridLayout *configLayout;

  QTreeWidget *setupTree;

  QFrame *activePage;
  /** */
  QFrame* personalPage;
  /** */
  QFrame* mapPage;
  /** */
  QFrame* flightPage;
  /** */
  QFrame* pathPage;
  /** */
  QFrame* topoPage;
  /** */
  QFrame* projPage;
  /** */
  QFrame* scalePage;
  /** */
  QFrame* airfieldPage;
  /** */
  QFrame* waypointPage;
  /** */
  QFrame* unitPage;

  /** */
  QLineEdit* igcPathE;
  QLineEdit* taskPathE;
  QLineEdit* waypointPathE;
  QLineEdit* mapPathE;
  QLineEdit* catalogPathE;
  LatEdit* homeLatE;
  LongEdit* homeLonE;
  QLineEdit* homeCountryE;
  QLineEdit* homeNameE;
  QLineEdit* preNameE;
  QLineEdit* surNameE;
  QLineEdit* dateOfBirthE;

  QSpinBox* altitudePenWidth;
  QSpinBox* cyclingPenWidth;
  QSpinBox* speedPenWidth;
  QSpinBox* varioPenWidth;
  QSpinBox* solidPenWidth;
  QSpinBox* enginePenWidth;

  QLineEdit* filterWelt2000;
  QSpinBox* homeRadiusWelt2000;
  QCheckBox* readOlWelt2000;

  /**
   * Initial value of home radius.
   */
  int homeRadiusWelt2000Value;

  /**
   * Initial value of country filter
   */
  QString filterWelt2000Text;

  /**
   * Initial value of outlanding checkbox.
   */
  bool readOlWelt2000Value;

  QSlider* lLimit;
  QSlider* uLimit;
  QSlider* switchScale;
  QSlider* wpLabel;
  QSlider* reduce1;
  QSlider* reduce2;
  QSlider* reduce3;
  QLCDNumber* lLimitN;
  QLCDNumber* uLimitN;
  QLCDNumber* switchScaleN;
  QLCDNumber* wpLabelN;
  QLCDNumber* reduce1N;
  QLCDNumber* reduce2N;
  QLCDNumber* reduce3N;
  QComboBox* elementSelect;
  QComboBox* projectionSelect;
  LatEdit* firstParallel;
  LatEdit* secondParallel;
  LongEdit* originLongitude;

  QButtonGroup *waypointButtonGroup;
  QPushButton* catalogPathSearch;

  QPushButton* flightTypeLeftTurnColorButton;
  QColor flightTypeLeftTurnColor;
  QPushButton* flightTypeRightTurnColorButton;
  QColor flightTypeRightTurnColor;
  QPushButton* flightTypeMixedTurnColorButton;
  QColor flightTypeMixedTurnColor;
  QPushButton* flightTypeStraightColorButton;
  QColor flightTypeStraightColor;
  QPushButton* flightTypeSolidColorButton;
  QColor flightTypeSolidColor;
  QPushButton* flightTypeEngineNoiseColorButton;
  QColor flightTypeEngineNoiseColor;

  /** Button array */
  QPushButton** ftcButtonArray[6];

  /** Color array of related buttons. */
  QColor* ftcColorArray[6];

  /** Default color array of related buttons. */
  QColor ftcColorArrayDefault[6];

  /** Widget to configure the map draw elements. */
  ConfigMapElement* configDrawWidget;

  /** Widget to configure the map print elements. */
  ConfigMapElement* configPrintWidget;

  int cylinPar;
  int lambertV1;
  int lambertV2;
  int lambertOrigin;
  int currentProjType;

  /** Combo box for distance unit configuration. */
  QComboBox* unitDistance;

  /** Combo box for position unit configuration. */
  QComboBox* unitPosition;

  /** Combo box for altitude unit configuration. */
  QComboBox* unitAltitude;
};

#endif
