/****************************************************************************
** Meta object code from reading C++ file 'OverlayWidget.h'
**
** Created: Thu Mar 10 15:05:49 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "OverlayWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'OverlayWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_OverlayWidget[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x0a,
      28,   14,   14,   14, 0x0a,
      42,   14,   14,   14, 0x0a,
      54,   14,   14,   14, 0x0a,
      67,   14,   14,   14, 0x0a,
      79,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_OverlayWidget[] = {
    "OverlayWidget\0\0toggleCube()\0toggleGourd()\0"
    "paintCube()\0paintGourd()\0increment()\0"
    "decrement()\0"
};

const QMetaObject OverlayWidget::staticMetaObject = {
    { &QGLWidget::staticMetaObject, qt_meta_stringdata_OverlayWidget,
      qt_meta_data_OverlayWidget, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &OverlayWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *OverlayWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *OverlayWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OverlayWidget))
        return static_cast<void*>(const_cast< OverlayWidget*>(this));
    return QGLWidget::qt_metacast(_clname);
}

int OverlayWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: toggleCube(); break;
        case 1: toggleGourd(); break;
        case 2: paintCube(); break;
        case 3: paintGourd(); break;
        case 4: increment(); break;
        case 5: decrement(); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
