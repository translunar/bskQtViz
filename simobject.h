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
#ifndef SIMOBJECT_H
#define SIMOBJECT_H

#include <QString>
#include <QVector>
#include <QVector3D>
#include <QVector4D>
#include <QQuaternion>

#include "geometry.h"

class SimObject
{
public:
    SimObject() // class and proporties will be used in adcssimdatamanager.com
        : name("")
        , position(QVector3D())
        , quaternion(QQuaternion())
        , geometryName("")
        , scaleByParentBoundingRadii(false)
        , scaleFactor(1.0f)
        , defaultMaterialOverride(0)
        , restrictToSceneBoundary(true) {}
    ~SimObject();

    QString name;
    // Position of the object (km)
    QVector3D position;
    // Quaternion describing the orientation of the object
    QQuaternion quaternion;

    // Name of geometry to draw for object
    QString geometryName;
    // Flag for specifying if object and any children should be scaled by
    // bounding radii of parent geometry, will not apply if set on top most parent
    bool scaleByParentBoundingRadii;
    // Scale factor to apply to scene object and any children
    float scaleFactor;
    // Override a mesh's default material (used for UnitLines, UnitSpheres, etc)
    QSharedPointer<Geometry::MaterialInfo> defaultMaterialOverride;
    // Parameters to pass to a geometry if its being dynamically updated
    QSharedPointer<GeometryUpdateParameters> updateParameters;

    // Determines if object should be scaled and placed at edge of scene
    bool restrictToSceneBoundary;

    // Store child objects
    QVector<SimObject> simObjects;

    // Computes rotation quaternion from (1, 0, 0) to vector
    static QQuaternion computeRotation(QVector3D vector);
    // Computes rotation quaternion that will rotate vector1 to vector2
    static QQuaternion computeRotation(QVector3D vector1, QVector3D vector2);

};

#endif // SIMOBJECT_H
