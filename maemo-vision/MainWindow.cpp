#include "mainwindow.h"
#include "CameraThread.h"
#include <QCloseEvent>

MainWindow::MainWindow(CameraThread* thrd, QWidget *parent) :
        QMainWindow(parent), cameraThread(thrd)
{

}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //app closes after the camera thread finishes
    event->ignore();
    emit closeApplication();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
}
