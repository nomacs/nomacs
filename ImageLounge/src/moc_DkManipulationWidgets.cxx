/****************************************************************************
** Meta object code from reading C++ file 'DkManipulationWidgets.h'
**
** Created: Thu Dec 18 00:26:16 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkManipulationWidgets.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkManipulationWidgets.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkImageManipulationWidget[] = {

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
      43,   32,   31,   31, 0x05,

 // slots: signature, parameters, type, tag, flags
      77,   73,   31,   31, 0x09,
     102,   73,   31,   31, 0x09,
     133,   73,   31,   31, 0x09,
     154,   73,   31,   31, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkImageManipulationWidget[] = {
    "nmc::DkImageManipulationWidget\0\0"
    "updatedImg\0updateDialogImgSignal(QImage)\0"
    "val\0updateSliderSpinBox(int)\0"
    "updateDoubleSliderSpinBox(int)\0"
    "updateSliderVal(int)\0updateDoubleSliderVal(double)\0"
};

void nmc::DkImageManipulationWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkImageManipulationWidget *_t = static_cast<DkImageManipulationWidget *>(_o);
        switch (_id) {
        case 0: _t->updateDialogImgSignal((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 1: _t->updateSliderSpinBox((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->updateDoubleSliderSpinBox((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->updateSliderVal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->updateDoubleSliderVal((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkImageManipulationWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkImageManipulationWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkImageManipulationWidget,
      qt_meta_data_nmc__DkImageManipulationWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkImageManipulationWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkImageManipulationWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkImageManipulationWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkImageManipulationWidget))
        return static_cast<void*>(const_cast< DkImageManipulationWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkImageManipulationWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::DkImageManipulationWidget::updateDialogImgSignal(QImage _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkBrightness[] = {

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

static const char qt_meta_stringdata_nmc__DkBrightness[] = {
    "nmc::DkBrightness\0"
};

void nmc::DkBrightness::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkBrightness::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkBrightness::staticMetaObject = {
    { &DkImageManipulationWidget::staticMetaObject, qt_meta_stringdata_nmc__DkBrightness,
      qt_meta_data_nmc__DkBrightness, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkBrightness::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkBrightness::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkBrightness::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkBrightness))
        return static_cast<void*>(const_cast< DkBrightness*>(this));
    return DkImageManipulationWidget::qt_metacast(_clname);
}

int nmc::DkBrightness::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkImageManipulationWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkContrast[] = {

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

static const char qt_meta_stringdata_nmc__DkContrast[] = {
    "nmc::DkContrast\0"
};

void nmc::DkContrast::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkContrast::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkContrast::staticMetaObject = {
    { &DkImageManipulationWidget::staticMetaObject, qt_meta_stringdata_nmc__DkContrast,
      qt_meta_data_nmc__DkContrast, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkContrast::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkContrast::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkContrast::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkContrast))
        return static_cast<void*>(const_cast< DkContrast*>(this));
    return DkImageManipulationWidget::qt_metacast(_clname);
}

int nmc::DkContrast::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkImageManipulationWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkSaturation[] = {

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
      26,   19,   18,   18, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkSaturation[] = {
    "nmc::DkSaturation\0\0sColor\0"
    "setSliderStyle(QString)\0"
};

void nmc::DkSaturation::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkSaturation *_t = static_cast<DkSaturation *>(_o);
        switch (_id) {
        case 0: _t->setSliderStyle((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkSaturation::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkSaturation::staticMetaObject = {
    { &DkImageManipulationWidget::staticMetaObject, qt_meta_stringdata_nmc__DkSaturation,
      qt_meta_data_nmc__DkSaturation, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkSaturation::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkSaturation::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkSaturation::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkSaturation))
        return static_cast<void*>(const_cast< DkSaturation*>(this));
    return DkImageManipulationWidget::qt_metacast(_clname);
}

int nmc::DkSaturation::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkImageManipulationWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkHue[] = {

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
      19,   12,   11,   11, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkHue[] = {
    "nmc::DkHue\0\0sColor\0setSaturationSliderColor(QString)\0"
};

void nmc::DkHue::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkHue *_t = static_cast<DkHue *>(_o);
        switch (_id) {
        case 0: _t->setSaturationSliderColor((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkHue::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkHue::staticMetaObject = {
    { &DkImageManipulationWidget::staticMetaObject, qt_meta_stringdata_nmc__DkHue,
      qt_meta_data_nmc__DkHue, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkHue::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkHue::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkHue::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkHue))
        return static_cast<void*>(const_cast< DkHue*>(this));
    return DkImageManipulationWidget::qt_metacast(_clname);
}

int nmc::DkHue::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkImageManipulationWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkHue::setSaturationSliderColor(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkGamma[] = {

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

static const char qt_meta_stringdata_nmc__DkGamma[] = {
    "nmc::DkGamma\0"
};

void nmc::DkGamma::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkGamma::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkGamma::staticMetaObject = {
    { &DkImageManipulationWidget::staticMetaObject, qt_meta_stringdata_nmc__DkGamma,
      qt_meta_data_nmc__DkGamma, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkGamma::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkGamma::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkGamma::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkGamma))
        return static_cast<void*>(const_cast< DkGamma*>(this));
    return DkImageManipulationWidget::qt_metacast(_clname);
}

int nmc::DkGamma::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkImageManipulationWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkExposure[] = {

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

static const char qt_meta_stringdata_nmc__DkExposure[] = {
    "nmc::DkExposure\0"
};

void nmc::DkExposure::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkExposure::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkExposure::staticMetaObject = {
    { &DkImageManipulationWidget::staticMetaObject, qt_meta_stringdata_nmc__DkExposure,
      qt_meta_data_nmc__DkExposure, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkExposure::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkExposure::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkExposure::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkExposure))
        return static_cast<void*>(const_cast< DkExposure*>(this));
    return DkImageManipulationWidget::qt_metacast(_clname);
}

int nmc::DkExposure::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkImageManipulationWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkUndoRedo[] = {

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
      17,   16,   16,   16, 0x09,
      31,   16,   16,   16, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkUndoRedo[] = {
    "nmc::DkUndoRedo\0\0undoPressed()\0"
    "redoPressed()\0"
};

void nmc::DkUndoRedo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkUndoRedo *_t = static_cast<DkUndoRedo *>(_o);
        switch (_id) {
        case 0: _t->undoPressed(); break;
        case 1: _t->redoPressed(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkUndoRedo::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkUndoRedo::staticMetaObject = {
    { &DkImageManipulationWidget::staticMetaObject, qt_meta_stringdata_nmc__DkUndoRedo,
      qt_meta_data_nmc__DkUndoRedo, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkUndoRedo::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkUndoRedo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkUndoRedo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkUndoRedo))
        return static_cast<void*>(const_cast< DkUndoRedo*>(this));
    return DkImageManipulationWidget::qt_metacast(_clname);
}

int nmc::DkUndoRedo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkImageManipulationWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkImageManipulationDialog[] = {

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
      44,   32,   31,   31, 0x05,

 // slots: signature, parameters, type, tag, flags
      79,   68,   31,   31, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkImageManipulationDialog[] = {
    "nmc::DkImageManipulationDialog\0\0"
    "isGrayscale\0isNotGrayscaleImg(bool)\0"
    "updatedImg\0updateImg(QImage)\0"
};

void nmc::DkImageManipulationDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkImageManipulationDialog *_t = static_cast<DkImageManipulationDialog *>(_o);
        switch (_id) {
        case 0: _t->isNotGrayscaleImg((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->updateImg((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkImageManipulationDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkImageManipulationDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkImageManipulationDialog,
      qt_meta_data_nmc__DkImageManipulationDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkImageManipulationDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkImageManipulationDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkImageManipulationDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkImageManipulationDialog))
        return static_cast<void*>(const_cast< DkImageManipulationDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkImageManipulationDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void nmc::DkImageManipulationDialog::isNotGrayscaleImg(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
