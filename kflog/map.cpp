/***********************************************************************
**
**   map.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstddirs.h>

#include <qdragobject.h>
#include <qpainter.h>
#include <qwhatsthis.h>
#include <ctype.h>
#include <stdlib.h>

#include <airspace.h>
#include <flight.h>
#include <kflog.h>
#include <map.h>
#include <mapcalc.h>
#include <mapcontents.h>
#include <mapmatrix.h>
#include <singlepoint.h>

// Festlegen der Gr��e der Pixmaps auf Desktop-Gr�sse
#define PIX_WIDTH  QApplication::desktop()->width()
#define PIX_HEIGHT QApplication::desktop()->height()

Map::Map(KFLogApp *m, QFrame* parent, const char* name)
  : QWidget(parent, name),
    mainApp(m), prePos(-50, -50), preCur1(-50, -50), preCur2(-50, -50)
{
  // defining the cursor for the map:
  static const unsigned char cross_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00,
    0x00, 0x80, 0x01, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x98, 0x19, 0x00,
    0x00, 0x8c, 0x31, 0x00, 0x00, 0x86, 0x61, 0x00, 0x00, 0x83, 0xc1, 0x00,
    0x00, 0xc1, 0x83, 0x00, 0x80, 0x60, 0x06, 0x01, 0x80, 0x30, 0x0c, 0x01,
    0xe0, 0x9f, 0xf9, 0x07, 0xe0, 0x9f, 0xf9, 0x07, 0x80, 0x30, 0x0c, 0x01,
    0x80, 0x60, 0x06, 0x01, 0x00, 0xc1, 0x83, 0x00, 0x00, 0x83, 0xc1, 0x00,
    0x00, 0x86, 0x61, 0x00, 0x00, 0x8c, 0x31, 0x00, 0x00, 0x98, 0x19, 0x00,
    0x00, 0xe0, 0x07, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  QBitmap bitCursorMask;
  bitCursorMask.resize(40,40);
  bitCursorMask.fill(Qt::color0);
  pixCursor.resize(40,40);
  pixCursor.fill(white);

  QPainter cursor(&pixCursor);
  cursor.setPen(QPen(QColor(255,0,255), 2));
  cursor.drawLine(0,0,40,40);
  cursor.drawLine(0,40,40,0);
  cursor.setPen(QPen(QColor(255,0,255), 3));
  cursor.drawEllipse(10, 10, 20, 20);
  cursor.end();

  cursor.begin(&bitCursorMask);
  cursor.setPen(QPen(Qt::color1, 2));
  cursor.drawLine(0,0,40,40);
  cursor.drawLine(0,40,40,0);
  cursor.setPen(QPen(Qt::color1, 3));
  cursor.drawEllipse(10, 10, 20, 20);
  cursor.end();

  pixCursor.setMask(bitCursorMask);

  pixCursor1 = QPixmap(KGlobal::dirs()->findResource("appdata",
						     "pics/flag_green.png"));
  pixCursor2 = QPixmap(KGlobal::dirs()->findResource("appdata",
						     "pics/flag_red.png"));

  pixCursorBuffer1.resize(32,32);
  pixCursorBuffer1.fill(white);

  pixCursorBuffer2.resize(32,32);
  pixCursorBuffer2.fill(white);

  pixAero.resize( PIX_WIDTH, PIX_HEIGHT );
  pixAirspace.resize( PIX_WIDTH, PIX_HEIGHT );
  pixFlight.resize( PIX_WIDTH, PIX_HEIGHT );
  pixPlan.resize( PIX_WIDTH, PIX_HEIGHT );
  pixGrid.resize( PIX_WIDTH, PIX_HEIGHT );
  pixUnderMap.resize( PIX_WIDTH, PIX_HEIGHT );
  pixIsoMap.resize( PIX_WIDTH, PIX_HEIGHT );
  bitMapMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitAirspaceMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitPlanMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitFlightMask.resize( PIX_WIDTH, PIX_HEIGHT );

  airspaceRegList = new QList<QRegion>;
  airspaceRegList->setAutoDelete(true);

  const QBitmap cross(32, 32, cross_bits, true);
  const QCursor crossCursor(cross, cross);

  setMouseTracking(true);
  setBackgroundColor(QColor(255,255,255));
  setCursor(crossCursor);
  setAcceptDrops(true);

  // For Planning
  planning = 1;
  pixPlan.fill(white);
  prePlanPos.setX(-999);
  prePlanPos.setY(-999);  

  QWhatsThis::add(this, i18n("<B>The map</B>"
			     "<P>To move or scale the map, please use the buttons in the "
			     "<B>Map-control</B>-area. Or center the map to the current "
			     "cursor-positon by using the right mouse-button.</P>"
			     "<P>To zoom in or out, use the slider or the two buttons on the "
			     "toolbar. You can also zoom with \"&lt;Ctrl&gt;&lt;+&gt;\" (zoom in) "
			     "and \"&lt;Ctrl&gt;&lt;-&gt;\" (zoom out).</P>"
			     "<P>With the menu-item \"Options\" -> \"Configure KFLog\" you can "
			     "configure, which map elements should be displayed at which "
			     "scale.</P>"));

  // create the animation timer
  timerAnimate = new QTimer( this );
  connect( timerAnimate, SIGNAL(timeout()), this,
	   SLOT(slotAnimateFlightTimeout()) );      
}

Map::~Map()
{

}

void Map::mouseMoveEvent(QMouseEvent* event)
{
  extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  int index;

  if(planning == 1 || planning == 3)
    {
      // 3: Task beendet verschieben eines Punktes
      const QPoint current = event->pos();

      BaseMapElement* hitElement;


      QPoint sitePos, preSitePos, nextSitePos, point;
      double dX, dY, delta(32.0);

      if(_globalMapMatrix.isSwitchScale()) delta = 32.0;


      if(taskPoints.size() > 0)
        {
          int dX_old = delta + 10;
          int dY_old = delta + 10;
          /*
           *  Muss f�r alle Punktdaten durchgef�hrt werden
           */
          if(planning == 1)
            {
	      preSitePos = _globalMapMatrix.map(taskPoints.at(taskPoints.size() - 1)->getPosition());
	    }
          else if(planning == 3)
            {
              if(moveWPindex > 0)
		preSitePos = _globalMapMatrix.map(taskPoints.at(moveWPindex - 1)->getPosition());
              if(moveWPindex < taskPoints.size() - 1)  
		nextSitePos = _globalMapMatrix.map(taskPoints.at(moveWPindex + 1)->getPosition());              
            }
                 
          point.setX(current.x());
          point.setY(current.y());
        
          for(unsigned int loop = 0;
	      loop < _globalMapContents.getListLength(
						      MapContents::GliderList); loop++)
            {
              hitElement = _globalMapContents.getElement(
							 MapContents::GliderList, loop);
              sitePos = ((SinglePoint*)hitElement)->getMapPosition();
              
              dX = abs(sitePos.x() - current.x());
              dY = abs(sitePos.y() - current.y());

              if((( dX < delta )  && ( dY < delta )) &&
                 (( dX < dX_old ) && ( dY < dY_old )))
                {
                  // im Snapping Bereich
                  dX_old = dX;
                  dY_old = dY;
                    
                  point.setX(sitePos.x());
                  point.setY(sitePos.y());
                }
	    }
                
          QPainter planP(this);
	  planP.setRasterOp(XorROP);
          planP.setBrush(NoBrush);
          planP.setPen(QPen(QColor(255,0,0), 5));

	  if(prePlanPos.x() >= 0)
	    {
              // alte Linien l�schen
              if(!(planning == 3 && moveWPindex == 0))
                {  
		  planP.drawLine(preSitePos.x(),preSitePos.y(),
                                 prePlanPos.x(),prePlanPos.y());
                }
              if((planning == 3) && moveWPindex != taskPoints.size() - 1)
                {
		  planP.drawLine(nextSitePos.x(),nextSitePos.y(),
				 prePlanPos.x(),prePlanPos.y());
                }
	    }

          if((current.x() > 0 && current.x() < this->width()) &&
             (current.y() > 0 && current.y() < this->height()))
            {
              // Linien zeichnen
              if(!(planning == 3 && moveWPindex == 0))
                {
                  planP.drawLine(preSitePos.x(),preSitePos.y(),
				 point.x(),point.y());
                }
              if((planning == 3) && moveWPindex != taskPoints.size() - 1)
                {                                 
                  planP.drawLine(nextSitePos.x(),nextSitePos.y(),
                                 point.x(),point.y());
                }
                
              prePlanPos = point;
            }
          else
            {
              // au�erhalb der Karte
              prePlanPos.setX(-999);
              prePlanPos.setY(-999);

            }
          emit showTaskText(taskPoints,_globalMapMatrix.mapToWgs(point));
          planP.end();            
        }
    }

    
    
  if (!timerAnimate->isActive()){
    if(prePos.x() >= 0)
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
	     prePos.x() - 20, prePos.y() - 20, 40, 40);

    flightPoint cP;

    if((index = _globalMapContents.searchFlightPoint(event->pos(), cP)) != -1)
      {
        emit showFlightPoint(_globalMapMatrix.mapToWgs(event->pos()), cP);
        prePos = _globalMapMatrix.map(cP.projP);
        preIndex = index;
        bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
      }
    else
      {
        emit showPoint(_globalMapMatrix.mapToWgs(event->pos()));
        prePos.setX(-50);
        prePos.setY(-50);
      }
  }
}

