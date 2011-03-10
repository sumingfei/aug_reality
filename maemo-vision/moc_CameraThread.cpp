/****************************************************************************
** Meta object code from reading C++ file 'CameraThread.h'
**
** Created: Thu Mar 10 14:09:19 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "CameraThread.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CameraThread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CameraThread[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      11,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x05,
      43,   35,   13,   13, 0x05,
      65,   13,   13,   13, 0x05,
      83,   13,   13,   13, 0x05,
     113,   13,   13,   13, 0x05,
     128,   13,   13,   13, 0x05,
     144,   13,   13,   13, 0x05,
     161,   13,   13,   13, 0x05,
     179,   13,   13,   13, 0x05,
     197,   13,   13,   13, 0x05,
     221,  215,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
     240,   13,   13,   13, 0x0a,
     247,   13,   13,   13, 0x0a,
     264,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CameraThread[] = {
    "CameraThread\0\0newImage(ImageItem*)\0"
    "isBurst\0captureComplete(bool)\0"
    "viewfinderFrame()\0newSnapshotFrame(FCam::Frame)\0"
    "focusPressed()\0focusReleased()\0"
    "shutterPressed()\0shutterReleased()\0"
    "lensCoverClosed()\0lensCoverOpened()\0"
    "event\0panic(FCam::Event)\0stop()\0"
    "failGracefully()\0pause()\0"
};

const QMetaObject CameraThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_CameraThread,
      qt_meta_data_CameraThread, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CameraThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CameraThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CameraThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CameraThread))
        return static_cast<void*>(const_cast< CameraThread*>(this));
    return QThread::qt_metacast(_clname);
}

int CameraThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: newImage((*reinterpret_cast< ImageItem*(*)>(_a[1]))); break;
        case 1: captureComplete((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: viewfinderFrame(); break;
        case 3: newSnapshotFrame((*reinterpret_cast< FCam::Frame(*)>(_a[1]))); break;
        case 4: focusPressed(); break;
        case 5: focusReleased(); break;
        case 6: shutterPressed(); break;
        case 7: shutterReleased(); break;
        case 8: lensCoverClosed(); break;
        case 9: lensCoverOpened(); break;
        case 10: panic((*reinterpret_cast< FCam::Event(*)>(_a[1]))); break;
        case 11: stop(); break;
        case 12: failGracefully(); break;
        case 13: pause(); break;
        default: ;
        }
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void CameraThread::newImage(ImageItem * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CameraThread::captureComplete(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CameraThread::viewfinderFrame()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void CameraThread::newSnapshotFrame(FCam::Frame _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void CameraThread::focusPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void CameraThread::focusReleased()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}

// SIGNAL 6
void CameraThread::shutterPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 6, 0);
}

// SIGNAL 7
void CameraThread::shutterReleased()
{
    QMetaObject::activate(this, &staticMetaObject, 7, 0);
}

// SIGNAL 8
void CameraThread::lensCoverClosed()
{
    QMetaObject::activate(this, &staticMetaObject, 8, 0);
}

// SIGNAL 9
void CameraThread::lensCoverOpened()
{
    QMetaObject::activate(this, &staticMetaObject, 9, 0);
}

// SIGNAL 10
void CameraThread::panic(FCam::Event _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}
QT_END_MOC_NAMESPACE
