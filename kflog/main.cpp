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

      if (args->count())
          kflog->slotFileOpenRecent((QString)args->arg(0));

      QTimer::singleShot(700, kflog, SLOT(slotStartComplete()));

  		args->clear();
    }

  return app.exec();
}  
