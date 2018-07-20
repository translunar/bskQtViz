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
#ifndef TORQUERODINFO_H
#define TORQUERODINFO_H

#include "adcssimdatamanager.h"
#include "graphics.h"

#include <QDockWidget>
#include <QList>
#include <QSettings>
#include <QLabel>
#include <QProgressBar>
#include <QMenu>
#include <QSignalMapper>

namespace Ui {
class TorqueRodInfo;
}

class TorqueRodInfo : public QDockWidget
{
    Q_OBJECT
    
    QList <QLabel*> list;
    QList <OnePlusMinusPercentBar*> barList;
    QList <QLabel*> list1;
    QList <QLabel*> list2;
    QList <QLabel*> list3;
    QList <QProgressBar*> powerList;
    
public:
    explicit TorqueRodInfo(QWidget *parent = 0);
    ~TorqueRodInfo();
    
    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    void updateTorqueRods(AdcsSimDataManager *simDataManager);
    Legend *m_legend;
    
signals:
    void setStateTR(ComponentState_t, int);
    
    
public slots:
    void showOnOffStateContextMenu(const QPoint &);
    void setStateTR(QString);
    void setOnOffLegendTR(bool);
    
private:
    Ui::TorqueRodInfo *ui;
    
    int m_layoutNumCol;
    int m_selecLabel;
    int m_minimumColWidth;
    QString getStateToolTip(ComponentState_t componentState);
    QString getStateStyleSheet(ComponentState_t componentState);
    QString m_tipColor;
};

#endif // TORQUERODINFO_H
