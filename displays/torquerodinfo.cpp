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
#include "torquerodinfo.h"
#include "ui_torquerodinfo.h"
#include "math.h"
#include "enumconversions.h"

TorqueRodInfo::TorqueRodInfo(QWidget *parent) :
    QDockWidget(parent),
    m_legend(new Legend),
    ui(new Ui::TorqueRodInfo),
    m_layoutNumCol(3),
    m_selecLabel(-1),
    m_minimumColWidth(30),
    m_tipColor("% Dipole Moment")

{
    ui->setupUi(this);
    addAction(ui->actionVerbose);
    ui->actionVerbose->setChecked(true);
    
    ui->PowerPlotGroup->hide();
    //addAction(ui->actionPowerPlot);
    //ui->actionPowerPlot->setChecked(true);
    
    QGridLayout *layout = new QGridLayout;
    QGridLayout *infoLayout = new QGridLayout;
    QGridLayout *barLayout = new QGridLayout;
    
    for (int i=0; i< NUM_TR; i++)
    {
        // TR Graphic Group
        QVBoxLayout *vBox = new QVBoxLayout;
        
        QLabel *label = new QLabel;
        label->setText(tr("TR%1").arg(i + 1));
        label->setMargin(6);
        label->setAlignment(Qt::AlignCenter);
        vBox-> addWidget(label);
        list.append(label);
        list[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(list[i], SIGNAL(customContextMenuRequested(const QPoint &)),
                this, SLOT(showOnOffStateContextMenu(const QPoint &)));
        
        OnePlusMinusPercentBar *m_controlBar = new OnePlusMinusPercentBar;
        m_controlBar->getToolTip(&m_tipColor);
        vBox->addWidget(m_controlBar);
        barList.append(m_controlBar);
        
        layout->addLayout(vBox, i / m_layoutNumCol, i % m_layoutNumCol);
        
        // TR Verbose Group
        QVBoxLayout *vInfoBox = new QVBoxLayout;
        QLabel *infoLabel2 = new QLabel;
        QLabel *infoLabel3 = new QLabel;
        
        infoLabel2->setMargin(6);
        infoLabel2->setAlignment(Qt::AlignCenter);
        infoLabel3->setMargin(6);
        infoLabel3->setAlignment(Qt::AlignCenter);
        
        vInfoBox->addWidget(infoLabel2);
        list2.append(infoLabel2);
        vInfoBox->addWidget(infoLabel3);
        list3.append(infoLabel3);
        infoLayout->addLayout(vInfoBox, i / m_layoutNumCol, i % m_layoutNumCol);
        infoLayout->setColumnStretch(i,0);
        
        /* TR Power Group
        QVBoxLayout *powerBox = new QVBoxLayout;
        QProgressBar *powerBar = new QProgressBar;
        powerBar->setMinimum(0);
        powerBar->setMinimumWidth(55);
        powerList.append(powerBar);
        powerBox->addWidget(powerBar);
        barLayout->addLayout(powerBox, i / m_layoutNumCol, i % m_layoutNumCol);*/
    }
    ui->GraphicGroup->setLayout(layout);
    ui->VerboseGroupNew->setLayout(infoLayout);
    ui->PowerPlotGroup->setLayout(barLayout);
    
    
    QHBoxLayout *hBox = new QHBoxLayout;
    m_legend->getLegendInfo("<font size=3> Dipole Moment <br>[ A · m^2 ] </font>");
    hBox->addWidget(m_legend);
    ui->LegendGroup->setLayout(hBox);
    ui->LegendGroup->hide();
    
}

TorqueRodInfo::~TorqueRodInfo()
{
    delete ui;
}

void TorqueRodInfo::saveSettings(QSettings *settings)
{
    settings->beginGroup("torqueRodInfo");
    settings->setValue("viewVerbose", ui->actionVerbose->isChecked());
    settings->setValue("viewPowerPlot", ui->actionPowerPlot->isChecked());
    
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}

void TorqueRodInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("torqueRodInfo");
    ui->actionVerbose->setChecked(settings->value("viewVerbose").toBool());
    ui->actionPowerPlot->setChecked(settings->value("viewPowerPlot").toBool());
    
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void TorqueRodInfo::setOnOffLegendTR(bool b)
{
    if (b==false)
    {
        ui->LegendGroup->hide();
    } else {
        ui->LegendGroup->show();
    }
}

void TorqueRodInfo::showOnOffStateContextMenu(const QPoint &pos)
{
    QLabel *selectedLabel= dynamic_cast<QLabel*>(QObject::sender());
    
    for (int i=0; i<NUM_TR; i++)
    {
        if (list[i]->text()==selectedLabel->text())
        {
            m_selecLabel = i;
        }
    }
    
    if(m_selecLabel!=-1){
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
        connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setStateTR(QString)));
        menu->exec(list[m_selecLabel]->mapToGlobal(pos));
    }
}

