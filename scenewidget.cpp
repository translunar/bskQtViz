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
#include "scenewidget.h"

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QCoreApplication>

SceneWidget::SceneWidget(QWidget *parent, SimDataManager *simDataManager)
    : QOpenGLWidget(parent)
    , m_renderer(new Renderer(this, simDataManager))
{
    setFocusPolicy(Qt::StrongFocus);
    connect(m_renderer->camera(), SIGNAL(updateRequested()), this, SLOT(update()));
}

SceneWidget::~SceneWidget()
{
    cleanup();
}

QSize SceneWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize SceneWidget::sizeHint() const
{
    return QSize(400, 400);
}

QVector<shortcutPair> SceneWidget::getShortcuts()
{
    QVector<shortcutPair> sceneShortcuts;
    cameraFpvKeys fpvKeys = m_renderer->camera()->getFpvKeys();
    for(cameraFpvKeys::iterator i = fpvKeys.begin(); i != fpvKeys.end(); ++i) {
        sceneShortcuts.push_back(shortcutPair("FPV " + i.key(), i.value().key));
    }
    return sceneShortcuts;
}

void SceneWidget::cleanup()
{
    makeCurrent();
    m_renderer->cleanup();
    doneCurrent();
}

void SceneWidget::setCameraMode(int mode)
{
    m_renderer->camera()->setCameraMode((CameraMode)mode);
}

void SceneWidget::setTargetObject(int targetIndex)
{
    m_renderer->setTargetObject(targetIndex);
    update();
}

void SceneWidget::setZoomSpeed(double value)
{
    m_renderer->camera()->setZoomSpeed(value);
}

void SceneWidget::setPanSpeed(double value)
{
    m_renderer->camera()->setPanSpeed(value);
}

void SceneWidget::setFpvTranslateSpeed(double value)
{
    m_renderer->camera()->setFpvTranslateSpeed(value);
}

void SceneWidget::setFpvRotateSpeed(double value)
{
    m_renderer->camera()->setFpvRotateSpeed(value);
}

void SceneWidget::resetAll()
{
    m_renderer->camera()->resetCameraView();
    update();
}

void SceneWidget::setWireframe(bool value)
{
    m_renderer->setWireframe(value);
    update();
}

void SceneWidget::setCameraTargetVisible(bool value)
{
    m_renderer->setCameraTargetVisible(value);
    update();
}

void SceneWidget::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &SceneWidget::cleanup);
    m_renderer->initializeScene();
}

void SceneWidget::paintGL()
{
    m_renderer->renderScene();
}

void SceneWidget::resizeGL(int width, int height)
{
    m_renderer->camera()->setWindow(width, height);
}

void SceneWidget::mousePressEvent(QMouseEvent *event)
{
    m_renderer->camera()->mousePressEvent(event);
}

void SceneWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_renderer->camera()->mouseMoveEvent(event);
}

void SceneWidget::wheelEvent(QWheelEvent *event)
{
    m_renderer->camera()->wheelEvent(event);
}

void SceneWidget::keyPressEvent(QKeyEvent *event)
{
    m_renderer->camera()->keyPressEvent(event);
}

void SceneWidget::keyReleaseEvent(QKeyEvent *event)
{
    m_renderer->camera()->keyReleaseEvent(event);
}
