/***********************************************************************
**
**   resource.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef RESSOURCE_H
#define RESSOURCE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define DEFAULT_V1                 "54°00,0000N"
#define DEFAULT_V2                 "50°00,0000N"

#define FILEFORMAT    "#KFLog-Mapfile Version: 0.5 (c) 2000 The KFLog-Team"

/* liefert den größeren der beiden übergebenen Werte zurück. */
#define MAX(a,b)   ( ( a > b ) ? a : b )

/* liefert den kleineren der beiden übergebenen Werte zurück. */
#define MIN(a,b)   ( ( a < b ) ? a : b )

/* Liefert den Betrag (sollte nicht bei übergebenen Funktionen
   verwendet werden) */
#define ABS(a)     ( ( a > -a ) ? a : -a )

// Macros for "scaleBorder"
#define ID_SHORELINE                      0
#define ID_BIGHYDRO                       1
#define ID_MIDHYDRO                       2
#define ID_SMALLHYDRO                     3
#define ID_DAM                            4
#define ID_RAILTRACK                      5
#define ID_AERIALTRACK                    6
#define ID_STATION                        7
#define ID_OBSTACLE                       8
#define ID_SPOT                           9
#define ID_PASS                          10
#define ID_GLACIER                       11
#define ID_HIGHWAY                       12
#define ID_HIGHWAY_E                     13
#define ID_MIDROAD                       14
#define ID_SMALLROAD                     15
#define ID_LANDMARK                      16
#define ID_HUGECITY                      17
#define ID_BIGCITY                       18
#define ID_MIDCITY                       19
#define ID_SMALLCITY                     20
#define ID_VILLAGE                       21
#define ID_AIRSPACE                      22
#define ID_RADIO                         23
#define ID_AIRPORT                       24
#define ID_GLIDERSITE                    25
#define ID_OUTLANDING                    26
#define ID_WAYPOINTS                     27
#define ID_ADDSITES                      28

// Macros for "ShowElements"
#define ID_VOR                            0
#define ID_VORDME                         1
#define ID_VORTAC                         2
#define ID_NDB                            3
#define ID_COMPPOINT                      4
#define ID_INTAIRPORT                     5
#define ID_AIRPORTEL                      6
#define ID_MAIRPORT                       7
#define ID_CMAIRPORT                      8
#define ID_AIRFIELD                       9
#define ID_CLOSEDAIR                     10
#define ID_CHELIPORT                     11
#define ID_MHELIPORT                     12
#define ID_AHELIPORT                     13
#define ID_ULTRALIGHT                    14
#define ID_HANGGLIDER                    15
#define ID_PARACHUTE                     16
#define ID_BALLON                        17

///////////////////////////////////////////////////////////////////
#define ID_PRIV_TOOLBAR             1

///////////////////////////////////////////////////////////////////
// File-menu entries
#define ID_HELP_CONTENTS            22222
#define ID_HELP_ABOUT               22223
#define ID_FILE_OPEN                10010
#define ID_FILE_MAPIMPORT           10011
#define ID_FILE_MAPEXPORT           10014
#define ID_FILE_CLOSE               10012
#define ID_FILE_CLOSE_ALL           10013
//#define ID_FILE_MAP                 10020
//#define ID_FILE_CREATE              10030
#define ID_FILE_WAYPOINT            10035
#define ID_ZOOM_IN                  10050
#define ID_ZOOM_OUT                 10060
#define ID_FILE_PRINT               10080
#define ID_FILE_QUIT                10100
#define ID_LAYER_ADDSITES           10101
#define ID_LAYER_AIRPORT            10102
#define ID_LAYER_AIRSPACE           10103
#define ID_LAYER_CITY               10104
#define ID_LAYER_FLIGHT             10105
#define ID_LAYER_GLIDER             10106
#define ID_LAYER_HYDRO              10107
#define ID_LAYER_LAND               10108
#define ID_LAYER_NAV                10109
#define ID_LAYER_OUT                10110
#define ID_LAYER_RAIL               10111
#define ID_LAYER_ROAD               10112
#define ID_LAYER_TOPO               10113
#define ID_LAYER_WAYPOINT           10114
#define ID_REDRAW                   10120
#define ID_CONF_MAP                 10210
#define ID_CONF_KFLOG               10211
#define ID_FILE_SAVE                10221
#define ID_CENTER_FLIGHT            10222
#define ID_CENTER_TASK              10223

