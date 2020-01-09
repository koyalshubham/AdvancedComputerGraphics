#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_1_Core>

#include <QGLFormat>

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {

    Q_OBJECT

public:
    MainView(QWidget *parent = 0) ;
        ~MainView();

protected:
    void initializeGL();
    void resizeGL( int w, int h );
    void paintGL();

    void keyPressEvent( QKeyEvent* e );

    GLuint CreateShader(GLenum type, const char *src);
    GLuint CreateShaderProgram(const std::vector<GLuint> &shaders);
private:
    GLint colorLocation;
    GLuint vao;
    GLuint program;

    // OpenGL State Information
    QOpenGLBuffer m_buffer;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLShaderProgram m_program;

    // Private Helpers
    void printVersionInformation();
};

#endif // MAINVIEW_H
