/*******************************************************************************************************
 DkNoMacs.cpp
 Created on:	21.04.2011

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>

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

#include "DkNoMacs.h"

// my stuff
#include "DkActionManager.h"
#include "DkBatch.h"
#include "DkCentralWidget.h"
#include "DkControlWidget.h"
#include "DkDialog.h"
#include "DkDockWidgets.h"
#include "DkImageContainer.h"
#include "DkImageLoader.h"
#include "DkLogWidget.h"
#include "DkManipulatorWidgets.h"
#include "DkMenu.h"
#include "DkMessageBox.h"
#include "DkMetaData.h"
#include "DkMetaDataWidgets.h"
#include "DkNetwork.h"
#include "DkQuickAccess.h"
#include "DkSettings.h"
#include "DkStatusBar.h"
#include "DkThumbsWidgets.h"
#include "DkTimer.h"
#include "DkToolbars.h"
#include "DkUpdater.h"
#include "DkUtils.h"
#include "DkViewPort.h"
#include "DkWidgets.h"

#ifdef WITH_PLUGINS
#include "DkPluginInterface.h"
#include "DkPluginManager.h"
#endif //  WITH_PLUGINS

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAction>
#include <QBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QResizeEvent>
#include <QStatusBar>
// #include <QPanGesture>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDrag>
#include <QErrorMessage>
#include <QEvent>
#include <QFileInfo>
#include <QInputDialog>
#include <QMimeData>
#include <QNetworkProxyFactory>
#include <QProcess>
#include <QProgressDialog>
#include <QScreen>
#include <QSettings>
#include <QShortcut>
#include <QSplashScreen>
#include <QStringBuilder>
#include <QSysInfo>
#include <QTimer>
#include <QUrlQuery>
#include <QVector2D>
#include <QtGlobal>
#include <qmath.h>
#pragma warning(pop) // no warnings from includes - end

#if defined(Q_OS_WIN) && !defined(SOCK_STREAM)
#include <winsock2.h> // needed since libraw 0.16
#endif

#include <assert.h>

#include <iostream>

namespace nmc
{

DkNomacsOSXEventFilter::DkNomacsOSXEventFilter(QObject *parent)
    : QObject(parent)
{
}

/*! Handle QFileOpenEvent for mac here */
bool DkNomacsOSXEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        emit loadFile(static_cast<QFileOpenEvent *>(event)->file());
        return true;
    }
    return QObject::eventFilter(obj, event);
}

DkNoMacs::DkNoMacs(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    setObjectName("DkNoMacs");
    QMainWindow::setWindowTitle("nomacs | Image Lounge");

    mMenu = new DkMenuBar(this, -1);

    DkActionManager &am = DkActionManager::instance();
    am.createActions(this);
    am.createMenus(mMenu);
    am.enableImageActions(false);

    mOpenDialog = 0;
    mSaveDialog = 0;
    mThumbSaver = 0;
    mOpacityDialog = 0;
    mUpdater = 0;
    mTranslationUpdater = 0;
    mExportTiffDialog = 0;
    mUpdateDialog = 0;
    mProgressDialog = 0;
    mProgressDialogTranslations = 0;
    mForceDialog = 0;
    mTrainDialog = 0;
    mExplorer = 0;
    mMetaDataDock = 0;
    mPrintPreviewDialog = 0;
    mThumbsDock = 0;
    mQuickAccess = 0;
#ifdef WITH_QUAZIP
    mArchiveExtractionDialog = 0;
#endif

    mOldGeometry = geometry();
    mOverlaid = false;

    resize(986, 686); // default window size, fits the settings tab
    setMinimumSize(20, 20);

    //// fun fact
    // double an = pow(3987, 12);
    // double bn = pow(4365, 12);

    // qDebug() << "3987 ^ 12 + 4365 ^ 12 = " << pow(an + bn, 1/12.0) << "^ 12";
    // qDebug() << "Sorry Fermat, but the Simpsons are right.";
}

DkNoMacs::~DkNoMacs()
{
}

void DkNoMacs::init()
{
    // assign icon -> in windows the 32px version
    QString iconPath = ":/nomacs/img/nomacs.svg";

    QIcon nmcIcon = QIcon(iconPath);

    if (!nmcIcon.isNull())
        setWindowIcon(nmcIcon);

    // shortcuts and actions
    createActions();
    createMenu();
    createContextMenu();
    createStatusBar();

    //// TODO - just for android register me as a gesture recognizer
    // grabGesture(Qt::PanGesture);
    // grabGesture(Qt::PinchGesture);
    // grabGesture(Qt::SwipeGesture);

    // load the window at the same position as last time
    readSettings();
    installEventFilter(this);

    if (DkSettings::normalMode(DkSettingsManager::param().app().appMode) != DkSettings::mode_frameless) {
        DkToolBarManager::inst().showDefaultToolBar(DkSettingsManager::param().app().showToolBar);
        showMenuBar(DkSettingsManager::param().app().showMenuBar);
        DkStatusBarManager::instance().show(DkSettingsManager::param().app().showStatusBar);
    }

    // connections to the image loader
    connect(getTabWidget(), &DkCentralWidget::imageUpdatedSignal, this, QOverload<QSharedPointer<DkImageContainerT>>::of(&DkNoMacs::setWindowTitle));
    connect(getTabWidget(), &DkCentralWidget::thumbViewLoadedSignal, this, [this](const QString &path) {
        setWindowTitle(path);
    });

    DkActionManager::instance().enableMovieActions(false);

// clean up nomacs
#ifdef Q_OS_WIN
    if (!nmc::DkSettingsManager::param().global().setupPath.isEmpty()
        && QApplication::applicationVersion() == nmc::DkSettingsManager::param().global().setupVersion) {
        // ask for exists - otherwise we always try to delete it if the user deleted it
        if (!QFileInfo(nmc::DkSettingsManager::param().global().setupPath).exists() || QFile::remove(nmc::DkSettingsManager::param().global().setupPath)) {
            nmc::DkSettingsManager::param().global().setupPath = "";
            nmc::DkSettingsManager::param().global().setupVersion = "";
            nmc::DkSettingsManager::param().save();
        }
    }
#endif // Q_WS_WIN
}

void DkNoMacs::createStatusBar()
{
    setStatusBar(DkStatusBarManager::instance().statusbar());
}

void DkNoMacs::createMenu()
{
    setMenuBar(mMenu);
    DkActionManager &am = DkActionManager::instance();
    mMenu->addMenu(am.fileMenu());
    mMenu->addMenu(am.editMenu());
    mMenu->addMenu(am.manipulatorMenu());
    mMenu->addMenu(am.viewMenu());
    mMenu->addMenu(am.panelMenu());
    mMenu->addMenu(am.toolsMenu());

    // no sync menu in frameless view
    if (DkSettingsManager::param().app().appMode != DkSettings::mode_frameless)
        mMenu->addMenu(am.syncMenu());

#ifdef WITH_PLUGINS
    // empty menu, populated on first click
    mPluginsMenu = mMenu->addMenu(tr("Pl&ugins"));

    // we need a dummy action or else QPA might hide the menu (Mac)
    mPluginsMenu->addAction(new QAction("<Plugins Loaded Here>"));

    am.pluginActionManager()->setMenu(mPluginsMenu);
#endif // WITH_PLUGINS

    mMenu->addMenu(am.helpMenu());

    // makes menu actions available even if menu bar is hidden
    addActions(mMenu->actions());
}

void DkNoMacs::createContextMenu()
{
}

