/************************************************************************
 **
 **   Copyright (c):  2006-2007 by Axel Pauli, axel@kflog.org
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id: welt2000.h 2262 2007-03-11 15:43:26Z axel $
 **
 *************************************************************************
 **
 **   welt2000.h
 **
 **   This class is part of Cumulus. It provides an interface to be
 **   able to read data from a welt2000.txt ascii file, provided by
 **   Michael Meier at:
 **
 **   http://www.segelflug.de/segelflieger/michael.meier/download/WELT2000.TXT
 **
 **   The file source contains data about airports, airfields,
 **   outlandings and turn points in a proprietary ascii format. Only
 **   the information about airfields, gliding fields and ultralight
 **   fields are extracted by this class from the source and put into
 **   the related lists (airport list and gliding list) of
 **   cumulus. Furthermore an compiled binary version of the extracted
 **   data is created during parsing, useable at a next run of cumulus
 **   for a faster startup. Because welt2000 supports only three kind
 **   of airfields (airfield, glider field, ul field) but cumulus some
 **   more, a configuration file has been introduced, which allows an
 **   additional mapping to other map elements of cumulus. Furthermore
 **   it provides an country filter mechanism. See later on for more
 **   information about the config file possibilities.
 **
 **   To start the load of welt2000 data, the method load has to be
 **   called. It will put all extracted data into the passed lists. The
 **   load method will search for a source file with name welt2000.txt
 **   resp. a compiled file with the name welt2000.txc in the default
 **   map directories of cumulus. The first found file is always
 **   taken. If a compiled file exists, different checks will be
 **   executed, if it is useable for reloading:
 **
 **   a) Internal header data magic, version, type, ... are controled.
 **      Problems will cause a reparsing of the source file.
 **
 **   b) Creation time is checked against source and config file's
 **      modification time. If one of them is younger a reparsing of the
 **      source file is started.
 **
 **   c) If the compiled file was generated by using 1000Km radius, home
 **      position change is checked. If true, a reparsing of the source
 **      file is started.
 **
 **   d) If map projection type or data have been changed, a reparsing
 *       of the source file is started.
 **
 **   If a source file parsing is necessary, it is first checked, if a
 **   new original source file has been installed. Such a file
 **   contains more as we need, a lot of turn points and out commented
 **   lines. If true then an extracted version will be created from it
 **   and written back under the same file name with a specific header
 **   for cumulus. That will reduce the file size over 50% (3MB to
 **   1.4MB). This compressed file is then the base for all parsing
 **   calls so long no new source is installed. Because all data for
 **   the whole world is contained in one file, we need a mechanism to
 **   extract only a subset from it and to protect cumulus for
 **   overloads (e.g. memory overflow). The are foreseen two
 **   possibilities for filtering:
 **
 **   a) In the configuration area of cumulus the user can define a
 **      country filter and a radius around his home poistion. These
 **      items do overwrite the items in the configuration file.
 **  
 **   b) A configuration file contains a country filter rule. Only the
 **      listed countries will be considered during parsing.
 **
 **   c) If no country filter rule is defined, then all data are used
 **      inside 1000Km radius of the home position. I hope that is an
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
 **   lines is also possible. Country abbrevations are coded according
 **   to ISO-3166. See in the header of welt2000.txt file which
 **   countries are inside to find and how is their spelling.
 **
 **   Furthermore the config file supports the remapping of single
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
 **    CivHeliport|MilHeliport|AmbHeliport|Glidersite|UltraLight|HangGlider]
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

#include <qmap.h>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qrect.h>

#include "basemapelement.h"
#include "mapcontents.h"

/**
 * @short Class to read welt2000 files
 * @author Axel Pauli
 *
 * This class can read and filter Welt2000 files, and store them again
 * in a binary format.
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
     * search on default places a welt2000 file and load it. A source
     * can be the original ascii file or a compiled version of it. The
     * results are put in the passed lists.
     *
     * @param airportList All airports have to be stored in this list
     * @param glidertList All gilder fields have to be stored in this list
     * @returns true (success) or false (error occured)
     */
    bool load( QPtrList<Airport>& airportList, QPtrList<GliderSite>& gliderSiteList );

private:

    /**
     * Parses the passed file in welt 2000 format and put the approriate
     * entries in the related lists.
     * 
     * @param path Full name with path of welt2000 file
     * @param airportList All airports have to be stored in this list
     * @param glidertList All gilder fields have to be stored in this list
     * @param doCompile create a binary file of the parser results,
     *                  if flag is set to true. Default is false
     * @returns true (success) or false (error occured)
     */
    bool parse( QString& path,
                QPtrList<Airport>& airportList,
                QPtrList<GliderSite>& gliderSiteList,
                bool doCompile=false );

    /**
     * The passed file has to be fulfill a welt2000 file format. All
     * not relevant entries, like turn points, will be filtered
     * out. The content of the old file is overwritten with the
     * filtered results to save disk space.
     * 
     * @param path Full name with path of welt2000 file
     * @returns true (success) or false (error occured)
     */
    bool filter( QString &path );

    /**
     * Read all entries from the configuration file related to welt2000.
     * 
     * @param path Full name with path of welt2000 configuration file
     * @returns true (success) or false (error occured)
     */
    bool readConfigEntries( QString &path );

    /**
     * Get the header data of a compiled file and put it in the class
     * variables.
     *
     * @param path Full name with path of welt2000 binary file
     * @returns true (success) or false (error occured)
     */
    bool setHeaderData( QString &path );

    /**
     * Get the distance back according to the set unit by the user.
     *
     * @param distance as number
     * @returns distance as double in the correct unit
     */
    double getDistanceInKm( const int distance );

private:

    QMap<QString, BaseMapElement::objectType> c_baseTypeMap;

    // Maps used for remapping of airfield types, will be populated
    // with content from config file
    QMap<QString, QString> c_icaoMap;  // remapping by icao identifiers
    QMap<QString, QString> c_shortMap; // remapping by short names

    // country filter list from config file
    QStringList c_countryList;
    // radius around home position
    double c_homeRadius;

    // header data members of compiled file
    Q_UINT32 h_magic;
    Q_INT8 h_fileType;
    UINT16 h_fileVersion;
    QDateTime h_creationDateTime;
    QStringList h_countryList;
    double h_homeRadius;
    QPoint h_homeCoord;
    QRect h_boundingBox;
    ProjectionBase *h_projection;
    bool h_headerIsValid;
};

#endif
