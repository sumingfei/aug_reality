#include <dlfcn.h>

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDir>
#include <QPainter>
#include <QMouseEvent>

#include <stdio.h>
#include <iostream>

#include "ThumbnailView.h"
#include "ImageItem.h"
#include "ScrollArea.h"
#include "UserDefaults.h"

#include "CameraThread.h"
extern CameraThread * cameraThread;


ZoomableThumbnail::ZoomableThumbnail(QWidget * parent) : QWidget(parent) {
    QObject::connect(&IOThread::reader(), SIGNAL(demosaicFinished()),
                     this, SLOT(update()));
    
    QVBoxLayout * layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addStretch(1);
    QWidget * zoomSliderBox = new QWidget();
    zoomSliderBox->setAutoFillBackground(TRUE);
    
    QHBoxLayout * sliderLayout = new QHBoxLayout();
    QLabel * leftLabel = new QLabel("Out");
    zoomSlider = new QSlider(Qt::Horizontal, this);
    QLabel * rightLabel = new QLabel("In");
    sliderLayout->addWidget(leftLabel);
    sliderLayout->addWidget(zoomSlider);
    sliderLayout->addWidget(rightLabel);
    zoomSliderBox->setLayout(sliderLayout);
    
    
    zoomSlider->setRange(0,1024);
    layout->addWidget(zoomSliderBox);
    this->setLayout(layout);
    QObject::connect(zoomSlider, SIGNAL(valueChanged(int)),
                     this, SLOT(update()));
    imageItem = NULL;
    ROI = QRect();
    ROICenter = QPoint();
}
void ZoomableThumbnail::mousePressEvent(QMouseEvent * event){
    lastMousePosition = event->pos();
    tracking = TRUE;
    event->accept();    
}
void ZoomableThumbnail::mouseReleaseEvent(QMouseEvent * event) {
    lastMousePosition = QPoint(-1,-1);
    tracking = FALSE;
    event->accept();
}

void ZoomableThumbnail::correctROI(){
    float alpha = zoomSlider->value()/1024.0f;
    int minWidth = 640;
    int minHeight = 480;
    int maxWidth = imageItem->fullResPixmap().width()-1; 
    int maxHeight = imageItem->fullResPixmap().height()-1;

    ROI.setWidth(minWidth * alpha + maxWidth * (1.0f - alpha));
    ROI.setHeight(minHeight *  alpha + maxHeight * (1.0f - alpha));
    ROI.moveCenter(ROICenter);

    ROI.moveLeft(qMax(0, ROI.x()));
    ROI.moveLeft(qMin(maxWidth - ROI.width(), ROI.x()));    
    ROI.moveTop(qMax(0, ROI.y()));
    ROI.moveTop(qMin(maxHeight - ROI.height(), ROI.y()));    

    ROICenter = ROI.center();
    
}

void ZoomableThumbnail::mouseMoveEvent(QMouseEvent * event) {
    QPoint delta = lastMousePosition - event->pos();    
    float scale = (float) ROI.width() / this->width();

    ROI.translate(delta * scale);
    ROICenter = ROI.center();
    /*
    int maxWidth = imageItem->fullResPixmap().width()-1; 
    int maxHeight = imageItem->fullResPixmap().height()-1;
    
    ROI.setX(qMax(0, ROI.x()));
    ROI.setX(qMin(maxWidth - ROI.width(), ROI.x()));    
    ROI.setY(qMax(0, ROI.y()));
    ROI.setY(qMin(maxHeight - ROI.height(), ROI.y()));    
    */
    lastMousePosition = event->pos();
    this->update();
    event->accept();
}

void ZoomableThumbnail::setImage(ImageItem * item) {
    imageItem = item;    
    if (imageItem && imageItem->frame().valid() && imageItem->frame().image().valid()) {
        ROI = QRect(0,0, imageItem->frame().image().width(), imageItem->frame().image().height()); 
    }
    zoomSlider->setSliderPosition(0);
}