void DkNoMacs::createActions()
{
    DkActionManager &am = DkActionManager::instance();

    connect(am.action(DkActionManager::menu_file_open), &QAction::triggered, this, &DkNoMacs::openFile);
    connect(am.action(DkActionManager::menu_file_open_dir), &QAction::triggered, this, &DkNoMacs::openDir);
    connect(am.action(DkActionManager::menu_file_quick_launch), &QAction::triggered, this, &DkNoMacs::openQuickLaunch);
    connect(am.action(DkActionManager::menu_file_open_list), &QAction::triggered, this, &DkNoMacs::openFileList);
    connect(am.action(DkActionManager::menu_file_save_list), &QAction::triggered, this, &DkNoMacs::saveFileList);
    connect(am.action(DkActionManager::menu_file_rename), &QAction::triggered, getTabWidget(), &DkCentralWidget::renameFile);
    connect(am.action(DkActionManager::menu_file_goto), &QAction::triggered, this, &DkNoMacs::goTo);

    connect(am.action(DkActionManager::menu_file_show_recent), &QAction::triggered, getTabWidget(), &DkCentralWidget::showRecentFiles);
    connect(am.action(DkActionManager::menu_file_new_instance), &QAction::triggered, this, [this]() {
        newInstance();
    });
    connect(am.action(DkActionManager::menu_file_private_instance), &QAction::triggered, this, [this]() {
        newInstance();
    });
    connect(am.action(DkActionManager::menu_file_find), &QAction::triggered, this, &DkNoMacs::find);
    connect(am.action(DkActionManager::menu_file_recursive), &QAction::triggered, this, &DkNoMacs::setRecursiveScan);
    connect(am.action(DkActionManager::menu_file_exit), &QAction::triggered, this, &DkNoMacs::close);

    connect(am.action(DkActionManager::menu_sort_filename), &QAction::triggered, this, &DkNoMacs::changeSorting);
    connect(am.action(DkActionManager::menu_sort_file_size), &QAction::triggered, this, &DkNoMacs::changeSorting);
    connect(am.action(DkActionManager::menu_sort_date_created), &QAction::triggered, this, &DkNoMacs::changeSorting);
    connect(am.action(DkActionManager::menu_sort_date_modified), &QAction::triggered, this, &DkNoMacs::changeSorting);
    connect(am.action(DkActionManager::menu_sort_random), &QAction::triggered, this, &DkNoMacs::changeSorting);
    connect(am.action(DkActionManager::menu_sort_ascending), &QAction::triggered, this, &DkNoMacs::changeSorting);
    connect(am.action(DkActionManager::menu_sort_descending), &QAction::triggered, this, &DkNoMacs::changeSorting);

    connect(am.action(DkActionManager::menu_panel_menu), &QAction::toggled, this, &DkNoMacs::showMenuBar);
    connect(am.action(DkActionManager::menu_panel_explorer), &QAction::toggled, this, [this](bool show) {
        showExplorer(show);
    });
    connect(am.action(DkActionManager::menu_panel_metadata_dock), &QAction::toggled, this, [this](bool show) {
        showMetaDataDock(show);
    });
    connect(am.action(DkActionManager::menu_edit_image), &QAction::toggled, this, [this](bool show) {
        showEditDock(show);
    });
    connect(am.action(DkActionManager::menu_panel_history), &QAction::toggled, this, [this](bool show) {
        showHistoryDock(show);
    });
    connect(am.action(DkActionManager::menu_panel_log), &QAction::toggled, this, [this](bool show) {
        showLogDock(show);
    });
    connect(am.action(DkActionManager::menu_panel_preview), &QAction::toggled, this, &DkNoMacs::showThumbsDock);
    connect(am.action(DkActionManager::menu_panel_toggle), &QAction::toggled, this, &DkNoMacs::toggleDocks);

    connect(am.action(DkActionManager::menu_view_fit_frame), &QAction::triggered, this, &DkNoMacs::fitFrame);
    connect(am.action(DkActionManager::menu_view_fullscreen), &QAction::triggered, this, &DkNoMacs::toggleFullScreen);
    connect(am.action(DkActionManager::menu_view_frameless), &QAction::toggled, this, &DkNoMacs::restartFrameless);
    connect(am.action(DkActionManager::menu_panel_transfertoolbar), &QAction::toggled, this, &DkNoMacs::restartWithPseudoColor);
    connect(am.action(DkActionManager::menu_view_opacity_change), &QAction::triggered, this, &DkNoMacs::showOpacityDialog);
    connect(am.action(DkActionManager::menu_view_opacity_up), &QAction::triggered, this, &DkNoMacs::opacityUp);
    connect(am.action(DkActionManager::menu_view_opacity_down), &QAction::triggered, this, &DkNoMacs::opacityDown);
    connect(am.action(DkActionManager::menu_view_opacity_an), &QAction::triggered, this, &DkNoMacs::animateChangeOpacity);
    connect(am.action(DkActionManager::menu_view_lock_window), &QAction::triggered, this, &DkNoMacs::lockWindow);

    connect(am.action(DkActionManager::menu_tools_thumbs), &QAction::triggered, this, &DkNoMacs::computeThumbsBatch);
    connect(am.action(DkActionManager::menu_tools_filter), &QAction::triggered, this, &DkNoMacs::find);
    connect(am.action(DkActionManager::menu_tools_export_tiff), &QAction::triggered, this, &DkNoMacs::exportTiff);
    connect(am.action(DkActionManager::menu_tools_extract_archive), &QAction::triggered, this, &DkNoMacs::extractImagesFromArchive);
    connect(am.action(DkActionManager::menu_tools_train_format), &QAction::triggered, this, &DkNoMacs::trainFormat);

    connect(am.action(DkActionManager::sc_test_rec), &QAction::triggered, this, &DkNoMacs::loadRecursion);
    connect(am.action(DkActionManager::sc_test_pong), &QAction::triggered, this, &DkNoMacs::startPong);

    connect(am.action(DkActionManager::menu_plugin_manager), &QAction::triggered, this, &DkNoMacs::openPluginManager);

    // help menu
    connect(am.action(DkActionManager::menu_help_about), &QAction::triggered, this, &DkNoMacs::aboutDialog);
    connect(am.action(DkActionManager::menu_help_documentation), &QAction::triggered, this, &DkNoMacs::openDocumentation);
    connect(am.action(DkActionManager::menu_help_bug), &QAction::triggered, this, &DkNoMacs::bugReport);
    connect(am.action(DkActionManager::menu_help_update), &QAction::triggered, this, &DkNoMacs::checkForUpdate);
    connect(am.action(DkActionManager::menu_help_update_translation), &QAction::triggered, this, &DkNoMacs::updateTranslations);

    connect(am.appManager(), &DkAppManager::openFileSignal, this, &DkNoMacs::openFileWith);
}

void DkNoMacs::clearFileHistory()
{
    DkSettingsManager::param().global().recentFiles.clear();
}

void DkNoMacs::clearFolderHistory()
{
    DkSettingsManager::param().global().recentFolders.clear();
}

DkCentralWidget *DkNoMacs::getTabWidget() const
{
    DkCentralWidget *cw = dynamic_cast<DkCentralWidget *>(centralWidget());
    return cw;
}

// Qt how-to
void DkNoMacs::closeEvent(QCloseEvent *event)
{
    DkCentralWidget *cw = static_cast<DkCentralWidget *>(centralWidget());

    if (cw && cw->getTabs().size() > 1) {
        DkMessageBox *msg = new DkMessageBox(QMessageBox::Question,
                                             tr("Quit nomacs"),
                                             tr("Do you want nomacs to save your tabs?"),
                                             (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel),
                                             this);
        msg->setButtonText(QMessageBox::Yes, tr("&Save and Quit"));
        msg->setButtonText(QMessageBox::No, tr("&Quit"));
        msg->setObjectName("saveTabsDialog");

        int answer = msg->exec();

        if (answer == QMessageBox::Cancel || answer == QMessageBox::NoButton) { // User canceled - do not close
            event->ignore();
            return;
        }

        cw->saveSettings(answer == QMessageBox::Yes);
    } else
        cw->saveSettings(false);

    if (!getTabWidget()->requestClose()) {
        // do not close if the user hit cancel in the save changes dialog
        event->ignore();
        return;
    }

    emit closeSignal();
    setVisible(false);

    DefaultSettings settings;

    // TODO: this is temporary until currentAppMode is replaced with appMode
    if (isFullScreen())
        DkSettingsManager::param().app().appMode = DkSettings::fullscreenMode(DkSettingsManager::param().app().appMode);

    if (objectName() != "DkNoMacsFrameless") {
        // the frameless geometry is always the screen rect, don't save
        // NOTE: saveGeometry() does not play well with mode switching, don't use it
        settings.setValue("windowGeometry", normalGeometry());
        settings.setValue("windowMaximized", isMaximized());
        qInfo() << "save geometry:" << normalGeometry() << "windowState:" << windowState();
    }

    settings.setValue("windowState", saveState());

    if (mExplorer)
        settings.setValue(mExplorer->objectName(), QMainWindow::dockWidgetArea(mExplorer));
    if (mMetaDataDock)
        settings.setValue(mMetaDataDock->objectName(), QMainWindow::dockWidgetArea(mMetaDataDock));
    if (mEditDock)
        settings.setValue(mEditDock->objectName(), QMainWindow::dockWidgetArea(mEditDock));
    if (mThumbsDock)
        settings.setValue(mThumbsDock->objectName(), QMainWindow::dockWidgetArea(mThumbsDock));

    nmc::DkSettingsManager::param().save();

    QMainWindow::closeEvent(event);
}

void DkNoMacs::resizeEvent(QResizeEvent *event)
{
    qDebug() << "resize event" << event->size();

    QMainWindow::resizeEvent(event);

    if (!mOverlaid)
        mOldGeometry = geometry();
    else if (windowOpacity() < 1.0f) {
        animateChangeOpacity();
        mOverlaid = false;
    }
}

void DkNoMacs::moveEvent(QMoveEvent *event)
{
    QMainWindow::moveEvent(event);

    if (!mOverlaid)
        mOldGeometry = geometry();
    else if (windowOpacity() < 1.0f) {
        animateChangeOpacity();
        mOverlaid = false;
    }
}

void DkNoMacs::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || (getTabWidget() && !getTabWidget()->getCurrentImage()))
        return;

    if (isFullScreen())
        exitFullScreen();
    else if (DkSettingsManager::instance().param().global().doubleClickForFullscreen)
        enterFullScreen();

    // QMainWindow::mouseDoubleClickEvent(event);
}

void DkNoMacs::mousePressEvent(QMouseEvent *event)
{
    mMousePos = event->pos();

    QMainWindow::mousePressEvent(event);
}

void DkNoMacs::mouseReleaseEvent(QMouseEvent *event)
{
    QMainWindow::mouseReleaseEvent(event);
}

void DkNoMacs::contextMenuEvent(QContextMenuEvent *event)
{
    QMainWindow::contextMenuEvent(event);

    if (!event->isAccepted())
        DkActionManager::instance().contextMenu()->exec(event->globalPos());
}

void DkNoMacs::mouseMoveEvent(QMouseEvent *event)
{
    QMainWindow::mouseMoveEvent(event);
}

// bool DkNoMacs::gestureEvent(QGestureEvent *event) {
//
//	DkViewPort* vp = viewport();
//
//	if (QGesture *swipe = event->gesture(Qt::SwipeGesture)) {
//		QSwipeGesture* swipeG = static_cast<QSwipeGesture *>(swipe);
//
//		qDebug() << "swipe detected\n";
//		if (vp) {
//
//			if (swipeG->horizontalDirection() == QSwipeGesture::Left)
//				vp->loadNextFileFast();
//			else if (swipeG->horizontalDirection() == QSwipeGesture::Right)
//				vp->loadPrevFileFast();
//
//			// TODO: recognize some other gestures please
//		}
//
//	}
//	else if (QGesture *pan = event->gesture(Qt::PanGesture)) {
//
//		QPanGesture* panG = static_cast<QPanGesture *>(pan);
//
//		qDebug() << "you're speedy: " << panG->acceleration();
//
//		QPointF delta = panG->delta();
//
//		if (panG->acceleration() > 10 && delta.x() && fabs(delta.y()/delta.x()) < 0.2) {
//
//			if (delta.x() < 0)
//				vp->loadNextFileFast();
//			else
//				vp->loadPrevFileFast();
//		}
//
//		if (vp)
//			vp->moveView(panG->delta());
//	}
//	else if (QGesture *pinch = event->gesture(Qt::PinchGesture)) {
//
//		QPinchGesture* pinchG = static_cast<QPinchGesture *>(pinch);
//
//		//if (pinchG->changeFlags() == QPinchGesture::ChangeFlag.ScaleFactorChanged) {
//		qDebug() << "scale Factor: " << pinchG->scaleFactor();
//		if (pinchG->scaleFactor() != 0 && vp) {
//			vp->zoom((float)pinchG->scaleFactor());
//		}
//		else if (pinchG->rotationAngle() != 0 && vp) {
//
//			float angle = (float)pinchG->rotationAngle();
//			qDebug() << "angle: " << angle;
//			//vp->rotate(angle);
//		}
//	}
//
//	qDebug() << "gesture event (NoMacs)";
//
//	//	pinchTriggered(static_cast<QPinchGesture *>(pinch));
//	return true;
// }

