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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kconfig.h>

#include <qtimer.h>

#include "kflog.h"
#include <kflogstartlogo.h>
#include <mapconfig.h>
#include <mapcontents.h>
#include <mapmatrix.h>

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
 * List of commandline-options
 */

static KCmdLineOptions options[] =
{
  { "e", 0, 0 },
  { "export-png ", I18N_NOOP("export to png graphics file"), "file:out.png" },
  { "w", 0, 0 },
  { "width ", I18N_NOOP("width of pixmap"), "640" },
  { "h", 0, 0 },
  { "height ", I18N_NOOP("height of pixmap"), "480" },
  { "c", 0, 0 },
  { "nocomment", I18N_NOOP("suppress comment"), 0 },
  { "b", 0, 0 },
  { "batch", I18N_NOOP("quit after export (batch mode)"), 0 },
  { "+[File]", I18N_NOOP("igc-file to open"), 0 },
  { 0, 0, 0 }
};

/*************************************************************************
 *
 * Okay, now let's start :-)
 *
 */
int main(int argc, char *argv[])
{
  /*
   * Using qstrdup here seems to be a workaround. Without, some of the
   * strings are not shown when using i18n. Strange (and against the api),
   * but this version works!
   */
  KAboutData aboutData( "kflog", "KFLog",
      VERSION, qstrdup(i18n("KFLog - The K-Flight-Logger")), KAboutData::License_GPL,
      "(c) 2001, 2002, The KFLog-Team",
      qstrdup(i18n("The maps used for KFLog are derived from both\n"
          "the GTOPO30-data and the Digital Chart of the World.\n"
          "If You have no mapfiles installed, please visit\n"
          "our homepage to download the area You need.")),
      "http://www.kflog.org", "bugs@kflog.org");

  aboutData.addAuthor("Heiner Lamprecht",
      qstrdup(i18n("Maintenance, Core-developer")), "heiner@kflog.org");
  aboutData.addAuthor("Florian Ehinger",
      qstrdup(i18n("Maintenance, Core-developer")), "florian@kflog.org");
  aboutData.addAuthor("Harald Maier",
      qstrdup(i18n("Developer (Waypoint-Dialog, Task-handling)")), "harry@kflog.org");
  aboutData.addAuthor("Thomas Nielsen",
      qstrdup(i18n("Developer (3D-Dialog)")), "thomas@kflog.org");

  aboutData.setTranslator(qstrdup(i18n("_: NAME OF TRANSLATORS\nYour names")),
    qstrdup(i18n("_: EMAIL OF TRANSLATORS\nYour emails")));

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  BaseMapElement::initMapElement(&_globalMapMatrix, &_globalMapConfig);

  if (app.isRestored())
    {
      RESTORE(KFLogApp);
    }
  else 
    {
      KFLogApp *kflog = new KFLogApp();
      KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
      KConfig* config = KGlobal::config();

      if (args->count()){
          if (args->isSet("export-png")){
            config->setGroup("General Options");
            config->writeEntry("ShowWaypointWarnings",false,false);
          }

          kflog->slotFileOpenRecent((QString)args->arg(0));

          if (args->isSet("export-png")){
            config->setGroup("CommentSettings");
            config->writeEntry("ShowComment",args->isSet("comment"),false);
            warning("Writing PNG...");
            KURL url((QString)args->getOption("export-png"));
            kflog->slotSavePixmap(url,args->getOption("width").toInt(),args->getOption("height").toInt());
          }

          if (args->isSet("batch")){
            args->clear();
            warning("Exiting.");
            return 0;
          }
      }
      QTimer::singleShot(700, kflog, SLOT(slotStartComplete()));

  		args->clear();
    }

  return app.exec();
}  
