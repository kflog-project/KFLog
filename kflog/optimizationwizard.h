/****************************************************************************
** Form interface generated from reading ui file './optimizationwizard.ui'
**
** Created: Don Mär 6 14:31:21 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef OLC_OPTIMIZATION_H
#define OLC_OPTIMIZATION_H

#include <qvariant.h>
#include <qwizard.h>
#include "evaluationdialog.h"
#include "flight.h"
#include "map.h"
#include "mapcontents.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class KProgress;
class KPushButton;
class KTextBrowser;
class QGroupBox;
class QLabel;
class QWidget;
extern MapContents _globalMapContents;

class OptimizationWizard : public QWizard
{
    Q_OBJECT

public:
    OptimizationWizard( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~OptimizationWizard();

    QWidget* page;
    KPushButton* kPushButton2;
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
    QWidget* page_2;
    KProgress* progress;
    KPushButton* btnStart;
    KPushButton* btnStop;
    QWidget* page_3;
    KTextBrowser* kTextBrowser1;

    virtual void init();
    virtual double optimizationResult( unsigned int pointList[LEGS+1], double * points );

public slots:
    virtual void slotStartOptimization();
    virtual void slotStopOptimization();
    virtual void slotSetTimes();
    virtual void setMapContents( Map * _map );

protected:
    Flight* flight;
    EvaluationDialog* evaluation;
    QPtrList<flightPoint> route;
    Optimization* optimization;

    QGridLayout* layout3;
    QVBoxLayout* layout5;
    QHBoxLayout* layout2;

protected slots:
    virtual void languageChange();
};

#endif // OLC_OPTIMIZATION_H
