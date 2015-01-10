/****************************************************************************
** Meta object code from reading C++ file 'DkImage.h'
**
** Created: Thu Dec 18 00:26:15 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkImage.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkImage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkImageLoader[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      49,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      26,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   20,   19,   19, 0x05,
      62,   19,   19,   19, 0x05,
     104,   82,   19,   19, 0x05,
     156,  146,   19,   19, 0x25,
     198,  194,   19,   19, 0x25,
     249,  231,   19,   19, 0x05,
     292,  286,   19,   19, 0x25,
     325,   19,   19,   19, 0x25,
     373,  354,   19,   19, 0x05,
     434,  420,   19,   19, 0x25,
     480,  473,   19,   19, 0x25,
     519,  514,   19,   19, 0x25,
     557,  547,   19,   19, 0x05,
     589,  194,   19,   19, 0x25,
     613,  514,   19,   19, 0x05,
     649,  644,   19,   19, 0x05,
     665,  514,   19,   19, 0x05,
     706,  700,   19,   19, 0x05,
     773,  760,   19,   19, 0x05,
     831,  700,   19,   19, 0x25,
     902,  884,   19,   19, 0x05,
     943,  934,   19,   19, 0x25,
     971,  194,   19,   19, 0x25,
     995,   19,   19,   19, 0x05,
    1029, 1022,   19,   19, 0x05,
    1097, 1090,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
    1129, 1121,   19,   19, 0x0a,
    1150, 1145,   19,   19, 0x0a,
    1176,   19,   19,   19, 0x2a,
    1235, 1195,   19,   19, 0x0a,
    1302, 1274,   19,   19, 0x2a,
    1354, 1337,   19,   19, 0x2a,
    1390, 1381,   19,   19, 0x2a,
    1410,  700,   19,   19, 0x0a,
    1450,   19,   19,   19, 0x2a,
    1457,  514,   19,   19, 0x0a,
    1473,   19,   19,   19, 0x0a,
    1519, 1496, 1486,   19, 0x0a,
    1578, 1561, 1486,   19, 0x2a,
    1624, 1615, 1486,   19, 0x2a,
    1657, 1653, 1486,   19, 0x2a,
    1678,   20,   19,   19, 0x0a,
    1720,   19, 1708,   19, 0x0a,
    1746, 1739,   19,   19, 0x0a,
    1764,   19,   19,   19, 0x2a,
    1789, 1778,   19,   19, 0x0a,
    1816,  514,   19,   19, 0x2a,
    1843,   19, 1838,   19, 0x0a,
    1856,   19,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkImageLoader[] = {
    "nmc::DkImageLoader\0\0filters\0"
    "folderFiltersChanged(QStringList)\0"
    "updateImageSignal()\0msg,start,timeDelayed\0"
    "updateInfoSignalDelayed(QString,bool,int)\0"
    "msg,start\0updateInfoSignalDelayed(QString,bool)\0"
    "msg\0updateInfoSignalDelayed(QString)\0"
    "start,timeDelayed\0"
    "updateSpinnerSignalDelayed(bool,int)\0"
    "start\0updateSpinnerSignalDelayed(bool)\0"
    "updateSpinnerSignalDelayed()\0"
    "file,s,edited,attr\0"
    "updateFileSignal(QFileInfo,QSize,bool,QString)\0"
    "file,s,edited\0updateFileSignal(QFileInfo,QSize,bool)\0"
    "file,s\0updateFileSignal(QFileInfo,QSize)\0"
    "file\0updateFileSignal(QFileInfo)\0"
    "msg,title\0newErrorDialog(QString,QString)\0"
    "newErrorDialog(QString)\0"
    "fileNotLoadedSignal(QFileInfo)\0play\0"
    "setPlayer(bool)\0updateFileWatcherSignal(QFileInfo)\0"
    "image\0imageUpdatedSignal(QSharedPointer<DkImageContainerT>)\0"
    "image,loaded\0"
    "imageLoadedSignal(QSharedPointer<DkImageContainerT>,bool)\0"
    "imageLoadedSignal(QSharedPointer<DkImageContainerT>)\0"
    "msg,time,position\0showInfoSignal(QString,int,int)\0"
    "msg,time\0showInfoSignal(QString,int)\0"
    "showInfoSignal(QString)\0"
    "errorDialogSignal(QString)\0images\0"
    "updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)\0"
    "hasGPS\0imageHasGPSSignal(bool)\0skipIdx\0"
    "changeFile(int)\0path\0directoryChanged(QString)\0"
    "directoryChanged()\0"
    "filename,saveImg,fileFilter,compression\0"
    "saveFile(QFileInfo,QImage,QString,int)\0"
    "filename,saveImg,fileFilter\0"
    "saveFile(QFileInfo,QImage,QString)\0"
    "filename,saveImg\0saveFile(QFileInfo,QImage)\0"
    "filename\0saveFile(QFileInfo)\0"
    "load(QSharedPointer<DkImageContainerT>)\0"
    "load()\0load(QFileInfo)\0deleteFile()\0"
    "QFileInfo\0img,name,fileExt,force\0"
    "saveTempFile(QImage,QString,QString,bool)\0"
    "img,name,fileExt\0saveTempFile(QImage,QString,QString)\0"
    "img,name\0saveTempFile(QImage,QString)\0"
    "img\0saveTempFile(QImage)\0"
    "setFolderFilters(QStringList)\0QStringList\0"
    "getFolderFilters()\0loaded\0imageLoaded(bool)\0"
    "imageLoaded()\0file,saved\0"
    "imageSaved(QFileInfo,bool)\0"
    "imageSaved(QFileInfo)\0bool\0unloadFile()\0"
    "reloadImage()\0"
};

void nmc::DkImageLoader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkImageLoader *_t = static_cast<DkImageLoader *>(_o);
        switch (_id) {
        case 0: _t->folderFiltersChanged((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 1: _t->updateImageSignal(); break;
        case 2: _t->updateInfoSignalDelayed((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 3: _t->updateInfoSignalDelayed((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->updateInfoSignalDelayed((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->updateSpinnerSignalDelayed((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->updateSpinnerSignalDelayed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->updateSpinnerSignalDelayed(); break;
        case 8: _t->updateFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< QSize(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 9: _t->updateFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< QSize(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 10: _t->updateFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< QSize(*)>(_a[2]))); break;
        case 11: _t->updateFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 12: _t->newErrorDialog((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 13: _t->newErrorDialog((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 14: _t->fileNotLoadedSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 15: _t->setPlayer((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->updateFileWatcherSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 17: _t->imageUpdatedSignal((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 18: _t->imageLoadedSignal((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 19: _t->imageLoadedSignal((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 20: _t->showInfoSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 21: _t->showInfoSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 22: _t->showInfoSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 23: _t->errorDialogSignal((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 24: _t->updateDirSignal((*reinterpret_cast< QVector<QSharedPointer<DkImageContainerT> >(*)>(_a[1]))); break;
        case 25: _t->imageHasGPSSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 26: _t->changeFile((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 27: _t->directoryChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 28: _t->directoryChanged(); break;
        case 29: _t->saveFile((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< QImage(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 30: _t->saveFile((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< QImage(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 31: _t->saveFile((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< QImage(*)>(_a[2]))); break;
        case 32: _t->saveFile((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 33: _t->load((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 34: _t->load(); break;
        case 35: _t->load((*reinterpret_cast< const QFileInfo(*)>(_a[1]))); break;
        case 36: _t->deleteFile(); break;
        case 37: { QFileInfo _r = _t->saveTempFile((*reinterpret_cast< QImage(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])));
            if (_a[0]) *reinterpret_cast< QFileInfo*>(_a[0]) = _r; }  break;
        case 38: { QFileInfo _r = _t->saveTempFile((*reinterpret_cast< QImage(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< QFileInfo*>(_a[0]) = _r; }  break;
        case 39: { QFileInfo _r = _t->saveTempFile((*reinterpret_cast< QImage(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QFileInfo*>(_a[0]) = _r; }  break;
        case 40: { QFileInfo _r = _t->saveTempFile((*reinterpret_cast< QImage(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QFileInfo*>(_a[0]) = _r; }  break;
        case 41: _t->setFolderFilters((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 42: { QStringList _r = _t->getFolderFilters();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = _r; }  break;
        case 43: _t->imageLoaded((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 44: _t->imageLoaded(); break;
        case 45: _t->imageSaved((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 46: _t->imageSaved((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 47: { bool _r = _t->unloadFile();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 48: _t->reloadImage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkImageLoader::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkImageLoader::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__DkImageLoader,
      qt_meta_data_nmc__DkImageLoader, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkImageLoader::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkImageLoader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkImageLoader::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkImageLoader))
        return static_cast<void*>(const_cast< DkImageLoader*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::DkImageLoader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 49)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 49;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkImageLoader::folderFiltersChanged(QStringList _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkImageLoader::updateImageSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void nmc::DkImageLoader::updateInfoSignalDelayed(QString _t1, bool _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 5
void nmc::DkImageLoader::updateSpinnerSignalDelayed(bool _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 8
void nmc::DkImageLoader::updateFileSignal(QFileInfo _t1, QSize _t2, bool _t3, QString _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 12
void nmc::DkImageLoader::newErrorDialog(QString _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 14
void nmc::DkImageLoader::fileNotLoadedSignal(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void nmc::DkImageLoader::setPlayer(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void nmc::DkImageLoader::updateFileWatcherSignal(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}

// SIGNAL 17
void nmc::DkImageLoader::imageUpdatedSignal(QSharedPointer<DkImageContainerT> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 17, _a);
}

// SIGNAL 18
void nmc::DkImageLoader::imageLoadedSignal(QSharedPointer<DkImageContainerT> _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 18, _a);
}

// SIGNAL 20
void nmc::DkImageLoader::showInfoSignal(QString _t1, int _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 20, _a);
}

// SIGNAL 23
void nmc::DkImageLoader::errorDialogSignal(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 23, _a);
}

// SIGNAL 24
void nmc::DkImageLoader::updateDirSignal(QVector<QSharedPointer<DkImageContainerT> > _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 24, _a);
}

// SIGNAL 25
void nmc::DkImageLoader::imageHasGPSSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 25, _a);
}
static const uint qt_meta_data_nmc__DkColorLoader[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      33,   20,   19,   19, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkColorLoader[] = {
    "nmc::DkColorLoader\0\0cols,indexes\0"
    "updateSignal(QVector<QColor>,QVector<int>)\0"
};

void nmc::DkColorLoader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkColorLoader *_t = static_cast<DkColorLoader *>(_o);
        switch (_id) {
        case 0: _t->updateSignal((*reinterpret_cast< const QVector<QColor>(*)>(_a[1])),(*reinterpret_cast< const QVector<int>(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkColorLoader::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkColorLoader::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_nmc__DkColorLoader,
      qt_meta_data_nmc__DkColorLoader, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkColorLoader::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkColorLoader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkColorLoader::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkColorLoader))
        return static_cast<void*>(const_cast< DkColorLoader*>(this));
    return QThread::qt_metacast(_clname);
}

int nmc::DkColorLoader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkColorLoader::updateSignal(const QVector<QColor> & _t1, const QVector<int> & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
