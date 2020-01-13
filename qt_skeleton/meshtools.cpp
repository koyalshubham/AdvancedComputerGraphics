#include "meshtools.h"

const uniform QVector3D n0, n1, n2, p0, p1, p2;
const uniform QVector3D ne0, ne1, ne2, pe0, pe1, pe2;

int Mesh::countFacelessHalfedges(QVector<HalfEdge> hes) {
    int cnt = 0;
    for (HalfEdge he : hes) {
        if (he.polygon == nullptr) {
            cnt++;
        }
    }
    qDebug() << "Halfedges without face: " << cnt;
    return cnt;
}

// Mark vertices on the boundary as such and make sure the
//'out' attribute points towards the halfedge on the boundary
// so we don't have to search for the boundary halfedge later
void Mesh::setVertexIsEdge(QVector<HalfEdge>& halfedges) {
    for (int i = 0; i < halfedges.size(); i++) {
        if (halfedges[i].polygon == nullptr) {
            halfedges[i].target->isEdgePoint = true;
            halfedges[i].target->out = halfedges[i].next;
        }
    }
}

void Mesh::subdivideLoop(Mesh& mesh) {
    QVector<Vertex>& newVertices = mesh.getVertices();
    QVector<HalfEdge>& newHalfEdges = mesh.getHalfEdges();
    QVector<Face>& newFaces = mesh.getFaces();

    setVertexIsEdge(halfEdges);
    unsigned int numVerts, numHalfEdges, numFaces;
    unsigned int vIndex, hIndex, fIndex;
    HalfEdge* currentEdge;

    qDebug() << ":: Creating new Loop mesh";

    numVerts = vertices.size();
    numHalfEdges = halfEdges.size();
    numFaces = faces.size();

    // Reserve memory
    newVertices.reserve(numVerts + numHalfEdges / 2);
    newHalfEdges.reserve(2*numHalfEdges + 6*numFaces);
    newFaces.reserve(4*numFaces);

    // Duplicate existing vertex points
    for (unsigned int k = 0; k < numVerts; k++) {
        // Coords (x,y,z), Out, Valence, Index
        newVertices.push_back( Vertex( vertexPoint(vertices[k].out),
                                             nullptr,
                                             vertices[k].val,
                                             k) );
    }

    vIndex = numVerts;
    qDebug() << " * Created vertex points";

    // Create new vertices using an edge and the mask
    for (unsigned int k = 0; k < numHalfEdges; k++) {
        currentEdge = &halfEdges[k];

        //only create a new vertex per set of halfEdges
        if (k < currentEdge->twin->index) {
            // Coords (x,y,z), Out, Valence, Index
            newVertices.push_back( Vertex(edgePoint(currentEdge),
                                                nullptr,
                                                6,
                                                vIndex) );
            vIndex++;
        }
    }

    qDebug() << " * Created edge points";

    // Split halfedges
    splitHalfEdges(newVertices, newHalfEdges);

    qDebug() << " * Split halfedges";

    hIndex = 2*numHalfEdges;
    fIndex = 0;

    // Create new faces and halfedges on the place of each old face
    for (unsigned int k = 0; k < numFaces; k++) {
        currentEdge = faces[k].side;

        // Three outer faces

        for (unsigned int m = 0; m < 3; m++) {

            unsigned int s = currentEdge->prev->index;
            unsigned int t = currentEdge->index;

            // Side, Val, Index
            newFaces.push_back( Face(nullptr,
                                3,
                                fIndex) );

            newFaces[fIndex].side = &newHalfEdges[ 2*t ];

            // Target, Next, Prev, Twin, Poly, Index
            newHalfEdges.append(HalfEdge( newHalfEdges[2*s].target, &newHalfEdges[2*s+1], &newHalfEdges[ 2*t ],
                    nullptr, &newFaces[fIndex], hIndex ));

            newHalfEdges.append(HalfEdge( newHalfEdges[2*t].target,
                                         nullptr,
                                         nullptr,
                                         &newHalfEdges[hIndex],
                                         nullptr,
                                         hIndex+1 ));

            newHalfEdges[hIndex].twin = &newHalfEdges[hIndex+1];

            newHalfEdges[2*s+1].next = &newHalfEdges[2*t];
            newHalfEdges[2*s+1].prev = &newHalfEdges[hIndex];
            newHalfEdges[2*s+1].polygon = &newFaces[fIndex];

            newHalfEdges[2*t].next = &newHalfEdges[hIndex];
            newHalfEdges[2*t].prev = &newHalfEdges[2*s+1];
            newHalfEdges[2*t].polygon = &newFaces[fIndex];

            // For edge points
            newHalfEdges[2*t].target->out = &newHalfEdges[hIndex];

            hIndex += 2;
            fIndex++;
            currentEdge = currentEdge->next;

        }

        // Inner face
        // Side, Val, Index

        newFaces.append(Face(&newHalfEdges[ hIndex-1 ], 3, fIndex));

        for (unsigned int m = 0; m < 3; m++) {

            if (m == 2) {
                newHalfEdges[hIndex - 1].next = &newHalfEdges[hIndex - 5];
            } else {
                newHalfEdges[hIndex - 5 + 2*m].next = &newHalfEdges[hIndex - 5 + 2*(m+1)];
            }

            if (m == 0) {
                newHalfEdges[hIndex - 5].prev = &newHalfEdges[hIndex - 1];
            } else {
                newHalfEdges[hIndex - 5 + 2*m].prev = &newHalfEdges[hIndex - 5 + 2*(m-1)];
            }

            newHalfEdges[hIndex - 5 + 2*m].polygon = &newFaces[fIndex];

        }

        fIndex++;

    }

    qDebug() << " * Created faces";

    // Find boundary halfedges and fill their Prev and Next pointers
    fillBoundaryHalfedges(halfEdges, newHalfEdges);

    qDebug() << " * Filled prev/next for boundary halfedges";

    // set outs for updated vertices
    for (unsigned int k = 0; k < numVerts; k++) {
        newVertices[k].out = &newHalfEdges[ 2 * vertices[k].out->index ];
    }

}

