#include <QApplication>
#include <QSignalMapper>

#include "CameraThread.h"
#include "PanicHandler.h"

#include "MainWindow.h"
#include "ScrollArea.h"
#include "Viewfinder.h"
#include "SnapshotView.h"

#include "RecognitionEngine.h"

#include "AppState.h"

#include <signal.h>

CameraThread *cameraThread;

// Handle Ctrl-C with a clean quit
void sigint(int) {
    cameraThread->stop();
}


int main(int argc, char *argv[])
{
     QApplication app(argc, argv);

     // We're going to be passing around Events using Qt Signals, so we
    // need to first register the type with Qt.
    qRegisterMetaType<FCam::Event>("FCam::Event");
    qRegisterMetaType<FCam::Frame>("FCam::Frame");

    // Make a thread that controls the camera and maintains its state
    cameraThread = new CameraThread();
    // The computer vision code is here:
    RecognitionEngine recEngine;

    ////////////////////////////////////
    //Setup widgets ////////////////////
    ////////////////////////////////////

    MainWindow mainWindow(cameraThread);
    VScrollArea scrollArea(&mainWindow);
    scrollArea.setGeometry(0, 0, 800, 480);

    AppState appState(&recEngine);
    Viewfinder viewfinder(&appState);

    // Tell camera thread about our overlay and recognition engine
    cameraThread->setOverlay(viewfinder.getOverlay());
    cameraThread->setAppState(&appState);

    //after taking a picture, review in this widget
    SnapshotView review(&appState);


    scrollArea.addWidget(&viewfinder);
    scrollArea.addWidget(&review);


    scrollArea.jumpTo(0);

    ////////////////////////////////////////
    //Connect objects with signals and slots
    ////////////////////////////////////////

    // When the shutter of focus button is pressed, the view should
    // slide to the viewfinder. The cameraThread doesn't know
    // about the scrollArea, so we send the signal via a signal mapper
    // which provides it.
    QSignalMapper mapper;
    mapper.setMapping(cameraThread, &viewfinder);
    QObject::connect(cameraThread, SIGNAL(focusPressed()),
                     &mapper, SLOT(map()));
    QObject::connect(&mapper, SIGNAL(mapped(QWidget *)),
                     &scrollArea, SLOT(jumpTo(QWidget *)));

    QSignalMapper mapper2;
    mapper2.setMapping(cameraThread, &review);
    QObject::connect(cameraThread, SIGNAL(shutterPressed()),
                     &mapper2, SLOT(map()));
    QObject::connect(&mapper2, SIGNAL(mapped(QWidget *)),
                     &scrollArea, SLOT(jumpTo(QWidget *)));


    // Hook up camera thread image signal to the review widget
    QObject::connect(cameraThread, SIGNAL(newSnapshotFrame(FCam::Frame)),
                     &review, SLOT(processSnapshotFrame(FCam::Frame)) );

    //Turn on or off the expensive computer vision stuff (on if in viewfinder mode, off else)
    QObject::connect(&scrollArea, SIGNAL(slidTo(int)),
                     &viewfinder, SLOT(processFrames(int)));


    //if the FCam driver if not installed, emit a panic message
    PanicHandler panicHandler;
    QObject::connect(cameraThread, SIGNAL(panic(FCam::Event)),
                     &panicHandler, SLOT(handleEvent(FCam::Event)));
    QObject::connect(&panicHandler, SIGNAL(eventHandled(FCam::Event)),
                     cameraThread, SLOT(failGracefully()));

    //When a new frame is available, send a signal
    QObject::connect(cameraThread, SIGNAL(viewfinderFrame()),
                     &viewfinder, SLOT(processFrame()));

    // Make Ctrl-C call app->exit(SIGINT)
    signal(SIGINT, sigint);

    // Connect the threads together in a daisy chain to get them all
    // to stop and then quit the app
    QObject::connect(&mainWindow, SIGNAL(closeApplication()),
                     cameraThread, SLOT(stop()));
    QObject::connect(cameraThread, SIGNAL(finished()),
                     &IOThread::writer(), SLOT(stop()));
    QObject::connect(&IOThread::writer(), SIGNAL(finished()),
                     &IOThread::reader(), SLOT(stop()));
    QObject::connect(&IOThread::reader(), SIGNAL(finished()),
                     &app, SLOT(quit()));

    ///////////////////////////////////
    // Launch the application ////////
    ///////////////////////////////////

    // Launch the camera control thread
    cameraThread->start();

    mainWindow.show();

    //Start main event loop
    int rval = app.exec();

    //Quiting ...
    printf("About to delete camera thread\n");
    delete cameraThread;    
    printf("Camera thread deleted\n");

    return rval;
}
