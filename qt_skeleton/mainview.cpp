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

static const char * vs_source[] =
{
    "#version 410 core                                                                     \n"
    "                                                                                                  \n"
    "void main(void)                                                                         \n"
    "{                                                                                                \n"
    "    const vec4 vertices[] = vec4[](vec4( 0.25, -0.25, 0.5, 1.0),  \n"
    "                                   vec4(-0.25, -0.25, 0.5, 1.0),                    \n"
    "                                   vec4( 0.25,  0.25, 0.5, 1.0));                   \n"
    "                                                                                                  \n"
    "    gl_Position = vertices[gl_VertexID];                                      \n"
    "}                                                                                                \n"
};

static const char * tcs_source[] =
{
    "#version 410 core                                                                                           \n"
    "                                                                                                                        \n"
    "layout (vertices = 3) out;                                                                                \n"
    "                                                                                                                        \n"
    "void main(void)                                                                                               \n"
    "{                                                                                                                      \n"
    "    if (gl_InvocationID == 0)                                                                             \n"
    "    {                                                                                                                  \n"
    "        gl_TessLevelInner[0] = 7.0;                                                                     \n"
    "        gl_TessLevelOuter[0] = 5.0;                                                                    \n"
    "        gl_TessLevelOuter[1] = 5.0;                                                                    \n"
    "        gl_TessLevelOuter[2] = 5.0;                                                                    \n"
    "    }                                                                                                                  \n"
    "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;    \n"
    "}                                                                                                                      \n"
};

static const char * tes_source[] =
{
    "#version 410 core                                                                 \n"
    "                                                                                              \n"
    "layout (triangles, equal_spacing, cw) in;                               \n"
    "                                                                                              \n"
    "void main(void)                                                                     \n"
    "{                                                                                            \n"
    "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +     \n"
    "                  (gl_TessCoord.y * gl_in[1].gl_Position) +             \n"
    "                  (gl_TessCoord.z * gl_in[2].gl_Position);                \n"
    "}                                                                                            \n"
};

static const char * fs_source[] =
{
    "#version 410 core                                     \n"
    "                                                                  \n"
    "out vec4 color;                                           \n"
    "                                                                   \n"
    "void main(void)                                          \n"
    "{                                                                 \n"
    "    color = vec4(0.0, 0.8, 1.0, 1.0);              \n"
    "}                                                                 \n"
};


MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {

}

MainView::~MainView() {
    // Actually destroy our OpenGL information
    //m_vao.destroy();
    //m_buffer.destroy();
    //delete m_program;

    glDeleteVertexArrays(1, &vao);
    //glDeleteTextures(1,&vao);
    glDeleteProgram(program);

}

GLuint MainView:: CreateShader(GLenum type, const char *src)
{
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        GLchar log[512] = {0};
        glGetShaderInfoLog(shader, 512, nullptr, log);

        qDebug() << "Shader compilation failed" << (const char *) log;

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint MainView:: CreateShaderProgram(const std::vector<GLuint> &shaders)
{
    GLuint program = glCreateProgram();

    for (GLuint shader : shaders) {
        glAttachShader(program, shader);
    }

    glLinkProgram(program);

    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        GLchar log[512] = {0};
        glGetProgramInfoLog(program, 512, nullptr, log);

        qDebug() << "Shader program link failed" << (const char *) log;

        glDeleteProgram(program);
        return 0;
    }

    return program;
}

void MainView::initializeGL()
{

    // Initialize OpenGL Backend
  initializeOpenGLFunctions();
  printVersionInformation();

  program = glCreateProgram();
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, vs_source, NULL);
  glCompileShader(vs);

  GLuint tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
  glShaderSource(tcs, 1, tcs_source, NULL);
  glCompileShader(tcs);

  GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
  glShaderSource(tes, 1, tes_source, NULL);
  glCompileShader(tes);

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, fs_source, NULL);
  glCompileShader(fs);

  glAttachShader(program, vs);
  glAttachShader(program, tcs);
  glAttachShader(program, tes);
  glAttachShader(program, fs);

  glLinkProgram(program);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

}

void MainView::resizeGL( int w, int h )
{
    Q_UNUSED(w) Q_UNUSED(h)
}

void MainView::paintGL()
{
//    glClearColor(0.0f, 0.1f, 0.1f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
    glClearBufferfv(GL_COLOR, 0, green);

    glUseProgram(program);
    glDrawArrays(GL_PATCHES, 0, 3);
}

void MainView::keyPressEvent( QKeyEvent* e )
{

}

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



