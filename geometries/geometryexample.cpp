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
#include "geometryexample.h"

#include <QSharedPointer>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

GeometryExample::GeometryExample()
    : Geometry()
{
    // Add default material
    QSharedPointer<MaterialInfo> material0 = getDefaultMaterial();
    QSharedPointer<MaterialInfo> material1 = addMaterial("material1",
            QVector4D(0.8f, 0.0f, 0.0f, 0.25),
            QVector4D(0.0f, 0.8f, 0.0f, 0.25),
            QVector4D(0.0f, 0.0f, 0.8f, 0.25),
            100.0);

    // Create root node
    QSharedPointer<Node> root = getRootNode();
    float dim = 1.0f;
    float dim1 = 0.01f;
    float dim2 = 2.0f * dim;
    QVector<float> x;
    QVector<float> y;
    QVector<float> z;
    QVector<float> r;
    QMatrix4x4 transform;
    QSharedPointer<Mesh> mesh;

    // Create cube geometry
    Node cuboidNode;
    cuboidNode.name = "Cubes";
    cuboidNode.transformation.setToIdentity();
    transform.setToIdentity();
    transform.translate(-dim / 2, -dim / 2, -dim / 2);
    mesh = QSharedPointer<Mesh>(new Mesh);
    defineCuboid(mesh, dim, dim, dim, transform);
    mesh->material = material0;
    mesh->textureFile = ":/resoureceImages/testCube.png";
    cuboidNode.meshes.push_back(mesh);
    // Create second cube geometry
    transform.setToIdentity();
    transform.translate(0, -dim / 2, 0);
    transform.translate(-dim2 / 2, -dim1, -dim2 / 2);
    mesh = QSharedPointer<Mesh>(new Mesh);
    defineCuboid(mesh, dim2, dim1, dim2, transform);
    mesh->material = material1;
    cuboidNode.meshes.push_back(mesh);

    root->nodes.push_back(cuboidNode);

    // Create spheroid geometry
    Node spheroidNode;
    spheroidNode.name = "Spheroid";
    spheroidNode.transformation.setToIdentity();
    spheroidNode.transformation.translate(0, 2, 0);
    mesh = QSharedPointer<Mesh>(new Mesh);
    defineSpheroid(mesh, dim, dim, 5.0f);
    mesh->material = material0;
    mesh->textureFile = "images/EarthHD.bmp";
    spheroidNode.meshes.push_back(mesh);

    root->nodes.push_back(spheroidNode);

    // Create conical frustum geometry
    Node cfNode;
    cfNode.name = "ConicalFrustum";
    cfNode.transformation.setToIdentity();
    cfNode.transformation.translate(-3.5, 0, 0);
    x.clear();
    r.clear();
    x.push_back(0.0);
    r.push_back(0.5);
    x.push_back(1.0);
    r.push_back(0.5);
    x.push_back(2.0);
    r.push_back(1.0);
    mesh = QSharedPointer<Mesh>(new Mesh);
    defineConicalFrustum(mesh, x, r, 40);
    mesh->material = material0;
    cfNode.meshes.push_back(mesh);
    // Create disk geometry with hole
    mesh = QSharedPointer<Mesh>(new Mesh);
    defineCircularPlane(mesh, 0.25, 0.5, 40, QMatrix4x4(), false);
    mesh->material = material0;
    cfNode.meshes.push_back(mesh);
    // Create disk geometry without hole
    mesh = QSharedPointer<Mesh>(new Mesh);
    transform.setToIdentity();
    transform.translate(2, 0, 0);
    mesh = QSharedPointer<Mesh>(new Mesh);
    defineCircularPlane(mesh, 0, 1.0, 40, transform);
    cfNode.meshes.push_back(mesh);

    root->nodes.push_back(cfNode);

    // Create rectangular prismoid
    Node rpNode;
    rpNode.name = "RectangularPrismoid";
    rpNode.transformation.setToIdentity();
    rpNode.transformation.translate(0, -2, 0);
    x.clear();
    y.clear();
    z.clear();
    x.push_back(0.0);
    y.push_back(0.5);
    z.push_back(0.25);
    x.push_back(0.5);
    y.push_back(1.0);
    z.push_back(0.5);
    x.push_back(1.0);
    y.push_back(0.5);
    z.push_back(0.25);
    mesh = QSharedPointer<Mesh>(new Mesh);
    defineRectangularPrismoid(mesh, x, y, z);
    mesh->material = material0;
    rpNode.meshes.push_back(mesh);
    // Add rectangular plane with hole
    transform.setToIdentity();
    mesh = QSharedPointer<Mesh>(new Mesh);
    defineRectangularPlane(mesh, 0.25, 0.5, 0.125, 0.25, transform, false);
    mesh->material = material0;
    rpNode.meshes.push_back(mesh);
    // Add rectangular plane without hole
    transform.setToIdentity();
    transform.translate(1.0, 0, 0);
    mesh = QSharedPointer<Mesh>(new Mesh);
    defineRectangularPlane(mesh, 0, 0.5, 0, 0.25, transform, true);
    mesh->material = material0;
    rpNode.meshes.push_back(mesh);

    root->nodes.push_back(rpNode);

    // Create lines
    QSharedPointer<MaterialInfo> material2(new MaterialInfo(QVector4D(1.0f, 1.0f, 0.0f, 1.0f)));
    Node lineNode;
    lineNode.name = "Lines";
    lineNode.transformation.setToIdentity();
    float radius = 3.0;
    int imax = 100;
    QVector<QVector3D> points;
    for(int i = 0; i < imax; i++) {
        points.push_back(QVector3D(radius * cos((float)i / (imax - 1) * M_PI),
                                   radius * sin((float)i / (imax - 1) * M_PI),
                                   0));
    }
    mesh = QSharedPointer<Mesh>(new Mesh);
    defineLine(mesh, points);
    mesh->material = material2;
    mesh->textureFile = ":/resoureceImages/fadex";
    lineNode.meshes.push_back(mesh);

    root->nodes.push_back(lineNode);
}

GeometryExample::~GeometryExample()
{

}

