/****************************************************************************
** Meta object code from reading C++ file 'DkThumbsWidgets.h'
**
** Created: Thu Dec 18 00:26:16 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkThumbsWidgets.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkThumbsWidgets.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkFilePreview[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      25,   20,   19,   19, 0x05,
      55,   51,   19,   19, 0x05,
      81,   77,   19,   19, 0x05,
     112,  107,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
     139,   19,   19,   19, 0x0a,
     160,  152,   19,   19, 0x0a,
     186,  179,   19,   19, 0x0a,
     251,  244,   19,   19, 0x0a,
     298,   19,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkFilePreview[] = {
    "nmc::DkFilePreview\0\0file\0"
    "loadFileSignal(QFileInfo)\0idx\0"
    "changeFileSignal(int)\0pos\0"
    "positionChangeSignal(int)\0show\0"
    "showThumbsDockSignal(bool)\0moveImages()\0"
    "fileIdx\0updateFileIdx(int)\0thumbs\0"
    "updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)\0"
    "cImage\0setFileInfo(QSharedPointer<DkImageContainerT>)\0"
    "newPosition()\0"
};

void nmc::DkFilePreview::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkFilePreview *_t = static_cast<DkFilePreview *>(_o);
        switch (_id) {
        case 0: _t->loadFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 1: _t->changeFileSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->positionChangeSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->showThumbsDockSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->moveImages(); break;
        case 5: _t->updateFileIdx((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->updateThumbs((*reinterpret_cast< QVector<QSharedPointer<DkImageContainerT> >(*)>(_a[1]))); break;
        case 7: _t->setFileInfo((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 8: _t->newPosition(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkFilePreview::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkFilePreview::staticMetaObject = {
    { &DkWidget::staticMetaObject, qt_meta_stringdata_nmc__DkFilePreview,
      qt_meta_data_nmc__DkFilePreview, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkFilePreview::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkFilePreview::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkFilePreview::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkFilePreview))
        return static_cast<void*>(const_cast< DkFilePreview*>(this));
    return DkWidget::qt_metacast(_clname);
}

int nmc::DkFilePreview::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkFilePreview::loadFileSignal(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkFilePreview::changeFileSignal(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkFilePreview::positionChangeSignal(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void nmc::DkFilePreview::showThumbsDockSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
static const uint qt_meta_data_nmc__DkThumbLabel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   19,   18,   18, 0x05,
      51,   19,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      77,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkThumbLabel[] = {
    "nmc::DkThumbLabel\0\0file\0"
    "loadFileSignal(QFileInfo&)\0"
    "showFileSignal(QFileInfo)\0updateLabel()\0"
};

void nmc::DkThumbLabel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkThumbLabel *_t = static_cast<DkThumbLabel *>(_o);
        switch (_id) {
        case 0: _t->loadFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 1: _t->showFileSignal((*reinterpret_cast< const QFileInfo(*)>(_a[1]))); break;
        case 2: _t->updateLabel(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkThumbLabel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkThumbLabel::staticMetaObject = {
    { &QGraphicsObject::staticMetaObject, qt_meta_stringdata_nmc__DkThumbLabel,
      qt_meta_data_nmc__DkThumbLabel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkThumbLabel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkThumbLabel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkThumbLabel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkThumbLabel))
        return static_cast<void*>(const_cast< DkThumbLabel*>(this));
    return QGraphicsObject::qt_metacast(_clname);
}

int nmc::DkThumbLabel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkThumbLabel::loadFileSignal(QFileInfo & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkThumbLabel::showFileSignal(const QFileInfo & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_nmc__DkThumbScene[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   19,   18,   18, 0x05,
      58,   50,   18,   18, 0x05,
      92,   88,   18,   18, 0x25,
     118,   18,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
     138,   18,   18,   18, 0x0a,
     158,   19,   18,   18, 0x0a,
     179,   18,   18,   18, 0x0a,
     196,   18,   18,   18, 0x0a,
     221,  213,   18,   18, 0x0a,
     250,  247,   18,   18, 0x0a,
     270,   19,   18,   18, 0x0a,
     305,  290,   18,   18, 0x0a,
     344,  332,   18,   18, 0x2a,
     374,  367,   18,   18, 0x2a,
     393,   18,   18,   18, 0x2a,
     408,  367,   18,   18, 0x0a,
     430,   18,   18,   18, 0x2a,
     455,  448,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkThumbScene[] = {
    "nmc::DkThumbScene\0\0file\0"
    "loadFileSignal(QFileInfo)\0msg,pos\0"
    "statusInfoSignal(QString,int)\0msg\0"
    "statusInfoSignal(QString)\0thumbLoadedSignal()\0"
    "updateThumbLabels()\0loadFile(QFileInfo&)\0"
    "increaseThumbs()\0decreaseThumbs()\0"
    "squares\0toggleSquaredThumbs(bool)\0dx\0"
    "resizeThumbs(float)\0showFile(QFileInfo)\0"
    "select,from,to\0selectThumbs(bool,int,int)\0"
    "select,from\0selectThumbs(bool,int)\0"
    "select\0selectThumbs(bool)\0selectThumbs()\0"
    "selectAllThumbs(bool)\0selectAllThumbs()\0"
    "thumbs\0updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)\0"
};

void nmc::DkThumbScene::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkThumbScene *_t = static_cast<DkThumbScene *>(_o);
        switch (_id) {
        case 0: _t->loadFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 1: _t->statusInfoSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->statusInfoSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->thumbLoadedSignal(); break;
        case 4: _t->updateThumbLabels(); break;
        case 5: _t->loadFile((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 6: _t->increaseThumbs(); break;
        case 7: _t->decreaseThumbs(); break;
        case 8: _t->toggleSquaredThumbs((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->resizeThumbs((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 10: _t->showFile((*reinterpret_cast< const QFileInfo(*)>(_a[1]))); break;
        case 11: _t->selectThumbs((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 12: _t->selectThumbs((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 13: _t->selectThumbs((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->selectThumbs(); break;
        case 15: _t->selectAllThumbs((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->selectAllThumbs(); break;
        case 17: _t->updateThumbs((*reinterpret_cast< QVector<QSharedPointer<DkImageContainerT> >(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkThumbScene::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkThumbScene::staticMetaObject = {
    { &QGraphicsScene::staticMetaObject, qt_meta_stringdata_nmc__DkThumbScene,
      qt_meta_data_nmc__DkThumbScene, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkThumbScene::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkThumbScene::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkThumbScene::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkThumbScene))
        return static_cast<void*>(const_cast< DkThumbScene*>(this));
    return QGraphicsScene::qt_metacast(_clname);
}

int nmc::DkThumbScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsScene::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkThumbScene::loadFileSignal(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkThumbScene::statusInfoSignal(QString _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 3
void nmc::DkThumbScene::thumbLoadedSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}
static const uint qt_meta_data_nmc__DkThumbsView[] = {

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
      24,   19,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      51,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkThumbsView[] = {
    "nmc::DkThumbsView\0\0file\0"
    "updateDirSignal(QFileInfo)\0fetchThumbs()\0"
};

void nmc::DkThumbsView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkThumbsView *_t = static_cast<DkThumbsView *>(_o);
        switch (_id) {
        case 0: _t->updateDirSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 1: _t->fetchThumbs(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkThumbsView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkThumbsView::staticMetaObject = {
    { &QGraphicsView::staticMetaObject, qt_meta_stringdata_nmc__DkThumbsView,
      qt_meta_data_nmc__DkThumbsView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkThumbsView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkThumbsView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkThumbsView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkThumbsView))
        return static_cast<void*>(const_cast< DkThumbsView*>(this));
    return QGraphicsView::qt_metacast(_clname);
}

int nmc::DkThumbsView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
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
void nmc::DkThumbsView::updateDirSignal(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkThumbScrollWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      31,   26,   25,   25, 0x05,

 // slots: signature, parameters, type, tag, flags
      66,   58,   25,   25, 0x0a,
      90,   83,   25,   25, 0x0a,
     148,   26,   25,   25, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkThumbScrollWidget[] = {
    "nmc::DkThumbScrollWidget\0\0file\0"
    "updateDirSignal(QFileInfo)\0visible\0"
    "setVisible(bool)\0thumbs\0"
    "updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)\0"
    "setDir(QFileInfo)\0"
};

void nmc::DkThumbScrollWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkThumbScrollWidget *_t = static_cast<DkThumbScrollWidget *>(_o);
        switch (_id) {
        case 0: _t->updateDirSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 1: _t->setVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->updateThumbs((*reinterpret_cast< QVector<QSharedPointer<DkImageContainerT> >(*)>(_a[1]))); break;
        case 3: _t->setDir((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkThumbScrollWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkThumbScrollWidget::staticMetaObject = {
    { &DkWidget::staticMetaObject, qt_meta_stringdata_nmc__DkThumbScrollWidget,
      qt_meta_data_nmc__DkThumbScrollWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkThumbScrollWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkThumbScrollWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkThumbScrollWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkThumbScrollWidget))
        return static_cast<void*>(const_cast< DkThumbScrollWidget*>(this));
    return DkWidget::qt_metacast(_clname);
}

int nmc::DkThumbScrollWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkThumbScrollWidget::updateDirSignal(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
