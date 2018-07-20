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
#include "playbackcontrols.h"
#include "ui_playbackcontrols.h"

#include <iostream>
#include <limits>

#include <QStyle>
#include <qmath.h>

extern "C" {
#include "astroConstants.h"
}

PlaybackControls::PlaybackControls(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlaybackControls)
    , m_dateTimeInitial(QDate(2000, 1, 1), QTime(12, 0), Qt::UTC)
    , m_dateTimeFinal(QDate(2000, 1, 1), QTime(12, 0), Qt::UTC)
    , m_dateTimePrev(m_dateTimeInitial)
    , m_estPlaySpeed(1.0)
    , m_estPlaySpeedPrev(1.0)
    , m_rawPlaySpeedPrev(1.0)
    , m_playSpeedPrev(1.0)
    , m_isSettingPlaySpeed(false)
    //, m_initialPlaySpeed(1.0)
{
    ui->setupUi(this);

    connect(ui->timeSlider, SIGNAL(sliderMoved(int)), this, SLOT(timeSliderMoved(int)));
    ui->timeSlider->hide();
    connect(ui->dateTimeEdit, SIGNAL(editingFinished()), this, SLOT(timeEdited()));
    setDateTimeBounds(m_dateTimeInitial, m_dateTimeFinal);
    ui->dateTimeEdit->hide();
    
    m_playSpeedSpinBox = new ScientificSpinBox;
    m_playSpeedSpinBox->setMinimumWidth(100);
    m_playSpeedSpinBox->setAlignment(Qt::AlignHCenter);
    m_playSpeedSpinBox->setValue(1.0);
    m_playSpeedSpinBox->setMinimum(0.0);
    m_playSpeedSpinBox->setSuffix("x");
    m_playSpeedSpinBox->setToolTip("Play Speed");
    ui->playSpeedLayout->insertWidget(1, m_playSpeedSpinBox);
    setPlaySpeed(1.0);
    connect(m_playSpeedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(playSpeedEdited(double)));

    ui->slowDownButton->setDefaultAction(ui->actionSlowDown);
    ui->actionSlowDown->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));
    connect(ui->actionSlowDown, SIGNAL(triggered()), this, SLOT(slowDown()));
    ui->speedUpButton->setDefaultAction(ui->actionSpeedUp);
    ui->actionSpeedUp->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    connect(ui->actionSpeedUp, SIGNAL(triggered()), this, SLOT(speedUp()));
    ui->maxSpeedButton->setDefaultAction(ui->actionMaxSpeed);
    ui->actionMaxSpeed->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    connect(ui->actionMaxSpeed, SIGNAL(triggered(bool)), this, SLOT(maxSpeedTriggered(bool)));
    ui->maxSpeedButton->hide();
    ui->actionMaxSpeed->setVisible(false);

    ui->goToBeginningButton->setDefaultAction(ui->actionGoToBeginning);
    ui->actionGoToBeginning->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    connect(ui->actionGoToBeginning, SIGNAL(triggered()), this, SLOT(requestDateTimeBegin()));
    ui->goToBeginningButton->hide();

    m_isPlaying = false;
    ui->playPauseButton->setDefaultAction(ui->actionPlayPause);
    ui->playPauseButton->setIconSize(QSize(24, 24));
    ui->playPauseButton->hide();
    
    ui->actionPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->actionPlayPause->setToolTip("Play");
    ui->actionPlayPause->setText("Play");
    connect(ui->actionPlayPause, SIGNAL(triggered()), this, SLOT(requestPlayPause()));
    ui->actionPlayPause->setVisible(false);
    
    ui->goToEndButton->setDefaultAction(ui->actionGoToEnd);
    ui->actionGoToEnd->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    connect(ui->actionGoToEnd, SIGNAL(triggered()), this, SLOT(requestDateTimeEnd()));
    ui->goToEndButton->hide();
    ui->actionGoToEnd->setVisible(false);
    
    // Configure speed toggle
    connect(ui->speedSlider, SIGNAL(sliderPressed()), this, SLOT(speedSliderPressed()));
    connect(ui->speedSlider, SIGNAL(sliderReleased()), this, SLOT(speedSliderReleased()));
    connect(ui->speedSlider, SIGNAL(sliderMoved(int)), this, SLOT(speedSliderMoved(int)));
    ui->speedSlider->hide();

    addAction(ui->actionGoToBeginning);
    addAction(ui->actionPlayPause);
    addAction(ui->actionGoToEnd);

    ui->slowerLabel->setPixmap(style()->standardPixmap(QStyle::SP_MediaSeekBackward).scaled(16, 16));
    ui->fasterLabel->setPixmap(style()->standardPixmap(QStyle::SP_MediaSeekForward).scaled(16, 16));
    ui->fasterLabel->setFixedSize(QSize(16, 16));
    ui->slowerLabel->hide();
    ui->fasterLabel->hide();

    //@TODO: Currently the playbackcontrols are configured for streaming (i.e. time editing, and go to
    // beginning and end buttons are disabled). If the option to play from a file becomes available this
    // needs to be updated to properly handle those inputs.
    setMode(PLAYBACK_STREAM);
    
    // Necessary to avoid weird things happening when Visualization is paused, closed and reopened
    if (m_playSpeedPrev >= 1/INFINITY || m_playSpeedSpinBox->value() >= 1/INFINITY)
    {
        setPlaySpeed(1.0);
        m_playSpeedSpinBox->setValue(1.0);
        m_playSpeedPrev = 1.0;
        
    }
}

