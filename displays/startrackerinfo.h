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
#ifndef STARTRACKERINFO_H
#define STARTRACKERINFO_H

#include <QDockWidget>
#include <QSettings>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QList>

#include "adcssimdatamanager.h"
#include "graphics.h"

//#include "starcatalogparser.h"

typedef std::vector < std::vector<double> > matrix;
typedef std::vector <matrix> matrixArray;

namespace Ui {
class StarTrackerInfo;
}

class StarTrackerInfo : public QDockWidget
{
    Q_OBJECT
    
    QList <QLabel*> labelList;

public:
    explicit StarTrackerInfo(QWidget *parent = 0);
    ~StarTrackerInfo();
    
    void saveSettings(QSettings *settings);
    void loadSettings(QSettings *settings);
    void updateST(AdcsSimDataManager *simDataManager);
    void parseStarsData(const char *dataPath);
    
    std::vector <double> DEC;
    std::vector <double> RA;
    std::vector <double> VM;

private:
    Ui::StarTrackerInfo *ui;
    QLabel *m_stLabel;
    QString getStateToolTip(ComponentState_t componentState, int resetCounter);
    QString getStateStyleSheet(ComponentState_t componentState, int resetCounter);
    double m_FOVangle;
    int m_Nd; // number of declination bands
    int m_Nr; // number of right ascension bands
    int m_SRn; // spherical rectangle nominal size
    
    QGraphicsScene *scene;
    QGraphicsScene *scene2;
    
    matrix star;
    std::vector <matrix> bandRA;
    std::vector <matrixArray> bandDEC;
    
    void drawStar(double dec, double ra, double vm, QGraphicsScene *sceneH);
    
    double m_width;
    double m_height;
    
    double m_DECn;
    double m_DECs;
    double m_RAe;
    double m_RAw;
    
    int m_iDECn;
    int m_iDECs;
    int m_iRAe;
    int m_iRAw;
    
    int m_upperBoundDEC;
    int m_lowerBoundDEC;
    int m_upperBoundRA;
    int m_lowerBoundRA;
    
    
};

class Star
{
public:
    Star()
    : declination(0.0)
    , rightAscension(0.0)
    , visualMagnitude(0.0) {}
    ~ Star();
    
    
    // Properties of each star
    double declination;
    double rightAscension;
    double visualMagnitude;
    
    // Store child objects
    QVector<Star> starsSR;
};






#endif // STARTRACKERINFO_H
