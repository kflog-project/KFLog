/***********************************************************************
**
**   topolegend.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "topolegend.h"
#include <qvbox.h>
#include "mapconfig.h"
#include <qlayout.h>

TopoLegend::TopoLegend(QWidget *parent, const char *name ) : QScrollView(parent,name) {
  extern MapConfig _globalMapConfig;
  int levels[] = {
      -1,0,10,25,50,75,100,150,200,250,300,350,400,450,500,600,700,800,900,1000,
      1250,1500,1750,2000,2250,2500,2750,3000,3250,3500,3750,4000,4250,4500,4750,
      5000,5250,5500,5750,6000,6250,6500,6750,7000,7250,7500,7750,8000,8250,8500,8750,10000};
  
  QLabel * lbl;
  QVBox* levelLayout = new QVBox(this->viewport());
  //QHBoxLayout * lo=new QHBoxLayout(this->viewport());
  //lo->addWidget(levelLayout);
  
  this->addChild(levelLayout);
  this->setHScrollBarMode(AlwaysOff);  
  this->setResizePolicy(AutoOneFit);
  
  for (int i=50; i>=0; --i) {
    lbl=new QLabel(levelLayout);
    lbl->setAlignment(AlignHCenter);
    if (i==50) {
      lbl->setText(QString(">= %1 m").arg(levels[i]));
    } else if(i==0) {
      lbl->setText(QString("< 0 m"));
    } else {
      lbl->setText(QString("%1 - %2 m").arg(levels[i]).arg(levels[i+1]));
    }
    lbl->setBackgroundMode(FixedColor);
    lbl->setBackgroundColor(_globalMapConfig.getIsoColor(i));
  }
  _itemHeight=lbl->height();
  ensureLevelVisible(0);
}

TopoLegend::~TopoLegend(){
}

/** Makes sure the indicated level is visible. */
void TopoLegend::ensureLevelVisible(unsigned int level){
  int y=0;
  if (level>51) level=51;
  
  y=int((51-level + 0.5) * _itemHeight);
  this->ensureVisible(10,y);
}
