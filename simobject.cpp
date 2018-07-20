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
#include "simobject.h"

SimObject::~SimObject()
{

}

QQuaternion SimObject::computeRotation(QVector3D vector)
{
    return computeRotation(QVector3D(1, 0, 0), vector);
}

QQuaternion SimObject::computeRotation(QVector3D vector1, QVector3D vector2)
{
    QVector3D v1 = vector1.normalized();
    QVector3D v2 = vector2.normalized();
    QVector3D temp = QVector3D::crossProduct(v1, v2);
    QQuaternion q(v1.length() * v2.length() + QVector3D::dotProduct(v1, v2),
                  temp.x(), temp.y(), temp.z());
    q.normalize();
    return q;
}