void ZoomableThumbnail::setVisible(bool visible) {
    QWidget::setVisible(visible);
    if (visible)  {
        if (imageItem) imageItem->load();
        this->update();
    }
}

void ZoomableThumbnail::paintEvent(QPaintEvent * event) {
    QWidget::paintEvent(event);
    QPainter painter(this); 
    //printf("about to draw full res pixmap\n");
    
    //float alpha = zoomSlider->value()/1024.0f;
    this->correctROI();
    
    int maxWidth = imageItem->fullResPixmap().width()-1; 
    int maxHeight = imageItem->fullResPixmap().height()-1;

    
    
    
    painter.drawPixmap(QRect(0,0,this->width(), this->height()), imageItem->fullResPixmap(), ROI);
    //painter.setPen(QColor("green"));
    //painter.drawLine(0,0,this->width(), this->height());
    //printf("drew full res pixmap\n");
    
    painter.setPen(QColor("white"));    
    QRect mapRect(10,10,64,48);
    painter.eraseRect(mapRect);    
    painter.drawRect(mapRect);

    QRect mapROI((float)ROI.x() / maxWidth * mapRect.width() + mapRect.x(),
                 (float)ROI.y() / maxHeight * mapRect.height() + mapRect.y(),
                 (float)ROI.width() / maxWidth * mapRect.width() + 1,
                 (float)ROI.height() / maxHeight * mapRect.height() + 1);
    painter.fillRect(mapROI, QColor("white"));
    
}



ThumbnailView::ThumbnailView(QWidget *parent): QWidget(parent) {
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);

    scrollArea = new HScrollArea(this);
    scrollArea->setFixedSize(640, 480);
    
    zoomableThumbnail = new ZoomableThumbnail(this);
    zoomableThumbnail->setGeometry(0,0,640,480);
    zoomableThumbnail->hide();
    
    hLayout->addWidget(scrollArea);

    leftLabel = new QLabel("", this);
    middleLabel = new QLabel("", this);
    rightLabel = new QLabel("", this);

    scrollArea->addWidget(leftLabel);
    scrollArea->addWidget(middleLabel);
    scrollArea->addWidget(rightLabel);
    scrollArea->jumpTo(middleLabel);

    QObject::connect(scrollArea, SIGNAL(slidTo(int)),
                     this, SLOT(handleScrolled(int)));

    // Make the button box
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setSpacing(0);
    
    //QPushButton * quitButton = new QPushButton("X", this);
    //quitButton->setFlat(TRUE);
    //QObject::connect(quitButton, SIGNAL(clicked()),
    //                 cameraThread, SLOT(stop()));    
    //vLayout->addWidget(quitButton);
    
    vLayout->addStretch(1);
    
    zoomButton = new QPushButton("Zoom", this);
    zoomButton->setCheckable(TRUE);
    QObject::connect(zoomButton, SIGNAL(toggled(bool)),
                     zoomableThumbnail, SLOT(setVisible(bool)));
    vLayout->addWidget(zoomButton);
    deleteButton = new QPushButton("Trash", this);
    vLayout->addWidget(deleteButton);   
    QObject::connect(deleteButton, SIGNAL(clicked()),
                     this, SLOT(trashSelectedPhoto()));    
