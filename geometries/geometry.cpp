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
#include "geometry.h"
#include "geometrymanager.h"

#include <iostream>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

Geometry::Geometry()
    : m_rootNode(new Node)
    , m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_vertexBufferUsage(QOpenGLBuffer::StaticDraw)
    , m_normalBufferUsage(QOpenGLBuffer::StaticDraw)
    , m_textureCoordBufferUsage(QOpenGLBuffer::StaticDraw)
    , m_indexBuffer(QOpenGLBuffer::IndexBuffer)
    , m_indexBufferUsage(QOpenGLBuffer::StaticDraw)
    , m_isOpenGLInitialized(false)
    , m_boundingRadii(-1.0f)
{
    m_rootNode->name = "root";
    m_rootNode->transformation.setToIdentity();
    m_defaultMaterial = addMaterial("defaultMaterial",
                                    QVector4D(0.1f, 0.1f, 0.1f, 1.0),
                                    QVector4D(0.6f, 0.6f, 0.6f, 1.0),
                                    QVector4D(0.1f, 0.1f, 0.1f, 1.0),
                                    100.0);
}

Geometry::~Geometry()
{

}

bool Geometry::load(QString pathToFile)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(pathToFile.toStdString(),
                           aiProcess_FindInvalidData |
                           aiProcess_JoinIdenticalVertices |
                           aiProcess_CalcTangentSpace |
                           aiProcess_FixInfacingNormals |
                           aiProcess_GenSmoothNormals |
                           aiProcess_GenUVCoords |
                           aiProcess_FlipUVs |
                           aiProcess_Triangulate |
                           aiProcess_SortByPType);
    if(!scene) {
        std::cout << "Error loading file: (assimp:) " << importer.GetErrorString() << std::endl;
        return false;
    }

    if(scene->HasMaterials()) {
        for(unsigned int i = 0; i < scene->mNumMaterials; i++) {
            QSharedPointer<MaterialInfo> material = processMaterial(scene->mMaterials[i]);
            m_materials.push_back(material);
        }
    }

    if(scene->HasMeshes()) {
        for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
            m_meshes.push_back(processMesh(scene->mMeshes[i]));
        }
    } else {
        std::cout << "Error: No meshes found" << std::endl;
        return false;
    }

    if(scene->mRootNode != NULL) {
        Node *rootNode = new Node;
        processNode(scene, scene->mRootNode, 0, *rootNode);
        m_rootNode.reset(rootNode);
    } else {
        std::cout << "Error loading model" << std::endl;
        return false;
    }

    calculateBoundingRadii();

    return true;
}

bool Geometry::initialize(QOpenGLShaderProgram *program)
{
    if(!createBuffers(program)) {
        return false;
    }
    return true;
}

void Geometry::draw(GeometryManager *geometryManager, QOpenGLShaderProgram *program,
                    QMatrix4x4 cameraMatrix, QMatrix4x4 objectMatrix, float scaleFactor)
{
    if(!m_isOpenGLInitialized) {
        initializeOpenGLFunctions();
        m_isOpenGLInitialized = true;
    }
    m_vao.bind();
    updateBuffers(program);
    if(m_indexBuffer.bind()) {
        drawNode(geometryManager, program, m_rootNode.data(), cameraMatrix, objectMatrix, scaleFactor);
        m_indexBuffer.release();
    }
    m_vao.release();
}

void Geometry::cleanup()
{
    m_vao.destroy();
    m_vertexBuffer.destroy();
    m_normalBuffer.destroy();
    m_textureCoordBuffer.destroy();
    m_indexBuffer.destroy();
}

QSet<QString> Geometry::textureFiles()
{
    QSet<QString> result;
    getNodeTextures(&result, m_rootNode.data());
    return result;
}

float Geometry::boundingRadii()
{
    if(m_boundingRadii < 0.0) {
        calculateBoundingRadii();
    }
    return m_boundingRadii;
}

void Geometry::setDefaultMaterial(QSharedPointer<Geometry::MaterialInfo> material)
{
    m_defaultMaterial->ambientColor = material->ambientColor;
    m_defaultMaterial->diffuseColor = material->diffuseColor;
    m_defaultMaterial->specularColor = material->specularColor;
    m_defaultMaterial->shininess = material->shininess;
}

QSharedPointer<Geometry::Node> Geometry::getRootNode()
{
    return m_rootNode;
}

QSharedPointer<Geometry::MaterialInfo> Geometry::addMaterial(QString name, QVector4D ambientColor, QVector4D diffuseColor, QVector4D specularColor, float shininess)
{
    QSharedPointer<MaterialInfo> m(new MaterialInfo);
    m->name = name;
    m->ambientColor = ambientColor;
    m->diffuseColor = diffuseColor;
    m->specularColor = specularColor;
    m->shininess = shininess;
    m_materials.push_back(m);
    return m;
}

void Geometry::allocate(bool isAllocated, QSharedPointer<Mesh> mesh, int &numVertices, int &numIndices)
{
    if(!isAllocated) {
        mesh->indexOffset = m_indices.length();
        mesh->indexCount = numIndices;
        mesh->vertexOffset = m_vertices.length();
        mesh->vertexCount = numVertices;
    }
    
    // If the new set of points is larger than the mesh vertex/indices counts then we need
    // to resize the geometry's knwoledge of number of vertices, indixes, normals and
    // texturecoords. This could also be done when numeVertices != mesh->vertexCount
    // however, it is likely less computationally expensive to simply use the oversized
    // mesh arrays than to resize them each time they change.
    if (numVertices > (int)mesh->vertexCount) {
        mesh->vertexCount = numVertices;
        int stopVertex = mesh->vertexOffset + numVertices;
        m_vertices.resize(stopVertex);
        m_normals.resize(stopVertex);
        m_textureCoords.resize(stopVertex);
    }
    if (numIndices > (int)mesh->indexCount) {
        mesh->indexCount = numIndices;
        int stopIndex = mesh->indexOffset + numIndices;
        m_indices.resize(stopIndex);
    }
    
    if(!isAllocated) {
        int stopIndex = mesh->indexOffset + numIndices;
        int stopVertex = mesh->vertexOffset + numVertices;
        m_vertices.resize(stopVertex);
        m_normals.resize(stopVertex);
        m_textureCoords.resize(stopVertex);
        m_indices.resize(stopIndex);
    }
    
}