PlaybackControls::~PlaybackControls()
{
    delete ui;
}

void PlaybackControls::saveSettings(QSettings *settings)
{
    settings->beginGroup("playbackControls");
    settings->setValue("playSpeed", m_playSpeedSpinBox->value());
    settings->endGroup();
}

void PlaybackControls::loadSettings(QSettings *settings)
{
//    settings->beginGroup("playbackControls");
//    setPlaySpeed(settings->value("playSpeed").toDouble());
//    m_playSpeedPrev= settings->value("playSpeed").toDouble();
//    m_playSpeedSpinBox->setValue(settings->value("playSpeed").toDouble());
//    settings->endGroup();
}

void PlaybackControls::setDateTimeBounds(QDateTime timeInitial, QDateTime timeFinal)
{
    if(timeInitial > timeFinal) {
        std::cout << "ERROR: Final time must be greater than initial time" << std::endl;
        return;
    }
    m_dateTimeInitial = timeInitial;
    m_dateTimeFinal = timeFinal;
    updateTimeSliderMaximum(timeInitial.msecsTo(timeFinal) / 1000.0);
    ui->dateTimeEdit->setMinimumDateTime(m_dateTimeInitial);
    ui->dateTimeEdit->setMaximumDateTime(m_dateTimeFinal);
    ui->dateTimeEdit->setToolTip("Simulation Time<br>Start: " 
        + m_dateTimeInitial.toString("yyyy/MM/dd HH:mm:ss.zzz")
        + "<br>Stop: "
        + m_dateTimeFinal.toString("yyyy/MM/dd HH:mm:ss.zzz"));
}

void PlaybackControls::setDateTimeBounds(QDateTime timeInitial, double timeFinalSec)
{
    QDateTime timeFinal = timeInitial.addMSecs((qint64)(timeFinalSec * 1000.0));
    setDateTimeBounds(timeInitial, timeFinal);
}

void PlaybackControls::setDateTimeBounds(double jd0, double timeFinalSec)
{
    // The day number is the integer part of the date
    int julianDays = qFloor(jd0);
    QDate d = QDate::fromJulianDay(julianDays);
    double julianMSecs = (jd0 - static_cast<double>(julianDays)) * 86400.0 * 1000.0;
    QDateTime timeInitial = QDateTime(d, QTime(12, 0, 0, 0), Qt::UTC).addMSecs(qRound(julianMSecs));
    setDateTimeBounds(timeInitial, timeFinalSec);
}

void PlaybackControls::setCurrentDateTime(double sec)
{
    setCurrentDateTime(seconds2QDateTime(sec));
}