//    shareButton = new QPushButton("Share", this);
//    QObject::connect(shareButton, SIGNAL(clicked()),
//                     this, SLOT(launchShareDialog()));
//    vLayout->addWidget(shareButton);

    QPushButton *switchButton = new QPushButton("Schematic", this);
    QObject::connect(switchButton, SIGNAL(clicked()),
                     this, SLOT(switchToSchematicMaps()));
    vLayout->addWidget(switchButton);

    vLayout->addStretch(1);
    photoIndexLabel = new QLabel("0/0");
    vLayout->addWidget(photoIndexLabel);
    
    hLayout->addLayout(vLayout);

    this->setLayout(hLayout);

    QObject::connect(&IOThread::writer(), SIGNAL(saveFinished(ImageItem *)),
                     this, SLOT(updateThumbnails()));

    QObject::connect(&IOThread::reader(), SIGNAL(loadFinished(ImageItem *)),
                     this, SLOT(updateThumbnails()));

    // Make image items for all the existing dng files
    UserDefaults &userDefaults = UserDefaults::instance();
    QString directoryPath = userDefaults["rawPath"].asString().c_str();
    QDir dir(directoryPath);
    QStringList files = dir.entryList();
    for (int i = 0; i < files.size(); i++) {
        QString f = files.at(i);
        if (f.endsWith(".dng")) {
            images.push_back(new ImageItem(directoryPath + files.at(i)));
        }
    }

    if (images.size() == 0) {
        images.push_back(new ImageItem());
    }

    // The current viewed image is the most recent one
    current = images.size() - 1;

    // Prefetch the first two images in the background.
    for (int i = 1; i < 3; i++) {
        int idx = images.size() - i;
        while (idx < 0) idx += images.size();
        images[idx]->loadThumbnailAsync();
    }

    // Set up the state necessary to get the osso context created and destroyed
    libOssoHandle = dlopen("libosso.so.1", RTLD_LAZY);   
    // Extract the function pointers
    osso_initialize = (void *(*)(const char *, const char *, int, void *))
        dlsym(libOssoHandle, "osso_initialize");
    osso_deinitialize = (void (*)(void *))
        dlsym(libOssoHandle, "osso_deinitialize");
    // create osso context that is passed to sharing 
    ossoContext = osso_initialize("com.nokia.fcamera", "1.0.0.", 0, NULL);
    
    // Set up the state necessary get the Nokia sharing dialog to work
    // Open the shared library the function lives in 
    libSharingDialogHandle = dlopen("libsharingdialog.so.0", RTLD_LAZY);    
    // Extract the function
    sharing_dialog_with_file = (void (*)(void *, void *, const char *))
        dlsym(libSharingDialogHandle, "sharing_dialog_with_file");  
        
        
    this->updateThumbnails();
}




ThumbnailView::~ThumbnailView() {
    // Clean up the sharing dialog stuff
    osso_deinitialize(ossoContext);
    dlclose(libSharingDialogHandle);
    dlclose(libOssoHandle);
}


void ThumbnailView::newImage(ImageItem *img) {
    if (images.size() == 1 && images[0]->placeholder()) {
        images.erase(images.begin());
    }
    images.push_back(img);    
    current = images.size() - 1;
    img->saveAsync();
    updateThumbnails();
}

void ThumbnailView::updateThumbnails() {
    // show the appropriate pixmaps    
    size_t indices[3];
    for (int i = 0; i < 3; i++) {
        indices[i] = current + i + images.size() - 1;
        while (indices[i] >= images.size()) indices[i] -= images.size();
    }
    if (images.size() == 1 && images[0]->placeholder()) {
        photoIndexLabel->setText("Photo    0\n       of    0");
    } else {
        photoIndexLabel->setText(QString().sprintf("Photo % 4d\n       of % 4d", current+1, images.size()));
    }
    if (images[indices[0]]->thumbnail().valid()) {
        leftLabel->setPixmap(images[indices[0]]->pixmap());        
    } else {
        leftLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        leftLabel->setLineWidth(3);       
        if (images[indices[0]]->placeholder()) {
            leftLabel->setText("No photos found. Take some pictures!");
        } else if (images[indices[0]]->valid()) {
            leftLabel->setText("Loading ...");   
        } else {
            leftLabel->setText("Error loading image ...");   
        }
    }

    deleteButton->setEnabled(images[indices[1]]->safeToDelete());
    zoomButton->setEnabled(!images[indices[1]]->placeholder());
    //shareButton->setEnabled(!images[indices[1]]->placeholder());
    
    if (images[indices[1]]->thumbnail().valid()) {
        middleLabel->setPixmap(images[indices[1]]->pixmap());
        zoomableThumbnail->setImage(images[indices[1]]);
    } else {
        middleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        middleLabel->setLineWidth(3); 
        if (images[indices[1]]->placeholder()) {
            middleLabel->setText("No photos found. Take some pictures!");
        } else if (images[indices[1]]->valid()) {
            middleLabel->setText("Loading ...");   
        } else {
            middleLabel->setText("Error loading image ...");   
        }
    }

    if (images[indices[2]]->thumbnail().valid()) {
        rightLabel->setPixmap(images[indices[2]]->pixmap());   
    } else {
        rightLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        rightLabel->setLineWidth(3);        
        if (images[indices[2]]->placeholder()) {
            rightLabel->setText("No photos found. Take some pictures!");
        } else if (images[indices[2]]->valid()) {
            rightLabel->setText("Loading ...");   
        } else {
            rightLabel->setText("Error loading image ...");   
        }
    }

    manageMemory();
}