void TorqueRodInfo::setStateTR(QString value)
{
    ComponentState_t state;
    std::stringstream ss(value.toStdString());
    ss >> enumFromString(state);
    emit setStateTR(state, m_selecLabel);
}


void TorqueRodInfo::updateTorqueRods(AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager = dynamic_cast<AdcsSimDataManager *>(simDataManager);
    if(adcsSimDataManager)
    {
        SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
        
        char outString[256] = " ";
        
        for(int i=0; i<NUM_TR; i++){
            
            /* TR Graphic Group */
            list[i]->setToolTip(getStateToolTip(scSim->tr[i].state));
            list[i]->setToolTipDuration(2000);
            list[i]->setStyleSheet(getStateStyleSheet(scSim->tr[i].state));
            //barList[i]->setPercent(std::min(std::max(scSim->tr[i].current, -100.0), 100.0));
            barList[i]->setPercent(scSim->tr[i].u / 6.0 * 100);

            /* TR Verbose Group*/
            QFont font("Arial",9);
            sprintf(outString, "%.2f", (scSim->tr[i].u));
            if (scSim->tr[i].u>=0){list2[i]->setText(tr("+%1").arg(outString));}
            else {list2[i]->setText(tr("%1").arg(outString));}
            list2[i]->setFont(font);
            list2[i]->setToolTip(tr("Dipole Moment TR%1 [ A · m^2 ]").arg(i+1));
            list2[i]->setToolTipDuration(4000);
            
            
            
            double value = fabs(scSim->tr[i].u / 6.0 * scSim->trPowerMax);
            // sc->control.maxTRDipoleMoment = 6.0
            sprintf(outString, "%.2f",value);
            list3[i]->setText(tr("%1").arg(outString));
            list3[i]->setFont(font);
            list3[i]->setToolTip(tr("Power Consumption TR%1 [ W ]").arg(i+1));
            list3[i]->setToolTipDuration(3000);
            
            /* TR Power Group
            powerList[i]->setMaximum(scSim->trPowerMax);
            powerList[i]->setValue(value);
            powerList[i]->setToolTip( tr("TR%1: ").arg(i+1) + tr("%1% Pmax").arg(int(value / (scSim->trPowerMax) * 100)));
            powerList[i]->setToolTipDuration(3000);
            
            if (value / (scSim->trPowerMax) * 100 >= 80)
            {
                powerList[i]->setStyleSheet("QProgressBar::chunk{background: #fb525a; width: 10px; margin: 0.5px;} QProgressBar{ border: 2px solid grey; border-radius: 7px;}");
            } else {
                powerList[i]->setStyleSheet("QProgressBar::chunk{background: #bdf1a8; width: 10px; margin: 0.5px;} QProgressBar{ border: 2px solid grey; border-radius: 7px;}");
            }*/
        }

    }
}

QString TorqueRodInfo::getStateToolTip(ComponentState_t componentState)
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
    return output;
}

QString TorqueRodInfo::getStateStyleSheet(ComponentState_t componentState)
{
    QString output = "QLabel{color:black;";
    switch(componentState) {
        case COMPONENT_OFF:
            output.append(" background-color:darkslategray};");
            break;
        case COMPONENT_START:
            output.append(" background-color:greenyellow};");
            break;
        case COMPONENT_ON:
            output.append(" background-color:green;}");
            break;
        case COMPONENT_FAULT:
            output.append(" background-color:red;}");
            break;
        case COMPONENT_TEST:
            output.append(" background-color:orange;}");
            break;
        default:
            output.append(" background-color:white;}");
            break;
    }
    return output;
}

