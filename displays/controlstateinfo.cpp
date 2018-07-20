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
#include "controlstateinfo.h"
#include "ui_controlstateinfo.h"
#include "enumConversions.h"

#include <QMenu>
#include <QAction>
#include <QSignalMapper>

ControlStateInfo::ControlStateInfo(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ControlStateInfo),
    m_lastControlMode(-1)
{
    ui->setupUi(this);
    ui->controlState->setMinimumWidth(80);
    ui->controlState->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->controlState, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showControlStateContextMenu(const QPoint &)));
    ui->adcsState->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->adcsState, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showAdcsStateContextMenu(const QPoint &)));
    ui->controlMode->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->controlMode, SIGNAL (customContextMenuRequested(const QPoint &)), this, SLOT(showControlModeContextMenu(const QPoint &)));
    
    if (m_lastControlMode==0)
    {
        ui->controlMode->setText("Control With Reaction Wheels");
    }
    else if (m_lastControlMode==1)
    {
        ui->controlMode->setText("Control with Thrusters");
    }
    else {ui->controlMode->setText("Default");}
}

ControlStateInfo::~ControlStateInfo()
{
    delete ui;
}

void ControlStateInfo:: saveSettings(QSettings *settings)
{
    settings->beginGroup("controlStateInfo");
    settings->setValue("controlModeLabel", m_lastControlMode);
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
    
}
void ControlStateInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("controlStateInfo");
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void ControlStateInfo::showControlModeContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(ui->controlMode);
    QSignalMapper *signalMapper = new QSignalMapper(menu);
    QAction *actionIsUstingThrust = new QAction(menu);
    actionIsUstingThrust->setText("Control With Thrusters");
    QAction *actionRegular = new QAction(menu);
    actionRegular->setText("Control With Reaction Wheels");
    
    connect(actionRegular, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(actionRegular, actionRegular->text());
    menu->addAction(actionRegular);
    connect(actionIsUstingThrust, SIGNAL(triggered()), signalMapper, SLOT(map()));
    signalMapper->setMapping(actionIsUstingThrust, actionIsUstingThrust->text());
    menu->addAction(actionIsUstingThrust);
    
    
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setThrustingControlMode(QString)));
    menu->exec(ui->controlState->mapToGlobal(pos));
}

void ControlStateInfo::setThrustingControlMode(QString value)
{
    if (value== "Control With Thrusters")
    {
        emit setThrustingControlMode(1);
    } else if (value=="Control With Reaction Wheels"){
        emit setThrustingControlMode(0);
    }
    
    ui->controlMode->setText(value);
}

void ControlStateInfo::showControlStateContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(ui->controlState);
    
    QSignalMapper *signalMapper = new QSignalMapper(menu);
    for(int i = 0; i < MAX_CMD_STATE; i++) {
        QAction *action = new QAction(menu);
        std::stringstream ss;
        ss << enumToString((CommandedState_t)i);
        action->setText(QString(ss.str().c_str()));
        connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
        signalMapper->setMapping(action, action->text());
        menu->addAction(action);
    }
    
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setControlState(QString)));
    menu->exec(ui->controlState->mapToGlobal(pos));
}

void ControlStateInfo::setControlState(QString value)
{
    CommandedState_t state;
    std::stringstream ss(value.toStdString());
    ss >> enumFromString(state);
    emit setControlState(state);
}

void ControlStateInfo::showAdcsStateContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(ui->adcsState);
    
    QSignalMapper *signalMapper = new QSignalMapper(menu);
    for(int i = 0; i < MAX_ADCS_STATE; i++) {
        QAction *action = new QAction(menu);
        std::stringstream ss;
        ss << enumToString((ADCSState_t)i);
        action->setText(QString(ss.str().c_str()));
        connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
        signalMapper->setMapping(action, action->text());
        menu->addAction(action);
    }
    
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setAdcsState(QString)));
    menu->exec(ui->adcsState->mapToGlobal(pos));
}

void ControlStateInfo::setAdcsState(QString value)
{
    ADCSState_t state;
    std::stringstream ss(value.toStdString());
    ss >> enumFromString(state);
    emit setAdcsState(state);
}

