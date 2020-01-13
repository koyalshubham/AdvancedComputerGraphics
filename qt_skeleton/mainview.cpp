#include "mainview.h"
#include "vertex.h"

// Structure for per-quad vertex attributes.
struct QuadVertex
{
    GLfloat x, y;
    GLfloat size;
    GLubyte r, g, b, a;
};

static const size_t kRows    = 10;
static const size_t kColumns = 10;

static const float kInnerTessellationLevel = 1.0f;
static const float kOuterTessellationLevel = 1.0f;

//static const char * vs_source[] =
//{
//    "#version 410 core                                                                     \n"
//    "                                                                                                  \n"
//    "void main(void)                                                                         \n"
//    "{                                                                                                \n"
//    "    const vec4 vertices[] = vec4[](vec4( 0.25, -0.25, 0.5, 1.0),  \n"
//    "                                   vec4(-0.25, -0.25, 0.5, 1.0),                    \n"
//    "                                   vec4( 0.25,  0.25, 0.5, 1.0));                   \n"
//    "                                                                                                  \n"
//    "    gl_Position = vertices[gl_VertexID];                                      \n"
//    "}                                                                                                \n"
//};

//static const char * tcs_source[] =
//{
//    "#version 410 core                                                                                           \n"
//    "                                                                                                                        \n"
//    "layout (vertices = 3) out;                                                                                \n"
//    "                                                                                                                        \n"
//    "void main(void)                                                                                               \n"
//    "{                                                                                                                      \n"
//    "    if (gl_InvocationID == 0)                                                                             \n"
//    "    {                                                                                                                  \n"
//    "        gl_TessLevelInner[0] = 7.0;                                                                     \n"
//    "        gl_TessLevelOuter[0] = 5.0;                                                                    \n"
//    "        gl_TessLevelOuter[1] = 5.0;                                                                    \n"
//    "        gl_TessLevelOuter[2] = 5.0;                                                                    \n"
//    "    }                                                                                                                  \n"
//    "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;    \n"
//    "}                                                                                                                      \n"
//};

//static const char * tes_source[] =
//{
//    "#version 410 core                                                                 \n"
//    "                                                                                              \n"
//    "layout (triangles, equal_spacing, cw) in;                               \n"
//    "                                                                                              \n"
//    "void main(void)                                                                     \n"
//    "{                                                                                            \n"
//    "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +     \n"
//    "                  (gl_TessCoord.y * gl_in[1].gl_Position) +             \n"
//    "                  (gl_TessCoord.z * gl_in[2].gl_Position);                \n"
//    "}                                                                                            \n"
//};

//static const char * fs_source[] =
//{
//    "#version 410 core                                     \n"
//    "                                                                  \n"
//    "out vec4 color;                                           \n"
//    "                                                                   \n"
//    "void main(void)                                          \n"
//    "{                                                                 \n"
//    "    color = vec4(0.0, 0.8, 1.0, 1.0);              \n"
//    "}                                                                 \n"
//};


MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {

    qDebug() << "✓✓ MainView constructor";

    modelLoaded = false;
    wireframeMode = true;
    uniformUpdateRequired = true;

    reflectionLinesEnabled = false;
    reflectionLinesSize = 0.5f;
    gaussianEnabled = false;

    rotAngle = 0.0;
    dispRatio = 16.0/9.0;
    FoV = 120.0;
}


MainView::~MainView() {
//    // Actually destroy our OpenGL information
//    //m_vao.destroy();
//    //m_buffer.destroy();
//    //delete m_program;

//    glDeleteVertexArrays(1, &vao);
//    //glDeleteTextures(1,&vao);
//    glDeleteProgram(program);

    qDebug() << "✗✗ MainView destructor";

    glDeleteBuffers(1, &meshCoordsBO);
    glDeleteBuffers(1, &meshNormalsBO);
    glDeleteBuffers(1, &meshIndexBO);
    glDeleteVertexArrays(1, &meshVAO);

    debugLogger->stopLogging();

    delete mainShaderProg;
    delete debugLogger;
}

//GLuint MainView:: CreateShader(GLenum type, const char *src)
//{
//    GLuint shader = glCreateShader(type);

//    glShaderSource(shader, 1, &src, nullptr);
//    glCompileShader(shader);

//    GLint status = GL_FALSE;
//    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

//    if (status == GL_FALSE) {
//        GLchar log[512] = {0};
//        glGetShaderInfoLog(shader, 512, nullptr, log);

