#include "OverlayWidget.h"
#include "gourd.h"

#include <QEvent>
#include <QPainter>
#include <QPen>
#include <QTimer>
#include <QtOpenGL>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define X_UYVY 0x59565955

OverlayWidget::OverlayWidget(AppState* appState, QWidget *par) : QGLWidget(QGLFormat() , par), appState(appState),
    frames(0),  filterInstalled(false), showDrawing(false), glIsInit(false), cubeExist(false), gourdExist(false),
    m_vertexBuffer(NULL), m_indexBuffer(NULL), m_normalBuffer(NULL), m_fAngle(0)
{
    cube_x = 0.0f; cube_y = 0.0f; cube_z = 0.0f;
    cube_rotate_x = 0;
    cube_rotate_y = 0;
    cube_rotate_z = 0;
    gourd_x = 0.0f; gourd_y = 0.0f; gourd_z = 0.0f;
    gourd_rotate_x = 0;
    gourd_rotate_y = 0;
    gourd_rotate_z = 0;
    modify = 1;

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

    // Record the original color key
    if (ioctl(overlay_fd, OMAPFB_GET_COLOR_KEY, &old_color_key)){
        perror("OMAPFB_GET_COLOR_KEY");
    }
    /*
    printf("old color key trans key = %x\n", old_color_key.trans_key);
    printf("old color key background = %x\n", old_color_key.background);
    printf("old color key key type = %x\n", old_color_key.key_type);
    printf("old color key channel out = %x\n", old_color_key.channel_out);
    */
    // Set up the color key
    struct omapfb_color_key color_key;
    color_key.key_type = OMAPFB_COLOR_KEY_GFX_DST;
    QColor key = colorKey();
    color_key.trans_key = ((key.red() >> 3) << 11) | ((key.green() >> 2) << 5) | ((key.blue() >> 3));
    if (ioctl(overlay_fd, OMAPFB_SET_COLOR_KEY, &color_key)) {
        perror("OMAPFB_SET_COLOR_KEY");
    }

    setAutoBufferSwap(false);
    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start();
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
    old_color_key.trans_key = 0x842;
    old_color_key.background = 0x0;
    old_color_key.key_type = OMAPFB_COLOR_KEY_GFX_DST;
    old_color_key.channel_out = OMAPFB_CHANNEL_OUT_LCD;
    
    if (ioctl(overlay_fd, OMAPFB_SET_COLOR_KEY, &old_color_key)) {
        perror("OMAPFB_SET_COLOR_KEY");
    }
    disable();
    ::close(overlay_fd);

    if(m_vertexBuffer)
        delete m_vertexBuffer;
    if(m_indexBuffer)
        delete m_indexBuffer;
    if(m_normalBuffer)
        delete m_normalBuffer;

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

    if (xcrop >= 640 || ycrop >= 480 || xoff >= 640 || yoff >= 480) {
        disable();
        return;
    }

    // Set the size and position on screen
    plane_info.enabled = 1;
    plane_info.pos_x = xoff;
    plane_info.pos_y = yoff;
    plane_info.out_width = 640 - xcrop - xoff;
    plane_info.out_height = 480 - ycrop - yoff;

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

void OverlayWidget::drawStats(QPainter &paint)
{
    QString framesPerSecond;
    framesPerSecond.setNum(frames /(time.elapsed() / 1000.0), 'f', 2);

    paint.setPen(Qt::white);

    paint.drawText(20, 40, framesPerSecond + " fps");

    if (!(frames % 100)) {
        time.start();
        frames = 0;
    }
    frames ++;
}



void OverlayWidget::paintEvent(QPaintEvent * event) {

    ////////////
    //2D drawing
    ////////////
    QPainter painter(this);
    if(showDrawing)
    {
        QPen penFrame(Qt::yellow, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(penFrame);
        for (int j = 0; j < 3; ++j) {
            painter.drawLine(appState->imageBoundary[j], appState->imageBoundary[j+1]);
        }
        painter.drawLine(appState->imageBoundary[3], appState->imageBoundary[0]);
    }

    drawStats(painter);

    painter.end();

    ////////////
    //3D drawing
    ////////////
    makeCurrent();

    if(!glIsInit)
    {
        initializeGL();
        glIsInit = true;
    }

    glFrontFace(GL_CW);
    glCullFace(GL_FRONT);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    if(cubeExist){
        paintCube();
    }
    if(gourdExist){
        paintGourd();
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    swapBuffers();
    //m_fAngle += 5.0;

}

void OverlayWidget::toggleCube(){
    cubeExist = !cubeExist;
    gourdExist = false;
}

void OverlayWidget::toggleGourd(){
    gourdExist = !gourdExist;
    cubeExist = false;
}

void OverlayWidget::increment(){
    switch(modify){
    case 1: if(cubeExist) cube_x+=0.1f; else gourd_x+=0.1f;
    case 2: if(cubeExist) cube_y+=0.1f; else gourd_y+=0.1f;
    case 3: if(cubeExist) cube_z+=0.1f; else gourd_z+=0.1f;
    case 4: if(cubeExist) cube_rotate_x+=2.0; else gourd_rotate_x+=2.0;
    case 5: if(cubeExist) cube_rotate_y+=2.0; else gourd_rotate_y+=2.0;
    case 6: if(cubeExist) cube_rotate_z+=2.0; else gourd_rotate_z+=2.0;
    }
}
void OverlayWidget::decrement(){
    switch(modify){
    case 1: if(cubeExist) cube_x-=0.1f; else gourd_x-=0.1f;
    case 2: if(cubeExist) cube_y-=0.1f; else gourd_y-=0.1f;
    case 3: if(cubeExist) cube_z-=0.1f; else gourd_z-=0.1f;
    case 4: if(cubeExist) cube_rotate_x-=2.0; else gourd_rotate_x-=2.0;
    case 5: if(cubeExist) cube_rotate_y-=2.0; else gourd_rotate_y-=2.0;
    case 6: if(cubeExist) cube_rotate_z-=2.0; else gourd_rotate_z-=2.0;
    }
}

void OverlayWidget::paintCube()
{
    QMatrix4x4 modelview;
    modelview.translate(cube_x, cube_y, cube_z);
    modelview.rotate(cube_rotate_x, 1.0f, 0.0f, 0.0f);
    modelview.rotate(cube_rotate_y, 0.0f, 1.0f, 0.0f);
    modelview.rotate(cube_rotate_z, 0.0f, 0.0f, 1.0f);
    modelview.scale(0.5f);

    program.bind();
    program.setUniformValue(matrixUniform, modelview);
    program.enableAttributeArray(normalAttr);
    program.enableAttributeArray(vertexAttr);
    program.setAttributeArray(vertexAttr, vertices.constData());
    program.setAttributeArray(normalAttr, normals.constData());
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    program.disableAttributeArray(normalAttr);
    program.disableAttributeArray(vertexAttr);

    program.release();

    // qDebug("draw %d vertices", vertices.size());

}

void OverlayWidget::paintGourd()
{
    QMatrix4x4 modelview;
    modelview.translate(gourd_x, gourd_y, gourd_z);
    //modelview.rotate(m_fAngle, 0.0f, 1.0f, 0.0f);
    modelview.rotate(gourd_rotate_x, 1.0f, 0.0f, 0.0f);
    modelview.rotate(gourd_rotate_y, 0.0f, 1.0f, 0.0f);
    modelview.rotate(gourd_rotate_z, 0.0f, 0.0f, 1.0f);
    modelview.scale(1.2f);

    program.bind();
    program.setUniformValue(matrixUniform, modelview);

    glGenBuffers(3, vboIds);
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numGourdVerts, GourdVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numGourdVerts, GourdVerts, GL_STATIC_DRAW); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * 3 * numGourdFaces, GourdFaces, GL_STATIC_DRAW);  
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glEnableVertexAttribArray(0);   
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[1]);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[2]);
    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, 0, 0);
    glBindAttribLocation(program.programId(), 0, "vertex");
    glBindAttribLocation(program.programId(), 1, "normal");

    glDrawElements(GL_TRIANGLES, 3 * numGourdFaces, GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    program.release();
    glDeleteBuffers(3, vboIds);

    // qDebug("drawing ...");

}