void DkNoMacs::readSettings()
{
    DefaultSettings settings;

    QRect defaultGeometry = geometry(); // from DkNoMacs()
    defaultGeometry.moveCenter(qApp->primaryScreen()->availableGeometry().center());

    // we don't use restoreGeometry() because it messes up mode switching,
    // the downside is we have to correct bad geometry ourselves
    QRect normalGeometry = settings.value("windowGeometry", defaultGeometry).toRect();

    bool recenter = false;

    const QScreen *screen = qApp->screenAt(normalGeometry.topLeft());
    if (!screen) {
        screen = qApp->primaryScreen();
        recenter = true;
    }

    const QRect screenRect = screen->availableGeometry();

    if (!screenRect.contains(normalGeometry, true)) { // must fit completely
        recenter = true;
        QSize size = normalGeometry.size();
        size.setHeight(qBound(minimumSize().height(), size.height(), screenRect.size().height()));
        size.setWidth(qBound(minimumSize().width(), size.width(), screenRect.size().width()));
        normalGeometry.setSize(size);
    }

    if (recenter)
        normalGeometry.moveCenter(screenRect.center());

    setGeometry(normalGeometry);

    bool wasMaximized = settings.value("windowMaximized", false).toBool();
    if (wasMaximized)
        setWindowState(Qt::WindowMaximized);

    qInfo() << "restore geometry:" << geometry() << "windowState:" << windowState();

    restoreState(settings.value("windowState").toByteArray());

    // restore state makes the toolbar visible - so hide it again...
    if (DkSettingsManager::param().app().appMode == DkSettings::mode_frameless) {
        DkToolBarManager::inst().showDefaultToolBar(false, false);
        DkStatusBarManager::instance().show(false, false);
    }
}

void DkNoMacs::toggleFullScreen()
{
    if (isFullScreen())
        exitFullScreen();
    else
        enterFullScreen();
}

void DkNoMacs::enterFullScreen()
{
    setUpdatesEnabled(false);

    // disable animations for panels to stop layout change immediately after switch
    DkFadeHelper::enableAnimations(false);

    int appMode = DkSettingsManager::param().app().currentAppMode;
    appMode = DkSettings::fullscreenMode(appMode);
    DkSettingsManager::param().app().currentAppMode = appMode;

    menuBar()->hide();
    DkToolBarManager::inst().show(false);
    DkStatusBarManager::instance().statusbar()->hide();
    getTabWidget()->showTabs(false);
    restoreDocks();

    qInfo() << "before enter fullscreen appMode:" << appMode << "geometry:" << geometry() << "normalGeometry:" << normalGeometry()
            << "windowState:" << windowState();

    mWasMaximized = isMaximized();

    if (getTabWidget()->getViewPort())
        getTabWidget()->getViewPort()->setFullScreen(true);

    DkFadeHelper::enableAnimations(true);
    setUpdatesEnabled(true);

    showFullScreen();

    qInfo() << "after enter fullscreen appMode:" << appMode << "geometry:" << geometry() << "normalGeometry:" << normalGeometry()
            << "windowState:" << windowState();

    update();
}

void DkNoMacs::exitFullScreen()
{
    if (isFullScreen()) {
        setUpdatesEnabled(false);
        DkFadeHelper::enableAnimations(false);

        int appMode = DkSettingsManager::param().app().currentAppMode;
        if (!DkSettings::modeIsFullscreen(appMode))
            qWarning() << "expected fullscreen app mode, but got" << appMode;

        appMode = DkSettings::normalMode(appMode);
        DkSettingsManager::param().app().currentAppMode = appMode;

        if (appMode != DkSettings::mode_frameless) {
            if (DkSettingsManager::param().app().showMenuBar)
                mMenu->show();
            if (DkSettingsManager::param().app().showStatusBar)
                DkStatusBarManager::instance().statusbar()->show();
        }

        DkToolBarManager::inst().restore();
        restoreDocks();

        if (getTabWidget())
            getTabWidget()->showTabs(true);

        if (getTabWidget()->getViewPort())
            getTabWidget()->getViewPort()->setFullScreen(false);

        DkFadeHelper::enableAnimations(true);
        setUpdatesEnabled(true);

        qInfo() << "before exit fullscreen appMode:" << appMode << "geometry:" << geometry() << "normalGeometry:" << normalGeometry()
                << "windowState:" << windowState();

        if (mWasMaximized)
            showMaximized();
        else
            showNormal();

        qInfo() << "after exit fullscreen appMode:" << appMode << "geometry:" << geometry() << "normalGeometry:" << normalGeometry()
                << "windowState:" << windowState();

        update(); // if no resize is triggered, the viewport won't change its color
    }
}

void DkNoMacs::toggleDocks(bool hide)
{
    if (hide) {
        showExplorer(false, false);
        showMetaDataDock(false, false);
        showEditDock(false, false);
        showHistoryDock(false, false);
        showLogDock(false, false);
        DkToolBarManager::inst().show(false, false);
        DkStatusBarManager::instance().show(false, false);
    } else {
        restoreDocks();
        DkToolBarManager::inst().restore();
        DkStatusBarManager::instance().show(DkSettingsManager::param().app().showStatusBar, false);
    }
}

void DkNoMacs::restoreDocks()
{
    showExplorer(DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showExplorer), false);
    showMetaDataDock(DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showMetaDataDock), false);
    showEditDock(DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showEditDock), false);
    showHistoryDock(DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showHistoryDock), false);
    showLogDock(DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showLogDock), false);
}

void DkNoMacs::restartFrameless(bool)
{
    QString exe = QApplication::applicationFilePath();
    QStringList args;

    if (objectName() != "DkNoMacsFrameless")
        args << "-m" << "frameless";
    else
        args << "-m" << "default";

    if (isFullScreen())
        args << "-f";

    if (DkSettingsManager::param().app().privateMode)
        args << "-p";

    if (getTabWidget()->getCurrentImage())
        args.append(getTabWidget()->getCurrentImage()->filePath());

    nmc::DkSettingsManager::param().save();

    bool started = mProcess.startDetached(exe, args);

    // close me if the new instance started
    if (started)
        close();

    qDebug() << "frameless arguments: " << args;
}

void DkNoMacs::showRecentFilesOnStartUp()
{
    QTimer::singleShot(100, this, [this]() {
        getTabWidget()->showRecentFiles();
    });
}

void DkNoMacs::startPong() const
{
    QString exe = QApplication::applicationFilePath();
    QStringList args;

    args.append("--pong");

    bool started = mProcess.startDetached(exe, args);
    qDebug() << "pong started: " << started;
}

void DkNoMacs::fitFrame()
{
    if (!getTabWidget()->getViewPort())
        return;

    auto vp = getTabWidget()->getViewPort();

    QRectF viewRect = vp->getImageViewRect();
    QRectF vpRect = vp->geometry();
    QRectF nmRect = frameGeometry();
    QSize frDiff = frameGeometry().size() - geometry().size();

    // compute new size
    QPointF c = nmRect.center();
    nmRect.setSize(nmRect.size() + viewRect.size() - vpRect.size());
    nmRect.moveCenter(c);

    // still fits on screen?
    QScreen *sc = QApplication::primaryScreen();
    QRect screenRect = sc->availableGeometry();
    QRect newGeometry = screenRect.intersected(nmRect.toRect());

    // correct frame
    newGeometry.setSize(newGeometry.size() - frDiff);
    newGeometry.moveTopLeft(newGeometry.topLeft() - frameGeometry().topLeft() + geometry().topLeft());

    setGeometry(newGeometry);

    // reset viewport if we did not clip -> compensates round-off errors
    if (screenRect.contains(nmRect.toRect()))
        vp->resetView();
}

void DkNoMacs::setRecursiveScan(bool recursive)
{
    DkSettingsManager::param().global().scanSubFolders = recursive;

    QSharedPointer<DkImageLoader> loader = getTabWidget()->getCurrentImageLoader();

    if (!loader)
        return;

    if (recursive)
        getTabWidget()->setInfo(tr("Recursive Folder Scan is Now Enabled"));
    else
        getTabWidget()->setInfo(tr("Recursive Folder Scan is Now Disabled"));

    loader->updateSubFolders(loader->getDirPath());
}

void DkNoMacs::showOpacityDialog()
{
    if (!mOpacityDialog) {
        mOpacityDialog = new DkOpacityDialog(this);
        mOpacityDialog->setWindowTitle(tr("Change Opacity"));
    }

    if (mOpacityDialog->exec())
        setWindowOpacity(mOpacityDialog->value() / 100.0f);
}

void DkNoMacs::opacityDown()
{
    changeOpacity(-0.3f);
}

void DkNoMacs::opacityUp()
{
    changeOpacity(0.3f);
}

