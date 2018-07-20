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
#include "ScientificSpinBox.h"

#include <limits>
#include <cmath>
#include <cerrno>

#include <QLineEdit>

extern "C" {
#include "shuntingYard.h"
}

ScientificSpinBox::ScientificSpinBox(QWidget *parent)
    : QAbstractSpinBox(parent)
    , m_minimum(std::numeric_limits<double>::min())
    , m_maximum(std::numeric_limits<double>::max())
    , m_value(0.0)
    , m_singleStep(1.0)
    , m_precision(3)
    , m_format('g')
    , m_prefix("")
    , m_suffix("")
    , m_contextMenu(new QMenu(this))
    , m_actionShowFractions(new QAction(this))
    , m_actionAutoScaleStep(new QAction(this))
    , m_modeMenu(new QMenu(this))
    , m_actionModeFloat(new QAction(this))
    , m_actionModeScientific(new QAction(this))
    , m_actionModeMixed(new QAction(this))
    , m_actionModeGroup(new QActionGroup(this))
{
    setupMenu();
    connect(this, SIGNAL(editingFinished()), this, SLOT(setValueFromText()));
}

ScientificSpinBox::~ScientificSpinBox()
{
}

double ScientificSpinBox::minimum() const
{
    return m_minimum;
}

//\property ScientificSpinBox::minimum
//\brief This property holds the maximum value of the spin box.
// 
// The lowest value that the spin box accepts
// It is enforced to be less than maximum and the current
// value is enforced to be more than or equal to this
void ScientificSpinBox::setMinimum(const double &value)
{
    m_minimum = std::fmin(value, m_maximum);
    if(m_value < value) {
        setValue(value);
    }
    updateDisplay();
}

double ScientificSpinBox::maximum() const
{
    return m_maximum;
}

//\property ScientificSpinBox::maximum
//\brief This property holds the maximum value of the spin box.
//
// The highest value that the spin box accepts
// It is enforced to be greater than minimum and the current
// value is enforced to be less than or equal to this
void ScientificSpinBox::setMaximum(const double &value)
{
    m_maximum = std::fmax(value, m_minimum);
    if(m_value > value) {
        setValue(value);
    }
    updateDisplay();
}

double ScientificSpinBox::singleStep() const
{
    return m_singleStep;
}

void ScientificSpinBox::setSingleStep(const double &value)
{
    m_singleStep = value == 0 ? 1.0 : value;
}

double ScientificSpinBox::value() const
{
    return m_value;
}

void ScientificSpinBox::setValue(double value)
{
    m_value = value;
    updateDisplay();
}

int ScientificSpinBox::precision() const
{
    return m_precision;
}

void ScientificSpinBox::setPrecision(const int &value)
{
    m_precision = value;
    updateDisplay();
}

QString ScientificSpinBox::prefix() const
{
    return m_prefix;
}

void ScientificSpinBox::setPrefix(const QString &text)
{
    m_prefix = text;
    updateDisplay();
}

QString ScientificSpinBox::suffix() const
{
    return m_suffix;
}

void ScientificSpinBox::setSuffix(const QString &text)
{
    m_suffix = text;
    updateDisplay();
}

bool ScientificSpinBox::showFractions() const
{
    return m_actionShowFractions->isChecked();
}

void ScientificSpinBox::setShowFractions(bool value)
{
    m_actionShowFractions->setChecked(value);
    updateDisplay();
}

bool ScientificSpinBox::autoScaleStep() const
{
    return m_actionAutoScaleStep->isChecked();
}

void ScientificSpinBox::setAutoScaleStep(bool value)
{
    m_actionAutoScaleStep->setChecked(value); 
    updateDisplay();
}

double ScientificSpinBox::textToValue(const QString &text) const
{
    // Remove excess white space
    QString trimmedText = text.simplified();
    // Check if prefix present
    if(trimmedText.indexOf(m_prefix) == 0) {
        trimmedText = trimmedText.remove(0, m_prefix.length());
    }
    // Check if suffix present
    int lastIndex = trimmedText.lastIndexOf(m_suffix);
    if(lastIndex == (trimmedText.length() - 1)) {
        trimmedText.chop(m_suffix.length());
    }
    double newValue = shuntingYard(trimmedText.toStdString().c_str());
    return newValue;
}

QString ScientificSpinBox::valueToText(const double &value) const
{
    double outputValue(value);
    bool isFraction = false;
    if(m_actionShowFractions->isChecked()) {
        if(fabs(outputValue) < 1.0) {
            outputValue = 1.0 / outputValue;
            isFraction = true;
        }
    }
QString text = QString::number(outputValue, m_format, m_precision);
if(m_actionShowFractions->isChecked() && isFraction) {
    text.prepend("1/");
}
text.prepend(m_prefix);
text.append(m_suffix);
return text;
}

