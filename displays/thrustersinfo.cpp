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
#include "thrustersinfo.h"
#include "ui_thrustersinfo.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QAction>
#include <QMenu>

ThrustersInfo::ThrustersInfo(QWidget *parent, AdcsSimDataManager *simData)
    : QDockWidget(parent)
    , ui(new Ui::ThrustersInfo)
    , m_layoutNumCol(4)
    , m_layoutNumColDV(3)
    , m_thrustersON(false)
    , m_numUpdates(0)
//    m_buttonClicked(false),
//    m_timeUntilThrustersAreTurnOn(new QLineEdit(this)),
//    m_newDurationTime(new QLineEdit(this)),
//    maxTimeDuration(4000),
//    m_button(new QPushButton(this)),
//    m_label (new QLabel(this)),
    , m_legend (new QProgressBar(this))
{
    m_simDataManager = simData;
    this->ui->setupUi(this);
}

ThrustersInfo::~ThrustersInfo()
{
    delete ui;
}

void ThrustersInfo::resetUiElements()
{
    this->acsList1.clear();
    this->acsList2.clear();
    this->dvList2.clear();
    this->legendLabelList.clear();
    
    QLayoutItem* item;
    if (this->ui->ACS_ThrustersGroup->children().count()){
        while ((item = this->ui->ACS_ThrustersGroup->layout()->takeAt(0)) != NULL)
        {
            delete item->widget();
            delete item;
        }
    }
    
    if (this->ui->DV_ThrustersGroup->children().count()){
        while ((item = this->ui->DV_ThrustersGroup->layout()->takeAt(0)) != NULL)
        {
            delete item->widget();
            delete item;
        }
    }
    
    //    if (this->ui->PowerPlotGroup->children().count()){
    //        while ((item = this->ui->PowerPlotGroup->layout()->takeAt(0)) != NULL)
    //        {
    //            delete item->widget();
    //            delete item;
    //        }
    //    }
}

