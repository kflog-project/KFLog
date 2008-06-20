/***********************************************************************
 **
 **   filetools.cpp
 **
 **   This file is part of KFlog2
 **
 ************************************************************************
 **
 **   Copyright (c):  2004 by André Somers, 2007 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <qrect.h>

#include "filetools.h"
#include "resource.h"

/** Save a QCString to a stream with only one leading byte instead of 4. */
void ShortSave (QDataStream &s, const QCString &str )
{
  if( str.size() > 255 )
    {
      qWarning( "ShortSave will truncate QCString to 255 characters!" );
    }

  Q_INT8 len;
  len=std::min(str.size(), uint(255));
  s << len;
  s.writeRawBytes( str.data(), len );
}


/** Save a QString to a stream with only one leading byte instead of 4. */
void ShortSave (QDataStream &s, const QString  &str )
{
  //this is basicly a wrapper around the above function, provided for conveneance.
  ShortSave(s, str.utf8());   //just use the QCString version
  return;
}


/** Load a QCString from a stream with only one leading byte instead of 4. */
void ShortLoad (QDataStream &s, QCString &str)
{
  str.detach();
  Q_INT8 len;
  s >> len;			    // read size of string

  if ( len == 0 || s.atEnd() ) {  // end of file reached
    str.resize( 0 );
    return;
  }

  str.resize( len );
  s.readRawBytes( str.data(), len );
  return;
}


/** Load a QString from a stream with only one leading byte instead of 4. */
void ShortLoad (QDataStream &s, QString &str)
{
  //this is basicly a wrapper around the above function, provided for conveneance.
  QCString tmp;
  ShortLoad(s, tmp);
  str=QString::fromUtf8(tmp);
  return;
}


/** Save a QPointArray to a stream in a shorter form. */
void ShortSave (QDataStream & s, const QPointArray& a)
{
  //first, check if we _can_ do this the short way...
  QRect bBox=a.boundingRect();
  if (bBox.width()<=254 && bBox.height()<=254) {
    //qDebug("using  8 bits format");
    //ok, we can do our own saving in 8 bits format to save space
    s << (Q_INT8)1; //set flag to say we used this format :-)
    QPoint topLeft=bBox.topLeft();  //get the coordinates of the top left of the box
    s << topLeft;
    QPointArray ca(a);
    ca.translate(-topLeft.x(),-topLeft.y()); // translate the box so it's top left on (0,0)
    //now, all points in the array fit into 16 bits!
    s << (Q_UINT32)ca.count();
    for (size_t i=0; i<ca.count(); i++) {
      s << (Q_UINT8)ca.point(i).x();
      s << (Q_UINT8)ca.point(i).y();
    }
  } else if (bBox.width()<65500 && bBox.height()<65500) {
    //qDebug("using 16 bits format");
    //ok, we can do our own saving in 16 bits format to save space
    s << (Q_INT8)2; //set flag to say we used this format :-)
    QPoint topLeft=bBox.topLeft();  //get the coordinates of the top left of the box
    s << topLeft;
    QPointArray ca(a);
    ca.translate(-topLeft.x(),-topLeft.y()); // translate the box so it's top left on (0,0)
    //now, all points in the array fit into 32 bits!
    s << (Q_UINT32)ca.count();
    for (int i=0; i<ca.count(); i++) {
      s << (Q_UINT16)ca.point(i).x();
      s << (Q_UINT16)ca.point(i).y();
    }
  } else {
    //qDebug("using long 32 bits format.");
    //too big. We need to use the normal 2x32 bits format :-(
    s << (Q_INT8)4; //we need to set a flag that we use the normal format
    s << a;
  }
}


/** Load a QPointArray in a short form from a stream. */
void ShortLoad (QDataStream & s, QPointArray& a)
{
  Q_INT8 ar_type;
  QPoint topLeft;
  Q_UINT32 pointcount;
  Q_UINT16 px2, py2;
  Q_UINT8 px1, py1;

  s >> ar_type;      //the type is basicly the number of bytes used per coordinate-part
  if (ar_type==4) {
    //the array was stored as a standard QPointArray
    s >> a;
  } else if (ar_type==2) {
    //the array was stored using a top left point + 16 bit points
    s >> topLeft;
    s >> pointcount;
    a.resize(pointcount);
    for (uint i=0; i<pointcount; i++) {
      s >> px2;
      s >> py2;
      a.setPoint(i, px2,py2);
    }
    a.translate(topLeft.x(),topLeft.y()); //translate back to original coordinates
  } else if (ar_type==1) {
    //the array was stored using a top left point + 8 bit points
    s >> topLeft;
    s >> pointcount;
    a.resize(pointcount);

    for (uint i=0; i<pointcount; i++) {
      s >> px1;
      s >> py1;
      a.setPoint(i, px1,py1);
    }
    a.translate(topLeft.x(),topLeft.y()); //translate back to original coordinates
  } else  {
    // WOW!!! we don't know what this is!
    qWarning( "ShortLoad: Unknown polygon format!" );
  }
}
