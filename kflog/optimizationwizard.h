/****************************************************************************
** Form interface generated from reading ui file 'optimizationwizard.ui'
**
** Created: Sam Mär 8 12:18:37 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef OLC_OPTIMIZATION_H
#define OLC_OPTIMIZATION_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwizard.h>

#include "flight.h"
#include "map.h"
#include "mapcontents.h"

class EvaluationDialog;
class QFrame;
class QGroupBox;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QProgressBar;
class QPushButton;
class QTextBrowser;
class QVBoxLayout;
class QWidget;

extern MapContents _globalMapContents;

class OptimizationWizard : public QWizard
{
    Q_OBJECT

public:
    OptimizationWizard( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~OptimizationWizard();

    QWidget* page;
    EvaluationDialog* evaluation;
    QGroupBox* groupBox1;
    QLabel* lblStartHeight;
    QLabel* lblStopTime;
    QLabel* lblDiffHeight;
    QLabel* textLabel1_3_2_2;
    QLabel* lblStartTime;
    QLabel* textLabel1_4_2;
    QLabel* textLabel1_2_2_2;
    QLabel* lblStopHeight;
    QLabel* lblDiffTime;
    QPushButton* kPushButton2;
    QWidget* page_2;
    QTextBrowser* kTextBrowser1;
    QFrame* frame3;
    QProgressBar* progress;
    QPushButton* btnStart;
    QPushButton* btnStop;

    virtual void init();
    virtual double optimizationResult( unsigned int pointList[LEGS+3], double * points );

public slots:
    virtual void slotStartOptimization();
    virtual void slotStopOptimization();
    virtual void slotSetTimes();
    virtual void setMapContents( Map * _map );

protected:
    Flight* flight;
    QPtrList<flightPoint> route;
    Optimization* optimization;

    QVBoxLayout* pageLayout;
    QVBoxLayout* layout14;
    QHBoxLayout* layout12;
    QGridLayout* layout3;
    QVBoxLayout* pageLayout_2;
    QVBoxLayout* layout13;
    QHBoxLayout* layout12_2;
    QVBoxLayout* frame3Layout;
    QVBoxLayout* layout5;
    QHBoxLayout* layout2;

protected slots:
    virtual void languageChange();
private:
    QPixmap image0;

};

#endif // OLC_OPTIMIZATION_H
