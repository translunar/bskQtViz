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
#include "simtimeinfo.h"
#include "ui_simtimeinfo.h"

#include <time.h>

SimTimeInfo::SimTimeInfo(QWidget *parent) :
    QDockWidget(parent)
    , ui(new Ui::SimTimeInfo)
    , m_estPlaySpeed(0.0)
{
    ui->setupUi(this);
}

SimTimeInfo::~SimTimeInfo()
{
    delete ui;
}


void SimTimeInfo::setEstPlaySpeed(double value)
{
    m_estPlaySpeed = value;
}

void SimTimeInfo:: saveSettings(QSettings *settings)
{
    settings->beginGroup("simTimeInfo");
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}
void SimTimeInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("simTimeInfo");
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void SimTimeInfo::updateSimTime(AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager = dynamic_cast<AdcsSimDataManager *>(simDataManager);
    if (adcsSimDataManager)
    {
        SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
        double JD = scSim->spiceTime.julianDateCurrent;
        char outString[256] = "";
        sprintf(outString, "Current Julian Date: \n %.4f", JD);
        ui->simTime->setText(QString(outString));

        double time = simDataManager->getSimTime();
        double realTimeSpeedUpFactor = scSim->realTimeSpeedUpFactor;
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
    }
}
