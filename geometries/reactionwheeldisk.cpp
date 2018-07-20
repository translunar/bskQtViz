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
#include "reactionwheeldisk.h"
#include <iostream>

ReactionWheelDisk::ReactionWheelDisk()
	: Geometry()
	, m_numSlices(40)
    , m_radiusDisk(0.3)
    , m_thicknessDisk(0.04)
    , m_outerDiskPosition(-1.0)
    , m_lengthArrowLine(0.35)
{
	QSharedPointer<MaterialInfo> material0 = getDefaultMaterial();
    QSharedPointer<Node> root = getRootNode();
    QMatrix4x4 transform = QMatrix4x4();
    QMatrix4x4 transformArrow = QMatrix4x4();

    Node dNode;
    dNode.name = "ConicalFrustum";
    dNode.transformation.setToIdentity();
    dNode.transformation.translate(0,0,0);

    m_x = QVector<float>(2,0);
    m_r = QVector<float>(2,0);
    
    // Draw arrow lines
    m_x[1] = -0.75;
    m_r[0] = 0.007f;
    m_r[1] = 0.007f;
    m_ArrowLineMesh = QSharedPointer<Mesh>(new Mesh);
    defineConicalFrustum(m_ArrowLineMesh, m_x, m_r, m_numSlices, transformArrow, true, false);
    m_ArrowLineMesh->name = "ArrowLineRW";
    m_ArrowLineMesh->material = material0;
    dNode.meshes.push_back(m_ArrowLineMesh);
    
    // Draw arrows
    m_x[0] = 0.2f; //0.1f;
    m_r[0] = 0.04f; //0.03f;
    transformArrow.translate(-1.5,0,0);
    m_arrowMesh = QSharedPointer<Mesh>(new Mesh);
    defineConicalFrustum(m_arrowMesh, m_x, m_r, m_numSlices, transformArrow, true, false);
    m_arrowMesh->name = "ArrowRW";
    m_arrowMesh->material = material0;
    dNode.meshes.push_back(m_arrowMesh);
    
    
	// Draw a cilinder: radius = 3, length = 0.04
    m_r[0] = m_radiusDisk;
    m_r[1] = m_radiusDisk;
    m_x[0] = 0.0;
    m_x[1] = m_x[0] - m_thicknessDisk;
    transform.translate(0,0,0);
    transform.translate(m_outerDiskPosition,0,0);
	m_cilinderMesh = QSharedPointer<Mesh>(new Mesh);
	defineConicalFrustum(m_cilinderMesh, m_x, m_r, m_numSlices, transform);
	m_cilinderMesh->name = "CilinderRW";
	m_cilinderMesh->material = material0;
	dNode.meshes.push_back(m_cilinderMesh);
    
    // Draw disks
    m_InnerDiskMesh = QSharedPointer<Mesh>(new Mesh);
    defineCircularPlane(m_InnerDiskMesh, 0, m_r.at(0), m_numSlices, transform, false);
    m_InnerDiskMesh->name = "InnerDiskMeshRW";
    m_InnerDiskMesh->material = material0;
    dNode.meshes.push_back(m_InnerDiskMesh);
    
    m_OuterDiskMesh = QSharedPointer<Mesh>(new Mesh);
    transform.translate(m_x.at(1), 0, 0);
    defineCircularPlane(m_OuterDiskMesh, 0, m_r.at(0), m_numSlices, transform, true);
    m_OuterDiskMesh->name = "OuterDiskMeshRW";
    m_OuterDiskMesh->material = material0;
    dNode.meshes.push_back(m_OuterDiskMesh);

	root->nodes.push_back(dNode);
    
	setVertexBufferUsage(QOpenGLBuffer::StreamDraw);
}

ReactionWheelDisk::~ReactionWheelDisk(){}

void ReactionWheelDisk::update(GeometryUpdateParameters *parameters)
{
    ReactionWheelDiskUpdateParameters *p = (ReactionWheelDiskUpdateParameters *) parameters;
    if(p)
    {
        /* Update Disk position
        m_r[0] = m_radiusDisk;
        m_r[1] = m_radiusDisk;
        m_x[0] = 0.0;
        m_x[1] = m_x[0] - m_thicknessDisk;
        
        if (p->flag == true)
        {
            QMatrix4x4 transformDisk = QMatrix4x4();
            transformDisk.translate(m_outerDiskPosition - m_lengthArrowLine, 0, 0);
            
            defineConicalFrustum(m_cilinderMesh, m_x, m_r, m_numSlices, transformDisk, true, true);
            defineCircularPlane(m_InnerDiskMesh, 0, m_r.at(0), m_numSlices, transformDisk, true, true);
            transformDisk.translate(m_x.at(1), 0, 0);
            defineCircularPlane(m_OuterDiskMesh, 0, m_r.at(0), m_numSlices, transformDisk, true, true);
        }*/
        
        
        // Update Arrow pointing
        QMatrix4x4 transformArrow = QMatrix4x4();
        m_x[0] = 0.0f;
        m_r[0] = 0.04f;
        m_x[1] = 0.2f;
        m_r[1] = 0.0f;
        
        transformArrow.translate(m_outerDiskPosition, 0, 0);
        if (p->flag == true) // positive velocity
        {
            transformArrow.translate(m_lengthArrowLine, 0, 0);
        } else {
            transformArrow.translate(- m_lengthArrowLine - m_thicknessDisk , 0, 0);
            m_x[1] = - m_x[1];
        }
        
        defineConicalFrustum(m_arrowMesh, m_x, m_r, m_numSlices, transformArrow, true, true);
        
        // Update Arrow Line position
        QMatrix4x4 transformLineArrow = QMatrix4x4();
        m_x[0] = 0.0f;
        m_x[1] = m_lengthArrowLine;
        m_r[0] = 0.007f; // line thickness (less than this results in a threat)
        m_r[1] = 0.007f;
        transformLineArrow.translate(m_outerDiskPosition, 0, 0);
        
        if (p->flag == false) // negative velocity
        {
            transformLineArrow.translate(-m_thicknessDisk, 0, 0);
            m_x[1] = - m_x[1];
        }
        defineConicalFrustum(m_ArrowLineMesh, m_x, m_r, m_numSlices, transformLineArrow, true, true);
        
        
    }
}