void ThrustersInfo::configureLayout()
{
    this->resetUiElements();
    this->ui->setupUi(this);
    SpacecraftSim *scSim = this->m_simDataManager->getSpacecraftSim();
    
    ui->actionDV_Thrusters->setText(QString(QChar(0x94,0x03))+"V Thrusters");
    ui->ACS_ThrustersGroup->setTitle("Attitude Control Thrusters");
    ui->DV_ThrustersGroup->setTitle(QString(QChar(0x94,0x03))+"V Thrusters");
    
    ui->actionDV_Thrusters->setVisible(true);
    ui->actionDV_Thrusters->setChecked(true);
    //    ui->DV_ThrustersGroup->hide();
    
    ui->ACS_ThrustersGroup->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->ACS_ThrustersGroup,SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showLevelContextMenu(const QPoint &)));
    
    QGridLayout *acsLayout = new QGridLayout;
    QGridLayout *dvLayout = new QGridLayout;
    
    for (int i = 0; i < scSim->acsThrusters.size(); i++)
    {
        QVBoxLayout *vBox = new QVBoxLayout;
        QLabel *acsThrustLabel = new QLabel;
        //acsThrustLabel->setToolTip("% Thrust Max");
        acsThrustLabel->setMargin(6);
        acsThrustLabel->setAlignment(Qt::AlignCenter);
        vBox-> addWidget(acsThrustLabel);
        acsList1.append(acsThrustLabel);
        acsList1[i]->hide();
        
        OnOffCircle *tmpCircle = new OnOffCircle;
        acsList2.append(tmpCircle);
        vBox->addWidget(tmpCircle);
        
        acsLayout->addLayout(vBox, i / (m_layoutNumCol), i % (m_layoutNumCol));
    }
    
    for (int i = 0; i < m_layoutNumCol; i++)
    {
        acsLayout->setColumnStretch(i, 1);
    }
    
    QHBoxLayout *hBox = new QHBoxLayout;
    QLabel *label0 = new QLabel;
    label0->setText("<font size=1><br>0</font>");
    label0->setAlignment(Qt::AlignRight);
    label0->setFixedWidth(12);
    hBox->addWidget(label0);
    label0->hide();
    legendLabelList.append(label0);
    m_legend->setMaximum(1);
    m_legend->setMinimum(0);
    m_legend->setValue(1);
    m_legend->setFormat("thruster level %");
    m_legend->setStyleSheet("QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #a2e6a2, stop: 1 green);}");
    m_legend->setAlignment(Qt::AlignCenter);
    m_legend->hide();
    hBox->addWidget(m_legend);
    QLabel *label100 = new QLabel;
    label100->setText("<font size=1><br>100</font>");
    label100->setAlignment(Qt::AlignLeft);
    label100->setFixedWidth(12);
    hBox->addWidget(label100);
    label100->hide();
    legendLabelList.append(label100);
    
    QVBoxLayout *container= new QVBoxLayout;
    
    container->addLayout(acsLayout);
    container->addLayout(hBox);
    ui->ACS_ThrustersGroup->setLayout(container);
    
    for (int i = 0; i < scSim->dvThrusters.size(); i++){
        QVBoxLayout *vBox2 = new QVBoxLayout;
        OnOffCircle *tmpCircleDV = new OnOffCircle;
        dvList2.append(tmpCircleDV);
        vBox2->addWidget(tmpCircleDV);
        dvLayout->addLayout(vBox2, i / m_layoutNumColDV, i % m_layoutNumColDV);
    }
    
    ui->verticalLayoutThrusters->addLayout(dvLayout);
    
    //    m_label->setText("Add Maneuver");
    //    m_label->setAlignment(Qt::AlignCenter);
    //
    //    m_durationTimeValidator = new QIntValidator(0, maxTimeDuration);
    //    m_button->setText("Set New Maneuver");
    //    m_button->setToolTip("Press to set up the new maneuver");
    //    connect(m_button, SIGNAL(clicked()), this, SLOT(setNewManeuver()));
    //    m_timeUntilThrustersAreTurnOn->setPlaceholderText("Seconds until Turn On");
    //    m_timeUntilThrustersAreTurnOn->setToolTip("0 to begin now");
    //    m_timeUntilThrustersAreTurnOn->setToolTipDuration(3000);
    //    m_timeUntilThrustersAreTurnOn->setAttribute(Qt::WA_MacShowFocusRect, 0);
    //    m_timeUntilThrustersAreTurnOn->setStyleSheet("QLineEdit{padding: 1px; border-style: solid; border: 0.5px solid grey;}");
    //    m_timeUntilThrustersAreTurnOn->setValidator(m_durationTimeValidator);
    //    m_timeUntilThrustersAreTurnOn->setAlignment(Qt::AlignCenter);
    //    m_newDurationTime->setPlaceholderText("Duration in sec");
    //    m_newDurationTime->setToolTip("Duration in sec");
    //    m_newDurationTime->setToolTipDuration(3000);
    //    m_newDurationTime->setAttribute(Qt::WA_MacShowFocusRect, 0);
    //    m_newDurationTime->setStyleSheet("QLineEdit{padding: 1px; border-style: solid; border: 0.5px solid grey;}");
    //    m_newDurationTime->setAlignment(Qt::AlignCenter);
    //    m_newDurationTime->setValidator(new QIntValidator(0, maxTimeDuration, this));
    
    //    QLabel *blankLabel = new QLabel;
    //    ui->verticalLayoutThrusters->addWidget(blankLabel);
    //    ui->verticalLayoutThrusters->addWidget(m_label);
    //    ui->verticalLayoutThrusters->addWidget(m_timeUntilThrustersAreTurnOn);
    //    ui->verticalLayoutThrusters->addWidget(m_newDurationTime);
    //    ui->verticalLayoutThrusters->addWidget(m_button);
    for (int i = 0; i < acsList1.size(); i++)
    {
        acsList1[i]->show();
    }
    for (int i = 0; i < dvList2.size(); i++)
    {
        dvList2[i]->show();
    }
}

void ThrustersInfo::saveSettings(QSettings *settings)
{
    settings->beginGroup("thrusterInfor");
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}

void ThrustersInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("thrusterInfo");
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void ThrustersInfo::setNewManeuver()
{
    //    m_buttonClicked = true;
}

void ThrustersInfo::showLevelContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(ui->ACS_ThrustersGroup);
    
    QAction *actionLevel = new QAction(tr("Show Thrusting Status"), menu);
    //    QAction *actionLegend = new QAction(tr("Show Color Legend"), menu);
    QAction *actionLevelHide = new QAction(tr("Hide Thrusting Status"), menu);
    //    QAction *actionLegendHide = new QAction(tr("Hide Color Legend"), menu);
    if (acsList1[0]->isHidden())
    {
        menu->addAction(actionLevel);
        for (int i=0; i<acsList1.size(); i++)
        {
            connect(actionLevel, SIGNAL(triggered()), acsList1[i], SLOT(show()));
        }
    } else{
        menu->addAction(actionLevelHide);
        for (int i=0; i<acsList1.size(); i++)
        {
            connect(actionLevelHide, SIGNAL(triggered()), acsList1[i], SLOT(hide()));
        }
    }
    //    if (m_legend->isHidden())
    //    {
    //        menu->addAction(actionLegend);
    //        connect(actionLegend, SIGNAL(triggered()), m_legend, SLOT(show()));
    //        for (int i=0; i<legendLabelList.size();i++)
    //        {
    //            connect(actionLegend, SIGNAL(triggered()), legendLabelList[i], SLOT(show()));
    //        }
    //
    //    } else {
    //        menu->addAction(actionLegendHide);
    //        connect(actionLegendHide, SIGNAL(triggered()), m_legend, SLOT(hide()));
    //        for (int i=0; i<legendLabelList.size();i++)
    //        {
    //            connect(actionLegendHide, SIGNAL(triggered()), legendLabelList[i], SLOT(hide()));
    //        }
    //    }
    menu->exec(ui->ACS_ThrustersGroup->mapToGlobal(pos));
}

