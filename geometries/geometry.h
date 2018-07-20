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
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <qopengl.h>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QSharedPointer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_2_1>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

class GeometryManager;

// Class to be inherited by children of Geometry in order to pass in dynamic parameters
class GeometryUpdateParameters
{
public:
};

class Geometry : public QOpenGLFunctions_2_1
{
public:
    Geometry();
    ~Geometry();

    class MaterialInfo
    {
    public:
        QString name;
        QVector4D ambientColor;
        QVector4D diffuseColor;
        QVector4D specularColor;
        float shininess;

        MaterialInfo() {}
        MaterialInfo(QVector4D color)
            : ambientColor(color)
            , diffuseColor(QVector4D(0, 0, 0, 0))
            , specularColor(QVector4D(0, 0, 0, 0))
            , shininess(1) {}
    };

    typedef struct Mesh {
        QString name;
        unsigned int indexOffset;
        unsigned int indexCount;
        unsigned int vertexOffset;
        unsigned int vertexCount;
        unsigned int primitiveType;
        QString textureFile;
        QSharedPointer<MaterialInfo> material;
    } Mesh;

    typedef struct Node {
        QString name;
        QMatrix4x4 transformation;
        QVector<QSharedPointer<Mesh> > meshes;
        QVector<Node> nodes;
    } Node;

    bool load(QString pathToFile);

    bool initialize(QOpenGLShaderProgram *program);
    bool isInitialized() {
        return m_vao.isCreated();
    }
    virtual void update(GeometryUpdateParameters *) {}
    void draw(GeometryManager *geometryManager, QOpenGLShaderProgram *program,
              QMatrix4x4 cameraMatrix, QMatrix4x4 objectMatrix, float scaleFactor);
    void cleanup();

    QSet<QString> textureFiles();
    float boundingRadii();

    QSharedPointer<MaterialInfo> getDefaultMaterial() {
        return m_defaultMaterial;
    }
    void setDefaultMaterial(QSharedPointer<MaterialInfo> material);

protected:
    QSharedPointer<Node> getRootNode();

    void setVertexBufferUsage(QOpenGLBuffer::UsagePattern usage) {
        m_vertexBufferUsage = usage;
    }
    void setNormalBufferUsage(QOpenGLBuffer::UsagePattern usage) {
        m_normalBufferUsage = usage;
    }
    void setTextureCoordBufferUsage(QOpenGLBuffer::UsagePattern usage) {
        m_textureCoordBufferUsage = usage;
    }
    void setIndexBufferUsage(QOpenGLBuffer::UsagePattern usage) {
        m_indexBufferUsage = usage;
    }

    QSharedPointer<MaterialInfo> addMaterial(QString name, QVector4D ambientColor,
            QVector4D diffuseColor, QVector4D specularColor,
            float shininess);

    // Allocates (if requested) the buffers for the number of vertices and indices specified
    // if already allocated, adjusts numVertices and numIndices to avoid buffer overrun
    void allocate(bool isAllocate, QSharedPointer<Mesh> mesh, int &numVertices, int &numIndices);
    // Define a line between the two points specified
    // transform = transformation to apply to default geometry
    // isAllocated = has the geometry already been allocated (vs dynamically changing)
    void defineLine(QSharedPointer<Mesh> mesh, QVector<QVector3D> points,
                    QMatrix4x4 transform = QMatrix4x4(), bool isAllocated = false);
    // Define regular cuboid, origin in corner with cuboid extending in positive
    //  x, y, and z directions
    // xdim, ydim, zdim = dimensions in x, y, and z directions
    // transform = transformation to apply to default geometry
    // isNormalsOut = true if normals point out from center
    // isAllocated = has the geometry already been allocated (vs dynamically changing)
    void defineCuboid(QSharedPointer<Mesh> mesh, float xdim, float ydim, float zdim,
                      QMatrix4x4 transform = QMatrix4x4(),
                      bool isNormalsOut = true, bool isAllocated = false);
    //
    void defineSteradianPortionSphere(QSharedPointer<Mesh> mesh, float angle, float radius, float res,
                                       QMatrix4x4 transform, bool isNormalsOut, bool isAllocated);