//This cannot be used; Makes your phone crashes, needs reboot
void OverlayWidget::paintGourdUsingQGLBuffer()
{
    QMatrix4x4 modelview;
    modelview.translate(-0.5f, 0.0f, 0.0f);
    modelview.rotate(m_fAngle, 0.0f, 1.0f, 0.0f);
    modelview.scale(0.75f);

    program.bind();
    program.setUniformValue(matrixUniform, modelview);

    m_vertexBuffer->bind();
    glEnableVertexAttribArray(0);

    m_normalBuffer->bind();
    glEnableVertexAttribArray(1);

    m_indexBuffer->bind();

    GLuint vId = m_vertexBuffer->bufferId();
    GLuint nId = m_normalBuffer->bufferId();

    glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, 0, 0);

    glBindAttribLocation(program.programId(), 0, "vertex");
    glBindAttribLocation(program.programId(), 1, "normal");

    glDrawElements(GL_TRIANGLES, 3 * numGourdFaces, GL_UNSIGNED_SHORT, 0);

    program.release();

    // qDebug("drawing ...");

}

void OverlayWidget::initializeGL()
{

    QGLShader *vshader = new QGLShader(QGLShader::Vertex, this);
    const char *vsrc =
            "attribute highp vec4 vertex;\n"
            "attribute mediump vec4 normal;\n"
            "uniform mediump mat4 matrix;\n"
            "varying mediump vec4 color;\n"
            "varying mediump vec3 normalv;\n"
            "void main(void)\n"
            "{\n"
            "    normalv = (matrix*normal).xyz;\n"
            "    vec3 toLight = normalize(vec3(0.0, 0.3, 1.0));\n"
            "    float angle = max(dot(normalv, toLight), 0.0);\n"
            "    vec3 col = vec3(0.40, 1.0, 0.0);\n"
            "    color = vec4(col * 0.2 + col * 0.8 * angle, 1.0);\n"
            "    color = clamp(color, 0.0, 1.0);\n"
            "    gl_Position = matrix * vertex;\n"
            "}\n";

    if(!vshader->compileSourceCode(vsrc))
        qDebug("could not compile vertex shader!");   
    if(!program.addShader(vshader))
        qDebug("could not add vertex shader!");


    QGLShader *fshader = new QGLShader(QGLShader::Fragment, this);
    const char *fsrc =
            "varying mediump vec4 color;\n"
            "varying mediump vec3 normalv;\n"
            "void main(void)\n"
            "{\n"
            "    mediump float edgeness = dot(vec3(0,0,1), normalize(normalv));\n"
            "    if(abs(edgeness) < 0.05)\n"
            "       gl_FragColor = vec4(0,0,0,1);\n"
            "    else \n"
            "       gl_FragColor = color;\n"
            "}\n";

    if(!fshader->compileSourceCode(fsrc))
        qDebug("could not compile fragment shader!");   
    if(!program.addShader(fshader))
        qDebug("could not add fragment shader!");

    if(!program.link())
        qDebug("could not link program!"); ;

    vertexAttr = program.attributeLocation("vertex");
    normalAttr = program.attributeLocation("normal");
    matrixUniform = program.uniformLocation("matrix");

    createCubeGeometry();
    setupViewport(width(), height());

    //Why can't we initialize this here?
    ///////////////////////////////////
    //    glGenBuffers(3, vboIds);

    //    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    //    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numGourdVerts, GourdVerts, GL_STATIC_DRAW);

    //    glBindBuffer(GL_ARRAY_BUFFER, vboIds[1]);
    //    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numGourdVerts, GourdVerts, GL_STATIC_DRAW);

    //    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[2]);
    //    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * 3 * numGourdFaces, GourdFaces, GL_STATIC_DRAW);

    ////////////////////////////////////

    qDebug("Done init GL");

}


