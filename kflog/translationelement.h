/***********************************************************************
**
**   translationelement.h
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

#ifndef TRANSLATIONELEMENT_H
#define TRANSLATIONELEMENT_H

#include <QString>

/**This class may bes used for translation of nternal id's to
text representations
  *@author Harald Maier
  */

class TranslationElement {
public: 
	TranslationElement(int id, const char *text);
	~TranslationElement();
public: // Public attributes
  /** the internal id */
  int id;
  /** the text representation */
  QString text;
};

#endif