void Geometry::defineLine(QSharedPointer<Geometry::Mesh> mesh, QVector<QVector3D> points,
                          QMatrix4x4 transform, bool isAllocated)
{
    int numVertices = points.size();
    int numIndices = points.size();
    allocate(isAllocated, mesh, numVertices, numIndices);

    int iVertex = mesh->vertexOffset;
    int iIndex = mesh->indexOffset;
    for(int i = 0; i < (int)mesh->vertexCount; i++) {
        if(i < numVertices) {
            m_vertices[iVertex] = transform * points.at(i);
            m_normals[iVertex] = QVector3D();
            m_textureCoords[iVertex] = QVector2D((float)i / (numVertices - 1), 0);
        } else {
            m_vertices[iVertex] = transform * points.back();
            m_normals[iVertex] = QVector3D();
            m_textureCoords[iVertex] = QVector2D(1, 0);
        }
        iVertex++;
    }
    for(int i = 0; i < numIndices; i++) {
        m_indices[iIndex++] = mesh->vertexOffset + i;
    }
    mesh->material = m_defaultMaterial;
    mesh->primitiveType = GL_LINE_STRIP;

    if(!isAllocated) {
        m_meshes.push_back(mesh);
    }
}

void Geometry::defineCuboid(QSharedPointer<Mesh> mesh, float xdim, float ydim, float zdim,
                            QMatrix4x4 transform, bool isNormalsOut, bool isAllocated)
{
    int numVertices = 24;
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float x1 = x + xdim;
    float y1 = y + ydim;
    float z1 = z + zdim;
    float vertices[] = {
        x, y, z1,     x1, y, z1,    x1, y1, z1,   x, y1, z1,
        x1, y, z1,    x1, y, z,     x1, y1, z,    x1, y1, z1,
        x1, y, z,     x, y, z,      x, y1, z,     x1, y1, z,
        x, y, z,      x, y, z1,     x, y1, z1,    x, y1, z,
        x, y1, z1,    x1, y1, z1,   x1, y1, z,    x, y1, z,
        x, y, z,      x1, y, z,     x1, y, z1,    x, y, z1
    };
    float normals[] = {
        0, 0, 1,    0, 0, 1,    0, 0, 1,    0, 0, 1,
        1, 0, 0,    1, 0, 0,    1, 0, 0,    1, 0, 0,
        0, 0, -1,    0, 0, -1,    0, 0, -1,    0, 0, -1,
        -1, 0, 0,   -1, 0, 0,   -1, 0, 0,   -1, 0, 0,
        0, 1, 0,    0, 1, 0,    0, 1, 0,    0, 1, 0,
        0, -1, 0,    0, -1, 0,    0, -1, 0,    0, -1, 0
    };
    //       5
    // 1(+z) 2(+x) 3 4
    //       6(+y)
    float texcoords[] = {
        0.f, 1.f / 3.f,   0.25f, 1.f / 3.f, 0.25f, 2.f / 3.f, 0.f, 2.f / 3.f,
        0.25f, 1.f / 3.f, 0.5f, 1.f / 3.f,  0.5f, 2.f / 3.f,  0.25f, 2.f / 3.f,
        0.5f, 1.f / 3.f,  0.75f, 1.f / 3.f, 0.75f, 2.f / 3.f, 0.5f, 2.f / 3.f,
        0.75f, 1.f / 3.f, 1.f, 1.f / 3.f,   1.f, 2.f / 3.f,   0.75f, 2.f / 3.f,
        0.25f, 1.f,     0.25f, 2.f / 3.f, 0.5f, 2.f / 3.f,  0.5f, 1.f,
        0.25f, 1.f / 3.f, 0.25f, 0.f,     0.5f, 0.f,      0.5f, 1.f / 3.f,
    };
    int numIndices = 36;
    unsigned int indicesOut[] = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };
    unsigned int indicesIn[] = {
        0, 2, 1, 0, 3, 2,
        4, 6, 5, 4, 7, 6,
        8, 10, 9, 8, 11, 10,
        12, 14, 13, 12, 15, 14,
        16, 18, 17, 16, 19, 18,
        20, 22, 21, 20, 23, 22
    };

    allocate(isAllocated, mesh, numVertices, numIndices);
    QMatrix4x4 normalTransform = removeTranslationAndScale(transform);

    for(int i = 0; i < numVertices; i++) {
        QVector3D vertex(vertices[3 * i], vertices[3 * i + 1], vertices[3 * i + 2]);
        m_vertices[i + mesh->vertexOffset] = transform * vertex;
        QVector3D normal = (isNormalsOut ? 1.0 : -1.0)
                           * QVector3D(normals[3 * i], normals[3 * i + 1], normals[3 * i + 2]);
        m_normals[i + mesh->vertexOffset] = normalTransform * normal;
        m_textureCoords[i + mesh->vertexOffset] = QVector2D(texcoords[2 * i], texcoords[2 * i + 1]);
    }
    for(int i = 0; i < numIndices; i++) {
        if(isNormalsOut) {
            m_indices[i + mesh->indexOffset] = indicesOut[i] + mesh->vertexOffset;
        } else {
            m_indices[i + mesh->indexOffset] = indicesIn[i] + mesh->vertexOffset;
        }
    }
    mesh->material = m_defaultMaterial;
    mesh->primitiveType = GL_TRIANGLES;
    if(!isAllocated) {
        m_meshes.push_back(mesh);
    }
}

