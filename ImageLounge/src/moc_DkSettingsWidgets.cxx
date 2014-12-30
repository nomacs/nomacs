/****************************************************************************
** Meta object code from reading C++ file 'DkSettingsWidgets.h'
**
** Created: Thu Dec 18 00:26:14 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkSettingsWidgets.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkSettingsWidgets.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkSpinBoxWidget[] = {

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

static const char qt_meta_stringdata_nmc__DkSpinBoxWidget[] = {
    "nmc::DkSpinBoxWidget\0"
};

void nmc::DkSpinBoxWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkSpinBoxWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkSpinBoxWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkSpinBoxWidget,
      qt_meta_data_nmc__DkSpinBoxWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkSpinBoxWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkSpinBoxWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkSpinBoxWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkSpinBoxWidget))
        return static_cast<void*>(const_cast< DkSpinBoxWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkSpinBoxWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkDoubleSpinBoxWidget[] = {

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

static const char qt_meta_stringdata_nmc__DkDoubleSpinBoxWidget[] = {
    "nmc::DkDoubleSpinBoxWidget\0"
};

void nmc::DkDoubleSpinBoxWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkDoubleSpinBoxWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkDoubleSpinBoxWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkDoubleSpinBoxWidget,
      qt_meta_data_nmc__DkDoubleSpinBoxWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkDoubleSpinBoxWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkDoubleSpinBoxWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkDoubleSpinBoxWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkDoubleSpinBoxWidget))
        return static_cast<void*>(const_cast< DkDoubleSpinBoxWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkDoubleSpinBoxWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkCheckBoxDelegate[] = {

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
      25,   24,   24,   24, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkCheckBoxDelegate[] = {
    "nmc::DkCheckBoxDelegate\0\0cbChanged(int)\0"
};

void nmc::DkCheckBoxDelegate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkCheckBoxDelegate *_t = static_cast<DkCheckBoxDelegate *>(_o);
        switch (_id) {
        case 0: _t->cbChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkCheckBoxDelegate::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkCheckBoxDelegate::staticMetaObject = {
    { &QStyledItemDelegate::staticMetaObject, qt_meta_stringdata_nmc__DkCheckBoxDelegate,
      qt_meta_data_nmc__DkCheckBoxDelegate, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkCheckBoxDelegate::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkCheckBoxDelegate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkCheckBoxDelegate::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkCheckBoxDelegate))
        return static_cast<void*>(const_cast< DkCheckBoxDelegate*>(this));
    return QStyledItemDelegate::qt_metacast(_clname);
}

int nmc::DkCheckBoxDelegate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QStyledItemDelegate::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkWhiteListViewModel[] = {

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

static const char qt_meta_stringdata_nmc__DkWhiteListViewModel[] = {
    "nmc::DkWhiteListViewModel\0"
};

void nmc::DkWhiteListViewModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkWhiteListViewModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkWhiteListViewModel::staticMetaObject = {
    { &QAbstractTableModel::staticMetaObject, qt_meta_stringdata_nmc__DkWhiteListViewModel,
      qt_meta_data_nmc__DkWhiteListViewModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkWhiteListViewModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkWhiteListViewModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkWhiteListViewModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkWhiteListViewModel))
        return static_cast<void*>(const_cast< DkWhiteListViewModel*>(this));
    return QAbstractTableModel::qt_metacast(_clname);
}

int nmc::DkWhiteListViewModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkSettingsListView[] = {

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
      31,   25,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkSettingsListView[] = {
    "nmc::DkSettingsListView\0\0event\0"
    "keyPressEvent(QKeyEvent*)\0"
};

void nmc::DkSettingsListView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkSettingsListView *_t = static_cast<DkSettingsListView *>(_o);
        switch (_id) {
        case 0: _t->keyPressEvent((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkSettingsListView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkSettingsListView::staticMetaObject = {
    { &QListView::staticMetaObject, qt_meta_stringdata_nmc__DkSettingsListView,
      qt_meta_data_nmc__DkSettingsListView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkSettingsListView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkSettingsListView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkSettingsListView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkSettingsListView))
        return static_cast<void*>(const_cast< DkSettingsListView*>(this));
    return QListView::qt_metacast(_clname);
}

int nmc::DkSettingsListView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QListView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkSettingsWidget[] = {

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

static const char qt_meta_stringdata_nmc__DkSettingsWidget[] = {
    "nmc::DkSettingsWidget\0"
};

void nmc::DkSettingsWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkSettingsWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkSettingsWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_nmc__DkSettingsWidget,
      qt_meta_data_nmc__DkSettingsWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkSettingsWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkSettingsWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkSettingsWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkSettingsWidget))
        return static_cast<void*>(const_cast< DkSettingsWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int nmc::DkSettingsWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkFileWidget[] = {

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
      19,   18,   18,   18, 0x08,
      48,   42,   18,   18, 0x08,
      76,   71,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkFileWidget[] = {
    "nmc::DkFileWidget\0\0tmpPathButtonPressed()\0"
    "state\0useTmpPathChanged(int)\0path\0"
    "lineEditChanged(QString)\0"
};

void nmc::DkFileWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkFileWidget *_t = static_cast<DkFileWidget *>(_o);
        switch (_id) {
        case 0: _t->tmpPathButtonPressed(); break;
        case 1: _t->useTmpPathChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->lineEditChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkFileWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkFileWidget::staticMetaObject = {
    { &DkSettingsWidget::staticMetaObject, qt_meta_stringdata_nmc__DkFileWidget,
      qt_meta_data_nmc__DkFileWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkFileWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkFileWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkFileWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkFileWidget))
        return static_cast<void*>(const_cast< DkFileWidget*>(this));
    return DkSettingsWidget::qt_metacast(_clname);
}

int nmc::DkFileWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkSettingsWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkSynchronizeSettingsWidget[] = {

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
      40,   34,   33,   33, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkSynchronizeSettingsWidget[] = {
    "nmc::DkSynchronizeSettingsWidget\0\0"
    "state\0enableNetworkCheckBoxChanged(int)\0"
};

void nmc::DkSynchronizeSettingsWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkSynchronizeSettingsWidget *_t = static_cast<DkSynchronizeSettingsWidget *>(_o);
        switch (_id) {
        case 0: _t->enableNetworkCheckBoxChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkSynchronizeSettingsWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkSynchronizeSettingsWidget::staticMetaObject = {
    { &DkSettingsWidget::staticMetaObject, qt_meta_stringdata_nmc__DkSynchronizeSettingsWidget,
      qt_meta_data_nmc__DkSynchronizeSettingsWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkSynchronizeSettingsWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkSynchronizeSettingsWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkSynchronizeSettingsWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkSynchronizeSettingsWidget))
        return static_cast<void*>(const_cast< DkSynchronizeSettingsWidget*>(this));
    return DkSettingsWidget::qt_metacast(_clname);
}

int nmc::DkSynchronizeSettingsWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkSettingsWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkFileFilterSettingWidget[] = {

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

static const char qt_meta_stringdata_nmc__DkFileFilterSettingWidget[] = {
    "nmc::DkFileFilterSettingWidget\0"
};

void nmc::DkFileFilterSettingWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkFileFilterSettingWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkFileFilterSettingWidget::staticMetaObject = {
    { &DkSettingsWidget::staticMetaObject, qt_meta_stringdata_nmc__DkFileFilterSettingWidget,
      qt_meta_data_nmc__DkFileFilterSettingWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkFileFilterSettingWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkFileFilterSettingWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkFileFilterSettingWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkFileFilterSettingWidget))
        return static_cast<void*>(const_cast< DkFileFilterSettingWidget*>(this));
    return DkSettingsWidget::qt_metacast(_clname);
}

int nmc::DkFileFilterSettingWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkSettingsWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkDisplaySettingsWidget[] = {

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
      38,   30,   29,   29, 0x08,
      57,   30,   29,   29, 0x08,
      80,   30,   29,   29, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkDisplaySettingsWidget[] = {
    "nmc::DkDisplaySettingsWidget\0\0checked\0"
    "showFileName(bool)\0showCreationDate(bool)\0"
    "showRating(bool)\0"
};

void nmc::DkDisplaySettingsWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkDisplaySettingsWidget *_t = static_cast<DkDisplaySettingsWidget *>(_o);
        switch (_id) {
        case 0: _t->showFileName((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->showCreationDate((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->showRating((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkDisplaySettingsWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkDisplaySettingsWidget::staticMetaObject = {
    { &DkSettingsWidget::staticMetaObject, qt_meta_stringdata_nmc__DkDisplaySettingsWidget,
      qt_meta_data_nmc__DkDisplaySettingsWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkDisplaySettingsWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkDisplaySettingsWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkDisplaySettingsWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkDisplaySettingsWidget))
        return static_cast<void*>(const_cast< DkDisplaySettingsWidget*>(this));
    return DkSettingsWidget::qt_metacast(_clname);
}

int nmc::DkDisplaySettingsWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkSettingsWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkMetaDataSettingsWidget[] = {

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

static const char qt_meta_stringdata_nmc__DkMetaDataSettingsWidget[] = {
    "nmc::DkMetaDataSettingsWidget\0"
};

void nmc::DkMetaDataSettingsWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkMetaDataSettingsWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkMetaDataSettingsWidget::staticMetaObject = {
    { &DkSettingsWidget::staticMetaObject, qt_meta_stringdata_nmc__DkMetaDataSettingsWidget,
      qt_meta_data_nmc__DkMetaDataSettingsWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkMetaDataSettingsWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkMetaDataSettingsWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkMetaDataSettingsWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkMetaDataSettingsWidget))
        return static_cast<void*>(const_cast< DkMetaDataSettingsWidget*>(this));
    return DkSettingsWidget::qt_metacast(_clname);
}

int nmc::DkMetaDataSettingsWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkSettingsWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkResourceSettingsWidgets[] = {

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
      41,   32,   31,   31, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkResourceSettingsWidgets[] = {
    "nmc::DkResourceSettingsWidgets\0\0"
    "newValue\0memorySliderChanged(int)\0"
};

void nmc::DkResourceSettingsWidgets::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkResourceSettingsWidgets *_t = static_cast<DkResourceSettingsWidgets *>(_o);
        switch (_id) {
        case 0: _t->memorySliderChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkResourceSettingsWidgets::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkResourceSettingsWidgets::staticMetaObject = {
    { &DkSettingsWidget::staticMetaObject, qt_meta_stringdata_nmc__DkResourceSettingsWidgets,
      qt_meta_data_nmc__DkResourceSettingsWidgets, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkResourceSettingsWidgets::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkResourceSettingsWidgets::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkResourceSettingsWidgets::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkResourceSettingsWidgets))
        return static_cast<void*>(const_cast< DkResourceSettingsWidgets*>(this));
    return DkSettingsWidget::qt_metacast(_clname);
}

int nmc::DkResourceSettingsWidgets::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkSettingsWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkGlobalSettingsWidget[] = {

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
      29,   28,   28,   28, 0x05,

 // slots: signature, parameters, type, tag, flags
      44,   28,   28,   28, 0x08,
      66,   28,   28,   28, 0x08,
      81,   28,   28,   28, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkGlobalSettingsWidget[] = {
    "nmc::DkGlobalSettingsWidget\0\0"
    "applyDefault()\0setToDefaultPressed()\0"
    "bgColorReset()\0iconColorReset()\0"
};

void nmc::DkGlobalSettingsWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkGlobalSettingsWidget *_t = static_cast<DkGlobalSettingsWidget *>(_o);
        switch (_id) {
        case 0: _t->applyDefault(); break;
        case 1: _t->setToDefaultPressed(); break;
        case 2: _t->bgColorReset(); break;
        case 3: _t->iconColorReset(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkGlobalSettingsWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkGlobalSettingsWidget::staticMetaObject = {
    { &DkSettingsWidget::staticMetaObject, qt_meta_stringdata_nmc__DkGlobalSettingsWidget,
      qt_meta_data_nmc__DkGlobalSettingsWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkGlobalSettingsWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkGlobalSettingsWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkGlobalSettingsWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkGlobalSettingsWidget))
        return static_cast<void*>(const_cast< DkGlobalSettingsWidget*>(this));
    return DkSettingsWidget::qt_metacast(_clname);
}

int nmc::DkGlobalSettingsWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkSettingsWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkGlobalSettingsWidget::applyDefault()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_nmc__DkRemoteControlWidget[] = {

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

static const char qt_meta_stringdata_nmc__DkRemoteControlWidget[] = {
    "nmc::DkRemoteControlWidget\0"
};

void nmc::DkRemoteControlWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkRemoteControlWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkRemoteControlWidget::staticMetaObject = {
    { &DkSettingsWidget::staticMetaObject, qt_meta_stringdata_nmc__DkRemoteControlWidget,
      qt_meta_data_nmc__DkRemoteControlWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkRemoteControlWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkRemoteControlWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkRemoteControlWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkRemoteControlWidget))
        return static_cast<void*>(const_cast< DkRemoteControlWidget*>(this));
    return DkSettingsWidget::qt_metacast(_clname);
}

int nmc::DkRemoteControlWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkSettingsWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkSettingsDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      23,   22,   22,   22, 0x05,
      41,   22,   22,   22, 0x05,
      59,   22,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
      87,   80,   22,   22, 0x08,
     117,   22,   22,   22, 0x08,
     132,   22,   22,   22, 0x08,
     148,   22,   22,   22, 0x08,
     162,   22,   22,   22, 0x08,
     183,  177,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkSettingsDialog[] = {
    "nmc::DkSettingsDialog\0\0languageChanged()\0"
    "settingsChanged()\0setToDefaultSignal()\0"
    "qmodel\0listViewSelected(QModelIndex)\0"
    "saveSettings()\0cancelPressed()\0"
    "initWidgets()\0setToDefault()\0state\0"
    "advancedSettingsChanged(int)\0"
};

void nmc::DkSettingsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkSettingsDialog *_t = static_cast<DkSettingsDialog *>(_o);
        switch (_id) {
        case 0: _t->languageChanged(); break;
        case 1: _t->settingsChanged(); break;
        case 2: _t->setToDefaultSignal(); break;
        case 3: _t->listViewSelected((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 4: _t->saveSettings(); break;
        case 5: _t->cancelPressed(); break;
        case 6: _t->initWidgets(); break;
        case 7: _t->setToDefault(); break;
        case 8: _t->advancedSettingsChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkSettingsDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkSettingsDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkSettingsDialog,
      qt_meta_data_nmc__DkSettingsDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkSettingsDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkSettingsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkSettingsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkSettingsDialog))
        return static_cast<void*>(const_cast< DkSettingsDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkSettingsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void nmc::DkSettingsDialog::languageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkSettingsDialog::settingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void nmc::DkSettingsDialog::setToDefaultSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
QT_END_MOC_NAMESPACE