void Map::mousePressEvent(QMouseEvent* event)
{
  // First: delete the cursor, if visible:
  if(prePos.x() >= 0)
    bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
	   prePos.x() - 20, prePos.y() - 20, 40, 40);

  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  const QPoint current = event->pos();


  bool show = false, isAirport = false;

  SinglePoint *hitElement;
  QString text;

  QPoint sitePos;
  double dX, dY, delta(16.0);

  if(_globalMapMatrix.isSwitchScale()) delta = 8.0;


  if(event->button() == MidButton)
    {
      _globalMapMatrix.centerToPoint(event->pos());
      _globalMapMatrix.createMatrix(this->size());
      __redrawMap();
    }
  else if(event->button() == LeftButton)
    {
      //    _start = event->pos();

      if(planning == 1 || planning == 2 || planning == 3)
        {
          ////////// Planen
          int dX_old = delta + 10;
          int dY_old = delta + 10;  
          /*
           *  Muss f�r alle Punktdaten durchgef�hrt werden
           */
          for(unsigned int loop = 0;
	      loop < _globalMapContents.getListLength(
						      MapContents::GliderList); loop++)
            {
              hitElement = (SinglePoint*)_globalMapContents.getElement(
								       MapContents::GliderList, loop);
              sitePos = hitElement->getMapPosition();
    
              dX = abs(sitePos.x() - current.x());
              dY = abs(sitePos.y() - current.y());

              // Abstand entspricht der Icon-Gr��e.
              if( (( dX < delta )  && ( dY < delta )) &&
		  (( dX < dX_old ) && ( dY < dY_old )) )
                {
                  // im Bereich eines WP
                  dX_old = dX;
                  dY_old = dY;
                  
                  if(planning == 1)
                    {
                      if(taskPoints.size() > 0 &&
                         hitElement->getPosition().x() ==
                         taskPoints.at(taskPoints.size() - 1)->getPosition().x())
                        {
                          // gleicher Punkt --> l�schen
			  //                          taskPoints.at(taskPoints.size()) == ;                    
                          pixPlan.fill(white);
                          taskPoints.resize(taskPoints.size() - 1);
			  // __redrawMap();
                        }
                      else
                        {
                          // neuen Punkt an Task Liste anh�ngen
                          taskPoints.resize(taskPoints.size() + 1);
                          taskPoints.at(taskPoints.size() - 1) = hitElement;
                        }
                    
                      // Aufgabe zeichnen
                      if(taskPoints.size() > 1)
                        {
                          __drawPlannedTask();
                          __showLayer();
                        }
                    }
                  else if(planning == 2)
                    {
                      // Punkt wird verschoben
                      for(unsigned int n = 0; n < taskPoints.size(); n++)
                        {
                          if(hitElement->getPosition().x() ==
			     taskPoints.at(n)->getPosition().x())
                            {
                              planning = 3;                            
                              moveWPindex = n;
                              prePlanPos.setX(-999);
			      prePlanPos.setY(-999);                              
                              break;
                            }
                        }
                    }
                  else if(planning == 3)
                    {
                      planning = 2;
		      taskPoints.at(moveWPindex) = hitElement;
                
                      // Aufgabe zeichnen
                      if(taskPoints.size() > 1)
                        {
                          pixPlan.fill(white);                        
                          __drawPlannedTask();
                          __showLayer();
                        }
                    }
                }
            }
        }
    }
  else if(event->button() == RightButton)
    {
      if(planning == 1 || planning == 3)
        {
	  QPoint preSitePos, nextSitePos;
	  // Strecke l�schen
	  moveWPindex = -999;

          QPainter planP(this);
	  planP.setRasterOp(XorROP);
          planP.setBrush(NoBrush);
          planP.setPen(QPen(QColor(255,0,0), 5));
	  
	  if(prePlanPos.x() >= 0)
	    {
              // alte Linien l�schen
              preSitePos = _globalMapMatrix.map(taskPoints.at(taskPoints.size() - 1)->getPosition());
              if(planning == 3)
                {
                  preSitePos = _globalMapMatrix.map(taskPoints.at(moveWPindex - 1)->getPosition());
                  nextSitePos = _globalMapMatrix.map(taskPoints.at(moveWPindex + 1)->getPosition());

		  planP.drawLine(nextSitePos.x(),nextSitePos.y(),
				 prePlanPos.x(),prePlanPos.y());
                }           
	      planP.drawLine(preSitePos.x(),preSitePos.y(),
                             prePlanPos.x(),prePlanPos.y());
	    }
          planP.end();                      
          
          prePlanPos.setX(-999);
          prePlanPos.setY(-999);
          planning = 2;
        }
      else
        {
	  /*
	   * Segelflugpl�tze, soweit vorhanden, kommen als erster Eintrag
	   */
	  for(unsigned int loop = 0;
	      loop < _globalMapContents.getListLength(
						      MapContents::GliderList); loop++)
	    {
	      hitElement = (SinglePoint*)_globalMapContents.getElement(
								       MapContents::GliderList, loop);
	      sitePos = hitElement->getMapPosition();
	      
	      dX = sitePos.x() - current.x();
	      dY = sitePos.y() - current.y();
	      
	      // Abstand entspricht der Icon-Gr��e.
	      if( ( ( dX < delta ) && ( dX > -delta ) ) &&
		  ( ( dY < delta ) && ( dY > -delta ) ) )
		{
		  text = text + ((SinglePoint*)hitElement)->getInfoString();
		  // Text anzeigen
		  QWhatsThis::enterWhatsThisMode();
		  QWhatsThis::leaveWhatsThisMode(text);
		  isAirport = true;
		}
	    }

	  //      text = "";    // Wir wollen _nur_ Flugpl�tze anzeigen!

	  if(_globalMapMatrix.isSwitchScale()) delta = 8.0;

	  for(unsigned int loop = 0;
	      loop < _globalMapContents.getListLength(
						      MapContents::AirportList); loop++)
	    {
	      hitElement = (SinglePoint*)_globalMapContents.getElement(
								       MapContents::AirportList, loop);
	      sitePos = hitElement->getMapPosition();

	      dX = sitePos.x() - current.x();
	      dY = sitePos.y() - current.y();

	      // Abstand entspricht der Icon-Gr��e.
	      if( ( ( dX < delta ) && ( dX > -delta ) ) &&
		  ( ( dY < delta ) && ( dY > -delta ) ) )
		{
		  text = text + hitElement->getInfoString();
		  // Text anzeigen
		  QWhatsThis::enterWhatsThisMode();
		  QWhatsThis::leaveWhatsThisMode(text);
		  isAirport = true;
		}
	    }

	  if(_globalMapContents.getFlightList()->count() > 0)
	    {
	      QList<wayPoint>* wpList =
                _globalMapContents.getFlight()->getWPList();

	      delta = 25;
	      bool isWP = false;
	      QString wpText;
	      wpText = "<B>Waypoint:</B><UL>";

	      for(unsigned int loop = 0; loop < wpList.count(); loop++)
		{
		  sitePos = _globalMapMatrix.map(wpList.at(loop)->projP);

		  dX = sitePos.x() - current.x();
		  dY = sitePos.y() - current.y();

		  // We do not search for the sector ...
		  if( ( ( dX < delta ) && ( dX > -delta ) ) &&
		      ( ( dY < delta ) && ( dY > -delta ) ) )
		    {
		      isWP = true;

		      QString tmpText, timeText;

		      if(wpList.at(loop)->sector1 != 0)
			{
			  timeText = printTime(wpList.at(loop)->sector1);
			  tmpText = i18n("Sector 1");
			}
		      else if(wpList.at(loop)->sector2 != 0)
			{
			  timeText = printTime(wpList.at(loop)->sector2);
			  tmpText = i18n("Sector 2");
			}
		      else if(wpList.at(loop)->sectorFAI != 0)
			{
			  timeText = printTime(wpList.at(loop)->sectorFAI);
			  tmpText = i18n("FAI-Sector");
			}
		      else
			{
			  timeText = "&nbsp;" + i18n("not reached");
			}

		      switch(wpList.at(loop)->type)
			{
			case FlightTask::TakeOff:
			  tmpText = i18n("Take Off");
			  break;
			case FlightTask::Begin:
			  tmpText = i18n("Begin of task");
			  break;
			case FlightTask::End:
			  tmpText = i18n("End of task");
			  break;
			case FlightTask::Landing:
			  tmpText = i18n("Landing");
			  break;
			}

		      wpText = wpText + "<LI><B>" + wpList.at(loop)->name +
			"</B>  " +
			"&nbsp;" + timeText + " / " + tmpText + "<BR>" +
			printPos(wpList.at(loop)->origP.x()) + " / " +
			printPos(wpList.at(loop)->origP.y(), false) + "</LI>";
		    }
		}

	      if(isWP)
		{
		  wpText = wpText + "</UL>";
		  text = text + wpText;
		  // Text anzeigen
		  QWhatsThis::enterWhatsThisMode();
		  QWhatsThis::leaveWhatsThisMode(text);
		  isAirport = true;
		}
	    }

	  if(isAirport)  return;

	  text = text + "<B>" + i18n("Airspace-Structure") + ":</B><UL>";

	  for(unsigned int loop = 0; loop < airspaceRegList->count(); loop++)
	    {
	      if(airspaceRegList->at(loop)->contains(current))
		{
		  hitElement = (SinglePoint*)_globalMapContents.getElement(
									   MapContents::AirspaceList, loop);

		  text = text + "<LI>" + hitElement->getInfoString()
                    + "</LI>";
		  show = true;
		}
	    }
	  text = text + "</UL>";

	  if(show)
	    {
	      //  Text anzeigen
	      QWhatsThis::enterWhatsThisMode();
	      QWhatsThis::leaveWhatsThisMode(text);
	    }
	}
    }
}

