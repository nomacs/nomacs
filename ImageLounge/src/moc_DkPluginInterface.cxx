/****************************************************************************
** Meta object code from reading C++ file 'DkPluginInterface.h'
**
** Created: Thu Dec 18 00:26:16 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkPluginInterface.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkPluginInterface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkPluginViewPort[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      50,   23,   22,   22, 0x05,
      86,   73,   22,   22, 0x25,
     104,   22,   22,   22, 0x25,
     131,  118,   22,   22, 0x05,
     164,  159,   22,   22, 0x05,
     190,  184,   22,   22, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkPluginViewPort[] = {
    "nmc::DkPluginViewPort\0\0"
    "askForSaving,alreadySaving\0"
    "closePlugin(bool,bool)\0askForSaving\0"
    "closePlugin(bool)\0closePlugin()\0"
    "toolbar,show\0showToolbar(QToolBar*,bool)\0"
    "file\0loadFile(QFileInfo)\0image\0"
    "loadImage(QImage)\0"
};

void nmc::DkPluginViewPort::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkPluginViewPort *_t = static_cast<DkPluginViewPort *>(_o);
        switch (_id) {
        case 0: _t->closePlugin((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->closePlugin((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->closePlugin(); break;
        case 3: _t->showToolbar((*reinterpret_cast< QToolBar*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->loadFile((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 5: _t->loadImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkPluginViewPort::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkPluginViewPort::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkPluginViewPort,
      qt_meta_data_nmc__DkPluginViewPort, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkPluginViewPort::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkPluginViewPort::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkPluginViewPort::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkPluginViewPort))
        return static_cast<void*>(const_cast< DkPluginViewPort*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkPluginViewPort::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkPluginViewPort::closePlugin(bool _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 3
void nmc::DkPluginViewPort::showToolbar(QToolBar * _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void nmc::DkPluginViewPort::loadFile(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void nmc::DkPluginViewPort::loadImage(QImage _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE
