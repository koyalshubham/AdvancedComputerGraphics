#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLDebugLogger>

#include <QOpenGLShaderProgram>

#include <QMouseEvent>
#include "mesh.h"

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {

    Q_OBJECT

public:
    MainView(QWidget *Parent = 0);
    ~MainView();

    bool modelLoaded;
    bool wireframeMode;

    float FoV;
    float dispRatio;
    float rotAngle;

    bool uniformUpdateRequired;

    void setSubdivisionLevel(int level);
    void updateMatrices();
    void updateUniforms();
    void updateMeshBuffers(Mesh& currentMesh);

protected:
    void initializeGL();
    void resizeGL(int newWidth, int newHeight);
    void paintGL();

    void renderMesh();

    void mousePressEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void keyPressEvent(QKeyEvent* event);

private:
    QOpenGLDebugLogger* debugLogger;

    QMatrix4x4 modelViewMatrix, projectionMatrix;
    QMatrix3x3 normalMatrix;

    // Uniforms
    GLint uniModelViewMatrix, uniProjectionMatrix, uniNormalMatrix;

    // ---

    QOpenGLShaderProgram* mainShaderProg;

    GLuint meshVAO, meshCoordsBO, meshNormalsBO, meshIndexBO;
    unsigned int meshIBOSize;

    // ---


    void createShaderPrograms();
    void createBuffers();

private slots:
    void onMessageLogged( QOpenGLDebugMessage Message );

};

#endif // MAINVIEW_H