void Geometry::defineSteradianPortionSphere(QSharedPointer<Mesh> mesh, float angle, float radius, float res,
                              QMatrix4x4 transform, bool isNormalsOut, bool isAllocated)
{
    unsigned int numLat = (unsigned int)(angle / res) + 1;
    unsigned int numLon = (unsigned int)(angle / res) + 1;
    int numVertices = numLat * numLon;
    int numIndices = (numLat - 1) * (numLon - 1) * 6;
    res = res * M_PI / 180.0f;
    
    allocate(isAllocated, mesh, numVertices, numIndices);
    QMatrix4x4 normalTransform = removeTranslationAndScale(transform);
    
    int iVertex = mesh->vertexOffset;
    int iIndex = mesh->indexOffset;
    for(unsigned int iLat = 0; iLat < numLat; iLat++) {
        for(unsigned int iLon = 0; iLon < numLon; iLon++) {
            float lat = iLat * res - (angle/2*M_PI/180);
            float lon = iLon * res - (angle/2*M_PI/180);
            float slat = sin(lat);
            float clat = cos(lat);
            float slon = sin(lon);
            float clon = cos(lon);
            
            QVector3D vertex(radius * clat * clon,
                             radius * clat * slon,
                             radius * slat);
            m_vertices[iVertex] = transform * vertex;
            m_normals[iVertex] = normalTransform * (vertex.normalized() * (isNormalsOut ? 1.0 : -1.0));
            m_textureCoords[iVertex] = QVector2D((float)iLon / (float)(numLon - 1),
                                                 (float)iLat / (float)(numLat - 1));
            iVertex++;
            
            if(iLon < (numLon - 1) && iLat < (numLat - 1)) {
                m_indices[iIndex++] = (mesh->vertexOffset + iLat * numLon + iLon);
                if(isNormalsOut) {
                    m_indices[iIndex++] = (mesh->vertexOffset + iLat * numLon + 1 + iLon);
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + iLon);
                } else {
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + iLon);
                    m_indices[iIndex++] = (mesh->vertexOffset + iLat * numLon + 1 + iLon);
                }
                m_indices[iIndex++] = (mesh->vertexOffset + iLat * numLon + 1 + iLon);
                if(isNormalsOut) {
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + (iLon + 1));
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + iLon);
                } else {
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + iLon);
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + (iLon + 1));
                }
            }
        }
    }
    mesh->material = m_defaultMaterial;
    mesh->primitiveType = GL_TRIANGLES;
    if(!isAllocated) {
        m_meshes.push_back(mesh);
    }
}

void Geometry::defineSpheroid(QSharedPointer<Mesh> mesh, float xyradius, float zradius, float res,
                              QMatrix4x4 transform, bool isNormalsOut, bool isAllocated)
{
    float eSq = 1.0f - ((zradius * zradius) / (xyradius * xyradius));
    unsigned int numLat = (unsigned int)(180.0f / res) + 1;
    unsigned int numLon = (unsigned int)(360.0f / res) + 1;
    int numVertices = numLat * numLon;
    int numIndices = (numLat - 1) * (numLon - 1) * 6;
    res = res * M_PI / 180.0f;

    allocate(isAllocated, mesh, numVertices, numIndices);
    QMatrix4x4 normalTransform = removeTranslationAndScale(transform);

    int iVertex = mesh->vertexOffset;
    int iIndex = mesh->indexOffset;
    for(unsigned int iLat = 0; iLat < numLat; iLat++) {
        for(unsigned int iLon = 0; iLon < numLon; iLon++) {
            float lat = iLat * res - M_PI * 0.5;
            float lon = iLon * res;
            float slat = sin(lat);
            float clat = cos(lat);
            float slon = sin(lon);
            float clon = cos(lon);
            float invDenom = 1.0f / sqrt(1.0f - eSq * slat * slat);

            QVector3D vertex((xyradius * invDenom) * clat * clon,
                             (xyradius * invDenom) * clat * slon,
                             (xyradius * (1.0f - eSq) * invDenom) * slat);
            m_vertices[iVertex] = transform * vertex;
            m_normals[iVertex] = normalTransform * (vertex.normalized() * (isNormalsOut ? 1.0 : -1.0));
            m_textureCoords[iVertex] = QVector2D((float)iLon / (float)(numLon - 1),
                                                 (float)iLat / (float)(numLat - 1));
            iVertex++;

            if(iLon < (numLon - 1) && iLat < (numLat - 1)) {
                m_indices[iIndex++] = (mesh->vertexOffset + iLat * numLon + iLon);
                if(isNormalsOut) {
                    m_indices[iIndex++] = (mesh->vertexOffset + iLat * numLon + 1 + iLon);
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + iLon);
                } else {
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + iLon);
                    m_indices[iIndex++] = (mesh->vertexOffset + iLat * numLon + 1 + iLon);
                }
                m_indices[iIndex++] = (mesh->vertexOffset + iLat * numLon + 1 + iLon);
                if(isNormalsOut) {
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + (iLon + 1));
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + iLon);
                } else {
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + iLon);
                    m_indices[iIndex++] = (mesh->vertexOffset + (iLat + 1) * numLon + (iLon + 1));
                }
            }
        }
    }
    mesh->material = m_defaultMaterial;
    mesh->primitiveType = GL_TRIANGLES;
    if(!isAllocated) {
        m_meshes.push_back(mesh);
    }
}