// ---

QVector3D vertexPoint(HalfEdge* firstEdge) {
    unsigned short n;
    QVector3D vertexPt;
    Vertex* currentVertex;

    currentVertex = firstEdge->twin->target;
    n = currentVertex->val;

    // Vertex sits on the boundary
    if (currentVertex->isEdgePoint) {
        vertexPt = QVector3D();
        // Mask 1/8, 3/4, 1/8 for the next vertex, current vertex, previous vertex respectively
        vertexPt += currentVertex->out->target->coords;
        vertexPt += 6* currentVertex->coords;
        vertexPt += currentVertex->out->prev->prev->target->coords;
        vertexPt /= 8;
        return vertexPt;
    }
    // Allocate more for non-boundary vertices
    unsigned short k;
    QVector3D sumStarPts, sumFacePts;
    float stencilValue;
    HalfEdge* currentEdge;

    sumStarPts = QVector3D();
    sumFacePts = QVector3D();
    currentEdge = firstEdge;

    for (k=0; k<n; k++) {
        sumStarPts += currentEdge->target->coords;
        currentEdge = currentEdge->prev->twin;
    }

    // Warren's rules
    if (n == 3) {
        stencilValue = 3.0/16.0;
    }
    else {
        stencilValue = 3.0/(8*n);
    }

    vertexPt = (1.0 - n*stencilValue) * currentVertex->coords + stencilValue * sumStarPts;


    return vertexPt;

}

QVector3D edgePoint(HalfEdge* firstEdge) {
    QVector3D EdgePt;
    HalfEdge* currentEdge;

    EdgePt = QVector3D();
    if (firstEdge->twin->polygon == nullptr) { currentEdge = firstEdge->twin; }
    else {
        currentEdge = firstEdge;
    }

    if (currentEdge->polygon == nullptr) {
        EdgePt = 0.5 * currentEdge->target->coords;
        EdgePt += 0.5 * currentEdge->twin->target->coords;
    } else {
        EdgePt  = 6.0 * currentEdge->target->coords;
        EdgePt += 2.0 * currentEdge->next->target->coords;
        EdgePt += 6.0 * currentEdge->twin->target->coords;
        EdgePt += 2.0 * currentEdge->twin->next->target->coords;
        EdgePt /= 16.0;
    }
    return EdgePt;
}

