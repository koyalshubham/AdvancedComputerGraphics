#-------------------------------------------------
#
# Project created by QtCreator 2016-12-01T23:50:32
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = qasApp
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    mainview.cpp \
    objfile.cpp \
    mesh.cpp \
    meshtools.cpp

HEADERS  += mainwindow.h \
    mainview.h \
    face.h \
    mesh.h \
    vertex.h \
    halfedge.h \
    meshtools.h \
    objfile.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