void Geometry::defineConicalFrustum(QSharedPointer<Mesh> mesh, QVector<float> x, QVector<float> r,
                                    unsigned int numSlices, QMatrix4x4 transform,
                                    bool isNormalsOut, bool isAllocated)
{
    unsigned int imax = x.size() - 1;
    unsigned int jmax = numSlices + 1;
    int numVertices = imax * jmax * 2;
    int numIndices = imax * jmax * 6;

    allocate(isAllocated, mesh, numVertices, numIndices);
    QMatrix4x4 normalTransform = removeTranslationAndScale(transform);

    int iVertex = mesh->vertexOffset;
    int iIndex = mesh->indexOffset;
    for(unsigned int i = 0; i < imax; i++) {
        for(unsigned int j = 0; j < jmax; j++) {
            float sliceAngle = j * (2 * M_PI / (float)numSlices);
            float y0 = r.at(i) * cos(sliceAngle);
            float z0 = r.at(i) * sin(sliceAngle);
            float y1 = r.at(i + 1) * cos(sliceAngle);
            float z1 = r.at(i + 1) * sin(sliceAngle);

            m_vertices[iVertex] = (transform * QVector3D(x.at(i), y0, z0));
            m_vertices[iVertex + 1] = (transform * QVector3D(x.at(i + 1), y1, z1));

            float slopeAngle = atan2(r.at(i + 1) - r.at(i), x.at(i + 1) - x.at(i));
            QVector3D normal(-sin(slopeAngle) * cos(sliceAngle),
                             cos(slopeAngle) * cos(sliceAngle),
                             sin(sliceAngle));
            normal *= isNormalsOut ? 1.0 : -1.0;
            normal = normalTransform * normal;
            m_normals[iVertex] = (normal);
            m_normals[iVertex + 1] = (normal);

            m_textureCoords[iVertex] = (QVector2D((float)j / (float)jmax,
                                                  (float)i / (float)x.size()));
            m_textureCoords[iVertex + 1] = (QVector2D((float)j / (float)jmax,
                                            (float)(i + 1) / (float)x.size()));

            iVertex += 2;

            if(j < (jmax - 1)) {
                m_indices[iIndex++] = (mesh->vertexOffset + i * jmax * 2 + (2 * j));
                if(isNormalsOut) {
                    m_indices[iIndex++] = (mesh->vertexOffset + i * jmax * 2 + (2 * j) + 2);
                    m_indices[iIndex++] = (mesh->vertexOffset + i * jmax * 2 + (2 * j) + 1);
                } else {
                    m_indices[iIndex++] = (mesh->vertexOffset + i * jmax * 2 + (2 * j) + 1);
                    m_indices[iIndex++] = (mesh->vertexOffset + i * jmax * 2 + (2 * j) + 2);
                }
                m_indices[iIndex++] = (mesh->vertexOffset + i * jmax * 2 + 1 + (2 * j));
                if(isNormalsOut) {
                    m_indices[iIndex++] = (mesh->vertexOffset + i * jmax * 2 + (2 * j) + 2);
                    m_indices[iIndex++] = (mesh->vertexOffset + i * jmax * 2 + (2 * j) + 3);
                } else {
                    m_indices[iIndex++] = (mesh->vertexOffset + i * jmax * 2 + (2 * j) + 3);
                    m_indices[iIndex++] = (mesh->vertexOffset + i * jmax * 2 + (2 * j) + 2);
                }
            }
        }
    }
    mesh->material = m_defaultMaterial;
    mesh->primitiveType = GL_TRIANGLES;
    if(!isAllocated) {
        m_meshes.push_back(mesh);
    }
}

void Geometry::defineCircularPlane(QSharedPointer<Mesh> mesh, float r1, float r2, unsigned int numSlices,
                                   QMatrix4x4 transform, bool isNormalsOut, bool isAllocated)
{
    if(r1 == r2 && r1!=0) {
        std::cout << "Circular plane: r1 cannot equal r2." << std::endl;
        return;
    }
    if(r1 > r2) {
        std::swap(r1, r2);
    }
    float angle = 2 * M_PI / (float)numSlices;
    unsigned int imax = numSlices + 1;

    int numVertices = imax * 2;
    int numIndices = (imax - 1) * 6;
    allocate(isAllocated, mesh, numVertices, numIndices);
    QMatrix4x4 normalTransform = removeTranslationAndScale(transform);

    QVector3D vertex;
    QVector3D normal = normalTransform * QVector3D(isNormalsOut ? 1.0 : -1.0, 0.0, 0.0);
    int iVertex = mesh->vertexOffset;
    int iIndex = mesh->indexOffset;
    for(unsigned int i = 0; i < imax; i++) {
        vertex = transform * QVector3D(0.0, cos(i * angle) * r1, sin(i * angle) * r1);
        m_vertices[iVertex] = (vertex);
        vertex = transform * QVector3D(0.0, cos(i * angle) * r2, sin(i * angle) * r2);
        m_vertices[iVertex + 1] = (vertex);

        m_normals[iVertex] = (normal);
        m_normals[iVertex + 1] = (normal);

        m_textureCoords[iVertex] = (QVector2D((float) i / imax, 0.0));
        m_textureCoords[iVertex + 1] = (QVector2D((float) i / imax, 1.0));

        iVertex += 2;

        if(i < (imax - 1)) {
            m_indices[iIndex++] = (mesh->vertexOffset + i * 2);
            if(isNormalsOut) {
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 + 1);
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 + 2);
            } else {
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 + 2);
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 + 1);
            }
            m_indices[iIndex++] = (mesh->vertexOffset + i * 2 + 1);
            if(isNormalsOut) {
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 + 3);
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 + 2);
            } else {
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 + 2);
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 + 3);
            }
        }
    }
    mesh->material = m_defaultMaterial;
    mesh->primitiveType = GL_TRIANGLES;
    if(!isAllocated) {
        m_meshes.push_back(mesh);
    }
}

