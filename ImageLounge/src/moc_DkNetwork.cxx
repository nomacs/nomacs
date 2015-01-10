/****************************************************************************
** Meta object code from reading C++ file 'DkNetwork.h'
**
** Created: Thu Dec 18 00:26:15 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkNetwork.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkNetwork.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkClientManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      40,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      20,       // signalCount

 // signals: signature, parameters, type, tag, flags
      56,   22,   21,   21, 0x05,
     136,  110,   21,   21, 0x05,
     182,  170,   21,   21, 0x05,
     220,  214,   21,   21, 0x05,
     248,  242,   21,   21, 0x05,
     285,  276,   21,   21, 0x05,
     317,  313,   21,   21, 0x25,
     341,  242,   21,   21, 0x05,
     370,   21,   21,   21, 0x05,
     395,   21,   21,   21, 0x05,
     436,  427,   21,   21, 0x05,
     465,  110,   21,   21, 0x05,
     505,   22,   21,   21, 0x05,
     560,  170,   21,   21, 0x05,
     607,  595,   21,   21, 0x05,
     654,  643,   21,   21, 0x05,
     691,   21,   21,   21, 0x05,
     720,  712,   21,   21, 0x05,
     771,  765,   21,   21, 0x05,
     809,   21,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
     831,  824,   21,   21, 0x0a,
     861,  856,   21,   21, 0x0a,
     896,  824,   21,   21, 0x0a,
     934,  925,   21,   21, 0x0a,
     953,   22,   21,   21, 0x0a,
    1015,  998,   21,   21, 0x0a,
    1040,  170,   21,   21, 0x0a,
    1068,  595,   21,   21, 0x0a,
    1097,   21,   21,   21, 0x0a,
    1133, 1116,   21,   21, 0x09,
    1176, 1152,   21,   21, 0x09,
    1271, 1229,   21,   21, 0x09,
    1335, 1324,   21,   21, 0x09,
    1397, 1377,   21,   21, 0x09,
    1488, 1443,   21,   21, 0x09,
    1599, 1566,   21,   21, 0x09,
    1680, 1657,   21,   21, 0x09,
    1736, 1324,   21,   21, 0x09,
    1792, 1777,   21,   21, 0x09,
    1843,   21,   21,   21, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkClientManager[] = {
    "nmc::DkClientManager\0\0"
    "transform,imgTransform,canvasSize\0"
    "receivedTransformation(QTransform,QTransform,QPointF)\0"
    "position,opacity,overlaid\0"
    "receivedPosition(QRect,bool,bool)\0"
    "op,filename\0receivedNewFile(qint16,QString)\0"
    "image\0receivedImage(QImage)\0title\0"
    "receivedImageTitle(QString)\0msg,time\0"
    "sendInfoSignal(QString,int)\0msg\0"
    "sendInfoSignal(QString)\0"
    "sendGreetingMessage(QString)\0"
    "sendSynchronizeMessage()\0"
    "sendDisableSynchronizeMessage()\0"
    "newtitle\0sendNewTitleMessage(QString)\0"
    "sendNewPositionMessage(QRect,bool,bool)\0"
    "sendNewTransformMessage(QTransform,QTransform,QPointF)\0"
    "sendNewFileMessage(qint16,QString)\0"
    "image,title\0sendNewImageMessage(QImage,QString)\0"
    "imageTitle\0sendNewUpcomingImageMessage(QString)\0"
    "sendGoodByeMessage()\0newList\0"
    "synchronizedPeersListChanged(QList<quint16>)\0"
    "peers\0updateConnectionSignal(QList<DkPeer>)\0"
    "receivedQuit()\0peerId\0synchronizeWith(quint16)\0"
    "port\0synchronizeWithServerPort(quint16)\0"
    "stopSynchronizeWith(quint16)\0newTitle\0"
    "sendTitle(QString)\0"
    "sendTransform(QTransform,QTransform,QPointF)\0"
    "newRect,overlaid\0sendPosition(QRect,bool)\0"
    "sendNewFile(qint16,QString)\0"
    "sendNewImage(QImage,QString)\0"
    "sendGoodByeToAll()\0socketDescriptor\0"
    "newConnection(int)\0peerId,title,connection\0"
    "connectionReadyForUse(quint16,QString,DkConnection*)\0"
    "synchronizedPeersOfOtherClient,connection\0"
    "connectionSynchronized(QList<quint16>,DkConnection*)\0"
    "connection\0connectionStopSynchronized(DkConnection*)\0"
    "connection,newTitle\0"
    "connectionSentNewTitle(DkConnection*,QString)\0"
    "connection,transform,imgTransform,canvasSize\0"
    "connectionReceivedTransformation(DkConnection*,QTransform,QTransform,Q"
    "PointF)\0"
    "connection,rect,opacity,overlaid\0"
    "connectionReceivedPosition(DkConnection*,QRect,bool,bool)\0"
    "connection,op,filename\0"
    "connectionReceivedNewFile(DkConnection*,qint16,QString)\0"
    "connectionReceivedGoodBye(DkConnection*)\0"
    "connection,msg\0"
    "connectionShowStatusMessage(DkConnection*,QString)\0"
    "disconnected()\0"
};

void nmc::DkClientManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkClientManager *_t = static_cast<DkClientManager *>(_o);
        switch (_id) {
        case 0: _t->receivedTransformation((*reinterpret_cast< QTransform(*)>(_a[1])),(*reinterpret_cast< QTransform(*)>(_a[2])),(*reinterpret_cast< QPointF(*)>(_a[3]))); break;
        case 1: _t->receivedPosition((*reinterpret_cast< QRect(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 2: _t->receivedNewFile((*reinterpret_cast< qint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 3: _t->receivedImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 4: _t->receivedImageTitle((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->sendInfoSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->sendInfoSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: _t->sendGreetingMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: _t->sendSynchronizeMessage(); break;
        case 9: _t->sendDisableSynchronizeMessage(); break;
        case 10: _t->sendNewTitleMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 11: _t->sendNewPositionMessage((*reinterpret_cast< QRect(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 12: _t->sendNewTransformMessage((*reinterpret_cast< QTransform(*)>(_a[1])),(*reinterpret_cast< QTransform(*)>(_a[2])),(*reinterpret_cast< QPointF(*)>(_a[3]))); break;
        case 13: _t->sendNewFileMessage((*reinterpret_cast< qint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 14: _t->sendNewImageMessage((*reinterpret_cast< QImage(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 15: _t->sendNewUpcomingImageMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 16: _t->sendGoodByeMessage(); break;
        case 17: _t->synchronizedPeersListChanged((*reinterpret_cast< QList<quint16>(*)>(_a[1]))); break;
        case 18: _t->updateConnectionSignal((*reinterpret_cast< QList<DkPeer>(*)>(_a[1]))); break;
        case 19: _t->receivedQuit(); break;
        case 20: _t->synchronizeWith((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 21: _t->synchronizeWithServerPort((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 22: _t->stopSynchronizeWith((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 23: _t->sendTitle((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 24: _t->sendTransform((*reinterpret_cast< QTransform(*)>(_a[1])),(*reinterpret_cast< QTransform(*)>(_a[2])),(*reinterpret_cast< QPointF(*)>(_a[3]))); break;
        case 25: _t->sendPosition((*reinterpret_cast< QRect(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 26: _t->sendNewFile((*reinterpret_cast< qint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 27: _t->sendNewImage((*reinterpret_cast< QImage(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 28: _t->sendGoodByeToAll(); break;
        case 29: _t->newConnection((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 30: _t->connectionReadyForUse((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< DkConnection*(*)>(_a[3]))); break;
        case 31: _t->connectionSynchronized((*reinterpret_cast< QList<quint16>(*)>(_a[1])),(*reinterpret_cast< DkConnection*(*)>(_a[2]))); break;
        case 32: _t->connectionStopSynchronized((*reinterpret_cast< DkConnection*(*)>(_a[1]))); break;
        case 33: _t->connectionSentNewTitle((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 34: _t->connectionReceivedTransformation((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QTransform(*)>(_a[2])),(*reinterpret_cast< QTransform(*)>(_a[3])),(*reinterpret_cast< QPointF(*)>(_a[4]))); break;
        case 35: _t->connectionReceivedPosition((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QRect(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 36: _t->connectionReceivedNewFile((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< qint16(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 37: _t->connectionReceivedGoodBye((*reinterpret_cast< DkConnection*(*)>(_a[1]))); break;
        case 38: _t->connectionShowStatusMessage((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 39: _t->disconnected(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkClientManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkClientManager::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_nmc__DkClientManager,
      qt_meta_data_nmc__DkClientManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkClientManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkClientManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkClientManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkClientManager))
        return static_cast<void*>(const_cast< DkClientManager*>(this));
    return QThread::qt_metacast(_clname);
}

int nmc::DkClientManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 40)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 40;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkClientManager::receivedTransformation(QTransform _t1, QTransform _t2, QPointF _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkClientManager::receivedPosition(QRect _t1, bool _t2, bool _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkClientManager::receivedNewFile(qint16 _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void nmc::DkClientManager::receivedImage(QImage _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void nmc::DkClientManager::receivedImageTitle(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void nmc::DkClientManager::sendInfoSignal(QString _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 7
void nmc::DkClientManager::sendGreetingMessage(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void nmc::DkClientManager::sendSynchronizeMessage()
{
    QMetaObject::activate(this, &staticMetaObject, 8, 0);
}

// SIGNAL 9
void nmc::DkClientManager::sendDisableSynchronizeMessage()
{
    QMetaObject::activate(this, &staticMetaObject, 9, 0);
}

// SIGNAL 10
void nmc::DkClientManager::sendNewTitleMessage(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void nmc::DkClientManager::sendNewPositionMessage(QRect _t1, bool _t2, bool _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void nmc::DkClientManager::sendNewTransformMessage(QTransform _t1, QTransform _t2, QPointF _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void nmc::DkClientManager::sendNewFileMessage(qint16 _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void nmc::DkClientManager::sendNewImageMessage(QImage _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void nmc::DkClientManager::sendNewUpcomingImageMessage(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void nmc::DkClientManager::sendGoodByeMessage()
{
    QMetaObject::activate(this, &staticMetaObject, 16, 0);
}

// SIGNAL 17
void nmc::DkClientManager::synchronizedPeersListChanged(QList<quint16> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 17, _a);
}

// SIGNAL 18
void nmc::DkClientManager::updateConnectionSignal(QList<DkPeer> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 18, _a);
}

// SIGNAL 19
void nmc::DkClientManager::receivedQuit()
{
    QMetaObject::activate(this, &staticMetaObject, 19, 0);
}
static const uint qt_meta_data_nmc__DkLocalClientManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x05,
      42,   26,   26,   26, 0x05,

 // slots: signature, parameters, type, tag, flags
      67,   60,   26,   26, 0x0a,
     101,   96,   26,   26, 0x0a,
     136,   60,   26,   26, 0x0a,
     170,  161,   26,   26, 0x0a,
     197,   26,   26,   26, 0x0a,
     264,  222,   26,   26, 0x08,
     328,  317,   26,   26, 0x08,
     370,   26,   26,   26, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkLocalClientManager[] = {
    "nmc::DkLocalClientManager\0\0receivedQuit()\0"
    "sendQuitMessage()\0peerId\0"
    "stopSynchronizeWith(quint16)\0port\0"
    "synchronizeWithServerPort(quint16)\0"
    "synchronizeWith(quint16)\0overlaid\0"
    "sendArrangeInstances(bool)\0"
    "sendQuitMessageToPeers()\0"
    "synchronizedPeersOfOtherClient,connection\0"
    "connectionSynchronized(QList<quint16>,DkConnection*)\0"
    "connection\0connectionStopSynchronized(DkConnection*)\0"
    "connectionReceivedQuit()\0"
};

void nmc::DkLocalClientManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkLocalClientManager *_t = static_cast<DkLocalClientManager *>(_o);
        switch (_id) {
        case 0: _t->receivedQuit(); break;
        case 1: _t->sendQuitMessage(); break;
        case 2: _t->stopSynchronizeWith((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 3: _t->synchronizeWithServerPort((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 4: _t->synchronizeWith((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 5: _t->sendArrangeInstances((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->sendQuitMessageToPeers(); break;
        case 7: _t->connectionSynchronized((*reinterpret_cast< QList<quint16>(*)>(_a[1])),(*reinterpret_cast< DkConnection*(*)>(_a[2]))); break;
        case 8: _t->connectionStopSynchronized((*reinterpret_cast< DkConnection*(*)>(_a[1]))); break;
        case 9: _t->connectionReceivedQuit(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkLocalClientManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkLocalClientManager::staticMetaObject = {
    { &DkClientManager::staticMetaObject, qt_meta_stringdata_nmc__DkLocalClientManager,
      qt_meta_data_nmc__DkLocalClientManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkLocalClientManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkLocalClientManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkLocalClientManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkLocalClientManager))
        return static_cast<void*>(const_cast< DkLocalClientManager*>(this));
    return DkClientManager::qt_metacast(_clname);
}

int nmc::DkLocalClientManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkClientManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkLocalClientManager::receivedQuit()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkLocalClientManager::sendQuitMessage()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
static const uint qt_meta_data_nmc__DkLANClientManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      38,   25,   24,   24, 0x05,
      89,   84,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
     125,  116,   24,   24, 0x0a,
     144,   84,   24,   24, 0x0a,
     186,  179,   24,   24, 0x0a,
     215,   24,   24,   24, 0x2a,
     242,  237,   24,   24, 0x0a,
     272,  260,   24,   24, 0x0a,
     301,  179,   24,   24, 0x0a,
     358,  326,   24,   24, 0x09,
     434,  411,   24,   24, 0x08,
     515,  491,   24,   24, 0x08,
     561,   24,   24,   24, 0x08,
     634,  592,   24,   24, 0x08,
     698,  687,   24,   24, 0x08,
     760,  740,   24,   24, 0x08,
     851,  806,   24,   24, 0x08,
     962,  929,   24,   24, 0x08,
    1043, 1020,   24,   24, 0x08,
    1121, 1099,   24,   24, 0x08,
    1200, 1176,   24,   24, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkLANClientManager[] = {
    "nmc::DkLANClientManager\0\0address,port\0"
    "sendSwitchServerMessage(QHostAddress,quint16)\0"
    "port\0serverPortChanged(quint16)\0"
    "newTitle\0sendTitle(QString)\0"
    "synchronizeWithServerPort(quint16)\0"
    "peerId\0stopSynchronizeWith(quint16)\0"
    "stopSynchronizeWith()\0flag\0startServer(bool)\0"
    "image,title\0sendNewImage(QImage,QString)\0"
    "synchronizeWith(quint16)\0"
    "peerServerPort,title,connection\0"
    "connectionReadyForUse(quint16,QString,DkConnection*)\0"
    "connection,image,title\0"
    "connectionReceivedNewImage(DkConnection*,QImage,QString)\0"
    "address,port,clientName\0"
    "startConnection(QHostAddress,quint16,QString)\0"
    "sendStopSynchronizationToAll()\0"
    "synchronizedPeersOfOtherClient,connection\0"
    "connectionSynchronized(QList<quint16>,DkConnection*)\0"
    "connection\0connectionStopSynchronized(DkConnection*)\0"
    "connection,newTitle\0"
    "connectionSentNewTitle(DkConnection*,QString)\0"
    "connection,transform,imgTransform,canvasSize\0"
    "connectionReceivedTransformation(DkConnection*,QTransform,QTransform,Q"
    "PointF)\0"
    "connection,rect,opacity,overlaid\0"
    "connectionReceivedPosition(DkConnection*,QRect,bool,bool)\0"
    "connection,op,filename\0"
    "connectionReceivedNewFile(DkConnection*,qint16,QString)\0"
    "connection,imageTitle\0"
    "connectionReceivedUpcomingImage(DkConnection*,QString)\0"
    "connection,address,port\0"
    "connectionReceivedSwitchServer(DkConnection*,QHostAddress,quint16)\0"
};

void nmc::DkLANClientManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkLANClientManager *_t = static_cast<DkLANClientManager *>(_o);
        switch (_id) {
        case 0: _t->sendSwitchServerMessage((*reinterpret_cast< QHostAddress(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 1: _t->serverPortChanged((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 2: _t->sendTitle((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->synchronizeWithServerPort((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 4: _t->stopSynchronizeWith((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 5: _t->stopSynchronizeWith(); break;
        case 6: _t->startServer((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->sendNewImage((*reinterpret_cast< QImage(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 8: _t->synchronizeWith((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 9: _t->connectionReadyForUse((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< DkConnection*(*)>(_a[3]))); break;
        case 10: _t->connectionReceivedNewImage((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QImage(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 11: _t->startConnection((*reinterpret_cast< QHostAddress(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 12: _t->sendStopSynchronizationToAll(); break;
        case 13: _t->connectionSynchronized((*reinterpret_cast< QList<quint16>(*)>(_a[1])),(*reinterpret_cast< DkConnection*(*)>(_a[2]))); break;
        case 14: _t->connectionStopSynchronized((*reinterpret_cast< DkConnection*(*)>(_a[1]))); break;
        case 15: _t->connectionSentNewTitle((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 16: _t->connectionReceivedTransformation((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QTransform(*)>(_a[2])),(*reinterpret_cast< QTransform(*)>(_a[3])),(*reinterpret_cast< QPointF(*)>(_a[4]))); break;
        case 17: _t->connectionReceivedPosition((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QRect(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 18: _t->connectionReceivedNewFile((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< qint16(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 19: _t->connectionReceivedUpcomingImage((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 20: _t->connectionReceivedSwitchServer((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QHostAddress(*)>(_a[2])),(*reinterpret_cast< quint16(*)>(_a[3]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkLANClientManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkLANClientManager::staticMetaObject = {
    { &DkClientManager::staticMetaObject, qt_meta_stringdata_nmc__DkLANClientManager,
      qt_meta_data_nmc__DkLANClientManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkLANClientManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkLANClientManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkLANClientManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkLANClientManager))
        return static_cast<void*>(const_cast< DkLANClientManager*>(this));
    return DkClientManager::qt_metacast(_clname);
}

int nmc::DkLANClientManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkClientManager::qt_metacall(_c, _id, _a);
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
void nmc::DkLANClientManager::sendSwitchServerMessage(QHostAddress _t1, quint16 _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkLANClientManager::serverPortChanged(quint16 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_nmc__DkRCClientManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      29,   24,   23,   23, 0x05,
      53,   24,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
      90,   83,   23,   23, 0x0a,
     115,   24,   23,   23, 0x0a,
     174,  132,   23,   23, 0x08,
     255,  227,   23,   23, 0x08,
     320,  304,   23,   23, 0x08,
     396,  364,   23,   23, 0x08,
     460,  449,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkRCClientManager[] = {
    "nmc::DkRCClientManager\0\0mode\0"
    "sendNewModeMessage(int)\0"
    "connectedReceivedNewMode(int)\0peerId\0"
    "synchronizeWith(quint16)\0sendNewMode(int)\0"
    "synchronizedPeersOfOtherClient,connection\0"
    "connectionSynchronized(QList<quint16>,DkConnection*)\0"
    "connection,allowedToConnect\0"
    "connectionReceivedPermission(DkConnection*,bool)\0"
    "connection,type\0"
    "connectionReceivedRCType(DkConnection*,int)\0"
    "peerServerPort,title,connection\0"
    "connectionReadyForUse(quint16,QString,DkConnection*)\0"
    "connection\0connectionReceivedGoodBye(DkConnection*)\0"
};

void nmc::DkRCClientManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkRCClientManager *_t = static_cast<DkRCClientManager *>(_o);
        switch (_id) {
        case 0: _t->sendNewModeMessage((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->connectedReceivedNewMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->synchronizeWith((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 3: _t->sendNewMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->connectionSynchronized((*reinterpret_cast< QList<quint16>(*)>(_a[1])),(*reinterpret_cast< DkConnection*(*)>(_a[2]))); break;
        case 5: _t->connectionReceivedPermission((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 6: _t->connectionReceivedRCType((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->connectionReadyForUse((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< DkConnection*(*)>(_a[3]))); break;
        case 8: _t->connectionReceivedGoodBye((*reinterpret_cast< DkConnection*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkRCClientManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkRCClientManager::staticMetaObject = {
    { &DkLANClientManager::staticMetaObject, qt_meta_stringdata_nmc__DkRCClientManager,
      qt_meta_data_nmc__DkRCClientManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkRCClientManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkRCClientManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkRCClientManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkRCClientManager))
        return static_cast<void*>(const_cast< DkRCClientManager*>(this));
    return DkLANClientManager::qt_metacast(_clname);
}

int nmc::DkRCClientManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkLANClientManager::qt_metacall(_c, _id, _a);
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
void nmc::DkRCClientManager::sendNewModeMessage(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkRCClientManager::connectedReceivedNewMode(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_nmc__DkLocalTcpServer[] = {

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
      36,   23,   22,   22, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkLocalTcpServer[] = {
    "nmc::DkLocalTcpServer\0\0DkDescriptor\0"
    "serverReiceivedNewConnection(int)\0"
};

void nmc::DkLocalTcpServer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkLocalTcpServer *_t = static_cast<DkLocalTcpServer *>(_o);
        switch (_id) {
        case 0: _t->serverReiceivedNewConnection((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkLocalTcpServer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkLocalTcpServer::staticMetaObject = {
    { &QTcpServer::staticMetaObject, qt_meta_stringdata_nmc__DkLocalTcpServer,
      qt_meta_data_nmc__DkLocalTcpServer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkLocalTcpServer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkLocalTcpServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkLocalTcpServer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkLocalTcpServer))
        return static_cast<void*>(const_cast< DkLocalTcpServer*>(this));
    return QTcpServer::qt_metacast(_clname);
}

int nmc::DkLocalTcpServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTcpServer::qt_metacall(_c, _id, _a);
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
void nmc::DkLocalTcpServer::serverReiceivedNewConnection(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkLANTcpServer[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      45,   21,   20,   20, 0x05,
     117,  104,   20,   20, 0x05,
     151,   20,   20,   20, 0x05,
     182,   20,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
     212,  207,   20,   20, 0x0a,
     230,   21,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkLANTcpServer[] = {
    "nmc::DkLANTcpServer\0\0address,port,clientName\0"
    "serverReiceivedNewConnection(QHostAddress,quint16,QString)\0"
    "DkDescriptor\0serverReiceivedNewConnection(int)\0"
    "sendStopSynchronizationToAll()\0"
    "sendNewClientBroadcast()\0flag\0"
    "startServer(bool)\0"
    "udpNewServerFound(QHostAddress,quint16,QString)\0"
};

void nmc::DkLANTcpServer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkLANTcpServer *_t = static_cast<DkLANTcpServer *>(_o);
        switch (_id) {
        case 0: _t->serverReiceivedNewConnection((*reinterpret_cast< QHostAddress(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 1: _t->serverReiceivedNewConnection((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->sendStopSynchronizationToAll(); break;
        case 3: _t->sendNewClientBroadcast(); break;
        case 4: _t->startServer((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->udpNewServerFound((*reinterpret_cast< QHostAddress(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkLANTcpServer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkLANTcpServer::staticMetaObject = {
    { &QTcpServer::staticMetaObject, qt_meta_stringdata_nmc__DkLANTcpServer,
      qt_meta_data_nmc__DkLANTcpServer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkLANTcpServer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkLANTcpServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkLANTcpServer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkLANTcpServer))
        return static_cast<void*>(const_cast< DkLANTcpServer*>(this));
    return QTcpServer::qt_metacast(_clname);
}

int nmc::DkLANTcpServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTcpServer::qt_metacall(_c, _id, _a);
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
void nmc::DkLANTcpServer::serverReiceivedNewConnection(QHostAddress _t1, quint16 _t2, QString _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkLANTcpServer::serverReiceivedNewConnection(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkLANTcpServer::sendStopSynchronizationToAll()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void nmc::DkLANTcpServer::sendNewClientBroadcast()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}
static const uint qt_meta_data_nmc__DkLANUdpSocket[] = {

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
      45,   21,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
     100,   20,   20,   20, 0x0a,
     116,   20,   20,   20, 0x0a,
     141,   20,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkLANUdpSocket[] = {
    "nmc::DkLANUdpSocket\0\0address,port,clientName\0"
    "udpSocketNewServerOnline(QHostAddress,quint16,QString)\0"
    "sendBroadcast()\0sendNewClientBroadcast()\0"
    "readBroadcast()\0"
};

void nmc::DkLANUdpSocket::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkLANUdpSocket *_t = static_cast<DkLANUdpSocket *>(_o);
        switch (_id) {
        case 0: _t->udpSocketNewServerOnline((*reinterpret_cast< QHostAddress(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 1: _t->sendBroadcast(); break;
        case 2: _t->sendNewClientBroadcast(); break;
        case 3: _t->readBroadcast(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkLANUdpSocket::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkLANUdpSocket::staticMetaObject = {
    { &QUdpSocket::staticMetaObject, qt_meta_stringdata_nmc__DkLANUdpSocket,
      qt_meta_data_nmc__DkLANUdpSocket, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkLANUdpSocket::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkLANUdpSocket::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkLANUdpSocket::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkLANUdpSocket))
        return static_cast<void*>(const_cast< DkLANUdpSocket*>(this));
    return QUdpSocket::qt_metacast(_clname);
}

int nmc::DkLANUdpSocket::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QUdpSocket::qt_metacall(_c, _id, _a);
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
void nmc::DkLANUdpSocket::udpSocketNewServerOnline(QHostAddress _t1, quint16 _t2, QString _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkPeer[] = {

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
      13,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      34,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkPeer[] = {
    "nmc::DkPeer\0\0sendGoodByeMessage()\0"
    "timerTimeout()\0"
};

void nmc::DkPeer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkPeer *_t = static_cast<DkPeer *>(_o);
        switch (_id) {
        case 0: _t->sendGoodByeMessage(); break;
        case 1: _t->timerTimeout(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkPeer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkPeer::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__DkPeer,
      qt_meta_data_nmc__DkPeer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkPeer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkPeer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkPeer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkPeer))
        return static_cast<void*>(const_cast< DkPeer*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::DkPeer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::DkPeer::sendGoodByeMessage()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_nmc__DkManagerThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x05,
      48,   21,   21,   21, 0x05,
      72,   21,   21,   21, 0x05,
     100,   21,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
     128,  121,   21,   21, 0x0a,
     153,  121,   21,   21, 0x0a,
     182,   21,   21,   21, 0x0a,
     201,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkManagerThread[] = {
    "nmc::DkManagerThread\0\0clientInitializedSignal()\0"
    "syncWithSignal(quint16)\0"
    "stopSyncWithSignal(quint16)\0"
    "goodByeToAllSignal()\0peerId\0"
    "synchronizeWith(quint16)\0"
    "stopSynchronizeWith(quint16)\0"
    "sendGoodByeToAll()\0quit()\0"
};

void nmc::DkManagerThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkManagerThread *_t = static_cast<DkManagerThread *>(_o);
        switch (_id) {
        case 0: _t->clientInitializedSignal(); break;
        case 1: _t->syncWithSignal((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 2: _t->stopSyncWithSignal((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 3: _t->goodByeToAllSignal(); break;
        case 4: _t->synchronizeWith((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 5: _t->stopSynchronizeWith((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 6: _t->sendGoodByeToAll(); break;
        case 7: _t->quit(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkManagerThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkManagerThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_nmc__DkManagerThread,
      qt_meta_data_nmc__DkManagerThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkManagerThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkManagerThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkManagerThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkManagerThread))
        return static_cast<void*>(const_cast< DkManagerThread*>(this));
    return QThread::qt_metacast(_clname);
}

int nmc::DkManagerThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::DkManagerThread::clientInitializedSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkManagerThread::syncWithSignal(quint16 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkManagerThread::stopSyncWithSignal(quint16 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void nmc::DkManagerThread::goodByeToAllSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}
static const uint qt_meta_data_nmc__DkLocalManagerThread[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkLocalManagerThread[] = {
    "nmc::DkLocalManagerThread\0"
};

void nmc::DkLocalManagerThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkLocalManagerThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkLocalManagerThread::staticMetaObject = {
    { &DkManagerThread::staticMetaObject, qt_meta_stringdata_nmc__DkLocalManagerThread,
      qt_meta_data_nmc__DkLocalManagerThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkLocalManagerThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkLocalManagerThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkLocalManagerThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkLocalManagerThread))
        return static_cast<void*>(const_cast< DkLocalManagerThread*>(this));
    return DkManagerThread::qt_metacast(_clname);
}

int nmc::DkLocalManagerThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkManagerThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkLanManagerThread[] = {

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
      31,   25,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
      55,   25,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkLanManagerThread[] = {
    "nmc::DkLanManagerThread\0\0start\0"
    "startServerSignal(bool)\0startServer(bool)\0"
};

void nmc::DkLanManagerThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkLanManagerThread *_t = static_cast<DkLanManagerThread *>(_o);
        switch (_id) {
        case 0: _t->startServerSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->startServer((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkLanManagerThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkLanManagerThread::staticMetaObject = {
    { &DkManagerThread::staticMetaObject, qt_meta_stringdata_nmc__DkLanManagerThread,
      qt_meta_data_nmc__DkLanManagerThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkLanManagerThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkLanManagerThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkLanManagerThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkLanManagerThread))
        return static_cast<void*>(const_cast< DkLanManagerThread*>(this));
    return DkManagerThread::qt_metacast(_clname);
}

int nmc::DkLanManagerThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkManagerThread::qt_metacall(_c, _id, _a);
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
void nmc::DkLanManagerThread::startServerSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkRCManagerThread[] = {

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
      29,   24,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
      48,   24,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkRCManagerThread[] = {
    "nmc::DkRCManagerThread\0\0mode\0"
    "newModeSignal(int)\0sendNewMode(int)\0"
};

void nmc::DkRCManagerThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkRCManagerThread *_t = static_cast<DkRCManagerThread *>(_o);
        switch (_id) {
        case 0: _t->newModeSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sendNewMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkRCManagerThread::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkRCManagerThread::staticMetaObject = {
    { &DkLanManagerThread::staticMetaObject, qt_meta_stringdata_nmc__DkRCManagerThread,
      qt_meta_data_nmc__DkRCManagerThread, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkRCManagerThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkRCManagerThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkRCManagerThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkRCManagerThread))
        return static_cast<void*>(const_cast< DkRCManagerThread*>(this));
    return DkLanManagerThread::qt_metacast(_clname);
}

int nmc::DkRCManagerThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkLanManagerThread::qt_metacall(_c, _id, _a);
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
void nmc::DkRCManagerThread::newModeSignal(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkUpdater[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   16,   15,   15, 0x05,
      63,   16,   15,   15, 0x05,
     108,   99,   15,   15, 0x05,
     136,  134,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
     168,   15,   15,   15, 0x0a,
     186,   15,   15,   15, 0x0a,
     216,   15,   15,   15, 0x0a,
     256,   15,   15,   15, 0x0a,
     277,  272,   15,   15, 0x0a,
     329,  314,   15,   15, 0x0a,
     367,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkUpdater[] = {
    "nmc::DkUpdater\0\0msg,title\0"
    "displayUpdateDialog(QString,QString)\0"
    "showUpdaterMessage(QString,QString)\0"
    "filePath\0downloadFinished(QString)\0,\0"
    "downloadProgress(qint64,qint64)\0"
    "checkForUpdates()\0replyFinished(QNetworkReply*)\0"
    "replyError(QNetworkReply::NetworkError)\0"
    "performUpdate()\0data\0"
    "downloadFinishedSlot(QNetworkReply*)\0"
    "received,total\0updateDownloadProgress(qint64,qint64)\0"
    "cancelUpdate()\0"
};

void nmc::DkUpdater::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkUpdater *_t = static_cast<DkUpdater *>(_o);
        switch (_id) {
        case 0: _t->displayUpdateDialog((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 1: _t->showUpdaterMessage((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->downloadFinished((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->downloadProgress((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        case 4: _t->checkForUpdates(); break;
        case 5: _t->replyFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 6: _t->replyError((*reinterpret_cast< QNetworkReply::NetworkError(*)>(_a[1]))); break;
        case 7: _t->performUpdate(); break;
        case 8: _t->downloadFinishedSlot((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 9: _t->updateDownloadProgress((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        case 10: _t->cancelUpdate(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkUpdater::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkUpdater::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__DkUpdater,
      qt_meta_data_nmc__DkUpdater, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkUpdater::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkUpdater::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkUpdater::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkUpdater))
        return static_cast<void*>(const_cast< DkUpdater*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::DkUpdater::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkUpdater::displayUpdateDialog(QString _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkUpdater::showUpdaterMessage(QString _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkUpdater::downloadFinished(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void nmc::DkUpdater::downloadProgress(qint64 _t1, qint64 _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
static const uint qt_meta_data_nmc__DkTranslationUpdater[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x05,
      50,   48,   26,   26, 0x05,

 // slots: signature, parameters, type, tag, flags
      86,   26,   26,   26, 0x0a,
     104,   26,   26,   26, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkTranslationUpdater[] = {
    "nmc::DkTranslationUpdater\0\0"
    "translationUpdated()\0,\0"
    "showUpdaterMessage(QString,QString)\0"
    "checkForUpdates()\0replyFinished(QNetworkReply*)\0"
};

void nmc::DkTranslationUpdater::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkTranslationUpdater *_t = static_cast<DkTranslationUpdater *>(_o);
        switch (_id) {
        case 0: _t->translationUpdated(); break;
        case 1: _t->showUpdaterMessage((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->checkForUpdates(); break;
        case 3: _t->replyFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkTranslationUpdater::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkTranslationUpdater::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__DkTranslationUpdater,
      qt_meta_data_nmc__DkTranslationUpdater, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkTranslationUpdater::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkTranslationUpdater::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkTranslationUpdater::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkTranslationUpdater))
        return static_cast<void*>(const_cast< DkTranslationUpdater*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::DkTranslationUpdater::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void nmc::DkTranslationUpdater::translationUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkTranslationUpdater::showUpdaterMessage(QString _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_nmc__FileDownloader[] = {

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
      21,   20,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
      41,   34,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__FileDownloader[] = {
    "nmc::FileDownloader\0\0downloaded()\0"
    "pReply\0fileDownloaded(QNetworkReply*)\0"
};

void nmc::FileDownloader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FileDownloader *_t = static_cast<FileDownloader *>(_o);
        switch (_id) {
        case 0: _t->downloaded(); break;
        case 1: _t->fileDownloaded((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::FileDownloader::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::FileDownloader::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__FileDownloader,
      qt_meta_data_nmc__FileDownloader, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::FileDownloader::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::FileDownloader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::FileDownloader::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__FileDownloader))
        return static_cast<void*>(const_cast< FileDownloader*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::FileDownloader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::FileDownloader::downloaded()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
