/****************************************************************************
** Meta object code from reading C++ file 'DkMenu.h'
**
** Created: Thu Dec 18 00:26:16 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkMenu.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkMenu.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkMenuBar[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x0a,
      27,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkMenuBar[] = {
    "nmc::DkMenuBar\0\0showMenu()\0hideMenu()\0"
};

void nmc::DkMenuBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkMenuBar *_t = static_cast<DkMenuBar *>(_o);
        switch (_id) {
        case 0: _t->showMenu(); break;
        case 1: _t->hideMenu(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkMenuBar::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkMenuBar::staticMetaObject = {
    { &QMenuBar::staticMetaObject, qt_meta_stringdata_nmc__DkMenuBar,
      qt_meta_data_nmc__DkMenuBar, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkMenuBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkMenuBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkMenuBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkMenuBar))
        return static_cast<void*>(const_cast< DkMenuBar*>(this));
    return QMenuBar::qt_metacast(_clname);
}

int nmc::DkMenuBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMenuBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkTcpAction[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x05,
      49,   17,   17,   17, 0x05,
      94,   87,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
     122,  114,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkTcpAction[] = {
    "nmc::DkTcpAction\0\0synchronizeWithSignal(quint16)\0"
    "disableSynchronizeWithSignal(quint16)\0"
    "enable\0enableActions(bool)\0checked\0"
    "synchronize(bool)\0"
};

void nmc::DkTcpAction::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkTcpAction *_t = static_cast<DkTcpAction *>(_o);
        switch (_id) {
        case 0: _t->synchronizeWithSignal((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 1: _t->disableSynchronizeWithSignal((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 2: _t->enableActions((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->synchronize((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkTcpAction::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkTcpAction::staticMetaObject = {
    { &QAction::staticMetaObject, qt_meta_stringdata_nmc__DkTcpAction,
      qt_meta_data_nmc__DkTcpAction, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkTcpAction::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkTcpAction::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkTcpAction::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkTcpAction))
        return static_cast<void*>(const_cast< DkTcpAction*>(this));
    return QAction::qt_metacast(_clname);
}

int nmc::DkTcpAction::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAction::qt_metacall(_c, _id, _a);
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
void nmc::DkTcpAction::synchronizeWithSignal(quint16 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkTcpAction::disableSynchronizeWithSignal(quint16 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkTcpAction::enableActions(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_nmc__DkTcpMenu[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      60,   47,   15,   15, 0x0a,
      92,   85,   15,   15, 0x2a,
     112,   15,   15,   15, 0x2a,
     128,   15,   15,   15, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkTcpMenu[] = {
    "nmc::DkTcpMenu\0\0synchronizeWithSignal(quint16)\0"
    "enable,local\0enableActions(bool,bool)\0"
    "enable\0enableActions(bool)\0enableActions()\0"
    "updatePeers()\0"
};

void nmc::DkTcpMenu::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkTcpMenu *_t = static_cast<DkTcpMenu *>(_o);
        switch (_id) {
        case 0: _t->synchronizeWithSignal((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 1: _t->enableActions((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->enableActions((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->enableActions(); break;
        case 4: _t->updatePeers(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkTcpMenu::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkTcpMenu::staticMetaObject = {
    { &QMenu::staticMetaObject, qt_meta_stringdata_nmc__DkTcpMenu,
      qt_meta_data_nmc__DkTcpMenu, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkTcpMenu::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkTcpMenu::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkTcpMenu::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkTcpMenu))
        return static_cast<void*>(const_cast< DkTcpMenu*>(this));
    return QMenu::qt_metacast(_clname);
}

int nmc::DkTcpMenu::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMenu::qt_metacall(_c, _id, _a);
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
void nmc::DkTcpMenu::synchronizeWithSignal(quint16 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
