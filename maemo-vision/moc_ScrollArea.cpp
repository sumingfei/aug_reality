/****************************************************************************
** Meta object code from reading C++ file 'ScrollArea.h'
**
** Created: Thu Mar 10 14:09:29 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ScrollArea.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScrollArea.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ScrollArea[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x05,
      24,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      41,   11,   11,   11, 0x0a,
      53,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ScrollArea[] = {
    "ScrollArea\0\0slidTo(int)\0slidTo(QWidget*)\0"
    "jumpTo(int)\0jumpTo(QWidget*)\0"
};

const QMetaObject ScrollArea::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ScrollArea,
      qt_meta_data_ScrollArea, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ScrollArea::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ScrollArea::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ScrollArea::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ScrollArea))
        return static_cast<void*>(const_cast< ScrollArea*>(this));
    return QWidget::qt_metacast(_clname);
}

int ScrollArea::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: slidTo((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: slidTo((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 2: jumpTo((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: jumpTo((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void ScrollArea::slidTo(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ScrollArea::slidTo(QWidget * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