void Geometry::defineRectangularPrismoid(QSharedPointer<Mesh> mesh, QVector<float> x, QVector<float> ywidth, QVector<float> zwidth,
        QMatrix4x4 transform, bool isNormalsOut, bool isAllocated)
{
    unsigned int imax = x.size() - 1;
    unsigned int j = 8;
    unsigned int kmax = 8;
    int numVertices = imax * 16;
    int numIndices = imax * kmax / 2 * 6;

    allocate(isAllocated, mesh, numVertices, numIndices);
    QMatrix4x4 normalTransform = removeTranslationAndScale(transform)
                                 * (isNormalsOut ? 1.0 : -1.0);

    if(x.size() < 2 || ywidth.size() < 2 || zwidth.size() < 2
            || x.size() != ywidth.size() || x.size() != zwidth.size()) {
        std::cout << "Rectangular prismoid: x, y, z not the same length or too small." << std::endl;
        return;
    }

    int iVertex = mesh->vertexOffset;
    int iIndex = mesh->indexOffset;
    for(unsigned int i = 0; i < imax; i++) {
        m_vertices[iVertex + 0] = (transform * QVector3D(x.at(i), ywidth.at(i) * 0.5f, zwidth.at(i) * 0.5f));
        m_vertices[iVertex + 1] = (transform * QVector3D(x.at(i), -ywidth.at(i) * 0.5f, zwidth.at(i) * 0.5f));
        m_vertices[iVertex + 2] = (transform * QVector3D(x.at(i), -ywidth.at(i) * 0.5f, zwidth.at(i) * 0.5f));
        m_vertices[iVertex + 3] = (transform * QVector3D(x.at(i), -ywidth.at(i) * 0.5f, -zwidth.at(i) * 0.5f));
        m_vertices[iVertex + 4] = (transform * QVector3D(x.at(i), -ywidth.at(i) * 0.5f, -zwidth.at(i) * 0.5f));
        m_vertices[iVertex + 5] = (transform * QVector3D(x.at(i), ywidth.at(i) * 0.5f, -zwidth.at(i) * 0.5f));
        m_vertices[iVertex + 6] = (transform * QVector3D(x.at(i), ywidth.at(i) * 0.5f, -zwidth.at(i) * 0.5f));
        m_vertices[iVertex + 7] = (transform * QVector3D(x.at(i), ywidth.at(i) * 0.5f, zwidth.at(i) * 0.5f));

        m_vertices[iVertex + 8] = (transform * QVector3D(x.at(i + 1), ywidth.at(i + 1) * 0.5f, zwidth.at(i + 1) * 0.5f));
        m_vertices[iVertex + 9] = (transform * QVector3D(x.at(i + 1), -ywidth.at(i + 1) * 0.5f, zwidth.at(i + 1) * 0.5f));
        m_vertices[iVertex + 10] = (transform * QVector3D(x.at(i + 1), -ywidth.at(i + 1) * 0.5f, zwidth.at(i + 1) * 0.5f));
        m_vertices[iVertex + 11] = (transform * QVector3D(x.at(i + 1), -ywidth.at(i + 1) * 0.5f, -zwidth.at(i + 1) * 0.5f));
        m_vertices[iVertex + 12] = (transform * QVector3D(x.at(i + 1), -ywidth.at(i + 1) * 0.5f, -zwidth.at(i + 1) * 0.5f));
        m_vertices[iVertex + 13] = (transform * QVector3D(x.at(i + 1), ywidth.at(i + 1) * 0.5f, -zwidth.at(i + 1) * 0.5f));
        m_vertices[iVertex + 14] = (transform * QVector3D(x.at(i + 1), ywidth.at(i + 1) * 0.5f, -zwidth.at(i + 1) * 0.5f));
        m_vertices[iVertex + 15] = (transform * QVector3D(x.at(i + 1), ywidth.at(i + 1) * 0.5f, zwidth.at(i + 1) * 0.5f));

        float ySlopeAngle = atan2(ywidth.at(i + 1) / 2 - ywidth.at(i) / 2,
                                  x.at(i + 1) - x.at(i));
        float sySlopeAngle = sin(ySlopeAngle);
        float cySlopeAngle = cos(ySlopeAngle);
        float zSlopeAngle = atan2(zwidth.at(i + 1) / 2 - zwidth.at(i) / 2,
                                  x.at(i + 1) - x.at(i));
        float szSlopeAngle = sin(zSlopeAngle);
        float czSlopeAngle = cos(zSlopeAngle);

        m_normals[iVertex + 0] = (normalTransform * QVector3D(-szSlopeAngle, 0, czSlopeAngle));
        m_normals[iVertex + 1] = (normalTransform * QVector3D(-szSlopeAngle, 0, czSlopeAngle));
        m_normals[iVertex + 2] = (normalTransform * QVector3D(-sySlopeAngle, -cySlopeAngle, 0));
        m_normals[iVertex + 3] = (normalTransform * QVector3D(-sySlopeAngle, -cySlopeAngle, 0));
        m_normals[iVertex + 4] = (normalTransform * QVector3D(-szSlopeAngle, 0, -czSlopeAngle));
        m_normals[iVertex + 5] = (normalTransform * QVector3D(-szSlopeAngle, 0, -czSlopeAngle));
        m_normals[iVertex + 6] = (normalTransform * QVector3D(-sySlopeAngle, cySlopeAngle, 0));
        m_normals[iVertex + 7] = (normalTransform * QVector3D(-sySlopeAngle, cySlopeAngle, 0));

        m_normals[iVertex + 8] = (normalTransform * QVector3D(-szSlopeAngle, 0, czSlopeAngle));
        m_normals[iVertex + 9] = (normalTransform * QVector3D(-szSlopeAngle, 0, czSlopeAngle));
        m_normals[iVertex + 10] = (normalTransform * QVector3D(-sySlopeAngle, -cySlopeAngle, 0));
        m_normals[iVertex + 11] = (normalTransform * QVector3D(-sySlopeAngle, -cySlopeAngle, 0));
        m_normals[iVertex + 12] = (normalTransform * QVector3D(-szSlopeAngle, 0, -czSlopeAngle));
        m_normals[iVertex + 13] = (normalTransform * QVector3D(-szSlopeAngle, 0, -czSlopeAngle));
        m_normals[iVertex + 14] = (normalTransform * QVector3D(-sySlopeAngle, cySlopeAngle, 0));
        m_normals[iVertex + 15] = (normalTransform * QVector3D(-sySlopeAngle, cySlopeAngle, 0));

        m_textureCoords[iVertex + 0] = (QVector2D(0, (float)i / (float)imax));
        m_textureCoords[iVertex + 1] = (QVector2D(0.25f, (float)i / (float)imax));
        m_textureCoords[iVertex + 2] = (QVector2D(0.25f, (float)i / (float)imax));
        m_textureCoords[iVertex + 3] = (QVector2D(0.5f, (float)i / (float)imax));
        m_textureCoords[iVertex + 4] = (QVector2D(0.5f, (float)i / (float)imax));
        m_textureCoords[iVertex + 5] = (QVector2D(0.75f, (float)i / (float)imax));
        m_textureCoords[iVertex + 6] = (QVector2D(0.75f, (float)i / (float)imax));
        m_textureCoords[iVertex + 7] = (QVector2D(1.0f, (float)i / (float)imax));

        m_textureCoords[iVertex + 8] = (QVector2D(0, (float)(i + 1) / (float)imax));
        m_textureCoords[iVertex + 9] = (QVector2D(0.25f, (float)(i + 1) / (float)imax));
        m_textureCoords[iVertex + 10] = (QVector2D(0.25f, (float)(i + 1) / (float)imax));
        m_textureCoords[iVertex + 11] = (QVector2D(0.5f, (float)(i + 1) / (float)imax));
        m_textureCoords[iVertex + 12] = (QVector2D(0.5f, (float)(i + 1) / (float)imax));
        m_textureCoords[iVertex + 13] = (QVector2D(0.75f, (float)(i + 1) / (float)imax));
        m_textureCoords[iVertex + 14] = (QVector2D(0.75f, (float)(i + 1) / (float)imax));
        m_textureCoords[iVertex + 15] = (QVector2D(1.0f, (float)(i + 1) / (float)imax));

        iVertex += 16;

        for(unsigned int k = 0; k < kmax; k += 2) {
            m_indices[iIndex++] = (mesh->vertexOffset + i * 2 * j + k);
            if(isNormalsOut) {
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 * j + 1 + k);
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 * j + j + k);
            } else {
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 * j + j + k);
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 * j + 1 + k);
            }
            m_indices[iIndex++] = (mesh->vertexOffset + i * 2 * j + 1 + k);
            if(isNormalsOut) {
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 * j + j + 1 + k);
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 * j + j + k);
            } else {
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 * j + j + k);
                m_indices[iIndex++] = (mesh->vertexOffset + i * 2 * j + j + 1 + k);
            }
        }
    }
    mesh->material = m_defaultMaterial;
    mesh->primitiveType = GL_TRIANGLES;
    if(!isAllocated) {
        m_meshes.push_back(mesh);
    }
}

