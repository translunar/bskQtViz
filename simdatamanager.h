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
#ifndef SIMDATAMANAGER_H
#define SIMDATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QVector3D>
#include <QMap>
#include <QString>

extern "C" {
#include "utilities/astroConstants.h"
#include "utilities/orbitalMotion.h"
}

#include "simobject.h"

// Create a convience typedef
typedef QMap<QString, bool> toggleableObjectMap;

class SimDataManager : public QObject
{
    Q_OBJECT
public:
    explicit SimDataManager(QObject *parent = 0);
    ~SimDataManager();

    // Methods for loading in data
    virtual bool openConnection(QString ipAddress, QString port) = 0;
    virtual bool isConnectionOpen() {
        return m_inputType == INPUT_CONNECTION;
    }
    virtual bool closeConnection() = 0;
    virtual bool openFile(QString filename) = 0;
    virtual bool isFileOpen() {
        return m_inputType == INPUT_FILE;
    }
    virtual bool closeFile() = 0;

    // Methods for dispersing data loaded in
    QVector<SimObject> getSimObjects() {
        return m_simObjects;
    }
    int getPreferredCameraTarget() {
        return m_preferredCameraTarget;
    }
    virtual QVector3D getLightPosition() {
        return QVector3D(0, 0, 0);
    }

    // Get the simulation time
    double getSimTime() {
        return m_simTime;
    }
    
    // Get list of togglable mesh objects
    toggleableObjectMap getToggleableObjects() {
        return m_toggleableObjects;
    }

    void setIsSpacecraftTarget(int value) {
        if(value >= 0) {
            m_isSpacecraftTarget = value;
        }
    }

    void setTargetObject(int targetIndex);
    int getTargetObject(void)
    {
        return m_targetObjectIndex;
    }
    bool setToggleableObjectStatus(QString name, bool value); // set the true/false value for each key

signals:
    // Emit signal that the simulation data has been updated
    void simDataUpdated();
    // Emit signal that a message should be displayed in the top most status bar
    void showMessage(QString message);
    void showMessage(QString message, int timeout);

public slots:
    // Set the simulation time
    virtual void setSimTime(double time) = 0;

protected:
    QVector<SimObject> m_simObjects;
    int m_preferredCameraTarget;
    double m_simTime;
    int m_targetObjectIndex;
    bool m_isSpacecraftTarget;

    toggleableObjectMap m_toggleableObjects;
    
    enum {
        INPUT_NONE,
        INPUT_CONNECTION,
        INPUT_FILE
    } m_inputType;

    SimObject createXAxis(QVector4D color = QVector4D(1.0, 0.0, 0.0, 1.0));
    SimObject createYAxis(QVector4D color = QVector4D(0.0, 1.0, 0.0, 1.0));
    SimObject createZAxis(QVector4D color = QVector4D(0.0, 0.0, 1.0, 1.0));
    SimObject createOrbit(classicElements oe,  int type, double mu, QString name);
    SimObject createSun(QVector3D position, double scaleFactor);
    SimObject createThruster(QVector3D position, QQuaternion orientation, double thrustLevel, double plumeDiameter, double scaleFactor, QVector4D color = QVector4D(0.4f, 0.8f, 1.0f, 1.0f));
    SimObject createFieldOfView(QVector3D position, QQuaternion orientation, double scaleFactor, double fovAngle, QVector4D color);
    SimObject createFadingAxis(QVector3D position, QQuaternion orientation, QVector4D color);
    
    SimObject createDiskRW(QVector3D position, QQuaternion orientation, double scaleFactor, QVector4D color, bool flag);
    
    SimObject createTorqueBar (QVector3D position, QQuaternion orientation, double scaleFactor, QVector4D color);
    
    SimObject createStarTrackerFOV(QVector3D position, QQuaternion orientation, double scaleFactor, double fovAngle, QVector4D color);
};
#endif // SIMDATAMANAGER_H
