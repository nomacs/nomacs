/****************************************************************************
** Meta object code from reading C++ file 'DkCentralWidget.h'
**
** Created: Thu Dec 18 00:26:15 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkCentralWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkCentralWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkCentralWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
      52,   48,   21,   21, 0x0a,
     108,   99,   21,   21, 0x0a,
     132,  128,   21,   21, 0x0a,
     155,  128,   21,   21, 0x0a,
     186,  178,   21,   21, 0x0a,
     216,  204,   21,   21, 0x0a,
     267,  262,   21,   21, 0x2a,
     309,   21,   21,   21, 0x2a,
     331,  318,   21,   21, 0x0a,
     362,  353,   21,   21, 0x2a,
     387,  380,   21,   21, 0x0a,
     402,   21,   21,   21, 0x2a,
     414,   21,   21,   21, 0x0a,
     424,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkCentralWidget[] = {
    "nmc::DkCentralWidget\0\0loadFileSignal(QFileInfo)\0"
    "img\0imageLoaded(QSharedPointer<DkImageContainerT>)\0"
    "isLoaded\0updateTabIcon(bool)\0idx\0"
    "currentTabChanged(int)\0tabCloseRequested(int)\0"
    "from,to\0tabMoved(int,int)\0imgC,tabIdx\0"
    "addTab(QSharedPointer<DkImageContainerT>,int)\0"
    "imgC\0addTab(QSharedPointer<DkImageContainerT>)\0"
    "addTab()\0fileInfo,idx\0addTab(QFileInfo,int)\0"
    "fileInfo\0addTab(QFileInfo)\0tabIdx\0"
    "removeTab(int)\0removeTab()\0nextTab()\0"
    "previousTab()\0"
};

void nmc::DkCentralWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkCentralWidget *_t = static_cast<DkCentralWidget *>(_o);
        switch (_id) {
        case 0: _t->loadFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 1: _t->imageLoaded((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 2: _t->updateTabIcon((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->currentTabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->tabCloseRequested((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->tabMoved((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->addTab((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->addTab((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 8: _t->addTab(); break;
        case 9: _t->addTab((*reinterpret_cast< const QFileInfo(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 10: _t->addTab((*reinterpret_cast< const QFileInfo(*)>(_a[1]))); break;
        case 11: _t->removeTab((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->removeTab(); break;
        case 13: _t->nextTab(); break;
        case 14: _t->previousTab(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkCentralWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkCentralWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkCentralWidget,
      qt_meta_data_nmc__DkCentralWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkCentralWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkCentralWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkCentralWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkCentralWidget))
        return static_cast<void*>(const_cast< DkCentralWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkCentralWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkCentralWidget::loadFileSignal(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
