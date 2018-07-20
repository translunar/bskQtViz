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
#include "scsiminfodisplay.h"
#include "ui_scsiminfodisplay.h"

#include "reactionwheeldisplay.h"
#include "torqueroddisplay.h"
#include "oneplusminuspercentbar.h"

#include "enumConversions.h"
#include "Spacecraft.hpp"
extern "C" {
#include "astroConstants.h"
#include "linearAlgebra.h"
#include "spacecraftDefinitions.h"
#include <time.h>
}

#include <QMenu>
#include <QAction>
#include <QSignalMapper>

ScSimInfoDisplay::ScSimInfoDisplay(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::ScSimInfoDisplay)
    , m_layoutNumCol(4)
    , m_estPlaySpeed(0.0)
{
    ui->setupUi(this);

    // Set up some layouts
    QGridLayout *layout = new QGridLayout();
    ui->reactionWheelGroup->setLayout(layout);
    layout = new QGridLayout();
    ui->cssGroup->setLayout(layout);
    layout = new QGridLayout();
    ui->torqueRodGroup->setLayout(layout);

    // Set up context menu for sim info widget
    addAction(ui->actionSimulation_Time);
    addAction(ui->actionReaction_Wheels);
    addAction(ui->actionTorque_Bars);
    addAction(ui->actionCSS);
    addAction(ui->actionAngular_Rates);

    // Set up other context menus
    ui->controlState->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->controlState, SIGNAL(customContextMenuRequested(const QPoint &)),
        this, SLOT(showControlStateContextMenu(const QPoint &)));
    ui->adcsState->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->adcsState, SIGNAL(customContextMenuRequested(const QPoint &)),
        this, SLOT(showAdcsStateContextMenu(const QPoint &)));
    ui->angularRatesGroup->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->angularRatesGroup, SIGNAL(customContextMenuRequested(const QPoint &)),
        this, SLOT(showAngularRatesGroupContextMenu(const QPoint &)));
}

ScSimInfoDisplay::~ScSimInfoDisplay()
{
    delete ui;
}

void ScSimInfoDisplay::updateSimulationData(AdcsSimDataManager *simDataManager)
{
    // Cast the simdatamanager to the appropriate type
    AdcsSimDataManager *adcsSimDataManager = dynamic_cast<AdcsSimDataManager *>(simDataManager);
    if(adcsSimDataManager) {
        SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();

        // Update the simulation time
        updateSimTime(simDataManager->getSimTime(), scSim->realTimeSpeedUpFactor);

        updateReactionWheelDisplay(scSim);
        updateCssDisplay(scSim);
        updateTorqueRodDisplay(scSim);
        updateRatesDisplay(scSim);
        updateControlDisplay(scSim);
    }
}

void ScSimInfoDisplay::saveSettings(QSettings *settings)
{
    settings->beginGroup("scSimInfoDisplay");
    settings->setValue("viewSimulationTime", ui->actionSimulation_Time->isChecked());
    settings->setValue("viewReactionWheels", ui->actionReaction_Wheels->isChecked());
    settings->setValue("viewTorqueBars", ui->actionTorque_Bars->isChecked());
    settings->setValue("viewCss", ui->actionCSS->isChecked());
    settings->setValue("viewAngularRates", ui->actionAngular_Rates->isChecked());
    settings->endGroup();
}

void ScSimInfoDisplay::loadSettings(QSettings *settings)
{
    settings->beginGroup("scSimInfoDisplay");
    ui->actionSimulation_Time->setChecked(settings->value("viewSimulationTime").toBool());
    ui->actionReaction_Wheels->setChecked(settings->value("viewReactionWheels").toBool());
    ui->actionTorque_Bars->setChecked(settings->value("viewTorqueBars").toBool());
    ui->actionCSS->setChecked(settings->value("viewCss").toBool());
    ui->actionAngular_Rates->setChecked(settings->value("viewAngularRates").toBool());
    settings->endGroup();
}

void ScSimInfoDisplay::showControlStateContextMenu(const QPoint &pos)
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

void ScSimInfoDisplay::setControlState(QString value)
{
    CommandedState_t state;
    std::stringstream ss(value.toStdString());
    ss >> enumFromString(state);
    emit setControlState(state);
}

