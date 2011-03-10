#ifndef SnapshotView_H
#define SnapshotView_H

#include <QPushButton>
#include <QList>
#include <QImage>
#include <FCam/Frame.h>

class OverlayWidget;
class AppState;
class QMutex;

class ImageViewer : public QWidget {


public:
    ImageViewer(AppState* appState, QWidget *parent = 0): appState(appState), showDrawing(false)
    {}
    ~ImageViewer()
    {}

    void setImage(QImage img)
    {
        m_image = img;
    }

    void setShowDrawing(bool flag)
    {
        showDrawing = flag;
    }

protected:
    void paintEvent(QPaintEvent * event);

private:
    QImage m_image;
    bool showDrawing;
    const AppState* appState;

};


class SnapshotView : public QWidget {

    Q_OBJECT

public:
    SnapshotView(AppState* appState, QWidget *parent = 0);
    ~SnapshotView(){}

    void loadTemplateImageFeatures(const std::string& archiveFileName);

    ImageViewer* getImageWidget()
    {
        return imageWidget;
    }

public slots:

    void processSnapshotFrame(FCam::Frame);


private:

    AppState* appState;
    ImageViewer* imageWidget;

    bool demosaic(FCam::Frame frame);

    float iFrameRate;

    float homography[9];


};

#endif // SnapshotView_H
