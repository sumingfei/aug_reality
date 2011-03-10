#ifndef CAMERA_THREAD_H
#define CAMERA_THREAD_H

#include <QThread>
#include "ImageItem.h"
#include "CameraParameters.h"

#include <FCam/N900.h>
#include <QMetaType>

// Make it possible to pass FCam::Frames around in Qt signals
Q_DECLARE_METATYPE(FCam::Frame);
Q_DECLARE_METATYPE(FCam::Event);

class OverlayWidget;
class AppState;

/** This thread uses FCam to control the camera. It uses its public
 * parameters member to guide its behavior, and emits signals when a
 * new viewfinder or photograph comes in. */
class CameraThread : public QThread {
    Q_OBJECT;

  public:
    CameraThread(QObject *parent = NULL) : QThread(parent), autoFocus(&lens), overlay(NULL), appState(NULL){
        keepGoing = true;
        hdrViewfinder.resize(2);
        sensor.attach(&lens);
    }      

    // Which overlay should I be sending viewfinder frames to? 
    void setOverlay(OverlayWidget *o) {
        overlay = o;
    }

    // Which overlay should I be sending viewfinder frames to?
    void setAppState(AppState* _appState) {
        appState = _appState;
    }

    // The requested state of the camera. Fiddle with this object to
    // change the behavior of the camera 
    CameraParameters parameters;
    // Static utility function that tests whether the lens cover is currently shut.
    static bool lensCovered();

public slots:

    // Shut down the camera thread. 
    void stop() {
        keepGoing = false;
    }
    
    // Allow the camera thread to return. This _must_ be called after
    // handling a panic() signal.
    void failGracefully() {
        exitLock.unlock();
    }
    
    void pause() {
        sensor.stopStreaming();
    }
  signals:
     // A photograph was taken 
    void newImage(ImageItem *);
    
    // A capture sequence has completed
    void captureComplete(bool isBurst);

    // A new viewfinder frame arrived 
    void viewfinderFrame();

    // A new review frame arrived
    void newSnapshotFrame(FCam::Frame);

    // Various FCam events occurred 
    void focusPressed();
    void focusReleased();
    void shutterPressed();
    void shutterReleased();

    void lensCoverClosed();
    void lensCoverOpened();

    // An irrecoverable error has occured.
    // Connect something that will display an error message to the user.
    // After handling the error you _must_ call failGracefully()
    void panic(FCam::Event event);
    
  protected:
    // The thread that controls the camera
    void run();

  private:

    // An FCam sensor and lens
    FCam::N900::Sensor sensor;
    FCam::N900::Lens lens;
    
    // An autofocus helper object
    FCam::AutoFocus autoFocus;

    // The camera thread checks this flag once per iteration. If it's
    // false, it terminates.
    bool keepGoing;

    // An enum to help us distinguish the different types of frames that might come back
    enum {VIEWFINDER = 0, HDR_VIEWFINDER_LO, HDR_VIEWFINDER_HI, SINGLE, HDR, BURST, SHARPEST};

    // A shot to represent the viewfinder
    FCam::Shot viewfinder;

    // If we're in HDR mode, we use this pair of shots instead
    std::vector<FCam::Shot> hdrViewfinder;

    // A corresponding shot or burst to represent a full 5MP raw photo
    FCam::Shot photo;
    std::vector<FCam::Shot> hdrPhoto;

    // Our camera has a requested state stored in the CameraParameters
    // object. The actual state is reflected by the shots currently
    // streaming, and the lens object (which knows where the lens is
    // focused). This function modifies that actual state using the
    // requested state every time a viewfinder frame comes in.
    // 
    // Also, if some of the parameters are in automatic mode, we
    // expect the camera to update parts of the model like exposure
    // and gain every time a frame comes in. This function also takes
    // care of that.
    void updateState(const FCam::Frame &f);

    // Our own auto exposure algorithm 
    void meter(FCam::Shot *s, FCam::Frame f, float x, float y);
    
    // A lock that prevents the camera thread from exiting before
    // showing an error message. See exitGracefully() and panic()
    QMutex exitLock;

    // A pointer to the viewfinder that holds the overlay widget, so we can use its framebuffer
    // as a memory destination for viewfinding, and the recognition engine
    OverlayWidget* overlay;
    AppState* appState;
};



#endif
