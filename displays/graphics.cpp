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
#include "graphics.h"
#include "math.h"
#include <QPainter>
#include <QPen>
#include <QBrush>

PieChart::PieChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(50);
    colorPalette.append (QColor(qRgb(250,220,250)));
    colorPalette.append (QColor(qRgb(185,220,250)));
    colorPalette.append (QColor(qRgb(220,185,250)));
    
    colorPalette.append (QColor(qRgb(250,250,185)));
    colorPalette.append (QColor(qRgb(185,250,185)));
    colorPalette.append (QColor(qRgb(220,250,185)));
    
    colorPalette.append (QColor(qRgb(250,185,185)));
    colorPalette.append (QColor(qRgb(250,220,185)));
    colorPalette.append (QColor(qRgb(250,185,150)));
    
    int vector[]= {250,  180,220};
    for (int i=0; i<3; i++)
    {
        for (int j=0; j<3; j++)
        {
            for (int k=0; k<3; k++)
            {
                QColor newColor =QColor(qRgb(vector[i], vector[j], vector[k]));
                bool colorRepeat=false;
                for (int n=0; n<colorPalette.size();n++)
                {
                    if (newColor == colorPalette[n])
                    {
                        colorRepeat=true;
                        break;
                    }
                }
                if (colorRepeat==false)
                {
                    colorPalette.append(newColor);
                }
            }
        }
    }
}
PieChart::~PieChart(){}
void PieChart::getPowerList(QList <double> powerList, QList <QString> nameList)
{
    m_list = powerList;
    m_nameList = nameList;
    update();
}
void PieChart::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    float width = (float)this->width();
    float height = (float)this->height();
    
    double x0 = 0.0;
    QString output;
    for (int i=0; i<m_list.size(); i++)
    {
        QRectF rec;
        {
            rec = QRectF (QPointF(x0, height/2), QPointF(x0 + width*m_list[i], 0));
            x0 +=width*m_list[i];
        }
        QBrush brush= colorPalette[i];
        painter.fillRect(rec, brush);
            
        QString colorName = colorPalette[i].name();
        output.append("<font color='"+tr("%1").arg(colorName)+"'><b>"+tr("%1").arg(m_list[i]*100)+"% "+ m_nameList[i]+"</b></font><br>");
    }
    setToolTip(output);
}



OnePlusMinusPercentBar::OnePlusMinusPercentBar(QWidget *parent)
    : QWidget(parent)
    , m_percent(0)
    , m_color(QColor(qRgb(31, 120, 180)))

{
    setMinimumHeight(50);
}
OnePlusMinusPercentBar::~OnePlusMinusPercentBar(){}

void OnePlusMinusPercentBar::setPercent(float percent)
{
    m_percent = percent;
    update();
}

void OnePlusMinusPercentBar::getToolTip(QString *str)
{
    m_toolTip = str;
    QString output;
    if (str == QString("% Torque") || str == QString("% Dipole Moment") )
    {
        m_color = QColor(qRgb(31, 120, 180));
        m_tipColor = "#5abbfc";
    }
    if (str == QString("% Temperature"))
    {
        m_color = QColor(qRgb(95, 75, 159));
        m_tipColor = "#b8a3fa";
    }
    else if (str == QString("% Speed"))
    {
        m_color = qRgb(255, 255, 51);
        m_tipColor = "ffff33";
    }
    output.append("<font color='"+m_tipColor+"'><b>"+ *str +"</b></font><br>");
    setToolTip(output);
    setToolTipDuration(3000);
    update();
}

void OnePlusMinusPercentBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    float width = (float)(this->width())/3;
    float height = (float)(this->height());
    
    QBrush brush = m_color;
    QRectF rec;
    
    if(m_percent >= 0) {
        rec = QRectF(QPointF(width, height * 0.5 - height * 0.5 * (m_percent / 100.0)),
                     QPointF(width*2, height * 0.5));
    } else {
        rec = QRectF(QPointF(width, height * 0.5),
                     QPointF(width*2, height * 0.5 - height * 0.5 * (m_percent / 100.0)));
    }
    painter.fillRect(rec, brush);
    QPen pen0(m_color);
    painter.setPen(pen0);
    painter.drawLine(QPointF(width, height*0.5),QPointF(width*2, height*0.5));
    
    QPen pen(QColor(qRgb(0, 0, 0)));
    int tickWidthMajor = 3;
    int tickWidthMinor = 1;
    painter.setPen(pen);
    
    painter.drawLine(QPointF(width - 1, 0),
                     QPointF(width - 1, height));
    painter.drawLine(QPointF(width - 1 - tickWidthMajor, 0),
                     QPointF(width - 1 + tickWidthMajor, 0));
    painter.drawLine(QPointF(width - 1 - tickWidthMajor, height / 2),
                     QPointF(width - 1 + tickWidthMajor, height / 2));
    painter.drawLine(QPointF(width - 1 - tickWidthMajor, height - 1),
                     QPointF(width - 1 + tickWidthMajor, height - 1));
    painter.drawLine(QPointF(width - 1 - tickWidthMinor, height / 4),
                     QPointF(width - 1 + tickWidthMinor, height / 4));
    painter.drawLine(QPointF(width - 1 - tickWidthMinor, 3 * height / 4),
                     QPointF(width - 1 + tickWidthMinor, 3 * height / 4));
}


