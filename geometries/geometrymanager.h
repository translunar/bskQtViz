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
#ifndef GEOMETRYMANAGER_H
#define GEOMETRYMANAGER_H

#include "geometry.h"

#include <QObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>

class GeometryManager : public QObject
{
    Q_OBJECT
public:
    explicit GeometryManager(QObject *parent = 0);
    ~GeometryManager();

    QSharedPointer<Geometry> addGeometry(QString name, QString filename);
    QSharedPointer<Geometry> addGeometry(QString name, QSharedPointer<Geometry> geometry);
    QSharedPointer<Geometry> getGeometry(QString name);
    QList<QString> getAvailableGeometries();

    QSharedPointer<QOpenGLTexture> addTexture(QString file);
    QSharedPointer<QOpenGLTexture> getTexture(QString file);

    bool initializeGeometry(QString name, QOpenGLShaderProgram *program, bool forceInit = false);
    void drawGeometry(QString name, QOpenGLShaderProgram *program, QMatrix4x4 cameraMatrix, QMatrix4x4 objectMatrix, float scaleFactor,
                      QSharedPointer<Geometry::MaterialInfo> defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(),
                      QSharedPointer<GeometryUpdateParameters> parameters = QSharedPointer<GeometryUpdateParameters>());
    float getGeometryBoundingRadii(QString name);

    void cleanupGeometries();
    void cleanupTextures();

private:
    QHash<QString, QSharedPointer<Geometry> > m_geometries;
    void createDefaultGeometries();

    // Keep track of association between opengl ids and texture files
    QHash<QString, QSharedPointer<QOpenGLTexture> > m_textures;
    void cleanupTexture(QSharedPointer<QOpenGLTexture> texture);
    // Bind and generate all the textures for a single geometry
    bool createTextures(QSharedPointer<Geometry> geometry);
};

#endif // GEOMETRYMANAGER_H
