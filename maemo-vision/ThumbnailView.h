#ifndef FCAMERA_THUMBNAIL_VIEW
#define FCAMERA_THUMBNAIL_VIEW

#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include "ImageItem.h"
#include <QPushButton>


class HScrollArea;

/** A widget that provides a zoomable view of a captured image. One of
 * these is placed on top of the Thumbnail view when the user hits the
 * zoom button. */
class ZoomableThumbnail : public QWidget {
    Q_OBJECT
    
public:
    ZoomableThumbnail(QWidget * parent = 0);
    // Set the image item and reinitialize the region of interest.
    void setImage(ImageItem * item);
    // Override the paint event to draw the zoomed portion of the 
    // image item. Also draw a small "map" showing where in the image
    // the user is viewing.
    void paintEvent(QPaintEvent * event);    
    // Handle mouse events for panning within a zoomed image.
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    // If the region of interest is invalid (lies outside the bounds of the
    // image), correct it by translating to the closest valid ROI. Also
    // set the ROI size according to the zoom slider's value.
    void correctROI();    
    // Override QWidget::setVisible to load the image item when visible
    // is true in addition to normal duties.
    void setVisible(bool visible);
protected:
    // Some state to help keep track of mouse position for panning.
    QPoint lastMousePosition;
    bool tracking;
    // The slider that controls the size of the ROI
    QSlider * zoomSlider;
    // The image item currently being viewed.
    ImageItem * imageItem;
    // The region of interest and it's ideal center (used to make zooming anchor at
    // the ROI center rather than the top left corner). 
    QPoint ROICenter;
    QRect ROI;
};


/** The thumbnail review page. It accepts new ImageItems from the
 * CameraThread and deals with saving them, reviewing, sharing,
 * etc. It also populates itself using .dng files found on disk so you
 * can see photos taken other times you ran fcmera. */
class ThumbnailView: public QWidget {
    Q_OBJECT

public:
    ThumbnailView(QWidget *parent = 0);
    ~ThumbnailView();

public slots:
    // Add a new image to the viewer.
    void newImage(ImageItem *);
    // Generally update the state of the viewer such that the correct buttons
    // are enabled, the thumbnails are loaded and in their proper places within
    // the scroll view, etc.
    void updateThumbnails();
    // Correct the currently selected index to reflect a recent scroll action.
    void handleScrolled(int);
    // Launch the Maemo file sharing dialog for the currently selected image;
    // Allows user to email photos, upload to flickr, etc.
    void launchShareDialog();
    // Removes the currently selected photo's .dng from the RAW library.
    void trashSelectedPhoto();
    // Add an image to the thumbnail view at this path. Useful
    // for restoring trashed images back into the scroll view.
    void addImageAtPath(QString path);  

    void switchToSchematicMaps()
    {
        emit switchToSchematicMapsSignal(1);
    }
    
signals:
    // An image has been moved to the trash. This is usually connected 
    // to the extended settings widgets that manage the trash (so they can
    // be enabled or disabled when the trash gets modified.)
    void imageTrashed();
    void switchToSchematicMapsSignal(int);

private:
    // figure out which images to load, which images to drop from memory, etc
    void manageMemory();

    // Some buttons down the right to do things like upload or delete photos.
    // The pushbutton that triggers file deletion.
    QPushButton * deleteButton;
    // The pushbutton that toggles the visibility of the zoomable thumbnail.
    QPushButton * zoomButton;
    // The pushbutton that launches the sharing dialog.
    // QPushButton * shareButton;
    // The label that shows the current photo index (e.g. "7/9")
    QLabel * photoIndexLabel;    
    
    // The scrollable review area
    HScrollArea *scrollArea;

    // Three labels contained within it
    // Most of the time these labels are displaying thumbnail pixmaps. 
    QLabel *leftLabel, *middleLabel, *rightLabel;
    
    // A an array of all the images the thumbnail view is aware
    // of. Some of them are on-disk, some of them are only in memory.
    std::vector<ImageItem *> images;
    
    // The index of the currently visible photo
    int current;
    
    // The thumbnail widget that handles panning and zooming on the currently
    // selected image.
    ZoomableThumbnail * zoomableThumbnail;
    
    // Stuff necessary to get the Nokia sharing widget to run
    void *ossoContext;
    // A handle on libsharingdialog.so and libosso.so
    void *libSharingDialogHandle;
    void *libOssoHandle;
    // A pointer to the actual functions we call
    void (*sharing_dialog_with_file) (void *, void *, const char *);    
    void * (*osso_initialize)(const char *, const char *, int, void *);
    void (*osso_deinitialize)(void *);
};

#endif
