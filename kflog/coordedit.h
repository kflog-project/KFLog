/***********************************************************************
**
**   coordedit.h
**
**   This file is part of KFLog
**
************************************************************************
**
**   Copyright (c):  2001      by Harald Maier
**                   2009-2011 by Axel Pauli complete redesign done
**
************************************************************************
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: coordedit.h 4502 2010-12-09 22:32:02Z axel $
**
***********************************************************************/

/**
 * \class CoordEdit
 *
 * \author Harald Maier, Axel Pauli
 *
 * \brief Editor widget for WGS84 coordinates.
 *
 * This class is used to edit WGS84 coordinates. It is subclassed by
 * two extensions to handle latitude and longitude coordinates. Three
 * different coordinate formats are supported.
 *
 * -degrees, minutes, seconds
 * -degrees and decimal minutes
 * -decimal degrees
 *
 * \date 2001-2011
 *
 * \version $Id$
*/

#ifndef COOR_DEDIT_H
#define COOR_DEDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QString>

class CoordEdit : public QWidget
{
    Q_OBJECT

  private:

    Q_DISABLE_COPY ( CoordEdit )

  public:

    CoordEdit( QWidget *parent=0 );

    virtual ~CoordEdit();

    /**
     * Sets all edit fields according to the passed coordinate value.
     * The coordinate value is encoded in the KFLog internal format for degrees.
     */
    void setKFLogDegree(const int coord, const bool isLat);

    /** Calculates a degree value in the KFLog internal format for degrees from
     *  the input data fields.
     */
    int KFLogDegree();

    /**
     * Returns true, if initial input values have been modified.
     */
    bool isInputChanged();

    private slots:

    /**
     * Used to check the user input in the editor fields.
     */
    void slot_textEdited( const QString& text );

  protected:

    /**
     * Catch show events in this class to set the widths of some widgets.
     */
    void showEvent(QShowEvent *);

    /** Input fields for coordinate */
    QLineEdit *degreeBox;
    QLineEdit *minuteBox;
    QLineEdit *secondBox;

    /** Sky directions */
    QComboBox *directionBox;

    /** Initial values saved here for change control. */
    int     iniKflogDegree;
    QString iniDegree;
    QString iniMinute;
    QString iniSecond;
    QString iniDirection;
};

/**
 * \class LatEdit
 *
 * \author Harald Maier, Axel Pauli
 *
 * \brief Editor widget for WGS84 latitude coordinates.
 *
 * This class is used to edit WGS84 latitude coordinates. It is derived
 * from \ref CoordEdit. Three different coordinate formats are supported.
 *
 * -degrees, minutes, seconds
 * -degrees and decimal minutes
 * -decimal degrees
 *
 * \date 2001-2010
 */
class LatEdit : public CoordEdit
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( LatEdit )

public:

  LatEdit( QWidget *parent=0, const int base=1 );

  virtual ~LatEdit() {};

  /**
   * overloaded function
   */
  void setKFLogDegree(const int coord);
};

/**
 * \class LongEdit
 *
 * \author Harald Maier, Axel Pauli
 *
 * \brief Editor widget for WGS84 longitude coordinates.
 *
 * This class is used to edit WGS84 longitude coordinates. It is derived
 * from \ref CoordEdit. Three different coordinate formats are supported.
 *
 * -degrees, minutes, seconds
 * -degrees and decimal minutes
 * -decimal degrees
 *
 * \date 2001-2010
 */
class LongEdit : public CoordEdit
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( LongEdit )

public:

  LongEdit( QWidget *parent=0, const int base=1 );

  virtual ~LongEdit() {};

  /**
   * Overloaded function
   */
  void setKFLogDegree(const int coord);
};

#endif
