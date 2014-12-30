/****************************************************************************
** Meta object code from reading C++ file 'DkBasicLoader.h'
**
** Created: Thu Dec 18 00:26:17 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkBasicLoader.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkBasicLoader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkBasicLoader[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   20,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
      63,   51,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkBasicLoader[] = {
    "nmc::DkBasicLoader\0\0msg\0"
    "errorDialogSignal(QString)\0orientation\0"
    "rotate(int)\0"
};

void nmc::DkBasicLoader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkBasicLoader *_t = static_cast<DkBasicLoader *>(_o);
        switch (_id) {
        case 0: _t->errorDialogSignal((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->rotate((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkBasicLoader::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkBasicLoader::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__DkBasicLoader,
      qt_meta_data_nmc__DkBasicLoader, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkBasicLoader::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkBasicLoader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkBasicLoader::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkBasicLoader))
        return static_cast<void*>(const_cast< DkBasicLoader*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::DkBasicLoader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkBasicLoader::errorDialogSignal(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
