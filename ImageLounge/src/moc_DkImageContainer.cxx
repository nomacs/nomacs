/****************************************************************************
** Meta object code from reading C++ file 'DkImageContainer.h'
**
** Created: Thu Dec 18 00:26:16 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkImageContainer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkImageContainer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkImageContainerT[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: signature, parameters, type, tag, flags
      31,   24,   23,   23, 0x05,
      54,   23,   23,   23, 0x25,
      88,   73,   23,   23, 0x05,
     129,  120,   23,   23, 0x25,
     174,  156,   23,   23, 0x05,
     215,  206,   23,   23, 0x25,
     247,  243,   23,   23, 0x25,
     271,  243,   23,   23, 0x05,
     298,   24,   23,   23, 0x05,
     322,   23,   23,   23, 0x25,

 // slots: signature, parameters, type, tag, flags
     342,   23,   23,   23, 0x0a,
     364,   23,   23,   23, 0x09,
     379,   23,   23,   23, 0x09,
     393,   23,   23,   23, 0x09,
     410,   23,   23,   23, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkImageContainerT[] = {
    "nmc::DkImageContainerT\0\0loaded\0"
    "fileLoadedSignal(bool)\0fileLoadedSignal()\0"
    "fileInfo,saved\0fileSavedSignal(QFileInfo,bool)\0"
    "fileInfo\0fileSavedSignal(QFileInfo)\0"
    "msg,time,position\0showInfoSignal(QString,int,int)\0"
    "msg,time\0showInfoSignal(QString,int)\0"
    "msg\0showInfoSignal(QString)\0"
    "errorDialogSignal(QString)\0"
    "thumbLoadedSignal(bool)\0thumbLoadedSignal()\0"
    "checkForFileUpdates()\0bufferLoaded()\0"
    "imageLoaded()\0savingFinished()\0"
    "loadingFinished()\0"
};

void nmc::DkImageContainerT::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkImageContainerT *_t = static_cast<DkImageContainerT *>(_o);
        switch (_id) {
        case 0: _t->fileLoadedSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->fileLoadedSignal(); break;
        case 2: _t->fileSavedSignal((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->fileSavedSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 4: _t->showInfoSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 5: _t->showInfoSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->showInfoSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: _t->errorDialogSignal((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->thumbLoadedSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->thumbLoadedSignal(); break;
        case 10: _t->checkForFileUpdates(); break;
        case 11: _t->bufferLoaded(); break;
        case 12: _t->imageLoaded(); break;
        case 13: _t->savingFinished(); break;
        case 14: _t->loadingFinished(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkImageContainerT::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkImageContainerT::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__DkImageContainerT,
      qt_meta_data_nmc__DkImageContainerT, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkImageContainerT::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkImageContainerT::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkImageContainerT::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkImageContainerT))
        return static_cast<void*>(const_cast< DkImageContainerT*>(this));
    if (!strcmp(_clname, "DkImageContainer"))
        return static_cast< DkImageContainer*>(const_cast< DkImageContainerT*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::DkImageContainerT::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkImageContainerT::fileLoadedSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 2
void nmc::DkImageContainerT::fileSavedSignal(QFileInfo _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 4
void nmc::DkImageContainerT::showInfoSignal(QString _t1, int _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 7
void nmc::DkImageContainerT::errorDialogSignal(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void nmc::DkImageContainerT::thumbLoadedSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_END_MOC_NAMESPACE