TwoPlusMinusPercentBars::TwoPlusMinusPercentBars(QWidget *parent)
    : QWidget(parent)
    , m_percent1(0)
    , m_percent2(0)
    , m_percent3(0)
{
    setMinimumHeight(50);
    setToolTip(" <font color='#5abbfc'><b>% Torque</b></font><br><font color='#ffff33'>% Wheel Speed</font><br><font color='#b8a3fa'><b>% Temperature</b></font>");
    setToolTipDuration(5000);
}
TwoPlusMinusPercentBars::~TwoPlusMinusPercentBars(){}
void TwoPlusMinusPercentBars::setPercent1(float percent)
{
    m_percent1 = percent;
    update();
}
void TwoPlusMinusPercentBars::setPercent2(float percent)
{
    m_percent2 = percent;
    update();
}
void TwoPlusMinusPercentBars::setPercent3(float percent)
{
    m_percent3 = percent;
    update();
}
void TwoPlusMinusPercentBars::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    float width = (float)this->width();
    float height = (float)this->height();

    QBrush brush = QColor(qRgb(31, 120, 180));
    QRectF rec;
    if(m_percent1 >= 0) {
        rec = QRectF(QPointF(0, height * 0.5 - height * 0.5 * (m_percent1 / 100.0)),
                     QPointF(width/3, height * 0.5));
        
    } else {
        rec = QRectF(QPointF(0, height * 0.5),
                     QPointF(width/3, height * 0.5 - height * 0.5 * (m_percent1 / 100.0)));
    }
    painter.fillRect(rec, brush);
    QPen pen1(QColor(qRgb(31, 120, 180)));
    painter.setPen(pen1);
    painter.drawLine(QPointF(0, height*0.5), QPointF(width/3,height*0.5));

    brush = QColor(qRgb(255, 255, 51));
    if(m_percent2 >= 0) {
        rec = QRectF(QPointF(width/3, height * 0.5 - height * 0.5 * (m_percent2 / 100.0)),
                     QPointF(width*2/3, height * 0.5));
    } else {
        rec = QRectF(QPointF(width/3, height * 0.5),
                     QPointF(width*2/3, height * 0.5 - height * 0.5 * (m_percent2 / 100.0)));
    }
    painter.fillRect(rec, brush);
    QPen pen2(QColor(qRgb(255, 255, 51)));
    painter.setPen(pen2);
    painter.drawLine(QPointF(width/3, height*0.5), QPointF(width*2/3,height*0.5));
    
    brush = QColor(qRgb(95, 75, 159));
    if(m_percent3 >= 0) {
        rec = QRectF(QPointF(width * 2/3, height * 0.5 - height * 0.5 * (m_percent3 / 100.0)),
                     QPointF(width, height * 0.5));
    } else {
        rec = QRectF(QPointF(width * 2/3, height * 0.5),
                     QPointF(width, height * 0.5 - height * 0.5 * (m_percent3 / 100.0)));
    }
    painter.fillRect(rec, brush);
    QPen pen3(QColor(qRgb(95, 75, 159)));
    painter.setPen(pen3);
    painter.drawLine(QPointF(width *2/3, height*0.5), QPointF(width,height*0.5));

    QPen pen(QColor(qRgb(0, 0, 0)));
    int tickWidthMajor = 3;
    int tickWidthMinor = 1;
    painter.setPen(pen);
    painter.drawLine(QPointF(width / 3, 0),
                     QPointF(width / 3.0, height));
    painter.drawLine(QPointF(width / 3 - tickWidthMajor, 0),
                     QPointF(width / 3 + tickWidthMajor, 0));
    painter.drawLine(QPointF(width / 3 - tickWidthMajor, height / 2),
                     QPointF(width / 3 + tickWidthMajor, height / 2));
    painter.drawLine(QPointF(width / 3 - tickWidthMajor, height - 1),
                     QPointF(width / 3 + tickWidthMajor, height - 1));
    painter.drawLine(QPointF(width / 3 - tickWidthMinor, height / 4),
                     QPointF(width / 3 + tickWidthMinor, height / 4));
    painter.drawLine(QPointF(width / 3 - tickWidthMinor, 3 * height / 4),
                     QPointF(width / 3 + tickWidthMinor, 3 * height / 4));
    
    painter.drawLine(QPointF(width * 2/3, 0),
                     QPointF(width * 2/3.0, height));
    painter.drawLine(QPointF(width * 2/3 - tickWidthMajor, 0),
                     QPointF(width * 2/3 + tickWidthMajor, 0));
    painter.drawLine(QPointF(width * 2/3 - tickWidthMajor, height / 2),
                     QPointF(width * 2/3 + tickWidthMajor, height / 2));
    painter.drawLine(QPointF(width * 2/3 - tickWidthMajor, height - 1),
                     QPointF(width * 2/3 + tickWidthMajor, height - 1));
    painter.drawLine(QPointF(width * 2/3 - tickWidthMinor, height / 4),
                     QPointF(width * 2/3 + tickWidthMinor, height / 4));
    painter.drawLine(QPointF(width * 2/3 - tickWidthMinor, 3 * height / 4),
                     QPointF(width * 2/3 + tickWidthMinor, 3 * height / 4));
}