void Map::paintEvent(QPaintEvent* event = 0)
{
  if(event == 0)
    bitBlt(this, 0, 0, &pixBuffer);
  else
    bitBlt(this, event->rect().topLeft(), &pixBuffer, event->rect());

  /* Cursor-Position zur�cksetzen! */
  prePos.setX(-50);
  prePos.setY(-50);
}

void Map::__drawGrid()
{
  extern const MapMatrix _globalMapMatrix;
  const QRect mapBorder = _globalMapMatrix.getViewBorder();

  QPainter gridP(&pixGrid);
  gridP.setBrush(NoBrush);

      // die Kanten des Bereichs
      const int lon1 = mapBorder.left() / 600000 - 1;
      const int lon2 = mapBorder.right() / 600000 + 1;
      const int lat1 = mapBorder.top() / 600000 + 1;
      const int lat2 = mapBorder.bottom() / 600000 - 1;

      // Step between two degree-lines (in 1/60 degree)
      int step = 60;
      int gridStep = 1;
      int lineWidth = 1;

      switch(_globalMapMatrix.getScaleRange())
	{
	case 0:
	  step = 10;
	  break;
	case 1:
	  step = 30;
	  break;
	case 2:
	  gridStep = 2;
	  break;
	default:
	  gridStep = 4;
	}

      QPoint cP, cP2;

      // First the latitudes:
      for(int loop = 0; loop < (lat1 - lat2 + 1) ; loop += gridStep)
	{
	  int size = (lon2 - lon1 + 1) * 10;
	  QPointArray pointArray(size);

	  for(int lonloop = 0; lonloop < size; lonloop++)
	    {
	      cP = _globalMapMatrix.wgsToMap( ( lat2 + loop ) * 600000,
					      ( lon1 + ( lonloop * 0.1 ) ) * 600000);
	      pointArray.setPoint(lonloop, cP);
	    }

	  // Draw the small lines between:
	  int number = (int) (60.0 / step);
	  for(int loop2 = 1; loop2 < number; loop2++)
	    {
	      QPointArray pointArraySmall(size);

	      for(int lonloop = 0; lonloop < size; lonloop++)
		{
		  cP = _globalMapMatrix.wgsToMap(
						 ( lat2 + loop + ( loop2 * ( step / 60.0 ) ) ) * 600000,
						 (lon1 + (lonloop * 0.1)) * 600000);
		  pointArraySmall.setPoint(lonloop, cP);
		}
	      if(loop2 == (number / 2.0))
		{
		  gridP.setPen(QPen(QColor(0,0,0), 1, DashLine));
		  gridP.drawPolyline(_globalMapMatrix.map(pointArraySmall));
		}
	      else
		{
		  gridP.setPen(QPen(QColor(0,0,0), 1, DotLine));
		  gridP.drawPolyline(_globalMapMatrix.map(pointArraySmall));
		}
	    }
	  // Draw the main lines
	  gridP.setPen(QPen(QColor(0,0,0), 1));
	  gridP.drawPolyline(_globalMapMatrix.map(pointArray));
	}

      // Now the longitudes:
      for(int loop = lon1; loop <= lon2; loop += gridStep)
	{
	  cP = _globalMapMatrix.wgsToMap(lat1 * 600000, (loop * 600000));
	  cP2 = _globalMapMatrix.wgsToMap(lat2 * 600000, (loop * 600000));

	  // Draw the main longitudes:
	  gridP.setPen(QPen(QColor(0,0,0), 1));
	  gridP.drawLine(_globalMapMatrix.map(cP), _globalMapMatrix.map(cP2));

	  // Draw the small lines between:
	  int number = (int) (60.0 / step);

	  for(int loop2 = 1; loop2 < number; loop2++)
	    {
	      cP = _globalMapMatrix.wgsToMap((lat1 * 600000),
					     ((loop + (loop2 * step / 60.0)) * 600000));

	      cP2 = _globalMapMatrix.wgsToMap((lat2 * 600000),
					      ((loop + (loop2 * step / 60.0)) * 600000));

	      if(loop2 == (number / 2.0))
		gridP.setPen(QPen(QColor(0,0,0), 1, DashLine));
	      else
		gridP.setPen(QPen(QColor(0,0,0), lineWidth, DotLine));
	      gridP.drawLine(_globalMapMatrix.map(cP), _globalMapMatrix.map(cP2));
	    }
	}
      gridP.end();
    }

  void Map::__drawMap()
    {
      QPainter aeroP(&pixAero);
      QPainter airSpaceP(&pixAirspace);
      QPainter uMapP(&pixUnderMap);
      QPainter isoMapP(&pixIsoMap);
      QPainter mapMaskP(&bitMapMask);
      QPainter airspaceMaskP(&bitAirspaceMask);

      airspaceRegList->clear();

      extern MapContents _globalMapContents;

      pixIsoMap.fill(QColor(96,128,248));

  _globalMapContents.drawIsoList(&isoMapP, &mapMaskP);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::TopoList);

  mainApp->slotSetProgress(5);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::CityList);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::HydroList);

  mainApp->slotSetProgress(10);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::RoadList);

  mainApp->slotSetProgress(15);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::HighwayList);

  mainApp->slotSetProgress(20);

  mainApp->slotSetProgress(25);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::RailList);

  mainApp->slotSetProgress(30);

  mainApp->slotSetProgress(35);

  mainApp->slotSetProgress(40);

