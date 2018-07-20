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
#ifndef THRUSTERSINFO_H
#define THRUSTERSINFO_H

#include <QDockWidget>
#include <QSettings>
#include <QLabel>
#include <QList>
#include <QProgressBar>
#include <QLineEdit>
#include <QPushButton>
#include "adcssimdatamanager.h"
#include "oneplusminuspercentbar.h"
#include "graphics.h"

namespace Ui {
    class ThrustersInfo;
}

class ThrustersInfo : public QDockWidget
{
    Q_OBJECT

public:
    explicit ThrustersInfo(QWidget *parent = 0, AdcsSimDataManager *simData = NULL);
    ~ThrustersInfo();
    
    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    void updateThrusters(AdcsSimDataManager *simDataManager);

public:
    QList <QLabel*> acsList1;
    QList <OnOffCircle*> acsList2;
    QList <OnOffCircle*> dvList2;
    //QList <QProgressBar*> listOfOne;
    QList <QLabel*> legendLabelList;
    AdcsSimDataManager *m_simDataManager;
    
signals:
    void setNewManeuver(double, double);

public slots:
    void showLevelContextMenu(const QPoint &);
    void setNewManeuver();
    void configureLayout();

private:
    void resetUiElements();

private:
    Ui::ThrustersInfo *ui;
    int m_layoutNumCol;
    int m_layoutNumColDV;
    bool m_bool;
    int m_startTime;
    int m_durationTime;
    bool m_buttonClicked;
    bool m_thrustersON;
    int m_numUpdates;
    QLineEdit *m_timeUntilThrustersAreTurnOn;
    QLineEdit *m_newDurationTime;
    QValidator *m_durationTimeValidator;
    int maxTimeDuration;
    QPushButton *m_button;
    QLabel *m_label;
    QProgressBar *m_legend;
    
};

#endif // THRUSTERSINFO_H