ThreePlusMinusPercentBars::ThreePlusMinusPercentBars(QWidget *parent)
: QWidget(parent)
, m_percent1(0)
, m_percent2(0)
{
    setMinimumHeight(50);
    setToolTip(" <font color='#5abbfc'><b>% Radius Max</b></font><br><font color='#ffff33'><b>% Velocity Max </b></font><br> <i>Maximums based on expected values</i>");
    setToolTipDuration(6000);
}
ThreePlusMinusPercentBars::~ThreePlusMinusPercentBars(){}
void ThreePlusMinusPercentBars::setPercent1(float percent)
{
    m_percent1 = percent;
    update();
}
void ThreePlusMinusPercentBars::setPercent2(float percent)
{
    m_percent2 = percent;
    update();
}

void ThreePlusMinusPercentBars::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    
    QPainter painter(this);
    float width = (float)this->width();
    float height = (float)this->height();
    
    //(91, 210, 169)
    //(84, 110, 235)
    QBrush brush = QColor(qRgb(31, 120, 180));
    QRectF rec;
    m_percent1=100.0;
    rec = QRectF(QPointF(width/4, height * (1 - m_percent1 / 100.0) ),
                 QPointF(width/2, height));
    
    painter.fillRect(rec, brush);
    QPen pen1(QColor(qRgb(31, 120, 180)));
    painter.setPen(pen1);
    painter.drawLine(QPointF(width/4, height-1), QPointF(width/2, height-1));
    

    //(244, 176, 46)
    //(250, 175, 7)
    brush = QColor(qRgb(255, 255, 51));
    m_percent2=100.0;
    rec = QRectF(QPointF(width/2, height * (1 - m_percent2 / 100.0)),
                 QPointF(width*3/4, height));
    
    painter.fillRect(rec, brush);
    QPen pen2(QColor(qRgb(255, 255, 51)));
    painter.setPen(pen2);
    painter.drawLine(QPointF(width/2, height-1), QPointF(width*3/4, height-1));
    
    QPen pen(QColor(qRgb(0, 0, 0)));
    int tickWidthMajor = 3;
    int tickWidthMinor = 1;
    painter.setPen(pen);
    painter.drawLine(QPointF(width / 2, 0),
                     QPointF(width / 2.0, height));
    painter.drawLine(QPointF(width / 2 - tickWidthMajor, 0),
                     QPointF(width / 2 + tickWidthMajor, 0));
    painter.drawLine(QPointF(width / 2 - tickWidthMajor, height / 2),
                     QPointF(width / 2 + tickWidthMajor, height / 2));
    painter.drawLine(QPointF(width / 2 - tickWidthMajor, height - 1),
                     QPointF(width / 2 + tickWidthMajor, height - 1));
    painter.drawLine(QPointF(width / 2 - tickWidthMinor, height / 4),
                     QPointF(width / 2 + tickWidthMinor, height / 4));
    painter.drawLine(QPointF(width / 2 - tickWidthMinor, 3 * height / 4),
                     QPointF(width / 2 + tickWidthMinor, 3 * height / 4));
}



