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
#ifndef TWOPLUSMINUSPERCENTBARS_H
#define TWOPLUSMINUSPERCENTBARS_H

#include <QWidget>

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
#endif // TWOPLUSMINUSPERCENTBARS_H
