/*******************************************************************************************************
 DkNoMacs.h
 Created on:	21.04.2011

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

 This file is part of nomacs.

 This file is part of nomacs.

 nomacs is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 nomacs is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#pragma once

// Qt
#pragma warning(push, 0) // no warnings from includes - begin
#include <QMainWindow>
#include <QProcess>
#pragma warning(pop) // no warnings from includes - end

#include "DkImageContainer.h"

#ifdef Q_OS_WIN
#pragma warning(disable : 4996)
#pragma warning(disable : 4251) // TODO: remove
#endif

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QFileDialog;
class QProgressDialog;
class QDesktopWidget;
class QLabel;
class QShortcut;

namespace nmc
{

class DkCompressDialog;
class DkOpacityDialog;
class DkUpdateDialog;
class DkForceThumbDialog;
class DkTrainDialog;
#ifdef WITH_QUAZIP
class DkArchiveExtractionDialog;
#endif
class DkExplorer;
class DkMetaDataDock;
class DkEditDock;
class DkHistoryDock;
class DkLogDock;
class DkExportTiffDialog;
class DkUpdater;
class DkInstallUpdater;
class DkTranslationUpdater;
class DkPluginManagerDialog;
class DkThumbsSaver;
class DkPrintPreviewDialog;
class DkBatchContainer;
class DkCentralWidget;
class DkDockWidget;
class DkQuickAccess;
class DkImageLoader;
class DkQuickAccessEdit;
class DkDialogManager;

/*! A custom event filter for OSX.
Its main purpose is to provide Mac OS X "open file from finder"
functionality for nomacs. It *has* to be catched on the
QApplication level in the event().
*/
class DkNomacsOSXEventFilter : public QObject
{
    Q_OBJECT

public:
    DkNomacsOSXEventFilter(QObject *parent = 0);

signals:
    void loadFile(const QString &fi) const;

protected:
    /*! Handle QFileOpenEvent for mac here */
    bool eventFilter(QObject *obj, QEvent *event) override;
};

class DkMenuBar;

enum PluginMenuActions {
    menu_plugin_manager,

    menu_plugins_end, // nothing beyond this point
};

class DllCoreExport DkNoMacs : public QMainWindow
{
    Q_OBJECT

public:
    virtual ~DkNoMacs();

    virtual DkCentralWidget *getTabWidget() const;

    void loadFile(const QString &filePath);

signals:
    void sendArrangeSignal(bool overlaid) const;
    void closeSignal() const;
    void sendQuitLocalClientsSignal() const;

public slots:
    void toggleFullScreen();
    void enterFullScreen();
    void exitFullScreen();
    void toggleDocks(bool hide);
    void restoreDocks();
    void showExplorer(bool show, bool saveSettings = true);
    void showMetaDataDock(bool show, bool saveSettings = true);
    void showEditDock(bool show, bool saveSettings = true);
    void showHistoryDock(bool show, bool saveSettings = true);
    void showLogDock(bool show, bool saveSettings = true);
    void showThumbsDock(bool show);
    void thumbsDockAreaChanged();
    void openDir();
    void openFile();
    void openQuickLaunch();
    void openFileList();
    void saveFileList();
    void changeSorting(bool change);
    void goTo();
    void find(bool filterAction = true);
    void extractImagesFromArchive();
    void trainFormat();
    void exportTiff();
    void cleanSettings();
    void newInstance(const QString &filePath = QString());
    void showMenuBar(bool show);
    void openFileWith(QAction *action);
    void aboutDialog();
    void openDocumentation();
    void bugReport();
    void loadRecursion();
    void setWindowTitle(QSharedPointer<DkImageContainerT> imgC);
    void setWindowTitle(const QString &filePath, const QSize &size = QSize(), bool edited = false, const QString &attr = QString());
    void showOpacityDialog();
    void opacityUp();
    void opacityDown();
    void changeOpacity(float change);
    void animateChangeOpacity();
    void animateOpacityUp();
    void animateOpacityDown();
    void lockWindow(bool lock);
    void tcpSetWindowRect(QRect newRect, bool opacity, bool overlaid);
    void tcpSendWindowRect();
    void tcpSendArrange();
    void newClientConnected(bool connected);
    virtual void settingsChanged();
    void showUpdaterMessage(QString msg, QString title);
    void showUpdateDialog(QString msg, QString title);
    void performUpdate();
    void updateProgress(qint64 received, qint64 total);
    void updateProgressTranslations(qint64 received, qint64 total);
    void startSetup(QString);
    void updateTranslations();
    void checkForUpdate(bool silent = false);
    void startPong() const;
    void fitFrame();
    void setRecursiveScan(bool recursive);
    void openPluginManager();
    void clearFileHistory();
    void clearFolderHistory();
    void restartWithTranslationUpdate();
    void restartWithPseudoColor(bool contrast);
    void restartFrameless(bool frameless);
    void showRecentFilesOnStartUp();

