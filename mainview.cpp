#include "mainview.h"

MainView::MainView(QWidget *Parent) : QOpenGLWidget(Parent) {
    qDebug() << "✓✓ MainView constructor";

    modelLoaded = false;
    wireframeMode = true;
    uniformUpdateRequired = true;

    rotAngle = 0.0;
    dispRatio = 16.0/9.0;
    FoV = 120.0;
}

MainView::~MainView() {
    qDebug() << "✗✗ MainView destructor";

    glDeleteBuffers(1, &meshCoordsBO);
    glDeleteBuffers(1, &meshNormalsBO);
    glDeleteBuffers(1, &meshIndexBO);
    glDeleteVertexArrays(1, &meshVAO);

    debugLogger->stopLogging();

    delete mainShaderProg;
    delete debugLogger;
}

// ---

void MainView::createShaderPrograms() {
    qDebug() << ".. createShaderPrograms";

    mainShaderProg = new QOpenGLShaderProgram();
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshader.glsl");
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/tcshader.glsl");
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/teshader.glsl");
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshader.glsl");

    mainShaderProg->link();

    uniModelViewMatrix = glGetUniformLocation(mainShaderProg->programId(), "modelviewmatrix");
    uniProjectionMatrix = glGetUniformLocation(mainShaderProg->programId(), "projectionmatrix");
    uniNormalMatrix = glGetUniformLocation(mainShaderProg->programId(), "normalmatrix");

    uniInnerTessLevel = glGetUniformLocation(mainShaderProg->programId(), "InnerTessLevel");
    uniOuterTessLevel = glGetUniformLocation(mainShaderProg->programId(), "OuterTessLevel");
}

void MainView::createBuffers() {

    qDebug() << ".. createBuffers";

    glGenVertexArrays(1, &meshVAO);
    glBindVertexArray(meshVAO);

    glGenBuffers(1, &meshCoordsBO);
    glBindBuffer(GL_ARRAY_BUFFER, meshCoordsBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &meshNormalsBO);
    glBindBuffer(GL_ARRAY_BUFFER, meshNormalsBO);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &meshIndexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIndexBO);

    glBindVertexArray(0);
}

void MainView::updateMeshBuffers(Mesh& currentMesh) {
    //gather attributes for current mesh
    currentMesh.extractAttributes();
    QVector<QVector3D>& vertexCoords = currentMesh.getVertexCoords();
    QVector<QVector3D>& vertexNormals = currentMesh.getVertexNorms();
    QVector<unsigned int>& polyIndices = currentMesh.getPolyIndices();

    glBindBuffer(GL_ARRAY_BUFFER, meshCoordsBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D)*vertexCoords.size(), vertexCoords.data(), GL_DYNAMIC_DRAW);

    qDebug() << " → Updated meshCoordsBO";

    glBindBuffer(GL_ARRAY_BUFFER, meshNormalsBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D)*vertexNormals.size(), vertexNormals.data(), GL_DYNAMIC_DRAW);

    qDebug() << " → Updated meshNormalsBO";

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIndexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*polyIndices.size(), polyIndices.data(), GL_DYNAMIC_DRAW);

    qDebug() << " → Updated meshIndexBO";

    meshIBOSize = polyIndices.size();

    update();
}

void MainView::setInnerTessLevel(float level)
{
    InnerTessLevel = level;
    updateMatrices();
}

void MainView::setOuterTessLevel(float level)
{
    OuterTessLevel = level;
    updateMatrices();
}

void MainView::updateMatrices() {

    modelViewMatrix.setToIdentity();
    modelViewMatrix.translate(QVector3D(0.0, 0.0, -3.0));
    modelViewMatrix.scale(QVector3D(1.0, 1.0, 1.0));
    modelViewMatrix.rotate(rotAngle, QVector3D(0.0, 1.0, 0.0));

    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(FoV, dispRatio, 0.2, 4.0);

    normalMatrix = modelViewMatrix.normalMatrix();    

    uniformUpdateRequired = true;

    update();
}

void MainView::updateUniforms() {

    // mainShaderProg should be bound at this point!

    glUniformMatrix4fv(uniModelViewMatrix, 1, false, modelViewMatrix.data());
    glUniformMatrix4fv(uniProjectionMatrix, 1, false, projectionMatrix.data());
    glUniformMatrix3fv(uniNormalMatrix, 1, false, normalMatrix.data());

    //zxt::- Set the parameters for LOD
    glUniform1f(uniInnerTessLevel, InnerTessLevel);
    glUniform1f(uniOuterTessLevel, OuterTessLevel);
}

// ---

void MainView::initializeGL() {

    initializeOpenGLFunctions();
    qDebug() << ":: OpenGL initialized";

    debugLogger = new QOpenGLDebugLogger();
    connect( debugLogger, SIGNAL( messageLogged( QOpenGLDebugMessage ) ), this, SLOT( onMessageLogged( QOpenGLDebugMessage ) ), Qt::DirectConnection );

    if ( debugLogger->initialize() ) {
        qDebug() << ":: Logging initialized";
        debugLogger->startLogging( QOpenGLDebugLogger::SynchronousLogging );
        debugLogger->enableMessages();
    }

    QString glVersion;
    glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);
    // Default is GL_LESS
    glDepthFunc(GL_LEQUAL);

    // ---

    createShaderPrograms();
    createBuffers();

    // ---

    //zxt::- Set the parameters for LOD
    InnerTessLevel = 5.0f;
    OuterTessLevel = 5.0f;

    updateMatrices();

    //zxt::- Set Patch Parameter to use 6 vertices
    glPatchParameteri(GL_PATCH_VERTICES, 6);
}

void MainView::resizeGL(int newWidth, int newHeight) {

    qDebug() << ".. resizeGL";

    dispRatio = float(newWidth)/float(newHeight);
    updateMatrices();

}

void MainView::paintGL() {

    if (modelLoaded) {

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mainShaderProg->bind();

        if (uniformUpdateRequired) {
            updateUniforms();
            uniformUpdateRequired = false;
        }

        renderMesh();

        mainShaderProg->release();

    }
}

// ---

void MainView::renderMesh() {

    glBindVertexArray(meshVAO);

    if (wireframeMode) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    //zxt::- Use GL_PATCHES when tessellation is on
    glDrawElements(GL_PATCHES, meshIBOSize, GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_PATCHES, 0, 6);

    //glDrawElements(GL_TRIANGLES, meshIBOSize, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

}

// ---

void MainView::mousePressEvent(QMouseEvent* event) {
    setFocus();
}

void MainView::wheelEvent(QWheelEvent* event) {
    FoV -= event->delta() / 60.0;
    updateMatrices();
}

void MainView::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
    case 'Z':
        wireframeMode = !wireframeMode;
        update();
        break;
    }
}

// ---

void MainView::onMessageLogged( QOpenGLDebugMessage Message ) {
    qDebug() << " → Log:" << Message;
}