//  _globalMapContents.drawList(&uMapP, MapContents::VillageList);

  mainApp->slotSetProgress(45);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::LandmarkList);

  mainApp->slotSetProgress(50);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::ObstacleList);

  mainApp->slotSetProgress(55);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::ReportList);

  mainApp->slotSetProgress(60);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::NavList);

  mainApp->slotSetProgress(65);

  Airspace* currentAirS;
  for(unsigned int loop = 0; loop < _globalMapContents.getListLength(
              MapContents::AirspaceList); loop++)
    {
      currentAirS = (Airspace*)_globalMapContents.getElement(
          MapContents::AirspaceList, loop);
        // wir sollten nur die Luftr�ume in der Liste speichern, die
        // tats�chlich gezeichnet werden. Dann geht die Suche schneller.
      airspaceRegList->append(currentAirS->drawRegion(&airSpaceP,
            &airspaceMaskP));
    }

  mainApp->slotSetProgress(70);

  mainApp->slotSetProgress(75);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::AirportList);

  mainApp->slotSetProgress(80);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::AddSitesList);

  mainApp->slotSetProgress(85);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::OutList);

  mainApp->slotSetProgress(90);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::GliderList);

  mainApp->slotSetProgress(95);

  // Closing the painter ...
  aeroP.end();
  airSpaceP.end();
  uMapP.end();
  airspaceMaskP.end();
}

