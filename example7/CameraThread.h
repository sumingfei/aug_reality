#ifndef CAMERA_THREAD_H
#define CAMERA_THREAD_H

/** \file */

#include <QThread>

class OverlayWidget;
class InfoWidget;

class CameraThread : public QThread {
    Q_OBJECT;

  public:
    CameraThread(OverlayWidget *o, QObject *parent = NULL) : QThread(parent) {
        overlay = o;
        keepGoing = true;
    }
        
  public slots:
    void stop() {
        printf("Stopping!\n");
        keepGoing = false;
    }
    
  protected:
    void run();

  private:
    bool keepGoing;
    OverlayWidget *overlay;
};

#endif