///////////////////////////////////////////////////////////////////
// View-menu entries                    
#define ID_VIEW_TOOLBAR             12010
#define ID_VIEW_STATUSBAR           12020
#define ID_VIEW_OPTIONS             12030
///////////////////////////////////////////////////////////////////
// Flight-Menu entries
#define ID_FLIGHT_EVALUATION        13010
#define ID_FLIGHT_OPTIMIZE          13011
#define ID_FLIGHT_PRINT             13012

///////////////////////////////////////////////////////////////////
// Help-menu entries
#define ID_HELP                     1002

#define ID_HELP_WHATS_THIS     			1004

#define ID_STATUS_MSG               1001

#define IDS_DEFAULT                 ""
#define IDS_APP_ABOUT               "KFLog\nVersion " VERSION

#define ID_STATUS_COORD_R           10011
#define ID_STATUS_COORD_H           10012
#define ID_STATUS_PROGRESS          10013
#define IDS_COORD_DEFAULT_R         "                     "
#define IDS_COORD_DEFAULT_H         "                     "
#define ID_STATUS_VARIO             10014
#define ID_STATUS_HEIGHT            10015
#define ID_STATUS_SPEED             10016
#define ID_STATUS_TIME              10017
#define IDS_VARIO                   "                 "
#define IDS_HEIGHT                  "               "
#define IDS_SPEED                   "                   "
#define IDS_TIME                    "               "


//#define MAP_ICON(n) Icon(KApplication::kde_datadir() + "/kflog/map/" + n)
#define PICS_ICON(n) Icon(KApplication::kde_datadir() + "/kflog/pics/" + n)

// Icon-Namen:
// Man sollte den Namen mal besser an die Funktion anpassen ...
#define ICONPATH              KApplication::kde_datadir() + "/kflog/pics/"
//#define ICON_EDIT                   Icon(ICONPATH + "edit.xpm")
//#define ICON_SEND                   Icon(ICONPATH + "send.xpm")
//#define ICON_CHECK                  Icon(ICONPATH + "checkmark.xpm")
//#define ICON_URL                    Icon(ICONPATH + "url_home.xpm")
//#define ICON_NEW                    Icon(ICONPATH + "new.xpm")
#define ICON_PRINT                  Icon(ICONPATH + "print.xpm")
//#define ICON_MAIL                   Icon(ICONPATH + "mailpost.xpm")
#define ICON_TRASH                  Icon(ICONPATH + "trash.xpm")
//#define ICON_DOC                    Icon(ICONPATH + "doc.xpm")
//#define ICON_CLIP                   Icon(ICONPATH + "xclipboard.xpm")
#define ICON_OPEN                   Icon(ICONPATH + "open.xpm")
#define MINI_FOLDER                 Icon("mini/folder.xpm")
#define ICON_LOGGER_OPEN            Icon(ICONPATH + "logger.xpm")
#define ICON_LOGGER_CONNECTED       Icon(ICONPATH + "logger-connected.xpm")
#define ICON_TABLET                 Icon(ICONPATH + "mini-tablet.xpm")
#define ICON_RELOAD                 Icon(ICONPATH + "mini-turn.xpm")
#define ICON_ZOOMIN                 Icon(ICONPATH + "zoomin.xpm")
#define ICON_ZOOMOUT                Icon(ICONPATH + "zoomout.xpm")

#define LAYER_ICON_HYDRO            PICS_ICON("hydrography.xpm")
#define LAYER_ICON_TOPO             PICS_ICON("small/obst_group.xpm")
#define LAYER_ICON_ROAD             PICS_ICON("strassen.xpm")
#define LAYER_ICON_RAIL             PICS_ICON("eisenbahn.xpm")
#define LAYER_ICON_CITY             PICS_ICON("stadt2.xpm")
#define LAYER_ICON_LAND             PICS_ICON("landmarken.xpm")
#define LAYER_ICON_NAV              PICS_ICON("small/vortac.xpm")
#define LAYER_ICON_ADDSITES         PICS_ICON("small/jump.xpm")
#define LAYER_ICON_AIRPORT          PICS_ICON("small/civmilair.xpm")
#define LAYER_ICON_GLIDER           PICS_ICON("small/glider.xpm")
#define LAYER_ICON_AIRSPACE         PICS_ICON("small/airspace.xpm")
#define LAYER_ICON_OUT              PICS_ICON("small/outlanding.xpm")
#define LAYER_ICON_WAYPOINT         PICS_ICON("small/waypoint.xpm")
#define LAYER_ICON_FLIGHT           PICS_ICON("flight.xpm")

#endif // RESOURCE_H
