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
#include "geometrymanager.h"

#include <iostream>

#include "cameratarget.h"
#include "genericspacecraft.h"
#include "geometryexample.h"
#include "planet.h"
#include "starfield.h"
#include "unitline.h"
#include "linestrip.h"
#include "fadinglinestrip.h"
#include "thrusterGeometry.h"
#include "fieldOfView.h"
#include "reactionwheeldisk.h"
#include "torquerodbar.h"
#include "startrackerfov.h"

GeometryManager::GeometryManager(QObject *parent)
    : QObject(parent)
{
    createDefaultGeometries();
}

GeometryManager::~GeometryManager()
{

}

QSharedPointer<Geometry> GeometryManager::addGeometry(QString name, QString filename)
{
    if(m_geometries.contains(name)) {
        return m_geometries[name];
    } else {
        QSharedPointer<Geometry> geometry(new Geometry);
        bool result = geometry->load(filename);
        if(result) {
            m_geometries.insert(name, geometry);
            return geometry;
        } else {
            // Something went wrong so return null pointer
            std::cout << "addGeometry failed for " << name.toStdString() 
                << "(" << filename.toStdString() << ")" << std::endl;
            return QSharedPointer<Geometry>();
        }
    }
}

QSharedPointer<Geometry> GeometryManager::addGeometry(QString name, QSharedPointer<Geometry> geometry)
{
    if(m_geometries.contains(name)) {
        return m_geometries[name];
    } else {
        m_geometries.insert(name, geometry);
        return geometry;
    }
}

QSharedPointer<Geometry> GeometryManager::getGeometry(QString name)
{
    if(m_geometries.contains(name)) {
        return m_geometries[name];
    } else {
        std::cout << "Geometry " << name.toStdString() << " unrecognized" << std::endl;
        return QSharedPointer<Geometry>();
    }
}

QList<QString> GeometryManager::getAvailableGeometries()
{
    return m_geometries.keys();
}

QSharedPointer<QOpenGLTexture> GeometryManager::addTexture(QString file)
{
    if(m_textures.contains(file)) {
        return m_textures[file];
    } else {
        QImage image;
        bool result = image.load(file);
        if(!result) {
            image.load(":/resources/images/default.png");
        }
        if(!image.isNull()) {
            QSharedPointer<QOpenGLTexture> texture(new QOpenGLTexture(image.mirrored()));
            texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
            texture->setMagnificationFilter(QOpenGLTexture::Linear);
            texture->setWrapMode(QOpenGLTexture::Repeat);
            m_textures.insert(file, texture);
            return texture;
        } else {
            std::cout << "addTexture failed for: " << file.toStdString() << std::endl;
            return QSharedPointer<QOpenGLTexture>();
        }
    }
}

QSharedPointer<QOpenGLTexture> GeometryManager::getTexture(QString file)
{
    if(m_textures.contains(file)) {
        return m_textures[file];
    } else {
        std::cout << "Texture file " << file.toStdString() << " unrecognized" << std::endl;
        return QSharedPointer<QOpenGLTexture>();
    }
}

bool GeometryManager::initializeGeometry(QString name, QOpenGLShaderProgram *program, bool forceInit)
{
    if(m_geometries.contains(name)) {
        if(m_geometries[name]->isInitialized()) {
            if(forceInit) {
                m_geometries[name]->cleanup();
                if(m_geometries[name]->initialize(program)) {
                    if(createTextures(m_geometries[name])) {
                        return true;
                    } else {
                        std::cout << "Failed to create textures for " << name.toStdString() << std::endl;
                        return false;
                    }
                } else {
                    std::cout << "Failed to initialize geometry " << name.toStdString() << std::endl;
                    return false;
                }
            } else {
                return true;
            }
        } else {
            m_geometries[name]->cleanup();
            if(m_geometries[name]->initialize(program)) {
                if(createTextures(m_geometries[name])) {
                    return true;
                } else {
                    std::cout << "Failed to create textures for " << name.toStdString() << std::endl;
                    return false;
                }
            } else {
                std::cout << "Failed to initialize geometry " << name.toStdString() << std::endl;
                return false;
            }
        }
    } else {
        std::cout << "Geometry " << name.toStdString() << " unrecognized" << std::endl;
        return false;
    }
}

void GeometryManager::drawGeometry(QString name, QOpenGLShaderProgram *program, QMatrix4x4 cameraMatrix, QMatrix4x4 objectMatrix,
                                   float scaleFactor, QSharedPointer<Geometry::MaterialInfo> defaultMaterialOverride,
                                   QSharedPointer<GeometryUpdateParameters> parameters)
{
    if(m_geometries.contains(name)) {
        QSharedPointer<Geometry::MaterialInfo> origDefault;
        if(!defaultMaterialOverride.isNull()) {
            origDefault = m_geometries[name]->getDefaultMaterial();
            m_geometries[name]->setDefaultMaterial(defaultMaterialOverride);
        }
        m_geometries[name]->update(parameters.data());
        m_geometries[name]->draw(this, program, cameraMatrix, objectMatrix, scaleFactor);
        if(!defaultMaterialOverride.isNull()) {
            m_geometries[name]->setDefaultMaterial(origDefault);
        }
    }
}