void ThrustersInfo::updateThrusters(AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager = dynamic_cast<AdcsSimDataManager *>(simDataManager);
    if(adcsSimDataManager)
    {
        SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
        
        
        for (int i = 0; i < scSim->dvThrusters.size(); i++)
        {
            if (scSim->dvThrusters.at(i).level > 0)
            {
                QBrush brush = QColor(qRgb(101, 203, 253));
                setToolTip("On");
                dvList2[i]->getPercent(scSim->dvThrusters.at(i).level, brush);
                dvList2[i]->setToolTip("On");
            } else {
                dvList2[i]->setPercent(0);
                dvList2[i]->setToolTip("Off");
            }
            dvList2[i]->setToolTipDuration(2000);
        }
        ui->thrustBar->hide();

        char outString[256] = "";
        QFont f("Arial",8);
        std::vector<Thruster>::iterator itTh;
        for (itTh = scSim->acsThrusters.begin(); itTh != scSim->acsThrusters.end(); itTh++)
        {
            Thruster tmpTh = (*itTh);
            int i = itTh - scSim->acsThrusters.begin();
            acsList1[i]->setFont(f);
            if (tmpTh.level > 0.0) {
                double x = (100 - tmpTh.level) * 1.62;
                double y = 230 - 102*(x/162 - 1)*(x/162-1);
                QBrush brush = QColor(qRgb(int(x), int(y), int(x)));
                //                 QBrush brush = QColor(qRgb(0, 180, 0));
                acsList2[i]->getPercent(tmpTh.level, brush);
                acsList2[i]->setToolTipDuration(2000);
                
                if (tmpTh.level < 10) {
                    sprintf(outString, "%.0f", tmpTh.level);
                } else if (tmpTh.level < 100) {
                    sprintf(outString, "%.0f", tmpTh.level);
                } else {
                    sprintf(outString, "%.0f", scSim->acsThrusters[i].level);
                }
                acsList1[i]->setText(tr("%1%").arg(outString));
                acsList1[i]->setText("ON");
            } else {
                acsList2[i]->setPercent(0);
                acsList1[i]->setText("OFF");
            }
        }
        // THRUSTING MANEUVER
        //        if (scSim->DVThrusterManeuverDuration>0 && scSim->time<(scSim->DVThrusterManeuverDuration + scSim->DVThrusterManeuverStartTime))
        //        {
        //            m_label->hide();
        //            m_timeUntilThrustersAreTurnOn->hide();
        //            m_newDurationTime->hide();
        //        }
        //        else {
        //            m_label->show();
        //            m_timeUntilThrustersAreTurnOn->show();
        //            m_newDurationTime->show();
        //        }
        //
        //        maxTimeDuration = scSim->maxSimTime - scSim->time;
        //        if (m_timeUntilThrustersAreTurnOn->text().isEmpty() || m_newDurationTime->text().isEmpty())
        //        {
        //            m_button->hide();
        //
        //        } else {
        //            m_startTime = m_timeUntilThrustersAreTurnOn->text().toInt();
        //            if (m_startTime ==0) {m_startTime=1 /*sec*/;} //Necessary to account for the time lapse between signal's emission and new maneuver setting
        //            m_durationTime = m_newDurationTime->text().toInt();
        //            maxTimeDuration = scSim->maxSimTime - scSim->time - m_startTime;
        //            if (m_durationTime >=m_startTime)
        //            {
        //                m_button->show();
        //                if (m_buttonClicked==true)
        //                {
        //                    emit setNewManeuver(double(m_startTime), double (m_durationTime));
        //                    m_buttonClicked = false;
        //                    m_timeUntilThrustersAreTurnOn->clear();
        //                    m_newDurationTime->clear();
        //                }
        //            }
        //        }
        //
        //        ui->thrustBar->hide();
        //        ui->thrustBar->setAlignment(Qt::AlignLeft);
        //        if (scSim->DVThrusterManeuverStartTime == -1.0 && scSim->DVThrusterManeuverDuration == -1.0)
        //        {
        //            ui->thrust_1->setText("No maneuver set");
        //            ui->thrust_1->setAlignment(Qt::AlignCenter);
        //            ui->thrust_2->hide();
        //        } else {
        //            ui->thrustBar->show();
        //            ui->thrustBar->setAlignment(Qt::AlignCenter);
        //            ui->thrustBar->setMinimum(scSim->DVThrusterManeuverStartTime);
        //            ui->thrustBar->setMaximum(scSim->DVThrusterManeuverStartTime + scSim->DVThrusterManeuverDuration);
        //            ui->thrustBar->setStyleSheet("QProgressBar::chunk {background-color:#3ca2d5; }"); /* #66cbfe  59b3df*/
        //
        //            double time = simDataManager->getSimTime();
        //            char outString [256]="";
        //            if (scSim->DVThrusterManeuverStartTime - time >= 0)
        //            {
        //                sprintf(outString, "%.2f", scSim->DVThrusterManeuverStartTime - time);
        //                ui->thrust_1->setText(tr("Start in: %1 sec").arg(outString));
        //            }
        //            else if(time -(scSim->DVThrusterManeuverStartTime + scSim->DVThrusterManeuverDuration) >= 0)
        //            {
        //                ui->thrust_1->setText("Maneuver ended");
        //                ui->thrustBar->setValue(scSim->DVThrusterManeuverStartTime + scSim->DVThrusterManeuverDuration);
        //                ui->thrustBar->setStyleSheet("QProgressBar::chunk {background-color: darkslategray; }");
        //            }
        //            else
        //            {
        //                sprintf(outString, "%.2f", scSim->DVThrusterManeuverStartTime +  scSim->DVThrusterManeuverDuration - time);
        //                ui->thrust_1->setText(tr("Ending in: %1 sec").arg(outString));
        //                ui->thrustBar->setValue(time);
        //            }
        //            ui->thrust_1->setAlignment(Qt::AlignCenter);
        //            ui->thrust_2->setText(tr("Duration: %1 sec").arg(scSim->DVThrusterManeuverDuration));
        //            ui->thrust_2->setAlignment(Qt::AlignCenter);
        //            ui->thrust_2->show();
        //        }
    }
}