void ScSimInfoDisplay::showAdcsStateContextMenu(const QPoint &pos)
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

void ScSimInfoDisplay::setAdcsState(QString value)
{
    ADCSState_t state;
    std::stringstream ss(value.toStdString());
    ss >> enumFromString(state);
    emit setAdcsState(state);
}

void ScSimInfoDisplay::showAngularRatesGroupContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(ui->angularRatesGroup);

    QAction *action = new QAction(tr("Perturb rates"), menu);
    connect(action, SIGNAL(triggered()), this, SIGNAL(perturbRates()));
    menu->addAction(action);

    menu->exec(ui->angularRatesGroup->mapToGlobal(pos));
}

void ScSimInfoDisplay::setEstPlaySpeed(double value)
{
    m_estPlaySpeed = value;
}

void ScSimInfoDisplay::removeExcessItems(QGridLayout *layout, int maxItems)
{
    if(layout->count() > maxItems) {
        // Remove unnecessary items
        for(int i = layout->count(); i >= maxItems; i--) {
            QLayoutItem *child = layout->takeAt(i);
            if(child->widget() != 0) {
                delete child->widget();
            }
            delete child;
        }
    }
}

void ScSimInfoDisplay::updateSimTime(double time, double realTimeSpeedUpFactor)
{
    int hrs = (int)(time / 3600.0);
    time -= 3600.0 * hrs;
    int min = (int)(time / 60.0);
    time -= 60.0 * min;
    int sec = (int)time;
    time -= sec;
    char outString[256] = "";
    if (realTimeSpeedUpFactor >= 1.0){
        sprintf(outString, "%02d:%02d:%02d.%03d (%gx)", hrs, min, sec, (int)std::floor(time * 1000),realTimeSpeedUpFactor);
    } else if (realTimeSpeedUpFactor < 1.0 && realTimeSpeedUpFactor > 0.0) {
        sprintf(outString, "%02d:%02d:%02d.%03d (1/%gx)", hrs, min, sec, (int)std::floor(time * 1000),1. / realTimeSpeedUpFactor);
    } else if(realTimeSpeedUpFactor == 0.0) {
        sprintf(outString, "%02d:%02d:%02d.%03d (paused)", hrs, min, sec, (int)std::floor(time * 1000));
    } else {
        sprintf(outString, "%02d:%02d:%02d.%03d (max, ~%dx)", hrs, min, sec, (int)std::floor(time * 1000), ((int)(m_estPlaySpeed/25.+0.5))*25);
    }
    ui->simTime->setText(QString(outString));
}

void ScSimInfoDisplay::updateReactionWheelDisplay(SpacecraftSim *scSim)
{
    int numRw = numElems(scSim->rw);
    QGridLayout *layout = (QGridLayout *)ui->reactionWheelGroup->layout();
    removeExcessItems(layout, numRw);
    for(int i = 0; i < numRw; i++) {
        ReactionWheelDisplay *display = 0;
        if(layout->count() > i) {
            display = (ReactionWheelDisplay *)layout->itemAt(i)->widget();
        } else {
            display = new ReactionWheelDisplay;
            display->setText(tr("RW%1").arg(i + 1));
            layout->addWidget(display, i / m_layoutNumCol, i % m_layoutNumCol);
        }
        display->setTextToolTip(getStateToolTip(scSim->rw[i].state, scSim->rw[i].resetCounter));
        display->setStatusStyle(getStateStyleSheet(scSim->rw[i].state, scSim->rw[i].resetCounter));
        display->setControlPercent(scSim->rw[i].u / scSim->rw[i].maxU * 100.0);
        display->setSpeedPercent(scSim->rw[i].Omega / scSim->rw[i].highSpeed * 100.0);
    }
}

