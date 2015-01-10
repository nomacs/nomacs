/****************************************************************************
** Meta object code from reading C++ file 'DkConnection.h'
**
** Created: Thu Dec 18 00:26:16 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkConnection.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkConnection.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkConnection[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: signature, parameters, type, tag, flags
      51,   19,   18,   18, 0x05,
     146,  104,   18,   18, 0x05,
     214,  203,   18,   18, 0x05,
     275,  255,   18,   18, 0x05,
     361,  324,   18,   18, 0x05,
     459,  414,   18,   18, 0x05,
     550,  527,   18,   18, 0x05,
     598,  203,   18,   18, 0x05,
     646,  631,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
     709,  697,   18,   18, 0x0a,
     738,   18,   18,   18, 0x0a,
     768,   18,   18,   18, 0x0a,
     806,  797,   18,   18, 0x0a,
     861,  835,   18,   18, 0x0a,
     935,  901,   18,   18, 0x0a,
    1002,  990,   18,   18, 0x0a,
    1037,   18,   18,   18, 0x0a,
    1069, 1061,   18,   18, 0x0a,
    1114,   18,   18,   18, 0x09,
    1133,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkConnection[] = {
    "nmc::DkConnection\0\0peerServerPort,title,connection\0"
    "connectionReadyForUse(quint16,QString,DkConnection*)\0"
    "synchronizedPeersOfOtherClient,connection\0"
    "connectionStartSynchronize(QList<quint16>,DkConnection*)\0"
    "connection\0connectionStopSynchronize(DkConnection*)\0"
    "connection,newTitle\0"
    "connectionTitleHasChanged(DkConnection*,QString)\0"
    "connection,position,opacity,overlaid\0"
    "connectionNewPosition(DkConnection*,QRect,bool,bool)\0"
    "connection,transform,imgTransform,canvasSize\0"
    "connectionNewTransform(DkConnection*,QTransform,QTransform,QPointF)\0"
    "connection,op,filename\0"
    "connectionNewFile(DkConnection*,qint16,QString)\0"
    "connectionGoodBye(DkConnection*)\0"
    "connection,msg\0"
    "connectionShowStatusMessage(DkConnection*,QString)\0"
    "currenTitle\0sendGreetingMessage(QString)\0"
    "sendStartSynchronizeMessage()\0"
    "sendStopSynchronizeMessage()\0newtitle\0"
    "sendNewTitleMessage(QString)\0"
    "position,opacity,overlaid\0"
    "sendNewPositionMessage(QRect,bool,bool)\0"
    "transform,imgTransform,canvasSize\0"
    "sendNewTransformMessage(QTransform,QTransform,QPointF)\0"
    "op,filename\0sendNewFileMessage(qint16,QString)\0"
    "sendNewGoodbyeMessage()\0newList\0"
    "synchronizedPeersListChanged(QList<quint16>)\0"
    "processReadyRead()\0synchronizedTimerTimeout()\0"
};

void nmc::DkConnection::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkConnection *_t = static_cast<DkConnection *>(_o);
        switch (_id) {
        case 0: _t->connectionReadyForUse((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< DkConnection*(*)>(_a[3]))); break;
        case 1: _t->connectionStartSynchronize((*reinterpret_cast< QList<quint16>(*)>(_a[1])),(*reinterpret_cast< DkConnection*(*)>(_a[2]))); break;
        case 2: _t->connectionStopSynchronize((*reinterpret_cast< DkConnection*(*)>(_a[1]))); break;
        case 3: _t->connectionTitleHasChanged((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 4: _t->connectionNewPosition((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QRect(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 5: _t->connectionNewTransform((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QTransform(*)>(_a[2])),(*reinterpret_cast< QTransform(*)>(_a[3])),(*reinterpret_cast< QPointF(*)>(_a[4]))); break;
        case 6: _t->connectionNewFile((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< qint16(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 7: _t->connectionGoodBye((*reinterpret_cast< DkConnection*(*)>(_a[1]))); break;
        case 8: _t->connectionShowStatusMessage((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 9: _t->sendGreetingMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 10: _t->sendStartSynchronizeMessage(); break;
        case 11: _t->sendStopSynchronizeMessage(); break;
        case 12: _t->sendNewTitleMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 13: _t->sendNewPositionMessage((*reinterpret_cast< QRect(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 14: _t->sendNewTransformMessage((*reinterpret_cast< QTransform(*)>(_a[1])),(*reinterpret_cast< QTransform(*)>(_a[2])),(*reinterpret_cast< QPointF(*)>(_a[3]))); break;
        case 15: _t->sendNewFileMessage((*reinterpret_cast< qint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 16: _t->sendNewGoodbyeMessage(); break;
        case 17: _t->synchronizedPeersListChanged((*reinterpret_cast< QList<quint16>(*)>(_a[1]))); break;
        case 18: _t->processReadyRead(); break;
        case 19: _t->synchronizedTimerTimeout(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkConnection::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkConnection::staticMetaObject = {
    { &QTcpSocket::staticMetaObject, qt_meta_stringdata_nmc__DkConnection,
      qt_meta_data_nmc__DkConnection, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkConnection::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkConnection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkConnection::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkConnection))
        return static_cast<void*>(const_cast< DkConnection*>(this));
    return QTcpSocket::qt_metacast(_clname);
}

int nmc::DkConnection::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTcpSocket::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkConnection::connectionReadyForUse(quint16 _t1, QString _t2, DkConnection * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkConnection::connectionStartSynchronize(QList<quint16> _t1, DkConnection * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkConnection::connectionStopSynchronize(DkConnection * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void nmc::DkConnection::connectionTitleHasChanged(DkConnection * _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void nmc::DkConnection::connectionNewPosition(DkConnection * _t1, QRect _t2, bool _t3, bool _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void nmc::DkConnection::connectionNewTransform(DkConnection * _t1, QTransform _t2, QTransform _t3, QPointF _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void nmc::DkConnection::connectionNewFile(DkConnection * _t1, qint16 _t2, QString _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void nmc::DkConnection::connectionGoodBye(DkConnection * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void nmc::DkConnection::connectionShowStatusMessage(DkConnection * _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
static const uint qt_meta_data_nmc__DkLocalConnection[] = {

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
      24,   23,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
      49,   23,   23,   23, 0x09,
      68,   23,   23,   23, 0x09,
      82,   23,   23,   23, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkLocalConnection[] = {
    "nmc::DkLocalConnection\0\0"
    "connectionQuitReceived()\0processReadyRead()\0"
    "processData()\0sendQuitMessage()\0"
};

void nmc::DkLocalConnection::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkLocalConnection *_t = static_cast<DkLocalConnection *>(_o);
        switch (_id) {
        case 0: _t->connectionQuitReceived(); break;
        case 1: _t->processReadyRead(); break;
        case 2: _t->processData(); break;
        case 3: _t->sendQuitMessage(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkLocalConnection::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkLocalConnection::staticMetaObject = {
    { &DkConnection::staticMetaObject, qt_meta_stringdata_nmc__DkLocalConnection,
      qt_meta_data_nmc__DkLocalConnection, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkLocalConnection::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkLocalConnection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkLocalConnection::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkLocalConnection))
        return static_cast<void*>(const_cast< DkLocalConnection*>(this));
    return DkConnection::qt_metacast(_clname);
}

int nmc::DkLocalConnection::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkConnection::qt_metacall(_c, _id, _a);
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
void nmc::DkLocalConnection::connectionQuitReceived()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_nmc__DkLANConnection[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      45,   22,   21,   21, 0x05,
     116,   94,   21,   21, 0x05,
     187,  163,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
     246,   21,   21,   21, 0x09,
     277,  265,   21,   21, 0x0a,
     324,  313,   21,   21, 0x0a,
     387,  361,   21,   21, 0x0a,
     461,  427,   21,   21, 0x0a,
     528,  516,   21,   21, 0x0a,
     576,  563,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkLANConnection[] = {
    "nmc::DkLANConnection\0\0connection,image,title\0"
    "connectionNewImage(DkConnection*,QImage,QString)\0"
    "connection,imageTitle\0"
    "connectionUpcomingImage(DkConnection*,QString)\0"
    "connection,address,port\0"
    "connectionSwitchServer(DkConnection*,QHostAddress,quint16)\0"
    "processReadyRead()\0image,title\0"
    "sendNewImageMessage(QImage,QString)\0"
    "imageTitle\0sendNewUpcomingImageMessage(QString)\0"
    "position,opacity,overlaid\0"
    "sendNewPositionMessage(QRect,bool,bool)\0"
    "transform,imgTransform,canvasSize\0"
    "sendNewTransformMessage(QTransform,QTransform,QPointF)\0"
    "op,filename\0sendNewFileMessage(qint16,QString)\0"
    "address,port\0sendSwitchServerMessage(QHostAddress,quint16)\0"
};

void nmc::DkLANConnection::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkLANConnection *_t = static_cast<DkLANConnection *>(_o);
        switch (_id) {
        case 0: _t->connectionNewImage((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QImage(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 1: _t->connectionUpcomingImage((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->connectionSwitchServer((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< QHostAddress(*)>(_a[2])),(*reinterpret_cast< quint16(*)>(_a[3]))); break;
        case 3: _t->processReadyRead(); break;
        case 4: _t->sendNewImageMessage((*reinterpret_cast< QImage(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 5: _t->sendNewUpcomingImageMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: _t->sendNewPositionMessage((*reinterpret_cast< QRect(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 7: _t->sendNewTransformMessage((*reinterpret_cast< QTransform(*)>(_a[1])),(*reinterpret_cast< QTransform(*)>(_a[2])),(*reinterpret_cast< QPointF(*)>(_a[3]))); break;
        case 8: _t->sendNewFileMessage((*reinterpret_cast< qint16(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 9: _t->sendSwitchServerMessage((*reinterpret_cast< QHostAddress(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkLANConnection::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkLANConnection::staticMetaObject = {
    { &DkConnection::staticMetaObject, qt_meta_stringdata_nmc__DkLANConnection,
      qt_meta_data_nmc__DkLANConnection, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkLANConnection::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkLANConnection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkLANConnection::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkLANConnection))
        return static_cast<void*>(const_cast< DkLANConnection*>(this));
    return DkConnection::qt_metacast(_clname);
}

int nmc::DkLANConnection::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkConnection::qt_metacall(_c, _id, _a);
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
void nmc::DkLANConnection::connectionNewImage(DkConnection * _t1, QImage _t2, QString _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkLANConnection::connectionUpcomingImage(DkConnection * _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkLANConnection::connectionSwitchServer(DkConnection * _t1, QHostAddress _t2, quint16 _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_nmc__DkRCConnection[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      23,   21,   20,   20, 0x05,
      67,   21,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
     106,   20,   20,   20, 0x0a,
     129,   20,   20,   20, 0x0a,
     151,  146,   20,   20, 0x0a,
     167,   20,   20,   20, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkRCConnection[] = {
    "nmc::DkRCConnection\0\0,\0"
    "connectionNewPermission(DkConnection*,bool)\0"
    "connectionNewRCType(DkConnection*,int)\0"
    "sendAskForPermission()\0sendPermission()\0"
    "type\0sendRCType(int)\0processReadyRead()\0"
};

void nmc::DkRCConnection::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkRCConnection *_t = static_cast<DkRCConnection *>(_o);
        switch (_id) {
        case 0: _t->connectionNewPermission((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->connectionNewRCType((*reinterpret_cast< DkConnection*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->sendAskForPermission(); break;
        case 3: _t->sendPermission(); break;
        case 4: _t->sendRCType((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->processReadyRead(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkRCConnection::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkRCConnection::staticMetaObject = {
    { &DkLANConnection::staticMetaObject, qt_meta_stringdata_nmc__DkRCConnection,
      qt_meta_data_nmc__DkRCConnection, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkRCConnection::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkRCConnection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkRCConnection::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkRCConnection))
        return static_cast<void*>(const_cast< DkRCConnection*>(this));
    return DkLANConnection::qt_metacast(_clname);
}

int nmc::DkRCConnection::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkLANConnection::qt_metacall(_c, _id, _a);
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
void nmc::DkRCConnection::connectionNewPermission(DkConnection * _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkRCConnection::connectionNewRCType(DkConnection * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
