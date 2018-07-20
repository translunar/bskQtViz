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
#ifndef SCIENTIFIC_SPIN_BOX_H
#define SCIENTIFIC_SPIN_BOX_H

#include <QContextMenuEvent>
#include <QAbstractSpinBox>
#include <QString>
#include <QMenu>

class ScientificSpinBox : public QAbstractSpinBox
{
    Q_OBJECT

    Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(double value READ value WRITE setValue)
    Q_PROPERTY(int precision READ precision WRITE setPrecision)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(bool showFractions READ showFractions WRITE setShowFractions)
    Q_PROPERTY(bool autoScaleStep READ autoScaleStep WRITE setAutoScaleStep)

public:
    explicit ScientificSpinBox(QWidget *parent = 0);
    ~ScientificSpinBox();

    double minimum() const;
    void setMinimum(const double &value);

    double maximum() const;
    void setMaximum(const double &value);

    double singleStep() const;
    void setSingleStep(const double &value);

    double value() const;
    // set function is a slot

    int precision() const;
    void setPrecision(const int &value);

    QString prefix() const;
    void setPrefix(const QString &text);

    QString suffix() const;
    void setSuffix(const QString &text);

    bool showFractions() const;
    // set function is a slot

    bool autoScaleStep() const;
    // set function is a slot

    double textToValue(const QString &text) const;
    QString valueToText(const double &value) const;

public slots:
    void setValue(double value);
    void setShowFractions(bool value);
    void setAutoScaleStep(bool value);
    void contextMenuEvent(QContextMenuEvent *event);

signals:
    void valueChanged(double value);
    void valueChanged(const QString &text);

protected:
    virtual void stepBy(int steps);
    // Step enabled must be implemented for stepBy to work
    virtual StepEnabled stepEnabled() const;

private slots:
    void setValueFromText();
    void setModeFloat();
    void setModeScientific();
    void setModeMixed();

private:
    void updateDisplay();
    void setupMenu();

    double m_minimum;
    double m_maximum;
    double m_value;
    double m_singleStep;
    int m_precision;
    char m_format;
    QString m_prefix;
    QString m_suffix;

    QMenu *m_contextMenu;
    QAction *m_actionShowFractions;
    QAction *m_actionAutoScaleStep;
    QMenu *m_modeMenu;
    QAction *m_actionModeFloat;
    QAction *m_actionModeScientific;
    QAction *m_actionModeMixed;
    QActionGroup *m_actionModeGroup;
};

#endif