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

struct olc_contest
{
  char name[30];
  char URL[5];
  int index;
};

struct olc_contest contestList[]=
{
  {"International","i",0},
  {"Austria","at",0},
  {"Germany","d",0},
// to be continued...
  {"EOL","EOL",-1}
};

