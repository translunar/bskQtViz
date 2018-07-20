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
#include "taminfo.h"
#include "ui_taminfo.h"

TamInfo::TamInfo(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::TamInfo),
    m_minimumColumnWidth(210),
    m_controlBar(new OnePlusMinusPercentBar(this)),
    m_controlBarToolTip(QString ("% Temperature")),
    m_tamLabel(new QLabel(this)),
    m_statusLabel(new QLabel(this))

{
    ui->setupUi(this);
    addAction(ui->actionStatus);
    addAction(ui->actionVerbose);
    ui->actionStatus->setChecked(true);
    ui->actionVerbose->setChecked(true);
    
    ui->magField_b_1_Units->setText(QString(QChar(0xBC,0x03))+"T");
    ui->magField_b_2_Units->setText(QString(QChar(0xBC,0x03))+"T");
    ui->magField_b_3_Units->setText(QString(QChar(0xBC,0x03))+"T");
    
    QVBoxLayout *vBox = new QVBoxLayout;
    // Main Label
    m_tamLabel->setText("TAM");
    m_tamLabel->setMargin(6);
    m_tamLabel->setAlignment(Qt::AlignCenter);
    //m_tamLabel->setMinimumWidth(m_minimumColumnWidth);
    vBox-> addWidget(m_tamLabel);
    m_tamLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tamLabel, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showOnOffStateContextMenu(const QPoint &)));
    // Control Bar
    m_controlBar->getToolTip(&m_controlBarToolTip);
    vBox->addWidget(m_controlBar);
    // Status
    m_statusLabel->setMargin(6);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    vBox->addWidget(m_statusLabel);
    ui->GraphicGroup->setLayout(vBox);
    ui->VerboseGroup->setToolTip("<b>Magnetic Field <br> Magnetometer Frame <br> [microTesla] </b>");
    ui->VerboseGroup->setToolTipDuration(5000);
    
    
}

TamInfo::~TamInfo()
{
    delete ui;
}

void TamInfo::saveSettings(QSettings *settings)
{
    settings->beginGroup("tamInfo");
    settings->setValue("viewStatus", ui->actionStatus->isChecked());
    settings->setValue("viewVerbose", ui->actionVerbose->isChecked());
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}
void TamInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("tamInfo");
    ui->actionStatus->setChecked(settings->value("viewStatus").toBool());
    ui->actionVerbose->setChecked(settings->value("viewVerbose").toBool());
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void TamInfo::showOnOffStateContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(m_tamLabel);
    
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
    
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setStateTAM(QString)));
    menu->exec(m_tamLabel->mapToGlobal(pos));
}

void TamInfo::setStateTAM(QString value)
{
    ComponentState_t state;
    std::stringstream ss(value.toStdString());
    ss >> enumFromString(state);
    emit setStateTAM(state);
}

QString TamInfo::getStateStyleSheet(ComponentState_t componentState, int resetCounter)
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

QString TamInfo::getStateToolTip(ComponentState_t componentState, int resetCounter)
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

void TamInfo::updateTAM(AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager= dynamic_cast<AdcsSimDataManager *>(simDataManager);
    SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
    
//    m_tamLabel->setToolTip(getStateToolTip(scSim->tam.state, scSim->tam.resetCounter));
//    m_tamLabel->setToolTipDuration(2000);
//    m_tamLabel->setStyleSheet(getStateStyleSheet(scSim->tam.state, scSim->tam.resetCounter));
//    
//    QFont f("Arial", 9);
//    QString str;
//    if(scSim->tam.resetCounter==0)
//    {
//        str= "<font color='green'>" + QString::number(scSim->tam.resetCounter) + " RESETS </font>";
//    } else {
//        str="<font color='red'>" + QString::number(scSim->tam.resetCounter) + " RESETS </font>";
//    }
//    QString str2;
//    if (scSim->tam.numRepeat==0)
//    {
//        str2 = "<font color='green'>" + QString::number(scSim->tam.numRepeat) + " LECTURE ERRORS</font>";
//    } else {
//        str2 ="<font color='red'>" + QString::number(scSim->tam.numRepeat) + " LECTURE ERRORS</font>";
//    }
//    m_statusLabel->setText(tr("TEMPERATURE: <b><font size=3> %1 ºC</b></font>").arg(scSim->tam.temperature) +"<br><br>"+ str + "<br><br>" + str2);
//    m_statusLabel->setFont(f);
//    
//    float percent;
//    if (scSim->tam.temperature>=0)
//    {
//        percent = scSim->tam.temperature / 50;
//    } else {
//        percent = scSim->tam.temperature/ 30;
//    }
//    // sc->tam.maxTemp = 50;
//    // sc->tam.minTemp = -30;
//    m_controlBar->setPercent(percent);
//    
//    if (ui->actionStatus->isChecked())
//    {
//        m_statusLabel->setVisible(true);
//    } else {
//        m_statusLabel->setVisible(false);
//    }
//    
//    if (scSim->tam.state == COMPONENT_ON || scSim->tam.state == COMPONENT_START || scSim->tam.state == COMPONENT_TEST )
//    {
//        char outString[256] = "";
//        sprintf(outString, "%+.4f", scSim->tam.B[0]*(1000*1000));
//        ui->magField_b_1->setText(outString);
//        sprintf(outString, "%+.4f", scSim->tam.B[1]*(1000*1000));
//        ui->magField_b_2->setText(outString);
//        sprintf(outString, "%+.4f", scSim->tam.B[2]*(1000*1000));
//        ui->magField_b_3->setText(outString);
//    } else {
//        ui->magField_b_1->setText("0.0000");
//        ui->magField_b_2->setText("0.0000");
//        ui->magField_b_3->setText("0.0000");
//    }
}