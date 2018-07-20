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
#ifndef THRUSTER_H
#define THRUSTER_H

#include "geometry.h"

class ThrusterGeometryUpdateParameters : public GeometryUpdateParameters
{
public:
    float length;
    float plumeDiameter;
};

class ThrusterGeometry : public Geometry {

public:
    ThrusterGeometry();
    ~ThrusterGeometry();
    
    virtual void update(GeometryUpdateParameters *parameters);

protected:
    QSharedPointer<Mesh> m_thrusterOuterMesh;
    QSharedPointer<Mesh> m_thrusterInnerMesh;
    QSharedPointer<Mesh> m_thrusterEndOuterMesh;
    QSharedPointer<Mesh> m_thrusterEndInnerMesh;
    QVector<float> m_x;
    QVector<float> m_r;
    unsigned int m_numSlices;
};
#endif /* defined(THRUSTER_H) */
