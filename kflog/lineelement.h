/***********************************************************************
 **
 **   lineelement.h
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef LINEELEMENT_H
#define LINEELEMENT_H

#include "basemapelement.h"

/**
 * Class used for all elements, which consist of a pointarray.
 *
 * @see BaseMapElement#objectType
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class LineElement : public BaseMapElement
{
 public:
  /**
   * Creates a new mapelement.
   *
   * @param  name  The name
   * @param  pA  The pointarray containing the positions
   * @param  isVal  "true", if the element is a "valley".
   * @param  isClosed "true", if the element is a "closed area"
   */
  LineElement(QString name, unsigned int t, QPointArray pA,
              bool isVal = false, bool isClosed = false);
  /**
   * Destructor.
   */
  ~LineElement();
  /**
   * Draws the element into the given painter. Reimplemented from
   * BaseMapElement.
   *
   * @param  targetP  The painter to draw the element into.
   * @param  maskP  The maskpainter for targetP
   */
  virtual void drawMapElement(QPainter* targetP, QPainter* maskP);
  /**
   * Prints the element. Reimplemented from BaseMapElement.
   *
   * @param  printP  The painter to draw the element into.
   *
   * @param  isText  Shows, if the text of some mapelements should
   *                 be printed.
   */
  virtual void printMapElement(QPainter* printPainter, bool isText) const;
  /**
   * @return "true", if the element is a valley.
   *
   * @see #valley
   */
  virtual bool isValley() const;
  /** No descriptions */
  QRect getBoundingBox() { return bBox; }

 protected:
  /**
   * Proofes, if the object is in the drawing-area of the map.
   *
   * @return "true", if the bounding-box of the element intersects
   *         with the drawing-area of the map.
   */
  virtual bool __isVisible() const;
  /**
   * Contains the projected positions of the item.
   */
  QPointArray projPointArray;
  /**
   * The bounding-box of the element.
   */
  QRect bBox;
  /**
   * "true", if the element is a valley. Valleys are used for Isohypsen
   * and f.e. unurban-areas within urban areas or island in a lake.
   */
  bool valley;
  /**
   * "true", if the element is a closed polygone (like cities).
   */
  bool closed;
};

#endif
