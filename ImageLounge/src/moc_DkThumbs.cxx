/****************************************************************************
** Meta object code from reading C++ file 'DkThumbs.h'
**
** Created: Thu Dec 18 00:26:17 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkThumbs.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkThumbs.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkThumbNailT[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   19,   18,   18, 0x05,
      50,   18,   18,   18, 0x25,
      70,   18,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      85,   18,   18,   18, 0x09,
      99,   18,   18,   18, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkThumbNailT[] = {
    "nmc::DkThumbNailT\0\0loaded\0"
    "thumbLoadedSignal(bool)\0thumbLoadedSignal()\0"
    "colorUpdated()\0thumbLoaded()\0colorLoaded()\0"
};

void nmc::DkThumbNailT::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkThumbNailT *_t = static_cast<DkThumbNailT *>(_o);
        switch (_id) {
        case 0: _t->thumbLoadedSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->thumbLoadedSignal(); break;
        case 2: _t->colorUpdated(); break;
        case 3: _t->thumbLoaded(); break;
        case 4: _t->colorLoaded(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkThumbNailT::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkThumbNailT::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__DkThumbNailT,
      qt_meta_data_nmc__DkThumbNailT, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkThumbNailT::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkThumbNailT::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkThumbNailT::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkThumbNailT))
        return static_cast<void*>(const_cast< DkThumbNailT*>(this));
    if (!strcmp(_clname, "DkThumbNail"))
        return static_cast< DkThumbNail*>(const_cast< DkThumbNailT*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::DkThumbNailT::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkThumbNailT::thumbLoadedSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 2
void nmc::DkThumbNailT::colorUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
static const uint qt_meta_data_nmc__DkThumbsLoader[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x05,
      44,   36,   20,   20, 0x05,
      73,   64,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
     103,   93,   20,   20, 0x0a,
     132,  126,   20,   20, 0x2a,
     151,   20,   20,   20, 0x2a,
     167,   20,   20,   20, 0x0a,
     177,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkThumbsLoader[] = {
    "nmc::DkThumbsLoader\0\0updateSignal()\0"
    "percent\0progressSignal(int)\0numFiles\0"
    "numFilesSignal(int)\0start,end\0"
    "setLoadLimits(int,int)\0start\0"
    "setLoadLimits(int)\0setLoadLimits()\0"
    "loadAll()\0stop()\0"
};

void nmc::DkThumbsLoader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkThumbsLoader *_t = static_cast<DkThumbsLoader *>(_o);
        switch (_id) {
        case 0: _t->updateSignal(); break;
        case 1: _t->progressSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->numFilesSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->setLoadLimits((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->setLoadLimits((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->setLoadLimits(); break;
        case 6: _t->loadAll(); break;
        case 7: _t->stop(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkThumbsLoader::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkThumbsLoader::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_nmc__DkThumbsLoader,
      qt_meta_data_nmc__DkThumbsLoader, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkThumbsLoader::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkThumbsLoader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkThumbsLoader::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkThumbsLoader))
        return static_cast<void*>(const_cast< DkThumbsLoader*>(this));
    return QThread::qt_metacast(_clname);
}

int nmc::DkThumbsLoader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkThumbsLoader::updateSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkThumbsLoader::progressSignal(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkThumbsLoader::numFilesSignal(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
