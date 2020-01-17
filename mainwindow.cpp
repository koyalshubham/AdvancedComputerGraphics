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

    //zxt::- Do one single subdivision step
    on_SubdivSteps_valueChanged(1);

    ui->MainDisplay->updateMeshBuffers( meshes[0] );
    ui->MainDisplay->modelLoaded = true;

    ui->MainDisplay->update();
}

void MainWindow::on_RotateDial_valueChanged(int value) {
    ui->MainDisplay->rotAngle = value;
    ui->MainDisplay->updateMatrices();
}

void MainWindow::on_SubdivSteps_valueChanged(int value) {
    unsigned short k;
    for (k = meshes.size(); k < value + 1; k++) {
        meshes.append(Mesh());
        //meshes.append(meshes[k-1].subdivideLoop());
        meshes[k-1].subdivideLoop(meshes[k]);
    }

    //ui->MainDisplay->setSubdivisionLevel(int value);
    ui->MainDisplay->updateMeshBuffers( meshes[value] );
}

void MainWindow::on_LoadOBJ_clicked() {
    loadOBJ();
    ui->LoadOBJ->setEnabled(true);
    ui->SubdivSteps->setEnabled(true);
}

void MainWindow::on_sBoxInnerTL_valueChanged(int value)
{
    ui->MainDisplay->setInnerTessLevel(float(value));
}

void MainWindow::on_sBoxOuterTL_valueChanged(int value)
{
    ui->MainDisplay->setOuterTessLevel(float(value));
}
