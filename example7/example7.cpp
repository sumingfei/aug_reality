/** \file */

#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>

#include "OverlayWidget.h"

#include "CameraThread.h"

/***********************************************************/
/* Full camera application                                 */
/*                                                         */
/* This example uses QT to create a full camera            */
/* application for the N900. It displays viewfinder frames */
/* using an fbdev overlay.                                 */
/***********************************************************/

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    // Make the main window and a layout for it
    QWidget *window = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    // Put a slider on the left
    QSlider *slider = new QSlider(Qt::Vertical);
    layout->addWidget(slider);

    // Make an overlay for displaying viewfinder frames
    OverlayWidget *overlay = new OverlayWidget(window);
    overlay->setFixedSize(640, 480);
    layout->addWidget(overlay);

    // Make some buttons down the right
    QVBoxLayout *buttonLayout = new QVBoxLayout;
    layout->addLayout(buttonLayout);
    QPushButton *quitButton = new QPushButton("X");
    QPushButton *button1 = new QPushButton("1");
    QPushButton *button2 = new QPushButton("2");
    QPushButton *button3 = new QPushButton("3");
    quitButton->setFixedSize(80, 64);
    button1->setFixedSize(80, 64);
    button2->setFixedSize(80, 64);
    button3->setFixedSize(80, 64);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(button1);
    buttonLayout->addWidget(button2);
    buttonLayout->addWidget(button3);

    window->setLayout(layout);

    // Make a thread that controls the camera
    CameraThread cameraThread(overlay);

    // Hook up the quit button to stop the camera thread
    QObject::connect(quitButton, SIGNAL(clicked()), 
                     &cameraThread, SLOT(stop()));

    // Once the camera thread stops, quit the app
    QObject::connect(&cameraThread, SIGNAL(finished()), 
                     &app, SLOT(quit()));

    // Show the app full screen
    window->showFullScreen();

    // Launch the camera thread
    cameraThread.start();

    // Enter the QT main event loop
    return app.exec();
}
