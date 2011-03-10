#include "SnapshotView.h"

#include "CameraThread.h"
#include "RecognitionEngine.h"
#include "AppState.h"

#include <QtGui>

#include "CameraParameters.h"
#include "UserDefaults.h"
#include "FCam/Image.h"

#include<vector>
#define TREE_WIDTH 11

SnapshotView::SnapshotView(AppState* appState, QWidget *parent):
        QWidget(parent), appState(appState){

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Make an overlay for displaying viewfinder frames
    imageWidget = new ImageViewer(appState);
    imageWidget->setFixedSize(640, 480);
    layout->addWidget(imageWidget);

    // Make space for the parameter button tree drawing
    layout->addSpacing(TREE_WIDTH);

    // Make some buttons down the right
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(0);
    layout->addLayout(buttonLayout);

    QPushButton *exitButton = new QPushButton("Exit", this);
    buttonLayout->addWidget(exitButton);
    this->setLayout(layout);
}


void SnapshotView::processSnapshotFrame(FCam::Frame frame)
{
    ///////////////////
    // Do the real work
    ///////////////////
    time_t t0 = clock();

    appState->recEngineMutex.lock();
    bool runOK = appState->recEngine->surfRecognize();

    demosaic(frame);

    appState->recEngineMutex.unlock();

    time_t t1 = clock();
    float dur = float(t1 - t0) / CLOCKS_PER_SEC;

    ///////////////////
    //Update overlay
    ///////////////////
     imageWidget->setShowDrawing(runOK);

    if(runOK)
    {
        //update drawing
        appState->updateDrawing();
    }
    imageWidget->repaint();

}


bool SnapshotView::demosaic(FCam::Frame frame) {
    if (!frame.valid() || !frame.image().valid())
        return false;
    QImage demosaicQImage((const unsigned char*) appState->recEngine->large_iplGray->imageData, 640, 480, QImage::Format_Indexed8);
    QImage copy_image = demosaicQImage.convertToFormat(QImage::Format_RGB888);
    imageWidget->setImage(copy_image);
    return true;
}

void ImageViewer::paintEvent(QPaintEvent * event) {
    QPainter painter(this);
    painter.drawImage(QPoint(0,0),m_image);

    //draw frame
    if(this->showDrawing)
    {
        QPen penFrame(Qt::yellow, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(penFrame);
        for (int j = 0; j < 3; ++j) {
              painter.drawLine(appState->imageBoundary[j], appState->imageBoundary[j+1]);
          }
        painter.drawLine(appState->imageBoundary[3], appState->imageBoundary[0]);
    }
    painter.end();

}

