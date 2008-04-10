/***********************************************************************
**
**   openairparser.h
**
**   This file is part of KFlog2.
**
************************************************************************
**
**   Copyright (c):  2005 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef _openairparser_h
#define _openairparser_h

#include <qstring.h>
#include <qptrlist.h>
#include <qmap.h>
#include <qrect.h>
#include <qdatetime.h>
#include <qcstring.h>
#include <qbuffer.h>
#include <qdatastream.h>

#include "basemapelement.h"
#include "projectionbase.h"

class Airspace;
class QString;
/**
 * @short Parser for OpenAir SUA files
 *
 * This class implements a parser for OpenAir SUA files, containing
 * descriptions of airspace structures. The read structures are added
 * to the allready present list of structures.
 *
 * Since the build in airspace types do not exactly match the list of
 * airspaces found in "the wild", it is possible to use a special
 * mapping file that modifies and/or extends the default mapping.
 * For a file named airspace.txt, the matching mapping file would be
 * named airspace_mappings.conf in be placed in the same directory.
 *
 * @author André Somers, Axel Pauli
 * @version $Id$
 */
class OpenAirParser
{
public:

    /**
     * Constructor
     */
    OpenAirParser();

    /**
     * Destructor
     */
    virtual ~OpenAirParser();

    /**
     * Searchs on default places for openair files. That can be source
     * files or compiled versions of them.
     *
     * @returns number of successfully loaded files
     * @param list the list of Airspace objects the objects in this
     *   file should be added to.
     */

    uint load( QPtrList<Airspace>& list );

private:  //memberfunctions

    /**
     * Parses the file indicated and adds them to the indicated
     * airspace list.
     *
     * @returns true on success, and false on failure
     * @param path the path for the OpenAir file
     * @param list the list of Airspace objects the objects in this
     *   file should be added to.
     */
    bool parse(const QString& path, QPtrList<Airspace>& list);


    void resetState();
    void parseLine(QString&);
    void newAirspace();
    void newPA();
    void finishAirspace();
    void parseType(QString&);
    void parseAltitude(QString&, BaseMapElement::elevationType&, int&);
    bool parseCoordinate(QString&, int& lat, int& lon);
    bool parseCoordinate(QString&, QPoint&);
    bool parseCoordinatePart(QString&, int& lat, int& lon);
    bool parseVariable(QString);
    bool makeAngleArc(QString);
    bool makeCoordinateArc(QString);
    double bearing( QPoint& p1, QPoint& p2 );
    void addCircle(const double& rLat, const double& rLon);
    void addCircle(const double& radius);
    void addArc(const double& rLat, const double& rLon,
                double angle1, double angle2);
    void initializeStringMapping(const QString& path);
    void initializeBaseMapping();

    /**
     * Read the content of a compiled file and put it into the passed
     * list.
     *
     * @param path Full name with path of OpenAir binary file
     * @param list All airspace objects have to be stored in this list
     * @returns true (success) or false (error occured)
     */
    bool readCompiledFile( QString &path, QPtrList<Airspace>& list );

    /**
     * Get the header data of a compiled file and put it in the class
     * variables.
     *
     * @param path Full name with path of OpenAir binary file
     * @returns true (success) or false (error occured)
     */
    bool setHeaderData( QString &path );

private: //members
    QPtrList<Airspace> _airlist;
    uint _lineNumber;
    uint _objCounter; // counter for allocated objects
    bool _isCurrentAirspace;
    QString asName;
    //QString asTypeLetter;
    BaseMapElement::objectType asType;
    QPointArray asPA;
    int asUpper;
    BaseMapElement::elevationType asUpperType;
    int asLower;
    BaseMapElement::elevationType asLowerType;

    QPoint _center;
    double _awy_width;
    int _direction; //1 for clockwise, -1 for anticlockwise

    QMap<QString, BaseMapElement::objectType> m_baseTypeMap;
    QMap<QString, QString> m_stringTypeMap;

    // compile flag
    bool _doCompile;
    // bounding box
    QRect *_boundingBox;
    // temporary data buffer
    QCString *_bufData;
    QBuffer *_buffer;
    QDataStream *_outbuf;

    // header data members of compiled file
    Q_UINT32 h_magic;
    Q_INT8 h_fileType;
    UINT16 h_fileVersion;
    QDateTime h_creationDateTime;
    QRect h_boundingBox;
    ProjectionBase *h_projection;
    bool h_headerIsValid;
};

#endif
