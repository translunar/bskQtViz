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
#ifndef ORBITELEMENTSINFO_H
#define ORBITELEMENTSINFO_H

#include "astroConstants.h"
#include "adcssimdatamanager.h"
#include "graphics.h"
#include <time.h>

#include <QDockWidget>
#include <QGridLayout>
#include <QSettings>
#include <QLabel>

namespace Ui
{
    class OrbitElementsInfo;
}

class OrbitElementsInfo : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit OrbitElementsInfo(QWidget *parent = 0);
    ~OrbitElementsInfo();
    
    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    
    void updateOrbitElements(AdcsSimDataManager *simDataManager);
public slots:
    void showContextMenu(const QPoint &);
private:
    Ui::OrbitElementsInfo *ui;
    double b;
    double apoapsis;
    double periapsis;
    double M;
    double E;
    double B;
    Orbit2D *m_orbit;
    OrbitalPeriod *m_fraction;
    QLabel *m_Tlabel;
    int m_counter;
};

#endif // ORBITELEMENTSINFO_H
