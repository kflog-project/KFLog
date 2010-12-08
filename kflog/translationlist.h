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

#include <q3ptrlist.h>
#include <q3intdict.h>
//Added by qt3to4:
#include <Q3PtrCollection>

/**
  *@author Harald Maier
  */

typedef Q3PtrList<TranslationElement> TranslationListBase;

class TranslationList : public TranslationListBase {
public: 
	TranslationList();
	~TranslationList();
  virtual int compareItems(Q3PtrCollection::Item e1, Q3PtrCollection::Item e2);
  /** 
   * adds an item to the list
   */
  void append(const TranslationElement *item);
  /** 
   * return index by ID 
   */
  int idxById(int id);
  /** 
   * return a transaltion element item by given id 
   */
  TranslationElement *itemById(int id);
  /** 
   * return a transaltion text by given id, or an empty text if the item does not exist
   */
  QString itemText(int id);

private: // Private attributes
  /** index list elements by id */
  Q3IntDict<TranslationElement> idIndex;
};

#endif