float GeometryManager::getGeometryBoundingRadii(QString name)
{
    if(m_geometries.contains(name)) {
        return m_geometries[name]->boundingRadii();
    }
    return -1.0f;
}

void GeometryManager::cleanupGeometries()
{
    QHashIterator<QString, QSharedPointer<Geometry> > i(m_geometries);
    while(i.hasNext()) {
        i.next();
        i.value()->cleanup();
    }
}

void GeometryManager::createDefaultGeometries()
{
    QSharedPointer<Geometry> geometry;
    QString textureFile;

    geometry = QSharedPointer<Geometry>(new CameraTarget);
    m_geometries.insert("CameraTarget", geometry);

    geometry = QSharedPointer<Geometry>(new GeometryExample);
    m_geometries.insert("GeometryExample", geometry);

    geometry = QSharedPointer<Geometry>(new GenericSpacecraft);
    m_geometries.insert("GenericSpacecraft", geometry);

    geometry = QSharedPointer<Geometry>(new Starfield);
    m_geometries.insert("Starfield", geometry);

    geometry = QSharedPointer<Geometry>(new Planet(CELESTIAL_EARTH));
    m_geometries.insert("Earth", geometry);

    geometry = QSharedPointer<Geometry>(new Planet(CELESTIAL_MARS));
    m_geometries.insert("Mars", geometry);

    geometry = QSharedPointer<Geometry>(new Planet(CELESTIAL_SUN));
    m_geometries.insert("Sun", geometry);

    geometry = QSharedPointer<Geometry>(new Planet(CELESTIAL_MOON));
    m_geometries.insert("Moon", geometry);

    geometry = QSharedPointer<Geometry>(new Planet(CELESTIAL_PHOBOS));
    m_geometries.insert("Phobos", geometry);

    geometry = QSharedPointer<Geometry>(new Planet(CELESTIAL_DEIMOS));
    m_geometries.insert("Deimos", geometry);

    geometry = QSharedPointer<Geometry>(new UnitLine);
    m_geometries.insert("UnitLine", geometry);

    geometry = QSharedPointer<Geometry>(new LineStrip);
    m_geometries.insert("LineStrip", geometry);

    geometry = QSharedPointer<Geometry>(new FadingLineStrip);
    m_geometries.insert("FadingLineStrip", geometry);
    
    geometry = QSharedPointer<Geometry>(new ThrusterGeometry);
    m_geometries.insert("Thruster", geometry);
    
    geometry = QSharedPointer<Geometry>(new FieldOfView);
    m_geometries.insert("FieldOfView", geometry);
    
    geometry = QSharedPointer<Geometry>(new ReactionWheelDisk);
    m_geometries.insert("DiskRW", geometry);
    
    geometry = QSharedPointer<Geometry>(new TorqueRodBar);
    m_geometries.insert("TorqueBar", geometry);
    
    geometry = QSharedPointer<Geometry>(new StarTrackerFOV);
    m_geometries.insert("FovST", geometry);
}

void GeometryManager::cleanupTexture(QSharedPointer<QOpenGLTexture> texture)
{
    texture->destroy();
}

void GeometryManager::cleanupTextures()
{
    QHashIterator<QString, QSharedPointer<QOpenGLTexture> > i(m_textures);
    while(i.hasNext()) {
        i.next();
        cleanupTexture(i.value());
    }
}

bool GeometryManager::createTextures(QSharedPointer<Geometry> geometry)
{
    QSet<QString> textures = geometry->textureFiles();
    QSet<QString>::iterator iter = textures.begin();
    while(iter != textures.end()) {
        if(!m_textures.contains(*iter)) {
            QImage image;
            bool result;
            if((*iter).isEmpty()) {
                image.load(":/resources/images/default.png");
                // TODO: Error check on default image?
            } else {
                result = image.load(*iter);
                if(!result) {
                    std::cout << "Error loading " << (*iter).toStdString()
                        << ", loading default texture instead." << std::endl;
                    image.load(":/resources/images/default.png");
                }
            }
            if(!image.isNull()) {
                QSharedPointer<QOpenGLTexture> texture(new QOpenGLTexture(image.mirrored()));
                texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
                texture->setMagnificationFilter(QOpenGLTexture::Linear);
                texture->setWrapMode(QOpenGLTexture::ClampToEdge);
                m_textures.insert(*iter, texture);
            } else {
                return false;
            }
        }
        ++iter;
    }
    return true;
}
