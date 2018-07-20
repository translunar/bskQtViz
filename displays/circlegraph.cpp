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
#include "circlegraph.h"

#include <QPainter>
#include <QPen>
#include <QBrush>

CircleGraph::CircleGraph(QWidget *parent)
    : QWidget(parent)
    , m_percent(0)
{

}

CircleGraph::~CircleGraph()
{
    
}
void CircleGraph::setPercent(float percent)
{
    m_percent = percent;
    update();
}

void CircleGraph::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    /*QPen pen1(QColor(qRgb(31, 120, 180)));
    
    QPainter painter(this);
    painter.setPen(Qt::blue);

    float width = (float)this->width();
    float height = (float)this->height();
    QRectF rec;
    rec = QRectF(QPointF(-width * 0.5, height * 0.5),
                     QPointF(width * 0.5, -height * 0.5));
    
    painter.drawEllipse(rec);
    QBrush brush = QColor(qRgb(31, 120, 180));
    painter.fillRect(rec, brush);*/
    QPainter painter(this);
    painter.setPen(Qt::blue);
    painter.setFont(QFont("Arial", 30));
    painter.drawText(rect(), Qt::AlignCenter, "Qt");
    
    
}
