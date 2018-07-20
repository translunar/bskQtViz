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
#include "camera.h"

#include <math.h>

Camera::Camera()
    : m_cameraMode(CAMERA_ARCBALL)
    , m_eye(QVector3D(1, 0, 0))
    , m_target(QVector3D(0, 0, 0))
    , m_up(QVector3D(0, 0, 1))
    , m_targetDist(0.0)
    , m_targetDistInc(0.0)
    , m_verticalFov(30.0)
    , m_nearDist(1.0f)
    , m_farDist(0.55f*5000000.0f)
    , m_width(0)
    , m_height(0)
    , m_mouseDownX(0)
    , m_mouseDownY(0)
    , m_xAdj(0.0)
    , m_yAdj(0.0)
    , m_zoomDist(0.1)
    , m_panDist(0.1)
    , m_fpvTranslateSpeed(0.1)
    , m_fpvRotateSpeed(1)
    , m_fpvTimer(new QTimer(this))
{
    connect(m_fpvTimer, SIGNAL(timeout()), this, SLOT(updateFpvKeys()));
    m_fpvTimer->start(30); // timeout() signal emitted at each 30 msec

    m_fpvKeys.insert("Move Forward", CameraFpvKey(Qt::Key_W, false));
    m_fpvKeys.insert("Move Backward", CameraFpvKey(Qt::Key_S, false));
    m_fpvKeys.insert("Move Left", CameraFpvKey(Qt::Key_A, false));
    m_fpvKeys.insert("Move Right", CameraFpvKey(Qt::Key_D, false));
    m_fpvKeys.insert("Move Up", CameraFpvKey(Qt::Key_R, false));
    m_fpvKeys.insert("Move Down", CameraFpvKey(Qt::Key_F, false));
    m_fpvKeys.insert("Rotate Up", CameraFpvKey(Qt::Key_Down, false));
    m_fpvKeys.insert("Rotate Down", CameraFpvKey(Qt::Key_Up, false));
    m_fpvKeys.insert("Rotate Left", CameraFpvKey(Qt::Key_Left, false));
    m_fpvKeys.insert("Rotate Right", CameraFpvKey(Qt::Key_Right, false));
    m_fpvKeys.insert("Rotate CW", CameraFpvKey(Qt::Key_Q, false));
    m_fpvKeys.insert("Rotate CCW", CameraFpvKey(Qt::Key_E, false));
}

Camera::~Camera()
{
    delete m_fpvTimer;
}

void Camera::setWindow(int width, int height)
{
    m_width = width;
    m_height = height;
    m_xAdj = 2.0f / ((float)width);
    m_yAdj = 2.0f / ((float)height);

    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(m_verticalFov, float(width) / height, m_nearDist, m_farDist);
}

void Camera::initializeFrame(QVector3D eye, QVector3D target, QVector3D up)
{
    if(eye != target) {
        m_eye = eye;
        m_target = target;
    }
    if(up != (m_target - m_eye).normalized()) {
        m_up = up.normalized();
    }
    m_targetDist = (m_target - m_eye).length();
}

QMatrix4x4 Camera::getCameraMatrix()
{
    QMatrix4x4 m;
    m.lookAt(m_eye, m_target, m_up);
    return m_cameraMatrix * m;
}

QMatrix4x4 Camera::getOrthoMatrix()
{
    QMatrix4x4 m;
    m.ortho(QRect(0, 0, m_width, m_height));
    return m;
}

QVector3D Camera::getTargetPos()
{
    return QVector3D(0, 0, -(m_targetDist + m_targetDistInc));
}

void Camera::mousePressEvent(QMouseEvent *event) // QMouseEvent: event when a mouse button is pressed/released inside a widget
{
    m_mouseDownX = event->x(); // receiving Xposition of the mouse cursor, relative to the widget that received the event
    m_mouseDownY = event->y();
    emit updateRequested();
}