//        qDebug() << "Shader compilation failed" << (const char *) log;

//        glDeleteShader(shader);
//        return 0;
//    }

//    return shader;
//}

GLuint MainView:: CreateShaderProgram()
{
    qDebug() << ".. createShaderPrograms";

    mainShaderProg = new QOpenGLShaderProgram();
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/control.glsl");
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/evaluation.glsl");
    mainShaderProg->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");

    mainShaderProg->link();

    uniModelViewMatrix = glGetUniformLocation(mainShaderProg->programId(), "modelviewmatrix");
    uniProjectionMatrix = glGetUniformLocation(mainShaderProg->programId(), "projectionmatrix");
    uniNormalMatrix = glGetUniformLocation(mainShaderProg->programId(), "normalmatrix");

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

//Vertex MainView:: subdivide(Vertex& vertices){
//    Vertex newVertices = vertices;
//    unsigned int numVerts;

//    numVerts = vertices.size();
//    for (unsigned int k = 0; k<numVerts; k++){
//        newVertices.append(Vertex(vertexP))
//    }

//}

void MainView::updateMeshBuffers(Mesh& currentMesh) {
    makeCurrent();
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

    mainShaderProg->setUniformValue("reflectionLinesEnabled", reflectionLinesEnabled);
    mainShaderProg->setUniformValue("reflectionLinesSize", reflectionLinesSize);
    mainShaderProg->setUniformValue("gaussianEnabled", gaussianEnabled);

}
//void MainView::initializeGL()
//{

//    // Initialize OpenGL Backend
//  initializeOpenGLFunctions();
//  printVersionInformation();

//  program = glCreateProgram();
//  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
//  glShaderSource(vs, 1, vs_source, NULL);
//  glCompileShader(vs);

//  GLuint tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
//  glShaderSource(tcs, 1, tcs_source, NULL);
//  glCompileShader(tcs);

//  GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
//  glShaderSource(tes, 1, tes_source, NULL);
//  glCompileShader(tes);

//  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
//  glShaderSource(fs, 1, fs_source, NULL);
//  glCompileShader(fs);

//  glAttachShader(program, vs);
//  glAttachShader(program, tcs);
//  glAttachShader(program, tes);
//  glAttachShader(program, fs);

//  glLinkProgram(program);

//  glGenVertexArrays(1, &vao);
//  glBindVertexArray(vao);

//  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

//}

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

    CreateShaderProgram();
    createBuffers();

    // ---

    updateMatrices();
}
//void MainView::resizeGL( int w, int h )
//{
//    Q_UNUSED(w) Q_UNUSED(h)
//}

void MainView::resizeGL(int newWidth, int newHeight) {

    qDebug() << ".. resizeGL";

    dispRatio = float(newWidth)/float(newHeight);
    updateMatrices();

}

//void MainView::paintGL()
//{
////    glClearColor(0.0f, 0.1f, 0.1f, 1.0f);
////    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

//    static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
//    glClearBufferfv(GL_COLOR, 0, green);

//    glUseProgram(program);
//    glDrawArrays(GL_PATCHES, 0, 3);
//}

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

void MainView::renderMesh() {

    glBindVertexArray(meshVAO);

    if (wireframeMode) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    glDrawElements(GL_TRIANGLES, meshIBOSize, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

}

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
    case 'X':
        reflectionLinesEnabled = !reflectionLinesEnabled;
        update();
        break;
    case 'C':
        gaussianEnabled = !gaussianEnabled;
        update();
        break;
    }
}

void MainView::onMessageLogged( QOpenGLDebugMessage Message ) {
    qDebug() << " → Log:" << Message;
}



--
Shubham Koyal
S3555852

/*******************************************************************************
 * Private Helpers
 ******************************************************************************/

void MainView::printVersionInformation()
{
  QString glType;
  QString glVersion;
  QString glProfile;

  // Get Version Information
  glType = (context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
  glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

  // Get Profile Information
#define CASE(c) case QSurfaceFormat::c: glProfile = #c; break
  switch (format().profile())
  {
    CASE(NoProfile);
    CASE(CoreProfile);
    CASE(CompatibilityProfile);
  }
#undef CASE

  // qPrintable() will print our QString w/o quotes around it.
  qDebug() << qPrintable(glType) << qPrintable(glVersion) << "(" << qPrintable(glProfile) << ")";
}



