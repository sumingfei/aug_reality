#include "AppState.h"

#include <QFileInfo>
#include <QImage>
#include <QTextStream>
#include <QTimer>
#include <QtDebug>
#include <QVariant>
#include <QStringList>
#include <QDir>

#include "RecognitionEngine.h"


AppState::AppState( RecognitionEngine* _recEngine):
        recEngine(_recEngine)
{
    for(int i = 0; i < 4; i++)
        imageBoundary.append(QPointF());
}


void AppState::loadTemplateImageFeatures(QString& dbDirName)
{
    this->recEngine->reset();
    QDir dir(dbDirName, "*.xml");
    QFileInfoList fileList = dir.entryInfoList();
    std::vector<std::string> dbFileNames;
    for (int i = 0; i < fileList.size(); ++i) {
        QFileInfo fileInfo = fileList.at(i);
        std::string _name = fileInfo.canonicalFilePath().toStdString();
        dbFileNames.push_back(_name);
    }
    this->recEngine->buildDatabase(dbFileNames);

}

int Sign(float X){ return(X<0 ? -1 : 1); }

bool IsOutlineConvex(const std::vector<CvPoint2D32f> &O)
{
    if(O.size()<3) return false;

    int XCh=0,YCh=0;

    CvPoint2D32f A= cvPoint2D32f(O[0].x- O[1].x, O[0].y- O[1].y);
    for(size_t I=1;I<O.size()-1;++I)
    {
        CvPoint2D32f B= cvPoint2D32f(O[I].x- O[I+1].x, O[I].y- O[I + 1].y);
        if(Sign(A.x)!=Sign(B.x)) ++XCh;
        if(Sign(A.y)!=Sign(B.y)) ++YCh;

        A=B;
    }

    //check also last edge

    CvPoint2D32f B= cvPoint2D32f(O[O.size() - 1].x- O[0].x, O[O.size() - 1].y- O[0].y);
    if(Sign(A.x)!=Sign(B.x)) ++XCh;
    if(Sign(A.y)!=Sign(B.y)) ++YCh;

    return(XCh<=2 && YCh<=2);
}

bool AppState::updateDrawing()
{

    std::vector<CvPoint2D32f> rectPoints;
    int _h = recEngine->templateFeatures[recEngine->matchedTemplate].height;
    int _w = recEngine->templateFeatures[recEngine->matchedTemplate].width;
    std::vector<CvPoint2D32f> _srcPoints;
    _srcPoints.push_back(cvPoint2D32f(0,0));
    _srcPoints.push_back(cvPoint2D32f(0,_h));
    _srcPoints.push_back(cvPoint2D32f(_w,_h));
    _srcPoints.push_back(cvPoint2D32f(_w,0));
    for(int i = 0; i < 4; i++)
    {
        CvPoint2D32f dst;
        recEngine->transform(recEngine->homography, _srcPoints[i], dst);
        rectPoints.push_back(dst);
    }
    //check if convex polygon
    if(!IsOutlineConvex(rectPoints)) {
        return false;
    }
    for(unsigned int i = 0; i < rectPoints.size(); i++)
    {
        imageBoundary[i].setX(imgRatio * rectPoints[i].x);
        imageBoundary[i].setY(imgRatio * rectPoints[i].y);
    }
}
