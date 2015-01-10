/****************************************************************************
** Meta object code from reading C++ file 'DkViewPort.h'
**
** Created: Thu Dec 18 00:26:15 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkViewPort.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkViewPort.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkDelayedInfo[] = {

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
      25,   20,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
      41,   19,   19,   19, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkDelayedInfo[] = {
    "nmc::DkDelayedInfo\0\0time\0infoSignal(int)\0"
    "sendInfo()\0"
};

void nmc::DkDelayedInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkDelayedInfo *_t = static_cast<DkDelayedInfo *>(_o);
        switch (_id) {
        case 0: _t->infoSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sendInfo(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkDelayedInfo::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkDelayedInfo::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__DkDelayedInfo,
      qt_meta_data_nmc__DkDelayedInfo, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkDelayedInfo::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkDelayedInfo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkDelayedInfo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkDelayedInfo))
        return static_cast<void*>(const_cast< DkDelayedInfo*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::DkDelayedInfo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::DkDelayedInfo::infoSignal(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkDelayedMessage[] = {

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
      32,   23,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
      56,   22,   22,   22, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkDelayedMessage[] = {
    "nmc::DkDelayedMessage\0\0msg,time\0"
    "infoSignal(QString,int)\0sendInfo()\0"
};

void nmc::DkDelayedMessage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkDelayedMessage *_t = static_cast<DkDelayedMessage *>(_o);
        switch (_id) {
        case 0: _t->infoSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->sendInfo(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkDelayedMessage::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkDelayedMessage::staticMetaObject = {
    { &DkDelayedInfo::staticMetaObject, qt_meta_stringdata_nmc__DkDelayedMessage,
      qt_meta_data_nmc__DkDelayedMessage, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkDelayedMessage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkDelayedMessage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkDelayedMessage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkDelayedMessage))
        return static_cast<void*>(const_cast< DkDelayedMessage*>(this));
    return DkDelayedInfo::qt_metacast(_clname);
}

int nmc::DkDelayedMessage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkDelayedInfo::qt_metacall(_c, _id, _a);
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
void nmc::DkDelayedMessage::infoSignal(QString _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkControlWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      31,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      30,   22,   21,   21, 0x0a,
      48,   22,   21,   21, 0x0a,
      67,   22,   21,   21, 0x0a,
      86,   22,   21,   21, 0x0a,
     105,   22,   21,   21, 0x0a,
     127,  122,   21,   21, 0x0a,
     142,   21,   21,   21, 0x2a,
     153,   22,   21,   21, 0x0a,
     168,   22,   21,   21, 0x0a,
     187,   22,   21,   21, 0x0a,
     207,   22,   21,   21, 0x0a,
     231,   22,   21,   21, 0x0a,
     251,   22,   21,   21, 0x0a,
     280,  273,   21,   21, 0x0a,
     303,   21,   21,   21, 0x2a,
     322,  318,   21,   21, 0x0a,
     356,  351,   21,   21, 0x0a,
     421,  403,   21,   21, 0x0a,
     455,  446,   21,   21, 0x2a,
     480,  476,   21,   21, 0x2a,
     517,  497,   21,   21, 0x0a,
     560,  550,   21,   21, 0x2a,
     589,  476,   21,   21, 0x2a,
     618,  613,   21,   21, 0x0a,
     634,   21,   21,   21, 0x2a,
     658,  647,   21,   21, 0x0a,
     692,  686,   21,   21, 0x2a,
     716,   21,   21,   21, 0x2a,
     743,  736,   21,   21, 0x0a,
     768,  761,   21,   21, 0x0a,
     786,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkControlWidget[] = {
    "nmc::DkControlWidget\0\0visible\0"
    "showPreview(bool)\0showScroller(bool)\0"
    "showMetaData(bool)\0showFileInfo(bool)\0"
    "showPlayer(bool)\0hide\0hideCrop(bool)\0"
    "hideCrop()\0showCrop(bool)\0showOverview(bool)\0"
    "showHistogram(bool)\0showCommentWidget(bool)\0"
    "showThumbView(bool)\0showRecentFiles(bool)\0"
    "widget\0switchWidget(QWidget*)\0"
    "switchWidget()\0pos\0changeThumbNailPosition(int)\0"
    "imgC\0setFileInfo(QSharedPointer<DkImageContainerT>)\0"
    "msg,time,location\0setInfo(QString,int,int)\0"
    "msg,time\0setInfo(QString,int)\0msg\0"
    "setInfo(QString)\0msg,start,delayTime\0"
    "setInfoDelayed(QString,bool,int)\0"
    "msg,start\0setInfoDelayed(QString,bool)\0"
    "setInfoDelayed(QString)\0time\0"
    "setSpinner(int)\0setSpinner()\0start,time\0"
    "setSpinnerDelayed(bool,int)\0start\0"
    "setSpinnerDelayed(bool)\0setSpinnerDelayed()\0"
    "rating\0updateRating(int)\0loaded\0"
    "imageLoaded(bool)\0update()\0"
};

void nmc::DkControlWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkControlWidget *_t = static_cast<DkControlWidget *>(_o);
        switch (_id) {
        case 0: _t->showPreview((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->showScroller((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->showMetaData((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->showFileInfo((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->showPlayer((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->hideCrop((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->hideCrop(); break;
        case 7: _t->showCrop((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->showOverview((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->showHistogram((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->showCommentWidget((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->showThumbView((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->showRecentFiles((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->switchWidget((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 14: _t->switchWidget(); break;
        case 15: _t->changeThumbNailPosition((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->setFileInfo((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 17: _t->setInfo((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 18: _t->setInfo((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 19: _t->setInfo((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 20: _t->setInfoDelayed((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 21: _t->setInfoDelayed((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 22: _t->setInfoDelayed((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 23: _t->setSpinner((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 24: _t->setSpinner(); break;
        case 25: _t->setSpinnerDelayed((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 26: _t->setSpinnerDelayed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 27: _t->setSpinnerDelayed(); break;
        case 28: _t->updateRating((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 29: _t->imageLoaded((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 30: _t->update(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkControlWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkControlWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkControlWidget,
      qt_meta_data_nmc__DkControlWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkControlWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkControlWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkControlWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkControlWidget))
        return static_cast<void*>(const_cast< DkControlWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkControlWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkViewPort[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      55,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: signature, parameters, type, tag, flags
      51,   17,   16,   16, 0x05,
     114,  102,   16,   16, 0x05,
     151,  148,   16,   16, 0x25,
     187,  177,   16,   16, 0x05,
     224,  219,   16,   16, 0x05,
     268,  254,   16,   16, 0x05,
     312,  304,   16,   16, 0x05,
     340,  336,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
     360,   16,   16,   16, 0x0a,
     371,   16,   16,   16, 0x0a,
     383,   16,   16,   16, 0x0a,
     395,   16,   16,   16, 0x0a,
     407,   16,   16,   16, 0x0a,
     424,  418,   16,   16, 0x0a,
     451,   16,   16,   16, 0x0a,
     504,  471,   16,   16, 0x0a,
     557,  552,   16,   16, 0x0a,
     596,  581,   16,   16, 0x0a,
     623,   16,   16,   16, 0x2a,
     640,   16,   16,   16, 0x0a,
     675,  662,   16,   16, 0x0a,
     709,  703,   16,   16, 0x0a,
     750,  743,   16,   16, 0x0a,
     769,   16,   16,   16, 0x2a,
     789,  784,   16,   16, 0x0a,
     809,   16,   16,   16, 0x0a,
     822,   16,   16,   16, 0x0a,
     837,   16,   16,   16, 0x0a,
     856,   16,   16,   16, 0x0a,
     887,  875,   16,   16, 0x0a,
     917,  909,   16,   16, 0x2a,
     935,  909,   16,   16, 0x0a,
     949,   16,   16,   16, 0x0a,
     961,   16,   16,   16, 0x0a,
     972,   16,   16,   16, 0x0a,
     989,   16,   16,   16, 0x0a,
    1006,   16,   16,   16, 0x0a,
    1033, 1022, 1017,   16, 0x0a,
    1051,   16, 1017,   16, 0x2a,
    1076, 1065,   16,   16, 0x0a,
    1109,   16,   16,   16, 0x0a,
    1135, 1122,   16,   16, 0x0a,
    1193, 1187,   16,   16, 0x2a,
    1247, 1240,   16,   16, 0x0a,
    1269, 1265,   16,   16, 0x0a,
    1314, 1240,   16,   16, 0x0a,
    1337, 1240,   16,   16, 0x0a,
    1354, 1240,   16,   16, 0x0a,
    1376,   16,   16,   16, 0x0a,
    1401, 1394,   16,   16, 0x0a,
    1418,   16,   16,   16, 0x0a,
    1435,   16,   16,   16, 0x0a,
    1456,   16,   16,   16, 0x0a,
    1470,   16,   16,   16, 0x0a,
    1489, 1484,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkViewPort[] = {
    "nmc::DkViewPort\0\0transform,imgTransform,canvasSize\0"
    "sendTransformSignal(QTransform,QTransform,QPointF)\0"
    "op,filename\0sendNewFileSignal(qint16,QString)\0"
    "op\0sendNewFileSignal(qint16)\0img,title\0"
    "sendImageSignal(QImage,QString)\0msg,\0"
    "statusInfoSignal(QString,int)\0"
    "connect,local\0newClientConnectedSignal(bool,bool)\0"
    "isMovie\0movieLoadedSignal(bool)\0msg\0"
    "infoSignal(QString)\0rotateCW()\0"
    "rotateCCW()\0rotate180()\0resetView()\0"
    "fullView()\0event\0resizeEvent(QResizeEvent*)\0"
    "toggleResetMatrix()\0"
    "worldMatrix,imgMatrix,canvasSize\0"
    "tcpSetTransforms(QTransform,QTransform,QPointF)\0"
    "rect\0tcpSetWindowRect(QRect)\0"
    "relativeMatrix\0tcpSynchronize(QTransform)\0"
    "tcpSynchronize()\0tcpForceSynchronize()\0"
    "idx,filename\0tcpLoadFile(qint16,QString)\0"
    "peers\0tcpShowConnections(QList<DkPeer>)\0"
    "silent\0tcpSendImage(bool)\0tcpSendImage()\0"
    "file\0loadFile(QFileInfo)\0reloadFile()\0"
    "loadFullFile()\0loadNextFileFast()\0"
    "loadPrevFileFast()\0skipIdx,rec\0"
    "loadFileFast(int,int)\0skipIdx\0"
    "loadFileFast(int)\0loadFile(int)\0"
    "loadFirst()\0loadLast()\0loadSkipNext10()\0"
    "loadSkipPrev10()\0loadLena()\0bool\0"
    "fileChange\0unloadImage(bool)\0unloadImage()\0"
    "rect,bgCol\0cropImage(DkRotatingRect,QColor)\0"
    "repeatZoom()\0image,loaded\0"
    "updateImage(QSharedPointer<DkImageContainerT>,bool)\0"
    "image\0updateImage(QSharedPointer<DkImageContainerT>)\0"
    "newImg\0loadImage(QImage)\0img\0"
    "loadImage(QSharedPointer<DkImageContainerT>)\0"
    "setEditedImage(QImage)\0setImage(QImage)\0"
    "setThumbImage(QImage)\0settingsChanged()\0"
    "paused\0pauseMovie(bool)\0nextMovieFrame()\0"
    "previousMovieFrame()\0animateFade()\0"
    "animateMove()\0show\0togglePattern(bool)\0"
};

void nmc::DkViewPort::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkViewPort *_t = static_cast<DkViewPort *>(_o);
        switch (_id) {
        case 0: _t->sendTransformSignal((*reinterpret_cast< QTransform(*)>(_a[1])),(*reinterpret_cast< QTransform(*)>(_a[2])),(*reinterpret_cast< QPointF(*)>(_a[3]))); break;
        case 1: _t->sendNewFileSignal((*reinterpret_cast< qint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->sendNewFileSignal((*reinterpret_cast< qint16(*)>(_a[1]))); break;
        case 3: _t->sendImageSignal((*reinterpret_cast< QImage(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 4: _t->statusInfoSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->newClientConnectedSignal((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 6: _t->movieLoadedSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->infoSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: _t->rotateCW(); break;
        case 9: _t->rotateCCW(); break;
        case 10: _t->rotate180(); break;
        case 11: _t->resetView(); break;
        case 12: _t->fullView(); break;
        case 13: _t->resizeEvent((*reinterpret_cast< QResizeEvent*(*)>(_a[1]))); break;
        case 14: _t->toggleResetMatrix(); break;
        case 15: _t->tcpSetTransforms((*reinterpret_cast< QTransform(*)>(_a[1])),(*reinterpret_cast< QTransform(*)>(_a[2])),(*reinterpret_cast< QPointF(*)>(_a[3]))); break;
        case 16: _t->tcpSetWindowRect((*reinterpret_cast< QRect(*)>(_a[1]))); break;
        case 17: _t->tcpSynchronize((*reinterpret_cast< QTransform(*)>(_a[1]))); break;
        case 18: _t->tcpSynchronize(); break;
        case 19: _t->tcpForceSynchronize(); break;
        case 20: _t->tcpLoadFile((*reinterpret_cast< qint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 21: _t->tcpShowConnections((*reinterpret_cast< QList<DkPeer>(*)>(_a[1]))); break;
        case 22: _t->tcpSendImage((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 23: _t->tcpSendImage(); break;
        case 24: _t->loadFile((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 25: _t->reloadFile(); break;
        case 26: _t->loadFullFile(); break;
        case 27: _t->loadNextFileFast(); break;
        case 28: _t->loadPrevFileFast(); break;
        case 29: _t->loadFileFast((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 30: _t->loadFileFast((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 31: _t->loadFile((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 32: _t->loadFirst(); break;
        case 33: _t->loadLast(); break;
        case 34: _t->loadSkipNext10(); break;
        case 35: _t->loadSkipPrev10(); break;
        case 36: _t->loadLena(); break;
        case 37: { bool _r = _t->unloadImage((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 38: { bool _r = _t->unloadImage();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 39: _t->cropImage((*reinterpret_cast< DkRotatingRect(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 40: _t->repeatZoom(); break;
        case 41: _t->updateImage((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 42: _t->updateImage((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 43: _t->loadImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 44: _t->loadImage((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 45: _t->setEditedImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 46: _t->setImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 47: _t->setThumbImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 48: _t->settingsChanged(); break;
        case 49: _t->pauseMovie((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 50: _t->nextMovieFrame(); break;
        case 51: _t->previousMovieFrame(); break;
        case 52: _t->animateFade(); break;
        case 53: _t->animateMove(); break;
        case 54: _t->togglePattern((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkViewPort::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkViewPort::staticMetaObject = {
    { &DkBaseViewPort::staticMetaObject, qt_meta_stringdata_nmc__DkViewPort,
      qt_meta_data_nmc__DkViewPort, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkViewPort::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkViewPort::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkViewPort::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkViewPort))
        return static_cast<void*>(const_cast< DkViewPort*>(this));
    return DkBaseViewPort::qt_metacast(_clname);
}

int nmc::DkViewPort::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkBaseViewPort::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 55)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 55;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkViewPort::sendTransformSignal(QTransform _t1, QTransform _t2, QPointF _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkViewPort::sendNewFileSignal(qint16 _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 3
void nmc::DkViewPort::sendImageSignal(QImage _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void nmc::DkViewPort::statusInfoSignal(QString _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void nmc::DkViewPort::newClientConnectedSignal(bool _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void nmc::DkViewPort::movieLoadedSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void nmc::DkViewPort::infoSignal(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
static const uint qt_meta_data_nmc__DkViewPortFrameless[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      33,   26,   25,   25, 0x0a,
      50,   25,   25,   25, 0x0a,
      62,   25,   25,   25, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkViewPortFrameless[] = {
    "nmc::DkViewPortFrameless\0\0newImg\0"
    "setImage(QImage)\0resetView()\0"
    "moveView(QPointF)\0"
};

void nmc::DkViewPortFrameless::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkViewPortFrameless *_t = static_cast<DkViewPortFrameless *>(_o);
        switch (_id) {
        case 0: _t->setImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 1: _t->resetView(); break;
        case 2: _t->moveView((*reinterpret_cast< QPointF(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkViewPortFrameless::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkViewPortFrameless::staticMetaObject = {
    { &DkViewPort::staticMetaObject, qt_meta_stringdata_nmc__DkViewPortFrameless,
      qt_meta_data_nmc__DkViewPortFrameless, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkViewPortFrameless::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkViewPortFrameless::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkViewPortFrameless::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkViewPortFrameless))
        return static_cast<void*>(const_cast< DkViewPortFrameless*>(this));
    return DkViewPort::qt_metacast(_clname);
}

int nmc::DkViewPortFrameless::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkViewPort::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkViewPortContrast[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      29,   25,   24,   24, 0x05,
      55,   50,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
      81,   73,   24,   24, 0x0a,
     106,  100,   24,   24, 0x0a,
     139,   24,   24,   24, 0x0a,
     158,  151,   24,   24, 0x0a,
     180,   24,  173,   24, 0x0a,
     198,  191,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkViewPortContrast[] = {
    "nmc::DkViewPortContrast\0\0pos\0"
    "tFSliderAdded(qreal)\0mode\0imageModeSet(int)\0"
    "channel\0changeChannel(int)\0stops\0"
    "changeColorTable(QGradientStops)\0"
    "pickColor()\0enable\0enableTF(bool)\0"
    "QImage\0getImage()\0newImg\0setImage(QImage)\0"
};

void nmc::DkViewPortContrast::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkViewPortContrast *_t = static_cast<DkViewPortContrast *>(_o);
        switch (_id) {
        case 0: _t->tFSliderAdded((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 1: _t->imageModeSet((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->changeChannel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->changeColorTable((*reinterpret_cast< QGradientStops(*)>(_a[1]))); break;
        case 4: _t->pickColor(); break;
        case 5: _t->enableTF((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: { QImage _r = _t->getImage();
            if (_a[0]) *reinterpret_cast< QImage*>(_a[0]) = _r; }  break;
        case 7: _t->setImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkViewPortContrast::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkViewPortContrast::staticMetaObject = {
    { &DkViewPort::staticMetaObject, qt_meta_stringdata_nmc__DkViewPortContrast,
      qt_meta_data_nmc__DkViewPortContrast, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkViewPortContrast::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkViewPortContrast::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkViewPortContrast::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkViewPortContrast))
        return static_cast<void*>(const_cast< DkViewPortContrast*>(this));
    return DkViewPort::qt_metacast(_clname);
}

int nmc::DkViewPortContrast::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkViewPort::qt_metacall(_c, _id, _a);
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
void nmc::DkViewPortContrast::tFSliderAdded(qreal _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkViewPortContrast::imageModeSet(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
