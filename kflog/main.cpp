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

static const char *description =
	I18N_NOOP("KFLog - The K-Flight-Logger");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
	
	
static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP("file to open"), 0 },
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{
	KAboutData aboutData( "kflog", I18N_NOOP("KFLog"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 2001, Heiner Lamprecht", 0, 0, "heiner@kflog.de");
	aboutData.addAuthor("Heiner Lamprecht",0, "heiner@kflog.de");
	aboutData.addAuthor("Florian Ehinger",0, "florian@kflog.de");
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
//        kflog->openDocumentFile(args->arg(0));
		}
		else
		{
//		  kflog->openDocumentFile();
		}
		args->clear();
  }

  return app.exec();
}  
