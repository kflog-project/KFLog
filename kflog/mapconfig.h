/***********************************************************************
**
**   mapconfig.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class MapConfig
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Map configuration class.
 *
 * This class takes care of the configuration-data for displaying
 * and printing map-elements. To avoid problems, there should be only
 * one element per application.
 *
 * \date 2001-2023
 *
 * \version 1.1
 */

#ifndef MAP_CONFIG_H
#define MAP_CONFIG_H

#include <QBrush>
#include <QList>
#include <QObject>
#include <QPen>
#include <QPixmap>

/**
 * These are the levels used.
 * For internal reasons, -1 and 10000 are added to the list.
 */
const int topoLevels[] = {
  -1,0,10,25,50,75,100,150,200,250,300,350,400,450,500,600,700,800,900,1000,
  1250,1500,1750,2000,2250,2500,2750,3000,3250,3500,3750,4000,4250,4500,4750,
  5000,5250,5500,5750,6000,6250,6500,6750,7000,7250,7500,7750,8000,8250,8500,8750,10000};

class FlightPoint;

class MapConfig : public QObject
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( MapConfig )

public:

  /**
   * The possible data types, that could be drawn.
   *
   * @see #slotSetFlightDataType
   */
  enum DrawFlightPointType { Altitude=0, Cycling=1, Speed=2, Vario=3, Airspace=4, Solid=5 };

  /**
   * Creates a new MapConfig object.
   */
  MapConfig( QObject* object = 0 );
  /**
   * Destructor
   */
  virtual ~MapConfig();
  /**
   * @param  type  The typeID of the element.
   *
   * @return "true", if the current scale is smaller than the switch-scale,
   *         so that small icons should be used for displaying.
   */
  bool isBorder(unsigned int type);
  /**
   * @param  type  The typeID of the element.
   *
   * @return "true", if the current scale is smaller than the switch-scale,
   *          so that small icons should be used for printing.
   */
  bool isPrintBorder(unsigned int type);
  /**
   * @param  type  The typeID of the element.
   *
   * @return the pen for drawing a map element.
   */
  QPen& getDrawPen(unsigned int typeID);
  /**
   * @param  fP  The flight point, which is used to determine the color
   *             of the line.
   * @param  va_min       [m/s] minimum vario reading of the whole flight
   * @param  va_max       [m/s] maximum vario reading of the whole flight
   * @param  altitude_max [m]   maximum altitude of the whole flight (minimum altitude is assumed to be 0 m)
   * @param  speed_max    [m/s] maximum velocity during the whole flight
   * @param  dftp         draw type of flight point
   * @return the pen for drawing a line between two flight points of a flight.
   */
  QPen getDrawPen( FlightPoint* fP,
                   float va_min=-10,
                   float va_max=10,
                   int altitude_max = 5000,
                   float speed_max=80,
                   enum MapConfig::DrawFlightPointType dfpt=MapConfig::Altitude );
  /**
   * @param  c  A value between 0.0 and 1.0
   * @return Color from dark red(0.0)->red->yellow->green->cyan->blue->dark blue(1.0)
   */
  QColor getRainbowColor(float c);
  /**
   * @param  type  The typeID of the element.
   *
   * @return the pen for printing a mapelement.
   */
  QPen& getPrintPen(unsigned int typeID);
  /**
   * @param  type  The typeID of the element.
   *
   * @return the brush for drawing an area element.
   */
  QBrush& getDrawBrush(unsigned int typeID);
  /**
   * @param  type  The typeID of the element.
   *
   * @return the brush for printing an area element.
   */
  QBrush& getPrintBrush(unsigned int typeID);
  /**
   * @param  heighIndex  The index of the height of the isohypse.
   *
   * @return the color for a isohypse.
   */
  QColor& getIsoColor(unsigned int heightIndex);
  /**
   * @param  type  The typeID of the element.
   * @param  isWinch  Used only for glidersites to determine, if the
   *                  icon should indicate that only winch-launch is
   *                  available.
   *
   * @returns the icon-pixmap of the element.
   */
  QPixmap getPixmap(unsigned int typeID, bool isWinch = true);
  /**
   * @param  type  The typeID of the element.
   * @param  isWinch  Used only for glidersites to determine, if the
   *                  icon should indicate that only winch-launch is
   *                  available.
   * @param  smallIcon  Used to select the size of the returned pixmap.
   *                  if true, a small pixmap is returned, otherwise the larger
   *                  version is returned.
   * @returns the icon-pixmap of the element.
   */
  QPixmap getPixmap(unsigned int typeID, bool isWinch, bool smallIcon);

  /**
   * @param  type  The typeID of the element.
   * @param  isWinch  Used only for gliderfields to determine, if the
   *                  icon should indicate that only winch-launch is
   *                  available.
   * @param rotatable The rotation flag.
   *
   * @return the name of the pixmap of the element.
   */
  QString getPixmapName( unsigned int type, bool isWinch = false,
                         bool rotatable = false );

  /**
   * @param  typeID   The type identifier of the element.
   * @param  isWinch  Used only for glider sites to determine, if the
   *                  icon should indicate that only winch launch is
   *                  available.
   *
   * @returns the icon-pixmap of the element.
   */
  QPixmap getPixmapRotatable(unsigned int typeID, bool isWinch);

  /**
   * The possible data types, that could be drawn according their direction.
   *
   * \return True if map type is rotatable other otherwise false.
   */
  bool isRotatable( unsigned int typeID ) const;

  /**
   * \return The directory path to the map icons storage location.
   */
  QString getIconPath();

  /**
   * @param pixmapName The pixmap to be loaded.
   * @param smallIcon The kind of icon to be loaded.
   * @return The requested pixmap.
   */
  QPixmap loadPixmap( const QString& pixmapName, bool smallIcon=false );

  /**
   * @param pixmapName Removes the pixmap from the global cache.
   */
  void removePixmap( const QString& pixmapName );

  /**
   * @return pen color for isolines
   */
  QColor getIsoPenColor() {return QColor(80,80,80);}
  /**
   * @param  height height of isoline
   * @return PenStyle of isoline
   */
  Qt::PenStyle getIsoPenStyle(int height);

  /**
   * \return The opacity in the range 0...100% of the requested airspace.
   */
  int getAsOpacity( uint asType );

