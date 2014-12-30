/****************************************************************************
** Meta object code from reading C++ file 'DkToolbars.h'
**
** Created: Thu Dec 18 00:26:17 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkToolbars.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkToolbars.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkColorSlider[] = {

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
      42,   20,   19,   19, 0x05,
      85,   78,   19,   19, 0x05,
     124,  117,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
     159,  153,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkColorSlider[] = {
    "nmc::DkColorSlider\0\0sender,dragDistX,yPos\0"
    "sliderMoved(DkColorSlider*,int,int)\0"
    "sender\0sliderActivated(DkColorSlider*)\0"
    "slider\0colorChanged(DkColorSlider*)\0"
    "event\0paintEvent(QPaintEvent*)\0"
};

void nmc::DkColorSlider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkColorSlider *_t = static_cast<DkColorSlider *>(_o);
        switch (_id) {
        case 0: _t->sliderMoved((*reinterpret_cast< DkColorSlider*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->sliderActivated((*reinterpret_cast< DkColorSlider*(*)>(_a[1]))); break;
        case 2: _t->colorChanged((*reinterpret_cast< DkColorSlider*(*)>(_a[1]))); break;
        case 3: _t->paintEvent((*reinterpret_cast< QPaintEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkColorSlider::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkColorSlider::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkColorSlider,
      qt_meta_data_nmc__DkColorSlider, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkColorSlider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkColorSlider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkColorSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkColorSlider))
        return static_cast<void*>(const_cast< DkColorSlider*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkColorSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkColorSlider::sliderMoved(DkColorSlider * _t1, int _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkColorSlider::sliderActivated(DkColorSlider * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkColorSlider::colorChanged(DkColorSlider * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_nmc__DkGradient[] = {

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
      17,   16,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
      41,   35,   16,   16, 0x0a,
      88,   66,   16,   16, 0x0a,
     130,  123,   16,   16, 0x0a,
     165,  158,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkGradient[] = {
    "nmc::DkGradient\0\0gradientChanged()\0"
    "event\0paintEvent(QPaintEvent*)\0"
    "sender,dragDistX,yPos\0"
    "moveSlider(DkColorSlider*,int,int)\0"
    "slider\0changeColor(DkColorSlider*)\0"
    "sender\0activateSlider(DkColorSlider*)\0"
};

void nmc::DkGradient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkGradient *_t = static_cast<DkGradient *>(_o);
        switch (_id) {
        case 0: _t->gradientChanged(); break;
        case 1: _t->paintEvent((*reinterpret_cast< QPaintEvent*(*)>(_a[1]))); break;
        case 2: _t->moveSlider((*reinterpret_cast< DkColorSlider*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 3: _t->changeColor((*reinterpret_cast< DkColorSlider*(*)>(_a[1]))); break;
        case 4: _t->activateSlider((*reinterpret_cast< DkColorSlider*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkGradient::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkGradient::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkGradient,
      qt_meta_data_nmc__DkGradient, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkGradient::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkGradient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkGradient::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkGradient))
        return static_cast<void*>(const_cast< DkGradient*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkGradient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkGradient::gradientChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_nmc__DkTransferToolBar[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x05,
      49,   43,   23,   23, 0x05,
      91,   83,   23,   23, 0x05,
     125,  111,   23,   23, 0x05,
     169,   23,   23,   23, 0x05,
     185,   23,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
     209,  203,   23,   23, 0x0a,
     238,  234,   23,   23, 0x0a,
     263,  258,   23,   23, 0x0a,
     281,   23,   23,   23, 0x0a,
     296,  234,   23,   23, 0x0a,
     323,   23,   23,   23, 0x0a,
     340,   23,   23,   23, 0x08,
     350,   23,   23,   23, 0x08,
     368,  362,   23,   23, 0x08,
     393,  387,   23,   23, 0x08,
     422,   23,   23,   23, 0x08,
     434,  430,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkTransferToolBar[] = {
    "nmc::DkTransferToolBar\0\0pickColorRequest()\0"
    "stops\0colorTableChanged(QGradientStops)\0"
    "channel\0channelChanged(int)\0channel,stops\0"
    "transferFunctionChanged(int,QGradientStops)\0"
    "tFEnabled(bool)\0gradientChanged()\0"
    "event\0paintEvent(QPaintEvent*)\0pos\0"
    "insertSlider(qreal)\0mode\0setImageMode(int)\0"
    "saveGradient()\0deleteGradientMenu(QPoint)\0"
    "deleteGradient()\0applyTF()\0pickColor()\0"
    "index\0changeChannel(int)\0state\0"
    "enableTFCheckBoxClicked(int)\0reset()\0"
    "idx\0switchGradient(int)\0"
};

void nmc::DkTransferToolBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkTransferToolBar *_t = static_cast<DkTransferToolBar *>(_o);
        switch (_id) {
        case 0: _t->pickColorRequest(); break;
        case 1: _t->colorTableChanged((*reinterpret_cast< QGradientStops(*)>(_a[1]))); break;
        case 2: _t->channelChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->transferFunctionChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QGradientStops(*)>(_a[2]))); break;
        case 4: _t->tFEnabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->gradientChanged(); break;
        case 6: _t->paintEvent((*reinterpret_cast< QPaintEvent*(*)>(_a[1]))); break;
        case 7: _t->insertSlider((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 8: _t->setImageMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->saveGradient(); break;
        case 10: _t->deleteGradientMenu((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 11: _t->deleteGradient(); break;
        case 12: _t->applyTF(); break;
        case 13: _t->pickColor(); break;
        case 14: _t->changeChannel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->enableTFCheckBoxClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->reset(); break;
        case 17: _t->switchGradient((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkTransferToolBar::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkTransferToolBar::staticMetaObject = {
    { &QToolBar::staticMetaObject, qt_meta_stringdata_nmc__DkTransferToolBar,
      qt_meta_data_nmc__DkTransferToolBar, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkTransferToolBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkTransferToolBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkTransferToolBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkTransferToolBar))
        return static_cast<void*>(const_cast< DkTransferToolBar*>(this));
    return QToolBar::qt_metacast(_clname);
}

int nmc::DkTransferToolBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QToolBar::qt_metacall(_c, _id, _a);
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
void nmc::DkTransferToolBar::pickColorRequest()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkTransferToolBar::colorTableChanged(QGradientStops _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkTransferToolBar::channelChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void nmc::DkTransferToolBar::transferFunctionChanged(int _t1, QGradientStops _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void nmc::DkTransferToolBar::tFEnabled(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void nmc::DkTransferToolBar::gradientChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}
static const uint qt_meta_data_nmc__DkCropToolBar[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   20,   19,   19, 0x05,
      44,   19,   19,   19, 0x05,
      57,   19,   19,   19, 0x05,
      77,   72,   19,   19, 0x05,
     105,   99,   19,   19, 0x05,
     131,  125,   19,   19, 0x05,
     161,  151,   19,   19, 0x05,
     183,  176,   19,   19, 0x05,
     206,  201,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
     228,  221,   19,   19, 0x0a,
     252,   19,   19,   19, 0x0a,
     278,   19,   19,   19, 0x0a,
     306,   19,   19,   19, 0x0a,
     337,  332,   19,   19, 0x0a,
     382,  378,   19,   19, 0x0a,
     423,  419,   19,   19, 0x0a,
     457,  419,   19,   19, 0x0a,
     491,  419,   19,   19, 0x0a,
     524,   19,   19,   19, 0x0a,
     549,   20,   19,   19, 0x0a,
     576,   20,   19,   19, 0x0a,
     606,   20,   19,   19, 0x0a,
     634,  419,   19,   19, 0x0a,
     663,  655,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkCropToolBar[] = {
    "nmc::DkCropToolBar\0\0checked\0panSignal(bool)\0"
    "cropSignal()\0cancelSignal()\0diag\0"
    "aspectRatio(DkVector)\0angle\0"
    "angleSignal(double)\0brush\0colorSignal(QBrush)\0"
    "paintMode\0paintHint(int)\0invert\0"
    "shadingHint(bool)\0show\0showInfo(bool)\0"
    "aRatio\0setAspectRatio(QPointF)\0"
    "on_cropAction_triggered()\0"
    "on_cancelAction_triggered()\0"
    "on_swapAction_triggered()\0text\0"
    "on_ratioBox_currentIndexChanged(QString)\0"
    "idx\0on_guideBox_currentIndexChanged(int)\0"
    "val\0on_horValBox_valueChanged(double)\0"
    "on_verValBox_valueChanged(double)\0"
    "on_angleBox_valueChanged(double)\0"
    "on_bgColButton_clicked()\0"
    "on_panAction_toggled(bool)\0"
    "on_invertAction_toggled(bool)\0"
    "on_infoAction_toggled(bool)\0"
    "angleChanged(double)\0visible\0"
    "setVisible(bool)\0"
};

void nmc::DkCropToolBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkCropToolBar *_t = static_cast<DkCropToolBar *>(_o);
        switch (_id) {
        case 0: _t->panSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->cropSignal(); break;
        case 2: _t->cancelSignal(); break;
        case 3: _t->aspectRatio((*reinterpret_cast< const DkVector(*)>(_a[1]))); break;
        case 4: _t->angleSignal((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->colorSignal((*reinterpret_cast< const QBrush(*)>(_a[1]))); break;
        case 6: _t->paintHint((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->shadingHint((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->showInfo((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->setAspectRatio((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 10: _t->on_cropAction_triggered(); break;
        case 11: _t->on_cancelAction_triggered(); break;
        case 12: _t->on_swapAction_triggered(); break;
        case 13: _t->on_ratioBox_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->on_guideBox_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->on_horValBox_valueChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 16: _t->on_verValBox_valueChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 17: _t->on_angleBox_valueChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 18: _t->on_bgColButton_clicked(); break;
        case 19: _t->on_panAction_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 20: _t->on_invertAction_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 21: _t->on_infoAction_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 22: _t->angleChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 23: _t->setVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkCropToolBar::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkCropToolBar::staticMetaObject = {
    { &QToolBar::staticMetaObject, qt_meta_stringdata_nmc__DkCropToolBar,
      qt_meta_data_nmc__DkCropToolBar, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkCropToolBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkCropToolBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkCropToolBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkCropToolBar))
        return static_cast<void*>(const_cast< DkCropToolBar*>(this));
    return QToolBar::qt_metacast(_clname);
}

int nmc::DkCropToolBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QToolBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkCropToolBar::panSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkCropToolBar::cropSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void nmc::DkCropToolBar::cancelSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void nmc::DkCropToolBar::aspectRatio(const DkVector & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void nmc::DkCropToolBar::angleSignal(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void nmc::DkCropToolBar::colorSignal(const QBrush & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void nmc::DkCropToolBar::paintHint(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void nmc::DkCropToolBar::shadingHint(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void nmc::DkCropToolBar::showInfo(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_END_MOC_NAMESPACE
