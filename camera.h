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
#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QTimer>

enum CameraMode { CAMERA_ARCBALL, CAMERA_PAN, CAMERA_FPV };

// Helper class for keeping track of FPV keys
class CameraFpvKey {
public:
    Qt::Key key;
    bool isToggled;

    CameraFpvKey() {}
    CameraFpvKey(Qt::Key key, bool isToggled)
        : key(key)
        , isToggled(isToggled)
    {}
};
typedef QHash<QString, CameraFpvKey> cameraFpvKeys;

class Camera : public QObject
{
    Q_OBJECT
public:
    Camera();
    ~Camera();

    void setWindow(int width, int height);
    int getWidth() {
        return m_width;
    }
    int getHeight() {
        return m_height;
    }
    void initializeFrame(QVector3D eye, QVector3D target, QVector3D up);

    QMatrix4x4 getCameraMatrix();
    QMatrix4x4 getProjectionMatrix() {
        return m_projectionMatrix;
    }
    QMatrix4x4 getOrthoMatrix();
    QVector3D getTargetPos();
    cameraFpvKeys getFpvKeys() {
        return m_fpvKeys;
    }

    // Make sure the following events are called from the parent widget
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void setCameraMode(CameraMode mode) {
        m_cameraMode = mode;
    }
    void setNearDistance(float value) {
        m_nearDist = value;
    }
    void setFarDistance(float value) {
        m_farDist = value;
    }
    float getFarDistance() {
        return m_farDist;
    }
    void resetCameraView();
    void setZoomSpeed(double value) {
        m_zoomDist = value;
    }
    void setPanSpeed(double value) {
        m_panDist = value;
    }
    void setFpvTranslateSpeed(double value) {
        m_fpvTranslateSpeed = value;
    }
    void setFpvRotateSpeed(double value) {
        m_fpvRotateSpeed = value;
    }

signals:
    void updateRequested();

private:
    CameraMode m_cameraMode;
    cameraFpvKeys m_fpvKeys;

    // Keep track of what we are looking at from where
    QVector3D m_eye;
    QVector3D m_target;
    QVector3D m_up;
    double m_targetDist;
    double m_targetDistInc;
    double m_verticalFov;
    // Note: As the near clipping plane is set closer to zero, the effective
    // precision of the depth buffer decreases dramatically. Moving the far
    // clipping plane further away from the eye has a negative impact on depth
    // buffer precision, but its not as dramatic as moving the near plane.
    double m_nearDist;
    double m_farDist;

    // Drawing transformation matrices
    QMatrix4x4 m_cameraMatrix;
    QMatrix4x4 m_projectionMatrix;

    // Window width and height
    int m_width;
    int m_height;

    // Store locations of mouse at time of mousepress
    int m_mouseDownX;
    int m_mouseDownY;

    // Arcball
    void updateArcball(const int mousex, const int mousey);
    // Mouse adjustment factor, adjust to [0 2]
    float m_xAdj;
    float m_yAdj;
    // Map mouse position to vector on sphere
    QVector3D mapToSphere(int x, int y);

    // Zoom
    void updateZoom(const int mousex, const int mousey);
    void updateZoom(double distInc);
    // Distance to zoom for each pixel moved by mouse
    double m_zoomDist;

    // Pan
    void updatePanXY(const int mousex, const int mousey);
    void updatePanZ(const int mousex, const int mousey);
    // Distance to pan for each pixel moved by mouse
    double m_panDist;

    // FPV
    // Rotate the camera target
    void updateFpvRotate(int mousex, int mousey);
    // Speed for adjusting fpv movement
    double m_fpvTranslateSpeed;
    double m_fpvRotateSpeed;
    QTimer *m_fpvTimer;

private slots:
    // Translate the camera eye and target along view vector
    void updateFpvKeys();

};

#endif // CAMERA_H
