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
 * erste Parallele
 */
double _v1;

/**
 * zweite Parallele
 */
double _v2;

double _var1;
double _var2;

/**
 * Anzahl der Maßstabsbereiche
 */
#define SCALE_NUM 10

/**
 * Maßstabsgrenzen
 */
double _scale[SCALE_NUM];

/**
 * aktueller Maßstab
 */
double _currentScale;

/**
 * Arrays zum Festlegen, welche Kartenobjekte bei welchem
 * Maßstab angezeigt werden sollen. Der Einfachheit halber wird für
 * jeden Objekt-Typ eine Grenze festgelegt. Damit kann typeID zur Erkennung
 * verwendet werden.
 */
bool _showElements[72];
int _scaleBorder[72];

/**
 * Kurzbeschreibung des Programms
 */
static const char *description =
    I18N_NOOP("KFLog - The K-Flight-Logger");

/**
 *
 */
MapContents _globalMapContents;

/**
 * Used for transforming the mapitems.
 */
MapMatrix _globalMapMatrix;

/**
 * Auflistung der Kommandozeilen-Optionen
 */
static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP("file to open"), 0 },
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
		VERSION, description, KAboutData::License_GPL,
		"(c) 2001, The KFLog-Team", 0, 0, "heiner@kflog.de");
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