    // batch actions
    void computeThumbsBatch();
    void onWindowLoaded();

protected:
    // mouse events
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    // mouse events
    void moveEvent(QMoveEvent *event) override;

    // window events
    void contextMenuEvent(QContextMenuEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    // bool gestureEvent(QGestureEvent *event);

    // needed to hide menu
    bool mOtherKeyPressed = true;
    QPoint mPosGrabKey;
    bool mOverlaid = false;
    bool mWasMaximized = false;

    // menu
    DkMenuBar *mMenu = 0;
    QMenu *mPluginsMenu = 0;
    QMenu *mSyncMenu = 0;

    QPoint mMousePos;

    // quick access
    DkQuickAccessEdit *mQuickAccessEdit = 0;
    DkQuickAccess *mQuickAccess = 0;

    // file dialog
    QFileDialog *mOpenDialog = 0;
    QFileDialog *mSaveDialog = 0;
    DkOpacityDialog *mOpacityDialog = 0;
    DkUpdateDialog *mUpdateDialog = 0;
    QProgressDialog *mProgressDialog = 0;
    QProgressDialog *mProgressDialogTranslations = 0;
    DkForceThumbDialog *mForceDialog = 0;
    DkTrainDialog *mTrainDialog = 0;
#ifdef WITH_QUAZIP
    DkArchiveExtractionDialog *mArchiveExtractionDialog = 0;
#endif
    DkExplorer *mExplorer = 0;
    DkMetaDataDock *mMetaDataDock = 0;
    DkEditDock *mEditDock = 0;
    DkHistoryDock *mHistoryDock = 0;
    DkLogDock *mLogDock = 0;
    DkDockWidget *mThumbsDock = 0;
    DkExportTiffDialog *mExportTiffDialog = 0;
    DkThumbsSaver *mThumbSaver = 0;

    DkPrintPreviewDialog *mPrintPreviewDialog = 0;

    DkInstallUpdater *mInstallUpdater = 0;
    DkUpdater *mUpdater = 0;
    DkTranslationUpdater *mTranslationUpdater = 0;

    QRect mOldGeometry;
    QProcess mProcess;

    // functions
    DkNoMacs(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    virtual void init();

    virtual void createActions();
    virtual void createMenu();
    virtual void createContextMenu();
    virtual void createStatusBar();

    virtual void readSettings();

    // plugin functions
    bool mPluginMenuCreated = false;
};

class DllCoreExport DkNoMacsSync : public DkNoMacs
{
    Q_OBJECT

public:
    DkNoMacsSync(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    ~DkNoMacsSync();

signals:
    void startRCServerSignal(bool start);

protected:
    // mouse events
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    // gui
    virtual void createActions() override;
};

class DllCoreExport DkNoMacsIpl : public DkNoMacsSync
{
    Q_OBJECT

public:
    DkNoMacsIpl(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
};

class DllCoreExport DkNoMacsFrameless : public DkNoMacs
{
    Q_OBJECT

public:
    DkNoMacsFrameless(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~DkNoMacsFrameless();

public slots:
    void chooseMonitor(bool force);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void createContextMenu() override;
    void paintEvent(QPaintEvent *event) override;
};

class DllCoreExport DkNoMacsContrast : public DkNoMacsSync
{
    Q_OBJECT

public:
    DkNoMacsContrast(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    virtual ~DkNoMacsContrast();
};
}
