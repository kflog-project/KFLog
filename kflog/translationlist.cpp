/***********************************************************************
**
**   translationlist.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "translationlist.h"

TranslationList::TranslationList()
{
  setAutoDelete(true);
}

TranslationList::~TranslationList(){
}

int TranslationList::compareItems(QCollection::Item e1, QCollection::Item e2)
{
  return (((TranslationElement *)e1)->text.compare(((TranslationElement *)e2)->text));
}
