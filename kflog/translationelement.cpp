/***********************************************************************
**
**   translationelement.cpp
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

#include "translationelement.h"

TranslationElement::TranslationElement(int i, const char *t)
{
  id = i;
  text = t;
}

TranslationElement::~TranslationElement()
{
}
