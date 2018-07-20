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
#include "renderer.h"

#include <QImage>
#include <QOpenGLWidget>

#include "cameratarget.h"
#include "starfield.h"
#include "linestrip.h"
extern "C" {
#include "utilities/rigidBodyKinematics.h"
}

Renderer::Renderer(QObject *parent, SimDataManager *simDataManager)
    : QObject(parent)
    , m_simDataManager(simDataManager)
    , m_geometryManager(new GeometryManager(this))
    , m_lightShader(0)
    , m_watermarkShader(0)
    , m_useWireframe(false)
    , m_watermarkFile(":/resources/images/Basilisk-Logo.png")
    , m_showCameraTarget(false)
    , m_lightPosition(QVector3D(10, 0, 0))
    , m_lightIntensity(QVector3D(1, 1, 1))
{
    QSharedPointer<CameraTarget> cameraTarget(new CameraTarget);
    m_geometryManager->addGeometry("CameraTarget", cameraTarget);
    QSharedPointer<Starfield> starfield(new Starfield);
    m_geometryManager->addGeometry("Starfield", starfield);
    m_geometryManager->addGeometry("Spacecraft", "models/bskSpacecraft.stl");
}

Renderer::~Renderer()
{
    cleanup();
}

void Renderer::initializeScene()
{
    initializeOpenGLFunctions();
    initializeShaderPrograms();
    // Sim objects are created on the fly as needed
    initializeWatermark();

    glClearColor(0, 0, 0, 1);
    glLineWidth(2.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
}

void Renderer::renderScene()
{
    calculateCameraPosition();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_lightShader->bind();
    m_lightShader->setUniformValue("lightPosition_worldSpace", m_simDataManager->getLightPosition());
    m_lightShader->setUniformValue("lightIntensity", m_lightIntensity);
    m_lightShader->setUniformValue("projectionMatrix", m_camera.getProjectionMatrix());
    glPolygonMode(GL_FRONT_AND_BACK, m_useWireframe ? GL_LINE : GL_FILL);
    drawScene(m_lightShader);
    m_lightShader->release();

    drawWatermark();
}

void Renderer::cleanup()
{
    cleanupShaderPrograms();
    m_geometryManager->cleanupGeometries();
    m_geometryManager->cleanupTextures();
}

void Renderer::setTargetObject(int targetIndex)
{
    if(m_targetObjectIndex != targetIndex) {
        m_targetObjectIndex = targetIndex;
        // Setting the target adjusts the zoom distance, so reset the view
        // to avoid weird things happening
        m_camera.resetCameraView();
    }
}

void Renderer::setWireframe(bool value)
{
    m_useWireframe = value;
}

void Renderer::setCameraTargetVisible(bool value)
{
    m_showCameraTarget = value;
}

void Renderer::initializeShaderPrograms()
{
    cleanupShaderPrograms();

    // Create shader for lighting scene
    m_lightShader = new QOpenGLShaderProgram;
    m_lightShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/lightingVertexShader.glsl");
    m_lightShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/lightingFragmentShader.glsl");
    m_lightShader->bindAttributeLocation("vertexPosition_modelSpace", 0);
    m_lightShader->bindAttributeLocation("vertexNormal_modelSpace", 1);
    m_lightShader->bindAttributeLocation("vertexUV", 2);
    m_lightShader->link();

    // Create shader for rendering 2D watermark
    m_watermarkShader = new QOpenGLShaderProgram;
    m_watermarkShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/watermarkVertexShader.glsl");
    m_watermarkShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/watermarkFragmentShader.glsl");
    m_watermarkShader->bindAttributeLocation("vertex", 0);
    m_watermarkShader->bindAttributeLocation("vertexUV", 1);
    m_watermarkShader->link();
}

void Renderer::cleanupShaderPrograms()
{
    if(m_lightShader) {
        delete m_lightShader;
        m_lightShader = 0;
    }
    if(m_watermarkShader) {
        delete m_watermarkShader;
        m_watermarkShader = 0;
    }
}

bool Renderer::initializeSimObject(QOpenGLShaderProgram *program, SimObject simObject)
{
    if(!m_geometryManager->initializeGeometry(simObject.geometryName, program)) {
        return false;
    }
    for(int i = 0; i < simObject.simObjects.length(); i++) {
        if(!initializeSimObject(program, simObject.simObjects.at(i))) {
            return false;
        }
    }
    return true;
}

void Renderer::initializeWatermark()
{
    m_geometryManager->addTexture(m_watermarkFile);
}

void Renderer::drawWatermark()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // Adjust these parameters to change size and opacity
    float width = m_camera.getWidth() * 1.0f / 6.0f;
    float height = width * 669.0f / 1066.0f;
    float x0 = m_camera.getWidth() * 0.05f;
    float y0 = m_camera.getHeight() * 0.95f - height;
    float alpha = 0.50f;

    GLfloat const vertices[] = {
        x0,  y0,  0.0f,
        x0 + width, y0, 0.0f,
        x0 + width,  y0 + height, 0.0f,
        x0,  y0,  0.0f,
        x0 + width,  y0 + height, 0.0f,
        x0, y0 + height, 0.0f,
    };
    GLfloat const uv[] = {
        0.0, 1.0,
        1.0, 1.0,
        1.0, 0.0,
        0.0, 1.0,
        1.0, 0.0,
        0.0, 0.0
    };
    m_watermarkShader->bind();
    m_watermarkShader->enableAttributeArray(0);
    m_watermarkShader->setAttributeArray(0, vertices, 3);
    m_watermarkShader->enableAttributeArray(1);
    m_watermarkShader->setAttributeArray(1, uv, 2);
    m_watermarkShader->setUniformValue("projectionMatrix", m_camera.getOrthoMatrix());
    m_watermarkShader->setUniformValue("color", QVector4D(1, 1, 1, alpha));

    QSharedPointer<QOpenGLTexture> texture = m_geometryManager->getTexture(m_watermarkFile);
    glActiveTexture(GL_TEXTURE0);
    texture->bind(0);
    m_watermarkShader->setUniformValue("textureId", 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    texture->release();

    m_watermarkShader->disableAttributeArray(0);
    m_watermarkShader->release();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void Renderer::calculateCameraPosition()
{
    // Get camera position based on current simulation object positions
    // TODO: Adjust this to take into account whether to give position relative to body, orbit, or intertial frames
    QVector<SimObject> simObjects;
    QVector3D target(0, 0, 0);
    QVector3D eye(1, 0, 0);
    if(m_simDataManager != 0) {
        simObjects = m_simDataManager->getSimObjects();
        if(!simObjects.empty()) {
            target = simObjects.at(m_simDataManager->getTargetObject()).position;
            float dist = 10.0;
            float temp = m_geometryManager->getGeometryBoundingRadii(simObjects.at(m_simDataManager->getTargetObject()).geometryName);
            temp *= simObjects.at(m_simDataManager->getTargetObject()).scaleFactor;
            if(temp > 0.0) {
                dist = temp * 3.0f;
            }
            eye = QVector3D(target.x() + dist,
                            target.y() + 0,
                            target.z() + 0);
        }
    }
    m_camera.initializeFrame(eye, target, QVector3D(0, 0, 1));
}

void Renderer::drawScene(QOpenGLShaderProgram *program)
{
    // TODO: Note that drawing order can matter significantly when attempting
    // to implement transparency. Ideally one would do some sort of depth or
    // transparency sorting of all the triangles in the scene and then render
    // them in order in a second path. Lots of stuff online, but not enough
    // time to worry about that now
    QMatrix4x4 cameraMatrix = m_camera.getCameraMatrix();

    // Draw the starfield
    SimObject starfield;
    starfield.geometryName = "Starfield";
    starfield.position = cameraMatrix.inverted().map(QVector3D(0, 0, 0));
    starfield.quaternion = QQuaternion();
    // TODO: The scalefactor on the star field may need to be adjusted if the
    // far depth of the scene changes?
    starfield.scaleFactor = m_camera.getFarDistance() * 0.75f;
    drawSceneObject(program, starfield, cameraMatrix);

    // Draw the camera target
    if(m_showCameraTarget) {
        SimObject cameraTarget;
        cameraTarget.geometryName = "CameraTarget";
        QVector3D targetPos = m_camera.getTargetPos();
        cameraTarget.position = cameraMatrix.inverted().map(targetPos);
        cameraTarget.quaternion = QQuaternion();
        cameraTarget.scaleFactor = 0.01f * -targetPos.z();
        drawSceneObject(program, cameraTarget, cameraMatrix);
    }

    // Draw the simulation objects
    if(m_simDataManager != 0) {
        QVector<SimObject> simObjects = m_simDataManager->getSimObjects();
        for(int i = 0; i < simObjects.length(); i++) {
            drawSceneObject(program, simObjects.at(i), cameraMatrix);
        }
    }
}

void Renderer::drawSceneObject(QOpenGLShaderProgram *program, SimObject simObject,
                               QMatrix4x4 cameraMatrix)
{
    // If the object is outside the star field, move it relative to the camera
    // and resize it as necessary
    float scaleFactor = simObject.scaleFactor;

    QVector3D objectPos = simObject.position;
    QVector3D cameraPos = cameraMatrix.inverted().map(QVector3D());
    QVector3D objectDir = objectPos - cameraPos;
    float objectDist = objectDir.length();
    // TODO: The scale factor on the farDist is to make it so the object does
    // not get cut off by the starfield
    float farDist = m_camera.getFarDistance() * 0.6f;
    if(objectDist > farDist) {
        float newDist = farDist;
        float newScaleFactor = newDist / objectDist;
        scaleFactor *= newScaleFactor;
        objectPos = cameraPos + objectDir.normalized() * newDist;
    }
    simObject.position = objectPos;
    simObject.scaleFactor = scaleFactor;

    // Only perform check on top parent object, and then draw the rest normally
    drawSceneObjectRecursion(program, simObject, cameraMatrix, QMatrix4x4(), 1.0f);
}

void Renderer::drawSceneObjectRecursion(QOpenGLShaderProgram *program, SimObject simObject,
                                        QMatrix4x4 cameraMatrix, QMatrix4x4 objectMatrix, float scaleFactor)
{
    QMatrix4x4 m;
    m.translate(simObject.position);
    m.rotate(simObject.quaternion);

    scaleFactor *= simObject.scaleFactor;
    objectMatrix *= m;

    m_geometryManager->initializeGeometry(simObject.geometryName, program);
    m_geometryManager->drawGeometry(simObject.geometryName, program, cameraMatrix, objectMatrix, scaleFactor,
                                    simObject.defaultMaterialOverride, simObject.updateParameters);

    // Recursively draw this sceneObject's child sceneObjects
    for(int i = 0; i < simObject.simObjects.size(); i++) {
        float childScaleFactor = scaleFactor;
        if(simObject.simObjects.at(i).scaleByParentBoundingRadii) {
            childScaleFactor *= m_geometryManager->getGeometryBoundingRadii(simObject.geometryName);
        }
        drawSceneObjectRecursion(program, simObject.simObjects.at(i), cameraMatrix, objectMatrix, childScaleFactor);
    }
}

