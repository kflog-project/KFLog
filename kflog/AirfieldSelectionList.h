/***********************************************************************
**
**   AirfieldSelectionList.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2014-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class AirfieldSelectionList
 *
 * \author Axel Pauli
 *
 * \brief A widget with a combo list box and a search function for a single
 * airfield.
 *
 * A widget with a combo list box and a search function for a single airfield.
 * With the help of the search function you can navigate to a certain entry in
 * the combo list box. The current selected entry in the combo list box is
 * emitted as signal, if the set button is clicked.
 *
 * \date 2014-2023
 *
 * \version 1.1
 */

#pragma once

#include <QComboBox>
#include <QGroupBox>
#include <QHash>
#include <QLineEdit>
#include <QString>
#include <QWidget>

#include "singlepoint.h"

class AirfieldSelectionList : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( AirfieldSelectionList )

 public:

  AirfieldSelectionList( QWidget *parent=0 );

  virtual ~AirfieldSelectionList();

  /**
   * Gets the selection box pointer.
   *
   * \return Pointer to selection box.
   */
  QComboBox* getSelectionBox()
  {
    return m_selectionBox;
  }

  /**
   * Called to fill the selection box with content.
   */
  void fillSelectionBox();

  void setGroupBoxTitle( QString title )
  {
    m_groupBox->setTitle( title );
  };

 protected:

  void showEvent( QShowEvent *event );

 signals:

  /**
   * Emitted, if the set button is pressed to broadcast the selected point.
   */
  void takeThisPoint( const SinglePoint* singePoint );

 private slots:

 /**
  * Called, if the clear button is clicked.
  */
  void slotClearSearchEntry();

  /**
   * Called, if the set button is clicked to take over the selected entry from
   * the combo box.
   */
  void slotSetSelectedEntry();

  /**
   * Called if the return key is pressed.
   */
  void slotReturnPressed();

  /**
   * Called if the text in the search box is modified.
   */
  void slotTextEdited( const QString& text );

 private:

  QGroupBox* m_groupBox;
  QLineEdit* m_searchEntry;
  QComboBox* m_selectionBox;

  QHash<QString, SinglePoint*> m_airfieldDict;
};

