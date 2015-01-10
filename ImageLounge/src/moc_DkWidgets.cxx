/****************************************************************************
** Meta object code from reading C++ file 'DkWidgets.h'
**
** Created: Thu Dec 18 00:26:16 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkWidgets.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkWidgets.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      23,   15,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      43,   14,   14,   14, 0x0a,
      50,   14,   14,   14, 0x0a,
      57,   15,   14,   14, 0x0a,
      74,   14,   14,   14, 0x0a,
      93,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkWidget[] = {
    "nmc::DkWidget\0\0visible\0visibleSignal(bool)\0"
    "show()\0hide()\0setVisible(bool)\0"
    "animateOpacityUp()\0animateOpacityDown()\0"
};

void nmc::DkWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkWidget *_t = static_cast<DkWidget *>(_o);
        switch (_id) {
        case 0: _t->visibleSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->show(); break;
        case 2: _t->hide(); break;
        case 3: _t->setVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->animateOpacityUp(); break;
        case 5: _t->animateOpacityDown(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkWidget,
      qt_meta_data_nmc__DkWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkWidget))
        return static_cast<void*>(const_cast< DkWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::DkWidget::visibleSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkLabel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkLabel[] = {
    "nmc::DkLabel\0\0hide()\0"
};

void nmc::DkLabel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkLabel *_t = static_cast<DkLabel *>(_o);
        switch (_id) {
        case 0: _t->hide(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkLabel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkLabel::staticMetaObject = {
    { &QLabel::staticMetaObject, qt_meta_stringdata_nmc__DkLabel,
      qt_meta_data_nmc__DkLabel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkLabel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkLabel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkLabel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkLabel))
        return static_cast<void*>(const_cast< DkLabel*>(this));
    return QLabel::qt_metacast(_clname);
}

int nmc::DkLabel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkLabelBg[] = {

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

static const char qt_meta_stringdata_nmc__DkLabelBg[] = {
    "nmc::DkLabelBg\0"
};

void nmc::DkLabelBg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkLabelBg::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkLabelBg::staticMetaObject = {
    { &DkLabel::staticMetaObject, qt_meta_stringdata_nmc__DkLabelBg,
      qt_meta_data_nmc__DkLabelBg, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkLabelBg::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkLabelBg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkLabelBg::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkLabelBg))
        return static_cast<void*>(const_cast< DkLabelBg*>(this));
    return DkLabel::qt_metacast(_clname);
}

int nmc::DkLabelBg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkGradientLabel[] = {

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

static const char qt_meta_stringdata_nmc__DkGradientLabel[] = {
    "nmc::DkGradientLabel\0"
};

void nmc::DkGradientLabel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkGradientLabel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkGradientLabel::staticMetaObject = {
    { &DkLabel::staticMetaObject, qt_meta_stringdata_nmc__DkGradientLabel,
      qt_meta_data_nmc__DkGradientLabel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkGradientLabel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkGradientLabel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkGradientLabel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkGradientLabel))
        return static_cast<void*>(const_cast< DkGradientLabel*>(this));
    return DkLabel::qt_metacast(_clname);
}

int nmc::DkGradientLabel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkFadeLabel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   18,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
      46,   17,   17,   17, 0x0a,
      53,   17,   17,   17, 0x0a,
      60,   18,   17,   17, 0x0a,
      77,   17,   17,   17, 0x09,
      96,   17,   17,   17, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkFadeLabel[] = {
    "nmc::DkFadeLabel\0\0visible\0visibleSignal(bool)\0"
    "show()\0hide()\0setVisible(bool)\0"
    "animateOpacityUp()\0animateOpacityDown()\0"
};

void nmc::DkFadeLabel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkFadeLabel *_t = static_cast<DkFadeLabel *>(_o);
        switch (_id) {
        case 0: _t->visibleSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->show(); break;
        case 2: _t->hide(); break;
        case 3: _t->setVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->animateOpacityUp(); break;
        case 5: _t->animateOpacityDown(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkFadeLabel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkFadeLabel::staticMetaObject = {
    { &DkLabel::staticMetaObject, qt_meta_stringdata_nmc__DkFadeLabel,
      qt_meta_data_nmc__DkFadeLabel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkFadeLabel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkFadeLabel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkFadeLabel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkFadeLabel))
        return static_cast<void*>(const_cast< DkFadeLabel*>(this));
    return DkLabel::qt_metacast(_clname);
}

int nmc::DkFadeLabel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkLabel::qt_metacall(_c, _id, _a);
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
void nmc::DkFadeLabel::visibleSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkButton[] = {

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

static const char qt_meta_stringdata_nmc__DkButton[] = {
    "nmc::DkButton\0"
};

void nmc::DkButton::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkButton::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkButton::staticMetaObject = {
    { &QPushButton::staticMetaObject, qt_meta_stringdata_nmc__DkButton,
      qt_meta_data_nmc__DkButton, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkButton::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkButton::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkButton))
        return static_cast<void*>(const_cast< DkButton*>(this));
    return QPushButton::qt_metacast(_clname);
}

int nmc::DkButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPushButton::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkRatingLabel[] = {

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
      27,   20,   19,   19, 0x05,
      48,   19,   19,   19, 0x25,

 // slots: signature, parameters, type, tag, flags
      66,   19,   19,   19, 0x0a,
      76,   19,   19,   19, 0x0a,
      86,   19,   19,   19, 0x0a,
      96,   19,   19,   19, 0x0a,
     106,   19,   19,   19, 0x0a,
     116,   19,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkRatingLabel[] = {
    "nmc::DkRatingLabel\0\0rating\0"
    "newRatingSignal(int)\0newRatingSignal()\0"
    "rating0()\0rating1()\0rating2()\0rating3()\0"
    "rating4()\0rating5()\0"
};

void nmc::DkRatingLabel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkRatingLabel *_t = static_cast<DkRatingLabel *>(_o);
        switch (_id) {
        case 0: _t->newRatingSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->newRatingSignal(); break;
        case 2: _t->rating0(); break;
        case 3: _t->rating1(); break;
        case 4: _t->rating2(); break;
        case 5: _t->rating3(); break;
        case 6: _t->rating4(); break;
        case 7: _t->rating5(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkRatingLabel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkRatingLabel::staticMetaObject = {
    { &DkWidget::staticMetaObject, qt_meta_stringdata_nmc__DkRatingLabel,
      qt_meta_data_nmc__DkRatingLabel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkRatingLabel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkRatingLabel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkRatingLabel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkRatingLabel))
        return static_cast<void*>(const_cast< DkRatingLabel*>(this));
    return DkWidget::qt_metacast(_clname);
}

int nmc::DkRatingLabel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkRatingLabel::newRatingSignal(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkRatingLabelBg[] = {

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

static const char qt_meta_stringdata_nmc__DkRatingLabelBg[] = {
    "nmc::DkRatingLabelBg\0"
};

void nmc::DkRatingLabelBg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkRatingLabelBg::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkRatingLabelBg::staticMetaObject = {
    { &DkRatingLabel::staticMetaObject, qt_meta_stringdata_nmc__DkRatingLabelBg,
      qt_meta_data_nmc__DkRatingLabelBg, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkRatingLabelBg::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkRatingLabelBg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkRatingLabelBg::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkRatingLabelBg))
        return static_cast<void*>(const_cast< DkRatingLabelBg*>(this));
    return DkRatingLabel::qt_metacast(_clname);
}

int nmc::DkRatingLabelBg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkRatingLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkFileInfoLabel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      30,   22,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkFileInfoLabel[] = {
    "nmc::DkFileInfoLabel\0\0visible\0"
    "setVisible(bool)\0"
};

void nmc::DkFileInfoLabel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkFileInfoLabel *_t = static_cast<DkFileInfoLabel *>(_o);
        switch (_id) {
        case 0: _t->setVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkFileInfoLabel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkFileInfoLabel::staticMetaObject = {
    { &DkFadeLabel::staticMetaObject, qt_meta_stringdata_nmc__DkFileInfoLabel,
      qt_meta_data_nmc__DkFileInfoLabel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkFileInfoLabel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkFileInfoLabel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkFileInfoLabel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkFileInfoLabel))
        return static_cast<void*>(const_cast< DkFileInfoLabel*>(this));
    return DkFadeLabel::qt_metacast(_clname);
}

int nmc::DkFileInfoLabel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkFadeLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkPlayer[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,
      28,   14,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      50,   45,   14,   14, 0x0a,
      61,   14,   14,   14, 0x0a,
      74,   14,   14,   14, 0x0a,
      87,   14,   14,   14, 0x0a,
      98,   14,   14,   14, 0x0a,
     105,   14,   14,   14, 0x0a,
     119,  116,   14,   14, 0x0a,
     129,   14,   14,   14, 0x2a,
     141,   14,  136,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkPlayer[] = {
    "nmc::DkPlayer\0\0nextSignal()\0"
    "previousSignal()\0play\0play(bool)\0"
    "togglePlay()\0startTimer()\0autoNext()\0"
    "next()\0previous()\0ms\0show(int)\0show()\0"
    "bool\0isPlaying()\0"
};

void nmc::DkPlayer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkPlayer *_t = static_cast<DkPlayer *>(_o);
        switch (_id) {
        case 0: _t->nextSignal(); break;
        case 1: _t->previousSignal(); break;
        case 2: _t->play((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->togglePlay(); break;
        case 4: _t->startTimer(); break;
        case 5: _t->autoNext(); break;
        case 6: _t->next(); break;
        case 7: _t->previous(); break;
        case 8: _t->show((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->show(); break;
        case 10: { bool _r = _t->isPlaying();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkPlayer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkPlayer::staticMetaObject = {
    { &DkWidget::staticMetaObject, qt_meta_stringdata_nmc__DkPlayer,
      qt_meta_data_nmc__DkPlayer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkPlayer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkPlayer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkPlayer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkPlayer))
        return static_cast<void*>(const_cast< DkPlayer*>(this));
    return DkWidget::qt_metacast(_clname);
}

int nmc::DkPlayer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkPlayer::nextSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkPlayer::previousSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
static const uint qt_meta_data_nmc__DkFolderScrollBar[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   24,   23,   23, 0x05,
      58,   50,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
      85,   78,   23,   23, 0x0a,
     145,  140,   23,   23, 0x0a,
     206,  191,   23,   23, 0x0a,
     243,   23,   23,   23, 0x0a,
     250,   23,   23,   23, 0x0a,
     257,   50,   23,   23, 0x0a,
     274,   23,   23,   23, 0x0a,
     293,   23,   23,   23, 0x0a,
     316,  314,   23,   23, 0x09,
     336,   23,   23,   23, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkFolderScrollBar[] = {
    "nmc::DkFolderScrollBar\0\0idx\0"
    "changeFileSignal(int)\0visible\0"
    "visibleSignal(bool)\0images\0"
    "updateDir(QVector<QSharedPointer<DkImageContainerT> >)\0"
    "imgC\0updateFile(QSharedPointer<DkImageContainerT>)\0"
    "colors,indexes\0update(QVector<QColor>,QVector<int>)\0"
    "show()\0hide()\0setVisible(bool)\0"
    "animateOpacityUp()\0animateOpacityDown()\0"
    "i\0emitFileSignal(int)\0colorUpdated()\0"
};

void nmc::DkFolderScrollBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkFolderScrollBar *_t = static_cast<DkFolderScrollBar *>(_o);
        switch (_id) {
        case 0: _t->changeFileSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->visibleSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->updateDir((*reinterpret_cast< QVector<QSharedPointer<DkImageContainerT> >(*)>(_a[1]))); break;
        case 3: _t->updateFile((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 4: _t->update((*reinterpret_cast< const QVector<QColor>(*)>(_a[1])),(*reinterpret_cast< const QVector<int>(*)>(_a[2]))); break;
        case 5: _t->show(); break;
        case 6: _t->hide(); break;
        case 7: _t->setVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->animateOpacityUp(); break;
        case 9: _t->animateOpacityDown(); break;
        case 10: _t->emitFileSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->colorUpdated(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkFolderScrollBar::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkFolderScrollBar::staticMetaObject = {
    { &QScrollBar::staticMetaObject, qt_meta_stringdata_nmc__DkFolderScrollBar,
      qt_meta_data_nmc__DkFolderScrollBar, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkFolderScrollBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkFolderScrollBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkFolderScrollBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkFolderScrollBar))
        return static_cast<void*>(const_cast< DkFolderScrollBar*>(this));
    return QScrollBar::qt_metacast(_clname);
}

int nmc::DkFolderScrollBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QScrollBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkFolderScrollBar::changeFileSignal(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkFolderScrollBar::visibleSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_nmc__DkThumbsSaver[] = {

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
      35,   20,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
      55,   19,   19,   19, 0x0a,
      77,   70,   19,   19, 0x0a,
      95,   19,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkThumbsSaver[] = {
    "nmc::DkThumbsSaver\0\0currentFileIdx\0"
    "numFilesSignal(int)\0stopProgress()\0"
    "loaded\0thumbLoaded(bool)\0loadNext()\0"
};

void nmc::DkThumbsSaver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkThumbsSaver *_t = static_cast<DkThumbsSaver *>(_o);
        switch (_id) {
        case 0: _t->numFilesSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->stopProgress(); break;
        case 2: _t->thumbLoaded((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->loadNext(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkThumbsSaver::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkThumbsSaver::staticMetaObject = {
    { &DkWidget::staticMetaObject, qt_meta_stringdata_nmc__DkThumbsSaver,
      qt_meta_data_nmc__DkThumbsSaver, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkThumbsSaver::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkThumbsSaver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkThumbsSaver::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkThumbsSaver))
        return static_cast<void*>(const_cast< DkThumbsSaver*>(this));
    return DkWidget::qt_metacast(_clname);
}

int nmc::DkThumbsSaver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::DkThumbsSaver::numFilesSignal(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkFileSystemModel[] = {

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

static const char qt_meta_stringdata_nmc__DkFileSystemModel[] = {
    "nmc::DkFileSystemModel\0"
};

void nmc::DkFileSystemModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkFileSystemModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkFileSystemModel::staticMetaObject = {
    { &QFileSystemModel::staticMetaObject, qt_meta_stringdata_nmc__DkFileSystemModel,
      qt_meta_data_nmc__DkFileSystemModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkFileSystemModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkFileSystemModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkFileSystemModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkFileSystemModel))
        return static_cast<void*>(const_cast< DkFileSystemModel*>(this));
    return QFileSystemModel::qt_metacast(_clname);
}

int nmc::DkFileSystemModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFileSystemModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkSortFileProxyModel[] = {

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

static const char qt_meta_stringdata_nmc__DkSortFileProxyModel[] = {
    "nmc::DkSortFileProxyModel\0"
};

void nmc::DkSortFileProxyModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkSortFileProxyModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkSortFileProxyModel::staticMetaObject = {
    { &QSortFilterProxyModel::staticMetaObject, qt_meta_stringdata_nmc__DkSortFileProxyModel,
      qt_meta_data_nmc__DkSortFileProxyModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkSortFileProxyModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkSortFileProxyModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkSortFileProxyModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkSortFileProxyModel))
        return static_cast<void*>(const_cast< DkSortFileProxyModel*>(this));
    return QSortFilterProxyModel::qt_metacast(_clname);
}

int nmc::DkSortFileProxyModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSortFilterProxyModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkExplorer[] = {

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
      26,   17,   16,   16, 0x05,
      50,   46,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
      69,   17,   16,   16, 0x0a,
     101,   95,   16,   16, 0x0a,
     131,  126,   16,   16, 0x0a,
     157,  148,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkExplorer[] = {
    "nmc::DkExplorer\0\0fileInfo\0openFile(QFileInfo)\0"
    "dir\0openDir(QFileInfo)\0setCurrentPath(QFileInfo)\0"
    "index\0fileClicked(QModelIndex)\0show\0"
    "showColumn(bool)\0editable\0setEditable(bool)\0"
};

void nmc::DkExplorer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkExplorer *_t = static_cast<DkExplorer *>(_o);
        switch (_id) {
        case 0: _t->openFile((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 1: _t->openDir((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 2: _t->setCurrentPath((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 3: _t->fileClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 4: _t->showColumn((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->setEditable((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkExplorer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkExplorer::staticMetaObject = {
    { &QDockWidget::staticMetaObject, qt_meta_stringdata_nmc__DkExplorer,
      qt_meta_data_nmc__DkExplorer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkExplorer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkExplorer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkExplorer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkExplorer))
        return static_cast<void*>(const_cast< DkExplorer*>(this));
    return QDockWidget::qt_metacast(_clname);
}

int nmc::DkExplorer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDockWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkExplorer::openFile(QFileInfo _t1)const
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(const_cast< nmc::DkExplorer *>(this), &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkExplorer::openDir(QFileInfo _t1)const
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(const_cast< nmc::DkExplorer *>(this), &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_nmc__DkOverview[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   17,   16,   16, 0x05,
      45,   16,   16,   16, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkOverview[] = {
    "nmc::DkOverview\0\0dxy\0moveViewSignal(QPointF)\0"
    "sendTransformSignal()\0"
};

void nmc::DkOverview::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkOverview *_t = static_cast<DkOverview *>(_o);
        switch (_id) {
        case 0: _t->moveViewSignal((*reinterpret_cast< QPointF(*)>(_a[1]))); break;
        case 1: _t->sendTransformSignal(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkOverview::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkOverview::staticMetaObject = {
    { &DkWidget::staticMetaObject, qt_meta_stringdata_nmc__DkOverview,
      qt_meta_data_nmc__DkOverview, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkOverview::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkOverview::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkOverview::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkOverview))
        return static_cast<void*>(const_cast< DkOverview*>(this));
    return DkWidget::qt_metacast(_clname);
}

int nmc::DkOverview::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkOverview::moveViewSignal(QPointF _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkOverview::sendTransformSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
static const uint qt_meta_data_nmc__DkTransformRect[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   22,   21,   21, 0x05,
      65,   21,   21,   21, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkTransformRect[] = {
    "nmc::DkTransformRect\0\0,,,\0"
    "ctrlMovedSignal(int,QPointF,bool,bool)\0"
    "updateDiagonal(int)\0"
};

void nmc::DkTransformRect::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkTransformRect *_t = static_cast<DkTransformRect *>(_o);
        switch (_id) {
        case 0: _t->ctrlMovedSignal((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QPointF(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 1: _t->updateDiagonal((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkTransformRect::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkTransformRect::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkTransformRect,
      qt_meta_data_nmc__DkTransformRect, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkTransformRect::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkTransformRect::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkTransformRect::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkTransformRect))
        return static_cast<void*>(const_cast< DkTransformRect*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkTransformRect::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkTransformRect::ctrlMovedSignal(int _t1, QPointF _t2, bool _t3, bool _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkTransformRect::updateDiagonal(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_nmc__DkEditableRect[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   21,   20,   20, 0x05,
      87,   78,   20,   20, 0x25,
     128,  122,   20,   20, 0x05,
     155,  148,   20,   20, 0x05,
     181,  177,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
     241,  207,   20,   20, 0x0a,
     299,  277,   20,   20, 0x2a,
     334,  330,   20,   20, 0x0a,
     359,  354,   20,   20, 0x0a,
     398,  386,   20,   20, 0x0a,
     420,  122,   20,   20, 0x2a,
     445,  437,   20,   20, 0x0a,
     472,  462,   20,   20, 0x0a,
     490,   20,   20,   20, 0x2a,
     512,  505,   20,   20, 0x0a,
     542,  533,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkEditableRect[] = {
    "nmc::DkEditableRect\0\0cropArea,bgCol\0"
    "enterPressedSignal(DkRotatingRect,QColor)\0"
    "cropArea\0enterPressedSignal(DkRotatingRect)\0"
    "angle\0angleSignal(double)\0aRatio\0"
    "aRatioSignal(QPointF)\0msg\0"
    "statusInfoSignal(QString)\0"
    "idx,point,isShiftDown,changeState\0"
    "updateCorner(int,QPointF,bool,bool)\0"
    "idx,point,isShiftDown\0"
    "updateCorner(int,QPointF,bool)\0idx\0"
    "updateDiagonal(int)\0diag\0"
    "setFixedDiagonal(DkVector)\0angle,apply\0"
    "setAngle(double,bool)\0setAngle(double)\0"
    "panning\0setPanning(bool)\0paintMode\0"
    "setPaintHint(int)\0setPaintHint()\0"
    "invert\0setShadingHint(bool)\0showInfo\0"
    "setShowInfo(bool)\0"
};

void nmc::DkEditableRect::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkEditableRect *_t = static_cast<DkEditableRect *>(_o);
        switch (_id) {
        case 0: _t->enterPressedSignal((*reinterpret_cast< DkRotatingRect(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 1: _t->enterPressedSignal((*reinterpret_cast< DkRotatingRect(*)>(_a[1]))); break;
        case 2: _t->angleSignal((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: _t->aRatioSignal((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 4: _t->statusInfoSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->updateCorner((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QPointF(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 6: _t->updateCorner((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QPointF(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 7: _t->updateDiagonal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->setFixedDiagonal((*reinterpret_cast< const DkVector(*)>(_a[1]))); break;
        case 9: _t->setAngle((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 10: _t->setAngle((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 11: _t->setPanning((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->setPaintHint((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->setPaintHint(); break;
        case 14: _t->setShadingHint((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->setShowInfo((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkEditableRect::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkEditableRect::staticMetaObject = {
    { &DkWidget::staticMetaObject, qt_meta_stringdata_nmc__DkEditableRect,
      qt_meta_data_nmc__DkEditableRect, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkEditableRect::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkEditableRect::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkEditableRect::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkEditableRect))
        return static_cast<void*>(const_cast< DkEditableRect*>(this));
    return DkWidget::qt_metacast(_clname);
}

int nmc::DkEditableRect::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkEditableRect::enterPressedSignal(DkRotatingRect _t1, const QColor & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 2
void nmc::DkEditableRect::angleSignal(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void nmc::DkEditableRect::aRatioSignal(const QPointF & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void nmc::DkEditableRect::statusInfoSignal(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
static const uint qt_meta_data_nmc__DkCropWidget[] = {

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
      19,   18,   18,   18, 0x05,
      47,   34,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      75,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkCropWidget[] = {
    "nmc::DkCropWidget\0\0cancelSignal()\0"
    "toolbar,show\0showToolbar(QToolBar*,bool)\0"
    "crop()\0"
};

void nmc::DkCropWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkCropWidget *_t = static_cast<DkCropWidget *>(_o);
        switch (_id) {
        case 0: _t->cancelSignal(); break;
        case 1: _t->showToolbar((*reinterpret_cast< QToolBar*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->crop(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkCropWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkCropWidget::staticMetaObject = {
    { &DkEditableRect::staticMetaObject, qt_meta_stringdata_nmc__DkCropWidget,
      qt_meta_data_nmc__DkCropWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkCropWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkCropWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkCropWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkCropWidget))
        return static_cast<void*>(const_cast< DkCropWidget*>(this));
    return DkEditableRect::qt_metacast(_clname);
}

int nmc::DkCropWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkEditableRect::qt_metacall(_c, _id, _a);
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
void nmc::DkCropWidget::cancelSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkCropWidget::showToolbar(QToolBar * _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_nmc__DkColorChooser[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x05,
      36,   20,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
      47,   20,   20,   20, 0x0a,
      72,   20,   20,   20, 0x0a,
      97,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkColorChooser[] = {
    "nmc::DkColorChooser\0\0resetClicked()\0"
    "accepted()\0on_resetButton_clicked()\0"
    "on_colorButton_clicked()\0"
    "on_colorDialog_accepted()\0"
};

void nmc::DkColorChooser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkColorChooser *_t = static_cast<DkColorChooser *>(_o);
        switch (_id) {
        case 0: _t->resetClicked(); break;
        case 1: _t->accepted(); break;
        case 2: _t->on_resetButton_clicked(); break;
        case 3: _t->on_colorButton_clicked(); break;
        case 4: _t->on_colorDialog_accepted(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkColorChooser::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkColorChooser::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkColorChooser,
      qt_meta_data_nmc__DkColorChooser, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkColorChooser::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkColorChooser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkColorChooser::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkColorChooser))
        return static_cast<void*>(const_cast< DkColorChooser*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkColorChooser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::DkColorChooser::resetClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkColorChooser::accepted()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
static const uint qt_meta_data_nmc__DkHistogram[] = {

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

static const char qt_meta_stringdata_nmc__DkHistogram[] = {
    "nmc::DkHistogram\0"
};

void nmc::DkHistogram::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkHistogram::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkHistogram::staticMetaObject = {
    { &DkWidget::staticMetaObject, qt_meta_stringdata_nmc__DkHistogram,
      qt_meta_data_nmc__DkHistogram, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkHistogram::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkHistogram::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkHistogram::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkHistogram))
        return static_cast<void*>(const_cast< DkHistogram*>(this));
    return DkWidget::qt_metacast(_clname);
}

int nmc::DkHistogram::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkSlider[] = {

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
      21,   15,   14,   14, 0x05,
      38,   15,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      56,   15,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkSlider[] = {
    "nmc::DkSlider\0\0value\0sliderMoved(int)\0"
    "valueChanged(int)\0setValue(int)\0"
};

void nmc::DkSlider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkSlider *_t = static_cast<DkSlider *>(_o);
        switch (_id) {
        case 0: _t->sliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->setValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkSlider::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkSlider::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkSlider,
      qt_meta_data_nmc__DkSlider, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkSlider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkSlider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkSlider))
        return static_cast<void*>(const_cast< DkSlider*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkSlider::sliderMoved(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkSlider::valueChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_nmc__DkFolderLabel[] = {

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
      20,   19,   19,   19, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkFolderLabel[] = {
    "nmc::DkFolderLabel\0\0loadFileSignal(QFileInfo)\0"
};

void nmc::DkFolderLabel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkFolderLabel *_t = static_cast<DkFolderLabel *>(_o);
        switch (_id) {
        case 0: _t->loadFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkFolderLabel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkFolderLabel::staticMetaObject = {
    { &QLabel::staticMetaObject, qt_meta_stringdata_nmc__DkFolderLabel,
      qt_meta_data_nmc__DkFolderLabel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkFolderLabel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkFolderLabel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkFolderLabel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkFolderLabel))
        return static_cast<void*>(const_cast< DkFolderLabel*>(this));
    return QLabel::qt_metacast(_clname);
}

int nmc::DkFolderLabel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLabel::qt_metacall(_c, _id, _a);
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
void nmc::DkFolderLabel::loadFileSignal(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkImageLabel[] = {

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
      19,   18,   18,   18, 0x05,
      33,   18,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      59,   18,   18,   18, 0x0a,
      73,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkImageLabel[] = {
    "nmc::DkImageLabel\0\0labelLoaded()\0"
    "loadFileSignal(QFileInfo)\0thumbLoaded()\0"
    "removeFileFromList()\0"
};

void nmc::DkImageLabel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkImageLabel *_t = static_cast<DkImageLabel *>(_o);
        switch (_id) {
        case 0: _t->labelLoaded(); break;
        case 1: _t->loadFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 2: _t->thumbLoaded(); break;
        case 3: _t->removeFileFromList(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkImageLabel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkImageLabel::staticMetaObject = {
    { &QLabel::staticMetaObject, qt_meta_stringdata_nmc__DkImageLabel,
      qt_meta_data_nmc__DkImageLabel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkImageLabel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkImageLabel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkImageLabel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkImageLabel))
        return static_cast<void*>(const_cast< DkImageLabel*>(this));
    return QLabel::qt_metacast(_clname);
}

int nmc::DkImageLabel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLabel::qt_metacall(_c, _id, _a);
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
void nmc::DkImageLabel::labelLoaded()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkImageLabel::loadFileSignal(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_nmc__DkRecentFilesWidget[] = {

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
      35,   26,   25,   25, 0x05,

 // slots: signature, parameters, type, tag, flags
      61,   25,   25,   25, 0x0a,
      75,   25,   25,   25, 0x0a,
      99,   91,   25,   25, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkRecentFilesWidget[] = {
    "nmc::DkRecentFilesWidget\0\0fileInfo\0"
    "loadFileSignal(QFileInfo)\0updateFiles()\0"
    "updateFolders()\0visible\0setVisible(bool)\0"
};

void nmc::DkRecentFilesWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkRecentFilesWidget *_t = static_cast<DkRecentFilesWidget *>(_o);
        switch (_id) {
        case 0: _t->loadFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 1: _t->updateFiles(); break;
        case 2: _t->updateFolders(); break;
        case 3: _t->setVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkRecentFilesWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkRecentFilesWidget::staticMetaObject = {
    { &DkWidget::staticMetaObject, qt_meta_stringdata_nmc__DkRecentFilesWidget,
      qt_meta_data_nmc__DkRecentFilesWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkRecentFilesWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkRecentFilesWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkRecentFilesWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkRecentFilesWidget))
        return static_cast<void*>(const_cast< DkRecentFilesWidget*>(this));
    return DkWidget::qt_metacast(_clname);
}

int nmc::DkRecentFilesWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::DkRecentFilesWidget::loadFileSignal(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
