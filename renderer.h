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
#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "simdatamanager.h"
#include "geometrymanager.h"

#include <QObject>
#include <QOpenGLFunctions_2_1>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QSharedPointer>
#include <QVector3D>
#include <QMap>

class Renderer : public QObject, public QOpenGLFunctions_2_1
{
    Q_OBJECT
public:
    explicit Renderer(QObject *parent, SimDataManager *simDataManager);
    ~Renderer();

    void initializeScene();
    void renderScene();
    void cleanup();

    Camera *camera() {
        return &m_camera;
    }
    void setTargetObject(int targetIndex);
    void setWireframe(bool value);
    void setCameraTargetVisible(bool value);

private:
    SimDataManager *m_simDataManager;
    int m_targetObjectIndex;

    GeometryManager *m_geometryManager;

    QOpenGLShaderProgram *m_lightShader;
    QOpenGLShaderProgram *m_watermarkShader;
    void initializeShaderPrograms();
    void cleanupShaderPrograms();

    bool m_useWireframe;

    QString m_watermarkFile;

    Camera m_camera;
    bool m_showCameraTarget;

    QVector3D m_lightPosition;
    QVector3D m_lightIntensity;

    bool initializeSimObject(QOpenGLShaderProgram *program, SimObject simObject);

    void initializeWatermark();
    void drawWatermark();

    void calculateCameraPosition();
    void drawScene(QOpenGLShaderProgram *program);
    void drawSceneObject(QOpenGLShaderProgram *program, SimObject simObject,
                         QMatrix4x4 cameraMatrix);
    void drawSceneObjectRecursion(QOpenGLShaderProgram *program, SimObject simObject,
                                  QMatrix4x4 cameraMatrix, QMatrix4x4 objectMatrix, float scaleFactor);

};

#endif // SCENECONTROLLER_H
