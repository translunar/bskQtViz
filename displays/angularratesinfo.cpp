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
#include "angularratesinfo.h"
#include "ui_angularratesinfo.h"

#include <QMenu>
#include <QAction>
#include <QSignalMapper>

AngularRatesInfo::AngularRatesInfo(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::AngularRatesInfo),
    m_minimumColumnWidth (80)
{
    ui->setupUi(this);
    ui->angularRatesGroup->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->angularRatesGroup, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showAngularRatesGroupContextMenu(const QPoint &)));
}

AngularRatesInfo::~AngularRatesInfo()
{
    delete ui;
}

void AngularRatesInfo:: saveSettings(QSettings *settings)
{
    settings->beginGroup("angularRatesInfo");
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}
void AngularRatesInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("angularRatesInfo");
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void AngularRatesInfo::updateAngularRates(AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager= dynamic_cast<AdcsSimDataManager *>(simDataManager);
    if (adcsSimDataManager)
    {
        SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
        char outString[256] = "";
        sprintf(outString, "%+8.6f", scSim->omega[0]*R2D);
        ui->angRate_b_1->setText(outString);
        sprintf(outString, "%+8.6f", scSim->omega[1]*R2D);
        ui->angRate_b_2->setText(outString);
        ui->angRate_b_2->setMinimumWidth(m_minimumColumnWidth);
        sprintf(outString, "%+8.6f", scSim->omega[2]*R2D);
        ui->angRate_b_3->setText(outString);
    }
}

void AngularRatesInfo::showAngularRatesGroupContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(ui->angularRatesGroup);
    
    QAction *action = new QAction(tr("Perturb rates"), menu);
    connect(action, SIGNAL(triggered()), this, SIGNAL(perturbRates()));
    menu->addAction(action);
    
    menu->exec(ui->angularRatesGroup->mapToGlobal(pos));
}

