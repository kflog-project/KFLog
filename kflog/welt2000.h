/************************************************************************
 **
 **   This file is part of KFLog4.
 **
 **   Copyright (c):  2006-2014 by Axel Pauli, axel@kflog.org
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
 **   This class is part of Cumulus. It provides an interface to be
 **   able to read data from a welt2000.txt ascii file, craeted by
 **   Michael Meier and maintained by Howard Mills and Mike Köster
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
 *       useful compromise and protects cumulus for memory overflows.
 **
 **   Now some remarks about the configuration file and its
 **   content. Its name is welt2000.conf. The expected location is the
 **   same where the welt2000.txt file is to find. It can contain
 **   entries for country filtering and also entries for additional
 **   mappings. A comment line starts with a hashmark or a dollar sign
 **   and ends with the newline sign.
 **
 **   A country filter rule in the welt2000.conf file has to be defined
 **   in the following way:
 **
 **   FILTER countries=de,pl,cz,nl
 **
 **   Different countries can be defined within one rule but they have
 **   to be separated by commas. The definition of several filter
 **   lines is also possible. Country abbreviations are coded according
 **   to ISO-3166. See in the header of welt2000.txt file which
 **   countries are inside to find and how is their spelling.
 **
 **   Furthermore the configuration file supports the remapping of single
 **   airfield entries to other map elements of cumulus. The
 **   welt2000.txt file knows only three different types (airfield,
 **   glider field, ul field). Cumulus supports more. There are two
 **   possibilities for a remapping available:
 **
 **   a) A short name (first six left standing characters of definition)
 **      can be remapped according to the following rule:
 **
 **      MAP_SHORT_NAME <short-name>=<new-map-element-of-cumulus>
 **
 **   b) An ICAO identifier can be remapped according to the following rule:
 **
 **      MAP_ICAO <icao-sign>=<new-map-element-of-cumulus>
 **
 **   The supported cumulus map elements are:
 **
 **   [IntAirport|Airport|MilAirport|CivMilAirport|Airfield|ClosedAirfield|
 **    CivHeliport|MilHeliport|AmbHeliport|Gliderfield|UltraLight|HangGlider]
 **
 **   Against the original compiled version of an cumulus airfield
 **   file, the elements of a compiled welt2000 file were further
 **   reduced. All redundant entries with no information were removed
 **   from the output. Therefore the compiled file is incompatible
 **   with the kfc files but slimmer. To avoid confusion with the
 **   existing kfc files, which are derived from kfl, the extension
 **   .txc was choosen to show that this file is derived from a .txt
 **   file.
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
     * per day by comparing the last modification date of the current installed
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
