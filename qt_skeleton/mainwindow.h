#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "obj.h"
#include <QFileDialog>
#include "mesh.h"
#include "meshtools.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void loadOBJ();
    QVector<Mesh> meshes;

private slots:
    void on_RotateDial_valueChanged(int value);
    void on_SubdivSteps_valueChanged(int value);
    void on_LoadOBJ_clicked();

    void on_checkBox_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
