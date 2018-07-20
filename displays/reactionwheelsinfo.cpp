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
#include "reactionwheelsinfo.h"
#include "ui_reactionwheelsinfo.h"
#include "math.h"
#include "enumConversions.h"
#include <QListWidget>

extern "C" {
#include "spacecraftDefinitions.h"
}

ReactionWheelsInfo::ReactionWheelsInfo(QWidget *parent, AdcsSimDataManager *simData)
    : QDockWidget(parent)
    , ui(new Ui::ReactionWheelsInfo)
    , m_layoutNumCol(4)
    , m_selecLabel(-1)
    , m_minimumColWidth(30)
    , m_legend (new Legend)
{
    m_simDataManager = simData;
    this->ui->setupUi(this);
}

ReactionWheelsInfo::~ReactionWheelsInfo()
{
    delete ui;
}

void ReactionWheelsInfo::resetUiElements()
{
    this->list.clear();
    this->list1.clear();
    this->list2.clear();
    this->list3.clear();
    this->list4.clear();
    this->barList.clear();
    this->statusList.clear();
    this->powerList.clear();
    
    QLayoutItem* item;
    if (this->ui->GraphicGroup->children().count()){
        while ((item = this->ui->GraphicGroup->layout()->takeAt(0)) != NULL)
        {
            delete item->widget();
            delete item;
        }
    }
    
    if (this->ui->VerboseGroupNew->children().count()){
        while ((item = this->ui->VerboseGroupNew->layout()->takeAt(0)) != NULL)
        {
            delete item->widget();
            delete item;
        }
    }
    
    if (this->ui->PowerPlotGroup->children().count()){
        while ((item = this->ui->PowerPlotGroup->layout()->takeAt(0)) != NULL)
        {
            delete item->widget();
            delete item;
        }
    }
}

void ReactionWheelsInfo::configureLayout()
{
    this->resetUiElements();
    this->ui->setupUi(this);
    //    addAction(ui->actionStatus);
    //    ui->actionStatus->setChecked(true);
    addAction(ui->actionVerbose_DisplayNew);
    ui->actionVerbose_DisplayNew->setChecked(true);
    addAction(ui->actionPowerPlot);
    ui->actionPowerPlot->setChecked(true);
    
    QGridLayout *layout = new QGridLayout;
    QGridLayout *infoLayout = new QGridLayout;
    QGridLayout *barLayout = new QGridLayout;
    QHBoxLayout *hBox = new QHBoxLayout;
    
    m_legend-> getLegendInfo("<font size=3>" + QString(QChar(0xA9, 0x03)) +"·1e3 [ rpm ] </font>");
    hBox->addWidget(m_legend);
    ui->LegendGroup->setLayout(hBox);
    ui->LegendGroup->hide();
    
    SpacecraftSim *tmpSc = this->m_simDataManager->getSpacecraftSim();
    
    for (int i = 0; i < tmpSc->reactionWheels.size(); i++)
    {
        // RW Graphic Group
        QVBoxLayout *vBox = new QVBoxLayout;
        
        QLabel *label = new QLabel;
        label->setText(tr("RW%1").arg(i + 1));
        label->setMargin(6);
        label->setAlignment(Qt::AlignCenter);
        vBox->addWidget(label);
        list.append(label);
        
        //        list[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(list[i], SIGNAL(customContextMenuRequested(const QPoint &)),
                this, SLOT(showOnOffStateContextMenu(const QPoint &)));
        
        TwoPlusMinusPercentBars *m_controlBar = new TwoPlusMinusPercentBars;
        vBox->addWidget(m_controlBar);
        barList.append(m_controlBar);
        
        // RW Status
        QLabel *statusLabel = new QLabel;
        statusLabel->setMargin(6);
        statusLabel->setAlignment(Qt::AlignCenter);
        vBox->addWidget(statusLabel);
        statusList.append(statusLabel);
        
        layout->addLayout(vBox, i / m_layoutNumCol, i % m_layoutNumCol);
        
        // RW Verbose Group
        QVBoxLayout *vInfoBox = new QVBoxLayout;
        QLabel *infoLabel1 = new QLabel;
        QLabel *infoLabel2 = new QLabel;
        QLabel *infoLabel3 = new QLabel;
        QLabel *infoLabel4 = new QLabel;
        
        infoLabel1->setText(tr("RW%1 info").arg(i+1));
        infoLabel1->setMargin(6);
        infoLabel1->setAlignment(Qt::AlignCenter);
        infoLabel2->setMargin(6);
        infoLabel2->setAlignment(Qt::AlignCenter);
        infoLabel2->setMinimumWidth(m_minimumColWidth);
        infoLabel3->setMargin(6);
        infoLabel3->setAlignment(Qt::AlignCenter);
        infoLabel4->setMargin(6);
        infoLabel4->setAlignment(Qt::AlignCenter);
        
        vInfoBox->addWidget(infoLabel1);
        list1.append(infoLabel1);
        vInfoBox->addWidget(infoLabel2);
        list2.append(infoLabel2);
        vInfoBox->addWidget(infoLabel3);
        list3.append(infoLabel3);
        vInfoBox->addWidget(infoLabel4);
        list4.append(infoLabel4);
        infoLayout->addLayout(vInfoBox, i / m_layoutNumCol, i % m_layoutNumCol);
        
        // RW Power Group
        QVBoxLayout *powerBox = new QVBoxLayout;
        QProgressBar *powerBar = new QProgressBar;
        powerBar->setMinimum(0);
        powerBar->setMaximum(1);
        powerBar->setValue(0);
        powerBar->setMinimumWidth(55);
        powerList.append(powerBar);
        powerBox->addWidget(powerBar);
        barLayout->addLayout(powerBox, i / m_layoutNumCol, i % m_layoutNumCol);
        
    }
    
    for (int i=0; i < m_layoutNumCol; i++)
    {
        infoLayout->setColumnStretch(i, 1);
        barLayout->setColumnStretch(i, 1);
        layout->setColumnStretch(i, 1);
    }
    
    ui->GraphicGroup->setLayout(layout);
    ui->VerboseGroupNew->setLayout(infoLayout);
    ui->PowerPlotGroup->setLayout(barLayout);
}

