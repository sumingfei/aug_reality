#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class CameraThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
   MainWindow(CameraThread* thrd, QWidget *parent = 0);
   ~MainWindow();

signals:
    void closeApplication();

protected:
    void resizeEvent(QResizeEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    CameraThread* cameraThread;

};

#endif // MAINWINDOW_H
