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
#include "simdatamanager.h"
#include "unitline.h"
#include "linestrip.h"
#include "thrusterGeometry.h"
#include "fieldOfView.h"
#include "reactionwheeldisk.h"
#include "startrackerfov.h"

SimDataManager::SimDataManager(QObject *parent)
    : QObject(parent)
    , m_preferredCameraTarget(0)
    , m_simTime(0.0)
{

}

SimDataManager::~SimDataManager()
{

}

bool SimDataManager::setToggleableObjectStatus(QString name, bool value)
{
    if(m_toggleableObjects.contains(name)) {
        m_toggleableObjects[name] = value;
        return true;
    } else {
        return false;
    }
}

void SimDataManager::setTargetObject(int targetIndex)
{
    if(m_targetObjectIndex != targetIndex && targetIndex != -1) {
        m_targetObjectIndex = targetIndex;
        if(m_simObjects.size() > targetIndex) {
            m_isSpacecraftTarget = this->m_simObjects.at(m_targetObjectIndex).name.compare("Spacecraft", Qt::CaseInsensitive) == 0; // si la mida del vector m_simObject es mes gran que targetIndex, m_isSpaceCraftTarget=false
        }
    }
}

SimObject SimDataManager::createXAxis(QVector4D color)
{
    SimObject temp;
    temp.name = "X-Axis";
    temp.position = QVector3D(0, 0, 0);
    temp.quaternion = QQuaternion(1, 0, 0, 0);
    temp.geometryName = "UnitLine";
    temp.scaleFactor = 1.25f;
    temp.scaleByParentBoundingRadii = true;
    temp.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
    return temp;
}

SimObject SimDataManager::createYAxis(QVector4D color)
{
    SimObject temp;
    temp.name = "Y-Axis";
    temp.position = QVector3D(0, 0, 0);
    temp.quaternion = SimObject::computeRotation(QVector3D(0, 1, 0));
    temp.geometryName = "UnitLine";
    temp.scaleFactor = 1.25f;
    temp.scaleByParentBoundingRadii = true;
    temp.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
    return temp;
}

SimObject SimDataManager::createZAxis(QVector4D color)
{
    SimObject temp;
    temp.name = "Z-Axis";
    temp.position = QVector3D(0, 0, 0);
    temp.quaternion = SimObject::computeRotation(QVector3D(0, 0, 1));
    temp.geometryName = "UnitLine";
    temp.scaleFactor = 1.25f;
    temp.scaleByParentBoundingRadii = true;
    temp.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
    return temp;
}

SimObject SimDataManager::createOrbit(classicElements oe, int type, double mu, QString name)
{
    SimObject           temp;
    QVector<QVector3D>  points;
    QVector3D           point;
    double              f0;
    float               angle;
    double              r[3];
    double              v[3];
    float               numPoints = 360.;
    int                 i;
    int                 clippingState = 0;

    temp.name = "Orbit" + name;
    f0 = oe.f;

    if (type == 1) {
        /* hyperbolic departure orbit fraction */
        angle = (float)M_PI*2.0;
        for(i = 0; i <= numPoints; i++) {
            oe.f = f0 + angle * i / numPoints;
            if (cos(oe.f) > -1.0/oe.e) {
                elem2rv(mu, &oe, r, v);
                point = QVector3D (r[0], r[1], r[2]);
                points.push_back(point);
            } else {
                // add a large increment to force exit of loop and finish the hyperbolic departure arc
                i += numPoints;
            }
        }
        QSharedPointer<LineStripUpdateParameters> parameters(new LineStripUpdateParameters);
        parameters->linePoints = points;
        temp.updateParameters = parameters;
        temp.geometryName = "LineStrip";
        QVector4D color(1, 1, 0, 0.3);
        temp.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
    } else {
        /* full orbit */
        angle = (float)M_PI*2.0;
        for(i = 0; i <= numPoints; i++) {
            oe.f = f0 - angle * i / numPoints;
            if ((oe.alpha>=0. || cos(oe.f) > -1.0/oe.e) && !clippingState) {
                elem2rv(mu, &oe, r, v);
                point = QVector3D (r[0], r[1], r[2]);
                points.push_back(point);
            } else {
                // add a large increment to force exit of loop and finish the hyperbolic arrival arc
                i += numPoints;
            }
        }
        QSharedPointer<LineStripUpdateParameters> parameters(new LineStripUpdateParameters);
        parameters->linePoints = points;
        temp.updateParameters = parameters;
        temp.geometryName = "FadingLineStrip";
        QVector4D color(1, 1, 0, 1);
        temp.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
    }

    return temp;
}

SimObject SimDataManager::createSun(QVector3D position, double scaleFactor)
{
    SimObject sun;
    sun.name = "Sun";
    sun.position = position * scaleFactor;
    sun.quaternion = QQuaternion();
    sun.geometryName = "Sun";
    sun.restrictToSceneBoundary = true;
    sun.scaleFactor = 3.0 * scaleFactor;
    return sun;
}

SimObject SimDataManager::createThruster(QVector3D position, QQuaternion orientation, double thrustLevel, double plumeDiameter, double scaleFactor, QVector4D color)
{
    SimObject thruster;
    thruster.name = "Thruster";
    thruster.geometryName = "Thruster";
    thruster.position = position;
    thruster.quaternion = orientation;
    thruster.scaleFactor = scaleFactor;
    thruster.scaleByParentBoundingRadii = true;
    
    QSharedPointer<ThrusterGeometryUpdateParameters> parameters(new ThrusterGeometryUpdateParameters);
    parameters->length = thrustLevel/100.;
    parameters->plumeDiameter = plumeDiameter;
    thruster.updateParameters = parameters;
    thruster.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
    
    return thruster;
}

SimObject SimDataManager::createFieldOfView(QVector3D position, QQuaternion orientation, double scaleFactor, double fovAngle, QVector4D color)
{
    SimObject fieldOfView;
    fieldOfView.name = "FieldOfView";
    fieldOfView.geometryName = "FieldOfView";
    fieldOfView.position = position;
    fieldOfView.quaternion = orientation;
    fieldOfView.scaleFactor = scaleFactor;
    fieldOfView.scaleByParentBoundingRadii = true;
    fieldOfView.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
    QSharedPointer<FieldOfViewUpdateParameters> parameters(new FieldOfViewUpdateParameters);
    parameters->angle = fovAngle;
    fieldOfView.updateParameters = parameters;
    
    return fieldOfView;
}

SimObject SimDataManager::createFadingAxis(QVector3D position, QQuaternion orientation, QVector4D color)
{
    SimObject sensorNormal;
    sensorNormal.name = "SensorNormal";
    sensorNormal.position = position;
    sensorNormal.quaternion = orientation;
    sensorNormal.geometryName = "FadingLineStrip";
    sensorNormal.scaleFactor = 1.2f;
    sensorNormal.scaleByParentBoundingRadii = true;
    sensorNormal.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
    QSharedPointer<LineStripUpdateParameters> parameters(new LineStripUpdateParameters);
	parameters->linePoints.push_back(QVector3D(0, 0, 0));
    parameters->linePoints.push_back(QVector3D(2, 0, 0));
	sensorNormal.updateParameters = parameters;

    return sensorNormal;
}

SimObject SimDataManager::createDiskRW(QVector3D position, QQuaternion orientation, double scaleFactor, QVector4D color, bool flag)
{
    SimObject DiskRW;
    DiskRW.name = "DiskRW";
    DiskRW.geometryName = "DiskRW";
    DiskRW.position = position;
    DiskRW.quaternion = orientation;
    DiskRW.scaleFactor = scaleFactor;
    DiskRW.scaleByParentBoundingRadii = true;
    DiskRW.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
    
    QSharedPointer<ReactionWheelDiskUpdateParameters> parameters(new ReactionWheelDiskUpdateParameters);
    parameters->flag = flag;
    DiskRW.updateParameters = parameters;
    
    return DiskRW;
}

SimObject SimDataManager::createTorqueBar(QVector3D position, QQuaternion orientation, double scaleFactor, QVector4D color)
{
    SimObject TorqueBar;
    TorqueBar.name = "TorqueBar";
    TorqueBar.geometryName = "TorqueBar";
    TorqueBar.position = position;
    TorqueBar.quaternion = orientation;
    TorqueBar.scaleFactor = scaleFactor;
    TorqueBar.scaleByParentBoundingRadii = true;
    TorqueBar.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
    
    return TorqueBar;
}

SimObject SimDataManager::createStarTrackerFOV(QVector3D position, QQuaternion orientation, double scaleFactor, double fovAngle, QVector4D color)
{
    SimObject FovST;
    FovST.name = "FovST";
    FovST.geometryName = "FovST";
    FovST.position = position;
    FovST.quaternion = orientation;
    FovST.scaleFactor = scaleFactor;
    FovST.scaleByParentBoundingRadii = true;
    FovST.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
    
    /*QSharedPointer<StarTrackerFovUpdateParameters> parameters(new StarTrackerFovUpdateParameters);
    parameters->angle = fovAngle;
    starTrackerFOV.updateParameters = parameters;*/
    
    return FovST;
}
