/***********************************************************************
**
**   mapconfig.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAPCONFIG_H
#define MAPCONFIG_H

#include <qbrush.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qpen.h>
#include <qpixmap.h>


class flightPoint;

/**
 * This class takes care of the configuration-data for displaying
 * and printing map-elements. To avoid problems, there should be only
 * one element per application.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class MapConfig : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new MapConfig object.
     */
    MapConfig();
    /**
     * Destructor
     */
    ~MapConfig();
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
     * @return the pen for drawing a mapelement.
     */
    QPen getDrawPen(unsigned int typeID);
    /**
     * @param  fP  The flightpoint, which is used to determine the color
     *             of the line.
     * @param  overrideSwitch always return the color if true, regardless of
     *                        the zoom factor
     * @return the pen for drawing a line between two flightpoints of a flight.
     */
    QPen getDrawPen(flightPoint* fP, bool overrideSwitch=false);
    /**
     * @param  type  The typeID of the element.
     *
     * @return the pen for printing a mapelement.
     */
    QPen getPrintPen(unsigned int typeID);
    /**
     * @param  type  The typeID of the element.
     *
     * @return the brush for drawing an areaelement.
     */
    QBrush getDrawBrush(unsigned int typeID);
    /**
     * @param  type  The typeID of the element.
     *
     * @return the brush for printing an areaelement.
     */
    QBrush getPrintBrush(unsigned int typeID);
    /**
     * @param  heighIndex  The index of the height of the isohypse.
     *
     * @return the color for a isohypse.
     */
    QColor getIsoColor(unsigned int heightIndex);
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
     * @param  isWinch  Used only for glidersites to determine, if the
     *                  icon should indicate that only winch-launch is
     *                  available.
     *
     * @return the name of the pixmap of the element.
     */
    QString getPixmapName(unsigned int type, bool isWinch = true);
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
     * The possible datatypes, that could be drawn.
     *
     * @see #slotSetFlightDataType
     */
    enum DrawFlightPoint {Vario, Speed, Altitude, Cycling, Solid};

  public slots:
    /**
     * Forces MapConfig to read the configdata.
     */
    void slotReadConfig();
    /**
     * Sets the datatype to be used for drawing flights.
     *
     * @param  type  The datattype
     * @see #DrawFlightPoint
     */
    void slotSetFlightDataType(int type);
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
    QBrush __getBrush(unsigned int typeID, int scaleIndex);
    /**
     * Determines the pen to be used to draw or print a given element-type.
     *
     * @param  typeID  The typeID of the element.
     * @param  scaleIndex  The scaleindex to be used.
     *
     * @return the pen
     */
    QPen __getPen(unsigned int typeID, int sIndex);

    QPtrList<QColor> topographyColorList;

    QPtrList<QPen> airAPenList;
    QPtrList<QBrush> airABrushList;
    QPtrList<QPen> airBPenList;
    QPtrList<QBrush> airBBrushList;
    QPtrList<QPen> airCPenList;
    QPtrList<QBrush> airCBrushList;
    QPtrList<QPen> airDPenList;
    QPtrList<QBrush> airDBrushList;
    QPtrList<QPen> airElPenList;
    QPtrList<QBrush> airElBrushList;
    QPtrList<QPen> airEhPenList;
    QPtrList<QBrush> airEhBrushList;
    QPtrList<QPen> airFPenList;
    QPtrList<QBrush> airFBrushList;
    QPtrList<QPen> ctrCPenList;
    QPtrList<QBrush> ctrCBrushList;
    QPtrList<QPen> ctrDPenList;
    QPtrList<QBrush> ctrDBrushList;
    QPtrList<QPen> lowFPenList;
    QPtrList<QBrush> lowFBrushList;
    QPtrList<QPen> dangerPenList;
    QPtrList<QBrush> dangerBrushList;
    QPtrList<QPen> restrPenList;
    QPtrList<QBrush> restrBrushList;
    QPtrList<QPen> tmzPenList;
    QPtrList<QBrush> tmzBrushList;

    QPtrList<QPen> highwayPenList;
    QPtrList<QPen> roadPenList;
    QPtrList<QPen> trailPenList;
    QPtrList<QPen> railPenList;
    QPtrList<QPen> rail_dPenList;
    QPtrList<QPen> aerialcablePenList;
    QPtrList<QPen> riverPenList;
    QPtrList<QPen> river_tPenList;
    QPtrList<QBrush> river_tBrushList;
    QPtrList<QPen> canalPenList;
    QPtrList<QPen> cityPenList;
    QPtrList<QBrush> cityBrushList;
    QPtrList<QPen> forestPenList;
    QPtrList<QPen> glacierPenList;
    QPtrList<QPen> packicePenList;
    QPtrList<QBrush> forestBrushList;
    QPtrList<QBrush> glacierBrushList;
    QPtrList<QBrush> packiceBrushList;
    QPtrList<QPen> faiAreaLow500PenList;
    QPtrList<QBrush> faiAreaLow500BrushList;
    QPtrList<QPen> faiAreaHigh500PenList;
    QPtrList<QBrush> faiAreaHigh500BrushList;
    /**
     */
    bool* airABorder;
    bool* airBBorder;
    bool* airCBorder;
    bool* airDBorder;
    bool* airElBorder;
    bool* airEhBorder;
    bool* airFBorder;
    bool* ctrCBorder;
    bool* ctrDBorder;
    bool* dangerBorder;
    bool* lowFBorder;
    bool* restrBorder;
    bool* tmzBorder;

    bool* trailBorder;
    bool* roadBorder;
    bool* highwayBorder;
    bool* railBorder;
    bool* rail_dBorder;
    bool* aerialcableBorder;
    bool* riverBorder;
    bool* river_tBorder;
    bool* canalBorder;
    bool* cityBorder;

    bool* forestBorder;
    bool* glacierBorder;
    bool* packiceBorder;
    bool* faiAreaLow500Border;
    bool* faiAreaHigh500Border;
    /**
     * The current scaleindex for displaying the map. The index is set
     * from the mapmatrix-object each time, the map is zoomed.
     *
     * @see #slotSetMatrixValues
     * @see MapMatrix#displayMatrixValues
     */
    int scaleIndex;
    /**
     * The current scaleindex for printing the map. The index is set
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
    /**
     * The datatype to be used for drawing flights.
     *
     * @see #slotSetFlightDataType
     */
    int drawFType;
    int _drawWpLabelScale;
};

#endif