void PlaybackControls::setCurrentDateTime(QDateTime time)
{
//    if(time > m_dateTimeFinal) {
//        std::cout << "ERROR: Time must be less than or equal to final time." << std::endl;
//        return;
//    } else if(time < m_dateTimeInitial) {
//        std::cout << "ERROR: Time must be greater than or equal to initial time." << std::endl;
//        return;
//    }

    // Compute actual speed up factor
    double simDt = m_dateTimePrev.msecsTo(time) / 1000.0;
    clock_t clockTime = clock();
    double clockDt = ((double)(clockTime - m_clockTimePrev)) / CLOCKS_PER_SEC;
    double rawPlaySpeed = simDt / clockDt;
    // Low pass filter the result
    double lowPassFilterHz = 1.0;
    double temp = (2.0 * M_PI * lowPassFilterHz) * (clockDt);
    double a0 = 1.0 / (2.0 + temp);
    double a1 = 2.0 - temp;
    double a2 = temp;
    m_estPlaySpeed = a0 * (a1 * m_estPlaySpeedPrev + a2 * (rawPlaySpeed + m_rawPlaySpeedPrev));
    emit estPlaySpeedUpdated(m_estPlaySpeed);
    // Store requesite values for next time
    m_dateTimePrev = time;
    m_clockTimePrev = clockTime;
    m_estPlaySpeedPrev = m_estPlaySpeed;
    m_rawPlaySpeedPrev = rawPlaySpeed;

    // Update time
//    m_dateTime = time;
//    // Update slider
//    double x0 = qDateTime2Seconds(m_dateTimeInitial);
//    double x1 = qDateTime2Seconds(m_dateTimeFinal);
//    double xi = qDateTime2Seconds(time);
//    double y0 = (double)ui->timeSlider->minimum();
//    double y1 = (double)ui->timeSlider->maximum();
//    double yi = linearInterp(x0, x1, y0, y1, xi);
//    ui->timeSlider->setValue(qRound(yi));
//    // Update the time edit box
//    ui->dateTimeEdit->setDateTime(time);
}

void PlaybackControls::setPlaySpeed(double value)
{
    value = ceil(value);
    if(!m_isSettingPlaySpeed) {
        m_isSettingPlaySpeed = true;
        m_isPlaying = value != 0.0;
        updatePlayPause();
        if(value == -1.0) {
            m_playSpeedSpinBox->setReadOnly(true);
            m_playSpeedSpinBox->setPrefix("~");
            m_playSpeedSpinBox->setValue(m_estPlaySpeed);
//            m_playSpeedSpinBox->setValue(m_playSpeedPrev);
            ui->speedSlider->setEnabled(false);
        } else {
            if(m_isPlaying) {
                if(value != m_playSpeedSpinBox->value()) {
                    m_playSpeedSpinBox->setReadOnly(false);
                    m_playSpeedSpinBox->setPrefix("");
                    m_playSpeedSpinBox->setValue(value);
//                    ui->speedSlider->setEnabled(true);
                }
            } else {
                if(m_playSpeedSpinBox->value() != m_playSpeedPrev) {
                    m_playSpeedSpinBox->setReadOnly(false);
                    m_playSpeedSpinBox->setValue(m_playSpeedPrev);
                }
                ui->speedSlider->setEnabled(false);
            }
        }
        ui->actionMaxSpeed->setChecked(value == -1.0);
        m_isSettingPlaySpeed = false;
    }
    else {m_playSpeedSpinBox->setValue(1.0);}
}

void PlaybackControls::setMode(mode_t mode)
{
    bool enable = mode == PLAYBACK_FILE;
    ui->timeSlider->setEnabled(enable);
    ui->dateTimeEdit->setReadOnly(enable);
    ui->goToBeginningButton->setEnabled(enable);
    ui->goToEndButton->setEnabled(enable);
}

void PlaybackControls::requestPlayPause()
{
    if(m_isPlaying) {
        m_playSpeedPrev = m_playSpeedSpinBox->value();
        emit playSpeedRequested(0.0);
    } else {
        emit playSpeedRequested(m_playSpeedPrev);
    }
    
//    m_initialPlaySpeed=m_playSpeedSpinBox->value();
//    setPlaySpeed(m_initialPlaySpeed);
}

void PlaybackControls::updatePlayPause()
{
    if(m_isPlaying) {
        ui->actionPlayPause->setText("Pause");
        ui->actionPlayPause->setToolTip("Pause");
        ui->actionPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        ui->speedSlider->setEnabled(true);
        m_playSpeedSpinBox->setReadOnly(false);
    } else {
        ui->actionPlayPause->setText("Play");
        ui->actionPlayPause->setToolTip("Play");
        ui->actionPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        ui->speedSlider->setEnabled(false);
        m_playSpeedSpinBox->setReadOnly(true);
    }
}

