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

/* liefert den größeren der beiden übergebenen Werte zurück. */
#define MAX(a,b)   ( ( a > b ) ? a : b )

/* liefert den kleineren der beiden übergebenen Werte zurück. */
#define MIN(a,b)   ( ( a < b ) ? a : b )

/* Liefert den Betrag (sollte nicht bei übergebenen Funktionen
   verwendet werden) */
#define ABS(a)     ( ( a > -a ) ? a : -a )

#define ID_STATUS_MSG               1001

#endif // RESOURCE_H
