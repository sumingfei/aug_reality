#ifndef APPSTATE_H
#define APPSTATE_H

#include <string>
#include <QList>
#include <QPointF>
#include <QMutex>
#include <QObject>

class RecognitionEngine;

class AppState{

public:
    AppState(RecognitionEngine* _recEngine);

     void loadTemplateImageFeatures(QString& dbDirName);
     bool updateDrawing();

     RecognitionEngine* recEngine;
     QMutex recEngineMutex;

     QList<QPointF> imageBoundary;

private:

     const static int imgRatio = 2; //processed images are half size in each direction


};

#endif // APPSTATE_H