public slots:

  /**
   * Forces MapConfig to read the configuration data.
   */
  void slotReadConfig();

  /**
   * Sets the scaleindex an the flag for small icons. Called from
   * MapMatrix.
   *
   * @see MapMatrix#scaleAdd
   *
   * @param  index  The scaleindex
   * @param  isSwitch  "true" if the current scale is smaller than the
   *                   switch-scale
   */
  void slotSetMatrixValues(int index, bool isSwitch);
  /**
   * Sets the printScaleIndex.
   */
  void slotSetPrintMatrixValues(int index);
  bool useSmallIcons();
  bool drawWpLabels();

signals:
  /**
   * Emitted each time, the config has changed.
   */
  void configChanged();
  /**
   * Emitted each time the user has selected a new data-type for
   * drawing the flights.
   */
  void flightDataChanged();

private:
  /**
   * Determines the brush to be used to draw or print a given element-type.
   *
   * @param  typeID  The typeID of the element.
   * @param  scaleIndex  The scaleindex to be used.
   *
   * @return the brush
   */
  QBrush& __getBrush( unsigned int typeID, int scaleIndex );
  /**
   * Determines the pen to be used to draw or print a given element-type.
   *
   * @param  typeID  The typeID of the element.
   * @param  scaleIndex  The scale index to be used.
   *
   * @return the pen
   */
  QPen& __getPen( unsigned int typeID, int sIndex );

  /**
   * Reads the draw and print borders from the configuration file.
   */
  void __readBorder( QString group, bool *b );

  void __readPen( QString group,
                  QList<QPen> &penList,
                  bool *b,
                  QColor defaultColor1,
                  QColor defaultColor2,
                  QColor defaultColor3,
                  QColor defaultColor4,
                  QColor defaultColor5,
                  QColor defaultColor6,
                  int defaultPenSize1,
                  int defaultPenSize2,
                  int defaultPenSize3,
                  int defaultPenSize4,
                  int defaultPenSize5,
                  int defaultPenSize6,
                  Qt::PenStyle defaultPenStyle1,
                  Qt::PenStyle defaultPenStyle2,
                  Qt::PenStyle defaultPenStyle3,
                  Qt::PenStyle defaultPenStyle4,
                  Qt::PenStyle defaultPenStyle5,
                  Qt::PenStyle defaultPenStyle6 );

  void __readPenBrush(  QString group,
                        QList<QPen> &penList,
                        bool *b,
                        QList<QBrush> &brushList,
                        QColor defaultColor1,
                        QColor defaultColor2,
                        QColor defaultColor3,
                        QColor defaultColor4,
                        QColor defaultColor5,
                        QColor defaultColor6,
                        int defaultPenSize1,
                        int defaultPenSize2,
                        int defaultPenSize3,
                        int defaultPenSize4,
                        int defaultPenSize5,
                        int defaultPenSize6,
                        Qt::PenStyle defaultPenStyle1,
                        Qt::PenStyle defaultPenStyle2,
                        Qt::PenStyle defaultPenStyle3,
                        Qt::PenStyle defaultPenStyle4,
                        Qt::PenStyle defaultPenStyle5,
                        Qt::PenStyle defaultPenStyle6,
                        QColor defaultBrushColor1,
                        QColor defaultBrushColor2,
                        QColor defaultBrushColor3,
                        QColor defaultBrushColor4,
                        QColor defaultBrushColor5,
                        QColor defaultBrushColor6,
                        Qt::BrushStyle defaultBrushStyle1,
                        Qt::BrushStyle defaultBrushStyle2,
                        Qt::BrushStyle defaultBrushStyle3,
                        Qt::BrushStyle defaultBrushStyle4,
                        Qt::BrushStyle defaultBrushStyle5,
                        Qt::BrushStyle defaultBrushStyle6 );

  void __readTopo( QString entry, QColor color );

  /**
   * Reads the airspace opacity values, used for filling airspace structures.
   */
  void __readAsOpacity(QString group, QList<int> &opacityList);
  void __readAsOpacity( QString group, QList<int> &opacityList,
                           int defaultOpacity1,
                           int defaultOpacity2,
                           int defaultOpacity3,
                           int defaultOpacity4);

  QList<QColor> topographyColorList;

  QList<QPen> airAPenList;
  QList<QBrush> airABrushList;

  QList<QPen> airBPenList;
  QList<QBrush> airBBrushList;

  QList<QPen> airCPenList;
  QList<QBrush> airCBrushList;

  QList<QPen> airDPenList;
  QList<QBrush> airDBrushList;

  QList<QPen> airElPenList;
  QList<QBrush> airElBrushList;

  QList<QPen> airEPenList;
  QList<QBrush> airEBrushList;

  QList<QPen> airFPenList;
  QList<QBrush> airFBrushList;

  QList<QPen> airFirPenList;
  QList<QBrush> airFirBrushList;

  QList<QPen> ctrPenList;
  QList<QBrush> ctrBrushList;

  QList<QPen> suaPenList;
  QList<QBrush> suaBrushList;

  QList<QPen> dangerPenList;
  QList<QBrush> dangerBrushList;

  QList<QPen> restrPenList;
  QList<QBrush> restrBrushList;

  QList<QPen> rmzPenList;
  QList<QBrush> rmzBrushList;

  QList<QPen> tmzPenList;
  QList<QBrush> tmzBrushList;

  QList<QPen> gsPenList;
  QList<QBrush> gsBrushList;

  QList<QPen> wwPenList;
  QList<QBrush> wwBrushList;

  QList<QPen> highwayPenList;
  QList<QPen> roadPenList;
  QList<QPen> trailPenList;
  QList<QPen> railPenList;
  QList<QPen> rail_dPenList;
  QList<QPen> aerialcablePenList;
  QList<QPen> riverPenList;
  QList<QBrush> riverBrushList;
  QList<QPen> river_tPenList;
  QList<QBrush> river_tBrushList;
  QList<QPen> canalPenList;
  QList<QPen> cityPenList;
  QList<QBrush> cityBrushList;
  QList<QPen> forestPenList;
  QList<QPen> glacierPenList;
  QList<QPen> packicePenList;
  QList<QBrush> forestBrushList;
  QList<QBrush> glacierBrushList;
  QList<QBrush> packiceBrushList;
  QList<QPen> faiAreaLow500PenList;
  QList<QBrush> faiAreaLow500BrushList;
  QList<QPen> faiAreaHigh500PenList;
  QList<QBrush> faiAreaHigh500BrushList;

  QList<int> airAOpacityList;
  QList<int> airBOpacityList;
  QList<int> airCOpacityList;
  QList<int> airDOpacityList;
  QList<int> airElOpacityList;
  QList<int> airEOpacityList;
  QList<int> airFOpacityList;
  QList<int> airFirOpacityList;
  QList<int> ctrOpacityList;
  QList<int> suaOpacityList;
  QList<int> dangerOpacityList;
  QList<int> restrOpacityList;
  QList<int> rmzOpacityList;
  QList<int> tmzOpacityList;
  QList<int> gsOpacityList;
  QList<int> wwOpacityList;

  int defaultOpacity[4];

  /**
   * Draw and print boarders. Draw borders are stored at 0...3 and print
   * boarders at 4...5.
   */
  bool airABorder[6];
  bool airBBorder[6];
  bool airCBorder[6];
  bool airDBorder[6];
  bool airElBorder[6];
  bool airEBorder[6];
  bool airFBorder[6];
  bool airFirBorder[6];
  bool ctrBorder[6];
  bool dangerBorder[6];
  bool suaBorder[6];
  bool restrBorder[6];
  bool rmzBorder[6];
  bool tmzBorder[6];
  bool gsBorder[6];
  bool wwBorder[6];

  bool trailBorder[6];
  bool roadBorder[6];
  bool highwayBorder[6];
  bool railBorder[6];
  bool rail_dBorder[6];
  bool aerialcableBorder[6];
  bool riverBorder[6];
  bool river_tBorder[6];
  bool canalBorder[6];
  bool cityBorder[6];
  bool forestBorder[6];
  bool glacierBorder[6];
  bool packiceBorder[6];

  bool faiAreaLow500Border[6];
  bool faiAreaHigh500Border[6];

  /**
   * The current scale index for displaying the map. The index is set
   * from the mapmatrix-object each time, the map is zoomed.
   *
   * @see #slotSetMatrixValues
   * @see MapMatrix#displayMatrixValues
   */
  int scaleIndex;
  /**
   * The current scale index for printing the map. The index is set
   * from the mapmatrix-object after initializing the print-matrix
   *
   * @see #slotSetPrintMatrix
   * @see MapMatrix#printMatrixValues
   */
  int printScaleIndex;
  /**
   * true, if small icons should be drawn. Set from the mapmatrix-object
   * each time, the map is zoomed.
   */
  bool isSwitch;

  int _drawWpLabelScale;
};

#endif
