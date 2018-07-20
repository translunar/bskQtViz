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
//
//  thruster.cpp
//  qt_udp_development
//
//  Created by Patrick Kenneally on 5/18/15.
//
//

#include "thrusterGeometry.h"

ThrusterGeometry::ThrusterGeometry() 
    : Geometry()
    , m_numSlices(40)
{
    // Add default material
    QSharedPointer<MaterialInfo> material0 = getDefaultMaterial();
    
    // Create root node
    QSharedPointer<Node> root = getRootNode();
    
    QMatrix4x4 transform = QMatrix4x4();
    
    // Create conical frustum geometry
    Node cfNode;
    cfNode.name = "ConicalFrustum";
    cfNode.transformation.setToIdentity();
    cfNode.transformation.translate(0, 0, 0);

    m_x = QVector<float>(2,0);
    m_r = QVector<float>(2,0);
    m_x[1] = 1.0f;
    m_r[1] = 0.1f;

    // Draw a circular plane with hollow center at the cone end
    m_thrusterEndInnerMesh = QSharedPointer<Mesh>(new Mesh);
    transform.setToIdentity();
    transform.translate(m_x.at(1), 0, 0);
    defineCircularPlane(m_thrusterEndInnerMesh, m_r.at(1) - m_r.at(1)/10, m_r.at(1), m_numSlices, transform, false);
    m_thrusterEndInnerMesh->name = "ThrusterInnerEnd";
    m_thrusterEndInnerMesh->material = material0;
    //    m_thrusterEndMesh->textureFile = "images/plume.png";
    cfNode.meshes.push_back(m_thrusterEndInnerMesh);
    
    // Draw a circular plane with hollow center at the cone end
    m_thrusterEndOuterMesh = QSharedPointer<Mesh>(new Mesh);
    defineCircularPlane(m_thrusterEndOuterMesh, m_r.at(1) - m_r.at(1)/10, m_r.at(1), m_numSlices, transform);
    m_thrusterEndOuterMesh->name = "ThrusterOuterEnd";
    m_thrusterEndOuterMesh->material = material0;
    //    m_thrusterEndMesh->textureFile = "images/plume.png";
    cfNode.meshes.push_back(m_thrusterEndOuterMesh);
    
    // Draw a cone
    transform.translate(-m_x.at(1),0,0);
    m_thrusterInnerMesh = QSharedPointer<Mesh>(new Mesh);
    defineConicalFrustum(m_thrusterInnerMesh, m_x, m_r, m_numSlices, transform);
    m_thrusterInnerMesh->name = "ThrusterInner";
    m_thrusterInnerMesh->material = material0;
    m_thrusterInnerMesh->textureFile = ":/resources/images/thrusterFadex.png";
    cfNode.meshes.push_back(m_thrusterInnerMesh);
    
    m_thrusterOuterMesh = QSharedPointer<Mesh>(new Mesh);
    defineConicalFrustum(m_thrusterOuterMesh, m_x, m_r, m_numSlices, transform, false);
    m_thrusterOuterMesh->name = "ThrusterOuter";
    m_thrusterOuterMesh->material = material0;
    m_thrusterOuterMesh->textureFile = ":/resources/images/thrusterFadex.png";
    cfNode.meshes.push_back(m_thrusterOuterMesh);
    
    root->nodes.push_back(cfNode);
    
    // Set the buffers to dynamic so that its buffer gets updated
    setVertexBufferUsage(QOpenGLBuffer::StreamDraw);
//    setIndexBufferUsage(QOpenGLBuffer::StreamDraw);
}

ThrusterGeometry::~ThrusterGeometry()
{
    
}

void ThrusterGeometry::update(GeometryUpdateParameters *parameters)
{
    ThrusterGeometryUpdateParameters *p = (ThrusterGeometryUpdateParameters *)parameters;
    if(p) {
        m_x[1] = p->length;
        m_r[1] = p->plumeDiameter;
        QMatrix4x4 transform = QMatrix4x4();
        transform.translate(m_x.at(1), 0, 0);
        defineCircularPlane(m_thrusterEndInnerMesh, m_r.at(1) - m_r.at(1)/10
                            , m_r.at(1), m_numSlices, transform, false, true);
        defineCircularPlane(m_thrusterEndOuterMesh, m_r.at(1) - m_r.at(1)/10
                            , m_r.at(1), m_numSlices, transform, true, true);
        transform.translate(-m_x.at(1), 0, 0);
        defineConicalFrustum(m_thrusterInnerMesh, m_x, m_r, m_numSlices, transform, true, true);
        defineConicalFrustum(m_thrusterOuterMesh, m_x, m_r, m_numSlices, transform, false, true);
    }
}
