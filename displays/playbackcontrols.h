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
#ifndef PLAYBACKCONTROLS_H
#define PLAYBACKCONTROLS_H

#include <QWidget>
#include <QDateTime>
#include <time.h>
#include <QSettings>

#include "ScientificSpinBox.h"

namespace Ui {
class PlaybackControls;
}

class PlaybackControls : public QWidget
{
    Q_OBJECT

public:
    explicit PlaybackControls(QWidget *parent = 0);
    ~PlaybackControls();

    enum mode_t {
        PLAYBACK_FILE,
        PLAYBACK_STREAM,
        MAX_PLAYBACK_MODE
    };

    void setDateTimeBounds(QDateTime timeInitial, QDateTime timeFinal);
    void setDateTimeBounds(QDateTime timeInitial, double timeFinalSec);
    void setDateTimeBounds(double jd0, double timeFinalSec);
    void saveSettings (QSettings *settings);
    void loadSettings (QSettings *settings);

signals:
    void dateTimeRequested(QDateTime time);
    void dateTimeRequested(double timeSec);
    // Signals that a play speed change has been requested, display relys on slot setPlaySpeed
    void playSpeedRequested(double);
    void estPlaySpeedUpdated(double);

public slots:
    void setCurrentDateTime(double sec);
    void setCurrentDateTime(QDateTime time);
    void setPlaySpeed(double value);
    void setMode(mode_t mode);

private slots:
    void requestPlayPause();
    void updatePlayPause();

    // Request new time multiplier based on the user interacting with the speed toggle
    void speedSliderPressed();
    void speedSliderMoved(int);
    void speedSliderReleased();

    // Request new time
    void requestDateTimeBegin();
    void requestDateTimeEnd();
    void requestDateTime(QDateTime);
    // Request new time when slider changed by user
    void timeSliderMoved(int);
    // Request new time when timeEdit changed by user
    void timeEdited();
    // Request new play speed when changed by user 
    void playSpeedEdited(double);

    void speedUp();
    void slowDown();
    void maxSpeedTriggered(bool);

private:
    Ui::PlaybackControls *ui;
    ScientificSpinBox *m_playSpeedSpinBox;
    //double m_initialPlaySpeed;
    bool m_isPlaying;

    // Time bounds
    QDateTime m_dateTimeInitial;
    QDateTime m_dateTimeFinal;
    QDateTime m_dateTime;

    QDateTime m_dateTimePrev;
    clock_t m_clockTimePrev;
    double m_estPlaySpeed;
    double m_estPlaySpeedPrev;
    double m_rawPlaySpeedPrev;

    // Value of time multiplier when time toggle pressed
    double m_playSpeedPrev;
    bool m_isSettingPlaySpeed;

    void updateTimeSliderMaximum(double maxSec);

    QDateTime seconds2QDateTime(double s);
    double qDateTime2Seconds(QDateTime time);
    double linearInterp(double x0, double x1, double y0, double y1, double xi);
};

#endif // PLAYBACKCONTROLS_H