void OverlayWidget::createCubeGeometry()
{
    vertices.clear();
    normals.clear();

    for(int i = 0; i < 108; i++)
    {
        vertices << QVector3D(afVertices[i], afVertices[i+1], afVertices[i+2]);
        i++;
        i++;
    }

    for(int i = 0; i < 108; i++)
    {

        normals << QVector3D(afNormals[i], afNormals[i+1], afNormals[i+2]);
        i++;
        i++;
    }

    // color array
    GLfloat colors0[] = {1,1,1,  1,1,0,  1,0,0,  1,0,1,              // v0-v1-v2-v3
                         1,1,1,  1,0,1,  0,0,1,  0,1,1,              // v0-v3-v4-v5
                         1,1,1,  0,1,1,  0,1,0,  1,1,0,              // v0-v5-v6-v1
                         1,1,0,  0,1,0,  0,0,0,  1,0,0,              // v1-v6-v7-v2
                         0,0,0,  0,0,1,  1,0,1,  1,0,0,              // v7-v4-v3-v2
                         0,0,1,  0,0,0,  0,1,0,  0,1,1};             // v4-v7-v6-v5

}

//This cannot be currently used
void OverlayWidget::createGoourdGeometry()
{
    if(m_vertexBuffer)
        delete m_vertexBuffer;
    if(m_indexBuffer)
        delete m_indexBuffer;
    if(m_normalBuffer)
        delete m_normalBuffer;

    //defining vertex buffer objects
    m_vertexBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
    m_vertexBuffer->create();
    m_vertexBuffer->bind();
    m_vertexBuffer->setUsagePattern(QGLBuffer::StaticDraw);
    m_vertexBuffer->allocate(GourdVerts, numGourdVerts * 3 * sizeof(GLfloat));

    m_normalBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
    m_normalBuffer->create();
    m_normalBuffer->bind();
    m_normalBuffer->setUsagePattern(QGLBuffer::StaticDraw);
    m_normalBuffer->allocate(GourdVertNorms, numGourdVerts * 3 * sizeof(GLfloat));

    m_indexBuffer = new QGLBuffer(QGLBuffer::IndexBuffer);
    m_indexBuffer->create();
    m_indexBuffer->bind();
    m_indexBuffer->setUsagePattern(QGLBuffer::StaticDraw);
    m_indexBuffer->allocate(GourdFaces, numGourdFaces * 3 * sizeof(unsigned short));


}

void OverlayWidget::mousePressEvent(QMouseEvent *event)
{

}

void OverlayWidget::mouseMoveEvent(QMouseEvent *event)
{

}

void OverlayWidget::resizeGL(int width, int height)
{
    setupViewport(width, height);
}

void OverlayWidget::setupViewport(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);  
}

