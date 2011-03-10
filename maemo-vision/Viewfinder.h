#ifndef VIEWFINDER_H
#define VIEWFINDER_H

#include <QPushButton>
#include <QList>

#include <FCam/Frame.h>

class OverlayWidget;
class AppState;
class QMutex;

class Viewfinder: public QWidget {
    Q_OBJECT

public:
    Viewfinder(AppState* AppState, QWidget *parent = 0);
    ~Viewfinder();


    OverlayWidget* getOverlay()
    {
        return overlay;
    }

    AppState* appState;


public slots:
    void processFrames(int);
    void processFrame();
    void toggleCube();
    void toggleGourd();
    void paintCube();
    void paintGourd();
    void changeX();
    void changeY();
    void changeZ();
    void rotateX();
    void rotateY();
    void rotateZ();
    void plus();
    void minus();
private slots:
    void openDirectory();

private:
    OverlayWidget *overlay;  // The actual viewfinder on the left
    bool isProcessingFrames;
    const static int imgRatio = 2; //processed images are half size in each direction

};




#endif
