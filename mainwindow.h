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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QActionGroup> 
#include <QCheckBox>

#include "simdatamanager.h"
#include "scenewidget.h"
#include "playbackcontrols.h"
#include "orbitelementsinfo.h"
#include "reactionwheelsinfo.h"
#include "simtimeinfo.h"
#include "cssinfo.h"
#include "angularratesinfo.h"
#include "controlstateinfo.h"
#include "torquerodinfo.h"
#include "thrustersinfo.h"
#include "totalpowerinfo.h"
#include "modelsdisplayinfo.h"
#include "podinfo.h"
#include "iruinfo.h"
#include "taminfo.h"
#include "startrackerinfo.h"

//#include "starcatalogparser.h"


namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    QList <QCheckBox*> toggleablesCheckBoxList;

public:
    MainWindow();
    ~MainWindow();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    void setDataPath(QString &dataPath);
    QString dataPath();
    int loadSpiceFiles();
    void setUpDefaults(CelestialObject_t);
    
    int loadStarCatalog();
    
public slots:
    void simDataUpdated();
    void setOnOffDefaults(CelestialObject_t);
    void openLegendRW(bool);
    void openLegendTR(bool);

protected:
    void closeEvent(QCloseEvent *event);

protected slots:
    void setWindowStyle(QAction *action);
    void cycleCameraMode();
    void openConnection();
    void closeConnection();
    void openFile();
    void closeFile();
    void toggleFullScreen();

private:
    Ui::MainWindow *ui;
    
    // NOTE: Change the next line to implement an individual simulation data manager
    AdcsSimDataManager *m_simDataManager;
    QString *m_dataPath;
    SceneWidget *m_sceneWidget;
    PlaybackControls *m_playbackControls;
    QLabel *m_connectionStatus;
    ModelsDisplayInfo *m_modelsDisplayInfo;
    OrbitElementsInfo *m_orbitElementsInfo;
    ReactionWheelsInfo *m_reactionWheelsInfo;
    SimTimeInfo *m_simTimeInfo;
    CssInfo *m_cssInfo;
    AngularRatesInfo *m_angularRatesInfo;
    ControlStateInfo *m_controlStateInfo;
    TorqueRodInfo *m_torqueRodInfo;
    ThrustersInfo *m_thrustersInfo;
    TotalPowerInfo *m_totalPowerInfo;
    PodInfo *m_podInfo;
    IruInfo *m_iruInfo;
    TamInfo *m_tamInfo;
    StarTrackerInfo *m_starTrackerInfo;
    QComboBox *m_cameraTargetCombo;
    QComboBox *m_cameraModeCombo;
    QLabel *m_zoomSpeedLabel;
    QDoubleSpinBox *m_zoomSpeed;
    QVector<QAction *> m_zoomActions;
    QLabel *m_panSpeedLabel;
    QDoubleSpinBox *m_panSpeed;
    QVector<QAction *> m_panActions;
    QLabel *m_fpvTranslateLabel;
    QDoubleSpinBox *m_fpvTranslate;
    QLabel *m_fpvRotateLabel;
    QDoubleSpinBox *m_fpvRotate;
    QVector<QAction *> m_fpvActions;
    QActionGroup *m_styleActionGroup;
    
    //StarCatalogParser *m_starCatalogParser;
    
    void createShortcutsWidget();
    void createSceneWidget(); 
    void createPlaybackWidget();
    void createToggleablesWidget();

    void saveSettings();
    void loadSettings();
    
    int m_initialCameraMode;
    
    

private slots:
    void about();
    void updateViewToolbarStates();
    void updateToggleables(QWidget *widget);
    void setPlaybackControlsVisible(bool);
    void updateShortcutsStates();
};

#endif // MAINWINDOW_H
