/***********************************************************************
**
**   translationlist.h
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

#ifndef TRANSLATIONLIST_H
#define TRANSLATIONLIST_H

#include "translationelement.h"
#include <qlist.h>

/**
  *@author Harald Maier
  */

typedef QList<TranslationElement> TranslationListBase;

class TranslationList : public TranslationListBase {
public: 
	TranslationList();
	~TranslationList();
  virtual int compareItems(QCollection::Item e1, QCollection::Item e2);
};

#endif
