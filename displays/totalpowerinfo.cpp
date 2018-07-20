/*
 ISC License

 Copyright (c) 2016-2017, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */
#include "totalpowerinfo.h"
#include "ui_totalpowerinfo.h"
#include "piechart.h"
#include "math.h"


TotalPowerInfo::TotalPowerInfo(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::TotalPowerInfo)
    , m_minimumColumnWidth(150)
    , m_graph (new PieChart(this))
    , m_label (new QLabel(this))
    , m_value (new QLabel(this))
{
    ui->setupUi(this);
    
    addAction(ui->actionVerbose_DisplayNew);
    ui->actionVerbose_DisplayNew->setChecked(true);
    addAction(ui->actionPieChart);
    ui->actionPieChart->setChecked(true);
    
    powerList.append(m_powerRW);
    powerList.append(m_powerTR);
    powerList.append(m_powerST);
    powerList.append(m_power4);
    powerList.append(m_power5);
    powerList.append(m_power6);
    powerList.append(m_power7);
    powerList.append(m_power8);
    powerList.append(m_power9);
    
    nameList.append("Reaction Wheels");
    nameList.append("Torque Rods");
    nameList.append("Star Tracker");
    nameList.append("Component 4");
    nameList.append("Component 5");
    nameList.append("Component 6");
    nameList.append("Component 7");
    nameList.append("Component 8");
    nameList.append("Component 9");
    
    
    ui->rwLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->rwLabel, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenuRW(const QPoint &)));
    ui->rwPower->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->rwPower, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenuRW(const QPoint &)));
    
    listRW.append(ui->rwSubLabel_1);
    listRW.append(ui->rwSubLabel_2);
    listRW.append(ui->rwSubLabel_3);
    listRW.append(ui->rwSubPower_1);
    listRW.append(ui->rwSubPower_2);
    listRW.append(ui->rwSubPower_3);
    
    QFont f("Arial", 2);
    
    for (int i=0; i<listRW.size(); i++)
    {
        listRW[i]->show();
    }
    ui->rwSubLabel_1->setText("<font color='grey' size=2>POWER MIN</font>");
    ui->rwSubLabel_1->setToolTip("To mantain 0 speed");
    ui->rwSubLabel_1->setToolTipDuration(3000);
    ui->rwSubLabel_2->setText("<font color='grey' size=2>POWER DUE TO SPIN</font>");
    ui->rwSubLabel_3->setText("<font color='grey' size=2> POWER DUE TO TORQUE</font>");
    
    ui->pieChartLayout->addWidget(m_graph);
    QHBoxLayout *hBox = new QHBoxLayout;
    m_label->setAlignment(Qt::AlignLeft);
    m_value->setAlignment(Qt::AlignRight);
    hBox->addWidget(m_label);
    hBox->addWidget(m_value);
    QWidget *widgetContainer = new QWidget;
    widgetContainer->setMinimumWidth(m_minimumColumnWidth);
    widgetContainer->setLayout(hBox);
    ui->totPowerLayout->addWidget(widgetContainer);
}

TotalPowerInfo::~TotalPowerInfo()
{
    delete ui;
}

void TotalPowerInfo::showContextMenuRW(const QPoint &pos)
{
    QMenu *menu = new QMenu(ui->rwLabel);
    QAction *actionShowRW = new QAction(tr("Show RW Details"), menu);
    QAction *actionHideRW = new QAction(tr("Hide RW Details"), menu);
    if (ui->rwSubLabel_1->isHidden())
    {
        menu->addAction(actionShowRW);
        for (int i=0; i<listRW.size(); i++)
        {
            connect(actionShowRW, SIGNAL(triggered()), listRW[i], SLOT(show()));
        }
        
    } else {
        menu->addAction(actionHideRW);
        for (int i=0; i<listRW.size(); i++)
        {
            connect(actionHideRW, SIGNAL(triggered()), listRW[i], SLOT(hide()));
        }
    }
    menu->exec(ui->rwLabel->mapToGlobal(pos));
}

void TotalPowerInfo::saveSettings(QSettings *settings)
{
    settings->beginGroup("powerInfo");
    settings->setValue("viewTotal", ui->actionTotalPower->isChecked());
    settings->setValue("viewVerbose", ui->actionVerbose_DisplayNew->isChecked());
    settings->setValue("viewChart", ui->actionPieChart->isChecked());
    
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}

void TotalPowerInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("powerInfo");
    ui->actionTotalPower->setChecked(settings->value("viewTotal").toBool());
    ui->actionVerbose_DisplayNew->setChecked(settings->value("viewVerbose").toBool());
    ui->actionPieChart->setChecked(settings->value("viewChart").toBool());

    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void TotalPowerInfo::updatePower (AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager= dynamic_cast<AdcsSimDataManager *>(simDataManager);
    SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
    
    // RW Power Group
    char outStringRW[256]="";
    double P_minRW = 0.0;
    double P_spinRW = 0.0;
    double P_torqueRW = 0.0;
    double intermediateParam = 0.0;
    std::vector<RWSim>::iterator itRw;
    for (itRw = scSim->reactionWheels.begin(); itRw != scSim->reactionWheels.end(); itRw++) {
        intermediateParam = (scSim->rwPowerMax - scSim->rwPowerMin) / (itRw->Omega_max * RPM);
        P_minRW += scSim->rwPowerMin;
        P_spinRW += fabs(itRw->Omega) * intermediateParam;
        P_torqueRW += fabs(itRw->u_current * itRw->Omega);
    }
    double rwPower = P_minRW + P_spinRW + P_torqueRW;
    sprintf(outStringRW,"%.2f", rwPower);
    ui->rwPower->setText(QString(outStringRW)+" W");
    
    char outStringRWsub1[256]="";
    sprintf(outStringRWsub1, "%.2f", P_minRW);
    char outStringRWsub2[256]="";
    sprintf(outStringRWsub2, "%.2f", P_spinRW);
    char outStringRWsub3[256]="";
    sprintf(outStringRWsub3, "%.2f", P_torqueRW);
    
    ui->rwSubPower_1->setText("<font color='grey'>"+QString(outStringRWsub1)+" W</font>");
    ui->rwSubPower_2->setText("<font color='grey'>"+QString(outStringRWsub2)+" W</font>");
    ui->rwSubPower_3->setText("<font color='grey'>"+QString(outStringRWsub3)+" W</font>");
    
    // TR Power Group
    char outStringTR[256]="";
    double trPower =0.0;
    for(int i = 0; i < NUM_TR; i++) {
        trPower += fabs(scSim->tr[i].u) / 6.0 * scSim->trPowerMax;
        // sc->control.maxTRDipoleMoment = 6.0
    }
    sprintf(outStringTR,"%.2f", trPower);
    ui->trPower->setText(QString(outStringTR)+" W");
    
    // Total Power
    char outStringTotal[256]="";
    double totalPower = rwPower + trPower;
    sprintf(outStringTotal, "%.2f", totalPower);
    m_label->setText("<b>TOTAL POWER:</b> ");
    m_value->setText("<b>"+QString(outStringTotal)+" W</b>");
    
    for (int i=0; i<powerList.size(); i++)
    {
        powerList[i]= 0.1111;
    }
    m_graph->getPowerList(powerList, nameList);
    
}
