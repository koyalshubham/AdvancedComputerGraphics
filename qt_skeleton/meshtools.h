#ifndef MESHTOOLS_H
#define MESHTOOLS_H

#include "mesh.h"
#include <QVector3D>



QVector3D vertexPoint(HalfEdge* firstEdge);
QVector3D edgePoint(HalfEdge* firstEdge);
vec3 edgeCP(vec3 e, vec3 p0, vec3 p1);
vec3 Q(float u, float v, float w, vec3 p0, vec3 p1, vec3 p2, vec3 e0, vec3 e1, vec3 e2);
vec3 P(float u, float v, float w);
vec3 N(float u , float v, float w);

#endif // MESHTOOLS_H
