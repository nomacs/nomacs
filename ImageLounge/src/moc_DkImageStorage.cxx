/****************************************************************************
** Meta object code from reading C++ file 'DkImageStorage.h'
**
** Created: Thu Dec 18 00:26:15 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkImageStorage.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkImageStorage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkImageStorage[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x05,
      40,   36,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
      60,   20,   20,   20, 0x0a,
      88,   75,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkImageStorage[] = {
    "nmc::DkImageStorage\0\0imageUpdated()\0"
    "msg\0infoSignal(QString)\0computeImage()\0"
    "antiAliasing\0antiAliasingChanged(bool)\0"
};

void nmc::DkImageStorage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkImageStorage *_t = static_cast<DkImageStorage *>(_o);
        switch (_id) {
        case 0: _t->imageUpdated(); break;
        case 1: _t->infoSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->computeImage(); break;
        case 3: _t->antiAliasingChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkImageStorage::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkImageStorage::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__DkImageStorage,
      qt_meta_data_nmc__DkImageStorage, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkImageStorage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkImageStorage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkImageStorage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkImageStorage))
        return static_cast<void*>(const_cast< DkImageStorage*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::DkImageStorage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkImageStorage::imageUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkImageStorage::infoSignal(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
