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
#include "startrackerfov.h"

StarTrackerFOV::StarTrackerFOV() : Geometry()
    , m_numSlices(40)
{
    QSharedPointer<MaterialInfo> material0 = getDefaultMaterial();
    QSharedPointer<Node> root = getRootNode();
    QMatrix4x4 transform = QMatrix4x4();
    
    // Create field of view geometry
    Node sNode;
    sNode.name = "RectangularPrismoid";
    sNode.transformation.setToIdentity();
    sNode.transformation.translate(0, 0, 0);

    m_x = QVector<float>(2,0);
    m_r = QVector<float>(2,0);
    
    double scaleFOV = 2.0;
    m_x[1] = 2.5 * scaleFOV;
    m_r[1] = 1.0 * scaleFOV;
    
    // Draw a rectangular prismoid
    /*m_fovOuterConeMesh = QSharedPointer<Mesh>(new Mesh);
    defineRectangularPrismoid(m_fovOuterConeMesh, m_x, m_r, m_r, transform, false, false);
    m_fovOuterConeMesh->name = "StFovOuterConeMesh";
    m_fovOuterConeMesh->material = material0;
    sNode.meshes.push_back(m_fovOuterConeMesh);
    
    m_fovInnerConeMesh = QSharedPointer<Mesh>(new Mesh);
    defineRectangularPrismoid(m_fovInnerConeMesh, m_x, m_r, m_r, transform, true, false);
    m_fovInnerConeMesh->name = "StFovInnerConeMesh";
    m_fovInnerConeMesh->material = material0;
    sNode.meshes.push_back(m_fovInnerConeMesh);*/
    
    
    
    // Draw vectors from ST-frame origin to the vertex of the rectanglular fov
    float length = m_x[1];
    
    QSharedPointer<Mesh> lineV1(new Mesh);
    QVector<QVector3D> pointsV1;
    pointsV1.push_back(QVector3D(0, 0, 0));
    pointsV1.push_back(QVector3D(length, m_r[1] / 2, m_r[1] / 2));
    defineLine(lineV1, pointsV1, transform, false);
    lineV1->material = getDefaultMaterial();
    sNode.meshes.push_back(lineV1);
    
    QSharedPointer<Mesh> lineV2(new Mesh);
    QVector<QVector3D> pointsV2;
    pointsV2.push_back(QVector3D(0, 0, 0));
    pointsV2.push_back(QVector3D(length, m_r[1] / 2, - m_r[1] / 2));
    defineLine(lineV2, pointsV2, transform, false);
    lineV2->material = getDefaultMaterial();
    sNode.meshes.push_back(lineV2);
    
    QSharedPointer<Mesh> lineV3(new Mesh);
    QVector<QVector3D> pointsV3;
    pointsV3.push_back(QVector3D(0, 0, 0));
    pointsV3.push_back(QVector3D(length, - m_r[1] / 2, m_r[1] / 2));
    defineLine(lineV3, pointsV3, transform, false);
    lineV3->material = getDefaultMaterial();
    sNode.meshes.push_back(lineV3);
    
    QSharedPointer<Mesh> lineV4(new Mesh);
    QVector<QVector3D> pointsV4;
    pointsV4.push_back(QVector3D(0, 0, 0));
    pointsV4.push_back(QVector3D(length, - m_r[1] / 2, - m_r[1] / 2));
    defineLine(lineV4, pointsV4, transform, false);
    lineV4->material = getDefaultMaterial();
    sNode.meshes.push_back(lineV4);
    
    QSharedPointer<Mesh> line1(new Mesh);
    QVector<QVector3D> points1;
    points1.push_back(QVector3D(length, m_r[1] / 2, m_r[1] / 2));
    points1.push_back(QVector3D(length, - m_r[1] / 2, m_r[1] / 2));
    defineLine(line1, points1, transform, false);
    line1->material = getDefaultMaterial();
    sNode.meshes.push_back(line1);
    
    QSharedPointer<Mesh> line2(new Mesh);
    QVector<QVector3D> points2;
    points2.push_back(QVector3D(length, - m_r[1] / 2, m_r[1] / 2));
    points2.push_back(QVector3D(length, - m_r[1] / 2, - m_r[1] / 2));
    defineLine(line2, points2, transform, false);
    line2->material = getDefaultMaterial();
    sNode.meshes.push_back(line2);
    
    QSharedPointer<Mesh> line3(new Mesh);
    QVector<QVector3D> points3;
    points3.push_back(QVector3D(length, - m_r[1] / 2, - m_r[1] / 2));
    points3.push_back(QVector3D(length, m_r[1] / 2, - m_r[1] / 2));
    defineLine(line3, points3, transform, false);
    line3->material = getDefaultMaterial();
    sNode.meshes.push_back(line3);
    
    QSharedPointer<Mesh> line4(new Mesh);
    QVector<QVector3D> points4;
    points4.push_back(QVector3D(length, m_r[1] / 2, - m_r[1] / 2));
    points4.push_back(QVector3D(length, m_r[1] / 2, m_r[1] / 2));
    defineLine(line4, points4, transform, false);
    line4->material = getDefaultMaterial();
    sNode.meshes.push_back(line4);
    
    // Draw rectangular planes at the end cone
    m_fovOuterPlaneMesh = QSharedPointer<Mesh>(new Mesh);
    transform.translate(m_x[1], 0, 0);
    defineRectangularPlane(m_fovOuterPlaneMesh, 0, m_r[1], 0, m_r[1], transform, true, false);
    m_fovOuterPlaneMesh->name = "OuterPlaneMesh";
    m_fovOuterPlaneMesh->material = material0;
    sNode.meshes.push_back(m_fovOuterPlaneMesh);
    
    /*transform.rotate(M_PI, QVector3D(m_x[1],0,0));
    transform.translate(-m_x[1]/2, 0, 0);
    m_fovInnerPlaneMesh = QSharedPointer<Mesh>(new Mesh);
    defineRectangularPlane(m_fovInnerPlaneMesh, 0, m_r[1], 0, m_r[1], transform, false, false);
    m_fovInnerPlaneMesh->name = "InnerPlaneMesh";
    m_fovInnerPlaneMesh->material = material0;
    sNode.meshes.push_back(m_fovInnerPlaneMesh);*/
    
    
    root->nodes.push_back(sNode);
    setVertexBufferUsage(QOpenGLBuffer::StreamDraw);
}

StarTrackerFOV::~StarTrackerFOV() {}

