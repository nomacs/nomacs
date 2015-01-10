/****************************************************************************
** Meta object code from reading C++ file 'DkPong.h'
**
** Created: Thu Dec 18 00:26:15 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkPong.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkPong.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkPongPort[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkPongPort[] = {
    "nmc::DkPongPort\0\0gameLoop()\0"
};

void nmc::DkPongPort::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkPongPort *_t = static_cast<DkPongPort *>(_o);
        switch (_id) {
        case 0: _t->gameLoop(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkPongPort::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkPongPort::staticMetaObject = {
    { &QGraphicsView::staticMetaObject, qt_meta_stringdata_nmc__DkPongPort,
      qt_meta_data_nmc__DkPongPort, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkPongPort::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkPongPort::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkPongPort::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkPongPort))
        return static_cast<void*>(const_cast< DkPongPort*>(this));
    return QGraphicsView::qt_metacast(_clname);
}

int nmc::DkPongPort::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkPong[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   13,   12,   12, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkPong[] = {
    "nmc::DkPong\0\0event\0keyPressEvent(QKeyEvent*)\0"
};

void nmc::DkPong::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkPong *_t = static_cast<DkPong *>(_o);
        switch (_id) {
        case 0: _t->keyPressEvent((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkPong::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkPong::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_nmc__DkPong,
      qt_meta_data_nmc__DkPong, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkPong::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkPong::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkPong::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkPong))
        return static_cast<void*>(const_cast< DkPong*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int nmc::DkPong::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
