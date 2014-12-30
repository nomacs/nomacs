/****************************************************************************
** Meta object code from reading C++ file 'DkNoMacs.h'
**
** Created: Tue Dec 30 16:47:45 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DkNoMacs.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkNoMacs.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_nmc__DkNomacsOSXEventFilter[] = {

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
      32,   29,   28,   28, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkNomacsOSXEventFilter[] = {
    "nmc::DkNomacsOSXEventFilter\0\0fi\0"
    "loadFile(QFileInfo)\0"
};

void nmc::DkNomacsOSXEventFilter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkNomacsOSXEventFilter *_t = static_cast<DkNomacsOSXEventFilter *>(_o);
        switch (_id) {
        case 0: _t->loadFile((*reinterpret_cast< const QFileInfo(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkNomacsOSXEventFilter::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkNomacsOSXEventFilter::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_nmc__DkNomacsOSXEventFilter,
      qt_meta_data_nmc__DkNomacsOSXEventFilter, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkNomacsOSXEventFilter::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkNomacsOSXEventFilter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkNomacsOSXEventFilter::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkNomacsOSXEventFilter))
        return static_cast<void*>(const_cast< DkNomacsOSXEventFilter*>(this));
    return QObject::qt_metacast(_clname);
}

int nmc::DkNomacsOSXEventFilter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void nmc::DkNomacsOSXEventFilter::loadFile(const QFileInfo & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_nmc__DkNoMacs[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
     117,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   15,   14,   14, 0x05,
      66,   49,   14,   14, 0x05,
     106,   97,   14,   14, 0x05,
     130,   14,   14,   14, 0x05,
     161,   14,   14,   14, 0x05,
     189,   14,   14,   14, 0x05,
     230,   14,   14,   14, 0x05,
     264,   14,   14,   14, 0x05,
     282,  278,   14,   14, 0x05,
     309,   14,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
     338,   14,   14,   14, 0x0a,
     348,   14,   14,   14, 0x0a,
     366,   14,   14,   14, 0x0a,
     383,   14,   14,   14, 0x0a,
     407,   14,   14,   14, 0x0a,
     427,  422,   14,   14, 0x0a,
     446,  422,   14,   14, 0x0a,
     469,  422,   14,   14, 0x0a,
     490,   14,   14,   14, 0x0a,
     514,  422,   14,   14, 0x0a,
     536,   14,   14,   14, 0x2a,
     554,   14,   14,   14, 0x0a,
     564,   14,   14,   14, 0x0a,
     575,   14,   14,   14, 0x0a,
     595,  588,   14,   14, 0x0a,
     615,   14,   14,   14, 0x0a,
     635,  622,   14,   14, 0x0a,
     646,   14,   14,   14, 0x2a,
     661,  653,   14,   14, 0x0a,
     692,   14,   14,   14, 0x0a,
     710,  703,   14,   14, 0x0a,
     727,   14,   14,   14, 0x2a,
     740,   14,   14,   14, 0x0a,
     754,   14,   14,   14, 0x0a,
     781,   14,   14,   14, 0x0a,
     795,   14,   14,   14, 0x0a,
     809,   14,   14,   14, 0x0a,
     837,   14,   14,   14, 0x0a,
     850,   14,   14,   14, 0x0a,
     866,   14,   14,   14, 0x0a,
     879,   14,   14,   14, 0x0a,
     896,   14,   14,   14, 0x0a,
     911,   14,   14,   14, 0x0a,
     925,   14,   14,   14, 0x0a,
     946,  941,   14,   14, 0x0a,
     969,   14,   14,   14, 0x2a,
     998,  983,   14,   14, 0x0a,
    1023,  422,   14,   14, 0x2a,
    1043,  422,   14,   14, 0x0a,
    1061,  422,   14,   14, 0x0a,
    1091,  422,   14,   14, 0x0a,
    1122, 1109,   14,   14, 0x0a,
    1150,   14,   14,   14, 0x0a,
    1178, 1171,   14,   14, 0x0a,
    1201,   14,   14,   14, 0x0a,
    1215,   14,   14,   14, 0x0a,
    1235,   14,   14,   14, 0x0a,
    1247,   14,   14,   14, 0x0a,
    1268, 1264,   14,   14, 0x0a,
    1289,   14,   14,   14, 0x0a,
    1310, 1305,   14,   14, 0x0a,
    1382, 1360,   14,   14, 0x0a,
    1444, 1427,   14,   14, 0x2a,
    1491, 1481,   14,   14, 0x2a,
    1523,  941,   14,   14, 0x2a,
    1549,   14,   14,   14, 0x0a,
    1569,   14,   14,   14, 0x0a,
    1581,   14,   14,   14, 0x0a,
    1595,  588,   14,   14, 0x0a,
    1616,   14,   14,   14, 0x0a,
    1639,   14,   14,   14, 0x0a,
    1658,   14,   14,   14, 0x0a,
    1684, 1679,   14,   14, 0x0a,
    1726, 1701,   14,   14, 0x0a,
    1760,   14,   14,   14, 0x0a,
    1780,   14,   14,   14, 0x0a,
    1813, 1797,   14,   14, 0x0a,
    1853, 1843,   14,   14, 0x0a,
    1884, 1264,   14,   14, 0x2a,
    1911,   14,   14,   14, 0x0a,
    1923,   14,   14,   14, 0x0a,
    1941,   14,   14,   14, 0x0a,
    1963,   14,   14,   14, 0x0a,
    1976,   14,   14,   14, 0x0a,
    1998,   14,   14,   14, 0x0a,
    2018,   14,   14,   14, 0x0a,
    2035,   14,   14,   14, 0x0a,
    2053,   14,   14,   14, 0x0a,
    2067,   14,   14,   14, 0x0a,
    2081,   14,   14,   14, 0x0a,
    2096,   14,   14,   14, 0x0a,
    2124, 2114,   14,   14, 0x0a,
    2160, 2114,   14,   14, 0x0a,
    2194,   14,   14,   14, 0x0a,
    2225, 2210,   14,   14, 0x0a,
    2255,   14,   14,   14, 0x0a,
    2275,   14,   14,   14, 0x0a,
    2303, 2296,   14,   14, 0x0a,
    2330,   14,   14,   14, 0x2a,
    2353,   14,   14,   14, 0x0a,
    2380, 2370,   14,   14, 0x0a,
    2399,   14,   14,   14, 0x0a,
    2420, 2410,   14,   14, 0x0a,
    2452, 2443,   14,   14, 0x0a,
    2470, 2296,   14,   14, 0x0a,
    2495,   14,   14,   14, 0x0a,
    2513,   14,   14,   14, 0x0a,
    2533,   14,   14,   14, 0x0a,
    2553,   14,   14,   14, 0x0a,
    2604, 2577,   14,   14, 0x0a,
    2627, 2577,   14,   14, 0x0a,
    2657,   14,   14,   14, 0x0a,
    2676,   14,   14,   14, 0x0a,
    2701, 2697,   14,   14, 0x0a,
    2720,   14,   14,   14, 0x0a,
    2737,   14,   14,   14, 0x0a,
    2758,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkNoMacs[] = {
    "nmc::DkNoMacs\0\0newTitle\0"
    "sendTitleSignal(QString)\0newRect,overlaid\0"
    "sendPositionSignal(QRect,bool)\0overlaid\0"
    "sendArrangeSignal(bool)\0"
    "synchronizeWithSignal(quint16)\0"
    "stopSynchronizeWithSignal()\0"
    "synchronizeWithServerPortSignal(quint16)\0"
    "synchronizeRemoteControl(quint16)\0"
    "closeSignal()\0img\0saveTempFileSignal(QImage)\0"
    "sendQuitLocalClientsSignal()\0restart()\0"
    "enterFullScreen()\0exitFullScreen()\0"
    "openKeyboardShortcuts()\0openSettings()\0"
    "show\0showExplorer(bool)\0showMetaDataDock(bool)\0"
    "showThumbsDock(bool)\0thumbsDockAreaChanged()\0"
    "showRecentFiles(bool)\0showRecentFiles()\0"
    "openDir()\0openFile()\0renameFile()\0"
    "change\0changeSorting(bool)\0goTo()\0"
    "filterAction\0find(bool)\0find()\0filters\0"
    "updateFilterState(QStringList)\0"
    "saveFile()\0silent\0saveFileAs(bool)\0"
    "saveFileAs()\0saveFileWeb()\0"
    "extractImagesFromArchive()\0trainFormat()\0"
    "resizeImage()\0openImgManipulationDialog()\0"
    "exportTiff()\0computeMosaic()\0deleteFile()\0"
    "openAppManager()\0setWallpaper()\0"
    "printDialog()\0cleanSettings()\0file\0"
    "newInstance(QFileInfo)\0newInstance()\0"
    "show,permanent\0showStatusBar(bool,bool)\0"
    "showStatusBar(bool)\0showMenuBar(bool)\0"
    "showToolbarsTemporarily(bool)\0"
    "showToolbar(bool)\0toolbar,show\0"
    "showToolbar(QToolBar*,bool)\0"
    "showGpsCoordinates()\0action\0"
    "openFileWith(QAction*)\0aboutDialog()\0"
    "openDocumentation()\0bugReport()\0"
    "featureRequest()\0msg\0errorDialog(QString)\0"
    "loadRecursion()\0imgC\0"
    "setWindowTitle(QSharedPointer<DkImageContainerT>)\0"
    "file,size,edited,attr\0"
    "setWindowTitle(QFileInfo,QSize,bool,QString)\0"
    "file,size,edited\0setWindowTitle(QFileInfo,QSize,bool)\0"
    "file,size\0setWindowTitle(QFileInfo,QSize)\0"
    "setWindowTitle(QFileInfo)\0showOpacityDialog()\0"
    "opacityUp()\0opacityDown()\0"
    "changeOpacity(float)\0animateChangeOpacity()\0"
    "animateOpacityUp()\0animateOpacityDown()\0"
    "lock\0lockWindow(bool)\0newRect,opacity,overlaid\0"
    "tcpSetWindowRect(QRect,bool,bool)\0"
    "tcpSendWindowRect()\0tcpSendArrange()\0"
    "connected,local\0newClientConnected(bool,bool)\0"
    "msg,which\0showStatusMessage(QString,int)\0"
    "showStatusMessage(QString)\0copyImage()\0"
    "copyImageBuffer()\0copyPixelColorValue()\0"
    "pasteImage()\0flipImageHorizontal()\0"
    "flipImageVertical()\0normalizeImage()\0"
    "autoAdjustImage()\0unsharpMask()\0"
    "invertImage()\0convertImage()\0"
    "settingsChanged()\0msg,title\0"
    "showUpdaterMessage(QString,QString)\0"
    "showUpdateDialog(QString,QString)\0"
    "performUpdate()\0received,total\0"
    "updateProgress(qint64,qint64)\0"
    "startSetup(QString)\0updateTranslations()\0"
    "enable\0enableNoImageActions(bool)\0"
    "enableNoImageActions()\0checkForUpdate()\0"
    "frameless\0setFrameless(bool)\0fitFrame()\0"
    "recursive\0setRecursiveScan(bool)\0"
    "contrast\0setContrast(bool)\0"
    "enableMovieActions(bool)\0runLoadedPlugin()\0"
    "openPluginManager()\0initPluginManager()\0"
    "runPluginFromShortcut()\0"
    "askForSaving,alreadySaving\0"
    "closePlugin(bool,bool)\0"
    "applyPluginChanges(bool,bool)\0"
    "clearFileHistory()\0clearFolderHistory()\0"
    "url\0downloadFile(QUrl)\0fileDownloaded()\0"
    "computeThumbsBatch()\0onWindowLoaded()\0"
};

void nmc::DkNoMacs::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkNoMacs *_t = static_cast<DkNoMacs *>(_o);
        switch (_id) {
        case 0: _t->sendTitleSignal((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->sendPositionSignal((*reinterpret_cast< QRect(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->sendArrangeSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->synchronizeWithSignal((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 4: _t->stopSynchronizeWithSignal(); break;
        case 5: _t->synchronizeWithServerPortSignal((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 6: _t->synchronizeRemoteControl((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 7: _t->closeSignal(); break;
        case 8: _t->saveTempFileSignal((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 9: _t->sendQuitLocalClientsSignal(); break;
        case 10: _t->restart(); break;
        case 11: _t->enterFullScreen(); break;
        case 12: _t->exitFullScreen(); break;
        case 13: _t->openKeyboardShortcuts(); break;
        case 14: _t->openSettings(); break;
        case 15: _t->showExplorer((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->showMetaDataDock((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: _t->showThumbsDock((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 18: _t->thumbsDockAreaChanged(); break;
        case 19: _t->showRecentFiles((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 20: _t->showRecentFiles(); break;
        case 21: _t->openDir(); break;
        case 22: _t->openFile(); break;
        case 23: _t->renameFile(); break;
        case 24: _t->changeSorting((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 25: _t->goTo(); break;
        case 26: _t->find((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 27: _t->find(); break;
        case 28: _t->updateFilterState((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 29: _t->saveFile(); break;
        case 30: _t->saveFileAs((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 31: _t->saveFileAs(); break;
        case 32: _t->saveFileWeb(); break;
        case 33: _t->extractImagesFromArchive(); break;
        case 34: _t->trainFormat(); break;
        case 35: _t->resizeImage(); break;
        case 36: _t->openImgManipulationDialog(); break;
        case 37: _t->exportTiff(); break;
        case 38: _t->computeMosaic(); break;
        case 39: _t->deleteFile(); break;
        case 40: _t->openAppManager(); break;
        case 41: _t->setWallpaper(); break;
        case 42: _t->printDialog(); break;
        case 43: _t->cleanSettings(); break;
        case 44: _t->newInstance((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 45: _t->newInstance(); break;
        case 46: _t->showStatusBar((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 47: _t->showStatusBar((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 48: _t->showMenuBar((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 49: _t->showToolbarsTemporarily((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 50: _t->showToolbar((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 51: _t->showToolbar((*reinterpret_cast< QToolBar*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 52: _t->showGpsCoordinates(); break;
        case 53: _t->openFileWith((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 54: _t->aboutDialog(); break;
        case 55: _t->openDocumentation(); break;
        case 56: _t->bugReport(); break;
        case 57: _t->featureRequest(); break;
        case 58: _t->errorDialog((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 59: _t->loadRecursion(); break;
        case 60: _t->setWindowTitle((*reinterpret_cast< QSharedPointer<DkImageContainerT>(*)>(_a[1]))); break;
        case 61: _t->setWindowTitle((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< QSize(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 62: _t->setWindowTitle((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< QSize(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 63: _t->setWindowTitle((*reinterpret_cast< QFileInfo(*)>(_a[1])),(*reinterpret_cast< QSize(*)>(_a[2]))); break;
        case 64: _t->setWindowTitle((*reinterpret_cast< QFileInfo(*)>(_a[1]))); break;
        case 65: _t->showOpacityDialog(); break;
        case 66: _t->opacityUp(); break;
        case 67: _t->opacityDown(); break;
        case 68: _t->changeOpacity((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 69: _t->animateChangeOpacity(); break;
        case 70: _t->animateOpacityUp(); break;
        case 71: _t->animateOpacityDown(); break;
        case 72: _t->lockWindow((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 73: _t->tcpSetWindowRect((*reinterpret_cast< QRect(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 74: _t->tcpSendWindowRect(); break;
        case 75: _t->tcpSendArrange(); break;
        case 76: _t->newClientConnected((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 77: _t->showStatusMessage((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 78: _t->showStatusMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 79: _t->copyImage(); break;
        case 80: _t->copyImageBuffer(); break;
        case 81: _t->copyPixelColorValue(); break;
        case 82: _t->pasteImage(); break;
        case 83: _t->flipImageHorizontal(); break;
        case 84: _t->flipImageVertical(); break;
        case 85: _t->normalizeImage(); break;
        case 86: _t->autoAdjustImage(); break;
        case 87: _t->unsharpMask(); break;
        case 88: _t->invertImage(); break;
        case 89: _t->convertImage(); break;
        case 90: _t->settingsChanged(); break;
        case 91: _t->showUpdaterMessage((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 92: _t->showUpdateDialog((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 93: _t->performUpdate(); break;
        case 94: _t->updateProgress((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        case 95: _t->startSetup((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 96: _t->updateTranslations(); break;
        case 97: _t->enableNoImageActions((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 98: _t->enableNoImageActions(); break;
        case 99: _t->checkForUpdate(); break;
        case 100: _t->setFrameless((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 101: _t->fitFrame(); break;
        case 102: _t->setRecursiveScan((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 103: _t->setContrast((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 104: _t->enableMovieActions((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 105: _t->runLoadedPlugin(); break;
        case 106: _t->openPluginManager(); break;
        case 107: _t->initPluginManager(); break;
        case 108: _t->runPluginFromShortcut(); break;
        case 109: _t->closePlugin((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 110: _t->applyPluginChanges((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 111: _t->clearFileHistory(); break;
        case 112: _t->clearFolderHistory(); break;
        case 113: _t->downloadFile((*reinterpret_cast< const QUrl(*)>(_a[1]))); break;
        case 114: _t->fileDownloaded(); break;
        case 115: _t->computeThumbsBatch(); break;
        case 116: _t->onWindowLoaded(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkNoMacs::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkNoMacs::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_nmc__DkNoMacs,
      qt_meta_data_nmc__DkNoMacs, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkNoMacs::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkNoMacs::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkNoMacs::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkNoMacs))
        return static_cast<void*>(const_cast< DkNoMacs*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int nmc::DkNoMacs::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 117)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 117;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkNoMacs::sendTitleSignal(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nmc::DkNoMacs::sendPositionSignal(QRect _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkNoMacs::sendArrangeSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void nmc::DkNoMacs::synchronizeWithSignal(quint16 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void nmc::DkNoMacs::stopSynchronizeWithSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void nmc::DkNoMacs::synchronizeWithServerPortSignal(quint16 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void nmc::DkNoMacs::synchronizeRemoteControl(quint16 _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void nmc::DkNoMacs::closeSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 7, 0);
}

// SIGNAL 8
void nmc::DkNoMacs::saveTempFileSignal(QImage _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void nmc::DkNoMacs::sendQuitLocalClientsSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 9, 0);
}
static const uint qt_meta_data_nmc__DkNoMacsSync[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x05,
      51,   45,   18,   18, 0x05,
      77,   45,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
     104,   18,   18,   18, 0x0a,
     150,  120,   18,   18, 0x0a,
     187,  178,   18,   18, 0x2a,
     210,   45,   18,   18, 0x0a,
     233,   45,   18,   18, 0x0a,
     264,  256,   18,   18, 0x0a,
     285,   45,   18,   18, 0x0a,
     309,   18,   18,   18, 0x0a,
     327,   18,   18,   18, 0x0a,
     363,  347,   18,   18, 0x0a,
     393,   45,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkNoMacsSync[] = {
    "nmc::DkNoMacsSync\0\0clientInitializedSignal()\0"
    "start\0startRCServerSignal(bool)\0"
    "startTCPServerSignal(bool)\0tcpConnectAll()\0"
    "syncMode,connectWithWhiteList\0"
    "tcpChangeSyncMode(int,bool)\0syncMode\0"
    "tcpChangeSyncMode(int)\0tcpRemoteControl(bool)\0"
    "tcpRemoteDisplay(bool)\0connect\0"
    "tcpAutoConnect(bool)\0startUpnpRenderer(bool)\0"
    "settingsChanged()\0clientInitialized()\0"
    "connected,local\0newClientConnected(bool,bool)\0"
    "startTCPServer(bool)\0"
};

void nmc::DkNoMacsSync::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkNoMacsSync *_t = static_cast<DkNoMacsSync *>(_o);
        switch (_id) {
        case 0: _t->clientInitializedSignal(); break;
        case 1: _t->startRCServerSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->startTCPServerSignal((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->tcpConnectAll(); break;
        case 4: _t->tcpChangeSyncMode((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->tcpChangeSyncMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->tcpRemoteControl((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->tcpRemoteDisplay((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->tcpAutoConnect((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->startUpnpRenderer((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->settingsChanged(); break;
        case 11: _t->clientInitialized(); break;
        case 12: _t->newClientConnected((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 13: _t->startTCPServer((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkNoMacsSync::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkNoMacsSync::staticMetaObject = {
    { &DkNoMacs::staticMetaObject, qt_meta_stringdata_nmc__DkNoMacsSync,
      qt_meta_data_nmc__DkNoMacsSync, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkNoMacsSync::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkNoMacsSync::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkNoMacsSync::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkNoMacsSync))
        return static_cast<void*>(const_cast< DkNoMacsSync*>(this));
    return DkNoMacs::qt_metacast(_clname);
}

int nmc::DkNoMacsSync::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkNoMacs::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void nmc::DkNoMacsSync::clientInitializedSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void nmc::DkNoMacsSync::startRCServerSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nmc::DkNoMacsSync::startTCPServerSignal(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_nmc__DkNoMacsIpl[] = {

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

static const char qt_meta_stringdata_nmc__DkNoMacsIpl[] = {
    "nmc::DkNoMacsIpl\0"
};

void nmc::DkNoMacsIpl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkNoMacsIpl::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkNoMacsIpl::staticMetaObject = {
    { &DkNoMacsSync::staticMetaObject, qt_meta_stringdata_nmc__DkNoMacsIpl,
      qt_meta_data_nmc__DkNoMacsIpl, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkNoMacsIpl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkNoMacsIpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkNoMacsIpl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkNoMacsIpl))
        return static_cast<void*>(const_cast< DkNoMacsIpl*>(this));
    return DkNoMacsSync::qt_metacast(_clname);
}

int nmc::DkNoMacsIpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkNoMacsSync::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_nmc__DkNoMacsFrameless[] = {

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
      31,   24,   23,   23, 0x0a,
      58,   23,   23,   23, 0x2a,
      81,   23,   23,   23, 0x0a,
     105,   98,   23,   23, 0x0a,
     127,   23,   23,   23, 0x2a,

       0        // eod
};

static const char qt_meta_stringdata_nmc__DkNoMacsFrameless[] = {
    "nmc::DkNoMacsFrameless\0\0enable\0"
    "enableNoImageActions(bool)\0"
    "enableNoImageActions()\0exitFullScreen()\0"
    "screen\0updateScreenSize(int)\0"
    "updateScreenSize()\0"
};

void nmc::DkNoMacsFrameless::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DkNoMacsFrameless *_t = static_cast<DkNoMacsFrameless *>(_o);
        switch (_id) {
        case 0: _t->enableNoImageActions((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->enableNoImageActions(); break;
        case 2: _t->exitFullScreen(); break;
        case 3: _t->updateScreenSize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->updateScreenSize(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData nmc::DkNoMacsFrameless::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkNoMacsFrameless::staticMetaObject = {
    { &DkNoMacs::staticMetaObject, qt_meta_stringdata_nmc__DkNoMacsFrameless,
      qt_meta_data_nmc__DkNoMacsFrameless, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkNoMacsFrameless::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkNoMacsFrameless::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkNoMacsFrameless::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkNoMacsFrameless))
        return static_cast<void*>(const_cast< DkNoMacsFrameless*>(this));
    return DkNoMacs::qt_metacast(_clname);
}

int nmc::DkNoMacsFrameless::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkNoMacs::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_nmc__DkNoMacsContrast[] = {

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

static const char qt_meta_stringdata_nmc__DkNoMacsContrast[] = {
    "nmc::DkNoMacsContrast\0"
};

void nmc::DkNoMacsContrast::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData nmc::DkNoMacsContrast::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject nmc::DkNoMacsContrast::staticMetaObject = {
    { &DkNoMacsSync::staticMetaObject, qt_meta_stringdata_nmc__DkNoMacsContrast,
      qt_meta_data_nmc__DkNoMacsContrast, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &nmc::DkNoMacsContrast::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *nmc::DkNoMacsContrast::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *nmc::DkNoMacsContrast::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_nmc__DkNoMacsContrast))
        return static_cast<void*>(const_cast< DkNoMacsContrast*>(this));
    return DkNoMacsSync::qt_metacast(_clname);
}

int nmc::DkNoMacsContrast::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DkNoMacsSync::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
