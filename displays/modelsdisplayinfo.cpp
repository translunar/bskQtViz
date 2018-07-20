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
#include "modelsdisplayinfo.h"
#include "ui_modelsdisplayinfo.h"
#include "enumConversions.h"

#include <QMenu>
#include <QAction>
#include <QSignalMapper>

ModelsDisplayInfo::ModelsDisplayInfo(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ModelsDisplayInfo)
{
    ui->setupUi(this);
    
    //addAction(ui->actionEnabled_Models);
    //addAction(ui->actionSelec_Models);
    
    // list to store all labels from modelGrup
    list.insert(0, ui->gGradient_1);
    list.insert(1, ui->rwJitter_1);
    list.insert(2, ui->gPerturb_1);
    list.insert(3, ui->atmDrag_1);
    list.insert(4, ui->srp_1);
    
    // Setting up context menu for each label in the list
    for (int i=0; i< list.size(); i++)
    {
        list[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(list[i], SIGNAL(customContextMenuRequested(const QPoint &)),
                this, SLOT(showModelContextMenu(const QPoint &)));
    }
    ui->albedo->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->albedo, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showAlbedoContextMenu(const QPoint &)));
    ui->earthMagField->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->earthMagField, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showEarthMagFieldContextMenu(const QPoint &)));
    
}


ModelsDisplayInfo::~ModelsDisplayInfo()
{
    delete ui;
}

void ModelsDisplayInfo::showModelContextMenu(const QPoint &pos)
{
    QLabel *selectedLabel= dynamic_cast<QLabel*>(QObject::sender());
    for (int i=0; i<list.size(); i++)
    {
        if (list[i]->text()==selectedLabel->text())
        {
            m_selecLabel = i;
        }
    }
    if(m_selecLabel!=-1){
        QMenu *menu = new QMenu(list[m_selecLabel]);
        QSignalMapper *signalMapper = new QSignalMapper(menu);
        // Adding desired actions
        QAction *actionON = new QAction(menu);
        actionON->setText(QString("Enable "+ list[m_selecLabel]->text()));
        connect(actionON, SIGNAL(triggered()), signalMapper, SLOT(map()));
        signalMapper->setMapping(actionON, actionON->text());
        menu->addAction(actionON);
        QAction *actionOFF = new QAction(menu);
        actionOFF->setText(QString("Disable "+ list[m_selecLabel]->text()));
        connect(actionOFF, SIGNAL(triggered()), signalMapper, SLOT(map()));
        signalMapper->setMapping(actionOFF, actionOFF->text());
        menu->addAction(actionOFF);
        connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setMode(QString)));
        menu->exec(list[m_selecLabel]->mapToGlobal(pos));
    }
}
void ModelsDisplayInfo::showAlbedoContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(ui->albedo);
    
    QSignalMapper *signalMapper = new QSignalMapper(menu);
    for(int i = 0; i < MAX_ALBEDO_MODEL; i++) {
        QAction *action = new QAction(menu);
        std::stringstream ss;
        ss << enumToString((AlbedoModel_t)i);
        action->setText(QString(ss.str().c_str()));
        connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
        signalMapper->setMapping(action, action->text());
        menu->addAction(action);
    }
    
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setAlbedoModel(QString)));
    menu->exec(ui->albedo->mapToGlobal(pos));
}
void ModelsDisplayInfo::showEarthMagFieldContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(ui->earthMagField);
    
    QSignalMapper *signalMapper = new QSignalMapper(menu);
    for(int i = 0; i < MAX_MAGFIELD_MODEL; i++) {
        QAction *action = new QAction(menu);
        std::stringstream ss;
        ss << enumToString((EarthMagFieldModel_t)i);
        action->setText(QString(ss.str().c_str()));
        connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
        signalMapper->setMapping(action, action->text());
        menu->addAction(action);
    }
    
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(setEarthMagField(QString)));
    menu->exec(ui->earthMagField->mapToGlobal(pos));
}

void ModelsDisplayInfo::setAlbedoModel(QString value)
{
    AlbedoModel_t model;
    std::stringstream ss(value.toStdString());
    ss >> enumFromString(model);
    emit setAlbedoModel(model);
}
void ModelsDisplayInfo::setEarthMagField(QString value)
{
    EarthMagFieldModel_t model;
    std::stringstream ss(value.toStdString());
    ss >> enumFromString(model);
    emit setEarthMagField(model);
}
void ModelsDisplayInfo::setMode(QString str)
{
    int mode=-1;
    int model;
    
    for (int i=0; i<list.size();i++)
    {
        if(str==QString("Enable "+ list[i]->text()))
        {
            model = i;
            mode = 1;
        }
        else if (str==QString("Disable "+ list[i]->text()))
        {
            model = i;
            mode = 0;
        }
    }
    if (mode ==0 || mode==1)
    {
        switch (model)
        {
            case 0:
                emit setGravityGradientTorque(mode);
                break;
            case 1:
                emit setReactionWheelJitter(mode);
                break;
            case 2:
                emit setGravityPerturbModel(mode);
                break;
            case 3:
                emit setAtmosDragModel(mode);
                break;
            case 4:
                emit setSrpModel(mode);
                break;
            default:
                break;
        }
    }
}



void ModelsDisplayInfo::getModelInfo(AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager = dynamic_cast<AdcsSimDataManager *>(simDataManager);
    if(adcsSimDataManager) {
        SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
        
        // ENABLED MODELS
        QString styleON = "QLabel {  border-color:silver; border-width:2px; border-style:solid; } ";
        QString toolTipON = "Model Enabled";
        QString styleOFF = "QLabel { color:grey}";
        QString toolTipOFF = "Model Disabled";
        scList.insert(0, scSim->useGravityGradientTorque);
        scList.insert(1, scSim->useReactionWheelJitter);
        scList.insert(2, scSim->useGravityPerturbModel);
        scList.insert(3, scSim->useAtmosDragModel);
        scList.insert(4, scSim->useSrpModel);
        for (int i=0; i<list.size(); i++)
        {
            if(scList[i]==1){
                list[i]->setStyleSheet(styleON);
                list[i]->setToolTip(toolTipON);
            } else{
                list[i]->setStyleSheet(styleOFF);
                list[i]->setToolTip(toolTipOFF);
            }
            list[i]->setToolTipDuration(2000);
        }
        
        
        // SELECTED MODELS
        std::stringstream ss;
        std::stringstream ss2;
        QFont f("Arial", 8);
        ss << enumToString(scSim->albedoModel);
        ui->albedo->setText(QString(ss.str().c_str()));
        ui->albedo->setToolTip("Albedo model");
        ui->albedo->setToolTipDuration(2000);
        ui->albedo->setFont(f);
        
        ss2 << enumToString(scSim->earthMagFieldModel);
        ui->earthMagField->setText(QString(ss2.str().c_str()));
        ui->earthMagField->setToolTip("Earth Magnetic Field model");
        ui->earthMagField->setToolTipDuration(2000);
        ui->earthMagField->setFont(f);
        
    }
}

void ModelsDisplayInfo::saveSettings(QSettings *settings)
{
    settings->beginGroup("scSimInfoDisplay");
    //settings->setValue("viewEnabled", ui->actionEnabled_Models->isChecked());
    //settings->setValue("viewSelected", ui->actionSelec_Models->isChecked());
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}

void ModelsDisplayInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("scSimInfoDisplay");
    //ui->actionEnabled_Models->setChecked(settings->value("viewEnabled").toBool());
    //ui->actionSelec_Models->setChecked(settings->value("viewSelected").toBool());
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}