void DkNoMacs::changeOpacity(float change)
{
    float newO = (float)windowOpacity() + change;
    if (newO > 1)
        newO = 1.0f;
    if (newO < 0.1)
        newO = 0.1f;
    setWindowOpacity(newO);
}

void DkNoMacs::animateOpacityDown()
{
    float newO = (float)windowOpacity() - 0.03f;

    if (newO < 0.3f) {
        setWindowOpacity(0.3f);
        return;
    }

    setWindowOpacity(newO);
    QTimer::singleShot(20, this, &DkNoMacs::animateOpacityDown);
}

void DkNoMacs::animateOpacityUp()
{
    float newO = (float)windowOpacity() + 0.03f;

    if (newO > 1.0f) {
        setWindowOpacity(1.0f);
        return;
    }

    setWindowOpacity(newO);
    QTimer::singleShot(20, this, &DkNoMacs::animateOpacityUp);
}

// >DIR: diem - why can't we put it in mViewport?
void DkNoMacs::animateChangeOpacity()
{
    float newO = (float)windowOpacity();

    if (newO >= 1.0f)
        animateOpacityDown();
    else
        animateOpacityUp();
}

void DkNoMacs::lockWindow(bool lock)
{
#ifdef Q_OS_WIN

    qDebug() << "locking: " << lock;

    if (lock && windowOpacity() < 1.0f) {
        // setAttribute(Qt::WA_TransparentForMouseEvents);
        HWND hwnd = (HWND)winId(); // get handle of the widget
        LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
        SetWindowLong(hwnd, GWL_EXSTYLE, styles | WS_EX_TRANSPARENT);
        SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        auto keyStr = DkActionManager::instance().action(DkActionManager::menu_view_lock_window)->shortcut().toString();
        getTabWidget()->setInfo(tr("Window Locked\nTo unlock: gain focus (ALT+Tab),\nthen press %1").arg(keyStr));
    } else if (lock && windowOpacity() == 1.0f) {
        getTabWidget()->setInfo(tr("You should first reduce opacity\n before working through the window."));
        DkActionManager::instance().action(DkActionManager::menu_view_lock_window)->setChecked(false);
    } else {
        qDebug() << "deactivating...";
        HWND hwnd = (HWND)winId(); // get handle of the widget
        LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
        SetWindowLong(hwnd, GWL_EXSTYLE, styles & ~WS_EX_TRANSPARENT);

        SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
#else
    // TODO: find corresponding command for linux etc

    // setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    // show();
#endif
}

void DkNoMacs::newClientConnected(bool connected)
{
    mOverlaid = false;
    // add methods if clients are connected

    DkActionManager &am = DkActionManager::instance();
    am.action(DkActionManager::menu_sync_view)->setEnabled(connected);
    am.action(DkActionManager::menu_sync_pos)->setEnabled(connected);
    am.action(DkActionManager::menu_sync_arrange)->setEnabled(connected);
}

void DkNoMacs::tcpSetWindowRect(QRect newRect, bool opacity, bool overlaid)
{
    this->mOverlaid = overlaid;

    // we are currently overlaid...
    if (!overlaid) {
        setGeometry(mOldGeometry);
        if (opacity)
            animateOpacityUp();
        mOldGeometry = geometry();
    } else {
#ifdef Q_OS_WIN
        showMinimized();
        setWindowState(Qt::WindowActive);
#else
        Qt::WindowFlags flags = windowFlags();
        setWindowFlags(Qt::WindowStaysOnTopHint); // we need this to 'generally' (for all OSs) bring the window to front
        setWindowFlags(flags); // reset flags
        showNormal();
#endif

        mOldGeometry = geometry();

        this->move(newRect.topLeft());
        this->resize(newRect.size() - (frameGeometry().size() - geometry().size()));

        // setGeometry(newRect);
        if (opacity)
            animateOpacityDown();

        // this->setActiveWindow();
    }
}

void DkNoMacs::tcpSendWindowRect()
{
    mOverlaid = !mOverlaid;

    qDebug() << "overlaying";
    // change my geometry
    tcpSetWindowRect(this->frameGeometry(), !mOverlaid, mOverlaid);

    auto cm = DkSyncManager::inst().client();
    if (cm)
        cm->sendPosition(frameGeometry(), mOverlaid);
}

void DkNoMacs::tcpSendArrange()
{
    mOverlaid = !mOverlaid;
    emit sendArrangeSignal(mOverlaid);
}

void DkNoMacs::showExplorer(bool show, bool saveSettings)
{
    if (!show && !mExplorer)
        return;

    if (!mExplorer) {
        // get last location
#ifdef WIN32
        // don't use the browse explorer on windows
        // see #536
        mExplorer = new DkExplorer(tr("File Explorer"));
#else
        mExplorer = new DkBrowseExplorer(tr("File Explorer"));
#endif
        mExplorer->registerAction(DkActionManager::instance().action(DkActionManager::menu_panel_explorer));
        mExplorer->setDisplaySettings(&DkSettingsManager::param().app().showExplorer);
        addDockWidget(mExplorer->getDockLocationSettings(Qt::LeftDockWidgetArea), mExplorer);

        connect(mExplorer, &DkExplorer::openFile, getTabWidget(), [this](const QString &path) {
            getTabWidget()->load(path);
        });
        connect(mExplorer, &DkExplorer::openDir, getTabWidget(), &DkCentralWidget::load);
        connect(getTabWidget(), &DkCentralWidget::imageUpdatedSignal, mExplorer, &DkExplorer::setCurrentImage);
        connect(getTabWidget(), &DkCentralWidget::thumbViewLoadedSignal, mExplorer, &DkExplorer::setCurrentPath);
    }

    mExplorer->setVisible(show, saveSettings);

    auto imgC = getTabWidget()->getCurrentImage();
    if (imgC && imgC->fileInfo().exists()) {
        mExplorer->setCurrentPath(imgC->fileInfo().path());
    } else {
        QStringList filePaths = DkSettingsManager::param().global().recentFiles;
        if (!filePaths.isEmpty()) {
            DkFileInfo fileInfo(filePaths.first());
            if (DkUtils::tryExists(fileInfo))
                mExplorer->setCurrentPath(fileInfo.path());
        }
    }
}

void DkNoMacs::showMetaDataDock(bool show, bool saveSettings)
{
    if (!show && !mMetaDataDock)
        return;

    if (!mMetaDataDock) {
        mMetaDataDock = new DkMetaDataDock(tr("Meta Data Info"), this);
        mMetaDataDock->registerAction(DkActionManager::instance().action(DkActionManager::menu_panel_metadata_dock));
        mMetaDataDock->setDisplaySettings(&DkSettingsManager::param().app().showMetaDataDock);
        addDockWidget(mMetaDataDock->getDockLocationSettings(Qt::RightDockWidgetArea), mMetaDataDock);

        connect(getTabWidget(), &DkCentralWidget::imageUpdatedSignal, mMetaDataDock, &DkMetaDataDock::setImage);
        connect(getTabWidget(), &DkCentralWidget::thumbViewLoadedSignal, mMetaDataDock, [this]() {
            mMetaDataDock->setImage(nullptr);
        });
    }

    mMetaDataDock->setVisible(show, saveSettings);

    if (getTabWidget()->getCurrentImage())
        mMetaDataDock->setImage(getTabWidget()->getCurrentImage());
}

void DkNoMacs::showEditDock(bool show, bool saveSettings)
{
    if (!show && !mEditDock)
        return;

    if (!mEditDock) {
        mEditDock = new DkEditDock(tr("Edit Image"), this);
        mEditDock->registerAction(DkActionManager::instance().action(DkActionManager::menu_edit_image));
        mEditDock->setDisplaySettings(&DkSettingsManager::param().app().showEditDock);
        addDockWidget(mEditDock->getDockLocationSettings(Qt::RightDockWidgetArea), mEditDock);

        connect(getTabWidget(), &DkCentralWidget::imageUpdatedSignal, mEditDock, &DkEditDock::setImage);
    }

    mEditDock->setVisible(show, saveSettings);

    if (getTabWidget()->getCurrentImage())
        mEditDock->setImage(getTabWidget()->getCurrentImage());
}

void DkNoMacs::showHistoryDock(bool show, bool saveSettings)
{
    if (!show && !mHistoryDock)
        return;

    if (!mHistoryDock) {
        mHistoryDock = new DkHistoryDock(tr("History"), this);
        mHistoryDock->registerAction(DkActionManager::instance().action(DkActionManager::menu_panel_history));
        mHistoryDock->setDisplaySettings(&DkSettingsManager::param().app().showHistoryDock);
        addDockWidget(mHistoryDock->getDockLocationSettings(Qt::RightDockWidgetArea), mHistoryDock);

        connect(getTabWidget(), &DkCentralWidget::imageUpdatedSignal, mHistoryDock, &DkHistoryDock::updateImage);
    }

    mHistoryDock->setVisible(show, saveSettings);

    if (show && getTabWidget()->getCurrentImage())
        mHistoryDock->updateImage(getTabWidget()->getCurrentImage());
}

void DkNoMacs::showLogDock(bool show, bool saveSettings)
{
    if (!show && !mLogDock)
        return;

    if (!mLogDock) {
        // get last location
        mLogDock = new DkLogDock(tr("Console"), this);
        mLogDock->registerAction(DkActionManager::instance().action(DkActionManager::menu_panel_log));
        mLogDock->setDisplaySettings(&DkSettingsManager::param().app().showLogDock);
        addDockWidget(mLogDock->getDockLocationSettings(Qt::LeftDockWidgetArea), mLogDock);
    }

    mLogDock->setVisible(show, saveSettings);
    qInfoClean() << QStringLiteral("Say \"Hi\" to ") << QApplication::applicationName() << " " << QApplication::applicationVersion();
}

void DkNoMacs::showThumbsDock(bool show)
{
    if (!show && !mThumbsDock)
        return;

    // nothing todo here
    if (mThumbsDock && mThumbsDock->isVisible() && show)
        return;

    if (!getTabWidget()->getViewPort())
        return;

    auto vp = getTabWidget()->getViewPort();
    int winPos = vp->getController()->getFilePreview()->getWindowPosition();

    if (winPos != DkFilePreview::cm_pos_dock_hor && winPos != DkFilePreview::cm_pos_dock_ver) {
        if (mThumbsDock) {
            // DkSettingsManager::param().display().thumbDockSize = qMin(thumbsDock->width(), thumbsDock->height());
            DefaultSettings settings;
            settings.setValue("thumbsDockLocation", QMainWindow::dockWidgetArea(mThumbsDock));

            mThumbsDock->hide();
            mThumbsDock->setWidget(0);
            mThumbsDock->deleteLater();
            mThumbsDock = 0;
        }
        return;
    }

    if (!mThumbsDock) {
        mThumbsDock = new DkDockWidget(tr("Thumbnails"), this);
        mThumbsDock->registerAction(DkActionManager::instance().action(DkActionManager::menu_panel_preview));
        mThumbsDock->setDisplaySettings(&DkSettingsManager::param().app().showFilePreview);
        mThumbsDock->setWidget(vp->getController()->getFilePreview());
        addDockWidget(mThumbsDock->getDockLocationSettings(Qt::TopDockWidgetArea), mThumbsDock);
        thumbsDockAreaChanged();

        QLabel *thumbsTitle = new QLabel(mThumbsDock);
        thumbsTitle->setObjectName("thumbsTitle");
        thumbsTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        thumbsTitle->setPixmap(QPixmap(":/nomacs/img/widget-separator.png").scaled(QSize(16, 4)));
        thumbsTitle->setFixedHeight(16);
        mThumbsDock->setTitleBarWidget(thumbsTitle);

        connect(mThumbsDock, &DkDockWidget::dockLocationChanged, this, &DkNoMacs::thumbsDockAreaChanged);
    }

    if (show != mThumbsDock->isVisible())
        mThumbsDock->setVisible(show);
}

void DkNoMacs::thumbsDockAreaChanged()
{
    Qt::DockWidgetArea area = dockWidgetArea(mThumbsDock);

    int thumbsOrientation = DkFilePreview::cm_pos_dock_hor;

    if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
        thumbsOrientation = DkFilePreview::cm_pos_dock_ver;

    if (getTabWidget()->getViewPort())
        getTabWidget()->getViewPort()->getController()->getFilePreview()->setWindowPosition(thumbsOrientation);
}

void DkNoMacs::openDir()
{
    // load system default open dialog
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        tr("Open an Image Directory"),
                                                        getTabWidget()->getCurrentDir(),
                                                        QFileDialog::ShowDirsOnly | DkDialog::fileDialogOptions());

    if (dirName.isEmpty())
        return;

    getTabWidget()->load(dirName);
}