Orbit2D::Orbit2D(QWidget *parent)
    : QWidget(parent)
    , m_e(1.0)
    , m_a(0.0)
    , m_f(0.0)
    , m_ref(0)
    , m_maxPoints(360)
{
    
    setMinimumSize(50, 150);
    
    for (int i=0; i<m_maxPoints; i++)
    {
        r_xList.append(0);
        r_yList.append(0);
    }
}
Orbit2D::~Orbit2D(){}
void Orbit2D::getOrbit(float e, float a, float f, double ref)
{
    m_a = a ;
    m_e = e;
    m_f = f;
    m_ref = float(a/ref);
    update();
}
void Orbit2D::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black));
    
    float width = (float)this->width() -20;
    float height = (float)this->height();
    
    float x0 = 10 + width * 0.5 * (1 + m_e);
    float y0 = height * 0.5;
    
    QPen pen(QColor(qRgb(0, 0, 0)));
    painter.setPen(pen);
    
    int tickWidthMajor = 4;
    int tickWidthMinor = 3;
    
    painter.drawLine(QPointF(10, height*0.5), QPointF(width + 10, height*0.5));
    
    painter.drawLine(QPointF(10, height / 2 - tickWidthMajor),
                     QPointF(10, height / 2 + tickWidthMajor));
    
    painter.drawLine(QPointF(10+ width / 2, height / 2 - tickWidthMajor),
                     QPointF(10+ width / 2, height / 2 + tickWidthMajor));
    
    painter.drawLine(QPointF(10+ width - 1, height / 2 - tickWidthMajor),
                     QPointF(10+ width - 1, height / 2 + tickWidthMajor));
    
    painter.drawLine(QPointF(10+ width / 4, height / 2 - tickWidthMinor),
                     QPointF(10+ width / 4, height / 2 + tickWidthMinor));
    
    painter.drawLine(QPointF(10+ 3 * width / 4, height / 2 - tickWidthMinor),
                     QPointF(10+ 3 * width / 4, height / 2 + tickWidthMinor));
    
    QPainterPath path;
    if (m_a>=0)
    {
        painter.drawLine(QPointF(x0, 0), QPointF(x0, height));
        
        painter.drawLine(QPointF(x0 - tickWidthMajor, 0),
                         QPointF(x0 + tickWidthMajor, 0));
        
        painter.drawLine(QPointF(x0 - tickWidthMajor, height / 2),
                         QPointF(x0 + tickWidthMajor, height / 2));
        
        painter.drawLine(QPointF(x0 - tickWidthMajor, height - 1),
                         QPointF(x0 + tickWidthMajor, height - 1));
        
        painter.drawLine(QPointF(x0 - tickWidthMinor, height / 4),
                         QPointF(x0 + tickWidthMinor, height / 4));
        
        painter.drawLine(QPointF(x0 - tickWidthMinor, 3 * height / 4),
                         QPointF(x0 + tickWidthMinor, 3 * height / 4));
        
        int r = 10 + width-x0;
        m_maxPoints = 360;
        r_xList[0] = x0 + r;
        r_yList[0] = y0;
        m_a = width / 2;
        path.moveTo(QPointF(r_xList[0], r_yList[0]));
        for (int i=0; i < m_maxPoints; i++)
        {
            double r_vec =  m_a *(1 - m_e * m_e) / (1 + m_e * cos(2*M_PI/360 *i));
            r_xList[i] = r_vec * cos(2*M_PI/360 *i) + x0;
            r_yList[i] = r_vec * sin(2*M_PI/360 *i) + y0;
            path.lineTo(QPointF(r_xList[i], r_yList[i]));
        }
        path.lineTo(QPointF(r_xList[0], r_yList[0]));
        painter.setPen(QPen(Qt::yellow, 0.5, Qt::SolidLine));
        painter.drawPath(path);
        
        painter.setBrush(QBrush(Qt::yellow));
        painter.drawEllipse(QPointF(x0,y0), 3, 3); // focus
        
        painter.setPen(QColor(Qt::yellow));
        painter.drawText(QPointF(10+ width, height*0.5), QString(QChar(0xC0, 0x03)));
        painter.drawText(QPointF(0, height*0.5), QString(QChar(0xB1, 0x03)));
        
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        painter.setPen(QColor(Qt::black));
        
//        painter.drawText(QPointF(width, height*0.5 + 15), QString(tr("%1 R").arg(int(m_ref*(1-m_e)))));
//        painter.drawText(QPointF(0, height*0.5 + 15), QString(tr("%1 R").arg(int (m_ref*(1+m_e)))));
        
        double pos = m_a *(1 - m_e * m_e) / (1 + m_e * cos(m_f));
        QPointF pointPos = QPointF(x0 + pos * cos(2*M_PI - m_f), y0 + pos * sin(2*M_PI - m_f));
        painter.setPen(QPen(QColor(qRgb(90, 190, 255)),0.5, Qt::SolidLine));
        painter.setBrush(QColor(qRgb(90, 190, 255)));
        painter.drawEllipse(pointPos, 3, 3); // spacecraft
    }
    else if(m_a<0)
    {
        painter.drawLine(QPointF(10+ width*0.5, 0), QPointF(10+ width*0.5, height));
        
        painter.drawLine(QPointF(10, height / 2 - tickWidthMajor),
                         QPointF(10, height / 2 + tickWidthMajor));
        
        painter.drawLine(QPointF(10+ width / 2, height / 2 - tickWidthMajor),
                         QPointF(10+ width / 2, height / 2 + tickWidthMajor));
        
        painter.drawLine(QPointF(10+ width - 1, height / 2 - tickWidthMajor),
                         QPointF(10+ width - 1, height / 2 + tickWidthMajor));
        
        painter.drawLine(QPointF(10+ width / 4, height / 2 - tickWidthMinor),
                         QPointF(10+ width / 4, height / 2 + tickWidthMinor));
        
        painter.drawLine(QPointF(10+ 3 * width / 4, height / 2 - tickWidthMinor),
                         QPointF(10+ 3 * width / 4, height / 2 + tickWidthMinor));
        
        m_ref = -m_ref;
        m_a = width/2;
        float x0 = 10 + width * 0.5 - m_a*(m_e -1);
        float y0 = height * 0.5;
        m_maxPoints = 360;
        r_xList[0] = 10 + width * 0.5;
        r_yList[0] = y0;
       
        path.moveTo(QPointF(r_xList[0], r_yList[0]));
        for (int i=0; i< m_maxPoints; i++)
        {
            double f_local = 2*M_PI/360 *i;
            if (cos(f_local) > -1/m_e)
            {
                double r_vec =  m_a *(m_e * m_e - 1) / (1 + m_e * cos(f_local));
                r_xList[i] = r_vec * cos(f_local) + x0;
                r_yList[i] = r_vec * sin(f_local) + y0;
                path.lineTo(QPointF(r_xList[i], r_yList[i]));
            } else {
                i++;
            }
        }
        painter.setPen(QPen(Qt::yellow, 0.5, Qt::SolidLine));
        painter.drawPath(path);
        painter.setBrush(QBrush(Qt::yellow));
        painter.drawEllipse(QPointF(x0,y0), 3, 3); // focus
        
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        painter.setPen(QColor(Qt::black));
        float perigeeDist = m_ref * (m_e - 1);
        painter.drawText(QPointF(width*0.5 + 10, height*0.5), QString(tr("%1 R").arg(int (perigeeDist))));
        
        double pos = m_a *(m_e * m_e -1) / (1 + m_e * cos(m_f));
        QPointF pointPos = QPointF(x0 + pos * cos(2*M_PI - m_f), y0 + pos * sin(2*M_PI - m_f));
        painter.setPen(QColor(qRgb(31, 120, 180)));
        painter.setBrush(QColor(qRgb(90, 190, 255)));
        painter.drawEllipse(pointPos, 3, 3); // spacecraft
    }
}


