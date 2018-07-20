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
#include "planet.h"

#include <QString>
#include <QSharedPointer>

#include "astroConstants.h"

Planet::Planet(CelestialObject_t celestialObject)
    : Geometry()
{
    float xyradius = 1.0f;
    float zradius = 1.0;
    QString textureFile;
    QSharedPointer<MaterialInfo> material(new MaterialInfo);
    material->specularColor = QVector4D(0.0, 0.0, 0.0, 1.0);
    material->shininess = 100.0;

    switch(celestialObject) {
        case CELESTIAL_MERCURY:
            xyradius = (float)REQ_MERCURY;
            zradius = (float)REQ_MERCURY;
            material->diffuseColor = QVector4D(141.0, 141.0, 141.0, 255.0) / 255.0;
            break;
        case CELESTIAL_VENUS:
            xyradius = (float)REQ_VENUS;
            zradius = (float)REQ_VENUS;
            material->diffuseColor = QVector4D(238.0, 111.0, 42.0, 255.0) / 255.0;
            break;
        case CELESTIAL_EARTH:
            xyradius = (float)REQ_EARTH;
            zradius = (float)RP_EARTH;
            material->diffuseColor = QVector4D(1.0, 1.0, 1.0, 1.0);
            textureFile = "images/EarthHD.bmp";
            break;
        case CELESTIAL_MOON:
            xyradius = (float)REQ_MOON;
            zradius = (float)REQ_MOON;
            material->diffuseColor = QVector4D(211.0, 211.0, 211.0, 255.0) / 255.0;
            textureFile = "images/Moon.png";
            break;
        case CELESTIAL_MARS:
            xyradius = (float)REQ_MARS;
            zradius = (float)RP_MARS;
            material->diffuseColor = QVector4D(1.0, 1.0, 1.0, 1.0);
            textureFile = "images/MarsHD.bmp";
            break;
        case CELESTIAL_PHOBOS:
            xyradius = (float)REQ_PHOBOS;
            zradius = (float)REQ_PHOBOS;
            material->diffuseColor = QVector4D(1.0, 1.0, 1.0, 1.0);
            textureFile = "images/2kPhobos.png";
            break;
        case CELESTIAL_DEIMOS:
            xyradius = (float)REQ_DEIMOS;
            zradius = (float)REQ_DEIMOS;
            material->diffuseColor = QVector4D(1.0, 1.0, 1.0, 1.0);
            textureFile = "images/2kDeimos.png";
            break;
        case CELESTIAL_JUPITER:
            xyradius = (float)REQ_JUPITER;
            zradius = (float)REQ_JUPITER;
            material->diffuseColor = QVector4D(157.0, 150.0, 106.0, 255.0) / 255.0;
            break;
        case CELESTIAL_SATURN:
            xyradius = (float)REQ_SATURN;
            zradius = (float)REQ_SATURN;
            material->diffuseColor = QVector4D(222.0, 200.0, 150.0, 255.0) / 255.0;
            break;
        case CELESTIAL_URANUS:
            xyradius = (float)REQ_URANUS;
            zradius = (float)REQ_URANUS;
            material->diffuseColor = QVector4D(158.0, 191.0, 196.0, 255.0) / 255.0;
            break;
        case CELESTIAL_NEPTUNE:
            xyradius = (float)REQ_NEPTUNE;
            zradius = (float)REQ_NEPTUNE;
            material->diffuseColor = QVector4D(120.0, 92.0, 203.0, 255.0) / 255.0;
            break;
        case CELESTIAL_PLUTO:
            xyradius = (float)REQ_PLUTO;
            zradius = (float)REQ_PLUTO;
            material->diffuseColor = QVector4D(159.0, 167.0, 169.0, 255.0) / 255.0;
            break;
        case CELESTIAL_SUN:
            xyradius = (float)REQ_SUN;
            zradius = (float)REQ_SUN;
            material->diffuseColor = QVector4D(0.0, 0.0, 0.0, 1.0);
            textureFile = "images/Sun.bmp";
            break;
        case MAX_CELESTIAL:
            break;
    }

    // TODO: Adjust ambient color as necessary so not completely dark in shadow of planet
    if(celestialObject == CELESTIAL_SUN) {
        material->ambientColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    } else {
        material->ambientColor = material->diffuseColor * 0.1f;
    }

    // Create root node
    QSharedPointer<Node> root = getRootNode();

    float resolution = 2.5f; // degrees
    QSharedPointer<Mesh> spheroid(new Mesh);
    defineSpheroid(spheroid, xyradius, zradius, resolution);
    spheroid->material = material;
    spheroid->textureFile = textureFile;
    root->meshes.push_back(spheroid);
}

Planet::Planet(float equatorialRadius, float polarRadius, QVector4D color, QString textureFile)
{
    QSharedPointer<MaterialInfo> material(new MaterialInfo);
    material->specularColor = QVector4D(0.0, 0.0, 0.0, 1.0);
    material->shininess = 100.0;
    material->diffuseColor = color;
    // TODO: Adjust ambient color as necessary so not completely dark in shadow of planet
    material->ambientColor = material->diffuseColor * 0.1f;

    // Create root node
    QSharedPointer<Node> root = getRootNode();

    float resolution = 2.5f; // degrees
    QSharedPointer<Mesh> spheroid(new Mesh);
    defineSpheroid(spheroid, equatorialRadius, polarRadius, resolution);
    spheroid->material = material;
    spheroid->textureFile = textureFile;
    root->meshes.push_back(spheroid);
}

Planet::~Planet()
{

}