void ControlStateInfo::updateControl(AdcsSimDataManager *simDataManager)
{
//    AdcsSimDataManager *adcsSimDataManager= dynamic_cast<AdcsSimDataManager *>(simDataManager);
//    if (adcsSimDataManager)
//    {
//        SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
//        
//        /* #TODO: this is just a provisional way to update the controlMode label properly*/
//        SpacecraftSimVisualization *scSimVisualization = adcsSimDataManager->getSpacecraftSimVisualization();
//        // Availability condition of RW to turn AC Thrusters ON
//        int numAvailableRWs=NUM_RW;
//        for(int i=0; i<NUM_RW; i++)
//        {
//            if (scSim->rw[i].state!=COMPONENT_ON)
//            {
//                numAvailableRWs= numAvailableRWs-1;
//            }
//        }
//        // adcs State condition for AC Thrusters use
//        if (scSim->adcsState == ADCS_THRUSTING || scSim->adcsState == ADCS_RATE_DAMPING)
//        {
//            scSimVisualization->controlUsingThrusters = 2;
//        } else if(scSim->time>0. && numAvailableRWs < 3) {
//            scSimVisualization->controlUsingThrusters=2;
//        }
//        
//        if (scSimVisualization->controlUsingThrusters==0)
//        {
//            ui->controlMode->setText("Control With Reaction Wheels");
//        } else if (scSimVisualization->controlUsingThrusters==1) {
//            ui->controlMode->setText("Control with Thrusters");
//        } else if(scSimVisualization->controlUsingThrusters==2) {
//            ui->controlMode->setText("Default");
//        }
//        
//        
//        char outString[256] = "";
//        
//        QString style = "QLabel{color:black;";
//        switch(scSim->ctrlState) {
//            case CMD_ON:
//                style.append(" background-color:green;}");
//                ui->controlState->setText("On");
//                break;
//            case CMD_OFF:
//            default:
//                style.append(" background-color:red;}");
//                ui->controlState->setText("Off");
//                break;
//        }
//        ui->controlState->setStyleSheet(style);
//        
//        std::stringstream ss;
//        ss << enumToString(scSim->adcsState);
//        ui->adcsState->setText(QString(ss.str().c_str()));
//        QFont f("Arial",7);
//        ui->adcsState->setFont(f);
//        
//        QString styleOn = "QLabel{color:green;}";
//        style = ui->adcsStateLabel->styleSheet();
//        ui->sunPointingError->setStyleSheet(scSim->adcsState == ADCS_SUN_POINTING ? styleOn : style);
//        
//        sprintf(outString, "%+7.4f deg", scSim->sunAngle * R2D);
//        ui->sunPointingError->setText(outString);
//        
//        // Hide everything and then turn on what is desired in switch statement
//        ui->attitudePointingError->hide();
//        ui->attitudePointingErrorLabel->hide();
//        ui->attitudeRateError->hide();
//        ui->attitudeRateErrorLabel->hide();
//        ui->Status1->hide();
//        ui->Status1Label->hide();
//        ui->Status2->hide();
//        ui->Status2Label->hide();
//        
//        switch (scSim->adcsState) {
//            case ADCS_HILL_POINTING:
//            {
//                ui->attitudePointingErrorLabel->show();
//                ui->attitudePointingErrorLabel->setText("Hill Att. Error");
//                ui->attitudePointingError->show();
//                double HillAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_NP)) * R2D;
//                sprintf(outString, "%+7.4f deg", HillAngle);
//                ui->attitudePointingError->setText(outString);
//                ui->attitudePointingError->setStyleSheet(styleOn);
//                
//                ui->attitudeRateErrorLabel->show();
//                ui->attitudeRateErrorLabel->setText("Hill Rate Error");
//                ui->attitudeRateError->show();
//                double rateError = v3Norm(scSim->control.omega_BR_NP) * R2D;
//                sprintf(outString, "%+7.5f deg/s", rateError);
//                ui->attitudeRateError->setText(outString);
//                ui->attitudeRateError->setStyleSheet(styleOn);
//                break;
//            }
//            case ADCS_HILL_SPIN:
//            {
//                ui->attitudePointingErrorLabel->show();
//                ui->attitudePointingErrorLabel->setText("Hill-Spin Att. Error");
//                ui->attitudePointingError->show();
//                double HillAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_NS)) * R2D;
//                sprintf(outString, "%+7.4f deg", HillAngle);
//                ui->attitudePointingError->setText(outString);
//                ui->attitudePointingError->setStyleSheet(styleOn);
//                
//                ui->attitudeRateErrorLabel->show();
//                ui->attitudeRateErrorLabel->setText("Hill-Spin Rate Error");
//                ui->attitudeRateError->show();
//                double rateError = v3Norm(scSim->control.omega_BR_NS) * R2D;
//                sprintf(outString, "%+7.5f deg/s", rateError);
//                ui->attitudeRateError->setText(outString);
//                ui->attitudeRateError->setStyleSheet(styleOn);
//                break;
//            }
//            case ADCS_VELOCITY_POINTING:
//            {
//                ui->attitudePointingErrorLabel->show();
//                ui->attitudePointingErrorLabel->setText("Velocity Att. Error");
//                ui->attitudePointingError->show();
//                double velocityAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_VP)) * R2D;
//                sprintf(outString, "%+7.4f deg", velocityAngle);
//                ui->attitudePointingError->setText(outString);
//                ui->attitudePointingError->setStyleSheet(styleOn);
//                ui->attitudeRateError->setStyleSheet(styleOn);
//                
//                ui->attitudeRateErrorLabel->show();
//                ui->attitudeRateErrorLabel->setText("Velocity Rate Error");
//                ui->attitudeRateError->show();
//                double rateError = v3Norm(scSim->control.omega_BR_VP) * R2D;
//                sprintf(outString, "%+7.5f deg/s", rateError);
//                ui->attitudeRateError->setText(outString);
//                break;
//            }
//            case ADCS_VELOCITY_SPIN:
//            {
//                ui->attitudePointingErrorLabel->show();
//                ui->attitudePointingErrorLabel->setText("Vel.-Spin Att. Error");
//                ui->attitudePointingError->show();
//                double velocityAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_VS)) * R2D;
//                sprintf(outString, "%+7.4f deg", velocityAngle);
//                ui->attitudePointingError->setText(outString);
//                ui->attitudePointingError->setStyleSheet(styleOn);
//                
//                ui->attitudeRateErrorLabel->show();
//                ui->attitudeRateErrorLabel->setText("Vel.-Spin Rate Error");
//                ui->attitudeRateError->show();
//                double rateError = v3Norm(scSim->control.omega_BR_VS) * R2D;
//                sprintf(outString, "%+7.5f deg/s", rateError);
//                ui->attitudeRateError->setText(outString);
//                ui->attitudeRateError->setStyleSheet(styleOn);
//                break;
//            }
//            case ADCS_EARTH_POINTING:
//            {
//                ui->attitudePointingErrorLabel->show();
//                ui->attitudePointingErrorLabel->setText("Earth Pointing Error");
//                ui->attitudePointingError->show();
//                double velocityAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_EP)) * R2D;
//                sprintf(outString, "%+7.4f deg", velocityAngle);
//                ui->attitudePointingError->setText(outString);
//                ui->attitudePointingError->setStyleSheet(styleOn);
//                
//                ui->attitudeRateErrorLabel->show();
//                ui->attitudeRateErrorLabel->setText("Earth Rate Error");
//                ui->attitudeRateError->show();
//                double rateError = v3Norm(scSim->control.omega_BR_EP) * R2D;
//                sprintf(outString, "%+7.5f deg/s", rateError);
//                ui->attitudeRateError->setText(outString);
//                ui->attitudeRateError->setStyleSheet(styleOn);
//                
//                ui->Status1Label->show();
//                ui->Status1Label->setText("Deadband");
//                ui->Status1->show();
//                style = "QLabel{color:black;";
//                if(scSim->control.deadbandStatus == DEADBAND_OFF) {
//                    style.append(" background-color:gray;}");
//                    ui->Status1->setText("Off");
//                } else if(scSim->control.deadbandStatus == DEADBAND_ON)  {
//                    style.append(" background-color:green;}");
//                    ui->Status1->setText("On");
//                } else {
//                    style.append(" background-color:orange;}");
//                    ui->Status1->setText("No Signal");
//                }
//                ui->Status1->setStyleSheet(style);
//                break;
//            }
//            case ADCS_THRUSTING:
//            case ADCS_INERTIAL3D_POINTING:
//            {
//                ui->attitudePointingErrorLabel->show();
//                ui->attitudePointingErrorLabel->setText("Inertial Pointing Error");
//                ui->attitudePointingError->show();
//                double velocityAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_IP)) * R2D;
//                sprintf(outString, "%+7.4f deg", velocityAngle);
//                ui->attitudePointingError->setText(outString);
//                ui->attitudePointingError->setStyleSheet(styleOn);
//                
//                ui->attitudeRateErrorLabel->show();
//                ui->attitudeRateErrorLabel->setText("Inertial Rate Error");
//                ui->attitudeRateError->show();
//                double rateError = v3Norm(scSim->control.omega_BR_IP) * R2D;
//                sprintf(outString, "%+7.5f deg/s", rateError);
//                ui->attitudeRateError->setText(outString);
//                ui->attitudeRateError->setStyleSheet(styleOn);
//                break;
//            }
//            case ADCS_INERTIAL3D_SPIN:
//            {
//                ui->attitudePointingErrorLabel->show();
//                ui->attitudePointingErrorLabel->setText("Inertial Pointing Error");
//                ui->attitudePointingError->show();
//                double velocityAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_IS)) * R2D;
//                sprintf(outString, "%+7.4f deg", velocityAngle);
//                ui->attitudePointingError->setText(outString);
//                ui->attitudePointingError->setStyleSheet(styleOn);
//                
//                ui->attitudeRateErrorLabel->show();
//                ui->attitudeRateErrorLabel->setText("Inertial Rate Error");
//                ui->attitudeRateError->show();
//                double rateError = v3Norm(scSim->control.omega_BR_IS) * R2D;
//                sprintf(outString, "%+7.5f deg/s", rateError);
//                ui->attitudeRateError->setText(outString);
//                ui->attitudeRateError->setStyleSheet(styleOn);
//                break;
//            }
//            case ADCS_RATE_DAMPING:
//            {
//                ui->attitudeRateErrorLabel->show();
//                ui->attitudeRateErrorLabel->setText("Inertial Rate Error");
//                ui->attitudeRateError->show();
//                double rateError = v3Norm(scSim->control.omega_BR_RD) * R2D;
//                sprintf(outString, "%+7.5f deg/s", rateError);
//                ui->attitudeRateError->setText(outString);
//                ui->attitudeRateError->setStyleSheet(styleOn);
//                break;
//            }
//            case ADCS_SUN_POINTING:
//            {
//                ui->attitudeRateErrorLabel->show();
//                ui->attitudeRateErrorLabel->setText("Rate Error");
//                ui->attitudeRateError->show();
//                double rateError = v3Norm(scSim->control.omega_BR_SP) * R2D;
//                sprintf(outString, "%+7.5f deg/s", rateError);
//                ui->attitudeRateError->setText(outString);
//                ui->attitudeRateError->setStyleSheet(styleOn);
//                
//                ui->Status1Label->show();
//                ui->Status1Label->setText("Deadband");
//                ui->Status1->show();
//                style = "QLabel{color:black;";
//                if(scSim->control.deadbandStatus == DEADBAND_OFF) {
//                    style.append(" background-color:gray;}");
//                    ui->Status1->setText("Off");
//                } else if(scSim->control.deadbandStatus == DEADBAND_ON)  {
//                    style.append(" background-color:green;}");
//                    ui->Status1->setText("On");
//                } else {
//                    style.append(" background-color:orange;}");
//                    ui->Status1->setText("No Signal");
//                }
//                ui->Status1->setStyleSheet(style);
//                break;
//            }
//            default:
//            {
//                break;
//            }
//        }
//    }
}