OrbitalPeriod::OrbitalPeriod(QWidget *parent)
: QWidget(parent)
,   m_fraction(0.0)
,   m_fraction0(0.0)
,   m_counter(0)
{
    setMinimumSize(140, 140);
    //setToolTip("Orbit in Time Representation <br> <i>clock starts at Perigee</i>");
    //setToolTipDuration(4000);
}
OrbitalPeriod::~ OrbitalPeriod(){}
void OrbitalPeriod::getOrbitFraction(double fraction, double fraction0, int counter)
{
    m_fraction = fraction;
    m_fraction0 = fraction0;
    m_counter = counter;
    update();
}
void OrbitalPeriod::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    float width = (float)this->width();
    float height = (float)this->height();
    
    float r = fminf(width, height);
    QRectF rec;
    rec = QRectF(QPointF(0, r*0.5 ),
                 QPointF(r*0.5 , 0));
    rec.translate(width/2 - r * 0.25, height/7);
    
    double startAngle = 16 * m_fraction0 * 360;
    if (m_fraction < m_fraction0)
    {
        m_fraction = m_fraction + 1;
    }
    double spanAngle = 16 * ((m_fraction - m_fraction0)) * 360;
    
    
    painter.setPen(QPen(Qt::yellow, 0.5, Qt::SolidLine));
    painter.drawEllipse(rec);
    painter.setBrush(QColor(255, 255, 50, 50));
    painter.drawPie(rec, startAngle, spanAngle);
    
    painter.setFont(QFont("Arial", 8));
    painter.setPen(QColor(Qt::lightGray));
    painter.drawText(QPointF(width/2 - 22, 15), QString("Orbit in Time"));
    painter.setPen(QColor(Qt::yellow));
    painter.drawText(QPointF(width/2 - 33, height - 35), QString(tr("%1 Completed Orbits").arg(m_counter)));
    
    painter.setPen(QColor(Qt::black));
    painter.setBrush(QBrush(Qt::black));
    float xP = width/2 + r*0.25;
    float yP = height/7 + r*0.25;
    painter.drawEllipse(QPointF(xP, yP), 1, 1);
    painter.drawEllipse(QPointF(xP - r*0.5, yP), 1, 1);
    painter.setFont(QFont("Arial", 13, QFont::Bold));
    painter.drawText(QPointF(xP+5, yP), QString("t"+QString(QChar(0xC0, 0x03))));
    painter.drawText(QPointF(xP - r*0.5 - 17, yP), QString("t"+QString(QChar(0xB1, 0x03))));
    
}


