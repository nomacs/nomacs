/****************************************************************************
** Meta object code from reading C++ file 'DkMetaDataWidgets.h'
**
** Created: Thu Dec 18 00:26:15 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkMetaDataWidgets.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkMetaDataWidgets.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkMetaDataModel[] = {

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

static const char qt_meta_stringdata_nmc__DkMetaDataModel[] = {
    "nmc::DkMetaDataModel\0"
};

void nmc::DkMetaDataModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkMetaDataModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkMetaDataModel::staticMetaObject = {
    { &QAbstractTableModel::staticMetaObject, qt_meta_stringdata_nmc__DkMetaDataModel,
      qt_meta_data_nmc__DkMetaDataModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkMetaDataModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkMetaDataModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkMetaDataModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkMetaDataModel))
        return static_cast<void*>(const_cast< DkMetaDataModel*>(this));
    return QAbstractTableModel::qt_metacast(_clname);
}

int nmc::DkMetaDataModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkMetaDataDock[] = {

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
      26,   21,   20,   20, 0x0a,
      77,   70,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkMetaDataDock[] = {
    "nmc::DkMetaDataDock\0\0imgC\0"
    "setImage(QSharedPointer<DkImageContainerT>)\0"
    "loaded\0thumbLoaded(bool)\0"
};

void nmc::DkMetaDataDock::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkMetaDataDock *_t = static_cast<DkMetaDataDock *>(_o);
        switch (_id) {
        case 0: _t->setImage((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 1: _t->thumbLoaded((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkMetaDataDock::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkMetaDataDock::staticMetaObject = {
    { &QDockWidget::staticMetaObject, qt_meta_stringdata_nmc__DkMetaDataDock,
      qt_meta_data_nmc__DkMetaDataDock, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkMetaDataDock::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkMetaDataDock::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkMetaDataDock::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkMetaDataDock))
        return static_cast<void*>(const_cast< DkMetaDataDock*>(this));
    return QDockWidget::qt_metacast(_clname);
}

int nmc::DkMetaDataDock::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDockWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkMetaDataInfo[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      26,   21,   20,   20, 0x0a,
      81,   74,   20,   20, 0x0a,
      96,   20,   20,   20, 0x0a,
     117,  111,   20,   20, 0x0a,
     154,  146,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkMetaDataInfo[] = {
    "nmc::DkMetaDataInfo\0\0imgC\0"
    "setImageInfo(QSharedPointer<DkImageContainerT>)\0"
    "rating\0setRating(int)\0updateLabels()\0"
    "event\0mouseMoveEvent(QMouseEvent*)\0"
    "visible\0setVisible(bool)\0"
};

void nmc::DkMetaDataInfo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkMetaDataInfo *_t = static_cast<DkMetaDataInfo *>(_o);
        switch (_id) {
        case 0: _t->setImageInfo((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 1: _t->setRating((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->updateLabels(); break;
        case 3: _t->mouseMoveEvent((*reinterpret_cast< QMouseEvent*(*)>(_a[1]))); break;
        case 4: _t->setVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkMetaDataInfo::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkMetaDataInfo::staticMetaObject = {
    { &DkWidget::staticMetaObject, qt_meta_stringdata_nmc__DkMetaDataInfo,
      qt_meta_data_nmc__DkMetaDataInfo, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkMetaDataInfo::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkMetaDataInfo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkMetaDataInfo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkMetaDataInfo))
        return static_cast<void*>(const_cast< DkMetaDataInfo*>(this));
    return DkWidget::qt_metacast(_clname);
}

int nmc::DkMetaDataInfo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkCommentTextEdit[] = {

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
      24,   23,   23,   23, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkCommentTextEdit[] = {
    "nmc::DkCommentTextEdit\0\0focusLost()\0"
};

void nmc::DkCommentTextEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkCommentTextEdit *_t = static_cast<DkCommentTextEdit *>(_o);
        switch (_id) {
        case 0: _t->focusLost(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkCommentTextEdit::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkCommentTextEdit::staticMetaObject = {
    { &QTextEdit::staticMetaObject, qt_meta_stringdata_nmc__DkCommentTextEdit,
      qt_meta_data_nmc__DkCommentTextEdit, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkCommentTextEdit::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkCommentTextEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkCommentTextEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkCommentTextEdit))
        return static_cast<void*>(const_cast< DkCommentTextEdit*>(this));
    return QTextEdit::qt_metacast(_clname);
}

int nmc::DkCommentTextEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTextEdit::qt_metacall(_c, _id, _a);
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
void nmc::DkCommentTextEdit::focusLost()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_nmc__DkCommentWidget[] = {

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
      26,   22,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
      50,   21,   21,   21, 0x0a,
      80,   21,   21,   21, 0x0a,
     108,   21,   21,   21, 0x0a,
     132,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkCommentWidget[] = {
    "nmc::DkCommentWidget\0\0msg\0"
    "showInfoSignal(QString)\0"
    "on_CommentLabel_textChanged()\0"
    "on_CommentLabel_focusLost()\0"
    "on_saveButton_clicked()\0"
    "on_cancelButton_clicked()\0"
};

void nmc::DkCommentWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkCommentWidget *_t = static_cast<DkCommentWidget *>(_o);
        switch (_id) {
        case 0: _t->showInfoSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->on_CommentLabel_textChanged(); break;
        case 2: _t->on_CommentLabel_focusLost(); break;
        case 3: _t->on_saveButton_clicked(); break;
        case 4: _t->on_cancelButton_clicked(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkCommentWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkCommentWidget::staticMetaObject = {
    { &DkFadeLabel::staticMetaObject, qt_meta_stringdata_nmc__DkCommentWidget,
      qt_meta_data_nmc__DkCommentWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkCommentWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkCommentWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkCommentWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkCommentWidget))
        return static_cast<void*>(const_cast< DkCommentWidget*>(this));
    return DkFadeLabel::qt_metacast(_clname);
}

int nmc::DkCommentWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkFadeLabel::qt_metacall(_c, _id, _a);
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
void nmc::DkCommentWidget::showInfoSignal(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