void Map::__drawFlight()
{
  QPainter flightP(&pixFlight);
  QPainter flightMaskP(&bitFlightMask);

  extern MapContents _globalMapContents;

  _globalMapContents.drawList(&flightP, &flightMaskP, MapContents::FlightList);

  flightP.end();
  flightMaskP.end();
}

void Map::__drawPlannedTask()
{
  extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;

  // Strecke zeichnen
  //  pixPlan.fill(white);  
  QPainter planP(&pixPlan);
  QPen drawP(QColor(0,0,0), 5);
  drawP.setJoinStyle(Qt::MiterJoin);
  planP.setBrush(NoBrush);
  planP.setPen(drawP);

  QPointArray points(taskPoints.size());
  QPoint temp;
  for(unsigned int n = 0; n < taskPoints.size(); n++)
    {
      temp = _globalMapMatrix.map(taskPoints.at(n)->getPosition());

      points.setPoint(n,temp.x(),temp.y());
    }
  planP.drawPolyline(points);
  planP.end();
}
void Map::resizeEvent(QResizeEvent* event)
{
  if(!event->size().isEmpty())
    {
      extern MapMatrix _globalMapMatrix;
      _globalMapMatrix.createMatrix(event->size());
      pixBuffer.resize(event->size());
      pixBuffer.fill(white);

      __redrawMap();
    }
  emit changed(event->size());
}