void Camera::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton) {
        switch(m_cameraMode) {
            case CAMERA_PAN:
                updatePanXY(event->x(), event->y());
                break;
            case CAMERA_FPV:
                updateFpvRotate(event->x(), event->y());
                break;
            case CAMERA_ARCBALL:
            default:
                updateArcball(event->x(), event->y());
                break;
        }
    } else if(event->buttons() & Qt::RightButton) {
        switch(m_cameraMode) {
            case CAMERA_PAN:
                updatePanZ(event->x(), event->y());
                break;
            case CAMERA_FPV:
                break;
            case CAMERA_ARCBALL:
            default:
                updateZoom(event->x(), event->y());
                break;
        }
    }
    emit updateRequested();
}

void Camera::wheelEvent(QWheelEvent *event)
{
    double distInc = event->delta() * m_zoomDist;
    switch(m_cameraMode) {
        case CAMERA_FPV:
            break;
        case CAMERA_ARCBALL:
        case CAMERA_PAN:
        default:
            updateZoom(distInc);
            break;
    }
    emit updateRequested();
}

void Camera::keyPressEvent(QKeyEvent *event)
{
    // Only trigger if shift/control/alt/meta are not pressed
    if(event->modifiers() == 0) {
        if(m_cameraMode == CAMERA_FPV) {
            for(cameraFpvKeys::iterator i = m_fpvKeys.begin(); i != m_fpvKeys.end(); ++i) {
                if(static_cast<Qt::Key>(event->key()) == i.value().key) {
                    i.value().isToggled = true;
                    break;
                }
            }
        }
    }
    emit updateRequested();
}

void Camera::keyReleaseEvent(QKeyEvent *event)
{
    // Only trigger if shift/control/alt/meta are not pressed
    if(event->modifiers() == 0) {
        if(m_cameraMode == CAMERA_FPV) {
            for(cameraFpvKeys::iterator i = m_fpvKeys.begin(); i != m_fpvKeys.end(); ++i) {
                if(static_cast<Qt::Key>(event->key()) == i.value().key) {
                    i.value().isToggled = false;
                    break;
                }
            }
        }
    }
    emit updateRequested();
}

void Camera::resetCameraView()
{
    m_cameraMatrix = QMatrix4x4();
    m_targetDistInc = 0.0;
}

void Camera::updateArcball(const int mousex, const int mousey)
{
    QVector3D v0 = mapToSphere(m_mouseDownX, m_mouseDownY).normalized();
    QVector3D v1 = mapToSphere(mousex, mousey).normalized();
    QVector3D v2 = (v0 + v1).normalized();
    QVector3D axis = QVector3D::crossProduct(v0, v2);
    double cos = QVector3D::dotProduct(v0, v2);
    QQuaternion q = QQuaternion(cos, axis).normalized();
    QMatrix4x4 m;
    m.rotate(q); // multiplies Qmatrix m by another that rotates coordinates according to QQuaternion q 

    // Translate to target, apply rotation and reverse movement
    QVector3D v(0, 0, m_targetDist + m_targetDistInc);
    QMatrix4x4 mb;
    mb.translate(v);
    QMatrix4x4 mnb;
    mnb.translate(-v);
    m_cameraMatrix = (mnb * m * mb) * m_cameraMatrix;

    m_mouseDownX = mousex;
    m_mouseDownY = mousey;
}

QVector3D Camera::mapToSphere(int x, int y)
{
    QVector3D tmp = QVector3D();
    // Adjust mouse coord to [-1 ... 1]
    tmp.setX((double)(x * m_xAdj) - 1.0);
    tmp.setY(1.0 - (double)(y * m_yAdj));

    if(tmp.lengthSquared() > 1.0) {
        // point outside sphere, normalize to point on sphere
        tmp.normalize();
    } else {
        // point mapped inside sphere
        tmp.setZ(sqrt(1.0 - tmp.lengthSquared()));
    }
    return tmp;
}

void Camera::updateZoom(const int mousex, const int mousey)
{
    double distInc = (double)(m_mouseDownY - mousey) * m_zoomDist;
    updateZoom(distInc);

    m_mouseDownX = mousex;
    m_mouseDownY = mousey;
}

void Camera::updateZoom(double distInc)
{
    double currentDist = m_targetDist + m_targetDistInc;
    distInc = std::min(currentDist, distInc);

    QMatrix4x4 m;
    m.translate(0, 0, distInc);
    m_targetDistInc -= distInc;
    m_cameraMatrix = m * m_cameraMatrix;
}

