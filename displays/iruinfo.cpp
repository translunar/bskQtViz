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
#include "iruinfo.h"
#include "ui_iruinfo.h"

IruInfo::IruInfo(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::IruInfo),
    m_minimumColumnWidth (170),
    m_iruLabel (new QLabel(this)),
    m_statusLabel(new QLabel(this))
{
    ui->setupUi(this);
    addAction(ui->actionStatus);
    addAction(ui->action_AngularRates);
    ui->actionStatus->setChecked(true);
    ui->action_AngularRates->setChecked(true);
    //addAction(ui->action_Bias);
    
    QVBoxLayout *vBox = new QVBoxLayout;
    m_iruLabel->setText("IRU");
    m_iruLabel->setMargin(6);
    m_iruLabel->setAlignment(Qt::AlignCenter);
    m_iruLabel->setMinimumWidth(m_minimumColumnWidth);
    vBox-> addWidget(m_iruLabel);
    
    m_iruLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_iruLabel, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showOnOffStateContextMenu(const QPoint &)));
    
    m_statusLabel->setMargin(6);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    vBox->addWidget(m_statusLabel);
    
    ui->MainGroup->setLayout(vBox);
    ui->biasGroup->setToolTip(" <b> Stability and Drift Bias <br> Gyro Frame</b>");
    ui->biasGroup->setToolTipDuration(5000);
    ui->angularRatesGroup->setToolTip("<b>Angular Rates <br> Gyro Frame");
    ui->angularRatesGroup->setToolTipDuration(5000);
}

IruInfo::~IruInfo()
{
    delete ui;
}

void IruInfo::saveSettings(QSettings *settings)
{
    settings->beginGroup("iruInfo");
    settings->setValue("viewStatus", ui->actionStatus->isChecked());
    settings->setValue("viewRates", ui->action_AngularRates->isChecked());
    settings->setValue("viewBias", ui->action_Bias->isChecked());
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}
void IruInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("iruInfo");
    ui->actionStatus->setChecked(settings->value("viewStatus").toBool());
    ui->action_AngularRates->setChecked(settings->value("viewRates").toBool());
    ui->action_Bias->setChecked(settings->value("viewBias").toBool());
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void IruInfo::showOnOffStateContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(m_iruLabel);
    
    QSignalMapper *signalMapper = new QSignalMapper(menu);
    for(int i = 0; i < MAX_COMPONENT_STATE; i++) {
        QAction *action = new QAction(menu);
        std::stringstream ss;
        ss << enumToString((ComponentState_t)i);
        action->setText(QString(ss.str().c_str()));
        connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
        signalMapper->setMapping(action, action->text());
        menu->addAction(action);
    }
    
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setStateIRU(QString)));
    menu->exec(m_iruLabel->mapToGlobal(pos));
}

void IruInfo::setStateIRU(QString value)
{
    ComponentState_t state;
    std::stringstream ss(value.toStdString());
    ss >> enumFromString(state);
    emit setStateIRU(state);
}

QString IruInfo::getStateToolTip(ComponentState_t componentState, int resetCounter)
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

QString IruInfo::getStateStyleSheet(ComponentState_t componentState, int resetCounter)
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

void IruInfo::updateIRU(AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager= dynamic_cast<AdcsSimDataManager *>(simDataManager);
    SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
    
    // Main Label
    m_iruLabel->setToolTip(getStateToolTip(scSim->iru.state, scSim->iru.resetCounter));
    m_iruLabel->setToolTipDuration(2000);
    m_iruLabel->setStyleSheet(getStateStyleSheet(scSim->iru.state, scSim->iru.resetCounter));
    
    // Reset Status
    QFont f("Arial", 9);
    QString str;
    if(scSim->iru.resetCounter==0)
    {
        str= "<font color='green'>"+QString::number(scSim->iru.resetCounter)+ " RESETS </font>";
    } else {
        str="<font color='red'>"+QString::number(scSim->iru.resetCounter)+ " RESETS </font>";
    }
    QString str2;
    if (scSim->iru.crcError==0)
    {
        str2= "<font color='green'>"+QString::number(scSim->iru.crcError)+ " TRANSMISSION ERRORS </font>";
    } else {
        str2= "<font color='red'>"+QString::number(scSim->iru.crcError)+ " TRANSMISSION ERRORS </font>";
    }
    m_statusLabel->setText(str + "<br><br>" + str2);
    m_statusLabel->setFont(f);
    
    if (ui->actionStatus->isChecked())
    {
        m_statusLabel->setVisible(true);
    } else {
        m_statusLabel->setVisible(false);
    }
    
    // Info Labels
    /*This if-else condition is temporary. Values should be updated according to IRU ComponentState in the Sim*/
    if (scSim->iru.state == COMPONENT_ON || scSim->iru.state == COMPONENT_START || scSim->iru.state == COMPONENT_TEST )
    {
        char outString[256] = "";
        sprintf(outString, "%+8.6f", scSim->iru.omega[0]*R2D);
        ui->angRate_b_1->setText(outString);
        sprintf(outString, "%+8.6f", scSim->iru.omega[1]*R2D);
        ui->angRate_b_2->setText(outString);
        sprintf(outString, "%+8.6f", scSim->iru.omega[2]*R2D);
        ui->angRate_b_3->setText(outString);
    }
    else {
        ui->angRate_b_1->setText("0.0");
        ui->angRate_b_2->setText("0.0");
        ui->angRate_b_3->setText("0.0");
    }
    // Noise is  detected regardless of the state of the component. OK?
    char outString[256] = "";
    sprintf(outString, "%+8.6f", scSim->iru.bias[0]*R2D);
    ui->bias_b_1->setText(outString);
    sprintf(outString, "%+8.6f", scSim->iru.bias[1]*R2D);
    ui->bias_b_2->setText(outString);
    sprintf(outString, "%+8.6f", scSim->iru.bias[2]*R2D);
    ui->bias_b_3->setText(outString);
    
}