void Map::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept(QTextDrag::canDecode(event));
}

void Map::dropEvent(QDropEvent* event)
{
  QStringList dropList;

  if(QUriDrag::decodeToUnicodeUris(event, dropList))
    {
      for(QStringList::Iterator it = dropList.begin();
              it != dropList.end(); it++)
          mainApp->slotFileOpenRecent((*it).latin1());
    }
}

void Map::__redrawMap()
{
  // Statusbar noch nicht "genial" eingestellt ...
  mainApp->slotSetProgress(0);

  pixPlan.fill(white);

  pixAero.fill(white);
  pixAirspace.fill(white);
  pixGrid.fill(white);
  pixUnderMap.fill(black);
  pixIsoMap.fill(white);
  pixFlight.fill(white);

  bitMapMask.fill(Qt::color0);
  bitFlightMask.fill(Qt::color0);
  bitPlanMask.fill(Qt::color0);
  bitAirspaceMask.fill(Qt::color0);

  QPoint temp1(preCur1);
  QPoint temp2(preCur2);

  preCur1.setX(-50);
  preCur2.setX(-50);

  extern MapContents _globalMapContents;
  _globalMapContents.proofeSection();

  __drawGrid();
  __drawMap();
  __drawFlight();
  __drawPlannedTask();
  // Linie zum aktuellen Punkt l�schen
  prePlanPos.setX(-999);
  prePlanPos.setY(-999);  

  __showLayer();

  mainApp->slotSetProgress(100);

  slotDrawCursor(temp1,temp2);
}

