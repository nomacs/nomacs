/****************************************************************************
** Meta object code from reading C++ file 'DkDialog.h'
**
** Created: Thu Dec 18 00:26:17 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkSplashScreen[] = {

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

static const char qt_meta_stringdata_nmc__DkSplashScreen[] = {
    "nmc::DkSplashScreen\0"
};

void nmc::DkSplashScreen::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkSplashScreen::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkSplashScreen::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkSplashScreen,
      qt_meta_data_nmc__DkSplashScreen, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkSplashScreen::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkSplashScreen::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkSplashScreen::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkSplashScreen))
        return static_cast<void*>(const_cast< DkSplashScreen*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkSplashScreen::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkFileValidator[] = {

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

static const char qt_meta_stringdata_nmc__DkFileValidator[] = {
    "nmc::DkFileValidator\0"
};

void nmc::DkFileValidator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkFileValidator::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkFileValidator::staticMetaObject = {
    { &QValidator::staticMetaObject, qt_meta_stringdata_nmc__DkFileValidator,
      qt_meta_data_nmc__DkFileValidator, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkFileValidator::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkFileValidator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkFileValidator::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkFileValidator))
        return static_cast<void*>(const_cast< DkFileValidator*>(this));
    return QValidator::qt_metacast(_clname);
}

int nmc::DkFileValidator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QValidator::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkTrainDialog[] = {

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
      25,   20,   19,   19, 0x0a,
      55,   46,   19,   19, 0x0a,
      73,   19,   19,   19, 0x2a,
      84,   19,   19,   19, 0x0a,
      95,   19,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkTrainDialog[] = {
    "nmc::DkTrainDialog\0\0text\0textChanged(QString)\0"
    "filePath\0loadFile(QString)\0loadFile()\0"
    "openFile()\0accept()\0"
};

void nmc::DkTrainDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkTrainDialog *_t = static_cast<DkTrainDialog *>(_o);
        switch (_id) {
        case 0: _t->textChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->loadFile((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->loadFile(); break;
        case 3: _t->openFile(); break;
        case 4: _t->accept(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkTrainDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkTrainDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkTrainDialog,
      qt_meta_data_nmc__DkTrainDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkTrainDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkTrainDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkTrainDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkTrainDialog))
        return static_cast<void*>(const_cast< DkTrainDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkTrainDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkAppManager[] = {

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
      26,   19,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      51,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkAppManager[] = {
    "nmc::DkAppManager\0\0action\0"
    "openFileSignal(QAction*)\0openTriggered()\0"
};

void nmc::DkAppManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkAppManager *_t = static_cast<DkAppManager *>(_o);
        switch (_id) {
        case 0: _t->openFileSignal((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 1: _t->openTriggered(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkAppManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkAppManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__DkAppManager,
      qt_meta_data_nmc__DkAppManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkAppManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkAppManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkAppManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkAppManager))
        return static_cast<void*>(const_cast< DkAppManager*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::DkAppManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::DkAppManager::openFileSignal(QAction * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkAppManagerDialog[] = {

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
      29,   25,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
      54,   24,   24,   24, 0x0a,
      77,   24,   24,   24, 0x0a,
     103,   24,   24,   24, 0x0a,
     126,   24,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkAppManagerDialog[] = {
    "nmc::DkAppManagerDialog\0\0act\0"
    "openWithSignal(QAction*)\0"
    "on_addButton_clicked()\0on_deleteButton_clicked()\0"
    "on_runButton_clicked()\0accept()\0"
};

void nmc::DkAppManagerDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkAppManagerDialog *_t = static_cast<DkAppManagerDialog *>(_o);
        switch (_id) {
        case 0: _t->openWithSignal((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 1: _t->on_addButton_clicked(); break;
        case 2: _t->on_deleteButton_clicked(); break;
        case 3: _t->on_runButton_clicked(); break;
        case 4: _t->accept(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkAppManagerDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkAppManagerDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkAppManagerDialog,
      qt_meta_data_nmc__DkAppManagerDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkAppManagerDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkAppManagerDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkAppManagerDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkAppManagerDialog))
        return static_cast<void*>(const_cast< DkAppManagerDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkAppManagerDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void nmc::DkAppManagerDialog::openWithSignal(QAction * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkSearchDialog[] = {

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
      26,   21,   20,   20, 0x05,
      52,   20,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
      83,   78,   20,   20, 0x0a,
     117,   20,   20,   20, 0x0a,
     139,   20,   20,   20, 0x0a,
     165,   20,   20,   20, 0x0a,
     202,  191,   20,   20, 0x0a,
     247,  191,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkSearchDialog[] = {
    "nmc::DkSearchDialog\0\0file\0"
    "loadFileSignal(QFileInfo)\0"
    "filterSignal(QStringList)\0text\0"
    "on_searchBar_textChanged(QString)\0"
    "on_okButton_pressed()\0on_filterButton_pressed()\0"
    "on_cancelButton_pressed()\0modelIndex\0"
    "on_resultListView_doubleClicked(QModelIndex)\0"
    "on_resultListView_clicked(QModelIndex)\0"
};

void nmc::DkSearchDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkSearchDialog *_t = static_cast<DkSearchDialog *>(_o);
        switch (_id) {
        case 0: _t->loadFileSignal((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 1: _t->filterSignal((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 2: _t->on_searchBar_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->on_okButton_pressed(); break;
        case 4: _t->on_filterButton_pressed(); break;
        case 5: _t->on_cancelButton_pressed(); break;
        case 6: _t->on_resultListView_doubleClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 7: _t->on_resultListView_clicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkSearchDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkSearchDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkSearchDialog,
      qt_meta_data_nmc__DkSearchDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkSearchDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkSearchDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkSearchDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkSearchDialog))
        return static_cast<void*>(const_cast< DkSearchDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkSearchDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void nmc::DkSearchDialog::loadFileSignal(QFileInfo _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkSearchDialog::filterSignal(QStringList _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_nmc__DkResizeDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x09,
      48,   20,   20,   20, 0x09,
      77,   72,   20,   20, 0x09,
     113,   72,   20,   20, 0x09,
     149,   72,   20,   20, 0x09,
     184,   72,   20,   20, 0x09,
     220,   72,   20,   20, 0x09,
     264,  260,   20,   20, 0x09,
     300,  260,   20,   20, 0x09,
     336,  260,   20,   20, 0x09,
     375,  260,   20,   20, 0x09,
     415,   20,   20,   20, 0x09,
     442,   20,   20,   20, 0x09,
     471,   20,   20,   20, 0x09,
     493,  485,   20,   20, 0x09,
     510,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkResizeDialog[] = {
    "nmc::DkResizeDialog\0\0on_lockButtonDim_clicked()\0"
    "on_lockButton_clicked()\0text\0"
    "on_wPixelEdit_valueChanged(QString)\0"
    "on_hPixelEdit_valueChanged(QString)\0"
    "on_widthEdit_valueChanged(QString)\0"
    "on_heightEdit_valueChanged(QString)\0"
    "on_resolutionEdit_valueChanged(QString)\0"
    "idx\0on_sizeBox_currentIndexChanged(int)\0"
    "on_unitBox_currentIndexChanged(int)\0"
    "on_resUnitBox_currentIndexChanged(int)\0"
    "on_resampleBox_currentIndexChanged(int)\0"
    "on_resampleCheck_clicked()\0"
    "on_gammaCorrection_clicked()\0drawPreview()\0"
    "visible\0setVisible(bool)\0accept()\0"
};

void nmc::DkResizeDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkResizeDialog *_t = static_cast<DkResizeDialog *>(_o);
        switch (_id) {
        case 0: _t->on_lockButtonDim_clicked(); break;
        case 1: _t->on_lockButton_clicked(); break;
        case 2: _t->on_wPixelEdit_valueChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->on_hPixelEdit_valueChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->on_widthEdit_valueChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->on_heightEdit_valueChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: _t->on_resolutionEdit_valueChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: _t->on_sizeBox_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->on_unitBox_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->on_resUnitBox_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->on_resampleBox_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->on_resampleCheck_clicked(); break;
        case 12: _t->on_gammaCorrection_clicked(); break;
        case 13: _t->drawPreview(); break;
        case 14: _t->setVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->accept(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkResizeDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkResizeDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkResizeDialog,
      qt_meta_data_nmc__DkResizeDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkResizeDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkResizeDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkResizeDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkResizeDialog))
        return static_cast<void*>(const_cast< DkResizeDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkResizeDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkShortcutDelegate[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      35,   25,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
      76,   71,   24,   24, 0x09,
      97,   24,   24,   24, 0x29,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkShortcutDelegate[] = {
    "nmc::DkShortcutDelegate\0\0text,item\0"
    "checkDuplicateSignal(QString,void*)\0"
    "text\0textChanged(QString)\0textChanged()\0"
};

void nmc::DkShortcutDelegate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkShortcutDelegate *_t = static_cast<DkShortcutDelegate *>(_o);
        switch (_id) {
        case 0: _t->checkDuplicateSignal((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< void*(*)>(_a[2]))); break;
        case 1: _t->textChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->textChanged(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkShortcutDelegate::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkShortcutDelegate::staticMetaObject = {
    { &QItemDelegate::staticMetaObject, qt_meta_stringdata_nmc__DkShortcutDelegate,
      qt_meta_data_nmc__DkShortcutDelegate, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkShortcutDelegate::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkShortcutDelegate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkShortcutDelegate::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkShortcutDelegate))
        return static_cast<void*>(const_cast< DkShortcutDelegate*>(this));
    return QItemDelegate::qt_metacast(_clname);
}

int nmc::DkShortcutDelegate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QItemDelegate::qt_metacall(_c, _id, _a);
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
void nmc::DkShortcutDelegate::checkDuplicateSignal(QString _t1, void * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkShortcutEditor[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       1,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
      35,   22, 0x4c095103,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkShortcutEditor[] = {
    "nmc::DkShortcutEditor\0QKeySequence\0"
    "shortcut\0"
};

void nmc::DkShortcutEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkShortcutEditor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkShortcutEditor::staticMetaObject = {
    { &QLineEdit::staticMetaObject, qt_meta_stringdata_nmc__DkShortcutEditor,
      qt_meta_data_nmc__DkShortcutEditor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkShortcutEditor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkShortcutEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkShortcutEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkShortcutEditor))
        return static_cast<void*>(const_cast< DkShortcutEditor*>(this));
    return QLineEdit::qt_metacast(_clname);
}

int nmc::DkShortcutEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QKeySequence*>(_v) = shortcut(); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setShortcut(*reinterpret_cast< QKeySequence*>(_v)); break;
        }
        _id -= 1;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
static const uint qt_meta_data_nmc__DkShortcutsModel[] = {

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
      28,   23,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
      63,   53,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkShortcutsModel[] = {
    "nmc::DkShortcutsModel\0\0info\0"
    "duplicateSignal(QString)\0text,item\0"
    "checkDuplicate(QString,void*)\0"
};

void nmc::DkShortcutsModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkShortcutsModel *_t = static_cast<DkShortcutsModel *>(_o);
        switch (_id) {
        case 0: _t->duplicateSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->checkDuplicate((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< void*(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkShortcutsModel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkShortcutsModel::staticMetaObject = {
    { &QAbstractTableModel::staticMetaObject, qt_meta_stringdata_nmc__DkShortcutsModel,
      qt_meta_data_nmc__DkShortcutsModel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkShortcutsModel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkShortcutsModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkShortcutsModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkShortcutsModel))
        return static_cast<void*>(const_cast< DkShortcutsModel*>(this));
    return QAbstractTableModel::qt_metacast(_clname);
}

int nmc::DkShortcutsModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
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
void nmc::DkShortcutsModel::duplicateSignal(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkShortcutsDialog[] = {

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
      24,   23,   23,   23, 0x0a,
      37,   33,   23,   23, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkShortcutsDialog[] = {
    "nmc::DkShortcutsDialog\0\0accept()\0cur\0"
    "contextMenu(QPoint)\0"
};

void nmc::DkShortcutsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkShortcutsDialog *_t = static_cast<DkShortcutsDialog *>(_o);
        switch (_id) {
        case 0: _t->accept(); break;
        case 1: _t->contextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkShortcutsDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkShortcutsDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkShortcutsDialog,
      qt_meta_data_nmc__DkShortcutsDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkShortcutsDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkShortcutsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkShortcutsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkShortcutsDialog))
        return static_cast<void*>(const_cast< DkShortcutsDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkShortcutsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
static const uint qt_meta_data_nmc__DkUpdateDialog[] = {

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
      35,   20,   20,   20, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkUpdateDialog[] = {
    "nmc::DkUpdateDialog\0\0startUpdate()\0"
    "okButtonClicked()\0"
};

void nmc::DkUpdateDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkUpdateDialog *_t = static_cast<DkUpdateDialog *>(_o);
        switch (_id) {
        case 0: _t->startUpdate(); break;
        case 1: _t->okButtonClicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkUpdateDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkUpdateDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkUpdateDialog,
      qt_meta_data_nmc__DkUpdateDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkUpdateDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkUpdateDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkUpdateDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkUpdateDialog))
        return static_cast<void*>(const_cast< DkUpdateDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkUpdateDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::DkUpdateDialog::startUpdate()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_nmc__DkPrintPreviewWidget[] = {

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
      27,   26,   26,   26, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkPrintPreviewWidget[] = {
    "nmc::DkPrintPreviewWidget\0\0zoomChanged()\0"
};

void nmc::DkPrintPreviewWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkPrintPreviewWidget *_t = static_cast<DkPrintPreviewWidget *>(_o);
        switch (_id) {
        case 0: _t->zoomChanged(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkPrintPreviewWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkPrintPreviewWidget::staticMetaObject = {
    { &QPrintPreviewWidget::staticMetaObject, qt_meta_stringdata_nmc__DkPrintPreviewWidget,
      qt_meta_data_nmc__DkPrintPreviewWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkPrintPreviewWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkPrintPreviewWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkPrintPreviewWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkPrintPreviewWidget))
        return static_cast<void*>(const_cast< DkPrintPreviewWidget*>(this));
    return QPrintPreviewWidget::qt_metacast(_clname);
}

int nmc::DkPrintPreviewWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPrintPreviewWidget::qt_metacall(_c, _id, _a);
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
void nmc::DkPrintPreviewWidget::zoomChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_nmc__DkPrintPreviewDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x0a,
      54,   46,   26,   26, 0x08,
      87,   80,   26,   26, 0x08,
     106,   26,   26,   26, 0x08,
     115,   26,   26,   26, 0x08,
     125,   26,   26,   26, 0x08,
     145,   26,   26,   26, 0x08,
     168,  164,   26,   26, 0x08,
     191,   26,   26,   26, 0x08,
     202,   26,   26,   26, 0x08,
     214,   26,   26,   26, 0x08,
     222,   26,   26,   26, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkPrintPreviewDialog[] = {
    "nmc::DkPrintPreviewDialog\0\0"
    "updateZoomFactor()\0printer\0"
    "paintRequested(QPrinter*)\0action\0"
    "fitImage(QAction*)\0zoomIn()\0zoomOut()\0"
    "zoomFactorChanged()\0dpiFactorChanged()\0"
    "dpi\0updateDpiFactor(qreal)\0resetDpi()\0"
    "pageSetup()\0print()\0centerImage()\0"
};

void nmc::DkPrintPreviewDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkPrintPreviewDialog *_t = static_cast<DkPrintPreviewDialog *>(_o);
        switch (_id) {
        case 0: _t->updateZoomFactor(); break;
        case 1: _t->paintRequested((*reinterpret_cast< QPrinter*(*)>(_a[1]))); break;
        case 2: _t->fitImage((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 3: _t->zoomIn(); break;
        case 4: _t->zoomOut(); break;
        case 5: _t->zoomFactorChanged(); break;
        case 6: _t->dpiFactorChanged(); break;
        case 7: _t->updateDpiFactor((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 8: _t->resetDpi(); break;
        case 9: _t->pageSetup(); break;
        case 10: _t->print(); break;
        case 11: _t->centerImage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkPrintPreviewDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkPrintPreviewDialog::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_nmc__DkPrintPreviewDialog,
      qt_meta_data_nmc__DkPrintPreviewDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkPrintPreviewDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkPrintPreviewDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkPrintPreviewDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkPrintPreviewDialog))
        return static_cast<void*>(const_cast< DkPrintPreviewDialog*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int nmc::DkPrintPreviewDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkOpacityDialog[] = {

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

static const char qt_meta_stringdata_nmc__DkOpacityDialog[] = {
    "nmc::DkOpacityDialog\0"
};

void nmc::DkOpacityDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkOpacityDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkOpacityDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkOpacityDialog,
      qt_meta_data_nmc__DkOpacityDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkOpacityDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkOpacityDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkOpacityDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkOpacityDialog))
        return static_cast<void*>(const_cast< DkOpacityDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkOpacityDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkExportTiffDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      29,   25,   24,   24, 0x05,
      49,   24,   24,   24, 0x05,
      73,   69,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
      94,   24,   24,   24, 0x0a,
     118,   24,   24,   24, 0x0a,
     151,  142,   24,   24, 0x0a,
     189,  184,   24,   24, 0x0a,
     208,   24,   24,   24, 0x0a,
     217,   24,   24,   24, 0x0a,
     262,  230,  226,   24, 0x0a,
     309,   24,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkExportTiffDialog[] = {
    "nmc::DkExportTiffDialog\0\0img\0"
    "updateImage(QImage)\0updateProgress(int)\0"
    "msg\0infoMessage(QString)\0"
    "on_openButton_pressed()\0on_saveButton_pressed()\0"
    "filename\0on_fileEdit_textChanged(QString)\0"
    "file\0setFile(QFileInfo)\0accept()\0"
    "reject()\0int\0file,saveFile,from,to,overwrite\0"
    "exportImages(QFileInfo,QFileInfo,int,int,bool)\0"
    "processingFinished()\0"
};

void nmc::DkExportTiffDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkExportTiffDialog *_t = static_cast<DkExportTiffDialog *>(_o);
        switch (_id) {
        case 0: _t->updateImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 1: _t->updateProgress((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->infoMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->on_openButton_pressed(); break;
        case 4: _t->on_saveButton_pressed(); break;
        case 5: _t->on_fileEdit_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->setFile((*reinterpret_cast< const QFileInfo(*)>(_a[1]))); break;
        case 7: _t->accept(); break;
        case 8: _t->reject(); break;
        case 9: { int _r = _t->exportImages((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< QFileInfo(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 10: _t->processingFinished(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkExportTiffDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkExportTiffDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkExportTiffDialog,
      qt_meta_data_nmc__DkExportTiffDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkExportTiffDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkExportTiffDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkExportTiffDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkExportTiffDialog))
        return static_cast<void*>(const_cast< DkExportTiffDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkExportTiffDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void nmc::DkExportTiffDialog::updateImage(QImage _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkExportTiffDialog::updateProgress(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkExportTiffDialog::infoMessage(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_nmc__DkUnsharpDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   22,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
      48,   46,   21,   21, 0x0a,
      81,   46,   21,   21, 0x0a,
     120,  115,   21,   21, 0x0a,
     139,   22,   21,   21, 0x0a,
     156,   21,   21,   21, 0x0a,
     173,   21,   21,   21, 0x0a,
     206,  189,  182,   21, 0x0a,
     237,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkUnsharpDialog[] = {
    "nmc::DkUnsharpDialog\0\0img\0updateImage(QImage)\0"
    "i\0on_sigmaSlider_valueChanged(int)\0"
    "on_amountSlider_valueChanged(int)\0"
    "file\0setFile(QFileInfo)\0setImage(QImage)\0"
    "computePreview()\0reject()\0QImage\0"
    "img,sigma,amount\0computeUnsharp(QImage,int,int)\0"
    "unsharpFinished()\0"
};

void nmc::DkUnsharpDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkUnsharpDialog *_t = static_cast<DkUnsharpDialog *>(_o);
        switch (_id) {
        case 0: _t->updateImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 1: _t->on_sigmaSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->on_amountSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->setFile((*reinterpret_cast< const QFileInfo(*)>(_a[1]))); break;
        case 4: _t->setImage((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 5: _t->computePreview(); break;
        case 6: _t->reject(); break;
        case 7: { QImage _r = _t->computeUnsharp((*reinterpret_cast< const QImage(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< QImage*>(_a[0]) = _r; }  break;
        case 8: _t->unsharpFinished(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkUnsharpDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkUnsharpDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkUnsharpDialog,
      qt_meta_data_nmc__DkUnsharpDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkUnsharpDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkUnsharpDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkUnsharpDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkUnsharpDialog))
        return static_cast<void*>(const_cast< DkUnsharpDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkUnsharpDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void nmc::DkUnsharpDialog::updateImage(QImage _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkMosaicDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      25,   21,   20,   20, 0x05,
      45,   20,   20,   20, 0x05,
      69,   65,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
      90,   20,   20,   20, 0x0a,
     114,   20,   20,   20, 0x0a,
     145,  136,   20,   20, 0x0a,
     180,  178,   20,   20, 0x0a,
     213,  178,   20,   20, 0x0a,
     247,  178,   20,   20, 0x0a,
     280,  178,   20,   20, 0x0a,
     313,  178,   20,   20, 0x0a,
     347,  178,   20,   20, 0x0a,
     382,  178,   20,   20, 0x0a,
     425,  420,   20,   20, 0x0a,
     444,   20,   20,   20, 0x0a,
     454,   20,   20,   20, 0x0a,
     494,  467,  463,   20, 0x0a,
     543,   20,   20,   20, 0x0a,
     560,   20,   20,   20, 0x0a,
     589,  582,   20,   20, 0x0a,
     621,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkMosaicDialog[] = {
    "nmc::DkMosaicDialog\0\0img\0updateImage(QImage)\0"
    "updateProgress(int)\0msg\0infoMessage(QString)\0"
    "on_openButton_pressed()\0on_dbButton_pressed()\0"
    "filename\0on_fileEdit_textChanged(QString)\0"
    "i\0on_newWidthBox_valueChanged(int)\0"
    "on_newHeightBox_valueChanged(int)\0"
    "on_numPatchesV_valueChanged(int)\0"
    "on_numPatchesH_valueChanged(int)\0"
    "on_darkenSlider_valueChanged(int)\0"
    "on_lightenSlider_valueChanged(int)\0"
    "on_saturationSlider_valueChanged(int)\0"
    "file\0setFile(QFileInfo)\0compute()\0"
    "reject()\0int\0file,filter,suffix,from,to\0"
    "computeMosaic(QFileInfo,QString,QString,int,int)\0"
    "mosaicFinished()\0postProcessFinished()\0"
    "button\0buttonClicked(QAbstractButton*)\0"
    "updatePatchRes()\0"
};

void nmc::DkMosaicDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkMosaicDialog *_t = static_cast<DkMosaicDialog *>(_o);
        switch (_id) {
        case 0: _t->updateImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 1: _t->updateProgress((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->infoMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->on_openButton_pressed(); break;
        case 4: _t->on_dbButton_pressed(); break;
        case 5: _t->on_fileEdit_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->on_newWidthBox_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->on_newHeightBox_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->on_numPatchesV_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->on_numPatchesH_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->on_darkenSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->on_lightenSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->on_saturationSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->setFile((*reinterpret_cast< const QFileInfo(*)>(_a[1]))); break;
        case 14: _t->compute(); break;
        case 15: _t->reject(); break;
        case 16: { int _r = _t->computeMosaic((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 17: _t->mosaicFinished(); break;
        case 18: _t->postProcessFinished(); break;
        case 19: _t->buttonClicked((*reinterpret_cast< QAbstractButton*(*)>(_a[1]))); break;
        case 20: _t->updatePatchRes(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkMosaicDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkMosaicDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkMosaicDialog,
      qt_meta_data_nmc__DkMosaicDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkMosaicDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkMosaicDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkMosaicDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkMosaicDialog))
        return static_cast<void*>(const_cast< DkMosaicDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkMosaicDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void nmc::DkMosaicDialog::updateImage(QImage _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkMosaicDialog::updateProgress(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkMosaicDialog::infoMessage(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_nmc__DkForceThumbDialog[] = {

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

static const char qt_meta_stringdata_nmc__DkForceThumbDialog[] = {
    "nmc::DkForceThumbDialog\0"
};

void nmc::DkForceThumbDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkForceThumbDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkForceThumbDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkForceThumbDialog,
      qt_meta_data_nmc__DkForceThumbDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkForceThumbDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkForceThumbDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkForceThumbDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkForceThumbDialog))
        return static_cast<void*>(const_cast< DkForceThumbDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkForceThumbDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkWelcomeDialog[] = {

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
      22,   21,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkWelcomeDialog[] = {
    "nmc::DkWelcomeDialog\0\0accept()\0"
};

void nmc::DkWelcomeDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkWelcomeDialog *_t = static_cast<DkWelcomeDialog *>(_o);
        switch (_id) {
        case 0: _t->accept(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkWelcomeDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkWelcomeDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkWelcomeDialog,
      qt_meta_data_nmc__DkWelcomeDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkWelcomeDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkWelcomeDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkWelcomeDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkWelcomeDialog))
        return static_cast<void*>(const_cast< DkWelcomeDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkWelcomeDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkArchiveExtractionDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      37,   32,   31,   31, 0x0a,
      64,   58,   31,   31, 0x0a,
      85,   32,   31,   31, 0x0a,
     118,  109,   31,   31, 0x0a,
     139,   31,   31,   31, 0x2a,
     153,   31,   31,   31, 0x0a,
     167,   31,   31,   31, 0x0a,
     177,   31,   31,   31, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkArchiveExtractionDialog[] = {
    "nmc::DkArchiveExtractionDialog\0\0text\0"
    "textChanged(QString)\0state\0"
    "checkbocChecked(int)\0dirTextChanged(QString)\0"
    "filePath\0loadArchive(QString)\0"
    "loadArchive()\0openArchive()\0openDir()\0"
    "accept()\0"
};

void nmc::DkArchiveExtractionDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkArchiveExtractionDialog *_t = static_cast<DkArchiveExtractionDialog *>(_o);
        switch (_id) {
        case 0: _t->textChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->checkbocChecked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->dirTextChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->loadArchive((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->loadArchive(); break;
        case 5: _t->openArchive(); break;
        case 6: _t->openDir(); break;
        case 7: _t->accept(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkArchiveExtractionDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkArchiveExtractionDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkArchiveExtractionDialog,
      qt_meta_data_nmc__DkArchiveExtractionDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkArchiveExtractionDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkArchiveExtractionDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkArchiveExtractionDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkArchiveExtractionDialog))
        return static_cast<void*>(const_cast< DkArchiveExtractionDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkArchiveExtractionDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
