/********************************************************************************
** Form generated from reading UI file 'optimizationwizardbZ4918.ui'
**
** Created: Tue Oct 26 22:24:16 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef OPTIMIZATIONWIZARDBZ4918_H
#define OPTIMIZATIONWIZARDBZ4918_H

#include <Qt3Support/Q3Frame>
#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3Wizard>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "evaluationdialog.h"
#include "flight.h"
//#include "kprogress.h"
//#include "kpushbutton.h"
#include "ktextbrowser.h"
#include "map.h"
#include "mapcontents.h"

QT_BEGIN_NAMESPACE

class Ui_OLC_Optimization
{
public:
    QWidget *page;
    QVBoxLayout *vboxLayout;
    QVBoxLayout *vboxLayout1;
    EvaluationDialog *evaluation;
    QHBoxLayout *hboxLayout;
    Q3GroupBox *groupBox1;
    QWidget *layout3;
    QGridLayout *gridLayout;
    QLabel *lblStartHeight;
    QLabel *lblStopTime;
    QLabel *lblDiffHeight;
    QLabel *textLabel1_3_2_2;
    QLabel *lblStartTime;
    QLabel *textLabel1_4_2;
    QLabel *textLabel1_2_2_2;
    QLabel *lblStopHeight;
    QLabel *lblDiffTime;
    QPushButton *kPushButton2;
    QWidget *page1;
    QVBoxLayout *vboxLayout2;
    QVBoxLayout *vboxLayout3;
    QTextBrowser *kTextBrowser1;
    QHBoxLayout *hboxLayout1;
    QSpacerItem *spacer7;
    Q3Frame *frame3;
    QVBoxLayout *vboxLayout4;
    QVBoxLayout *vboxLayout5;
    QProgressBar *progress;
    QHBoxLayout *hboxLayout2;
    QPushButton *btnStart;
    QPushButton *btnStop;
    QSpacerItem *spacer6;

    void setupUi(Q3Wizard *OLC_Optimization)
    {
        if (OLC_Optimization->objectName().isEmpty())
            OLC_Optimization->setObjectName(QString::fromUtf8("OLC_Optimization"));
        OLC_Optimization->resize(446, 329);
        page = new QWidget(OLC_Optimization);
        page->setObjectName(QString::fromUtf8("page"));
        vboxLayout = new QVBoxLayout(page);
        vboxLayout->setSpacing(6);
        vboxLayout->setContentsMargins(11, 11, 11, 11);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout1 = new QVBoxLayout();
        vboxLayout1->setSpacing(6);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        evaluation = new EvaluationDialog(page);
        evaluation->setObjectName(QString::fromUtf8("evaluation"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(evaluation->sizePolicy().hasHeightForWidth());
        evaluation->setSizePolicy(sizePolicy);
        evaluation->setMinimumSize(QSize(100, 100));

        vboxLayout1->addWidget(evaluation);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        groupBox1 = new Q3GroupBox(page);
        groupBox1->setObjectName(QString::fromUtf8("groupBox1"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox1->sizePolicy().hasHeightForWidth());
        groupBox1->setSizePolicy(sizePolicy1);
        layout3 = new QWidget(groupBox1);
        layout3->setObjectName(QString::fromUtf8("layout3"));
        layout3->setGeometry(QRect(10, 20, 290, 80));
        gridLayout = new QGridLayout(layout3);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        lblStartHeight = new QLabel(layout3);
        lblStartHeight->setObjectName(QString::fromUtf8("lblStartHeight"));
        lblStartHeight->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lblStartHeight->setWordWrap(false);

        gridLayout->addWidget(lblStartHeight, 0, 2, 1, 1);

        lblStopTime = new QLabel(layout3);
        lblStopTime->setObjectName(QString::fromUtf8("lblStopTime"));
        lblStopTime->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lblStopTime->setWordWrap(false);

        gridLayout->addWidget(lblStopTime, 1, 1, 1, 1);

        lblDiffHeight = new QLabel(layout3);
        lblDiffHeight->setObjectName(QString::fromUtf8("lblDiffHeight"));
        lblDiffHeight->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lblDiffHeight->setWordWrap(false);

        gridLayout->addWidget(lblDiffHeight, 2, 2, 1, 1);

        textLabel1_3_2_2 = new QLabel(layout3);
        textLabel1_3_2_2->setObjectName(QString::fromUtf8("textLabel1_3_2_2"));
        textLabel1_3_2_2->setWordWrap(false);

        gridLayout->addWidget(textLabel1_3_2_2, 2, 0, 1, 1);

        lblStartTime = new QLabel(layout3);
        lblStartTime->setObjectName(QString::fromUtf8("lblStartTime"));
        lblStartTime->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lblStartTime->setWordWrap(false);

        gridLayout->addWidget(lblStartTime, 0, 1, 1, 1);

        textLabel1_4_2 = new QLabel(layout3);
        textLabel1_4_2->setObjectName(QString::fromUtf8("textLabel1_4_2"));
        textLabel1_4_2->setWordWrap(false);

        gridLayout->addWidget(textLabel1_4_2, 0, 0, 1, 1);

        textLabel1_2_2_2 = new QLabel(layout3);
        textLabel1_2_2_2->setObjectName(QString::fromUtf8("textLabel1_2_2_2"));
        textLabel1_2_2_2->setWordWrap(false);

        gridLayout->addWidget(textLabel1_2_2_2, 1, 0, 1, 1);

        lblStopHeight = new QLabel(layout3);
        lblStopHeight->setObjectName(QString::fromUtf8("lblStopHeight"));
        lblStopHeight->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lblStopHeight->setWordWrap(false);

        gridLayout->addWidget(lblStopHeight, 1, 2, 1, 1);

        lblDiffTime = new QLabel(layout3);
        lblDiffTime->setObjectName(QString::fromUtf8("lblDiffTime"));
        lblDiffTime->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        lblDiffTime->setWordWrap(false);

        gridLayout->addWidget(lblDiffTime, 2, 1, 1, 1);


        hboxLayout->addWidget(groupBox1);

        kPushButton2 = new QPushButton(page);
        kPushButton2->setObjectName(QString::fromUtf8("kPushButton2"));
        sizePolicy1.setHeightForWidth(kPushButton2->sizePolicy().hasHeightForWidth());
        kPushButton2->setSizePolicy(sizePolicy1);

        hboxLayout->addWidget(kPushButton2);


        vboxLayout1->addLayout(hboxLayout);


        vboxLayout->addLayout(vboxLayout1);

        OLC_Optimization->addPage(page, QString::fromUtf8("Task start and end times"));
        page1 = new QWidget(OLC_Optimization);
        page1->setObjectName(QString::fromUtf8("page1"));
        page1->setGeometry(QRect(0, 0, 100, 30));
        vboxLayout2 = new QVBoxLayout(page1);
        vboxLayout2->setSpacing(6);
        vboxLayout2->setContentsMargins(11, 11, 11, 11);
        vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
        vboxLayout3 = new QVBoxLayout();
        vboxLayout3->setSpacing(6);
        vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
        kTextBrowser1 = new KTextBrowser(page1);
        kTextBrowser1->setObjectName(QString::fromUtf8("kTextBrowser1"));

        vboxLayout3->addWidget(kTextBrowser1);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        spacer7 = new QSpacerItem(21, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacer7);

        frame3 = new Q3Frame(page1);
        frame3->setObjectName(QString::fromUtf8("frame3"));
        sizePolicy1.setHeightForWidth(frame3->sizePolicy().hasHeightForWidth());
        frame3->setSizePolicy(sizePolicy1);
        frame3->setFrameShape(QFrame::StyledPanel);
        frame3->setFrameShadow(QFrame::Plain);
        vboxLayout4 = new QVBoxLayout(frame3);
        vboxLayout4->setSpacing(6);
        vboxLayout4->setContentsMargins(11, 11, 11, 11);
        vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
        vboxLayout5 = new QVBoxLayout();
        vboxLayout5->setSpacing(6);
        vboxLayout5->setObjectName(QString::fromUtf8("vboxLayout5"));
        progress = new QProgressBar(frame3);
        progress->setObjectName(QString::fromUtf8("progress"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(progress->sizePolicy().hasHeightForWidth());
        progress->setSizePolicy(sizePolicy2);

        vboxLayout5->addWidget(progress);

        hboxLayout2 = new QHBoxLayout();
        hboxLayout2->setSpacing(6);
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        btnStart = new QPushButton(frame3);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));
        sizePolicy2.setHeightForWidth(btnStart->sizePolicy().hasHeightForWidth());
        btnStart->setSizePolicy(sizePolicy2);

        hboxLayout2->addWidget(btnStart);

        btnStop = new QPushButton(frame3);
        btnStop->setObjectName(QString::fromUtf8("btnStop"));
        btnStop->setEnabled(false);
        sizePolicy2.setHeightForWidth(btnStop->sizePolicy().hasHeightForWidth());
        btnStop->setSizePolicy(sizePolicy2);

        hboxLayout2->addWidget(btnStop);


        vboxLayout5->addLayout(hboxLayout2);


        vboxLayout4->addLayout(vboxLayout5);


        hboxLayout1->addWidget(frame3);

        spacer6 = new QSpacerItem(51, 21, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacer6);


        vboxLayout3->addLayout(hboxLayout1);


        vboxLayout2->addLayout(vboxLayout3);

        OLC_Optimization->addPage(page1, QString::fromUtf8("Optimization"));

        retranslateUi(OLC_Optimization);
        QObject::connect(kPushButton2, SIGNAL(pressed()), OLC_Optimization, SLOT(slotSetTimes()));
        QObject::connect(btnStop, SIGNAL(pressed()), OLC_Optimization, SLOT(slotStopOptimization()));
        QObject::connect(btnStart, SIGNAL(pressed()), OLC_Optimization, SLOT(slotStartOptimization()));

        QMetaObject::connectSlotsByName(OLC_Optimization);
    } // setupUi

    void retranslateUi(Q3Wizard *OLC_Optimization)
    {
        OLC_Optimization->setWindowTitle(QApplication::translate("OLC_Optimization", "OLC Optimization", 0, QApplication::UnicodeUTF8));
        groupBox1->setTitle(QApplication::translate("OLC_Optimization", "Task Start/End Point", 0, QApplication::UnicodeUTF8));
        lblStartHeight->setText(QApplication::translate("OLC_Optimization", "1999m", 0, QApplication::UnicodeUTF8));
        lblStopTime->setText(QApplication::translate("OLC_Optimization", "11:11:11", 0, QApplication::UnicodeUTF8));
        lblDiffHeight->setText(QApplication::translate("OLC_Optimization", "1999m", 0, QApplication::UnicodeUTF8));
        textLabel1_3_2_2->setText(QApplication::translate("OLC_Optimization", "<p align=\"right\">Difference:</p>", 0, QApplication::UnicodeUTF8));
        lblStartTime->setText(QApplication::translate("OLC_Optimization", "11:11:11", 0, QApplication::UnicodeUTF8));
        textLabel1_4_2->setText(QApplication::translate("OLC_Optimization", "<p align=\"right\">Start of Task:</p>", 0, QApplication::UnicodeUTF8));
        textLabel1_2_2_2->setText(QApplication::translate("OLC_Optimization", "<p align=\"right\">End of Task:</p>", 0, QApplication::UnicodeUTF8));
        lblStopHeight->setText(QApplication::translate("OLC_Optimization", "1999m", 0, QApplication::UnicodeUTF8));
        lblDiffTime->setText(QApplication::translate("OLC_Optimization", "11:11:11", 0, QApplication::UnicodeUTF8));
        kPushButton2->setText(QApplication::translate("OLC_Optimization", "Set Times", 0, QApplication::UnicodeUTF8));
        OLC_Optimization->setTitle(page, QApplication::translate("OLC_Optimization", "Task start and end times", 0, QApplication::UnicodeUTF8));
        btnStart->setText(QApplication::translate("OLC_Optimization", "Start Optimization", 0, QApplication::UnicodeUTF8));
        btnStop->setText(QApplication::translate("OLC_Optimization", "Stop Optimization", 0, QApplication::UnicodeUTF8));
        OLC_Optimization->setTitle(page1, QApplication::translate("OLC_Optimization", "Optimization", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class OLC_Optimization: public Ui_OLC_Optimization {};
} // namespace Ui

QT_END_NAMESPACE

#endif // OPTIMIZATIONWIZARDBZ4918_H
