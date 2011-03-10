#ifndef PANIC_HANDLER_H
#define PANIC_HANDLER_H

#include <QObject>
#include <QMetaType>
#include <FCam/N900.h>


class PanicHandler : public QObject {
    Q_OBJECT
public:
    PanicHandler(QObject * parent = 0);
public slots:
    void handleEvent(FCam::Event e);
signals:
    void eventHandled(FCam::Event e);
};

#endif