void Geometry::defineRectangularPlane(QSharedPointer<Mesh> mesh, float y1, float y2,
                                      float z1, float z2, QMatrix4x4 transform,
                                      bool isNormalsOut, bool isAllocated)
{
    if(y1 == y2) {
        std::cout << "Rectangular plane: y1 cannot equal y2." << std::endl;
        return;
    }
    if(z1 == z2) {
        std::cout << "Rectangular plane: z1 cannot equal z2." << std::endl;
        return;
    }
    if(y1 > y2) {
        std::swap(y1, y2);
    }
    if(z1 > z2) {
        std::swap(z1, z2);
    }
    int numVertices = 16;
    int numIndices = 48;
    allocate(isAllocated, mesh, numVertices, numIndices);
    QMatrix4x4 normalTransform = removeTranslationAndScale(transform);

    int iVertex = mesh->vertexOffset;
    int iIndex = mesh->indexOffset;
    if(y1 == 0 || z1 == 0) {
        // No hole
        m_vertices[iVertex + 0] = (transform * QVector3D(0, -y2 / 2, -z2 / 2));
        m_vertices[iVertex + 1] = (transform * QVector3D(0, y2 / 2, -z2 / 2));
        m_vertices[iVertex + 2] = (transform * QVector3D(0, y2 / 2, z2 / 2));
        m_vertices[iVertex + 3] = (transform * QVector3D(0, -y2 / 2, z2 / 2));

        for(unsigned int i = 0; i < 4; i++) {
            m_normals[iVertex + i] = (normalTransform * QVector3D(isNormalsOut ? 1.0 : -1.0, 0, 0));
        }

        m_textureCoords[iVertex + 0] = (QVector2D(0, 0));
        m_textureCoords[iVertex + 1] = (QVector2D(1, 0));
        m_textureCoords[iVertex + 2] = (QVector2D(1, 1));
        m_textureCoords[iVertex + 3] = (QVector2D(0, 1));

        //iVertex += 4;

        m_indices[iIndex++] = (mesh->vertexOffset + 0);
        m_indices[iIndex++] = (mesh->vertexOffset + 1);
        m_indices[iIndex++] = (mesh->vertexOffset + 2);
        m_indices[iIndex++] = (mesh->vertexOffset + 0);
        m_indices[iIndex++] = (mesh->vertexOffset + 2);
        m_indices[iIndex++] = (mesh->vertexOffset + 3);

        mesh->indexCount = 6;
        mesh->vertexCount = 4;

    } else {
        m_vertices[iVertex + 0] = (transform * QVector3D(0, -y2 / 2, -z2 / 2));
        m_vertices[iVertex + 1] = (transform * QVector3D(0, -y1 / 2, -z2 / 2));
        m_vertices[iVertex + 2] = (transform * QVector3D(0, y1 / 2, -z2 / 2));
        m_vertices[iVertex + 3] = (transform * QVector3D(0, y2 / 2, -z2 / 2));

        m_vertices[iVertex + 4] = (transform * QVector3D(0, -y2 / 2, -z1 / 2));
        m_vertices[iVertex + 5] = (transform * QVector3D(0, -y1 / 2, -z1 / 2));
        m_vertices[iVertex + 6] = (transform * QVector3D(0, y1 / 2, -z1 / 2));
        m_vertices[iVertex + 7] = (transform * QVector3D(0, y2 / 2, -z1 / 2));

        m_vertices[iVertex + 8] = (transform * QVector3D(0, -y2 / 2, z1 / 2));
        m_vertices[iVertex + 9] = (transform * QVector3D(0, -y1 / 2, z1 / 2));
        m_vertices[iVertex + 10] = (transform * QVector3D(0, y1 / 2, z1 / 2));
        m_vertices[iVertex + 11] = (transform * QVector3D(0, y2 / 2, z1 / 2));

        m_vertices[iVertex + 12] = (transform * QVector3D(0, -y2 / 2, z2 / 2));
        m_vertices[iVertex + 13] = (transform * QVector3D(0, -y1 / 2, z2 / 2));
        m_vertices[iVertex + 14] = (transform * QVector3D(0, y1 / 2, z2 / 2));
        m_vertices[iVertex + 15] = (transform * QVector3D(0, y2 / 2, z2 / 2));

        for(unsigned int i = 0; i < 16; i++) {
            m_normals[iVertex + i] = (normalTransform * QVector3D(isNormalsOut ? 1.0 : -1.0, 0, 0));
        }

        m_textureCoords[iVertex + 0] = (QVector2D(0, 0));
        m_textureCoords[iVertex + 1] = (QVector2D((y2 / 2 - y1 / 2) / y2, 0));
        m_textureCoords[iVertex + 2] = (QVector2D((y2 / 2 + y1 / 2) / y2, 0));
        m_textureCoords[iVertex + 3] = (QVector2D(1, 0));

        m_textureCoords[iVertex + 4] = (QVector2D(0, (z2 / 2 - z1 / 2) / z2));
        m_textureCoords[iVertex + 5] = (QVector2D((y2 / 2 - y1 / 2) / y2, (z2 / 2 - z1 / 2) / z2));
        m_textureCoords[iVertex + 6] = (QVector2D((y2 / 2 + y1 / 2) / y2, (z2 / 2 - z1 / 2) / z2));
        m_textureCoords[iVertex + 7] = (QVector2D(1, (z2 / 2 - z1 / 2) / z2));

        m_textureCoords[iVertex + 8] = (QVector2D(0, (z2 / 2 + z1 / 2) / z2));
        m_textureCoords[iVertex + 9] = (QVector2D((y2 / 2 - y1 / 2) / y2, (z2 / 2 + z1 / 2) / z2));
        m_textureCoords[iVertex + 10] = (QVector2D((y2 / 2 + y1 / 2) / y2, (z2 / 2 + z1 / 2) / z2));
        m_textureCoords[iVertex + 11] = (QVector2D(1, (z2 / 2 + z1 / 2) / z2));

        m_textureCoords[iVertex + 12] = (QVector2D(0, 1));
        m_textureCoords[iVertex + 13] = (QVector2D((y2 / 2 - y1 / 2) / y2, 1));
        m_textureCoords[iVertex + 14] = (QVector2D((y2 / 2 + y1 / 2) / y2, 1));
        m_textureCoords[iVertex + 15] = (QVector2D(1, 1));

        for(unsigned int i = 0; i < 3; i++) {
            for(unsigned int j = 0; j < 3; j++) {
                if(!(i == 1 && j == 1)) {
                    m_indices[iIndex++] = (mesh->vertexOffset + i * 4 + j);
                    if(isNormalsOut) {
                        m_indices[iIndex++] = (mesh->vertexOffset + i * 4 + j + 1);
                        m_indices[iIndex++] = (mesh->vertexOffset + (i + 1) * 4 + j + 1);
                    } else {
                        m_indices[iIndex++] = (mesh->vertexOffset + (i + 1) * 4 + j + 1);
                        m_indices[iIndex++] = (mesh->vertexOffset + i * 4 + j + 1);
                    }
                    m_indices[iIndex++] = (mesh->vertexOffset + i * 4 + j);
                    if(isNormalsOut) {
                        m_indices[iIndex++] = (mesh->vertexOffset + (i + 1) * 4 + j + 1);
                        m_indices[iIndex++] = (mesh->vertexOffset + (i + 1) * 4 + j);
                    } else {
                        m_indices[iIndex++] = (mesh->vertexOffset + (i + 1) * 4 + j);
                        m_indices[iIndex++] = (mesh->vertexOffset + (i + 1) * 4 + j + 1);
                    }
                }
            }
        }
    }
    mesh->material = m_defaultMaterial;
    mesh->primitiveType = GL_TRIANGLES;
    if(!isAllocated) {
        m_meshes.push_back(mesh);
    }
}