    // Define spheroid, origin in center, revolved around z axis
    // xyradius, zradius = radii of spheroid in x/y and z directions
    // res = deg, resolution of grid
    // transform = transformation to apply to default geometry
    // isNormalsOut = true if normals point out from center
    // isAllocated = has the geometry already been allocated (vs dynamically changing)
    void defineSpheroid(QSharedPointer<Mesh> mesh, float xyradius, float zradius, float res,
                        QMatrix4x4 transform = QMatrix4x4(),
                        bool isNormalsOut = true, bool isAllocated = false);
    // Define conical frustum with origin in center of base
    //  positive x increments have normals pointing out, and vice versa
    //  expanding radii disks have normals that point -x, and vice versa
    // x = x location of vertices
    // r = radius at each x location
    // numSlices = number of slices to draw, higher numbers means more triangles
    // transform = transformation to apply to default geometry
    // isAllocated = has the geometry already been allocated (vs dynamically changing)
    void defineConicalFrustum(QSharedPointer<Mesh> mesh, QVector<float> x, QVector<float> r,
                              unsigned int numSlices, QMatrix4x4 transform = QMatrix4x4(),
                              bool isNormalsOut = true, bool isAllocated = false);
    // Define circular plane (with optional hole) with origin in center, lying in Y-Z plane
    // r1, r2 = inner (hole) and outer radii of disk
    // numSlices = number of slices to draw, higher numbers means more triangles
    // transform = transformation to apply to default geometry
    // isNormalsOut = if true normals point in +x direction
    // isAllocated = has the geometry already been allocated (vs dynamically changing)
    void defineCircularPlane(QSharedPointer<Mesh> mesh, float r1, float r2, unsigned int numSlices,
                             QMatrix4x4 transform = QMatrix4x4(), bool isNormalsOut = true,
                             bool isAllocated = false);
    // Define rectangular prismoid with origin in center of base extending in x direction
    // x = x location of vertices
    // ywidth, zwidth = y and z widths at x locations
    // transform = transformation to apply to default geometry
    // isNormalsOut = true if normals point out from center
    // isAllocated = has the geometry already been allocated (vs dynamically changing)
    void defineRectangularPrismoid(QSharedPointer<Mesh> mesh, QVector<float> x, QVector<float> ywidth,
                                   QVector<float> zwidth, QMatrix4x4 transform = QMatrix4x4(),
                                   bool isNormalsOut = true, bool isAllocated = false);
    // Define rectangular plane (with optional hole) with origin in center, lying in Y-Z plane
    // y1, y2 = inner (hole) and outer dimensions of plane in y direction
    // z1, z2 = inner (hole) and outer dimensions of plane in z direction
    // transform = transformation to apply to default geometry
    // isNormalsOut = if true normals point in +x direction
    // isAllocated = has the geometry already been allocated (vs dynamically changing)
    void defineRectangularPlane(QSharedPointer<Mesh> mesh, float y1, float y2, float z1, float z2,
                                QMatrix4x4 transform = QMatrix4x4(), bool isNormalOut = true,
                                bool isAllocated = false);

private:
    QVector<QVector3D> m_vertices;
    QVector<QVector3D> m_normals;
    QVector<QVector2D> m_textureCoords;
    QVector<unsigned int> m_indices;

    QSharedPointer<MaterialInfo> m_defaultMaterial;
    QVector<QSharedPointer<MaterialInfo> > m_materials;
    QVector<QSharedPointer<Mesh> > m_meshes;
    QSharedPointer<Node> m_rootNode;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer::UsagePattern m_vertexBufferUsage;
    QOpenGLBuffer m_normalBuffer;
    QOpenGLBuffer::UsagePattern m_normalBufferUsage;
    QOpenGLBuffer m_textureCoordBuffer;
    QOpenGLBuffer::UsagePattern m_textureCoordBufferUsage;
    QOpenGLBuffer m_indexBuffer;
    QOpenGLBuffer::UsagePattern m_indexBufferUsage;

    bool m_isOpenGLInitialized;
    bool createBuffers(QOpenGLShaderProgram *program);
    void updateBuffers(QOpenGLShaderProgram *program);
    void drawNode(GeometryManager *geometryManager, QOpenGLShaderProgram *program,
                  Node *node, QMatrix4x4 cameraMatrix, QMatrix4x4 objectMatrix, float scaleFactor);

    float m_boundingRadii;
    // Calculate the bounding radii of a geometry
    void calculateBoundingRadii();

    QSharedPointer<MaterialInfo> processMaterial(aiMaterial *material);
    QSharedPointer<Mesh> processMesh(aiMesh *mesh);
    void processNode(const aiScene *scene, aiNode *node, Node *parentNode, Node &newNode);

    void getNodeTextures(QSet<QString> *textures, const Node *node);

    QMatrix4x4 removeTranslationAndScale(QMatrix4x4 transform);
};

#endif // GEOMETRY_H
