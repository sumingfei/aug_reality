#ifndef OVERLAY_WIDGET_H
#define OVERLAY_WIDGET_H

#include <QX11Info>
#include <QGLWidget>
#include <QtOpenGL/qglshaderprogram.h>
#include <QGLBuffer>
#include <QTime>

#define __user
#include "linux/omapfb.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <FCam/Image.h>
#include "AppState.h"

/** This widget manages an fbdev YUV overlay, suitable for drawing
 * viewfinder frames on. */
class OverlayWidget : public QGLWidget {

    Q_OBJECT

public:
    OverlayWidget(AppState* AppState, QWidget *parent = NULL);
    ~OverlayWidget();

    // If you draw on a widget at the same place as this one, using
    // any color but the one below, it will show through the overlay.
    static QColor colorKey() {return QColor(10, 0, 10);}

    // A reference to the frame buffer. Modifying this image will
    // update what's visible on screen immediately. (i.e., there's no
    // double-buffering).
    FCam::Image framebuffer();
    bool cubeExist;
    bool gourdExist;
    float cube_x;
    float cube_y;
    float cube_z;
    qreal cube_rotate_x;
    qreal cube_rotate_y;
    qreal cube_rotate_z;
    float gourd_x;
    float gourd_y;
    float gourd_z;
    qreal gourd_rotate_x;
    qreal gourd_rotate_y;
    qreal gourd_rotate_z;
    int modify;
    bool showDrawing;
public slots:
    void toggleCube();
    void toggleGourd();
    void paintCube();
    void paintGourd();
    void increment();
    void decrement();
protected:
    void initializeGL();
    void paintEvent(QPaintEvent *event);
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void showEvent(QShowEvent *event);

protected:
    struct omapfb_color_key old_color_key;
    
    void resizeEvent(QResizeEvent *);
    void moveEvent(QMoveEvent *);
    //void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    bool eventFilter(QObject *receiver, QEvent *event);

    //virtual void paintEvent(QPaintEvent * event);
    void drawStats(QPainter &paint);

    void enable();    
    void disable();

    FCam::Image framebuffer_;

    //struct fb_var_screeninfo var_info;
    struct fb_var_screeninfo overlay_info;
    struct omapfb_mem_info mem_info;
    struct omapfb_plane_info plane_info;
    int overlay_fd;

    bool filterInstalled;

private:

    const AppState* appState;

    qreal   m_fAngle;
    qreal   m_fScale;

    QGLShaderProgram program;
    int vertexAttr;
    int normalAttr;
    int matrixUniform;

    bool glIsInit;

    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;

    QGLBuffer* m_vertexBuffer;
    QGLBuffer* m_indexBuffer;
    QGLBuffer* m_normalBuffer;

    GLuint vboIds[3];

    QTime time;
    int frames;

    void setupViewport(int width, int height);
    void createCubeGeometry();
    void createGoourdGeometry();
    void paintGourdUsingQGLBuffer();
};

#endif
