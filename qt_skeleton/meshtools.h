#ifndef MESHTOOLS_H
#define MESHTOOLS_H

#include "mesh.h"
#include <QVector3D>



QVector3D vertexPoint(HalfEdge* firstEdge);
QVector3D edgePoint(HalfEdge* firstEdge);
QVector3D edgeCP(QVector3D e, QVector3D p0, QVector3D p1);
QVector3D Q(float u, float v, float w, QVector3D p0, QVector3D p1, QVector3D p2, QVector3D e0, QVector3D e1, QVector3D e2);
QVector3D N(float u , float v, float w);

#endif // MESHTOOLS_H