void Map::slotRedrawFlight()
{
  pixFlight.fill(white);
  bitFlightMask.fill(Qt::color0);
  __drawFlight();
  __showLayer();
}

void Map::slotRedrawMap()
{
  extern MapMatrix _globalMapMatrix;
  _globalMapMatrix.createMatrix(this->size());

//  emit changed(this->size());

  __redrawMap();
}

void Map::__showLayer()
{
  pixUnderMap.setMask(bitMapMask);
  pixFlight.setMask(bitFlightMask);

  bitBlt(&pixBuffer, 0, 0, &pixIsoMap);
  bitBlt(&pixBuffer, 0, 0, &pixUnderMap);
  bitBlt(&pixBuffer, 0, 0, &pixAero, 0, 0, -1, -1, NotEraseROP);

  pixAirspace.setMask(bitAirspaceMask);
  bitBlt(&pixBuffer, 0, 0, &pixAirspace);

  bitBlt(&pixBuffer, 0, 0, &pixFlight);
  bitBlt(&pixBuffer, 0, 0, &pixPlan, 0, 0, -1, -1, NotEraseROP);
  bitBlt(&pixBuffer, 0, 0, &pixGrid, 0, 0, -1, -1, NotEraseROP);

  paintEvent();
}

void Map::slotDrawCursor(QPoint p1, QPoint p2)
{
  extern const MapMatrix _globalMapMatrix;

  QPoint pos1(_globalMapMatrix.map(p1)), pos2(_globalMapMatrix.map(p2));

  QPoint prePos1(_globalMapMatrix.map(preCur1)),
         prePos2(_globalMapMatrix.map(preCur2));

  if(preCur1.x() > -50)
    {
      bitBlt(&pixBuffer, prePos1.x() - 32, prePos1.y() - 32, &pixCursorBuffer1);
      bitBlt(this, prePos1.x() - 32, prePos1.y() - 32, &pixCursorBuffer1);
    }
  if(preCur2.x() > -50)
    {
      bitBlt(&pixBuffer, prePos2.x() - 0, prePos2.y() - 32, &pixCursorBuffer2);
      bitBlt(this, prePos2.x() - 0, prePos2.y() - 32, &pixCursorBuffer2);
    }
  //
  // copying the pixmaps can crash the x-server, if the coordinates
  // are out of range.
  //
  //                                                Fixed 2001-12-01
  //
  if(pos1.x() > -50 && pos1.x() < width() + 50 &&
          pos1.y() > -50 && pos1.y() < height() + 50)
      bitBlt(&pixCursorBuffer1, 0, 0, &pixBuffer,
          pos1.x() - 32, pos1.y() - 32, 32, 32);

  if(pos2.x() > -50 && pos2.x() < width() + 50 &&
          pos2.y() > -50 && pos2.y() < height() + 50)
      bitBlt(&pixCursorBuffer2, 0, 0, &pixBuffer,
          pos2.x() - 0, pos2.y() - 32, 32, 32);

  if(pos1.x() > -50 && pos1.x() < width() + 50 &&
          pos1.y() > -50 && pos1.y() < height() + 50)
    {
      bitBlt(this, pos1.x() - 32, pos1.y() - 32, &pixCursor1);
      bitBlt(&pixBuffer, pos1.x() - 32, pos1.y() - 32, &pixCursor1);
    }

  if(pos2.x() > -50 && pos2.x() < width() + 50 &&
          pos2.y() > -50 && pos2.y() < height() + 50)
    {
      bitBlt(this, pos2.x() - 0, pos2.y() - 32, &pixCursor2);
      bitBlt(&pixBuffer, pos2.x() - 0, pos2.y() - 32, &pixCursor2);
    }

  preCur1 = p1;
  preCur2 = p2;
}

void Map::slotCenterToWaypoint(const unsigned int id)
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  if(id >= _globalMapContents.getFlight()->getWPList().count())
    {
      warning("KFLog: Map::slotCenterToWaypoint: wrong Waypoint-ID");
      return;
    }

  _globalMapMatrix.centerToPoint(_globalMapMatrix.map(
      _globalMapContents.getFlight()->getWPList().at(id)->projP));
  _globalMapMatrix.slotSetScale(_globalMapMatrix.getScale(MapMatrix::LowerLimit));

  emit changed(this->size());
}

void Map::slotCenterToFlight()
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  _globalMapMatrix.centerToRect(
          _globalMapContents.getFlight()->getFlightRect());
  _globalMapMatrix.createMatrix(this->size());
  __redrawMap();

  emit changed(this->size());
}

void Map::slotCenterToTask()
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  _globalMapMatrix.centerToRect(
          _globalMapContents.getFlight()->getTaskRect());
  _globalMapMatrix.createMatrix(this->size());
  __redrawMap();

  emit changed(this->size());
}