bool Geometry::createBuffers(QOpenGLShaderProgram *program)
{
    if(m_vao.create()) {
        m_vao.bind();
        
        if(m_vertexBuffer.create()) {
            m_vertexBuffer.setUsagePattern(m_vertexBufferUsage);
            if(m_vertexBuffer.bind()) {
                m_vertexBuffer.allocate(m_vertices.constData(),
                                        m_vertices.length() * sizeof(QVector3D));
                program->enableAttributeArray(0);
                program->setAttributeBuffer(0, GL_FLOAT, 0, 3);
            } else {
                return false;
            }
        } else {
            return false;
        }

        if(m_normalBuffer.create()) {
            m_normalBuffer.setUsagePattern(m_normalBufferUsage);
            if(m_normalBuffer.bind()) {
                m_normalBuffer.allocate(m_normals.constData(),
                                        m_normals.length() * sizeof(QVector3D));
                program->enableAttributeArray(1);
                program->setAttributeBuffer(1, GL_FLOAT, 0, 3);
            } else {
                return false;
            }
        } else {
            return false;
        }
        
        if(m_textureCoordBuffer.create()) {
            m_textureCoordBuffer.setUsagePattern(m_textureCoordBufferUsage);
            if(m_textureCoordBuffer.bind()) {
                m_textureCoordBuffer.allocate(m_textureCoords.constData(),
                                              m_textureCoords.length() * sizeof(QVector2D));
                program->enableAttributeArray(2);
                program->setAttributeBuffer(2, GL_FLOAT, 0, 2);
            } else {
                return false;
            }
        } else {
            return false;
        }

        if(m_indexBuffer.create()) {
            m_indexBuffer.setUsagePattern(m_indexBufferUsage);
            if(m_indexBuffer.bind()) {
                m_indexBuffer.allocate(m_indices.constData(),
                                       m_indices.length() * sizeof(unsigned int));
            } else {
                return false;
            }
        } else {
            return false;
        }

        m_vao.release();
    } else {
        return false;
    }
    return true;
}

void Geometry::updateBuffers(QOpenGLShaderProgram *program)
{
    if(m_vertexBufferUsage == QOpenGLBuffer::DynamicDraw
            || m_vertexBufferUsage == QOpenGLBuffer::StreamDraw) {
        if(m_vertexBuffer.bind()) {
            m_vertexBuffer.write(0, m_vertices.constData(),
                                 m_vertices.length() * sizeof(QVector3D));
            m_vertexBuffer.release();
        }
    }

    if(m_normalBufferUsage == QOpenGLBuffer::DynamicDraw
            || m_normalBufferUsage == QOpenGLBuffer::StreamDraw) {
        if(m_normalBuffer.bind()) {
            m_normalBuffer.write(0, m_normals.constData(),
                                 m_normals.length() * sizeof(QVector3D));
            m_normalBuffer.release();
        }
    }

    if(m_textureCoordBufferUsage == QOpenGLBuffer::DynamicDraw
            || m_textureCoordBufferUsage == QOpenGLBuffer::StreamDraw) {
        if(m_textureCoordBuffer.bind()) {
            m_textureCoordBuffer.write(0, m_textureCoords.constData(),
                                       m_textureCoords.length() * sizeof(QVector2D));
            m_textureCoordBuffer.release();
        }
    }

    if(m_indexBufferUsage == QOpenGLBuffer::DynamicDraw
            || m_indexBufferUsage == QOpenGLBuffer::StreamDraw) {
        if(m_indexBuffer.bind()) {
            m_indexBuffer.write(0, m_indices.constData(),
                                m_indices.length() * sizeof(unsigned int));
            m_indexBuffer.release();
        }
    }
}

