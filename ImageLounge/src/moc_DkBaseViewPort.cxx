/****************************************************************************
** Meta object code from reading C++ file 'DkBaseViewPort.h'
**
** Created: Thu Dec 18 00:26:15 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkBaseViewPort.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkBaseViewPort.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkBaseViewPort[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   21,   20,   20, 0x05,
      69,   54,   20,   20, 0x05,
      94,   20,   20,   20, 0x05,
     113,  109,   20,   20, 0x05,
     143,  137,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
     177,  172,   20,   20, 0x0a,
     197,   20,   20,   20, 0x0a,
     209,   20,   20,   20, 0x0a,
     222,   20,   20,   20, 0x0a,
     232,   20,   20,   20, 0x0a,
     244,   20,   20,   20, 0x0a,
     262,   20,   20,   20, 0x0a,
     271,   20,   20,   20, 0x0a,
     281,   20,   20,   20, 0x0a,
     293,   20,   20,   20, 0x0a,
     304,  137,   20,   20, 0x0a,
     331,   20,   20,   20, 0x0a,
     356,  350,   20,   20, 0x0a,
     399,  388,  383,   20, 0x0a,
     417,   20,  383,   20, 0x2a,
     438,  431,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkBaseViewPort[] = {
    "nmc::DkBaseViewPort\0\0enable\0"
    "enableNoImageSignal(bool)\0show,permanent\0"
    "showStatusBar(bool,bool)\0imageUpdated()\0"
    "img\0newImageSignal(QImage*)\0event\0"
    "keyReleaseSignal(QKeyEvent*)\0show\0"
    "togglePattern(bool)\0shiftLeft()\0"
    "shiftRight()\0shiftUp()\0shiftDown()\0"
    "moveView(QPointF)\0zoomIn()\0zoomOut()\0"
    "resetView()\0fullView()\0"
    "resizeEvent(QResizeEvent*)\0"
    "stopBlockZooming()\0brush\0"
    "setBackgroundBrush(QBrush)\0bool\0"
    "fileChange\0unloadImage(bool)\0unloadImage()\0"
    "newImg\0setImage(QImage)\0"
};

void nmc::DkBaseViewPort::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkBaseViewPort *_t = static_cast<DkBaseViewPort *>(_o);
        switch (_id) {
        case 0: _t->enableNoImageSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->showStatusBar((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->imageUpdated(); break;
        case 3: _t->newImageSignal((*reinterpret_cast< QImage*(*)>(_a[1]))); break;
        case 4: _t->keyReleaseSignal((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        case 5: _t->togglePattern((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->shiftLeft(); break;
        case 7: _t->shiftRight(); break;
        case 8: _t->shiftUp(); break;
        case 9: _t->shiftDown(); break;
        case 10: _t->moveView((*reinterpret_cast< QPointF(*)>(_a[1]))); break;
        case 11: _t->zoomIn(); break;
        case 12: _t->zoomOut(); break;
        case 13: _t->resetView(); break;
        case 14: _t->fullView(); break;
        case 15: _t->resizeEvent((*reinterpret_cast< QResizeEvent*(*)>(_a[1]))); break;
        case 16: _t->stopBlockZooming(); break;
        case 17: _t->setBackgroundBrush((*reinterpret_cast< const QBrush(*)>(_a[1]))); break;
        case 18: { bool _r = _t->unloadImage((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 19: { bool _r = _t->unloadImage();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 20: _t->setImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkBaseViewPort::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkBaseViewPort::staticMetaObject = {
    { &QGraphicsView::staticMetaObject, qt_meta_stringdata_nmc__DkBaseViewPort,
      qt_meta_data_nmc__DkBaseViewPort, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkBaseViewPort::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkBaseViewPort::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkBaseViewPort::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkBaseViewPort))
        return static_cast<void*>(const_cast< DkBaseViewPort*>(this));
    return QGraphicsView::qt_metacast(_clname);
}

int nmc::DkBaseViewPort::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkBaseViewPort::enableNoImageSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkBaseViewPort::showStatusBar(bool _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkBaseViewPort::imageUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void nmc::DkBaseViewPort::newImageSignal(QImage * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void nmc::DkBaseViewPort::keyReleaseSignal(QKeyEvent * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_END_MOC_NAMESPACE
