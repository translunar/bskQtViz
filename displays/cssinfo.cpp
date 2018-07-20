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
#include "cssinfo.h"
#include "ui_cssinfo.h"
#include "enumConversions.h"
#include "SpacecraftSimDefinitions.h"
extern "C" {
#include "spacecraftDefinitions.h"
}
CssInfo::CssInfo(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::CssInfo),
    m_layoutNumCol(4),
    percentDirect(0.0),
    m_selecLabel(-1),
    m_minimumColWidth(40)

{
    ui->setupUi(this);
    addAction(ui->actionVerbose);
    ui->actionVerbose->setChecked(true);
    QGridLayout *layout = new QGridLayout;
    for (int i=0; i< NUM_CSS; i++)
    {
        QLabel *label= new QLabel;
        label->setText(tr("css%1").arg(i + 1));
        label->setMargin(6);
        label->setAlignment(Qt::AlignCenter);
        //label->setMinimumWidth(m_minimumColWidth);
        int row= i/m_layoutNumCol;
        int column =i % m_layoutNumCol;
        layout->addWidget(label, row*2, column);
        list.append(label);
        list[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(list[i],SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showOnOffStateContextMenu(const QPoint &)));
        
        QLabel *info = new QLabel;
        info->setMargin(6);
        info->setAlignment(Qt::AlignCenter);
        info->hide();
        layout->addWidget(info, row*2+1,column);
        infoList.append(info);
        infoList[i]->hide();
    }
    ui->cssGroup->setLayout(layout);
    }

CssInfo::~CssInfo()
{
    delete ui;
}

void CssInfo::showOnOffStateContextMenu(const QPoint &pos)
{
    QLabel *selectedLabel= dynamic_cast<QLabel*>(QObject::sender());
    for (int i=0; i<NUM_CSS; i++)
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
            if (i==COMPONENT_ON || i==COMPONENT_FAULT)
            {
                QAction *action = new QAction(menu);
                std::stringstream ss;
                ss << enumToString((ComponentState_t)i);
                action->setText(QString(ss.str().c_str()));
                connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
                signalMapper->setMapping(action, action->text());
                menu->addAction(action);
            }
        }
        connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setStateCSS(QString)));
        menu->exec(list[m_selecLabel]->mapToGlobal(pos));
    }
}
void CssInfo::setStateCSS(QString value)
{
    ComponentState_t state;
    std::stringstream ss(value.toStdString());
        ss >> enumFromString(state);
        emit setStateCSS(state, m_selecLabel);
}
void CssInfo::saveSettings(QSettings *settings)
{
    settings->beginGroup("cssInfo");
    settings->setValue("viewVerbose",ui->actionVerbose->isChecked());
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}

void CssInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("cssInfo");
    ui->actionVerbose->setChecked(settings->value("viewVerbose").toBool());
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void CssInfo::updateCSS( AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager= dynamic_cast<AdcsSimDataManager *>(simDataManager);
    if (adcsSimDataManager)
    {
        SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
       
        for(int i = 0; i < list.size(); i++) {
            
            QString style = "QLabel{color:black;";
            QString toolTip;
            if(scSim->css[i].state == COMPONENT_ON) {
                if(scSim->css[i].directValue > 0.0) {
                    //style.append(" border-color: color: green; border-width:2px; border-style:solid; background-color: green;");
                    style.append(" background-color: green;");
                    toolTip.append("<font color='#0d4202'><b>Direct</b></font>");
                } else {
                    style.append(" background-color: darkslategray;");
                    toolTip.append("No Direct");
                }
                if(scSim->css[i].albedoValue > 0.0) {
                    style.append(" border-color:lime; border-width:2px; border-style:solid;}");
                    toolTip.append(" + <font color='lime'>Albedo</font>");
                } else {
                    style.append("border-color: darkslategray; border-width:2px; border-style:solid;}");
                }
            } else if(scSim->css[i].state == COMPONENT_FAULT){
                style.append("background-color:darkslategray; border-color: red; border-width:2px; border-style:solid;}");
                toolTip.append("Off (fault)");
            } else {
                style.append("background-color:darkslategray;}");
                toolTip.append("Off");
            }
            
            list[i]->setToolTip(toolTip);
            list[i]->setToolTipDuration(2000);
            list[i]->setStyleSheet(style);
            
            char outString[256] = " ";
                sprintf(outString, "%.3f", scSim->css[i].sensedValue);
                if (scSim->css[i].directValue==0 && scSim->css[i].albedoValue!=0)
                {
                        infoList[i]->setText("<font color='lime' size=2> "+ QString(outString)+"</font>");
                        infoList[i]->setToolTip( "<b><font color='lime'>Albedo</font><br>[W/m^2]</b>");
                } else if (scSim->css[i].directValue!=0 && scSim->css[i].albedoValue==0) {
                    infoList[i]->setText("<font color='green' size=2> "+ QString(outString)+"</font>");
                    infoList[i]->setToolTip( "<b><font color='#0d4202'> Direct </font><br>[W/m^2]</b>");
                } else {
                    infoList[i]->setText("<font size=2> "+QString(outString)+"</font>");
                    infoList[i]->setToolTip( "<b>Total sensed value<br><font color='#0d4202'>Direct</font>+<font color='lime'>Albedo</font><br>[W/m^2]</b>");
                }

            infoList[i]->setToolTipDuration(4500);
            if(ui->actionVerbose->isChecked())
            {
                infoList[i]->show();
            }
            else {infoList[i]->hide();}
        }
    }
}
 