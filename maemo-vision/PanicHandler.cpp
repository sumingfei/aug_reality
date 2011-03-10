#include "PanicHandler.h"
#include <QMessageBox>

PanicHandler::PanicHandler(QObject * parent) : QObject(parent) {

}

void PanicHandler::handleEvent(FCam::Event e) {
    if (e.type == FCam::Event::Error && (e.data == FCam::Event::DriverLockedError || e.data == FCam::Event::DriverMissingError)) {
            QMessageBox * message = new QMessageBox();
            if (e.data == FCam::Event::DriverLockedError) {
                message->setText("Cannot connect to sensor");
                message->setInformativeText("Please exit any other camera applications and then relaunch FCamera.");
            } else {
                message->setText("Cannot find FCam drivers");
                message->setInformativeText("If you just installed FCamera or the FCam drivers, you will need to "
                                            "reboot your N900 before running any FCam applications. Please reboot "
                                            "your N900 now. If problems persist after reboot try installing FCam again "
                                            "or ask for help at fcam.garage.maemo.org."
                                         );            
            }            
            message->addButton("Exit FCamera", QMessageBox::AcceptRole);
            message->exec();
    }
    emit eventHandled(e);
}    

