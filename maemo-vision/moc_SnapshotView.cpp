/****************************************************************************
** Meta object code from reading C++ file 'SnapshotView.h'
**
** Created: Thu Mar 10 14:09:35 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "SnapshotView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SnapshotView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SnapshotView[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SnapshotView[] = {
    "SnapshotView\0\0processSnapshotFrame(FCam::Frame)\0"
};

const QMetaObject SnapshotView::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SnapshotView,
      qt_meta_data_SnapshotView, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SnapshotView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SnapshotView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SnapshotView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SnapshotView))
        return static_cast<void*>(const_cast< SnapshotView*>(this));
    return QWidget::qt_metacast(_clname);
}

int SnapshotView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: processSnapshotFrame((*reinterpret_cast< FCam::Frame(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
