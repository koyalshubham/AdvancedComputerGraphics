#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    qDebug() << "✓✓ MainWindow constructor";
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    qDebug() << "✗✗ MainWindow destructor";
    delete ui;

    meshes.clear();
    meshes.squeeze();
}

void MainWindow::loadOBJ() {
    OBJFile newModel = OBJFile(QFileDialog::getOpenFileName(this, "Import OBJ File", "models/", tr("Obj Files (*.obj)")));
    meshes.clear();
    meshes.squeeze();
    meshes.append( Mesh(&newModel) );

    ui->MainDisplay->updateMeshBuffers( meshes[0] );
    ui->MainDisplay->modelLoaded = true;

    ui->MainDisplay->update();
}

void MainWindow::on_RotateDial_valueChanged(int value) {
    ui->MainDisplay->rotAngle = value;
    ui->MainDisplay->updateMatrices();
}


void MainWindow::on_checkBox_stateChanged(int arg1)
{
    meshes.QAS(meshes);
    ui->MainDisplay->updateMeshBuffers(meshes);
}
