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
#include "podinfo.h"
#include "ui_podinfo.h"

PodInfo::PodInfo(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::PodInfo),
    m_minimumColumnWidth (210),
    m_podLabel(new QLabel(this)),
    m_controlBar(new ThreePlusMinusPercentBars(this)),
    m_statusLabel (new QLabel(this))
{
    ui->setupUi(this);
    addAction(ui->actionStatus);
    ui->actionStatus->setChecked(true);
    
    QVBoxLayout *vBox = new QVBoxLayout;
    
    // Main Label
    m_podLabel->setText("POD");
    m_podLabel->setMargin(6);
    m_podLabel->setAlignment(Qt::AlignCenter);
    //m_podLabel->setMinimumWidth(m_minimumColumnWidth);
    vBox-> addWidget(m_podLabel);
    m_podLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_podLabel, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showOnOffStateContextMenu(const QPoint &)));
    
    // Control Bar
    vBox->addWidget(m_controlBar);
    
    // Status
    m_statusLabel->setMargin(6);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    vBox->addWidget(m_statusLabel);
    
    ui->GraphicGroup->setLayout(vBox);
    setToolTip("Precise Orbit Determination Systen");
    setToolTipDuration(2000);
}

PodInfo::~PodInfo()
{
    delete ui;
}

void PodInfo::saveSettings(QSettings *settings)
{
    settings->beginGroup("podInfo");
    settings->setValue("viewStatus", ui->actionStatus->isChecked());
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}
void PodInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("podInfo");
    ui->actionStatus->setChecked(settings->value("viewStatus").toBool());
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void PodInfo::showOnOffStateContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(m_podLabel);
    
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
    
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setStatePOD(QString)));
    menu->exec(m_podLabel->mapToGlobal(pos));
}

void PodInfo::setStatePOD(QString value)
{
    ComponentState_t state;
    std::stringstream ss(value.toStdString());
    ss >> enumFromString(state);
    emit setStatePOD(state);
}

QString PodInfo::getStateStyleSheet(ComponentState_t componentState, int resetCounter)
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

QString PodInfo::getStateToolTip(ComponentState_t componentState, int resetCounter)
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

void PodInfo::updatePOD(AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager= dynamic_cast<AdcsSimDataManager *>(simDataManager);
    SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
    
    m_podLabel->setToolTip(getStateToolTip(scSim->pod.state, scSim->pod.resetCounter));
    m_podLabel->setToolTipDuration(2000);
    m_podLabel->setStyleSheet(getStateStyleSheet(scSim->pod.state, scSim->pod.resetCounter));
    
    QFont f("Arial", 10);
    QString str;
    if(scSim->pod.resetCounter==0)
    {
        str= "<font color='green'>"+QString::number(scSim->pod.resetCounter)+ " RESETS </font>";
    } else {
        str="<font color='red'>"+QString::number(scSim->pod.resetCounter)+ " RESETS </font>";
    }
    //"<i><font color='black'>Precise Orbit Determination</font></i><br><br>"+
    m_statusLabel->setText(tr("GPS FREQUENCY: <b><font size=3> %1 HZ</b></font>").arg(scSim->pod.gpsPosDataHz)+tr("<br><br>POD START UP TIME: <b><font size=3>%1 SEC</font></b>").arg(scSim->pod.startUpTime) + "<br><br>" + str);
    m_statusLabel->setFont(f);

    if (ui->actionStatus->isChecked())
    {
        m_statusLabel->setVisible(true);
    } else {
        //m_statusLabel->setText("<i><font color='black'>Precise Orbit Determination</font></i>");
        m_statusLabel->setVisible(false);
    }
    if (ui->actionGraphic->isChecked())
    {
        m_controlBar->setVisible(true);
    } else {
        m_controlBar->setVisible(false);
    }
}
