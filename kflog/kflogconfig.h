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
 * \version 1.2
 */

#ifndef KFLOG_CONFIG_H
#define KFLOG_CONFIG_H

#include <QByteArray>
#include <QButtonGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QLCDNumber>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTreeWidget>
#include <QTableWidget>
#include <QTranslator>
#include <QWidget>

#include "coordedit.h"

class AirfieldSelectionList;
class ConfigMapElement;
class ConfigPrintElement;
class SinglePoint;

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
      AirA, AirB, AirC, AirD, AirElow, AirE, AirF, AirFir, Ctr, Danger,
      LowFlight, Restricted, Prohibited, Rmz, Tmz, GliderSector, WaveWindow,
      Forest, Trail, Railway_D, Aerial_Cable, River_T,
      Glacier, PackIce, FAIAreaLow500, FAIAreaHigh500, Separator };

  enum DefaultWaypointCatalog { Empty = 0, LastUsed, Specific };

  /**
   * Define an enumeration type as point source index.
   */
  enum PointSourceIndex { OpenAIP=0, Welt2000=1 };

  static QByteArray rot47( const QByteArray& input );

  /**
   * Checks, if a default waypoint catalog is defined.
   *
   * \return true in case of success otherwise false
   */
  static bool existsDefaultWaypointCatalog();

  /**
   * Gets the path to the default waypoint catalog.
   *
   * \return Path to catalog or empty string, if catalog is undefined.
   */
  static QString getDefaultWaypointCatalog();

  /**
   * Sets the passed catalog as last used catalog, if the user has selected
   * that in its settings.
   *
   * \param catalog Path to catalog.
   */
  static void setLastUsedWaypointCatalog( QString& catalog );

  /**
   * Sets the language for the GUI surface to the new passed country code.
   * If the passed argument is an empty string the language is reset to
   * the default language English.
   *
   * \param newLanguage two letter coutry code for the new language to be
   *        selected. If empty, English is set as default.
   *
   * \return True in case of success, otherwise false.
   */
  static bool setGuiLanguage( QString newLanguage="" );

 protected:

  void showEvent( QShowEvent *event );

 signals:

  /** */
  void scaleChanged(int min, int max);

  /** Emitted, if the Ok button was pressed and all new data have been saved. */
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
   * Emitted to request a Welt2000 update check.
   */
  void checkWelt20004Update();

  /**
   * Emitted to request point data reload after a configuration change.
   */
  void reloadPointData();

  /**
   * Emitted to request a reload of all airspace file.
   */
  void airspaceFileListChanged();

  /**
   * Emitted to request an openAIP airspace file download.
   */
  void downloadOpenAipAirspaces( bool askUser );

  /**
   * Emitted to request an openAIP point data file download.
   */
  void downloadOpenAipPointFiles( bool askUser );

  /**
   * Emitted to initiate an openAIP point data update check.
   */
  void checkOpenAipPointData4Update();

  /**
   * Emitted to initiate an openAIP airspace data update check.
   */
  void checkOpenAipAsData4Update();

  /**
   * Emitted, if the map elements should be set to their default values.
   */
  void setMapElements2Default();

  /**
   * Emitted, if the map scale threshold is changed.
   *
   * \param thresholdNumber The number of the threshold, which is changed
   *
   * \param newValue The new threshold value
   */
  void scaleThresholdChanged( int thresholdNumber, int newValue );

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

  /**
   * Loads the content of the current airspace file directory into the
   * file table.
   */
  void slotLoadAirspaceFilesIntoTable();

  /**
   * Loads the content of the current airfield file directory into the
   * file table.
   */
  void slotLoadOpenAipPointFilesIntoTable();

  /**
   * Takes this point as new homesite position.
   */
  void slotTakeThisPoint( const SinglePoint* sp );

 private slots:

 /**
  * Called, if the OK button was pressed.
  */
 void slotOk();

 /**
  * Called to convert all entered text to upper cases.
  */
 void slotTextEditedCountry( const QString& text );

 /**
 * Called to toggle the check box of the clicked table cell in the airspace
 * file table.
 */
 void slotToggleAsCheckBox( int, int );

 /**
 * Called to toggle the check box of the clicked table cell in the point source
 * file table.
 */
 void slotTogglePointSourceCheckBox( int, int );

 /**
  * Called, if the point source has been changed.
  */
 void slotPointSourceChanged(int sourceIndex);

 /**
  * Called if a Welt2000 file shall be downloaded.
  */
 void slotDownloadWelt2000();

 /**
  * Called if openAIP airspace files shall be downloaded.
  */
 void slotDownloadOpenAipAs();

 /**
  * Called if openAIP point data files shall be downloaded.
  */
 void slotDownloadOpenAipPointData();

 /**
  * Called to set all map elements to their default value.
  */
 void slotSetMapElements2Default();

 /**
  * Called, if the Welt2000 update checkbox is clicked.
  */
 void slotWelt2000UpdateStateChanged( int state );

 /**
  * Called, if the value in the Welt2000 update box is changed.
  */
 void slotWelt2000UpdatePeriodChanged( int newValue );

 /**
  * Called, if the openAIP point update checkbox is clicked.
  */
 void slotPointsOpenAipUpdateStateChanged( int state );

 /**
  * Called, if the value in the openAIP point update box is changed.
  */
 void slotPointsOpenAipUpdatePeriodChanged( int newValue );

 /**
  * Called, if the openAIP airspace update checkbox is clicked.
  */
 void slotAsOpenAipUpdateStateChanged( int state );

 /**
  * Called, if the value in the openAIP airspace update box is changed.
  */
 void slotAsOpenAipUpdatePeriodChanged( int newValue );

 private:

 /** Creates a special configuration widget. */
  void __addPersonalTab();

  /** Creates a special configuration widget. */
  void __addMapTab();

  /** Creates a special configuration widget. */
  void __addFlightTab();

  /** Creates a special configuration widget. */
  void __addPathTab();

  /** Creates a special configuration widget. */
  void __addProjectionTab();

  /** Creates a special configuration widget. */
  void __addScaleTab();

  /** Creates a special configuration widget. */
  int __setScaleValue(int value);

  /** Creates a special configuration widget. */
  int __getScaleValue(double value);

  /** Creates a special configuration widget. */
  void __addPointsTab();

  /** Creates a special configuration widget. */
  void __addAirspaceTab();

  /** Creates a special configuration widget. */
  void __addWaypointTab();

  /** Creates a special configuration widget. */
  void __addUnitTab();

  /**
   * Checks the airspace file table for changes.
   */
  void __checkAirspaceFileTable();

  /**
   * Checks the point file table for changes.
   *
   * \returns true in case of changes otherwise false.
   */
  bool __checkPointFileTable();

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

  /**
   * Sets the entries in the language combo box.
   */
  void __setLanguageEntriesInBox();

  /**
   * Main layout manager for this class.
   */
  QGridLayout *configLayout;

  QTreeWidget *setupTree;

  /** Points to the activated and visible configuration page. */
  QWidget *activePage;

  /** Configuration page */
  QWidget* personalPage;
  /** Configuration page */
  QWidget* mapPage;
  /** Configuration page */
  QWidget* flightPage;
  /** Configuration page */
  QWidget* pathPage;
  /** Configuration page */
  QWidget* topoPage;
  /** Configuration page */
  QWidget* projPage;
  /** Configuration page */
  QWidget* scalePage;
  /** Configuration page */
  QWidget* pointsPage;
  /** Configuration page */
  QWidget* airspacePage;
  /** Configuration page */
  QWidget* waypointPage;
  /** Configuration page */
  QWidget* unitPage;

  QLineEdit* igcPathE;
  QLineEdit* taskPathE;
  QLineEdit* waypointPathE;
  QLineEdit* mapPathE;
  QLineEdit* catalogPathE;
  LatEdit*   homeLatE;
  LongEdit*  homeLonE;
  QLineEdit* homeCountryE;
  QLineEdit* homeNameE;
  QLineEdit* preNameE;
  QLineEdit* surNameE;
  QLineEdit* dateOfBirthE;
  QComboBox* languageBox;

  QSpinBox* altitudePenWidth;
  QSpinBox* cyclingPenWidth;
  QSpinBox* speedPenWidth;
  QSpinBox* varioPenWidth;
  QSpinBox* solidPenWidth;
  QSpinBox* enginePenWidth;

  QLineEdit* welt2000CountryFilter;
  QSpinBox*  welt2000HomeRadius;
  QCheckBox* welt2000ReadOl;
  QCheckBox* welt2000EnableUpdates;
  QSpinBox*  welt2000UpdatePeriod;

  QComboBox* pointsSourceBox;
  QLineEdit* pointsOpenAipCountries;
  QSpinBox*  pointsOpenAipHomeRadius;
  QCheckBox* pointsOpenAipEnableUpdates;
  QSpinBox*  pointsOpenAipUpdatePeriod;

  QLineEdit* asOpenAipCountries;
  QCheckBox* asOpenAipEnableUpdates;
  QSpinBox*  asOpenAipUpdatePeriod;

  /**
   * Initial value of home radius.
   */
  int m_wel2000HomeRadiusValue;

  /**
   * Initial value of country filter
   */
  QString m_welt2000CountryFilterValue;

  /**
   * Initial value of outlanding checkbox.
   */
  bool m_welt2000ReadOlValue;

  /**
   * Initial value of Welt2000 update checkbox.
   */
  bool m_welt2000UpdateCheck;

  /**
   * Initial value of Welt2000 update spinbox.
   */
  int m_welt2000UpdateValue;

  /**
   * Initial value of openAIP airfield home radius.
   */
  int m_afOpenAipHomeRadiusValue;

  /**
   * Initial value of openAIP airfield countries
   */
  QString m_afOpenAipCountryValue;

  /**
   * Initial value of openAIP point update checkbox.
   */
  bool m_afOpenAipUpdateCheck;

  /**
   * Initial value of openAIP point update spinbox.
   */
  int m_afOpenAipUpdateValue;

  /**
   * Initial value of openAIP airspace update checkbox.
   */
  bool m_asOpenAipUpdateCheck;

  /**
   * Initial value of openAIP airspace update spinbox.
   */
  int m_asOpenAipUpdateValue;


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

  // Displays for real map scale
  QLabel* lLimitL;
  QLabel* uLimitL;
  QLabel* switchScaleL;
  QLabel* wpLabelL;
  QLabel* reduce1L;
  QLabel* reduce2L;
  QLabel* reduce3L;

  QComboBox* elementSelect;
  QComboBox* projectionSelect;
  LatEdit* firstParallel;
  LatEdit* secondParallel;
  LongEdit* originLongitude;
  QLabel* secondParallelLabel;
  QLabel* originLongitudeLabel;

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

  int m_cylinPar;
  int m_lambertV1;
  int m_lambertV2;
  int m_lambertOrigin;
  int m_currentProjType;

  /** Combo box for distance unit configuration. */
  QComboBox* m_unitDistance;

  /** Combo box for position unit configuration. */
  QComboBox* m_unitPosition;

  /** Combo box for altitude unit configuration. */
  QComboBox* m_unitAltitude;

  /** Combo box for horizontal speed unit configuration. */
  QComboBox* m_unitHSpeed;

  /** Combo box for vertical speed unit configuration. */
  QComboBox* m_unitVSpeed;

  /** Table for airspace files to be loaded. */
  QTableWidget* m_asFileTable;

  /** Table for openAIP point files to be loaded. */
  QTableWidget* m_pointFileTable;

  /** Group box for Welt2000 airfield configuration widget. */
  QGroupBox* m_welt2000Group;

  /** Group box for openAIP airfield configuration widget. */
  QGroupBox* m_openAipGroup;

  /** Widget with a airfield list. */
  AirfieldSelectionList* m_airfieldSelectionList;

  /** Qt translator for the GUI surface. */
  static QTranslator* s_guiTranslator;

  static QTranslator* s_qtTranslator;
};

#endif
