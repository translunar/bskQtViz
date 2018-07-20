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
#include "torquerodbar.h"

TorqueRodBar::TorqueRodBar()
	: Geometry()
	, m_numSlices(20)
    , m_radius(0.05)
    , m_length(0.5)
{
	QSharedPointer<MaterialInfo> material0 = getDefaultMaterial();
    QSharedPointer<Node> root = getRootNode();
    QMatrix4x4 transform = QMatrix4x4();

    Node bNode;
    bNode.name = "ConicalFrustum";
    bNode.transformation.setToIdentity();
    bNode.transformation.translate(0,0,0);

    m_x = QVector<float>(2,0);
    m_r = QVector<float>(2, m_radius);
    //m_x[0] = 1.75;
    m_x[1] = m_x[0] + m_length;


	// Draw a cilinder
	transform.translate( 2.25 * m_x.at(1),0,0);
    //transform.translate( m_x.at(0),0,0);
	m_InnerCilinderMesh = QSharedPointer<Mesh>(new Mesh);
	defineConicalFrustum(m_InnerCilinderMesh, m_x, m_r, m_numSlices, transform, false);
	m_InnerCilinderMesh->name = "InnerCilinderTR";
	m_InnerCilinderMesh->material = material0;
	bNode.meshes.push_back(m_InnerCilinderMesh);
    
    m_OuterCilinderMesh = QSharedPointer<Mesh>(new Mesh);
    defineConicalFrustum(m_OuterCilinderMesh, m_x, m_r, m_numSlices, transform, true);
    m_OuterCilinderMesh->name = "OuterCilinderTR";
    m_OuterCilinderMesh->material = material0;
    bNode.meshes.push_back(m_OuterCilinderMesh);
    
    // Draw circular planes
    m_OuterTapMesh = QSharedPointer<Mesh>(new Mesh);
    defineCircularPlane(m_OuterTapMesh, 0, m_r.at(1), m_numSlices, transform, true);
    m_OuterTapMesh->name = "OuterTapMesh";
    m_OuterTapMesh->material = material0;
    bNode.meshes.push_back(m_OuterTapMesh);
    
    // Draw magnetic dipole lines around the cilinder
        // Save elipse points in a vector
    int maxPoints = 180;
    double e = 0.95;
    double a = m_length / 2 + m_radius;
    transform.translate(2 * a - a * (1-e) - m_radius, 0, 0);
    QVector<QVector3D> points;
    for (int i=0; i < maxPoints; i++)
    {
        double r_vec =  a * (1 - e * e) / (1 + e * cos(2*M_PI/180 *i));
        double r_x = r_vec * cos(2*M_PI/180 *i);
        double r_z = r_vec * sin(2*M_PI/180 *i);
        
        points.push_back(QVector3D(r_x, 0, r_z));
    }
    
        // Draw eliptic dipoles
    int n = 4;
    for (int i=0; i <= n; i++)
    {
      QMatrix4x4 transformLoc = transform;
      transform.rotate(i* 360/(2*n), QVector3D(1,0,0));
      
      transformLoc.translate(0,0,m_radius);
      QSharedPointer <Mesh> dipoleMesh (new Mesh);
      defineLine(dipoleMesh, points, transformLoc, false);
      dipoleMesh->material = getDefaultMaterial();
      bNode.meshes.push_back(dipoleMesh);
      
      QSharedPointer<Mesh> OppDipoleMesh (new Mesh);
      transformLoc.translate(0, 0, -2*m_radius);
      defineLine (OppDipoleMesh, points, transformLoc, false);
      OppDipoleMesh->material = getDefaultMaterial();
      bNode.meshes.push_back(OppDipoleMesh);
    }
    
	root->nodes.push_back(bNode);

}

TorqueRodBar::~ TorqueRodBar(){}