OnOffCircle::OnOffCircle(QWidget *parent)
    : QWidget(parent)
    , m_percent(0)
{
    setFixedSize(35, 35);
}
OnOffCircle::~OnOffCircle(){}
void OnOffCircle::getPercent(float percent, QBrush brush)
{
    m_brush = brush;
    setPercent(percent);
}
void OnOffCircle::setPercent(float percent)
{
    m_percent = percent;
    update();
}
void OnOffCircle::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    float width = (float)this->width();
    float height = (float)this->height();
    QRectF rec;
    rec = QRectF(QPointF(0, height*0.5 ),
                 QPointF(height*0.5 , 0));
    rec.translate(width/2.5, height/4);
    QPen pen1(qRgb(47, 79, 79));
    painter.setPen(pen1);
    if (m_percent==0)
    {
        painter.save();
    }
    painter.setBrush(m_brush);
    if (m_percent==0)
    {
        painter.restore();
    }
    painter.drawEllipse(rec);
}

Legend::Legend(QWidget *parent)
: QWidget(parent)
{
    setFixedHeight(35);
    colorPalette.append (QColor(qRgb(110, 130, 255)));
    colorPalette.append (QColor(qRgb(100, 200, 255)));
    colorPalette.append (QColor(qRgb(110, 255, 130)));
    colorPalette.append (QColor(qRgb(255, 255, 130)));
    colorPalette.append (QColor(qRgb(255, 170,100)));
    colorPalette.append (QColor(qRgb(255, 0, 70)));
    setToolTip(m_tooltip);
}
Legend::~ Legend(){}

void Legend::getLegendInfo(QString tooltip)
{
    m_tooltip = tooltip;
    update();
}

void Legend::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    float width = (float) this->width();
    float height = (float)this->height();
    QRectF rec;
    rec = QRectF(QPointF(0, height-10), QPointF(width , 0));
    QLinearGradient gradient (QPointF (0, height), QPointF(width, height));
    for (int i = 0; i < colorPalette.size(); i++)
    {
        gradient.setColorAt( i * 0.2, colorPalette[i]);
    }
    painter.fillRect(rec, gradient);
    
    painter.setFont(QFont("Arial", 10));
    painter.setPen(QColor(Qt::white));
    
//    int i;
//    for ( i = 0; i < colorPalette.size() + 1; i++)
//    {
//        if (i == colorPalette.size())
//        {
//            painter.drawText(QPointF(width - 5, height), QString(tr("%1").arg(i-1)));
//        }
//        else {
//             painter.drawText(QPointF(width * 0.2 * i, height), QString(tr("%1").arg(i)));
//        }
//    }
    
    this->setToolTip(m_tooltip);
    
}