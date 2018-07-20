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
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "astroConstants.h"

#include <QWidget>
#include <QBrush>
#include <QList>

class Legend : public QWidget
{
    Q_OBJECT
    
public:
    Legend(QWidget *parent = 0);
    ~ Legend();
    
    
    QList <QColor> colorPalette;
    void getLegendInfo (QString tooltip);
    
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    
private:
    QString m_tooltip;
};

class Orbit2D : public QWidget
{
    Q_OBJECT
    QList <float> r_xList;
    QList <float> r_yList;
    
public:
    Orbit2D(QWidget *parent = 0);
    ~Orbit2D();
    
    void getOrbit(float e, float a, float f, double ref);
    
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    
private:
    float m_e;
    float m_a;
    float m_f;
    float m_ref;
    int m_maxPoints;
};

class OrbitalPeriod : public QWidget
{
    Q_OBJECT
    
public:
    OrbitalPeriod(QWidget *parent = 0);
    ~OrbitalPeriod();
    
    void getOrbitFraction(double fraction, double fraction0, int counter);
    
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    
private:
    double m_fraction;
    double m_fraction0;
    int m_counter;
};

class OnOffCircle : public QWidget
{
    Q_OBJECT
    
public:
    OnOffCircle(QWidget *parent = 0);
    ~OnOffCircle();

    void getPercent(float percent, QBrush brush);
    void setPercent(float percent);
    
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    
private:
    float m_percent;
    QBrush m_brush;;
};

class OnePlusMinusPercentBar : public QWidget
{
    Q_OBJECT
public:
    OnePlusMinusPercentBar(QWidget *parent = 0);
    ~OnePlusMinusPercentBar();
    void setPercent(float percent);
    void getToolTip(QString *str);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
    float m_percent;
    QColor m_color;
    QString m_tipColor;
    QString *m_toolTip;
    
};


class PieChart : public QWidget
{
    Q_OBJECT
    
    QList <double> m_list;
    QList <QString> m_nameList;
    QList <QColor> colorPalette;
public:
    PieChart(QWidget *parent = 0);
    ~PieChart();
    void getPowerList(QList<double> list, QList<QString> nameList);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
};



class TwoPlusMinusPercentBars : public QWidget
{
    Q_OBJECT
public:
    TwoPlusMinusPercentBars(QWidget *parent = 0);
    ~TwoPlusMinusPercentBars();
    void setPercent1(float percent);
    void setPercent2(float percent);
    void setPercent3(float percent);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
    float m_percent1;
    float m_percent2;
    float m_percent3;
};


class ThreePlusMinusPercentBars : public QWidget
{
    Q_OBJECT
public:
    ThreePlusMinusPercentBars(QWidget *parent = 0);
    ~ThreePlusMinusPercentBars();
    void setPercent1(float percent);
    void setPercent2(float percent);
    void setPercent3(float percent);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
    float m_percent1;
    float m_percent2;
    float m_percent3;
};



#endif // GRAPHICS_H