void DkNoMacs::openFile()
{
    QStringList openFilters = DkSettingsManager::param().app().openFilters;
    openFilters.pop_front();
    openFilters.prepend(tr("All Files (*.*)"));

    // load system default open dialog
    QStringList filePaths =
        QFileDialog::getOpenFileNames(this, tr("Open Image"), getTabWidget()->getCurrentDir(), openFilters.join(";;"), nullptr, DkDialog::fileDialogOptions());

    if (filePaths.isEmpty())
        return;

    int count = getTabWidget()->getTabs().count(); // Save current count of tabs for setting tab position later
    if (getTabWidget()->getTabs().at(0)->getMode() == DkTabInfo::tab_empty)
        count = 0;

    QSet<QString> duplicates;
    for (const QString &fp : filePaths) {
        bool dup = false;

        if (DkSettingsManager::param().global().checkOpenDuplicates) { // Should we check for duplicates?
            for (auto tab : getTabWidget()->getTabs()) {
                if (tab->getFilePath().compare(fp) == 0) {
                    duplicates.insert(tab->getFilePath());
                    dup = true;
                    break;
                }
            }
        }

        if (!dup) {
            // > 1: only open in tab if more than one file is opened
            bool newTab = (filePaths.size() > 1) || (getTabWidget()->getTabs().size() > 1);
            if (newTab)
                getTabWidget()->loadToTab(fp);
            else
                getTabWidget()->load(fp);
        }
    }
    if (duplicates.count() > 0) { // Show message if at least one duplicate was found
        QString duptext = tr("The following duplicates were not opened:");
        for (auto dup : duplicates) {
            duptext.append("\n" + dup);
        }
        getTabWidget()->getViewPort()->getController()->setInfo(duptext);
    }

    if (filePaths.count() > duplicates.count()) // Only set the active tab if there is actually something added
        getTabWidget()->setActiveTab(count); // Set first file opened to be the active tab
}

void DkNoMacs::openFileList()
{
    QStringList openFilters;
    openFilters.append(tr("Text file (*.txt)"));
    openFilters.append(tr("All files (*.*)"));

    // load system default open dialog
    QString fileName =
        QFileDialog::getOpenFileName(this, tr("Open Tabs"), getTabWidget()->getCurrentDir(), openFilters.join(";;"), nullptr, DkDialog::fileDialogOptions());

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    while (!file.atEnd()) {
        QString filePath = file.readLine().trimmed();
        if (filePath.isEmpty())
            continue;
        if (!DkFileInfo(filePath).exists()) {
            qWarning() << "[open file list] file does not exist:" << filePath;
            continue;
        }
        getTabWidget()->loadToTab(filePath);
    }
}

void DkNoMacs::saveFileList()
{
    QStringList saveFilters;
    saveFilters.append(tr("Text file (*.txt)"));
    saveFilters.append(tr("All files (*.*)"));

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Tab List"),
                                                    getTabWidget()->getCurrentDir(),
                                                    saveFilters.join(";;"),
                                                    nullptr,
                                                    DkDialog::fileDialogOptions());

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
        return;

    for (auto tab : getTabWidget()->getTabs()) {
        file.write(tab->getFilePath().toUtf8() + "\n");
    }

    file.close();
}

void DkNoMacs::openQuickLaunch()
{
    DkMainToolBar *tb = DkToolBarManager::inst().defaultToolBar();

    // create new model
    if (!mQuickAccess) {
        mQuickAccess = new DkQuickAccess(this);

        // add all actions
        mQuickAccess->addActions(DkActionManager::instance().allActions());

        connect(mQuickAccess, &DkQuickAccess::loadFileSignal, this, [this](const QString &path) {
            getTabWidget()->load(path);
        });
    }

    if (tb)
        connect(tb->getQuickAccess(), &DkQuickAccessEdit::executeSignal, mQuickAccess, &DkQuickAccess::execute, Qt::UniqueConnection);

    mQuickAccess->addDirs(DkSettingsManager::param().global().recentFolders);
    mQuickAccess->addFiles(DkSettingsManager::param().global().recentFiles);

    if (tb && tb->isVisible())
        tb->setQuickAccessModel(mQuickAccess->getModel());
    else {
        if (!mQuickAccessEdit) {
            mQuickAccessEdit = new DkQuickAccessEdit(this);
            connect(mQuickAccessEdit, &DkQuickAccessEdit::executeSignal, mQuickAccess, &DkQuickAccess::execute);
        }

        int right = getTabWidget()->geometry().right();
        mQuickAccessEdit->setFixedWidth(qRound(width() / 3.0f));
        mQuickAccessEdit->move(QPoint(right - mQuickAccessEdit->width() - 10, qRound(height() * 0.25)));
        mQuickAccessEdit->setModel(mQuickAccess->getModel());
        mQuickAccessEdit->show();
    }
}

void DkNoMacs::loadFile(const QString &filePath)
{
    if (!getTabWidget())
        return;

    getTabWidget()->load(filePath);
}

void DkNoMacs::find(bool filterAction)
{
    if (!getTabWidget()->getCurrentImageLoader())
        return;

    if (filterAction) {
        int db = (QObject::sender() == DkActionManager::instance().action(DkActionManager::menu_tools_filter)) ? DkSearchDialog::filter_button
                                                                                                               : DkSearchDialog::find_button;
        qDebug() << "default button: " << db;
        DkSearchDialog *searchDialog = new DkSearchDialog(this);
        searchDialog->setDefaultButton(db);

        searchDialog->setFiles(getTabWidget()->getCurrentImageLoader()->getFileNames());
        searchDialog->setPath(getTabWidget()->getCurrentImageLoader()->getDirPath());

        connect(searchDialog, &DkSearchDialog::filterSignal, getTabWidget()->getCurrentImageLoader().data(), &DkImageLoader::setFolderFilter);
        connect(searchDialog, &DkSearchDialog::loadFileSignal, this, [this](const QString &path) {
            getTabWidget()->load(path);
        });
        int answer = searchDialog->exec();

        DkActionManager::instance().action(DkActionManager::menu_tools_filter)->setChecked(answer == DkSearchDialog::filter_button);
    } else {
        // remove the filter
        getTabWidget()->getCurrentImageLoader()->setFolderFilter(QString());
    }
}

void DkNoMacs::changeSorting(bool change)
{
    // TODO: move to image loader?!

    if (change) {
        QString senderName = QObject::sender()->objectName();

        if (senderName == "menu_sort_filename")
            DkSettingsManager::param().global().sortMode = DkSettings::sort_filename;
        else if (senderName == "menu_sort_file_size")
            DkSettingsManager::param().global().sortMode = DkSettings::sort_file_size;
        else if (senderName == "menu_sort_date_created")
            DkSettingsManager::param().global().sortMode = DkSettings::sort_date_created;
        else if (senderName == "menu_sort_date_modified")
            DkSettingsManager::param().global().sortMode = DkSettings::sort_date_modified;
        else if (senderName == "menu_sort_random")
            DkSettingsManager::param().global().sortMode = DkSettings::sort_random;
        else if (senderName == "menu_sort_ascending")
            DkSettingsManager::param().global().sortDir = DkSettings::sort_ascending;
        else if (senderName == "menu_sort_descending")
            DkSettingsManager::param().global().sortDir = DkSettings::sort_descending;

        if (getTabWidget()->getCurrentImageLoader())
            getTabWidget()->getCurrentImageLoader()->sort();
    }

    QVector<QAction *> sortActions = DkActionManager::instance().sortActions();
    for (int idx = 0; idx < sortActions.size(); idx++) {
        if (idx < DkActionManager::menu_sort_ascending)
            sortActions[idx]->setChecked(idx == DkSettingsManager::param().global().sortMode);
        else if (idx >= DkActionManager::menu_sort_ascending)
            sortActions[idx]->setChecked(idx - DkActionManager::menu_sort_ascending == DkSettingsManager::param().global().sortDir);
    }
}

