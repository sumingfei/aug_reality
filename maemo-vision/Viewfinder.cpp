#include "Viewfinder.h"
#include "OverlayWidget.h"
#include "CameraThread.h"
#include "RecognitionEngine.h"
#include "AppState.h"

#include <QtGui>
#include <QMutex>

#include "CameraParameters.h"
#include "UserDefaults.h"

#include<vector>
#define TREE_WIDTH 11

Viewfinder::Viewfinder(AppState* appState, QWidget *parent):
        QWidget(parent), appState(appState), isProcessingFrames(false){

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    //layout->setSpacing(0);

    // Make an overlay for displaying viewfinder frames
    overlay = new OverlayWidget(appState, this);
    overlay->setFixedSize(640, 480);
    layout->addWidget(overlay);

    // Make space for the parameter button tree drawing
    layout->addSpacing(TREE_WIDTH);
    //layout->addSpacing(0);

    // Make some buttons down the right
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    //buttonLayout->setSpacing(2);
    //buttonLayout->setSpacing(0);
    layout->addLayout(buttonLayout);

    QPushButton *openFileButton = new QPushButton(this);
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/images/images/file-open-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
    openFileButton->setIcon(icon);
    openFileButton->setIconSize(QSize(70, 70));
    //openFileButton->setFlat(true);

    buttonLayout->addWidget(openFileButton);

    /*qq add some buttons*/
    QPushButton *chooseObjectButton = new QPushButton("Object");
    QPushButton *chooseOrientationButton = new QPushButton("Orientation");
    QPushButton *plusButton = new QPushButton("+");
    QPushButton *minusButton = new QPushButton("-");
    chooseObjectButton->setFixedSize(160, 70);
    chooseOrientationButton->setFixedSize(160, 70);
    //deleteObjectButton->setFixedSize(160, 80);

    plusButton->setFixedSize(80,64);
    minusButton->setFixedSize(80,64);

    QHBoxLayout *plus_minus_layout = new QHBoxLayout();
    plus_minus_layout->addWidget(plusButton);
    plus_minus_layout->addWidget(minusButton);
    QMenu *chooseObjectMenu = new QMenu(tr("&Obj"), this);
    QAction *cube = new QAction (tr("&Cube"), this);
    QAction *teapot = new QAction (tr("&Teapot"), this);
    QAction *gourd = new QAction (tr("&Gourd"), this);
    //connect()
    chooseObjectMenu->addAction(cube);
    chooseObjectMenu->addAction(teapot);
    chooseObjectMenu->addAction(gourd);
    chooseObjectButton->setMenu(chooseObjectMenu);

    QMenu *orientationMenu = new QMenu(tr("&Orient"), this);
    QAction *x = new QAction(tr("&X"), this);
    QAction *y = new QAction(tr("&Y"), this);
    QAction *z = new QAction(tr("&Z"), this);
    QAction *rotate_x = new QAction(tr("&Rotate_X"), this);
    QAction *rotate_y = new QAction(tr("&Rotate_Y"), this);
    QAction *rotate_z = new QAction(tr("&Rotate_Z"), this);
    orientationMenu->addAction(x);
    orientationMenu->addAction(y);
    orientationMenu->addAction(z);
    orientationMenu->addAction(rotate_x);
    orientationMenu->addAction(rotate_y);
    orientationMenu->addAction(rotate_z);
    chooseOrientationButton->setMenu(orientationMenu);


    buttonLayout->addWidget(chooseObjectButton);
    buttonLayout->addWidget(chooseOrientationButton);
    buttonLayout->addLayout(plus_minus_layout);
    //buttonLayout->addWidget(plusButton);
    //buttonLayout->addWidget(minusButton);

    //buttonLayout->addWidget(deleteObjectButton);
    this->setLayout(layout);

    QObject::connect(cube, SIGNAL(triggered()),this,SLOT(toggleCube()));
    //QObject::connect(teapot, SIGNAL(triggered()),this,SLOT(toggleTeapot()));
    QObject::connect(gourd, SIGNAL(triggered()),this,SLOT(toggleGourd()));
    QObject::connect(x, SIGNAL(triggered()),this,SLOT(changeX()));
    QObject::connect(y, SIGNAL(triggered()),this,SLOT(changeY()));
    QObject::connect(z, SIGNAL(triggered()),this,SLOT(changeZ()));
    QObject::connect(rotate_x, SIGNAL(triggered()),this,SLOT(rotateX()));
    QObject::connect(rotate_y, SIGNAL(triggered()),this,SLOT(rotateY()));
    QObject::connect(rotate_z, SIGNAL(triggered()),this,SLOT(rotateZ()));
    QObject::connect(plusButton, SIGNAL(clicked()),this,SLOT(plus()));
    QObject::connect(minusButton, SIGNAL(clicked()),this,SLOT(minus()));

    QObject::connect(openFileButton, SIGNAL(clicked()),
                      this, SLOT(openDirectory()));

}

void Viewfinder::paintCube(){
    overlay->paintCube();
}
void Viewfinder::paintGourd(){
    overlay->paintGourd();
}

void Viewfinder::changeX(){
    overlay->modify = 1;
}
void Viewfinder::changeY(){
    overlay->modify = 2;
}
void Viewfinder::changeZ(){
    overlay->modify = 3;
}
void Viewfinder::rotateX(){
    overlay->modify = 4;
}
void Viewfinder::rotateY(){
    overlay->modify = 5;
}
void Viewfinder::rotateZ(){
    overlay->modify = 6;
}
void Viewfinder::plus(){
    overlay->increment();
}
void Viewfinder::minus(){
    overlay->decrement();
}
void Viewfinder::toggleCube(){
    overlay->toggleCube();
}
void Viewfinder::toggleGourd(){
    overlay->toggleGourd();
}

void Viewfinder::openDirectory(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                     "/home/user",
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    appState->loadTemplateImageFeatures(dir);
}

Viewfinder::~Viewfinder()
{
}


void Viewfinder::processFrame() {
    if(!isProcessingFrames)
        return;

    ///////////////////
    // Do the real work
    ///////////////////
    time_t t0 = clock();

    appState->recEngineMutex.lock();
    bool runOK = appState->recEngine->surfTrack();
    appState->recEngineMutex.unlock();

    time_t t1 = clock();
    float dur = float(t1 - t0) / CLOCKS_PER_SEC;

    ///////////////////
    //Update overlay
    ///////////////////

    overlay->showDrawing = runOK;
    if(runOK)
    {
       //update drawing
        appState->updateDrawing();
    }
    overlay->repaint();
}

void Viewfinder::processFrames(int widgetIdx)
{

    isProcessingFrames = (widgetIdx == 0);

}