void ReactionWheelsInfo::setOnOffLegendRW(bool b)
{
    if (b==false)
    {
        ui->LegendGroup->hide();
    } else {
        ui->LegendGroup->show();
    }
}

void ReactionWheelsInfo::showOnOffStateContextMenu(const QPoint &pos)
{
    QLabel *selectedLabel= dynamic_cast<QLabel*>(QObject::sender());
    SpacecraftSim *tmpSc = this->m_simDataManager->getSpacecraftSim();
    for (int i = 0; i < tmpSc->reactionWheels.size(); i++)
    {
        if (list[i]->text() == selectedLabel->text())
        {
            m_selecLabel = i;
        }
    }
    
    if(m_selecLabel != -1){
        QMenu *menu = new QMenu(list[m_selecLabel]);
        QSignalMapper *signalMapper = new QSignalMapper(menu);
        for(int i = 0; i < MAX_COMPONENT_STATE; i++)
        {
            QAction *action = new QAction(menu);
            std::stringstream ss;
            ss << enumToString((ComponentState_t)i);
            action->setText(QString(ss.str().c_str()));
            connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
            signalMapper->setMapping(action, action->text());
            menu->addAction(action);
        }
        connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setOnOffState(QString)));
        menu->exec(list[m_selecLabel]->mapToGlobal(pos));
    }
}

void ReactionWheelsInfo::setOnOffState(QString value)
{
    ComponentState_t state;
    std::stringstream ss(value.toStdString());
    ss >> enumFromString(state);
    emit setOnOffState(state, m_selecLabel);
}

void ReactionWheelsInfo::saveSettings(QSettings *settings)
{
    settings->beginGroup("reactionWheelsInfo");
    settings->setValue("viewVerbose", ui->actionVerbose_DisplayNew->isChecked());
    settings->setValue("viewPowerPlot",ui->actionPowerPlot->isChecked());
    settings->setValue("viewStatus", ui->actionStatus->isChecked());
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}

void ReactionWheelsInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("reactionWheelsInfo");
    ui->actionVerbose_DisplayNew->setChecked(settings->value("viewVerbose").toBool());
    ui->actionPowerPlot->setChecked(settings->value("viewPowerPlot").toBool());
    ui->actionStatus->setChecked(settings->value("viewStatus").toBool());
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

QString ReactionWheelsInfo::getTempSpeedState(TempSpeedState_t propertyState)
{
    QString state;
    switch(propertyState){
        case TEMP_SPEED_NOMINAL:
            state.append("NOMINAL");
            break;
        case TEMP_SPEED_OVER_RATING:
            state.append("OVER RATING");
            break;
        case MAX_TEMP_SPEED_STATE:
            state.append("MAX");
            break;
        default:
            state.append("UNKNOWN");
            break;
    }
    return state;
}

QString ReactionWheelsInfo::getStateToolTip(ComponentState_t componentState, int resetCounter)
{
    QString output;
    switch(componentState) {
        case COMPONENT_OFF:
            output.append("Off");
            break;
        case COMPONENT_START:
            output.append("Start");
            break;
        case COMPONENT_ON:
            output.append("On");
            break;
        case COMPONENT_FAULT:
            output.append("Fault");
            break;
        case COMPONENT_TEST:
            output.append("Test");
            break;
        default:
            output.append("Unknown");
            break;
    }
    if(resetCounter > 0) {
        output.append(" (");
        output.append(QString::number(resetCounter));
        output.append(" resets)");
    }
    return output;
}

