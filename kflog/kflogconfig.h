/***********************************************************************
**
**   kflogconfig.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2010 by Constantijn Neeteson, Heiner Lamprecht, Florian Ehinger
**                   2011-2014 by Axel Pauli
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
 * \date 2000-2014
 *
 * @version $Id$
 */

#ifndef KFLOG_CONFIG_H
#define KFLOG_CONFIG_H

#include <QByteArray>
#include <QButtonGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QDialog>
#include <QLayout>
#include <QLCDNumber>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTreeWidget>
#include <QTableWidget>
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
  enum ElementType { Road = 0, Motorway, Railway, River, Canal, City,
      AirA, AirB, AirC, AirD, AirElow, AirE, AirF, AirFir, ControlC, ControlD, Danger,
      LowFlight, Restricted, Prohibited, Tmz, GliderSector, WaveWindow,
      Forest, Trail, Railway_D, Aerial_Cable, River_T,
      Glacier, PackIce, FAIAreaLow500, FAIAreaHigh500, Separator };

  enum DefaultWaypointCatalog { Empty = 0, LastUsed, Specific };

  static QByteArray rot47( const QByteArray& input );

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
  void downloadWelt2000( bool askUser );

  /**
   * Emitted to request a Welt2000 file reload after a configuration change.
   */
  void reloadWelt2000Data();

  /**
   * Emitted to request a reload of all airspace file.
   */
  void airspaceFileListChanged();

  /**
   * Emitted to request a openAIP airspace file download.
   */
  void downloadOpenAipAirspaces( bool askUser );

  /**
   * Emitted to request a openAIP airfield file download.
   */
  void downloadOpenAipAirfields( bool askUser );

 public slots:

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
   * Opens a color dialog and let the user define a new color.
   *
   * \param buttonIdentifier Identifier of pressed button.
   */
  void slotSelectFlightTypeColor( int buttonIdentifier );

  /**
   * Resets all flight path line widths to their default values.
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

 private slots:

 /**
  * Called, if the Ok button is pressed.
  */
 void slotOk();

 /**
  * Called to make all text to upper cases.
  */
 void slotTextEditedCountry( const QString& text );

 /**
 * Called to toggle the check box of the clicked table cell in the airspace
 * file table.
 */
 void slotToggleAsCheckBox( int, int );

 /**
  * Called, if the airfield source has been changed.
  */
 void slotAirfieldSourceChanged(int sourceIndex);

 /**
  * Called if a Welt2000 file shall be downloaded.
  */
 void slotDownloadWelt2000();

 /**
  * Called if openAIP airspace files shall be downloaded.
  */
 void slotDownloadOpenAipAs();

 /**
  * Called if openAIP airfield files shall be downloaded.
  */
 void slotDownloadOpenAipAf();

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
  void __addAirspaceTab();
  /** */
  void __addWaypointTab();

  /** Adds a tab for unit configuration. */
  void __addUnitTab();

  /**
   * Loads the content of the current airspace file directory into the
   * file table.
   */
  void __loadAirspaceFilesIntoTable();

  /**
   * Checks the openAIP country input for correctness. If not correct
   * a message box is popup to inform the user about that fact.
   *
   * \param input String to be checked
   *
   * \return true if checked string is ok otherwise false
   */
  bool __checkOpenAipCountryInput( QString& input );

  /**
   * Checks the Welt2000 country input for correctness. If not correct a message
   * box is popup to inform the user about that fact.
   *
   * \param input String to be checked
   *
   * \return true if checked string is ok otherwise false
   */
  bool __checkWelt2000Input( QString& input );

  QGridLayout *configLayout;

  QTreeWidget *setupTree;

  QWidget *activePage;
  /** */
  QWidget* personalPage;
  /** */
  QWidget* mapPage;
  /** */
  QWidget* flightPage;
  /** */
  QWidget* pathPage;
  /** */
  QWidget* topoPage;
  /** */
  QWidget* projPage;
  /** */
  QWidget* scalePage;
  /** */
  QWidget* airfieldPage;
  /** */
  QWidget* airspacePage;
  /** */
  QWidget* waypointPage;
  /** */
  QWidget* unitPage;

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

  QLineEdit* welt2000CountryFilter;
  QSpinBox*  welt2000HomeRadius;
  QCheckBox* welt2000ReadOl;

  QComboBox* afSourceBox;
  QLineEdit* afOpenAipCountries;
  QSpinBox*  afOpenAipHomeRadius;

  QLineEdit* asOpenAipCountries;

  /**
   * Initial value of home radius.
   */
  int wel2000HomeRadiusValue;

  /**
   * Initial value of country filter
   */
  QString welt2000CountryFilterValue;

  /**
   * Initial value of outlanding checkbox.
   */
  bool welt2000ReadOlValue;

  /**
   * Initial value of openAIP airfield home radius.
   */
  int afOpenAipHomeRadiusValue;

  /**
   * Initial value of openAIP airfield countries
   */
  QString afOpenAipCountryValue;

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

  /** Table for airspace files to be loaded. */
  QTableWidget* asFileTable;

  /** Table for openAIP airfield files to be loaded. */
  QTableWidget* afFileTable;

  /** Group box for Welt2000 airfield configuration widget. */
  QGroupBox* welt2000Group;

  /** Group box for openAIP airfield configuration widget. */
  QGroupBox* openAipGroup;
};

#endif