void DkNoMacs::goTo()
{
    if (!getTabWidget()->getCurrentImageLoader())
        return;

    QSharedPointer<DkImageLoader> loader = getTabWidget()->getCurrentImageLoader();

    bool ok = false;
    int fileIdx = QInputDialog::getInt(this, tr("Go To Image"), tr("Image Index:"), 1, 1, loader->numFiles(), 1, &ok);

    if (ok)
        loader->loadFileAt(fileIdx - 1);
}

void DkNoMacs::trainFormat()
{
    if (!mTrainDialog)
        mTrainDialog = new DkTrainDialog(this);

    mTrainDialog->setCurrentFile(getTabWidget()->getCurrentFilePath());
    bool okPressed = mTrainDialog->exec() != 0;

    if (okPressed && getTabWidget()->getCurrentImageLoader()) {
        getTabWidget()->getCurrentImageLoader()->load(mTrainDialog->getAcceptedFile());
        getTabWidget()->restart(); // quick & dirty, but currently he messes up the filteredFileList if the same folder was already loaded
    }
}

void DkNoMacs::extractImagesFromArchive()
{
#ifdef WITH_QUAZIP
    if (!mArchiveExtractionDialog)
        mArchiveExtractionDialog = new DkArchiveExtractionDialog(this);

    auto *tab = getTabWidget();
    auto imgC = tab->getCurrentImage();
    if (imgC && imgC->fileInfo().isFromZip())
        mArchiveExtractionDialog->setCurrentFile(imgC->dirPath(), true);
    else
        mArchiveExtractionDialog->setCurrentFile(tab->getCurrentFilePath(), false);

    mArchiveExtractionDialog->exec();
#endif
}

void DkNoMacs::exportTiff()
{
#ifdef WITH_LIBTIFF
    if (!mExportTiffDialog)
        mExportTiffDialog = new DkExportTiffDialog(this);

    mExportTiffDialog->setFile(getTabWidget()->getCurrentFilePath());
    mExportTiffDialog->exec();
#endif
}

void DkNoMacs::computeThumbsBatch()
{
    if (!mForceDialog)
        mForceDialog = new DkForceThumbDialog(this);
    mForceDialog->setWindowTitle(tr("Save Thumbnails"));
    mForceDialog->setDir(getTabWidget()->getCurrentDir());

    if (!mForceDialog->exec())
        return;

    if (!mThumbSaver)
        mThumbSaver = new DkThumbsSaver(this);

    if (getTabWidget()->getCurrentImageLoader())
        mThumbSaver->processDir(getTabWidget()->getCurrentImageLoader()->getImages(), mForceDialog->forceSave());
}

void DkNoMacs::aboutDialog()
{
    DkSplashScreen splash;
    splash.exec();
}

void DkNoMacs::openDocumentation()
{
    QString url = QString("https://nomacs.org/docs/getting-started/introduction/");
    QDesktopServices::openUrl(QUrl(url));
}

void DkNoMacs::bugReport()
{
    QString info = R"(
**Description**
A clear and concise description.

**Steps to Reproduce**
List the sequence of actions leading to the bug.

**Expected Behavior**
A clear and concise description of what you expected to happen.

**Screenshot / Screencast / Images**
Add screen recording or example files to help illustrate the problem.

**Additional context**
Add any other context about the problem.

**System Info**)";
    info += '\n' + DkUtils::getBuildInfo().replace("\n\n", "");

    QUrlQuery query;
    query.addQueryItem("body", info);
    QUrl url("https://github.com/nomacs/nomacs/issues/new");
    url.setQuery(query);

    QDesktopServices::openUrl(url);
}

void DkNoMacs::cleanSettings()
{
    DefaultSettings settings;
    settings.clear();

    readSettings();
    resize(400, 225);
    move(100, 100);
}

void DkNoMacs::newInstance(const QString &filePath)
{
    QString exe = QApplication::applicationFilePath();
    QStringList args;

    QAction *a = static_cast<QAction *>(sender());

    if (a && a == DkActionManager::instance().action(DkActionManager::menu_file_private_instance))
        args.append("-p");

    if (filePath.isEmpty())
        args.append(getTabWidget()->getCurrentFilePath());
    else
        args.append(filePath);

    DkSettingsManager::param().app().appMode = DkSettingsManager::param().app().currentAppMode;
    DkSettingsManager::param().save();

    QProcess::startDetached(exe, args);
}

void tagWall(const std::list<std::string> &code)
{
    for (auto line : code)
        std::cout << line << std::endl;
}

void DkNoMacs::loadRecursion()
{
    std::list<std::string> code;
    code.push_back("void tagWall(const std::list<std::string>& code) {");
    code.push_back("	for (auto line : code)");
    code.push_back("		std::cout << line << std::endl;");
    code.push_back("}");
    tagWall(code);

    QImage img = grab().toImage();

    if (getTabWidget()->getViewPort())
        getTabWidget()->getViewPort()->setImage(img);
}

// Added by fabian for transfer function:

void DkNoMacs::restartWithPseudoColor(bool contrast)
{
    qDebug() << "contrast: " << contrast;

    QString exe = QApplication::applicationFilePath();
    QStringList args;

    if (contrast)
        args << "-m" << "pseudocolor";
    else
        args << "-m" << "default";

    if (isFullScreen())
        args << "-f";

    if (DkSettingsManager::param().app().privateMode)
        args << "-p";

    args.append(getTabWidget()->getCurrentFilePath());

    bool started = mProcess.startDetached(exe, args);

    // close me if the new instance started
    if (started)
        close();

    qDebug() << "contrast arguments: " << args;
}

void DkNoMacs::onWindowLoaded()
{
    DefaultSettings settings;
    bool firstTime = settings.value("AppSettings/firstTime.nomacs.3", true).toBool();

    if (DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showExplorer))
        showExplorer(true);
    if (DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showMetaDataDock))
        showMetaDataDock(true);
    if (DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showEditDock))
        showEditDock(true);
    if (DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showHistoryDock))
        showHistoryDock(true);
    if (DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showLogDock))
        showLogDock(true);

    if (firstTime) {
        // here are some first time requests
        DkWelcomeDialog *wecomeDialog = new DkWelcomeDialog(this);
        wecomeDialog->exec();

        settings.setValue("AppSettings/firstTime.nomacs.3", false);

        if (wecomeDialog->isLanguageChanged()) {
            restartWithTranslationUpdate();
        }
    }

    checkForUpdate(true);

    // load settings AFTER everything is initialized
    getTabWidget()->loadSettings();

    toggleDocks(DkSettingsManager::param().app().hideAllPanels);
}

void DkNoMacs::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt) {
        mPosGrabKey = QCursor::pos();
        mOtherKeyPressed = false;
    } else
        mOtherKeyPressed = true;
}

void DkNoMacs::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt && !mOtherKeyPressed && (mPosGrabKey - QCursor::pos()).manhattanLength() == 0)
        mMenu->showMenu();
}

// >DIR diem: eating shortcut overrides (this allows us to use navigation keys like arrows)
bool DkNoMacs::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // consume esc key if fullscreen is on
        if (keyEvent->key() == Qt::Key_Escape && isFullScreen()) {
            exitFullScreen();
            return true;
        } else if (keyEvent->key() == Qt::Key_Escape && DkSettingsManager::param().app().closeOnEsc)
            close();
    }
    // if (event->type() == QEvent::Gesture) {
    //	return gestureEvent(static_cast<QGestureEvent*>(event));
    // }

    return false;
}

void DkNoMacs::showMenuBar(bool show)
{
    DkSettingsManager::param().app().showMenuBar = show;

    QAction *mp = DkActionManager::instance().action(DkActionManager::menu_panel_menu);
    mp->blockSignals(true);
    mp->setChecked(DkSettingsManager::param().app().showMenuBar);
    mp->blockSignals(false);

    int tts = (DkSettingsManager::param().app().showMenuBar) ? -1 : 5000;
    mMenu->setTimeToShow(tts);

    if (show)
        mMenu->showMenu();
    else if (!show)
        mMenu->hide();
}

void DkNoMacs::openFileWith(QAction *action)
{
    if (!action)
        return;

    QFileInfo app(action->toolTip());

    if (!app.exists())
        getTabWidget()->setInfo("Sorry, " % app.fileName() % " does not exist");

    QStringList args;

    // open the file location, not the file itself
    bool openLocation = app.fileName() == "explorer.exe";

    DkFileInfo fileInfo(getTabWidget()->getCurrentFilePath());
    if (!openLocation && fileInfo.isFromZip()) {
        // copy to temporary file
        QString tmpFilePath = DkUtils::getTemporaryFilePath(fileInfo.baseName(), fileInfo.suffix());
        if (tmpFilePath.isEmpty())
            return;

        QFile tmpFile(tmpFilePath);
        if (!tmpFile.open(QFile::WriteOnly))
            return;

        auto io = fileInfo.getIODevice();
        if (!io)
            return;

        QByteArray data = io->readAll();
        if (data.size() != tmpFile.write(data))
            return;

        fileInfo = DkFileInfo(tmpFilePath);
    }
    QString filePath = fileInfo.path();
    if (openLocation)
        args << "/select," << QDir::toNativeSeparators(fileInfo.isFromZip() ? fileInfo.dirPath() : filePath);
    else if (app.fileName().toLower() == "outlook.exe") {
        args << "/a" << QDir::toNativeSeparators(filePath);
    } else
        args << QDir::toNativeSeparators(filePath);

    bool started = mProcess.startDetached(app.absoluteFilePath(), args);

    if (started)
        qDebug() << "starting: " << app.fileName() << args;
    else
        getTabWidget()->setInfo("Sorry, I could not start: " % app.absoluteFilePath());
}

