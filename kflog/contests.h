/***********************************************************************
**
**   gliders.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 Christof Bodner
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef CONTESTS_H
#define CONTESTS_H

struct olc_contest
{
  char name[30];
  char URL[100];
  int index;
};

extern olc_contest contestList[];

#endif

