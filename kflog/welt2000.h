/************************************************************************
 **
 **   This file is part of KFLog.
 **
 **   Copyright (c):  2006-2014 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id: welt2000.h 4502 2010-12-09 22:32:02Z axel $
 **
 *************************************************************************
 **
 **   welt2000.h
 **
 **   This class is part of KFLog. It provides an interface to be
 **   able to read airfield data from a welt2000.txt Ascii file
 **
 **   http:http://www.segelflug.de/vereine/welt2000/download/WELT2000.TXT
 **
 **   The file source contains data about airports, airfields,
 **   outlandings and turn points in a proprietary ASCII format. Only
 **   the information about airfields, gliding fields and ultralight
 **   fields and outlandings are extracted by this class from the source
 *    and put into the related lists (airport, gliding or outlanding list)
 **   of cumulus. Furthermore an compiled binary version of the extracted
 **   data is created during parsing, usable at a next run of cumulus
 **   for a faster startup. Because welt2000 supports only three kinds
 **   of airfields (airfield, glider field, ul field) but cumulus some
 **   more, a configuration file has been introduced, which allows an
 **   additional mapping to other map elements of cumulus. Furthermore
 **   it provides an country filter mechanism. See later on for more
 **   information about the configuration file possibilities.
 **
 **   To start the load of welt2000 data, the method load has to be
 **   called. It will put all extracted data into the passed lists. The
 **   load method will search for a source file with name WELT2000.TXT
 **   in the default map directories of KFLog.
 **
 **   a) In the configuration area of KFLog the user can define either
 **      a country filter or a radius around his home position. These
 **      items do overwrite the items in the configuration file.
 **
 **   b) A configuration file contains a country filter rule. Only the
 **      listed countries will be considered during parsing.
 **
 **   c) If no country filter rule is defined, then all data are used
 **      inside 500Km radius around the home position. I hope that is an
 **      useful compromise and protects cumulus for memory overflows.
 **
 ***********************************************************************/

#ifndef _welt2000_h
#define _welt2000_h

#include <QMap>
#include <QList>
#include <QString>
#include <QStringList>
#include <QRect>
#include <QPoint>

#include "airfield.h"
#include "basemapelement.h"

/**
 * \class Welt2000
 *
 * \author Axel Pauli
 *
 * \brief Class to read, parse and filter a Welt2000 file.
 *
 * This class can read, parse and filter a Welt2000 file.
 *
 * \date 2006-2014
 *
 * \version $Id$
 */

class Welt2000
{
public:

    /**
     * Constructor
     */
    Welt2000();

    /**
     * Destructor
     */
    virtual ~Welt2000();

    /**
     * Search on default places a welt2000 file and load it. The
     * results are put in the passed lists.
     *
     * @param airfieldList All airports have to be stored in this list
     * @param gliderfieldList All gilder fields have to be stored in this list
     * @param outlandingList All outlanding fields have to be stored in this list
     * @return true (success) or false (error occurred)
     */
    bool load( QList<Airfield>& airfieldList,
               QList<Airfield>& gliderfieldList,
               QList<Airfield>& outlandingList );

    /**
     * Check if a Welt2000 download shall be done. That is allowed only onetimes
     * per 30 days by comparing the last modification date of the current installed
     * Welt2000 file and the current date.
     *
     * \return true, if an update shall be made otherwise false
     */
    bool check4update();

private:

    /**
     * Parses the passed file in welt2000 format and put the appropriate
     * entries in the related lists.
     *
     * @param path Full name with path of welt2000 file
     * @param airfieldList All airports have to be stored in this list
     * @param gliderfieldList All gilder fields have to be stored in this list
     * @param outlandingList All outlanding fields have to be stored in this list
     * @return true (success) or false (error occurred)
     */
    bool parse( QString& path,
                QList<Airfield>& airfieldList,
                QList<Airfield>& gliderfieldList,
                QList<Airfield>& outlandingList );

    /**
     * Get the distance back according to the set unit by the user.
     *
     * @param distance as number
     * @return distance as double in the correct unit
     */
    double getDistanceInKm( const int distance );

private:

    QMap<QString, BaseMapElement::objectType> c_baseTypeMap;

    // Maps used for remapping of airfield types, will be populated
    // with content from configuration file
    QMap<QString, QString> c_icaoMap;  // remapping by icao identifiers
    QMap<QString, QString> c_shortMap; // remapping by short names

    // country filter list from configuration file
    QStringList c_countryList;
    // radius around home position
    double c_homeRadius;
};

#endif