void Camera::updatePanXY(const int mousex, const int mousey)
{
    QVector3D panInc(-(m_mouseDownX - mousex), m_mouseDownY - mousey, 0);

    QMatrix4x4 m;
    m.translate(panInc * m_panDist);
    m_cameraMatrix = m * m_cameraMatrix;

    m_mouseDownX = mousex;
    m_mouseDownY = mousey;
}

void Camera::updatePanZ(const int mousex, const int mousey)
{
    QVector3D panInc(0, 0, m_mouseDownY - mousey);

    QMatrix4x4 m;
    m.translate(panInc * m_panDist);
    m_cameraMatrix = m * m_cameraMatrix;

    m_mouseDownX = mousex;
    m_mouseDownY = mousey;
}

void Camera::updateFpvRotate(int mousex, int mousey)
{
    // Apply x rotation
    QVector3D v0 = mapToSphere(m_width / 2, m_height / 2).normalized();
    QVector3D v1 = mapToSphere(m_width / 2 + (mousex - m_mouseDownX), m_height / 2).normalized();
    QVector3D v2 = (v0 + v1).normalized();
    QVector3D axis = QVector3D::crossProduct(v0, v2);
    double cos = QVector3D::dotProduct(v0, v2);
    QQuaternion q = QQuaternion(cos, axis).normalized();
    QMatrix4x4 mX;
    mX.rotate(q);

    // Apply y rotation
    v1 = mapToSphere(m_width / 2, m_height / 2 + (mousey - m_mouseDownY)).normalized();
    v2 = (v0 + v1).normalized();
    axis = QVector3D::crossProduct(v0, v2);
    cos = QVector3D::dotProduct(v0, v2);
    q = QQuaternion(cos, axis).normalized();
    QMatrix4x4 mY;
    mY.rotate(q);

    m_cameraMatrix = mY * mX * m_cameraMatrix;

    m_mouseDownX = mousex;
    m_mouseDownY = mousey;
}

void Camera::updateFpvKeys()
{
    if(m_cameraMode == CAMERA_FPV) {
        QVector3D delta;
        if(m_fpvKeys["Move Forward"].isToggled) {
            delta.setZ(delta.z() + m_fpvTranslateSpeed);
        }
        if(m_fpvKeys["Move Backward"].isToggled) {
            delta.setZ(delta.z() - m_fpvTranslateSpeed);
        }
        if(m_fpvKeys["Move Right"].isToggled) {
            delta.setX(delta.x() - m_fpvTranslateSpeed);
        }
        if(m_fpvKeys["Move Left"].isToggled) {
            delta.setX(delta.x() + m_fpvTranslateSpeed);
        }
        if(m_fpvKeys["Move Up"].isToggled) {
            delta.setY(delta.y() - m_fpvTranslateSpeed);
        }
        if(m_fpvKeys["Move Down"].isToggled) {
            delta.setY(delta.y() + m_fpvTranslateSpeed);
        }
        QMatrix4x4 m;
        m.translate(delta);
        m_cameraMatrix = m * m_cameraMatrix;

        delta = QVector3D();
        m = QMatrix4x4();
        if(m_fpvKeys["Rotate Up"].isToggled) {
            delta.setX(delta.x() + m_fpvRotateSpeed);
        }
        if(m_fpvKeys["Rotate Down"].isToggled) {
            delta.setX(delta.x() - m_fpvRotateSpeed);
        }
        if(m_fpvKeys["Rotate Right"].isToggled) {
            delta.setY(delta.y() + m_fpvRotateSpeed);
        }
        if(m_fpvKeys["Rotate Left"].isToggled) {
            delta.setY(delta.y() - m_fpvRotateSpeed);
        }
        if(m_fpvKeys["Rotate CW"].isToggled) {
            delta.setZ(delta.z() + m_fpvRotateSpeed);
        }
        if(m_fpvKeys["Rotate CCW"].isToggled) {
            delta.setZ(delta.z() - m_fpvRotateSpeed);
        }
        m.rotate(delta.x(), 1, 0, 0);
        m.rotate(delta.y(), 0, 1, 0);
        m.rotate(delta.z(), 0, 0, 1);
        m_cameraMatrix = m * m_cameraMatrix;

        emit updateRequested();
    }
}
