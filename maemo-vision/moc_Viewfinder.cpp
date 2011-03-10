/****************************************************************************
** Meta object code from reading C++ file 'Viewfinder.h'
**
** Created: Thu Mar 10 14:47:14 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "Viewfinder.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Viewfinder.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Viewfinder[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      31,   11,   11,   11, 0x0a,
      46,   11,   11,   11, 0x0a,
      59,   11,   11,   11, 0x0a,
      73,   11,   11,   11, 0x0a,
      85,   11,   11,   11, 0x0a,
      98,   11,   11,   11, 0x0a,
     108,   11,   11,   11, 0x0a,
     118,   11,   11,   11, 0x0a,
     128,   11,   11,   11, 0x0a,
     138,   11,   11,   11, 0x0a,
     148,   11,   11,   11, 0x0a,
     158,   11,   11,   11, 0x0a,
     165,   11,   11,   11, 0x0a,
     173,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Viewfinder[] = {
    "Viewfinder\0\0processFrames(int)\0"
    "processFrame()\0toggleCube()\0toggleGourd()\0"
    "paintCube()\0paintGourd()\0changeX()\0"
    "changeY()\0changeZ()\0rotateX()\0rotateY()\0"
    "rotateZ()\0plus()\0minus()\0openDirectory()\0"
};

const QMetaObject Viewfinder::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Viewfinder,
      qt_meta_data_Viewfinder, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Viewfinder::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Viewfinder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Viewfinder::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Viewfinder))
        return static_cast<void*>(const_cast< Viewfinder*>(this));
    return QWidget::qt_metacast(_clname);
}

int Viewfinder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: processFrames((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: processFrame(); break;
        case 2: toggleCube(); break;
        case 3: toggleGourd(); break;
        case 4: paintCube(); break;
        case 5: paintGourd(); break;
        case 6: changeX(); break;
        case 7: changeY(); break;
        case 8: changeZ(); break;
        case 9: rotateX(); break;
        case 10: rotateY(); break;
        case 11: rotateZ(); break;
        case 12: plus(); break;
        case 13: minus(); break;
        case 14: openDirectory(); break;
        default: ;
        }
        _id -= 15;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