/** L�scht den Fluglayer */
void Map::slotDeleteFlightLayer()
{
  extern MapContents _globalMapContents;
  _globalMapContents.closeFlight();
  pixFlight.fill(white);
  bitFlightMask.fill(black);
  __showLayer();
}

/**
 * Animation slot.
 * Called to start the animation timer.
 */
void Map::slotAnimateFlightStart()
{
  extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  if (_globalMapContents.getFlight()){
    // erase cursor at the current position
    index = _globalMapContents.searchFlightPoint(prePos, cP);
     bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                 prePos.x() - 20, prePos.y() - 20, 40, 40);
    // go to the start
    if ((index = _globalMapContents.searchGetPrevFlightPoint(1, cP)) != -1){
      emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
      prePos = _globalMapMatrix.map(cP.projP);
      preIndex = index;
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
       timerAnimate->start( 50, FALSE );                 // 50ms multi-shot timer
       nAnimateIndex = 0;
    }
  }
}

/**
 * Animation slot.
 * Called for every timeout of the animation timer. Advances the crosshair one single step.
 */
void Map::slotAnimateFlightTimeout()
{
  extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int length;

  if (_globalMapContents.getFlight()){
    length = _globalMapContents.getFlight()->getRouteLength()-1;
    if ((nAnimateIndex < length) && (nAnimateIndex >= 0)){
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);
      nAnimateIndex = _globalMapContents.searchGetNextFlightPoint(nAnimateIndex, cP);
      emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
      prePos = _globalMapMatrix.map(cP.projP);
      preIndex = nAnimateIndex;
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
    } else {
      // last flightPoint reached, stop animation
      timerAnimate->stop();
      nAnimateIndex = 0;
    }
  }
}

/**
 * Animation slot.
 * Called to start the animation timer.
 */
void Map::slotAnimateFlightStop()
{
  // stop animation on user request.
  if (timerAnimate->isActive()){
    timerAnimate->stop();
    nAnimateIndex = 0;
  }
}

/**
 * Stepping slots.
 */
void Map::slotFlightNext()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  if (prePos.x() >= 0){   // only step if crosshair is shown in map.
     bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                 prePos.x() - 20, prePos.y() - 20, 40, 40);
    // get the next point, preIndex now holds last point
    if ((index = _globalMapContents.searchGetNextFlightPoint(preIndex, cP)) != -1){
      emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
      prePos = _globalMapMatrix.map(cP.projP);
      preIndex = index;
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
    }
  }
}

void Map::slotFlightPrev()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  if (prePos.x() >= 0){   // only step if crosshair is shown in map.
     bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                 prePos.x() - 20, prePos.y() - 20, 40, 40);
    // get the next point, preIndex now holds last point
    if ((index = _globalMapContents.searchGetPrevFlightPoint(preIndex, cP)) != -1){
      emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
      prePos = _globalMapMatrix.map(cP.projP);
      preIndex = index;
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
    }
  }
}
void Map::slotFlightStepNext()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  if (prePos.x() >= 0){   // only step if crosshair is shown in map.
     bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                 prePos.x() - 20, prePos.y() - 20, 40, 40);
    // get the next point, preIndex now holds last point
    if ((index = _globalMapContents.searchStepNextFlightPoint(preIndex, cP, 10)) != -1){
      emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
      prePos = _globalMapMatrix.map(cP.projP);
      preIndex = index;
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
    }
  }
}

void Map::slotFlightStepPrev()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  if (prePos.x() >= 0){   // only step if crosshair is shown in map.
     bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                 prePos.x() - 20, prePos.y() - 20, 40, 40);
    // get the next point, preIndex now holds last point
    if ((index = _globalMapContents.searchStepPrevFlightPoint(preIndex, cP,10)) != -1){
      emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
      prePos = _globalMapMatrix.map(cP.projP);
      preIndex = index;
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
    }
  }
}

void Map::slotFlightHome()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  if (prePos.x() >= 0){   // only step if crosshair is shown in map.
     bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                 prePos.x() - 20, prePos.y() - 20, 40, 40);
    if ((index = _globalMapContents.searchGetNextFlightPoint(0, cP)) != -1){
      emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
      prePos = _globalMapMatrix.map(cP.projP);
      preIndex = index;
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
    }
  }
}

void Map::slotFlightEnd()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  if (prePos.x() >= 0){   // only step if crosshair is shown in map.
     bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                 prePos.x() - 20, prePos.y() - 20, 40, 40);
    if ((index = _globalMapContents.searchGetNextFlightPoint(_globalMapContents.getFlight()->getRouteLength()-1, cP)) != -1){
      emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
      prePos = _globalMapMatrix.map(cP.projP);
      preIndex = index;
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
    }
  }
}

