/***********************************************************************
**
**   da4record.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Eggert Ehmke
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef DA4RECORD_H
#define DA4RECORD_H


/**used to support filser da4 format
  *@author Eggert Ehmke
  */

#include "basemapelement.h"
#include "airport.h" 

class DA4Record {
private:
  #define DA4RecordSize 31
public: 
	DA4Record();
	~DA4Record();
  unsigned char* buffer() const { return (unsigned char*)_buffer; };
  static unsigned int size () { return DA4RecordSize; };
  void setType (BaseMapElement::objectType type);
  BaseMapElement::objectType type ();
  void setName (const QString& name);
  QString name () const;
  void setLat (float lat);
  void setLon (float lon);
  float lat () const;
  float lon () const;
  void setElev (short int elev);
  short int elev () const;
  void setFreq (float freq);
  float freq () const;
  void setLen (short int len);
  short int len () const;
  void setDir (short int dir);
  short int dir () const;
  void setSurface (Airport::SurfaceType surface);
  Airport::SurfaceType surface () const;
  void setFiller ();

private:
  unsigned char _buffer [DA4RecordSize];
};

#endif
