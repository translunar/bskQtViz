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
//  fieldOfView.cpp
//  Visualization
//
//  Created by Patrick Kenneally on 6/24/15.
//
//

#include "fieldOfView.h"

FieldOfView::FieldOfView() : Geometry()
        , m_x(1)
        , m_r(1)
        , m_numSlices(180)
{
    // Add default material
    QSharedPointer<MaterialInfo> material0 = getDefaultMaterial();
    
    // Create root node
    QSharedPointer<Node> root = getRootNode();
    QMatrix4x4 transform = QMatrix4x4();
    
    // Create field of view geometry
    Node sNode;
    sNode.name = "SteradianCone";
    sNode.transformation.setToIdentity();
    sNode.transformation.translate(0, 0, 0);

    m_x = QVector<float>(2,0);
    m_r = QVector<float>(2,0);
    m_x[1] = cos(M_PI/2);
    m_r[1] = sin(M_PI/2);

    m_fieldOfViewMesh = QSharedPointer<Mesh>(new Mesh);
    defineSteradianPortionSphere(m_fieldOfViewMesh, 180, 1, 2, transform, true, false);
    m_fieldOfViewMesh->name = "FieldOfView";
    m_fieldOfViewMesh->material = material0;
    sNode.meshes.push_back(m_fieldOfViewMesh);
    
    // Draw a circular plane at the end cone
    m_fieldOfViewConeMesh = QSharedPointer<Mesh>(new Mesh);
    defineConicalFrustum(m_fieldOfViewConeMesh, m_x, m_r, m_numSlices, transform);
    m_fieldOfViewConeMesh->name = "FieldOfViewCone";
    m_fieldOfViewConeMesh->material = material0;
    sNode.meshes.push_back(m_fieldOfViewConeMesh);
    
    root->nodes.push_back(sNode);
}

FieldOfView::FieldOfView(float angle) : FieldOfView() {}

FieldOfView::~FieldOfView() {}

void FieldOfView::update(GeometryUpdateParameters *parameters)
{
    // @TODO determine how to dynamically updated field of view angle
//    FieldOfViewUpdateParameters *p = (FieldOfViewUpdateParameters *)parameters;
//    if(p) {
//        defineSteradianCone(m_fieldOfViewMesh, p->angle, 1, 1, 2, QMatrix4x4(), true, false);
//    }
}
