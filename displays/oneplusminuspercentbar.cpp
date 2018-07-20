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
#include "oneplusminuspercentbar.h"

#include <QPainter>
#include <QPen>
#include <QBrush>

OnePlusMinusPercentBar::OnePlusMinusPercentBar(QWidget *parent)
    : QWidget(parent)
    , m_percent(0)
{
    setMinimumHeight(50);
    setToolTip(" <font color='#5abbfc'>% Torque Max</font>");
    setToolTipDuration(3000);
}

OnePlusMinusPercentBar::~OnePlusMinusPercentBar()
{

}

void OnePlusMinusPercentBar::setPercent(float percent)
{
    m_percent = percent;
    update();
}

void OnePlusMinusPercentBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    float width = (float)(this->width())/3;
    float height = (float)(this->height());
    
    QBrush brush = QColor(qRgb(31, 120, 180));
    QRectF rec;
    
    if(m_percent >= 0) {
        rec = QRectF(QPointF(width, height * 0.5 - height * 0.5 * (m_percent / 100.0)),
                     QPointF(width*2, height * 0.5));
    } else {
        rec = QRectF(QPointF(width, height * 0.5),
                     QPointF(width*2, height * 0.5 - height * 0.5 * (m_percent / 100.0)));
    }
    painter.fillRect(rec, brush);
    QPen pen0(QColor(qRgb(31, 120, 180)));
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

