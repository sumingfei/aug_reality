#include "OverlayWidget.h"
#include <QEvent>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

OverlayWidget::OverlayWidget(QWidget *par) : QWidget(par)  {
    /* Make QT do the work of keeping the overlay the magic color  */
    QWidget::setBackgroundRole(QPalette::Window); 
    QWidget::setAutoFillBackground(true); 
    QPalette overlayPalette = QWidget::palette(); 
    overlayPalette.setColor 
        (QPalette::Window,  
         colorKey()); 
    QWidget::setPalette(overlayPalette); 

    // Open the overlay device
    overlay_fd = open("/dev/fb1", O_RDWR);

    if (overlay_fd == -1) {
        perror("open");
    }

    // Get the current overlay and plane settings
    if (ioctl(overlay_fd, FBIOGET_VSCREENINFO, &overlay_info)) {
        perror("FBIO_VSCREENINFO");
    }
    if (ioctl(overlay_fd, OMAPFB_QUERY_PLANE, &plane_info)) {
        perror("OMAPFB_QUERY_PLANE");
    }

    // Disable the plane so we can allocate memory for it. 
    plane_info.enabled = 0;
    plane_info.pos_x = 0; 
    plane_info.pos_y = 0; 
    plane_info.out_width = 640;
    plane_info.out_height = 480;
    if (ioctl(overlay_fd, OMAPFB_SETUP_PLANE, &plane_info)) {
        perror("OMAPFB_SETUP_PLANE");
    }

    // Allocate the memory
    mem_info.size = 640*480*2;
    mem_info.type = 0;
    if (ioctl(overlay_fd, OMAPFB_SETUP_MEM, &mem_info)) {
        perror("OMAPFB_SETUP_MEM");
    }

    // mmap it into an FCam image
    void *ptr = mmap(NULL, mem_info.size, PROT_WRITE, MAP_SHARED, overlay_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
    }
    framebuffer_ = FCam::Image(640, 480, FCam::UYVY, (unsigned char *)ptr);

    // Clear the memory in case there was something hanging around from an earlier invocation
    memset(ptr, 128, 640*480*2);

    // Set the overlay properties
    overlay_info.xres = 640;
    overlay_info.yres = 480;
    overlay_info.xres_virtual = 640;
    overlay_info.yres_virtual = 480;
    overlay_info.xoffset = 0;
    overlay_info.yoffset = 0;
    overlay_info.nonstd = OMAPFB_COLOR_YUV422;    
    if (ioctl(overlay_fd, FBIOPUT_VSCREENINFO, &overlay_info)) {
        perror("FBIOPUT_VSCREENINFO");
    }

    // Set up the color key
    struct omapfb_color_key color_key;
    color_key.key_type = OMAPFB_COLOR_KEY_GFX_DST;
    QColor key = colorKey();
    color_key.trans_key = ((key.red() >> 3) << 11) | ((key.green() >> 2) << 5) | ((key.blue() >> 3));
    if (ioctl(overlay_fd, OMAPFB_SET_COLOR_KEY, &color_key)) {
        perror("OMAPFB_SET_COLOR_KEY");
    }

    filterInstalled = false;
}

bool OverlayWidget::eventFilter(QObject *, QEvent *event) {
    if (event->type() == QEvent::Move ||
        event->type() == QEvent::Resize ||
        event->type() == QEvent::Show) {
        enable();
    } else if (event->type() == QEvent::Hide) {
        disable();
    }

    // We don't capture this event, it should be propagated as normal
    return false;
}

void OverlayWidget::showEvent(QShowEvent *) {
    enable();
}

void OverlayWidget::hideEvent(QHideEvent *) {
    disable();
}

void OverlayWidget::resizeEvent(QResizeEvent *) {
    enable();
}

void OverlayWidget::moveEvent(QMoveEvent *) {
    enable();
}


OverlayWidget::~OverlayWidget() {
    disable();
    ::close(overlay_fd);
}

void OverlayWidget::enable() {
    // Shift the plane according to where the widget is, but keep it
    // at 640x480

    QPoint global = mapToGlobal(QPoint(0, 0));

    // round to even X
    global.setX(global.x()/2);
    global.setX(global.x()*2);

    int xoff = global.x() > 0 ? global.x() : 0;
    int yoff = global.y() > 0 ? global.y() : 0;
    int xcrop = global.x() < 0 ? -global.x() : 0;
    int ycrop = global.y() < 0 ? -global.y() : 0;

    if (xcrop > 640 || ycrop > 480) {
        disable();
        return;
    }

    // Set the size and position on screen
    plane_info.enabled = 1;
    plane_info.pos_x = xoff;
    plane_info.pos_y = yoff;
    plane_info.out_width = 640 - xcrop;
    plane_info.out_height = 480 - ycrop;

    if (ioctl(overlay_fd, OMAPFB_SETUP_PLANE, &plane_info)) {
        perror("OMAPFB_SETUP_PLANE");
    }

    // The image is always 640x480
    overlay_info.xres_virtual = 640;
    overlay_info.yres_virtual = 480;
    // Set the portion of it that's visible on screen
    overlay_info.xres = plane_info.out_width;
    overlay_info.yres = plane_info.out_height;
    overlay_info.xoffset = xcrop;
    overlay_info.yoffset = ycrop;
    overlay_info.nonstd = OMAPFB_COLOR_YUV422;    
    if (ioctl(overlay_fd, FBIOPUT_VSCREENINFO, &overlay_info)) {
        perror("FBIOPUT_VSCREENINFO");
    }

    if (!filterInstalled) {
        // if anything moves above me, we need to know about it to update the overlay
        for (QObject *obj = parent(); obj; obj = obj->parent()) {
            obj->installEventFilter(this);
        }
        filterInstalled = true;
    }
}

void OverlayWidget::disable() {
    plane_info.enabled = 0;
    if (ioctl(overlay_fd, OMAPFB_SETUP_PLANE, &plane_info)) {
        perror("OMAPFB_SETUP_PLANE");
    }
}

FCam::Image OverlayWidget::framebuffer() {
    return framebuffer_;
}
