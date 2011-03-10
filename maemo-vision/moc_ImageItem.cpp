/****************************************************************************
** Meta object code from reading C++ file 'ImageItem.h'
**
** Created: Thu Mar 10 14:09:25 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ImageItem.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ImageItem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IOThread[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,
      35,    9,    9,    9, 0x05,
      60,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      79,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_IOThread[] = {
    "IOThread\0\0loadFinished(ImageItem*)\0"
    "saveFinished(ImageItem*)\0demosaicFinished()\0"
    "stop()\0"
};

const QMetaObject IOThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_IOThread,
      qt_meta_data_IOThread, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IOThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IOThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IOThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IOThread))
        return static_cast<void*>(const_cast< IOThread*>(this));
    return QThread::qt_metacast(_clname);
}

int IOThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: loadFinished((*reinterpret_cast< ImageItem*(*)>(_a[1]))); break;
        case 1: saveFinished((*reinterpret_cast< ImageItem*(*)>(_a[1]))); break;
        case 2: demosaicFinished(); break;
        case 3: stop(); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void IOThread::loadFinished(ImageItem * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void IOThread::saveFinished(ImageItem * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void IOThread::demosaicFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
QT_END_MOC_NAMESPACE
