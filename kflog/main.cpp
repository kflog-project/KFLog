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
  /**
   * Short description of what KFLog is.
   */
  const char *description =
    I18N_NOOP("KFLog - The K-Flight-Logger");

	KAboutData aboutData( "kflog", I18N_NOOP("KFLog"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 2001, The KFLog-Team", 0, "http://www.kflog.org", "bugs@kflog.org");
	aboutData.addAuthor("Heiner Lamprecht", 0, "heiner@kflog.org");
	aboutData.addAuthor("Florian Ehinger", 0, "florian@kflog.org");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app;

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
    		{
    		  _globalMapContents.loadFlight((QString)args->arg(0));
          kflog->slotStartComplete();
    		}
		  else
    		{
          QTimer::singleShot(300, kflog, SLOT(slotStartComplete()));
    		}
  		args->clear();
    }

  return app.exec();
}  