QString ReactionWheelsInfo::getStateStyleSheet(ComponentState_t componentState, int resetCounter)
{
    QString output = "QLabel{color:black;";
    switch(componentState) {
        case COMPONENT_OFF:
            output.append(" background-color:darkslategray;");
            break;
        case COMPONENT_START:
            output.append(" background-color:greenyellow;");
            break;
        case COMPONENT_ON:
            output.append(" background-color:green;");
            break;
        case COMPONENT_FAULT:
            output.append(" background-color:red;");
            break;
        case COMPONENT_TEST:
            output.append(" background-color:orange;");
            break;
        default:
            output.append(" background-color:white;");
            break;
    }
    if(resetCounter == 0) {
        output.append("}");
    } else {
        output.append(" border-color:red; border-width:0.5px; border-style:solid;}");
    }
    return output;
}

void ReactionWheelsInfo::updateReactionWheels(AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager = dynamic_cast<AdcsSimDataManager *>(simDataManager);
    SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
    
    char outString[256] = " ";
    
    std::vector<RWSim>::iterator itRw;
    int i = 0;
    for (itRw = scSim->reactionWheels.begin(); itRw != scSim->reactionWheels.end(); ++itRw){
        /* RW Graphic Group */
        double temp = (scSim->rwPowerMax - scSim->rwPowerMin) / (itRw->Omega_max * RPM);
        
        i = itRw - scSim->reactionWheels.begin();
        //        list[i]->setToolTip(getStateToolTip(itRw->state, itRw->resetCounter));
        list[i]->setToolTipDuration(2000);
        //        list[i]->setStyleSheet(getStateStyleSheet(itRw->state, itRw->resetCounter));
        
        double percent_u = 100 * itRw->u_current / itRw->u_max;
        double percent_w = 100 * itRw->Omega / itRw->Omega_max;
        //        double motorTempMax = fmax(itRw->motorTemp1, itRw->motorTemp2);
        //        double percent_T = motorTempMax / (itRw->maxTemp) * 100.0;
        
        barList[i]->setPercent1(percent_u);
        barList[i]->setPercent2(percent_w);
        //        barList[i]->setPercent3(percent_T);
        
        QFont f("Arial", 7);
        QString str;
        //        if(itRw->resetCounter==0)
        //        {
        //            str= "<font color='green'>"+QString::number(itRw->resetCounter)+ " RESETS </font>";
        //        } else {
        //            str="<font color='red'>"+QString::number(itRw->resetCounter)+ " RESETS </font>";
        //        }
        //        statusList[i]->setText("TEMP<br> " + getTempSpeedState(itRw->tempState)+"<br><br> SPEED<br>"+ getTempSpeedState(itRw->speedState)+"<br><br>" + str);
        //        statusList[i]->setFont(f);
        if (ui->actionStatus->isChecked())
        {
            statusList[i]->setVisible(true);
        } else {
            statusList[i]->setVisible(false);
        }
        
        QFont font("Arial",9);
        /* RW Verbose Group*/
        sprintf(outString, "%.1f", itRw->u_current*1000);
        list2[i]->setText(tr("%1").arg(outString));
        list2[i]->setToolTip(tr("Torque RW%1 [mN·m]").arg(i+1));
        list2[i]->setToolTipDuration(3500);
        list2[i]->setFont(font);
        
        sprintf(outString, "%+.0f", (itRw->Omega)/RPM);
        list3[i]->setText(tr("%1").arg(outString));
        list3[i]->setToolTip(tr("Spin Rate RW%1 [rpm]").arg(i+1));
        list3[i]->setToolTipDuration(3500);
        list3[i]->setFont(font);
        
        double value = scSim->rwPowerMin + fabs(itRw->Omega) * temp + fabs(itRw->u_current * itRw->Omega);
        sprintf(outString, "%.2f",value);
        list4[i]->setText(tr("%1").arg(outString));
        list4[i]->setToolTip(tr("Power Consumption RW%1 [W]").arg(i));
        list4[i]->setToolTipDuration(3500);
        list4[i]->setFont(font);
        
        /* RW Power Group*/
        double max =scSim->rwPowerMin + fabs(itRw->Omega_max) * temp + fabs(itRw->u_max * itRw->Omega_max);
        powerList[i]->setMaximum(max);
        powerList[i]->setValue(value);
        powerList[i]->setToolTip( tr("RW%1: ").arg(itRw - scSim->reactionWheels.begin()+1) + tr("%1% Pmax").arg(int(value/max*100)));
        powerList[i]->setToolTipDuration(3500);
        powerList[i]->setMinimumWidth(m_minimumColWidth+10);
        
        if (value / max*100>=80)
        {
            powerList[i]->setStyleSheet("QProgressBar::chunk{background: #fb525a; width: 8px; margin: 0.5px;} QProgressBar{ border: 2px solid grey; border-radius: 7px;}");
        } else {
            powerList[i]->setStyleSheet("QProgressBar::chunk{background: #6efd8f; width: 8px; margin: 0.5px;} QProgressBar{ border: 2px solid grey; border-radius: 7px;}");
        }
    }
}