// Sets the unset pointers for newly made boundary halfedges
void Mesh::fillBoundaryHalfedges(QVector<HalfEdge>& oldHalfEdges, QVector<HalfEdge>& newHalfEdges) {
    for (int i=0; i < newHalfEdges.size(); i++) {
        if (newHalfEdges[i].polygon == nullptr) {
            if (i%2 == 0) { // Old index was i/2
                newHalfEdges[i].prev = &newHalfEdges[(oldHalfEdges[i/2].prev->index)*2+1];
                newHalfEdges[i].next = &newHalfEdges[i+1];
            } else {        // Old index was (i-1)/2
                newHalfEdges[i].prev = &newHalfEdges[i-1];
                newHalfEdges[i].next = &newHalfEdges[(oldHalfEdges[(i-1)/2].next->index)*2];
            }
        }
    }
}

void Mesh::splitHalfEdges(QVector<Vertex>& newVertices, QVector<HalfEdge>& newHalfEdges) {
    unsigned int vIndex = vertices.size();

    for (unsigned int k = 0; k < halfEdges.size(); ++k) {
        HalfEdge* currentEdge = &halfEdges[k];
        unsigned int m = currentEdge->twin->index;

        // Target, Next, Prev, Twin, Poly, Index
        newHalfEdges.append(HalfEdge(nullptr,
                                              nullptr,
                                              nullptr,
                                              nullptr,
                                              nullptr,
                                              2*k));

        newHalfEdges.append(HalfEdge(nullptr,
                                              nullptr,
                                              nullptr,
                                              nullptr,
                                              nullptr,
                                              2*k+1));

        if (k < m) {
            newHalfEdges[2*k].target = &newVertices[ vIndex ];
            newHalfEdges[2*k+1].target = &newVertices[ currentEdge->target->index ];
            vIndex++;
        }
        else {
            newHalfEdges[2*k].target = newHalfEdges[2*m].target;
            newHalfEdges[2*k+1].target = &newVertices[ currentEdge->target->index ];

            // Assign Twinss
            newHalfEdges[2*k].twin = &newHalfEdges[2*m+1];
            newHalfEdges[2*k+1].twin = &newHalfEdges[2*m];
            newHalfEdges[2*m].twin = &newHalfEdges[2*k+1];
            newHalfEdges[2*m+1].twin = &newHalfEdges[2*k];
        }
    }

    // Note that Next, Prev and Poly are not yet assigned at this point.

}
//QAS
QVector3D edgeCP(QVector3D e, QVector3D p0, QVector3D p1){
    return(e*4.0 - p0 - p1) *0.5;
}

QVector3D Q(float u, float v, float w, QVector3D p0, QVector3D p1, QVector3D p2, QVector3D e0, QVector3D e1, QVector3D e2){
    QVector3D n200 = p0, n020 = p1, n002 =p2;
    QVector3D n110 = edgeCP(e0, p0, p1);
    QVector3D n101 = edgeCP(e2, p1, p2);
    QVector3D n011 = edgeCP(e1, p1, p2);
    return w * (n200*w + n110*2*u) + u * (n020*u + n011*2*v) + v* (n002*v + n101*2*w);
}

QVector3D P(float u, float v, float w){
    return Q(u, v, w, p0, p1, p2, pe0, pe2, pe2);
}

QVector3D N(float u , float v, float w){
    return Q(u, v, w, n0, n1, n2, ne0, ne2, ne2);
}

void Mesh::QAS(Mesh &meshes){

    unsigned short k;
    int value = 1;
    //One subdivide step
    for (k = meshes.size(); k < value + 1; k++) {
        meshes.append(Mesh());
        //meshes.append(meshes[k-1].subdivideLoop());
        meshes[k-1].subdivideLoop(meshes[k]);
    }
    QVector3D u = meshes.vertexCoords;
    QVector3D v = meshes.vertexNormals;
    QVector3D w = 1.0 - u - v;
    meshes.vertices = P(u, v, w);
    meshes.vertexNormals = normalize(N(u,v,w));
}