void ScSimInfoDisplay::updateCssDisplay(SpacecraftSim *scSim)
{
    int numCss = numElems(scSim->css);
    QGridLayout *layout = (QGridLayout *)ui->cssGroup->layout();
    removeExcessItems(layout, numCss);
    for(int i = 0; i < numCss; i++) {
        QLabel *label = 0;
        if(layout->count() > i) {
            label = (QLabel *)layout->itemAt(i)->widget();
        } else {
            label = new QLabel;
            label->setText(tr("CSS%1").arg(i + 1));
            label->setMargin(6);
            label->setAlignment(Qt::AlignCenter);
            layout->addWidget(label, i / m_layoutNumCol, i % m_layoutNumCol);
        }
        QString style = "QLabel{color:black;";
        QString toolTip;
        if(scSim->css[i].state == COMPONENT_ON) {
            if(scSim->css[i].directValue > 0.0) {
                style.append(" background-color:green;");
                toolTip.append("Direct");
            } else {
                style.append(" background-color:darkslategray;");
                toolTip.append("No Direct");
            }
            if(scSim->css[i].albedoValue > 0.0) {
                style.append(" border-color:lime; border-width:2px; border-style:solid;}");
                toolTip.append(" + Albedo");
            } else {
                style.append("}");
            }
        } else {
            style.append(" background-color:darkslategray;}");
            toolTip.append("Off");
        }
        label->setStyleSheet(style);
        label->setToolTip(toolTip);
    }
}

void ScSimInfoDisplay::updateTorqueRodDisplay(SpacecraftSim *scSim)
{
    int numTr = numElems(scSim->tr);
    QGridLayout *layout = (QGridLayout *)ui->torqueRodGroup->layout();
    removeExcessItems(layout, numTr);
    for(int i = 0; i < numTr; i++) {
        TorqueRodDisplay *display = 0;
        if(layout->count() > i) {
            display = (TorqueRodDisplay *)layout->itemAt(i)->widget();
        } else {
            display = new TorqueRodDisplay;
            display->setText(tr("TR%1").arg(i + 1));
            layout->addWidget(display, i / m_layoutNumCol, i % m_layoutNumCol);
        }
        display->setTextToolTip(getStateToolTip(scSim->tr[i].state, 0));
        display->setStatusStyle(getStateStyleSheet(scSim->tr[i].state, 0));
        display->setControlPercent(std::min(std::max(scSim->tr[i].current, -100.0), 100.0));
    }
}

void ScSimInfoDisplay::updateRatesDisplay(SpacecraftSim *scSim)
{
    char outString[256] = "";

    sprintf(outString, "%+8.6f", scSim->omega[0]*R2D);
    ui->angRate_b_1->setText(outString);
    sprintf(outString, "%+8.6f", scSim->omega[1]*R2D);
    ui->angRate_b_2->setText(outString);
    sprintf(outString, "%+8.6f", scSim->omega[2]*R2D);
    ui->angRate_b_3->setText(outString);
}

QString ScSimInfoDisplay::getStateToolTip(ComponentState_t componentState, int resetCounter)
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

QString ScSimInfoDisplay::getStateStyleSheet(ComponentState_t componentState, int resetCounter)
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
        output.append(" border-color:red; border-width:2px; border-style:solid;}");
    }
    return output;
}

