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

#include <qptrlist.h>
#include <qintdict.h>

/**
  *@author Harald Maier
  */

typedef QPtrList<TranslationElement> TranslationListBase;

class TranslationList : public TranslationListBase {
public: 
	TranslationList();
	~TranslationList();
  virtual int compareItems(QCollection::Item e1, QCollection::Item e2);
  /** No descriptions */
  void append(const TranslationElement *item);
  /** return index by ID */
  int idxById(int id);
  /** return a transaltion element item by given id */
  TranslationElement *itemById(int id);
private: // Private attributes
  /** index list elements by id */
  QIntDict<TranslationElement> idIndex;
};

#endif