void ScientificSpinBox::contextMenuEvent(QContextMenuEvent *event)
{
    m_contextMenu->exec(event->globalPos());
}

void ScientificSpinBox::stepBy(int steps)
{
    double stepSize = singleStep();
    if(m_actionAutoScaleStep->isChecked()) {
        double val = value();
        int i = 0;
        while(fabs(val) > 10.0) {
            val /= 10.0;
            i++;
        }
        stepSize *= pow(10.0, i);
    }

    double newValue(value());
    if(steps != 0) {
        if(newValue == 0.0) {
            newValue = steps < 0 ? -1 : 1;
        } else if(steps > 0) {
            try {
                for(int i = 0; i < steps; i++) {
                    newValue += stepSize;
                }
            } catch(...) {
                newValue = maximum();
            }
        } else {
            try {
                for(int i = 0; i > steps; i--) {
                    newValue -= stepSize;
                }
            } catch(...) {
                newValue = minimum();
            }
        }
    }
    setValue(newValue);
}

QAbstractSpinBox::StepEnabled ScientificSpinBox::stepEnabled() const
{
    QAbstractSpinBox::StepEnabled stepFlag(QAbstractSpinBox::StepNone);
    if(value() > minimum()) {
        stepFlag |= QAbstractSpinBox::StepDownEnabled;
    }
    if(value() < maximum()) {
        stepFlag |= QAbstractSpinBox::StepUpEnabled;
    }
    return stepFlag;
}

void ScientificSpinBox::setValueFromText()
{
    QString input(lineEdit()->text());
    errno = 0;
    double newValue = textToValue(input);
    bool newValueDifferent = newValue != value();
    if(errno == 0) {
        setValue(newValue);
        if(newValueDifferent && valueToText(newValue).compare(input) == 0) {
            emit valueChanged(newValue);
        }
    } else {
        // Value not valid so revert to original value
        updateDisplay();
    }
}

void ScientificSpinBox::setModeFloat()
{
    m_format = 'f';
    updateDisplay();
}

void ScientificSpinBox::setModeScientific()
{
    m_format = 'e';
    updateDisplay();
}

void ScientificSpinBox::setModeMixed()
{
    m_format = 'g';
    updateDisplay();
}

void ScientificSpinBox::updateDisplay()
{
    QString newString(valueToText(value()));
    if(newString != lineEdit()->text()) {
        lineEdit()->setText(newString);
        emit valueChanged(newString);
        emit valueChanged(m_value);
    }
}

void ScientificSpinBox::setupMenu()
{
    // Add in the standard lineEdit context menu before adding in specializations
    m_contextMenu = lineEdit()->createStandardContextMenu();
    m_contextMenu->addSeparator();

    m_actionShowFractions->setText("Show Fractions");
    m_actionShowFractions->setCheckable(true);
    m_actionShowFractions->setChecked(true);
    m_contextMenu->addAction(m_actionShowFractions);
    connect(m_actionShowFractions, SIGNAL(triggered(bool)), this, SLOT(setShowFractions(bool)));

    m_actionAutoScaleStep->setText("Auto Scale Step");
    m_actionAutoScaleStep->setToolTip("Step size is multiplied by 10 to the order of the current value");
    m_actionAutoScaleStep->setCheckable(true);
    m_actionAutoScaleStep->setChecked(true);
    m_contextMenu->addAction(m_actionAutoScaleStep);
    connect(m_actionAutoScaleStep, SIGNAL(triggered(bool)), this, SLOT(setAutoScaleStep(bool)));

    m_actionModeFloat->setActionGroup(m_actionModeGroup);
    m_actionModeFloat->setText("Float");
    m_actionModeFloat->setCheckable(true);
    m_actionModeFloat->setChecked(true);
    connect(m_actionModeFloat, SIGNAL(triggered()), this, SLOT(setModeFloat()));

    m_actionModeScientific->setActionGroup(m_actionModeGroup);
    m_actionModeScientific->setText("Scientific");
    m_actionModeScientific->setCheckable(true);
    connect(m_actionModeScientific, SIGNAL(triggered()), this, SLOT(setModeScientific()));

    m_actionModeMixed->setActionGroup(m_actionModeGroup);
    m_actionModeMixed->setText("Mixed");
    m_actionModeMixed->setCheckable(true);
    connect(m_actionModeMixed, SIGNAL(triggered()), this, SLOT(setModeMixed()));

    m_modeMenu->setTitle("Mode");
    m_modeMenu->addActions(m_actionModeGroup->actions());
    m_contextMenu->addMenu(m_modeMenu);
}