void ScSimInfoDisplay::updateControlDisplay(SpacecraftSim *scSim)
{
    char outString[256] = "";

    QString style = "QLabel{color:black;";
    switch(scSim->ctrlState) {
        case CMD_ON:
            style.append(" background-color:green;}");
            ui->controlState->setText("On");
            break;
        case CMD_OFF:
        default:
            style.append(" background-color:red;}");
            ui->controlState->setText("Off");
            break;
    }
    ui->controlState->setStyleSheet(style);

    std::stringstream ss;
    ss << enumToString(scSim->adcsState);
    ui->adcsState->setText(QString(ss.str().c_str()));
    QFont f("Arial", 9);
    ui->adcsState->setFont(f);

    QString styleOn = "QLabel{color:green;}";
    style = ui->adcsStateLabel->styleSheet();
    ui->sunPointingError->setStyleSheet(scSim->adcsState == ADCS_SUN_POINTING ? styleOn : style);

    sprintf(outString, "%+7.4f deg", scSim->sunAngle * R2D);
    ui->sunPointingError->setText(outString);

    // Hide everything and then turn on what is desired in switch statement
    ui->attitudePointingError->hide();
    ui->attitudePointingErrorLabel->hide();
    ui->attitudeRateError->hide();
    ui->attitudeRateErrorLabel->hide();
    ui->Status1->hide();
    ui->Status1Label->hide();
    ui->Status2->hide();
    ui->Status2Label->hide();

    switch (scSim->adcsState) {
        case ADCS_HILL_POINTING:
        {
            ui->attitudePointingErrorLabel->show();
            ui->attitudePointingErrorLabel->setText("Hill Att. Error");
            ui->attitudePointingError->show();
            double HillAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_NP)) * R2D;
            sprintf(outString, "%+7.4f deg", HillAngle);
            ui->attitudePointingError->setText(outString);
            ui->attitudePointingError->setStyleSheet(styleOn);

            ui->attitudeRateErrorLabel->show();
            ui->attitudeRateErrorLabel->setText("Hill Rate Error");
            ui->attitudeRateError->show();
            double rateError = v3Norm(scSim->control.omega_BR_NP) * R2D;
            sprintf(outString, "%+7.5f deg/s", rateError);
            ui->attitudeRateError->setText(outString);
            ui->attitudeRateError->setStyleSheet(styleOn);
            break;
        }
        case ADCS_HILL_SPIN:
        {
            ui->attitudePointingErrorLabel->show();
            ui->attitudePointingErrorLabel->setText("Hill-Spin Att. Error");
            ui->attitudePointingError->show();
            double HillAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_NS)) * R2D;
            sprintf(outString, "%+7.4f deg", HillAngle);
            ui->attitudePointingError->setText(outString);
            ui->attitudePointingError->setStyleSheet(styleOn);

            ui->attitudeRateErrorLabel->show();
            ui->attitudeRateErrorLabel->setText("Hill-Spin Rate Error");
            ui->attitudeRateError->show();
            double rateError = v3Norm(scSim->control.omega_BR_NS) * R2D;
            sprintf(outString, "%+7.5f deg/s", rateError);
            ui->attitudeRateError->setText(outString);
            ui->attitudeRateError->setStyleSheet(styleOn);
            break;
        }
        case ADCS_VELOCITY_POINTING:
        {
            ui->attitudePointingErrorLabel->show();
            ui->attitudePointingErrorLabel->setText("Velocity Att. Error");
            ui->attitudePointingError->show();
            double velocityAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_VP)) * R2D;
            sprintf(outString, "%+7.4f deg", velocityAngle);
            ui->attitudePointingError->setText(outString);
            ui->attitudePointingError->setStyleSheet(styleOn);
            ui->attitudeRateError->setStyleSheet(styleOn);

            ui->attitudeRateErrorLabel->show();
            ui->attitudeRateErrorLabel->setText("Velocity Rate Error");
            ui->attitudeRateError->show();
            double rateError = v3Norm(scSim->control.omega_BR_VP) * R2D;
            sprintf(outString, "%+7.5f deg/s", rateError);
            ui->attitudeRateError->setText(outString);
            break;
        }
        case ADCS_VELOCITY_SPIN:
        {
            ui->attitudePointingErrorLabel->show();
            ui->attitudePointingErrorLabel->setText("Vel.-Spin Att. Error");
            ui->attitudePointingError->show();
            double velocityAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_VS)) * R2D;
            sprintf(outString, "%+7.4f deg", velocityAngle);
            ui->attitudePointingError->setText(outString);
            ui->attitudePointingError->setStyleSheet(styleOn);

            ui->attitudeRateErrorLabel->show();
            ui->attitudeRateErrorLabel->setText("Vel.-Spin Rate Error");
            ui->attitudeRateError->show();
            double rateError = v3Norm(scSim->control.omega_BR_VS) * R2D;
            sprintf(outString, "%+7.5f deg/s", rateError);
            ui->attitudeRateError->setText(outString);
            ui->attitudeRateError->setStyleSheet(styleOn);
            break;
        }
        case ADCS_EARTH_POINTING:
        {
            ui->attitudePointingErrorLabel->show();
            ui->attitudePointingErrorLabel->setText("Earth Pointing Error");
            ui->attitudePointingError->show();
            double velocityAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_EP)) * R2D;
            sprintf(outString, "%+7.4f deg", velocityAngle);
            ui->attitudePointingError->setText(outString);
            ui->attitudePointingError->setStyleSheet(styleOn);

            ui->attitudeRateErrorLabel->show();
            ui->attitudeRateErrorLabel->setText("Earth Rate Error");
            ui->attitudeRateError->show();
            double rateError = v3Norm(scSim->control.omega_BR_EP) * R2D;
            sprintf(outString, "%+7.5f deg/s", rateError);
            ui->attitudeRateError->setText(outString);
            ui->attitudeRateError->setStyleSheet(styleOn);

            ui->Status1Label->show();
            ui->Status1Label->setText("Deadband");
            ui->Status1->show();
            style = "QLabel{color:black;";
            if(scSim->control.deadbandStatus == DEADBAND_OFF) {
                style.append(" background-color:gray;}");
                ui->Status1->setText("Off");
            } else if(scSim->control.deadbandStatus == DEADBAND_ON)  {
                style.append(" background-color:green;}");
                ui->Status1->setText("On");
            } else {
                style.append(" background-color:orange;}");
                ui->Status1->setText("No Signal");
            }
            ui->Status1->setStyleSheet(style);


            break;
        }
        case ADCS_THRUSTING:
        case ADCS_INERTIAL3D_POINTING:
        {
            ui->attitudePointingErrorLabel->show();
            ui->attitudePointingErrorLabel->setText("Inertial Pointing Error");
            ui->attitudePointingError->show();
            double velocityAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_IP)) * R2D;
            sprintf(outString, "%+7.4f deg", velocityAngle);
            ui->attitudePointingError->setText(outString);
            ui->attitudePointingError->setStyleSheet(styleOn);

            ui->attitudeRateErrorLabel->show();
            ui->attitudeRateErrorLabel->setText("Inertial Rate Error");
            ui->attitudeRateError->show();
            double rateError = v3Norm(scSim->control.omega_BR_IP) * R2D;
            sprintf(outString, "%+7.5f deg/s", rateError);
            ui->attitudeRateError->setText(outString);
            ui->attitudeRateError->setStyleSheet(styleOn);

            break;
        }
        case ADCS_INERTIAL3D_SPIN:
        {
            ui->attitudePointingErrorLabel->show();
            ui->attitudePointingErrorLabel->setText("Inertial Pointing Error");
            ui->attitudePointingError->show();
            double velocityAngle = 4.0 * atan(v3Norm(scSim->control.sigma_BR_IS)) * R2D;
            sprintf(outString, "%+7.4f deg", velocityAngle);
            ui->attitudePointingError->setText(outString);
            ui->attitudePointingError->setStyleSheet(styleOn);

            ui->attitudeRateErrorLabel->show();
            ui->attitudeRateErrorLabel->setText("Inertial Rate Error");
            ui->attitudeRateError->show();
            double rateError = v3Norm(scSim->control.omega_BR_IS) * R2D;
            sprintf(outString, "%+7.5f deg/s", rateError);
            ui->attitudeRateError->setText(outString);
            ui->attitudeRateError->setStyleSheet(styleOn);

            break;
        }
        case ADCS_RATE_DAMPING:
        {
            ui->attitudeRateErrorLabel->show();
            ui->attitudeRateErrorLabel->setText("Inertial Rate Error");
            ui->attitudeRateError->show();
            double rateError = v3Norm(scSim->control.omega_BR_RD) * R2D;
            sprintf(outString, "%+7.5f deg/s", rateError);
            ui->attitudeRateError->setText(outString);
            ui->attitudeRateError->setStyleSheet(styleOn);

            break;
        }
        case ADCS_SUN_POINTING:
        {
            ui->attitudeRateErrorLabel->show();
            ui->attitudeRateErrorLabel->setText("Rate Error");
            ui->attitudeRateError->show();
            double rateError = v3Norm(scSim->control.omega_BR_SP) * R2D;
            sprintf(outString, "%+7.5f deg/s", rateError);
            ui->attitudeRateError->setText(outString);
            ui->attitudeRateError->setStyleSheet(styleOn);

            ui->Status1Label->show();
            ui->Status1Label->setText("Deadband");
            ui->Status1->show();
            style = "QLabel{color:black;";
            if(scSim->control.deadbandStatus == DEADBAND_OFF) {
                style.append(" background-color:gray;}");
                ui->Status1->setText("Off");
            } else if(scSim->control.deadbandStatus == DEADBAND_ON)  {
                style.append(" background-color:green;}");
                ui->Status1->setText("On");
            } else {
                style.append(" background-color:orange;}");
                ui->Status1->setText("No Signal");
            }
            ui->Status1->setStyleSheet(style);

            break;
        }
        default:
        {
            break;
        }
    }
}