void ThumbnailView::launchShareDialog() {
    const char * filename = images[current]->tempJPEGPath().toStdString().c_str();    
    
    sharing_dialog_with_file(ossoContext, NULL, filename);
}

void ThumbnailView::handleScrolled(int idx) {
    switch (idx) {
    case 0: // scrolled left
        current--;
        if (current < 0) current = images.size()-1;
        break;
    case 1: // scrolled to middle
        return;
        break;
    case 2: // scrolled right
        current++;
        if (current == (int)images.size()) current = 0;
        break; 
    }

    scrollArea->jumpTo(middleLabel);
    updateThumbnails();
}

int min3(int a, int b, int c) {
    if (a < b && a < c) return a;
    if (b < c) return b;
    return c;
}

void ThumbnailView::manageMemory() {
    // printf("about to loadthumbasync(), item name is %s\n", images[current]->fullPath().toStdString().c_str());
    images[current]->loadThumbnailAsync();
    //printf("loadedthumbasync()\n");
    for (int i = 0; i < (int)images.size(); i++) {
        int delta = min3(abs(current - i),
                         abs(current - i + images.size()),
                         abs(current - i - images.size()));
        if (delta <= 3 && delta > 0) {
            if (images[i]->thumbnail().valid()) {
                //images[i]->discardFrame();
            } else {
                images[i]->loadThumbnailAsync();
            }
        } else if (delta > 3) {
            //images[i]->discardFrame();
            //images[i]->discardThumbnail();
        }
    }
}

void ThumbnailView::trashSelectedPhoto(){
    deleteButton->setEnabled(FALSE);
    ImageItem * selectedImage = images[current];
    images.erase(images.begin() + current);
    
    QDir fileDir("/");        
    
    int copyIndex = 0;
    // An extra bit of filename to make sure the name is unique

    QString fullPath = selectedImage->fullPath();
    QString base = selectedImage->fullPath().left(selectedImage->fullPath().count() - 4); // remove the ".dng"
    QString idealName = base + ".dng";

    // Nuke the imageitem. We don't just delete it, because it may in
    // be an iothread queue somewhere.
    selectedImage->discardThumbnail();
    selectedImage->discardFrame();

    int count = 0;
    while(! fileDir.rename(fullPath.toStdString().c_str(), 
           (idealName + ".trash").toStdString().c_str())) {
           idealName = base + QString().sprintf(".%d.dng", ++copyIndex);           
           count++;
           if (count == 100) {
               // ok, something's up with this file. Let's give up and
               // just hide it in the UI. It'll reappear next time we
               // load fcamera.
               std::cerr << "Failed to delete file: " << fullPath.toStdString() << std::endl;
               break;
           }
    }
    
    emit imageTrashed();    
    
    if (images.empty()) {
        images.push_back(new ImageItem());        
        //You can't zoom in on a placeholder image, so make sure we're not.
        zoomButton->setChecked(FALSE);
    }
    this->handleScrolled(0);
}


void ThumbnailView::addImageAtPath(QString path) {
    ImageItem * newItem = new ImageItem(path);
    this->newImage(newItem);
}

