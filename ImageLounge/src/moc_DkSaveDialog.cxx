/****************************************************************************
** Meta object code from reading C++ file 'DkSaveDialog.h'
**
** Created: Thu Dec 18 00:26:16 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkSaveDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkSaveDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkTifDialog[] = {

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

static const char qt_meta_stringdata_nmc__DkTifDialog[] = {
    "nmc::DkTifDialog\0"
};

void nmc::DkTifDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkTifDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkTifDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkTifDialog,
      qt_meta_data_nmc__DkTifDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkTifDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkTifDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkTifDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkTifDialog))
        return static_cast<void*>(const_cast< DkTifDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkTifDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkCompressDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      31,   23,   22,   22, 0x0a,
      48,   22,   22,   22, 0x09,
      68,   59,   22,   22, 0x09,
      94,   22,   22,   22, 0x09,
     113,   22,   22,   22, 0x09,
     159,  127,   22,   22, 0x09,
     223,  198,   22,   22, 0x29,
     267,  256,   22,   22, 0x29,
     294,   22,   22,   22, 0x29,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkCompressDialog[] = {
    "nmc::DkCompressDialog\0\0visible\0"
    "setVisible(bool)\0newBgCol()\0lossless\0"
    "losslessCompression(bool)\0changeSizeWeb(int)\0"
    "drawPreview()\0bufferSize,bufferImgSize,factor\0"
    "updateFileSizeLabel(float,QSize,float)\0"
    "bufferSize,bufferImgSize\0"
    "updateFileSizeLabel(float,QSize)\0"
    "bufferSize\0updateFileSizeLabel(float)\0"
    "updateFileSizeLabel()\0"
};

void nmc::DkCompressDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkCompressDialog *_t = static_cast<DkCompressDialog *>(_o);
        switch (_id) {
        case 0: _t->setVisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->newBgCol(); break;
        case 2: _t->losslessCompression((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->changeSizeWeb((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->drawPreview(); break;
        case 5: _t->updateFileSizeLabel((*reinterpret_cast< float(*)>(_a[1])),(*reinterpret_cast< QSize(*)>(_a[2])),(*reinterpret_cast< float(*)>(_a[3]))); break;
        case 6: _t->updateFileSizeLabel((*reinterpret_cast< float(*)>(_a[1])),(*reinterpret_cast< QSize(*)>(_a[2]))); break;
        case 7: _t->updateFileSizeLabel((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 8: _t->updateFileSizeLabel(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkCompressDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkCompressDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_nmc__DkCompressDialog,
      qt_meta_data_nmc__DkCompressDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkCompressDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkCompressDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkCompressDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkCompressDialog))
        return static_cast<void*>(const_cast< DkCompressDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int nmc::DkCompressDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_END_MOC_NAMESPACE
