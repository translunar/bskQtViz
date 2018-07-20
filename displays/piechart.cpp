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
#include "piechart.h"
#include "math.h"
#include <QPainter>
#include <QPen>
#include <QBrush>

PieChart::PieChart(QWidget *parent)
    : QWidget(parent)
    , m_percentRW(0)
    , m_percentTR(0)
    , m_percentST(0)
    , m_colorRW(QColor(qRgb(122, 207, 238))) // #7acfee
    , m_colorTR(QColor(qRgb(196,162,230))) // #c4a2e6
    , m_colorST(QColor(qRgb(162,230,162))) // #a2e6a2
{
    setMinimumHeight(50);
}

PieChart::~PieChart()
{

}

void PieChart::setPercent(double percentRW, double percentTR, double percentST)
{
    m_percentRW = percentRW;
    m_percentTR = percentTR;
    m_percentST = percentST;
    update();
}

void PieChart::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    float width = (float)this->width();
    float height = (float)this->height();
    float minimum = fmin(width, height);
    QRectF rec;
    rec = QRectF(QPointF(0, minimum),
                 QPointF(minimum, 0));
    rec.translate((width-minimum)/2, 0);
    
    int startAngleRW= 90*16;
    //int spanAngleRW= -16 * percentRW * 360/100;
    int spanAngleRW= -16 * m_percentRW * 360/100;
    QBrush brush = m_colorRW;
    painter.setBrush(brush);
    painter.setPen(m_colorRW);
    painter.drawPie(rec, startAngleRW,spanAngleRW);
    
    int startAngleTR= startAngleRW + spanAngleRW;
    //int spanAngleTR= -16 * percentTR * 360/100;
    int spanAngleTR= -16 * m_percentTR * 360/100;
    QBrush brushTR = m_colorTR;
    painter.setPen(m_colorTR);
    painter.setBrush(brushTR);
    painter.drawPie(rec, startAngleTR,spanAngleTR);
   
    int startAngleST= startAngleTR + spanAngleTR;
    int spanAngleST= -270*16 - startAngleST;
    QBrush brushST = m_colorST;
    painter.setPen(m_colorST);
    painter.setBrush(brushST);
    painter.drawPie(rec, startAngleST,spanAngleST);
    
    setToolTip("<font color='#7acfee'><b>"+tr("%1 % REACTION WHEELS").arg(int(m_percentRW))+"</b></font><br><font color='#c4a2e6'><b>"+tr("%1 % TORQUE RODS").arg(int(m_percentTR))+"</b><br></font><font color='#a2e6a2'><b>"+tr("%1 % STAR TRACKER").arg(m_percentST)+"</b></font>");
    setToolTipDuration(5000);
}

