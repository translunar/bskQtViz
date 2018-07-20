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
#include "twoplusminuspercentbars.h"

#include <QPainter>
#include <QPen>
#include <QBrush>

TwoPlusMinusPercentBars::TwoPlusMinusPercentBars(QWidget *parent)
    : QWidget(parent)
    , m_percent1(0)
    , m_percent2(0)
    , m_percent3(0)
{
    setMinimumHeight(50);
    setToolTip(" <font color='#5abbfc'>% Torque Max</font><br><font color='#ffff33'>% Wheel Speed Max </font><br><font color='#b8a3fa'>% Temperature Max</font>");
    setToolTipDuration(5000);
}

TwoPlusMinusPercentBars::~TwoPlusMinusPercentBars()
{

}

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

