/***********************************************************************
**
**   printdialog.h
**
**   This file is part of KFLog2.
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

#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <qdialog.h>

#include <kcombobox.h>
#include <kprocess.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qmultilinedit.h>
#include <qprinter.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstring.h>

class KFLogApp;
class Flight;

/**
  * @author Heiner Lamprecht, Florian Ehinger
  * @version $Id$
  */
class PrintDialog : public QDialog
{
  Q_OBJECT

 public:
	/** */
	PrintDialog(KFLogApp* mApp, bool isMap = false);
	/** */
	~PrintDialog();
  /** */
  void openMapPrintDialog();
  /** */
  void openFlightPrintDialog(Flight* currentFlight);
  /** */
  enum Mode {Map, FlightData};
	/** */
	enum Paper {NotSet = -1, A2 = 0, A3 = 1, A4 = 2, A5 = 3, B2 = 4, B3 = 5,
          B4 = 6, Executive = 7, Legal = 8, Letter = 9, Tabloid = 10};
  /** */
  enum PrintMode {Grayscale, Color};
  	
 public slots:
  /** */
  void slotToggleFileOut(bool isFile);
  /** */
  void slotTogglePrintOut(bool isPrinter);
  /** */
  void slotToggleLandscape(bool isLandscape);
  /** */
  void slotTogglePortrait(bool isPortrait);
  /** */
  void slotToggleGrayscale(bool isGray);
  /** */
  void slotToggleColor(bool isColor);
  /** */
  void slotSearchFileOut();
  /** */
  void slotPreview();
  /** */
  void slotPrint();
  /** */
  void slotReadPrintStderr(KProcess* proc, char* str, int str_len);
  /** */
  void slotExitPrinting(KProcess*);

 private:
  /** */
  void __printPositionData(QPainter* painter, struct flightPoint* cPoint,
          int yPos, const char* text, bool printVario = false,
          bool printSpeed = false);
  /** */
  void __printPositionData(QPainter* painter, struct wayPoint* cPoint,
          int yPos);
  /** */
  QString __createMapPrint();
  /** */
  QString __createFlightPrint();
  /** */
  void __createLayout(bool isMap);
  /** */
  void __readOptions();
  /** */
  void __writeOptions();
  /** */
  QSize __getPaperSize(QPrinter* printer);
  /** */
  void __getMargin();
  /** */
  void __drawGrid(const double scale, QPainter* gridP, const QSize pS,
            const QRect mapBorder, const double dX,
            const double dY, const double gridLeft, const double gridRight,
            const double gridTop, const double gridBot);

  int mapCenterLat;
  int mapCenterLon;
  int printMode;
  int pageOrient;
  int colorMode;
  int leftMargin;
  int rightMargin;
  int topMargin;
  int bottomMargin;

  KFLogApp* mainApp;
  QLineEdit* printerQueueE;
  QLineEdit* printerCommandE;
  QLineEdit* fileE;
  QPushButton* fileSearch;
  KProcess previewProc;
  KProcess printProc;
  QString fileName;
  QString printCommand;
  QString previewCommand;
  QRadioButton* portraitB;
  QRadioButton* landscB;
  QRadioButton* printerB;
  QRadioButton* fileB;
  QSpinBox* leftM;
  QSpinBox* rightM;
  QSpinBox* topM;
  QSpinBox* bottomM;
  QComboBox* format;
  QRadioButton* greyB;
  QRadioButton* colorB;
  KComboBox* scaleSelect;
  double* scaleRange;

  Flight* currentFlight;
};

/*
 * Implementierung von inline-Funktionen
 */
inline QSize PrintDialog::__getPaperSize(QPrinter* printer)
{
  /*
   * Maﬂe des Papiers:
   *   DIN-A2:     420 x 594 mm     1190 x 1684
   *   DIN-A3:     297 x 420 mm      842 x 1190
   *   DIN-A4:     210 x 297 mm      595 x  842
   *   DIN-A5:     148 x 210 mm      420 x  595
   *   DIN-B2:     515 x 728 mm     1460 x 2064
   *   DIN-B3:     364 x 515 mm     1032 x 1460
   *   DIN-B4:     257 x 364 mm      729 x 1032
   *   Executive:  191 x 254 mm      542 x  720
   *   Legal:      216 x 356 mm      612 x 1009
   *   Letter:     216 x 279 mm      612 x  791
   *   Tabloid:    279 x 432 mm      791 x 1225
   *
   * Maﬂstab: 72 Punkte / Zoll  <=> 2.835 Punkte / mm
   */
  switch(format->currentItem()) {
    case A2:
      printer->setPageSize(QPrinter::A2);
      return QSize(1190, 1684);
    case A3:
      printer->setPageSize(QPrinter::A3);
      return QSize(842, 1190);
    case A4:
      printer->setPageSize(QPrinter::A4);
      return QSize(595, 842);
    case A5:
      printer->setPageSize(QPrinter::A5);
      return QSize(420, 595);
    case B2:
      printer->setPageSize(QPrinter::B2);
      return QSize(1460, 2064);
    case B3:
      printer->setPageSize(QPrinter::B3);
      return QSize(1032, 1460);
    case B4:
      printer->setPageSize(QPrinter::B4);
      return QSize(729, 1032);
    case Executive:
      printer->setPageSize(QPrinter::Executive);
      return QSize(542, 720);
    case Legal:
      printer->setPageSize(QPrinter::Legal);
      return QSize(612, 1009);
    case Letter:
      printer->setPageSize(QPrinter::Letter);
      return QSize(612, 791);
    case Tabloid:
      printer->setPageSize(QPrinter::Tabloid);
      return QSize(791, 1225);
    default:
      printer->setPageSize(QPrinter::A4);
      return QSize(0,0);
  }
}

inline void PrintDialog::__getMargin()
{
  leftMargin = (int)(leftM->value() / 25.4 * 72.0);
  rightMargin = (int)(rightM->value() / 25.4 * 72.0);
  topMargin = (int)(topM->value() / 25.4 * 72.0);
  bottomMargin = (int)(bottomM->value() / 25.4 * 72.0);
}

#endif