void Geometry::drawNode(GeometryManager *geometryManager, QOpenGLShaderProgram *program, Geometry::Node *node,
                        QMatrix4x4 cameraMatrix, QMatrix4x4 objectMatrix, float scaleFactor)
{
    QMatrix4x4 m;
    m.scale(scaleFactor);

    // Prepare matrices
    objectMatrix *= m * node->transformation;
    QMatrix4x4 modelViewMatrix = cameraMatrix * objectMatrix;
    QMatrix3x3 normalMatrix = modelViewMatrix.normalMatrix();

    program->setUniformValue("modelMatrix", objectMatrix);
    program->setUniformValue("viewMatrix", cameraMatrix);
    program->setUniformValue("normalMatrix", normalMatrix);

    // Draw each mesh in this node
    for(int i = 0; i < node->meshes.length(); i++) {
        QSharedPointer<QOpenGLTexture> texture = geometryManager->getTexture(node->meshes[i]->textureFile);
        if(!texture.isNull()) {
            glActiveTexture(GL_TEXTURE0);
            texture->bind(0);
        }
        program->setUniformValue("textureId", 0);
        program->setUniformValue("ambientColor", node->meshes[i]->material->ambientColor);
        program->setUniformValue("diffuseColor", node->meshes[i]->material->diffuseColor);
        program->setUniformValue("specularColor", node->meshes[i]->material->specularColor);
        program->setUniformValue("shininess", node->meshes[i]->material->shininess);
        glDrawElements(node->meshes[i]->primitiveType,
                       node->meshes[i]->indexCount,
                       GL_UNSIGNED_INT,
                       (const void *)(node->meshes[i]->indexOffset * sizeof(unsigned int)));
        if(!texture.isNull()) {
            texture->release();
        }
    }

    // Recursively draw this node's child nodes
    for(int i = 0; i < node->nodes.length(); i++) {
        drawNode(geometryManager, program, &node->nodes[i], cameraMatrix, objectMatrix, scaleFactor);
    }
}

void Geometry::calculateBoundingRadii()
{
    foreach(QVector3D vertex, m_vertices) {
        m_boundingRadii = qMax(m_boundingRadii, vertex.x());
        m_boundingRadii = qMax(m_boundingRadii, vertex.y());
        m_boundingRadii = qMax(m_boundingRadii, vertex.z());
    }
}

QSharedPointer<Geometry::MaterialInfo> Geometry::processMaterial(aiMaterial *material)
{
    QSharedPointer<MaterialInfo> m(new MaterialInfo);
    aiString mname;
    material->Get(AI_MATKEY_NAME, mname);
    if(mname.length > 0) {
        m->name = mname.C_Str();
    }

    aiColor3D ambientColor(0.0f, 0.0f, 0.0f);
    aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);
    aiColor3D specularColor(0.0f, 0.0f, 0.0f);
    float shininess = 0.0;

    material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
    material->Get(AI_MATKEY_SHININESS, shininess);

    m->ambientColor = QVector4D(ambientColor.r, ambientColor.g, ambientColor.b, 1.0);
    m->diffuseColor = QVector4D(diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0);
    m->specularColor = QVector4D(specularColor.r, specularColor.g, specularColor.b, 1.0);
    m->shininess = shininess;

    if(m->shininess == 0.0) {
        m->shininess = 5.0;
    }

    return m;
}

QSharedPointer<Geometry::Mesh> Geometry::processMesh(aiMesh *mesh)
{
    QSharedPointer<Mesh> newMesh(new Mesh);
    newMesh->name = mesh->mName.length != 0 ? mesh->mName.C_Str() : "";
    newMesh->indexOffset = m_indices.size();
    unsigned int indexCountBefore = m_indices.size();
    int vertexIndexOffset = m_vertices.size() / 3;

    // Get vertices
    if(mesh->mNumVertices > 0) {
        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
            aiVector3D &vec = mesh->mVertices[i];
            m_vertices.push_back(QVector3D(vec.x, vec.y, vec.z));
        }
    }
    // Get normals
    if(mesh->HasNormals()) {
        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
            aiVector3D &vec = mesh->mNormals[i];
            m_normals.push_back(QVector3D(vec.x, vec.y, vec.z));
        }
    }
    // Get texture coordinates
    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        if(mesh->HasTextureCoords(0)){
            aiVector3D &vec = mesh->mTextureCoords[0][i];
            m_textureCoords.push_back(QVector2D(vec.x, vec.y));
        } else {
            m_textureCoords.push_back(QVector2D(0, 0));
        }
    }
    // Get mesh indices
    for(unsigned int i = 0;  i < mesh->mNumFaces; i++) {
        aiFace *face = &mesh->mFaces[i];
        if(face->mNumIndices != 3) {
            std::cout << "Warning: Mesh face with not exactly 3 indices, ignoring this primitive." << std::endl;
            continue;
        }
        m_indices.push_back(face->mIndices[0] + vertexIndexOffset);
        m_indices.push_back(face->mIndices[1] + vertexIndexOffset);
        m_indices.push_back(face->mIndices[2] + vertexIndexOffset);
    }

    newMesh->indexCount = m_indices.size() - indexCountBefore;
    newMesh->material = m_materials.at(mesh->mMaterialIndex);
    newMesh->primitiveType = GL_TRIANGLES;

    return newMesh;
}

void Geometry::processNode(const aiScene *scene, aiNode *node, Geometry::Node *parentNode, Geometry::Node &newNode)
{
    newNode.name = node->mName.length != 0 ? node->mName.C_Str() : "";
    newNode.transformation = QMatrix4x4(node->mTransformation[0]);
    newNode.meshes.resize(node->mNumMeshes);
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        QSharedPointer<Mesh> mesh = m_meshes[node->mMeshes[i]];
        newNode.meshes[i] = mesh;
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        newNode.nodes.push_back(Node());
        processNode(scene, node->mChildren[i], parentNode, newNode.nodes[i]);
    }
}

void Geometry::getNodeTextures(QSet<QString> *textures, const Geometry::Node *node)
{
    for(int i = 0; i < node->meshes.length(); i++) {
        if(!textures->contains(node->meshes[i]->textureFile)) {
            textures->insert(node->meshes[i]->textureFile);
        }
    }
    for(int i = 0; i < node->nodes.length(); i++) {
        getNodeTextures(textures, &node->nodes[i]);
    }
}

QMatrix4x4 Geometry::removeTranslationAndScale(QMatrix4x4 transform)
{
    QMatrix4x4 normalTransform = transform;
    normalTransform.setColumn(3, QVector4D(0, 0, 0 , 1));
    normalTransform.setRow(3, QVector4D(0, 0, 0, 1));
    return normalTransform;
}

