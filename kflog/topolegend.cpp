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
#include <qtimer.h>
#include <klocale.h>
#include "resource.h"

TopoLegend::TopoLegend(QWidget *parent, const char *name ) : QScrollView(parent,name) {
  extern MapConfig _globalMapConfig;
  
  //These are the levels used, as defined in mapconfig.cpp.
  //For internal reasons, -1 and 10000 are added to the list.
  int levels[] = {
      -1,0,10,25,50,75,100,150,200,250,300,350,400,450,500,600,700,800,900,1000,
      1250,1500,1750,2000,2250,2500,2750,3000,3250,3500,3750,4000,4250,4500,4750,
      5000,5250,5500,5750,6000,6250,6500,6750,7000,7250,7500,7750,8000,8250,8500,8750,10000};
  
  QLabel * lbl;
  QVBox* levelLayout = new QVBox(this->viewport());

  //set fontsize:
  QFont lblFnt=QFont(this->font());
  lblFnt.setPointSizeFloat(9.5);
  this->setFont(lblFnt);
  
  
  this->addChild(levelLayout);        //we are using the QVBox above as our main and single widget
  this->setHScrollBarMode(AlwaysOff); //no horizontal scrollbar 
  this->setResizePolicy(AutoOneFit);  //make sure everything fits nicely
  
  QFontMetrics fm=this->fontMetrics(); //get a QFontMetrics object
  int minwidth=0;
  
  for (int i=50; i>=0; --i) {         //loop over levels defined above in reversed order
    lbl=new QLabel(levelLayout);      //create a new label as a child of the QVBox
    lbl->setAlignment(AlignHCenter);
    if (i==50) {                      //set the text. Normally, this is "xx - yy m", but the first and last get a different one
      lbl->setText(i18n("label used for top level in legend", ">= %1 m").arg(levels[i]));
    } else if(i==0) {
      lbl->setText(i18n("label used for lowest level (blue level) in legend", "< 0 m (water)"));
    } else {
      lbl->setText(i18n("label used for normal level in legend", "%1 - %2 m").arg(levels[i]).arg(levels[i+1]));
    }
    lbl->setMinimumSize(fm.size(0,lbl->text()).width(),0);
    minwidth=std::max(minwidth,fm.size(0,lbl->text()).width());
    
    lbl->setBackgroundMode(FixedColor);                        //set the label to get a fixed bg color
    lbl->setBackgroundColor(_globalMapConfig.getIsoColor(i));  //get the appropriate color from the mapconfig
    labelList.append(lbl);                                     //and add the label to our label list
  }
  this->setMinimumSize(minwidth+30,0);
  currentHighlight=-1;
}

TopoLegend::~TopoLegend(){
}

/** Makes sure the indicated level is visible. */
void TopoLegend::highlightLevel(int level){
  if (level==currentHighlight) return;
  
  //make sure it's visible, but only if this is a valid level!
  if (level >=0 && level<51) {
    int y=labelList.at(50-level)->y();
    this->ensureVisible(10,y);
    labelList.at(50-level)->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  }

  //highlight the selected label
  /*
  for (int i=0;i<51;i++) {
    if (i==(50-level)) {
      labelList.at(i)->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    } else {
      labelList.at(i)->setFrameStyle( QFrame::NoFrame );
    }
  } */

  if (currentHighlight>=0 && currentHighlight<51)
    labelList.at(50-currentHighlight)->setFrameStyle( QFrame::NoFrame );
  currentHighlight=level;

}