void PlaybackControls::speedSliderPressed()
{
    m_playSpeedPrev = m_playSpeedSpinBox->value();
}

void PlaybackControls::speedSliderMoved(int value)
{
    double mult = pow(2.0, value);
    emit playSpeedRequested(mult * m_playSpeedPrev);
}

void PlaybackControls::speedSliderReleased()
{
    ui->speedSlider->setValue(0);
    emit playSpeedRequested(m_playSpeedPrev);
}

void PlaybackControls::requestDateTimeBegin()
{
    requestDateTime(m_dateTimeInitial);
}

void PlaybackControls::requestDateTimeEnd()
{
    requestDateTime(m_dateTimeFinal);
}

void PlaybackControls::requestDateTime(QDateTime time)
{
    emit dateTimeRequested(time);
    emit dateTimeRequested(m_dateTimeInitial.msecsTo(time) * 1000.0);
}

void PlaybackControls::timeSliderMoved(int value)
{
    double x0 = (double)ui->timeSlider->minimum();
    double x1 = (double)ui->timeSlider->maximum();
    double y0 = qDateTime2Seconds(m_dateTimeInitial);
    double y1 = qDateTime2Seconds(m_dateTimeFinal);
    QDateTime newTime = seconds2QDateTime(linearInterp(x0, x1, y0, y1, (double)value));
    requestDateTime(newTime);
}

void PlaybackControls::timeEdited()
{
    QDateTime dateTime = ui->dateTimeEdit->dateTime();
    if(dateTime > m_dateTimeFinal) {
        dateTime = m_dateTimeFinal;
    } else if(dateTime < m_dateTimeInitial) {
        dateTime = m_dateTimeInitial;
    }
    requestDateTime(dateTime);
}

void PlaybackControls::playSpeedEdited(double value)
{
    if(!m_isSettingPlaySpeed) {
        if(m_isPlaying) {
            emit playSpeedRequested(value);
        } else {
            m_playSpeedPrev = value;
            setPlaySpeed(0.0);
        }
    }
}

void PlaybackControls::speedUp()
{
    if(m_isPlaying) {
        emit playSpeedRequested(m_playSpeedSpinBox->value() * 2.0);
    } else {
        m_playSpeedPrev *= 2.0;
        setPlaySpeed(0.0);
    }
}

void PlaybackControls::slowDown()
{
    if(m_isPlaying) {
        emit playSpeedRequested(m_playSpeedSpinBox->value() * 0.5);
    } else {
        m_playSpeedPrev *= 0.5;
        setPlaySpeed(0.0);
    }
}

void PlaybackControls::maxSpeedTriggered(bool isChecked)
{
    if(m_isPlaying) {
        if(isChecked) {
            m_playSpeedPrev = m_playSpeedSpinBox->value();
            emit playSpeedRequested(-1.0);
        } else {
            emit playSpeedRequested(m_playSpeedPrev);
        }
    } else {
        // Max speed doesn't work while paused
    }
}

void PlaybackControls::updateTimeSliderMaximum(double maxSec)
{
    ui->timeSlider->setMinimum(0);
    // Set the time slider step functions
    // Setting a higher maximum eliminates jumps in the bar
    if(maxSec > std::numeric_limits<int>::max()) {
        maxSec = (double)std::numeric_limits<int>::max();
    }
    ui->timeSlider->setMaximum((int)maxSec);
    ui->timeSlider->setPageStep((int)(maxSec * 0.1));
    // If the single step is too small you won't see any movement in time for a single step
    ui->timeSlider->setSingleStep(1);
}

QDateTime PlaybackControls::seconds2QDateTime(double s)
{
    return m_dateTimeInitial.addSecs(s);
}

double PlaybackControls::qDateTime2Seconds(QDateTime time)
{
    int temp = m_dateTimeInitial.msecsTo(time);
    return temp / 1000.0;
}

double PlaybackControls::linearInterp(double x0, double x1, double y0, double y1, double xi)
{
    double r = (xi - x0) / (x1 - x0);
    return y0 + r*(y1 - y0);
}
