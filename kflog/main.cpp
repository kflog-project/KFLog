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
	KAboutData aboutData( "kflog", I18N_NOOP("KFLog"),
		VERSION, I18N_NOOP("KFLog - The K-Flight-Logger"), KAboutData::License_GPL,
		"(c) 2001, The KFLog-Team", 0, "http://www.kflog.org",
		"bugs@kflog.org");
	aboutData.addAuthor("Heiner Lamprecht", 0, "heiner@kflog.org");
	aboutData.addAuthor("Florian Ehinger", 0, "florian@kflog.org");
	aboutData.addAuthor("Harald Maier", 0, "harry@kflog.org");
	aboutData.setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"),
	  I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));
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
      kflog->show();

      KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		
		  if (args->count())
          kflog->slotFileOpenRecent((QString)args->arg(0));
		  else
          QTimer::singleShot(300, kflog, SLOT(slotStartComplete()));

  		args->clear();
    }

  return app.exec();
}  