void DkNoMacs::setWindowTitle(QSharedPointer<DkImageContainerT> imgC)
{
    if (!imgC) {
        setWindowTitle(QString());
        return;
    }

    setWindowTitle(imgC->filePath(), imgC->image().size(), imgC->isEdited(), imgC->getTitleAttribute());
}

void DkNoMacs::setWindowTitle(const QString &filePath, const QSize &size, bool edited, const QString &attr)
{
    QString title;

    if (DkSettingsManager::param().global().extendedTabs && (getTabWidget()->getTabs().count() > 1)) {
        title.append(QString::number(getTabWidget()->getActiveTab() + 1) + "/" + QString::number(getTabWidget()->getTabs().count()) + " - ");
    }

    QFileInfo fInfo(filePath);
    title.append(QFileInfo(filePath).fileName());
    title = title.remove(".lnk");

    if (filePath.isEmpty()) {
        title = "nomacs - Image Lounge";
        if (DkSettingsManager::param().app().privateMode)
            title.append(tr(" [Private Mode]"));
    }

    if (edited)
        title.append("[*]");

    title.append(" ");
    title.append(attr); // append some attributes

    QString attributes;
    auto vp = getTabWidget()->getViewPort();

    if (!size.isEmpty())
        attributes = QString::asprintf(" - %i x %i", size.width(), size.height());
    if (size.isEmpty() && vp && !vp->getImageSize().isEmpty())
        attributes = QString::asprintf(" - %i x %i", vp->getImage().width(), vp->getImage().height());
    if (DkSettingsManager::param().app().privateMode)
        attributes.append(tr(" [Private Mode]"));

    QMainWindow::setWindowTitle(title.append(attributes));
    setWindowFilePath(filePath);
    setWindowModified(edited);

    auto cm = DkSyncManager::inst().client();
    if (cm)
        cm->sendTitle(windowTitle());

    // TODO: move!
    DkStatusBar *bar = DkStatusBarManager::instance().statusbar();

    if (fInfo.isDir()) {
        bar->setMessage("", DkStatusBar::status_time_info); // hide label
        bar->setMessage("", DkStatusBar::status_filesize_info);
        bar->setMessage("", DkStatusBar::status_filenumber_info);
    } else {
        if (((vp && !vp->getController()->getFileInfoLabel()->isVisible())
             || !DkSettingsManager::param().slideShow().display.testBit(DkSettings::display_creation_date))
            && getTabWidget()->getCurrentImage()) {
            // create statusbar info
            QSharedPointer<DkMetaDataT> metaData = getTabWidget()->getCurrentImage()->getMetaData();
            QString dateString = metaData->getExifValue("DateTimeOriginal");
            dateString = DkUtils::convertDateString(dateString, fInfo);
            bar->setMessage(dateString, DkStatusBar::status_time_info);
        } else
            bar->setMessage("", DkStatusBar::status_time_info); // hide label

        if (fInfo.exists())
            bar->setMessage(DkUtils::readableByte(fInfo.size()), DkStatusBar::status_filesize_info);
        else
            bar->setMessage("", DkStatusBar::status_filesize_info);
    }
}

void DkNoMacs::settingsChanged()
{
    if (!isFullScreen()) {
        showMenuBar(DkSettingsManager::param().app().showMenuBar);

        DkToolBarManager::inst().restore();
        DkStatusBarManager::instance().show(DkSettingsManager::param().app().showStatusBar);
    }
}

void DkNoMacs::checkForUpdate(bool silent)
{
    // updates are supported on windows only
#ifndef Q_OS_LINUX

    // do we really need to check for update?
    if (!silent
        || (!DkSettingsManager::param().sync().updateDialogShown && QDate::currentDate() > DkSettingsManager::param().sync().lastUpdateCheck
            && DkSettingsManager::param().sync().checkForUpdates)) {
        DkTimer dt;

        if (!mUpdater) {
            mUpdater = new DkUpdater(this);
            connect(mUpdater, &DkUpdater::displayUpdateDialog, this, &DkNoMacs::showUpdateDialog);
            connect(mUpdater, &DkUpdater::showUpdaterMessage, this, &DkNoMacs::showUpdaterMessage);
        }
        mUpdater->silent = silent;
        mUpdater->checkForUpdates();
        qDebug() << "checking for updates takes: " << dt;
    }
#endif // !#ifndef Q_OS_LINUX
}

void DkNoMacs::showUpdaterMessage(QString msg, QString title)
{
    QMessageBox infoDialog(this);
    infoDialog.setWindowTitle(title);
    infoDialog.setIcon(QMessageBox::Information);
    infoDialog.setText(msg);
    infoDialog.show();

    infoDialog.exec();
}

void DkNoMacs::showUpdateDialog(QString msg, QString title)
{
    if (mProgressDialog != 0 && !mProgressDialog->isHidden()) { // check if the progress bar is already open
        showUpdaterMessage(tr("Already downloading update"), "update");
        return;
    }

    DkSettingsManager::param().sync().updateDialogShown = true;
    DkSettingsManager::param().save();

    if (!mUpdateDialog) {
        mUpdateDialog = new DkUpdateDialog(this);
        mUpdateDialog->setWindowTitle(title);
        mUpdateDialog->upperLabel->setText(msg);
        connect(mUpdateDialog, &DkUpdateDialog::startUpdate, this, &DkNoMacs::performUpdate);
    }

    mUpdateDialog->exec();
}

void DkNoMacs::performUpdate()
{
    if (!mUpdater) {
        qDebug() << "WARNING updater is NULL where it should not be.";
        return;
    }

    mUpdater->performUpdate();

    if (!mProgressDialog) {
        mProgressDialog = new QProgressDialog(tr("Downloading update..."), tr("Cancel Update"), 0, 100, this);
        mProgressDialog->setWindowIcon(windowIcon());
        connect(mProgressDialog, &QProgressDialog::canceled, mUpdater, &DkUpdater::cancelUpdate);
        connect(mUpdater, &DkUpdater::downloadProgress, this, &DkNoMacs::updateProgress);
        connect(mUpdater, &DkUpdater::downloadFinished, mProgressDialog, &QProgressDialog::close);
        connect(mUpdater, &DkUpdater::downloadFinished, this, &DkNoMacs::startSetup);
    }
    // mProgressDialog->setWindowModality(Qt::ApplicationModal);

    mProgressDialog->show();
    // progressDialog->raise();
    // progressDialog->activateWindow();
    // mProgressDialog->setWindowModality(Qt::NonModal);
}

void DkNoMacs::updateProgress(qint64 received, qint64 total)
{
    mProgressDialog->setMaximum((int)total);
    mProgressDialog->setValue((int)received);
}

void DkNoMacs::updateProgressTranslations(qint64 received, qint64 total)
{
    qDebug() << "rec:" << received << "  total:" << total;
    mProgressDialogTranslations->setMaximum((int)total);
    mProgressDialogTranslations->setValue((int)received);
}

void DkNoMacs::startSetup(QString filePath)
{
    qDebug() << "starting setup filePath:" << filePath;

    if (!QFile::exists(filePath))
        qDebug() << "file does not exist";
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filePath))) {
        QString msg = tr("Unable to install new version<br>") + tr("You can download the new version from our web page")
            + "<br><a href=\"https://nomacs.org/docs/getting-started/installation/\">www.nomacs.org</a><br>";
        showUpdaterMessage(msg, "update");
    }
}

void DkNoMacs::updateTranslations()
{
    if (!mTranslationUpdater) {
        mTranslationUpdater = new DkTranslationUpdater(false, this);
        connect(mTranslationUpdater, &DkTranslationUpdater::showUpdaterMessage, this, &DkNoMacs::showUpdaterMessage);
    }

    if (!mProgressDialogTranslations) {
        mProgressDialogTranslations = new QProgressDialog(tr("Downloading new translations..."), tr("Cancel"), 0, 100, this);
        mProgressDialogTranslations->setWindowIcon(windowIcon());
        connect(mProgressDialogTranslations, &QProgressDialog::canceled, mTranslationUpdater, &DkTranslationUpdater::cancelUpdate);
        connect(mTranslationUpdater, &DkTranslationUpdater::downloadProgress, this, &DkNoMacs::updateProgressTranslations);
        connect(mTranslationUpdater, &DkTranslationUpdater::downloadFinished, mProgressDialogTranslations, &QProgressDialog::close);
    }
    // mProgressDialogTranslations->setWindowModality(Qt::ApplicationModal);

    mProgressDialogTranslations->show();
    // progressDialog->raise();
    // progressDialog->activateWindow();
    // mProgressDialogTranslations->setWindowModality(Qt::NonModal);

    mTranslationUpdater->checkForUpdates();
}

void DkNoMacs::restartWithTranslationUpdate()
{
    if (!mTranslationUpdater) {
        mTranslationUpdater = new DkTranslationUpdater(false, this);
        connect(mTranslationUpdater, &DkTranslationUpdater::showUpdaterMessage, this, &DkNoMacs::showUpdaterMessage);
    }

    mTranslationUpdater->silent = true;

    connect(mTranslationUpdater, &DkTranslationUpdater::downloadFinished, getTabWidget(), &DkCentralWidget::restart);
    updateTranslations();
}

