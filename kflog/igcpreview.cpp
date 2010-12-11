/***********************************************************************
**
**   igcpreview.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <qfileinfo.h>
#include <q3textstream.h>
//Added by qt3to4:
#include <QLabel>

#include "igcpreview.h"
#include "mapcalc.h"

IGCPreview::IGCPreview(QWidget* parent)
  : QLabel(parent), Q3FilePreview()
{
  textLabel = this;
  textLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
}

IGCPreview::~IGCPreview()
{
  textLabel->setText("");
}

void IGCPreview::previewUrl(const Q3Url &url)
{
  /******************************************************************
   * Sample Header:
   *
   * AGCS13X
   * HFDTE300600
   * HFFXA100
   * HFPLTPILOT:FLORIAN EHINGER
   * HFGTYGLIDERTYPE:LS 4
   * HFGIDGLIDERID:D 5903
   * HFDTM100GPSDATUM:WGS84
   * HFRFWFIRMWAREVERSION:3.6
   * HFRHWHARDWAREVERSION:3.4
   * HFFTYFR TYPE:GARRECHT INGENIEURGESELLSCHAFT,VOLKSLOGGER 1.0
   * HFCCLCOMPETITIONCLASS:STANDARDT
   * I023638FXA3941ENL
   * LCONV-VER:4.24
   * C30060000000030060000013
   * C4832867N00856750EPOLTRN
   * C4832867N00856750EPOLTRN
   * C4804900N00845767ESPACHN
   * C4851617N01008267EWASSRL
   * C4856467N00851133EMUEHLC
   * C4832867N00856750EPOLTRN
   * C4832867N00856750EPOLTRN
   *
   *****************************************************************/
  QString fName;
  if(url.isLocalFile())  fName = url.path();
  else  return;

  QFileInfo fInfo(fName);
  if(!fInfo.exists() || !fInfo.size() ||
      (((QString)fInfo.extension()).lower() != "igc"))  return;

  QFile igcFile(fName);
  if(!igcFile.open(QIODevice::ReadOnly))  return;

  QString s, pilotName, gliderType, gliderID, date;
  Q3TextStream stream(&igcFile);

  bool isFirst = true;

  QString previewText, waypointText("<EM>" + tr("no task given") + "</EM>");

  while (!stream.eof())
    {
      s = stream.readLine();

      if(s.mid(0,1) == "H")
        {
          // We have a headerline
          if(s.mid(1,4).upper() == "FPLT")
              pilotName = s.mid(s.find(':')+1,100);
          else if(s.mid(1,4).upper() == "FGTY")
              gliderType = s.mid(s.find(':')+1,100);
          else if(s.mid(1,4).upper() == "FGID")
              gliderID = s.mid(s.find(':')+1,100);
          else if(s.mid(1,4).upper() == "FDTE")
              date = s.mid(5,2) + "." + s.mid(7,2) + "." + s.mid(9,2);
        }
      else if(s.mid(0,1) == "B")
        {
          // We don't show the flight in the preview, only the task ...
          break;
        }
      else if(s.mid(0,1) == "C")
        {
          if( ( ( ( s.mid( 8,1) == "N" ) || ( s.mid( 8,1) == "S" ) ) ||
                ( ( s.mid(17,1) == "W" ) || ( s.mid(17,1) == "E" ) ) ))
            {
              // We have a waypoint
              if(((QString)s.mid(1,7)).toInt() != 0 &&
                      ((QString)s.mid(9,8)).toInt())
                  if(isFirst)
                    {
                      waypointText = s.mid(18,20);
                      isFirst = false;
                    }
                  else
                    {
                      waypointText += " - " + s.mid(18,20);
                    }
            }
        }
    }

    previewText = (QString)"<TABLE BORDER=0><TR><TD>" + date +
        "</TD></TR>" + "<TR><TD>" + pilotName + "</TD></TR>" +
        "<TR><TD>" + gliderType + " / " + gliderID + "</TD></TR>" +
        "</TABLE><HR NOSHADE WIDHT=90% ALIGN=center>" + waypointText;

    textLabel->setText(previewText);
}
