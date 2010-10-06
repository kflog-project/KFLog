/***********************************************************************
**
**   main.cpp
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

#include <qapplication.h>
#include <qsettings.h>
#include <qtimer.h>

#include "kflog.h"
#include "kflogconfig.h"
#include "kflogstartlogo.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "mapmatrix.h"

/**
 * Contains all mapelements and takes control over drawing or printing
 * the elements.
 */
MapContents _globalMapContents;

/**
 * Used for transforming the mapitems.
 */
MapMatrix _globalMapMatrix;

/**
 * Contains all configuration-info for drawing and printing the elements.
 */
MapConfig _globalMapConfig;

/**
 * Contains all settings of KFLog.
 */
QSettings _settings;

/**
 * List of commandline-options
 */

//static KCmdLineOptions options[] =
//{
//  { "e", 0, 0 },
//  { "export-png ", I18N_NOOP("export to png graphics file"), "file:out.png" },
//  { "w", 0, 0 },
//  { "width ", I18N_NOOP("width of pixmap"), "640" },
//  { "h", 0, 0 },
//  { "height ", I18N_NOOP("height of pixmap"), "480" },
//  { "c", 0, 0 },
//  { "nocomment", I18N_NOOP("suppress comment"), 0 },
//  { "b", 0, 0 },
//  { "batch", I18N_NOOP("quit after export (batch mode)"), 0 },
//  { "waypoints ", I18N_NOOP("waypoint-catalog to open"), 0 },
//  { "+[File]", I18N_NOOP("igc-file to open"), 0 },
//  { 0, 0, 0 }
//};

/*************************************************************************
 *
 * Okay, now let's start :-)
 *
 */
int main(int argc, char **argv)
{
  /*
   * Using qstrdup here seems to be a workaround. Without, some of the
   * strings are not shown when using i18n. Strange (and against the api),
   * but this version works!
   */
//  KAboutData aboutData( "kflog", "KFLog",
//      VERSION, qstrdup(QObject::tr("KFLog - The K-Flight-Logger")), KAboutData::License_GPL,
//      "(c) 2001-2009, The KFLog-Team",
//      qstrdup(QObject::tr("The maps used for KFLog are derived from the\n"
//          "void-filled CGIAR SRTM data (http://srtm.csi.cgiar.org),\n"
//          "the GTOPO30-data, and the Digital Chart of the World.\n"
//          "If You have no mapfiles installed, please visit\n"
//          "our homepage to download the area You need.")),
//      "http://www.kflog.org", "kflog-devel@kflog.org");
//
//  aboutData.addAuthor("Hendrik Hoeth",
//      qstrdup(QObject::tr("Maintenance, Developer, Mapdata")), "hoeth@linta.de");
//  aboutData.addAuthor("Constantijn Neeteson",
//      qstrdup(QObject::tr("Maintenance, Core-developer")), "kflog@neeteson.net");
//  aboutData.addAuthor("Florian Ehinger",
//      qstrdup(QObject::tr("Maintenance, Core-developer, Mapdata")), "florian@kflog.org");
//  aboutData.addAuthor("Heiner Lamprecht",
//      qstrdup(QObject::tr("Maintenance, Core-developer")), "heiner@kflog.org");
//  aboutData.addAuthor("Andr\303\251 Somers",
//      qstrdup(QObject::tr("Developer (Waypoint-handling, Plugin architecture, ...)")), "andre@kflog.org");
//  aboutData.addAuthor("Christof Bodner",
//      qstrdup(QObject::tr("Developer (OLC Optimization)")), "christof@kflog.org");
//  aboutData.addAuthor("Eggert Ehmke",
//      qstrdup(QObject::tr("Developer")), "eggert@kflog.org");
//  aboutData.addAuthor("Harald Maier",
//      qstrdup(QObject::tr("Developer (Waypoint-Dialog, Task-handling)")), "harry@kflog.org");
//  aboutData.addAuthor("Thomas Nielsen",
//      qstrdup(QObject::tr("Developer (3D-Dialog)")), "thomas@kflog.org");
//  aboutData.addAuthor("Jan Krüger",
//      qstrdup(QObject::tr("Developer (3D-Dialog)")), "jan@kflog.org");
//
//  aboutData.setTranslator(qstrdup(QObject::tr("_: NAME OF TRANSLATORS\nYour names")),
//    qstrdup(QObject::tr("_: EMAIL OF TRANSLATORS\nYour emails")));

  QApplication qApp(argc, argv);
  BaseMapElement::initMapElement(&_globalMapMatrix, &_globalMapConfig);

  QSettings _settings;
  _settings.setPath("KFLog.com", "KFLog");

  KFLog * kflog = new KFLog();
  kflog->show();

  QString argument, fileOpenIGC, fileExportPNG, width = "640", height = "480";
  QString waypointsOptionArg;
  bool batch = false, comment = true, exportPNG = false, fileOpen = false;
  for(unsigned int i = 0; i<qApp.argc(); i++) {
    argument = QString(qApp.argv()[i]);
    if(argument=="--batch" || argument=="-b")
      batch = true;
    else if((argument=="--export-png" || argument=="-e") && i+2<qApp.argc()) {
      exportPNG = true;
      fileExportPNG = QString(qApp.argv()[i++]);
    } else if(argument=="--export-png" || argument=="-e") {
      exportPNG = true;
      fileExportPNG = QString("file:out.png");
    } else if(argument=="--height" || argument=="-h")
      if(i+2<qApp.argc())
        height = QString(qApp.argv()[i++]);
    else if(argument=="--width" || argument=="-w")
      if(i+2<qApp.argc())
        width = QString(qApp.argv()[i++]);
    else if(argument=="--nocomment" || argument=="-c")
      comment = false;
    else if(argument=="--waypoints" && i+1<qApp.argc())
      waypointsOptionArg = qApp.argv()[i++];
    else if(i!=0) {
      fileOpen = true;
      fileOpenIGC = QString(qApp.argv()[i]);
    }
  }

  if(!waypointsOptionArg.isEmpty()) {
    warning("WaypointCatalog specified at startup : %s", (const char*)waypointsOptionArg);
    kflog->slotSetWaypointCatalog(waypointsOptionArg);
  }
  else {
    // read the user configuration
    int useCatalog = _settings.readNumEntry("/KFLog/Waypoints/DefaultWaypointCatalog", KFLogConfig::LastUsed);
    switch (useCatalog) {
    case KFLogConfig::LastUsed:
      // no break;
    case KFLogConfig::Specific:
      waypointsOptionArg = _settings.readEntry("/KFLog/Waypoints/DefaultCatalogName", "");
      kflog->slotSetWaypointCatalog(waypointsOptionArg);
    }
  }

  if(fileOpen) {
      if(exportPNG){
        _settings.writeEntry("/KFLog/GeneralOptions/ShowWaypointWarnings", false);
      }
      kflog->slotOpenFile((const char*)fileOpenIGC);

      if(exportPNG){
        _settings.writeEntry("/KFLog/CommentSettings/ShowComment", comment);
        warning("Writing PNG...");
        QUrl url(fileExportPNG);
        kflog->slotSavePixmap(url, width.toInt(), height.toInt());
      }

      if(batch){
        warning("Exiting.");
        return 0;
      }

  }
  QTimer::singleShot(700, kflog, SLOT(slotStartComplete()));

  return qApp.exec();
}