void DkNoMacs::openPluginManager()
{
#ifdef WITH_PLUGINS

    if (getTabWidget()->getViewPort())
        getTabWidget()->getViewPort()->getController()->closePlugin(true);

    if (DkPluginManager::instance().getRunningPlugin()) {
        QMessageBox infoDialog(this);
        infoDialog.setWindowTitle("Close plugin");
        infoDialog.setIcon(QMessageBox::Information);
        infoDialog.setText("Please close the currently opened plugin first.");
        infoDialog.show();

        infoDialog.exec();
        return;
    }

    DkPluginManagerDialog *pluginDialog = new DkPluginManagerDialog(this);
    pluginDialog->exec();
    pluginDialog->deleteLater();

    DkPluginActionManager *am = DkActionManager::instance().pluginActionManager();
    am->updateMenu();

#endif // WITH_PLUGINS
}

// DkNoMacsSync --------------------------------------------------------------------
DkNoMacsSync::DkNoMacsSync(QWidget *parent, Qt::WindowFlags flags)
    : DkNoMacs(parent, flags)
{
}

DkNoMacsSync::~DkNoMacsSync()
{
}

void DkNoMacsSync::createActions()
{
    DkNoMacs::createActions();

    DkActionManager &am = DkActionManager::instance();

    // sync menu
    connect(am.action(DkActionManager::menu_sync_pos), &QAction::triggered, this, &DkNoMacs::tcpSendWindowRect);
    connect(am.action(DkActionManager::menu_sync_arrange), &QAction::triggered, this, &DkNoMacs::tcpSendArrange);

    auto cm = DkSyncManager::inst().client();
    assert(cm);

    // just for local client
    const auto localCM = dynamic_cast<DkLocalClientManager *>(cm);
    if (localCM != nullptr) {
        connect(this, &DkNoMacsSync::sendArrangeSignal, localCM, &DkLocalClientManager::sendArrangeInstances);
        connect(this, &DkNoMacsSync::sendQuitLocalClientsSignal, localCM, &DkLocalClientManager::sendQuitMessageToPeers);
    }

    connect(cm, &DkClientManager::clientConnectedSignal, this, &DkNoMacsSync::newClientConnected);
    connect(cm, &DkClientManager::receivedPosition, this, &DkNoMacsSync::tcpSetWindowRect);
}

// mouse events
void DkNoMacsSync::mouseMoveEvent(QMouseEvent *event)
{
    int dist = QPoint(event->pos() - mMousePos).manhattanLength();

    // create drag sync action
    if (event->buttons() == Qt::LeftButton && dist > QApplication::startDragDistance() && event->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {
        qDebug() << "generating a drag event...";

        auto cm = dynamic_cast<DkLocalClientManager *>(DkSyncManager::inst().client());
        assert(cm);
        auto md = cm->mimeData();

        QDrag *drag = new QDrag(this);
        drag->setMimeData(md);
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    } else
        DkNoMacs::mouseMoveEvent(event);
}

void DkNoMacsSync::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("network/sync-dir")) {
        event->accept();
    }

    QMainWindow::dragEnterEvent(event);
}

void DkNoMacsSync::dropEvent(QDropEvent *event)
{
    if (event->source() == this) {
        event->accept();
        return;
    }

    if (event->mimeData()->hasFormat("network/sync-dir")) {
        QByteArray connectionData = event->mimeData()->data("network/sync-dir");
        QDataStream dataStream(&connectionData, QIODevice::ReadOnly);
        quint16 peerId;
        dataStream >> peerId;

        auto cm = DkSyncManager::inst().client();
        cm->synchronizeWithServerPort(peerId);
    } else
        QMainWindow::dropEvent(event);
}

DkNoMacsIpl::DkNoMacsIpl(QWidget *parent, Qt::WindowFlags flags)
    : DkNoMacsSync(parent, flags)
{
    DkSettingsManager::param().app().appMode = DkSettings::mode_default;

    // init members
    DkCentralWidget *cw = new DkCentralWidget(this);
    setCentralWidget(cw);

    init();
    setAcceptDrops(true);
    setMouseTracking(true); // receive mouse event everytime
}

void DkNoMacsIpl::paintEvent(QPaintEvent *event)
{
    // prevent visual glitch where initial fullscreen window is filled
    // with normal window bg color. Once the viewport comes up it takes over
    // drawing the background and dock widgets get the default bg color
    bool hasViewPort = getTabWidget() && getTabWidget()->hasViewPort();
    if (isFullScreen() && !hasViewPort) {
        QColor bgColor = nmc::DkSettingsManager::param().slideShow().backgroundColor;
        QPainter painter(this);
        painter.fillRect(rect(), bgColor);
    } else
        QMainWindow::paintEvent(event);
}

// FramelessNoMacs --------------------------------------------------------------------
DkNoMacsFrameless::DkNoMacsFrameless(QWidget *parent, Qt::WindowFlags flags)
    : DkNoMacs(parent, flags)
{
    setObjectName("DkNoMacsFrameless");
    DkSettingsManager::param().app().appMode = DkSettings::mode_frameless;

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    // init members
    DkCentralWidget *cw = new DkCentralWidget(this);
    setCentralWidget(cw);

    init();

    setAcceptDrops(true);
    setMouseTracking(true); // receive mouse event everytime

    // in frameless, you cannot control if menu is visible...
    DkActionManager &am = DkActionManager::instance();
    am.action(DkActionManager::menu_panel_menu)->setEnabled(false);
    am.action(DkActionManager::menu_panel_statusbar)->setEnabled(false);
    am.action(DkActionManager::menu_panel_statusbar)->setChecked(false);
    am.action(DkActionManager::menu_panel_toolbar)->setChecked(false);

    mMenu->setTimeToShow(5000);
    mMenu->hide();

    am.action(DkActionManager::menu_view_frameless)->blockSignals(true);
    am.action(DkActionManager::menu_view_frameless)->setChecked(true);
    am.action(DkActionManager::menu_view_frameless)->blockSignals(false);

    chooseMonitor(false);
    connect(am.action(DkActionManager::menu_view_monitors), &QAction::triggered, this, [this] {
        chooseMonitor(true);
    });

    DkStatusBarManager::instance().show(false, false);

    // actions that should always be disabled
    DkActionManager::instance().action(DkActionManager::menu_view_fit_frame)->setEnabled(false);
}

DkNoMacsFrameless::~DkNoMacsFrameless()
{
}

void DkNoMacsFrameless::createContextMenu()
{
    DkNoMacs::createContextMenu();

    DkActionManager &am = DkActionManager::instance();
    am.contextMenu()->addSeparator();
    am.contextMenu()->addAction(am.action(DkActionManager::menu_view_monitors));
    am.contextMenu()->addAction(am.action(DkActionManager::menu_file_exit));
}

void DkNoMacsFrameless::paintEvent(QPaintEvent *event)
{
    // paint everything except the central/transparent area
    const QRegion mask = QRegion(event->rect()).subtracted(centralWidget()->geometry());

    if (!mask.isEmpty()) {
        QPainter painter(this);
        const QColor &bgColor = DkSettingsManager::param().display().bgColor;
        for (auto it = mask.begin(); it != mask.end(); ++it)
            painter.fillRect(*it, bgColor);
    }

    DkNoMacs::paintEvent(event);
}

void DkNoMacsFrameless::chooseMonitor(bool force)
{
    QScreen *oldScreen = qApp->screenAt(geometry().topLeft());
    if (!oldScreen) {
        qWarning() << "[chooseMonitor] invalid geometry";
        return;
    }

    disconnect(oldScreen, nullptr, this, nullptr);
    QRect screenRect = oldScreen->availableGeometry();

    if (qApp->screens().count() > 1) {
        DkChooseMonitorDialog *cmd = new DkChooseMonitorDialog(this);
        cmd->setWindowTitle(tr("Choose a Monitor"));

        if (force || cmd->showDialog()) {
            int answer = cmd->exec();
            if (answer == QDialog::Accepted) {
                screenRect = cmd->screenRect();
            }
        } else {
            screenRect = cmd->screenRect();
        }
    }

    setGeometry(screenRect);

    QScreen *newScreen = qApp->screenAt(geometry().topLeft());
    if (!newScreen) {
        qWarning() << "[chooseMonitor] invalid screenRect returned";
        return;
    }

    connect(newScreen, &QScreen::availableGeometryChanged, this, [this](const QRect &rect) {
        qInfo() << "[chooseMonitor] screen geometry changed" << rect;
        chooseMonitor(true);
    });

    qInfo() << "[chooseMonitor] force:" << force << "set geometry:" << geometry() << "windowState:" << windowState();
}

// >DIR diem: eating shortcut overrides
bool DkNoMacsFrameless::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // consume esc key if fullscreen is on
        if (keyEvent->key() == Qt::Key_Escape && isFullScreen()) {
            exitFullScreen();
            return true;
        } else if (keyEvent->key() == Qt::Key_Escape) {
            close();
            return true;
        }
    }
    // if (event->type() == QEvent::Gesture) {
    //	return gestureEvent(static_cast<QGestureEvent*>(event));
    // }

    return false;
}

// Transfer function:

DkNoMacsContrast::DkNoMacsContrast(QWidget *parent, Qt::WindowFlags flags)
    : DkNoMacsSync(parent, flags)
{
    setObjectName("DkNoMacsContrast");
    DkSettingsManager::param().app().appMode = DkSettings::mode_contrast;

    // init members
    DkCentralWidget *cw = new DkCentralWidget(this);
    setCentralWidget(cw);

    init();

    DkToolBarManager::inst().createTransferToolBar();

    setAcceptDrops(true);
    setMouseTracking(true); // receive mouse event everytime

    // TODO: this should be checked but no event should be called
    DkActionManager &am = DkActionManager::instance();
    am.action(DkActionManager::menu_panel_transfertoolbar)->blockSignals(true);
    am.action(DkActionManager::menu_panel_transfertoolbar)->setChecked(true);
    am.action(DkActionManager::menu_panel_transfertoolbar)->blockSignals(false);
}

DkNoMacsContrast::~DkNoMacsContrast()
{
}

}
