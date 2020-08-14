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
#include "DkNetwork.h"
#include "DkViewPort.h"
#include "DkWidgets.h"
#include "DkDialog.h"
#include "DkSettings.h"
#include "DkMenu.h"
#include "DkMessageBox.h"
#include "DkMetaDataWidgets.h"
#include "DkManipulatorWidgets.h"
#include "DkThumbsWidgets.h"
#include "DkBatch.h"
#include "DkCentralWidget.h"
#include "DkMetaData.h"
#include "DkImageContainer.h"
#include "DkQuickAccess.h"
#include "DkUtils.h"
#include "DkControlWidget.h"
#include "DkImageLoader.h"
#include "DkTimer.h"
#include "DkActionManager.h"
#include "DkStatusBar.h"
#include "DkDockWidgets.h"
#include "DkLogWidget.h"
#include "DkUpdater.h"
#include "DkToolbars.h"

#ifdef  WITH_PLUGINS
#include "DkPluginInterface.h"
#include "DkPluginManager.h"
#endif //  WITH_PLUGINS

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QBoxLayout>
#include <QResizeEvent>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
//#include <QPanGesture>
#include <QSplashScreen>
#include <QErrorMessage>
#include <QDesktopServices>
#include <QClipboard>
#include <QEvent>
#include <QSettings>
#include <QFileInfo>
#include <QTimer>
#include <QProcess>
#include <QStringBuilder>
#include <QDesktopWidget>
#include <QProgressDialog>
#include <QDrag>
#include <QVector2D>
#include <qmath.h>
#include <QMimeData>
#include <QNetworkProxyFactory>
#include <QInputDialog>
#include <QApplication>
#include <QShortcut>
#pragma warning(pop)		// no warnings from includes - end

#if defined(Q_OS_WIN) && !defined(SOCK_STREAM)
#include <winsock2.h>	// needed since libraw 0.16
#endif

#ifdef Q_OS_WIN
#include <QWinTaskbarButton>
#endif

#include <assert.h>

#include <iostream>

namespace nmc {

DkNomacsOSXEventFilter::DkNomacsOSXEventFilter(QObject *parent) : QObject(parent) {
}

/*! Handle QFileOpenEvent for mac here */
bool DkNomacsOSXEventFilter::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::FileOpen) {
		emit loadFile(static_cast<QFileOpenEvent*>(event)->file());
		return true;
	}
	return QObject::eventFilter(obj, event);
}

DkNoMacs::DkNoMacs(QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags) {

	QMainWindow::setWindowTitle("nomacs | Image Lounge");
	setObjectName("DkNoMacs");

	mMenu = new DkMenuBar(this, -1);

	DkActionManager& am = DkActionManager::instance();
	am.createActions(this);
	am.createMenus(mMenu);
	am.enableImageActions(false);

	mSaveSettings = true;

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
	
	resize(850, 504);
	setMinimumSize(20, 20);

	//// fun fact
	//double an = pow(3987, 12);
	//double bn = pow(4365, 12);

	//qDebug() << "3987 ^ 12 + 4365 ^ 12 = " << pow(an + bn, 1/12.0) << "^ 12";
	//qDebug() << "Sorry Fermat, but the Simpsons are right.";
}

DkNoMacs::~DkNoMacs() {
}

void DkNoMacs::init() {

// assign icon -> in windows the 32px version
	QString iconPath = ":/nomacs/img/nomacs.svg";
	loadStyleSheet();

	QIcon nmcIcon = QIcon(iconPath);
	setObjectName("DkNoMacs");
	
	if (!nmcIcon.isNull())
		setWindowIcon(nmcIcon);

	// shortcuts and actions
	createActions();
	createMenu();
	createContextMenu();
	createStatusBar();

	//// TODO - just for android register me as a gesture recognizer
	//grabGesture(Qt::PanGesture);
	//grabGesture(Qt::PinchGesture);
	//grabGesture(Qt::SwipeGesture);

	// load the window at the same position as last time
	readSettings();
	installEventFilter(this);

	if (DkSettingsManager::param().app().appMode != DkSettings::mode_frameless) {
		DkToolBarManager::inst().showDefaultToolBar(DkSettingsManager::param().app().showToolBar);
		showMenuBar(DkSettingsManager::param().app().showMenuBar);
		DkStatusBarManager::instance().show(DkSettingsManager::param().app().showStatusBar);
	}

	// connections to the image loader
	connect(getTabWidget(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), this, SLOT(setWindowTitle(QSharedPointer<DkImageContainerT>)));

	DkActionManager::instance().enableMovieActions(false);

// clean up nomacs
#ifdef Q_OS_WIN
	if (!nmc::DkSettingsManager::param().global().setupPath.isEmpty() && QApplication::applicationVersion() == nmc::DkSettingsManager::param().global().setupVersion) {

		// ask for exists - otherwise we always try to delete it if the user deleted it
		if (!QFileInfo(nmc::DkSettingsManager::param().global().setupPath).exists() || QFile::remove(nmc::DkSettingsManager::param().global().setupPath)) {
			nmc::DkSettingsManager::param().global().setupPath = "";
			nmc::DkSettingsManager::param().global().setupVersion = "";
			nmc::DkSettingsManager::param().save();
		}
	}
#endif // Q_WS_WIN
}

void DkNoMacs::createStatusBar() {

	setStatusBar(DkStatusBarManager::instance().statusbar());
}

void DkNoMacs::loadStyleSheet() {

	DkThemeManager tm;
	tm.applyTheme();
}

void DkNoMacs::createMenu() {

	setMenuBar(mMenu);
	DkActionManager& am = DkActionManager::instance();
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
	// plugin menu
	mPluginsMenu = mMenu->addMenu(tr("Pl&ugins"));
	am.pluginActionManager()->setMenu(mPluginsMenu);
#endif // WITH_PLUGINS

	mMenu->addMenu(am.helpMenu());
}

void DkNoMacs::createContextMenu() {
}

void DkNoMacs::createActions() {
	
	DkActionManager& am = DkActionManager::instance();

	connect(am.action(DkActionManager::menu_file_open), SIGNAL(triggered()), this, SLOT(openFile()));
	connect(am.action(DkActionManager::menu_file_open_dir), SIGNAL(triggered()), this, SLOT(openDir()));
	connect(am.action(DkActionManager::menu_file_quick_launch), SIGNAL(triggered()), this, SLOT(openQuickLaunch()));
	connect(am.action(DkActionManager::menu_file_open_list), SIGNAL(triggered()), this, SLOT(openFileList()));
	connect(am.action(DkActionManager::menu_file_save_list), SIGNAL(triggered()), this, SLOT(saveFileList()));
	connect(am.action(DkActionManager::menu_file_rename), SIGNAL(triggered()), this, SLOT(renameFile()));
	connect(am.action(DkActionManager::menu_file_goto), SIGNAL(triggered()), this, SLOT(goTo()));
	connect(am.action(DkActionManager::menu_file_show_recent), SIGNAL(triggered(bool)), centralWidget(), SLOT(showRecentFiles(bool)));	
	connect(am.action(DkActionManager::menu_file_new_instance), SIGNAL(triggered()), this, SLOT(newInstance()));
	connect(am.action(DkActionManager::menu_file_private_instance), SIGNAL(triggered()), this, SLOT(newInstance()));
	connect(am.action(DkActionManager::menu_file_find), SIGNAL(triggered()), this, SLOT(find()));
	connect(am.action(DkActionManager::menu_file_recursive), SIGNAL(triggered(bool)), this, SLOT(setRecursiveScan(bool)));
	connect(am.action(DkActionManager::menu_file_exit), SIGNAL(triggered()), this, SLOT(close()));
	
	connect(am.action(DkActionManager::menu_sort_filename), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));
	connect(am.action(DkActionManager::menu_sort_date_created), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));
	connect(am.action(DkActionManager::menu_sort_date_modified), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));
	connect(am.action(DkActionManager::menu_sort_random), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));
	connect(am.action(DkActionManager::menu_sort_ascending), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));
	connect(am.action(DkActionManager::menu_sort_descending), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));

	connect(am.action(DkActionManager::menu_panel_menu), SIGNAL(toggled(bool)), this, SLOT(showMenuBar(bool)));
	connect(am.action(DkActionManager::menu_panel_explorer), SIGNAL(toggled(bool)), this, SLOT(showExplorer(bool)));
	connect(am.action(DkActionManager::menu_panel_metadata_dock), SIGNAL(toggled(bool)), this, SLOT(showMetaDataDock(bool)));
	connect(am.action(DkActionManager::menu_edit_image), SIGNAL(toggled(bool)), this, SLOT(showEditDock(bool)));
	connect(am.action(DkActionManager::menu_panel_history), SIGNAL(toggled(bool)), this, SLOT(showHistoryDock(bool)));
	connect(am.action(DkActionManager::menu_panel_log), SIGNAL(toggled(bool)), this, SLOT(showLogDock(bool)));
	connect(am.action(DkActionManager::menu_panel_preview), SIGNAL(toggled(bool)), this, SLOT(showThumbsDock(bool)));
	connect(am.action(DkActionManager::menu_panel_toggle), SIGNAL(toggled(bool)), this, SLOT(toggleDocks(bool)));

	connect(am.action(DkActionManager::menu_view_fit_frame), SIGNAL(triggered()), this, SLOT(fitFrame()));
	connect(am.action(DkActionManager::menu_view_fullscreen), SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
	connect(am.action(DkActionManager::menu_view_frameless), SIGNAL(toggled(bool)), this, SLOT(restartFrameless(bool)));
	connect(am.action(DkActionManager::menu_panel_transfertoolbar), SIGNAL(toggled(bool)), this, SLOT(restartWithPseudoColor(bool)));
	connect(am.action(DkActionManager::menu_view_opacity_change), SIGNAL(triggered()), this, SLOT(showOpacityDialog()));
	connect(am.action(DkActionManager::menu_view_opacity_up), SIGNAL(triggered()), this, SLOT(opacityUp()));
	connect(am.action(DkActionManager::menu_view_opacity_down), SIGNAL(triggered()), this, SLOT(opacityDown()));
	connect(am.action(DkActionManager::menu_view_opacity_an), SIGNAL(triggered()), this, SLOT(animateChangeOpacity()));
	connect(am.action(DkActionManager::menu_view_lock_window), SIGNAL(triggered(bool)), this, SLOT(lockWindow(bool)));
	
	connect(am.action(DkActionManager::menu_tools_thumbs), SIGNAL(triggered()), this, SLOT(computeThumbsBatch()));
	connect(am.action(DkActionManager::menu_tools_filter), SIGNAL(triggered(bool)), this, SLOT(find(bool)));
	connect(am.action(DkActionManager::menu_tools_export_tiff), SIGNAL(triggered()), this, SLOT(exportTiff()));
	connect(am.action(DkActionManager::menu_tools_extract_archive), SIGNAL(triggered()), this, SLOT(extractImagesFromArchive()));
	connect(am.action(DkActionManager::menu_tools_train_format), SIGNAL(triggered()), this, SLOT(trainFormat()));

	connect(am.action(DkActionManager::sc_test_rec), SIGNAL(triggered()), this, SLOT(loadRecursion()));
	connect(am.action(DkActionManager::sc_test_pong), SIGNAL(triggered()), this, SLOT(startPong()));
	
	connect(am.action(DkActionManager::menu_plugin_manager), SIGNAL(triggered()), this, SLOT(openPluginManager()));

	// help menu
	connect(am.action(DkActionManager::menu_help_about), SIGNAL(triggered()), this, SLOT(aboutDialog()));
	connect(am.action(DkActionManager::menu_help_documentation), SIGNAL(triggered()), this, SLOT(openDocumentation()));
	connect(am.action(DkActionManager::menu_help_bug), SIGNAL(triggered()), this, SLOT(bugReport()));
	connect(am.action(DkActionManager::menu_help_update), SIGNAL(triggered()), this, SLOT(checkForUpdate()));
	connect(am.action(DkActionManager::menu_help_update_translation), SIGNAL(triggered()), this, SLOT(updateTranslations()));

	connect(am.appManager(), SIGNAL(openFileSignal(QAction*)), this, SLOT(openFileWith(QAction*)));
}

void DkNoMacs::clearFileHistory() {
	DkSettingsManager::param().global().recentFiles.clear();
}

void DkNoMacs::clearFolderHistory() {
	DkSettingsManager::param().global().recentFolders.clear();
}

DkCentralWidget* DkNoMacs::getTabWidget() const {

	DkCentralWidget* cw = dynamic_cast<DkCentralWidget*>(centralWidget());
	return cw;
}

// Qt how-to
void DkNoMacs::closeEvent(QCloseEvent *event) {

	DkCentralWidget* cw = static_cast<DkCentralWidget*>(centralWidget());

	if (cw && cw->getTabs().size() > 1) {
		
		DkMessageBox* msg = new DkMessageBox(QMessageBox::Question, tr("Quit nomacs"), 
			tr("Do you want nomacs to save your tabs?"), 
			(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), this);
		msg->setButtonText(QMessageBox::Yes, tr("&Save and Quit"));
		msg->setButtonText(QMessageBox::No, tr("&Quit"));
		msg->setObjectName("saveTabsDialog");

		int answer = msg->exec();
	
		if (answer == QMessageBox::Cancel || answer == QMessageBox::NoButton) {	// User canceled - do not close
			event->ignore();
			return;
		}

		cw->saveSettings(answer == QMessageBox::Yes);
	}
	else
		cw->saveSettings(false);

	if (!getTabWidget()->requestClose()) {
		// do not close if the user hit cancel in the save changes dialog
		event->ignore();
		return;
	}

	emit closeSignal();
	qDebug() << "saving window settings...";
	setVisible(false);
	//showNormal();

	if (mSaveSettings) {
		DefaultSettings settings;
		settings.setValue("geometryNomacs", geometry());
		settings.setValue("geometry", saveGeometry());
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
	}

	QMainWindow::closeEvent(event);
}

void DkNoMacs::resizeEvent(QResizeEvent *event) {

	QMainWindow::resizeEvent(event);
	
	if (!mOverlaid)
		mOldGeometry = geometry();
	else if (windowOpacity() < 1.0f) {
		animateChangeOpacity();
		mOverlaid = false;
	}

}

void DkNoMacs::moveEvent(QMoveEvent *event) {

	QMainWindow::moveEvent(event);

	if (!mOverlaid)
		mOldGeometry = geometry();
	else if (windowOpacity() < 1.0f) {
		animateChangeOpacity();
		mOverlaid = false;
	}
}

void DkNoMacs::mouseDoubleClickEvent(QMouseEvent* event) {

	if (event->button() != Qt::LeftButton || (getTabWidget() && !getTabWidget()->getCurrentImage()))
		return;

	if (isFullScreen())
		exitFullScreen();
	else if (DkSettingsManager::instance().param().global().doubleClickForFullscreen)
		enterFullScreen();

	//QMainWindow::mouseDoubleClickEvent(event);
}


void DkNoMacs::mousePressEvent(QMouseEvent* event) {

	mMousePos = event->pos();

	QMainWindow::mousePressEvent(event);
}

void DkNoMacs::mouseReleaseEvent(QMouseEvent *event) {

	QMainWindow::mouseReleaseEvent(event);
}

void DkNoMacs::contextMenuEvent(QContextMenuEvent *event) {

	QMainWindow::contextMenuEvent(event);

	if (!event->isAccepted())
		DkActionManager::instance().contextMenu()->exec(event->globalPos());
}

void DkNoMacs::mouseMoveEvent(QMouseEvent *event) {

	QMainWindow::mouseMoveEvent(event);
}

//bool DkNoMacs::gestureEvent(QGestureEvent *event) {
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
//}

void DkNoMacs::readSettings() {
	
	DefaultSettings settings;

#ifdef Q_WS_WIN
	// fixes #392 - starting maximized on 2nd screen - tested on win8 only
	QRect r = settings.value("geometryNomacs", QRect()).toRect();

	if (r.width() && r.height())	// do not set the geometry if nomacs is loaded the first time
		setGeometry(r);
#endif

	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());

	// restore state makes the toolbar visible - so hide it again...
	if (DkSettingsManager::param().app().appMode == DkSettings::mode_frameless) {
		DkToolBarManager::inst().showDefaultToolBar(false);
		DkStatusBarManager::instance().show(false);
	}
}

void DkNoMacs::toggleFullScreen() {

	if (isFullScreen())
		exitFullScreen();
	else
		enterFullScreen();
}

void DkNoMacs::enterFullScreen() {
	
	DkSettingsManager::param().app().currentAppMode += qFloor(DkSettings::mode_end*0.5f);
	if (DkSettingsManager::param().app().currentAppMode < 0) {
		qDebug() << "illegal state: " << DkSettingsManager::param().app().currentAppMode;
		DkSettingsManager::param().app().currentAppMode = DkSettings::mode_default;
	}

	menuBar()->hide();
	DkToolBarManager::inst().show(false);
	DkStatusBarManager::instance().statusbar()->hide();
	getTabWidget()->showTabs(false);

	restoreDocks();

	// here is an issue with windows that I can't quite fix:
	// if we send nomacs to fullscreen from an attached window (i.e. split window)
	setWindowState(windowState() ^ Qt::WindowFullScreen);
	
	if (getTabWidget()->getViewPort())
		getTabWidget()->getViewPort()->setFullScreen(true);

}

void DkNoMacs::exitFullScreen() {

	if (isFullScreen()) {
		DkSettingsManager::param().app().currentAppMode -= qFloor(DkSettings::mode_end*0.5f);
		if (DkSettingsManager::param().app().currentAppMode < 0) {
			qDebug() << "illegal state: " << DkSettingsManager::param().app().currentAppMode;
			DkSettingsManager::param().app().currentAppMode = DkSettings::mode_default;
		}

		if (DkSettingsManager::param().app().showMenuBar) mMenu->show();
		if (DkSettingsManager::param().app().showStatusBar) DkStatusBarManager::instance().statusbar()->show();

		DkToolBarManager::inst().restore();
		restoreDocks();
		setWindowState(windowState() ^ Qt::WindowFullScreen);


		if (getTabWidget())
			getTabWidget()->showTabs(true);

		update();	// if no resize is triggered, the viewport won't change its color
	}

	if (getTabWidget()->getViewPort())
		getTabWidget()->getViewPort()->setFullScreen(false);
}

void DkNoMacs::toggleDocks(bool hide) {

	if (hide) {
		showExplorer(false, false);
		showMetaDataDock(false, false);
		showEditDock(false, false);
		showHistoryDock(false, false);
		showLogDock(false, false);
		DkToolBarManager::inst().show(false, false);
		DkStatusBarManager::instance().show(false, false);
	}
	else {
		restoreDocks();
		DkToolBarManager::inst().restore();
		DkStatusBarManager::instance().show(DkSettingsManager::param().app().showStatusBar, false);
	}
}

void DkNoMacs::restoreDocks() {

	showExplorer(DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showExplorer), false);
	showMetaDataDock(DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showMetaDataDock), false);
	showEditDock(DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showEditDock), false);
	showHistoryDock(DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showHistoryDock), false);
	showLogDock(DkDockWidget::testDisplaySettings(DkSettingsManager::param().app().showLogDock), false);
}

void DkNoMacs::restartFrameless(bool) {

	QString exe = QApplication::applicationFilePath();
	QStringList args;

	if (objectName() != "DkNoMacsFrameless")
		args << "-m" << "frameless";
	else
		args << "-m" << "default";

	if (getTabWidget()->getCurrentImage())
		args.append(getTabWidget()->getCurrentImage()->filePath());
	
	nmc::DkSettingsManager::param().save();
	
	bool started = mProcess.startDetached(exe, args);

	// close me if the new instance started
	if (started)
		close();

	qDebug() << "frameless arguments: " << args;
}

void DkNoMacs::showRecentFilesOnStartUp() {

	QTimer::singleShot(100, getTabWidget(), SLOT(showRecentFiles()));
}

void DkNoMacs::startPong() const {

	QString exe = QApplication::applicationFilePath();
	QStringList args;

	args.append("--pong");

	bool started = mProcess.startDetached(exe, args);
	qDebug() << "pong started: " << started;
}

void DkNoMacs::fitFrame() {

	if (!getTabWidget()->getViewPort())
		return;

	auto vp = getTabWidget()->getViewPort();

	QRectF viewRect = vp->getImageViewRect();
	QRectF vpRect = vp->geometry();
	QRectF nmRect = frameGeometry();
	QSize frDiff = frameGeometry().size()-geometry().size();

	// compute new size
	QPointF c = nmRect.center();
	nmRect.setSize(nmRect.size() + viewRect.size() - vpRect.size());
	nmRect.moveCenter(c);
	
	// still fits on screen?
	QDesktopWidget* dw = QApplication::desktop();
	QRect screenRect = dw->availableGeometry(this);
	QRect newGeometry = screenRect.intersected(nmRect.toRect());
	
	// correct frame
	newGeometry.setSize(newGeometry.size()-frDiff);
	newGeometry.moveTopLeft(newGeometry.topLeft() - frameGeometry().topLeft()+geometry().topLeft());

	setGeometry(newGeometry);

	// reset viewport if we did not clip -> compensates round-off errors
	if (screenRect.contains(nmRect.toRect()))
		vp->resetView();

}

void DkNoMacs::setRecursiveScan(bool recursive) {

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

void DkNoMacs::showOpacityDialog() {

	if (!mOpacityDialog) {
		mOpacityDialog = new DkOpacityDialog(this);
		mOpacityDialog->setWindowTitle(tr("Change Opacity"));
	}
	
	if (mOpacityDialog->exec())
		setWindowOpacity(mOpacityDialog->value()/100.0f);
}

void DkNoMacs::opacityDown() {

	changeOpacity(-0.3f);
}

void DkNoMacs::opacityUp() {
	
	changeOpacity(0.3f);
}

void DkNoMacs::changeOpacity(float change) {

	float newO = (float)windowOpacity() + change;
	if (newO > 1) newO = 1.0f;
	if (newO < 0.1) newO = 0.1f;
	setWindowOpacity(newO);
}

void DkNoMacs::animateOpacityDown() {

	float newO = (float)windowOpacity() - 0.03f;

	if (newO < 0.3f) {
		setWindowOpacity(0.3f);
		return;
	}

	setWindowOpacity(newO);
	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
}

void DkNoMacs::animateOpacityUp() {

	float newO = (float)windowOpacity() + 0.03f;

	if (newO > 1.0f) {
		setWindowOpacity(1.0f);
		return;
	}

	setWindowOpacity(newO);
	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
}

// >DIR: diem - why can't we put it in mViewport?
void DkNoMacs::animateChangeOpacity() {

	float newO = (float)windowOpacity();

	if (newO >= 1.0f)
		animateOpacityDown();
	else
		animateOpacityUp();
}

void DkNoMacs::lockWindow(bool lock) {

	
#ifdef Q_OS_WIN
	
	qDebug() << "locking: " << lock;

	if (lock) {
		//setAttribute(Qt::WA_TransparentForMouseEvents);
		HWND hwnd = (HWND) winId(); // get handle of the widget
		LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
		SetWindowLong(hwnd, GWL_EXSTYLE, styles | WS_EX_TRANSPARENT); 
		SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		
		auto keyStr = DkActionManager::instance().action(DkActionManager::menu_view_lock_window)->shortcut().toString();
		getTabWidget()->setInfo(tr("Window Locked\nTo unlock: gain focus (ALT+Tab),\nthen press %1")
			.arg(keyStr));
	}
	else if (lock && windowOpacity() == 1.0f) {
		getTabWidget()->setInfo(tr("You should first reduce opacity\n before working through the window."));
		DkActionManager::instance().action(DkActionManager::menu_view_lock_window)->setChecked(false);
	}
	else {
		qDebug() << "deactivating...";
		HWND hwnd = (HWND) winId(); // get handle of the widget
		LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
		SetWindowLong(hwnd, GWL_EXSTYLE, styles & ~WS_EX_TRANSPARENT); 

		SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
#else
	// TODO: find corresponding command for linux etc

	//setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	//show();
#endif
}

void DkNoMacs::newClientConnected(bool connected) {
	
	mOverlaid = false;
	// add methods if clients are connected

	DkActionManager& am = DkActionManager::instance();
	am.action(DkActionManager::menu_sync_view)->setEnabled(connected);
	am.action(DkActionManager::menu_sync_pos)->setEnabled(connected);
	am.action(DkActionManager::menu_sync_arrange)->setEnabled(connected);

}

void DkNoMacs::tcpSetWindowRect(QRect newRect, bool opacity, bool overlaid) {

	this->mOverlaid = overlaid;

	// we are currently overlaid...
	if (!overlaid) {

		setGeometry(mOldGeometry);
		if (opacity)
			animateOpacityUp();
		mOldGeometry = geometry();
	}
	else {

#ifdef Q_OS_WIN
		showMinimized();
		setWindowState(Qt::WindowActive);
#else
		Qt::WindowFlags flags = windowFlags();
		setWindowFlags(Qt::WindowStaysOnTopHint);	// we need this to 'generally' (for all OSs) bring the window to front
		setWindowFlags(flags);	// reset flags
		showNormal();
#endif

		mOldGeometry = geometry();
		
		this->move(newRect.topLeft());
		this->resize(newRect.size() - (frameGeometry().size() - geometry().size()));

		//setGeometry(newRect);
		if (opacity)
			animateOpacityDown();
		
		//this->setActiveWindow();
	}
}

void DkNoMacs::tcpSendWindowRect() {

	mOverlaid = !mOverlaid;

	qDebug() << "overlaying";
	// change my geometry
	tcpSetWindowRect(this->frameGeometry(), !mOverlaid, mOverlaid);


	auto cm = DkSyncManager::inst().client();
	if (cm)
		cm->sendPosition(frameGeometry(), mOverlaid);
}

void DkNoMacs::tcpSendArrange() {
	
	mOverlaid = !mOverlaid;
	emit sendArrangeSignal(mOverlaid);
}

void DkNoMacs::showExplorer(bool show, bool saveSettings) {

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

		connect(mExplorer, SIGNAL(openFile(const QString&)), getTabWidget(), SLOT(loadFile(const QString&)));
		connect(mExplorer, SIGNAL(openDir(const QString&)), getTabWidget(), SLOT(loadDirToTab(const QString&)));
		connect(getTabWidget(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mExplorer, SLOT(setCurrentImage(QSharedPointer<DkImageContainerT>)));
	}

	mExplorer->setVisible(show, saveSettings);

	if (getTabWidget()->getCurrentImage() && QFileInfo(getTabWidget()->getCurrentFilePath()).exists()) {
		mExplorer->setCurrentPath(getTabWidget()->getCurrentFilePath());
	}
	else {
		QStringList folders = DkSettingsManager::param().global().recentFiles;

		if (folders.size() > 0)
			mExplorer->setCurrentPath(folders[0]);
	}
}

void DkNoMacs::showMetaDataDock(bool show, bool saveSettings) {

	if (!show && !mMetaDataDock)
		return;

	if (!mMetaDataDock) {

		mMetaDataDock = new DkMetaDataDock(tr("Meta Data Info"), this);
		mMetaDataDock->registerAction(DkActionManager::instance().action(DkActionManager::menu_panel_metadata_dock));
		mMetaDataDock->setDisplaySettings(&DkSettingsManager::param().app().showMetaDataDock);
		addDockWidget(mMetaDataDock->getDockLocationSettings(Qt::RightDockWidgetArea), mMetaDataDock);

		connect(getTabWidget(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mMetaDataDock, SLOT(setImage(QSharedPointer<DkImageContainerT>)));
	}

	mMetaDataDock->setVisible(show, saveSettings);

	if (getTabWidget()->getCurrentImage())
		mMetaDataDock->setImage(getTabWidget()->getCurrentImage());
}

void DkNoMacs::showEditDock(bool show, bool saveSettings) {

	if (!show && !mEditDock)
		return;

	if (!mEditDock) {

		mEditDock = new DkEditDock(tr("Edit Image"), this);
		mEditDock->registerAction(DkActionManager::instance().action(DkActionManager::menu_edit_image));
		mEditDock->setDisplaySettings(&DkSettingsManager::param().app().showEditDock);
		addDockWidget(mEditDock->getDockLocationSettings(Qt::RightDockWidgetArea), mEditDock);

		connect(getTabWidget(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mEditDock, SLOT(setImage(QSharedPointer<DkImageContainerT>)));
	}

	mEditDock->setVisible(show, saveSettings);

	if (getTabWidget()->getCurrentImage())
		mEditDock->setImage(getTabWidget()->getCurrentImage());
}

void DkNoMacs::showHistoryDock(bool show, bool saveSettings) {

	if (!show && !mHistoryDock)
		return;

	if (!mHistoryDock) {

		mHistoryDock = new DkHistoryDock(tr("History"), this);
		mHistoryDock->registerAction(DkActionManager::instance().action(DkActionManager::menu_panel_history));
		mHistoryDock->setDisplaySettings(&DkSettingsManager::param().app().showHistoryDock);
		addDockWidget(mHistoryDock->getDockLocationSettings(Qt::RightDockWidgetArea), mHistoryDock);

		connect(getTabWidget(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mHistoryDock, SLOT(updateImage(QSharedPointer<DkImageContainerT>)));
	}

	mHistoryDock->setVisible(show, saveSettings);

	if (show && getTabWidget()->getCurrentImage())
		mHistoryDock->updateImage(getTabWidget()->getCurrentImage());
}

void DkNoMacs::showLogDock(bool show, bool saveSettings) {

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

void DkNoMacs::showThumbsDock(bool show) {
	
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

			//DkSettingsManager::param().display().thumbDockSize = qMin(thumbsDock->width(), thumbsDock->height());
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

		QLabel* thumbsTitle = new QLabel(mThumbsDock);
		thumbsTitle->setObjectName("thumbsTitle");
		thumbsTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		thumbsTitle->setPixmap(QPixmap(":/nomacs/img/widget-separator.png").scaled(QSize(16, 4)));
		thumbsTitle->setFixedHeight(16);
		mThumbsDock->setTitleBarWidget(thumbsTitle);

		connect(mThumbsDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(thumbsDockAreaChanged()));
	}

	if (show != mThumbsDock->isVisible())
		mThumbsDock->setVisible(show);
}

void DkNoMacs::thumbsDockAreaChanged() {

	Qt::DockWidgetArea area = dockWidgetArea(mThumbsDock);

	int thumbsOrientation = DkFilePreview::cm_pos_dock_hor;

	if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
		thumbsOrientation = DkFilePreview::cm_pos_dock_ver;

	if (getTabWidget()->getViewPort())
		getTabWidget()->getViewPort()->getController()->getFilePreview()->setWindowPosition(thumbsOrientation);

}

void DkNoMacs::openDir() {

	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(
		this, 
		tr("Open an Image Directory"),
		getTabWidget()->getCurrentDir(), 
		QFileDialog::ShowDirsOnly | DkDialog::fileDialogOptions()
	);

	if (dirName.isEmpty())
		return;

	getTabWidget()->loadDirToTab(dirName);
}

void DkNoMacs::openFile() {

	QStringList openFilters = DkSettingsManager::param().app().openFilters;
	openFilters.pop_front();
	openFilters.prepend(tr("All Files (*.*)"));

	// load system default open dialog
	QStringList filePaths = QFileDialog::getOpenFileNames(
		this,
		tr("Open Image"),
		getTabWidget()->getCurrentDir(),
		openFilters.join(";;"),
		nullptr,
		DkDialog::fileDialogOptions()
	);

	if (filePaths.isEmpty())
		return;

	int count = getTabWidget()->getTabs().count(); // Save current count of tabs for setting tab position later
	if (getTabWidget()->getTabs().at(0)->getMode() == DkTabInfo::tab_empty)
		count = 0; 
		
	QSet<QString> duplicates;
	for (const QString& fp : filePaths) {
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
			bool newTab = filePaths.size() > 1 | getTabWidget()->getTabs().size() > 1;
			getTabWidget()->loadFile(fp, newTab);
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

void DkNoMacs::openFileList() {
	
	QStringList openFilters;
	openFilters.append(tr("Text file (*.txt)"));
	openFilters.append(tr("All files (*.*)"));

	// load system default open dialog
	QString fileName = QFileDialog::getOpenFileName(
		this, 
		tr("Open Image"),
		getTabWidget()->getCurrentDir(),
		openFilters.join(";;"),
		nullptr,
		DkDialog::fileDialogOptions()
	);

	if (fileName.isEmpty())
		return;

	int count = getTabWidget()->getTabs().count();
	if (getTabWidget()->getTabs().at(0)->getMode() == DkTabInfo::tab_empty)
		count = 0;

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	while (!file.atEnd()) {
		QString line = file.readLine().simplified();
		if (QFileInfo::exists(line)) {
			getTabWidget()->loadFile(line, true);
		}
	}

	getTabWidget()->setActiveTab(count);
}

void DkNoMacs::saveFileList() {

	QStringList saveFilters;
	saveFilters.append(tr("Text file (*.txt)"));
	saveFilters.append(tr("All files (*.*)"));

	QString fileName = QFileDialog::getSaveFileName(
		this, 
		tr("Save Tab List"),
		getTabWidget()->getCurrentDir(),
		saveFilters.join(";;"),
		nullptr,
		DkDialog::fileDialogOptions()
	);

	if (fileName.isEmpty())
		return;

	QFile file(fileName);
	if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
		return;

	for (auto tab : getTabWidget()->getTabs()) {
		file.write(tab->getFilePath().toUtf8()+"\n");
	}

	file.close();
}

void DkNoMacs::openQuickLaunch() {

	DkMainToolBar* tb = DkToolBarManager::inst().defaultToolBar();

	// create new model
	if (!mQuickAccess) {
		mQuickAccess = new DkQuickAccess(this);
		
		// add all actions
		mQuickAccess->addActions(DkActionManager::instance().allActions());
		
		connect(mQuickAccess, SIGNAL(loadFileSignal(const QString&)), getTabWidget(), SLOT(loadFile(const QString&)));
	}

	if (tb)
		connect(tb->getQuickAccess(), SIGNAL(executeSignal(const QString&)), mQuickAccess, SLOT(execute(const QString&)), Qt::UniqueConnection);

	mQuickAccess->addDirs(DkSettingsManager::param().global().recentFolders);
	mQuickAccess->addFiles(DkSettingsManager::param().global().recentFiles);

	if (tb && tb->isVisible())
		tb->setQuickAccessModel(mQuickAccess->getModel());
	else {
		
		if (!mQuickAccessEdit) {
			mQuickAccessEdit = new DkQuickAccessEdit(this);
			connect(mQuickAccessEdit, SIGNAL(executeSignal(const QString&)), mQuickAccess, SLOT(execute(const QString&)));
		}

		int right = getTabWidget()->geometry().right();
		mQuickAccessEdit->setFixedWidth(qRound(width()/3.0f));
		mQuickAccessEdit->move(QPoint(right-mQuickAccessEdit->width()-10, qRound(height()*0.25)));
		mQuickAccessEdit->setModel(mQuickAccess->getModel());
		mQuickAccessEdit->show();
	}
}

void DkNoMacs::loadFile(const QString& filePath) {

	if (!getTabWidget())
		return;

	if (QFileInfo(filePath).isDir())
		getTabWidget()->loadDirToTab(filePath);
	else
		getTabWidget()->loadFile(filePath, false);

}

// TODO: move this
void DkNoMacs::renameFile() {

	// TODO:ref move!
	QString filePath = getTabWidget()->getCurrentFilePath();
	QFileInfo file(filePath);

	if (!file.absoluteDir().exists()) {
		getTabWidget()->setInfo(tr("Sorry, the directory: %1 does not exist\n").arg(file.absolutePath()));
		return;
	}
	if (file.exists() && !file.isWritable()) {
		getTabWidget()->setInfo(tr("Sorry, I can't write to the fileInfo: %1").arg(file.fileName()));
		return;
	}

	QString fileName = file.fileName();
	int dotIdx = fileName.lastIndexOf(".");
	QString baseName = dotIdx != -1 ? fileName.left(dotIdx) : fileName;

	bool ok = false;
	QString newFileName = QInputDialog::getText(this, baseName, tr("Rename:"), QLineEdit::Normal, baseName, &ok);

	if (ok && !newFileName.isEmpty() && newFileName != baseName) {
		
		if (!file.suffix().isEmpty())
			newFileName.append("." + file.suffix());
		
		qDebug() << "renaming: " << file.fileName() << " -> " << newFileName;
		QFileInfo renamedFile = QFileInfo(file.absoluteDir(), newFileName);

		// overwrite file?
		// the second comparison is important for windows (case insensitive filenames)
		if (renamedFile.exists() && renamedFile.absoluteFilePath().compare(file.absoluteFilePath(), Qt::CaseInsensitive) != 0) {

			QMessageBox infoDialog(this);
			infoDialog.setWindowTitle(tr("Question"));
			infoDialog.setText(tr("The fileInfo: %1  already exists.\n Do you want to replace it?").arg(newFileName));
			infoDialog.setIcon(QMessageBox::Question);
			infoDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			infoDialog.setDefaultButton(QMessageBox::No);
			infoDialog.show();
			int choice = infoDialog.exec();

			if (choice == QMessageBox::Yes) {

				QFile oldFile(renamedFile.absoluteFilePath());
				bool removed = oldFile.remove();

				// tell user that deleting went wrong, and stop the renaming
				if (!removed) {
					getTabWidget()->setInfo(tr("Sorry, I can't delete: %1").arg(file.fileName()));
					return;
				}
			}
			else
				return;		// cancel renaming
		}

		if (getTabWidget()->getViewPort())
			getTabWidget()->getViewPort()->unloadImage();

		QFile newFile(file.absoluteFilePath());
		bool renamed = newFile.rename(renamedFile.absoluteFilePath());
		
		// tell user that deleting went wrong, and stop the renaming
		if (!renamed)
			getTabWidget()->setInfo(tr("Sorry, I can't rename: %1").arg(file.fileName()));
		else if (DkSettingsManager::param().resources().loadSavedImage == DkSettings::ls_load)
			getTabWidget()->loadFile(renamedFile.absoluteFilePath());
		else if (getTabWidget()->getViewPort())
			getTabWidget()->getViewPort()->loadNextFileFast();

	}
}

void DkNoMacs::find(bool filterAction) {

	if (!getTabWidget()->getCurrentImageLoader())
		return;

	if (filterAction) {

		int db = (QObject::sender() == DkActionManager::instance().action(DkActionManager::menu_tools_filter)) ? DkSearchDialog::filter_button : DkSearchDialog::find_button;
		
		qDebug() << "default button: " << db;
		DkSearchDialog* searchDialog = new DkSearchDialog(this);
		searchDialog->setDefaultButton(db);

		searchDialog->setFiles(getTabWidget()->getCurrentImageLoader()->getFileNames());
		searchDialog->setPath(getTabWidget()->getCurrentImageLoader()->getDirPath());

		connect(searchDialog, SIGNAL(filterSignal(const QString&)), getTabWidget()->getCurrentImageLoader().data(), SLOT(setFolderFilter(const QString&)));
		connect(searchDialog, SIGNAL(loadFileSignal(const QString&)), getTabWidget(), SLOT(loadFile(const QString&)));
		int answer = searchDialog->exec();

		DkActionManager::instance().action(DkActionManager::menu_tools_filter)->setChecked(answer == DkSearchDialog::filter_button);		
	}
	else {
		// remove the filter 
		getTabWidget()->getCurrentImageLoader()->setFolderFilter(QString());
	}

}

void DkNoMacs::changeSorting(bool change) {

	// TODO: move to image loader?!

	if (change) {
	
		QString senderName = QObject::sender()->objectName();

		if (senderName == "menu_sort_filename")
			DkSettingsManager::param().global().sortMode = DkSettings::sort_filename;
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

	QVector<QAction*> sortActions = DkActionManager::instance().sortActions();
	for (int idx = 0; idx < sortActions.size(); idx++) {

		if (idx < DkActionManager::menu_sort_ascending)
			sortActions[idx]->setChecked(idx == DkSettingsManager::param().global().sortMode);
		else if (idx >= DkActionManager::menu_sort_ascending)
			sortActions[idx]->setChecked(idx-DkActionManager::menu_sort_ascending == DkSettingsManager::param().global().sortDir);
	}
}

void DkNoMacs::goTo() {

	if (!getTabWidget()->getCurrentImageLoader())
		return;

	QSharedPointer<DkImageLoader> loader = getTabWidget()->getCurrentImageLoader();
	
	bool ok = false;
	int fileIdx = QInputDialog::getInt(this, tr("Go To Image"), tr("Image Index:"), 1, 1, loader->numFiles(), 1, &ok);

	if (ok)
		loader->loadFileAt(fileIdx-1);

}

void DkNoMacs::trainFormat() {

	if (!mTrainDialog)
		mTrainDialog = new DkTrainDialog(this);

	mTrainDialog->setCurrentFile(getTabWidget()->getCurrentFilePath());
	bool okPressed = mTrainDialog->exec() != 0;

	if (okPressed && getTabWidget()->getCurrentImageLoader()) {
		getTabWidget()->getCurrentImageLoader()->load(mTrainDialog->getAcceptedFile());
		getTabWidget()->restart();	// quick & dirty, but currently he messes up the filteredFileList if the same folder was already loaded
	}

}

void DkNoMacs::extractImagesFromArchive() {
#ifdef WITH_QUAZIP

	if (!mArchiveExtractionDialog)
		mArchiveExtractionDialog = new DkArchiveExtractionDialog(this);

	if (getTabWidget()->getCurrentImage()) {
		if (getTabWidget()->getCurrentImage()->isFromZip())
			mArchiveExtractionDialog->setCurrentFile(getTabWidget()->getCurrentImage()->getZipData()->getZipFilePath(), true);
		else 
			mArchiveExtractionDialog->setCurrentFile(getTabWidget()->getCurrentFilePath(), false);
	}
	else 
		mArchiveExtractionDialog->setCurrentFile(getTabWidget()->getCurrentFilePath(), false);

	mArchiveExtractionDialog->exec();
#endif
}

void DkNoMacs::exportTiff() {

#ifdef WITH_LIBTIFF
	if (!mExportTiffDialog)
		mExportTiffDialog = new DkExportTiffDialog(this);

	mExportTiffDialog->setFile(getTabWidget()->getCurrentFilePath());
	mExportTiffDialog->exec();
#endif
}

void DkNoMacs::computeThumbsBatch() {

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

void DkNoMacs::aboutDialog() {

	DkSplashScreen* spScreen = new DkSplashScreen(this, 0);
	spScreen->exec();
	spScreen->deleteLater();
}

void DkNoMacs::openDocumentation() {

	QString url = QString("https://nomacs.org/documentation/");
	QDesktopServices::openUrl(QUrl(url));
}

void DkNoMacs::bugReport() {

	QString url = "https://github.com/nomacs/nomacs/issues/new";
	QDesktopServices::openUrl(QUrl(url));
}

void DkNoMacs::cleanSettings() {

	DefaultSettings settings;
	settings.clear();

	readSettings();
	resize(400, 225);
	move(100, 100);
}

void DkNoMacs::newInstance(const QString& filePath) {

	QString exe = QApplication::applicationFilePath();
	QStringList args;

	QAction* a = static_cast<QAction*>(sender());

	if (a && a == DkActionManager::instance().action(DkActionManager::menu_file_private_instance))
		args.append("-p");

	if (filePath.isEmpty())
		args.append(getTabWidget()->getCurrentFilePath());
	else
		args.append(filePath);
	
	QProcess::startDetached(exe, args);
}

void tagWall(const std::list<std::string>& code) {
	for (auto line : code)
		std::cout << line << std::endl;
}

void DkNoMacs::loadRecursion() {

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

void DkNoMacs::restartWithPseudoColor(bool contrast) {

	qDebug() << "contrast: " << contrast;

	QString exe = QApplication::applicationFilePath();
	QStringList args;

	if (contrast)
		args << "-m" << "pseudocolor";
	else
		args << "-m" << "default";
	args.append(getTabWidget()->getCurrentFilePath());
	
	bool started = mProcess.startDetached(exe, args);

	// close me if the new instance started
	if (started)
		close();

	qDebug() << "contrast arguments: " << args;
}

void DkNoMacs::onWindowLoaded() {

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
		DkWelcomeDialog* wecomeDialog = new DkWelcomeDialog(this);
		wecomeDialog->exec();

		settings.setValue("AppSettings/firstTime.nomacs.3", false);

		if (wecomeDialog->isLanguageChanged()) {
			restartWithTranslationUpdate();
		}
	}

	checkForUpdate(true);

	// load settings AFTER everything is initialized
	getTabWidget()->loadSettings();

// init global taskbar
#ifdef WIN32
	QWinTaskbarButton *button = new QWinTaskbarButton(this);
	button->setWindow(windowHandle());

	DkGlobalProgress::instance().setProgressBar(button->progress());
#endif

	toggleDocks(DkSettingsManager::param().app().hideAllPanels);
}

void DkNoMacs::keyPressEvent(QKeyEvent *event) {
	
	if (event->key() == Qt::Key_Alt) {
		mPosGrabKey = QCursor::pos();
		mOtherKeyPressed = false;
	}
	else
		mOtherKeyPressed = true;

}

void DkNoMacs::keyReleaseEvent(QKeyEvent* event) {

	if (event->key() == Qt::Key_Alt && !mOtherKeyPressed && (mPosGrabKey - QCursor::pos()).manhattanLength() == 0)
		mMenu->showMenu();
	
}

// >DIR diem: eating shortcut overrides (this allows us to use navigation keys like arrows)
bool DkNoMacs::eventFilter(QObject*, QEvent* event) {

	if (event->type() == QEvent::ShortcutOverride) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

		// consume esc key if fullscreen is on
		if (keyEvent->key() == Qt::Key_Escape && isFullScreen()) {
			exitFullScreen();
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Escape && DkSettingsManager::param().app().closeOnEsc)
			close();
	}
	//if (event->type() == QEvent::Gesture) {
	//	return gestureEvent(static_cast<QGestureEvent*>(event));
	//}

	return false;
}

void DkNoMacs::showMenuBar(bool show) {

	DkSettingsManager::param().app().showMenuBar = show;
	
	QAction* mp = DkActionManager::instance().action(DkActionManager::menu_panel_menu);
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

void DkNoMacs::openFileWith(QAction* action) {

	if (!action)
		return;

	QFileInfo app(action->toolTip());

	if (!app.exists())
		getTabWidget()->setInfo("Sorry, " % app.fileName() % " does not exist");

	QStringList args;
	
	QString filePath = getTabWidget()->getCurrentFilePath();

	if (app.fileName() == "explorer.exe")
		args << "/select," << QDir::toNativeSeparators(filePath);
	else if (app.fileName().toLower() == "outlook.exe") {
		args << "/a" << QDir::toNativeSeparators(filePath);
	}
	else
		args << QDir::toNativeSeparators(filePath);

	//bool started = process.startDetached("psOpenImages.exe", args);	// already deprecated
	bool started = mProcess.startDetached(app.absoluteFilePath(), args);

	if (started)
		qDebug() << "starting: " << app.fileName() << args;
	else 
		getTabWidget()->setInfo("Sorry, I could not start: " % app.absoluteFilePath());
}

void DkNoMacs::setWindowTitle(QSharedPointer<DkImageContainerT> imgC) {

	if (!imgC) {
		setWindowTitle(QString());
		return;
	}

	setWindowTitle(imgC->filePath(), imgC->image().size(), imgC->isEdited(), imgC->getTitleAttribute());
}

void DkNoMacs::setWindowTitle(const QString& filePath, const QSize& size, bool edited, const QString& attr) {

	QString title;

	if (DkSettingsManager::param().global().extendedTabs && (getTabWidget()->getTabs().count() > 1)) {
		title.append(QString::number(getTabWidget()->getActiveTab() + 1) + "/" + QString::number(getTabWidget()->getTabs().count()) + " - ");
	}

	QFileInfo fInfo = filePath;
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
	title.append(attr);	// append some attributes

	QString attributes;
	auto vp = getTabWidget()->getViewPort();

	if (!size.isEmpty())
		attributes.sprintf(" - %i x %i", size.width(), size.height());
	if (size.isEmpty() && vp && !vp->getImageSize().isEmpty())
		attributes.sprintf(" - %i x %i", vp->getImage().width(), vp->getImage().height());
	if (DkSettingsManager::param().app().privateMode) 
		attributes.append(tr(" [Private Mode]"));

	QMainWindow::setWindowTitle(title.append(attributes));
	setWindowFilePath(filePath);
	setWindowModified(edited);

	auto cm = DkSyncManager::inst().client();
	if (cm)
		cm->sendTitle(windowTitle());

	// TODO: move!
	DkStatusBar* bar = DkStatusBarManager::instance().statusbar();

	if (((vp && !vp->getController()->getFileInfoLabel()->isVisible()) || 
		!DkSettingsManager::param().slideShow().display.testBit(DkSettings::display_creation_date)) && getTabWidget()->getCurrentImage()) {
		
		// create statusbar info
		QSharedPointer<DkMetaDataT> metaData = getTabWidget()->getCurrentImage()->getMetaData();
		QString dateString = metaData->getExifValue("DateTimeOriginal");
		dateString = DkUtils::convertDateString(dateString, fInfo);
		bar->setMessage(dateString, DkStatusBar::status_time_info);
	}
	else 
		bar->setMessage("", DkStatusBar::status_time_info);	// hide label

	if (fInfo.exists())
		bar->setMessage(DkUtils::readableByte((float)fInfo.size()), DkStatusBar::status_filesize_info);
	else 
		bar->setMessage("", DkStatusBar::status_filesize_info);

}

void DkNoMacs::settingsChanged() {
	
	if (!isFullScreen()) {
		showMenuBar(DkSettingsManager::param().app().showMenuBar);

		DkToolBarManager::inst().restore();
		DkStatusBarManager::instance().show(DkSettingsManager::param().app().showStatusBar);
	}
}

void DkNoMacs::checkForUpdate(bool silent) {

	// updates are supported on windows only
#ifndef Q_OS_LINUX

	// do we really need to check for update?
	if (!silent || 
		(!DkSettingsManager::param().sync().updateDialogShown && 
		 QDate::currentDate() > DkSettingsManager::param().sync().lastUpdateCheck && 
		 DkSettingsManager::param().sync().checkForUpdates)) {

		DkTimer dt;

		if (!mUpdater) {
			mUpdater = new DkUpdater(this);
			connect(mUpdater, SIGNAL(displayUpdateDialog(QString, QString)), this, SLOT(showUpdateDialog(QString, QString)));
			connect(mUpdater, SIGNAL(showUpdaterMessage(QString, QString)), this, SLOT(showUpdaterMessage(QString, QString)));
		}
		mUpdater->silent = silent;
		mUpdater->checkForUpdates();
		qDebug() << "checking for updates takes: " << dt;
	}
#endif // !#ifndef Q_OS_LINUX
}

void DkNoMacs::showUpdaterMessage(QString msg, QString title) {
	
	QMessageBox infoDialog(this);
	infoDialog.setWindowTitle(title);
	infoDialog.setIcon(QMessageBox::Information);
	infoDialog.setText(msg);
	infoDialog.show();

	infoDialog.exec();
}

void DkNoMacs::showUpdateDialog(QString msg, QString title) {
	
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
		connect(mUpdateDialog, SIGNAL(startUpdate()), this, SLOT(performUpdate()));
	}

	mUpdateDialog->exec();
}

void DkNoMacs::performUpdate() {
	
	if (!mUpdater) {
		qDebug() << "WARNING updater is NULL where it should not be.";
		return;
	}

	mUpdater->performUpdate();

	if (!mProgressDialog) {
		mProgressDialog = new QProgressDialog(tr("Downloading update..."), tr("Cancel Update"), 0, 100, this);
		mProgressDialog->setWindowIcon(windowIcon());
		connect(mProgressDialog, SIGNAL(canceled()), mUpdater, SLOT(cancelUpdate()));
		connect(mUpdater, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateProgress(qint64, qint64)));
		connect(mUpdater, SIGNAL(downloadFinished(QString)), mProgressDialog, SLOT(close()));
		//connect(updater, SIGNAL(downloadFinished(QString)), progressDialog, SLOT(deleteLater()));
		connect(mUpdater, SIGNAL(downloadFinished(QString)), this, SLOT(startSetup(QString)));
	}
	//mProgressDialog->setWindowModality(Qt::ApplicationModal);

	mProgressDialog->show();
	//progressDialog->raise();
	//progressDialog->activateWindow();
	//mProgressDialog->setWindowModality(Qt::NonModal);
}

void DkNoMacs::updateProgress(qint64 received, qint64 total) {
	mProgressDialog->setMaximum((int)total);
	mProgressDialog->setValue((int)received);
}

void DkNoMacs::updateProgressTranslations(qint64 received, qint64 total) {
	qDebug() << "rec:" << received << "  total:" << total;
	mProgressDialogTranslations->setMaximum((int)total);
	mProgressDialogTranslations->setValue((int)received);
}

void DkNoMacs::startSetup(QString filePath) {
	
	qDebug() << "starting setup filePath:" << filePath;
	
	if (!QFile::exists(filePath))
		qDebug() << "file does not exist";
	if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filePath))) {
		QString msg = tr("Unable to install new version<br>") +
			tr("You can download the new version from our web page") +
			"<br><a href=\"https://nomacs.org/download/\">www.nomacs.org</a><br>";
		showUpdaterMessage(msg, "update");
	}
}

void DkNoMacs::updateTranslations() {
	
	if (!mTranslationUpdater) {
		mTranslationUpdater = new DkTranslationUpdater(false, this);
		connect(mTranslationUpdater, SIGNAL(showUpdaterMessage(QString, QString)), this, SLOT(showUpdaterMessage(QString, QString)));
	}

	if (!mProgressDialogTranslations) {
		mProgressDialogTranslations = new QProgressDialog(tr("Downloading new translations..."), tr("Cancel"), 0, 100, this);
		mProgressDialogTranslations->setWindowIcon(windowIcon());
		connect(mProgressDialogTranslations, SIGNAL(canceled()), mTranslationUpdater, SLOT(cancelUpdate()));
		//connect(progressDialogTranslations, SIGNAL(canceled()), translationUpdater, SLOT(cancelUpdate()));
		connect(mTranslationUpdater, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateProgressTranslations(qint64, qint64)));
		connect(mTranslationUpdater, SIGNAL(downloadFinished()), mProgressDialogTranslations, SLOT(close()));
	}
	//mProgressDialogTranslations->setWindowModality(Qt::ApplicationModal);

	mProgressDialogTranslations->show();
	//progressDialog->raise();
	//progressDialog->activateWindow();
	//mProgressDialogTranslations->setWindowModality(Qt::NonModal);

	mTranslationUpdater->checkForUpdates();
}

void DkNoMacs::restartWithTranslationUpdate() {
	
	if (!mTranslationUpdater) {
		mTranslationUpdater = new DkTranslationUpdater(false, this);
		connect(mTranslationUpdater, SIGNAL(showUpdaterMessage(QString, QString)), this, SLOT(showUpdaterMessage(QString, QString)));
	}

	mTranslationUpdater->silent = true;
	connect(mTranslationUpdater, SIGNAL(downloadFinished()), this, SLOT(restart()));
	updateTranslations();
}

void DkNoMacs::openPluginManager() {
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

	DkPluginManagerDialog* pluginDialog = new DkPluginManagerDialog(this);
	pluginDialog->exec();
	pluginDialog->deleteLater();

	DkPluginActionManager* am = DkActionManager::instance().pluginActionManager();
	am->updateMenu();

#endif // WITH_PLUGINS
}

// DkNoMacsSync --------------------------------------------------------------------
DkNoMacsSync::DkNoMacsSync(QWidget *parent, Qt::WindowFlags flags) : DkNoMacs(parent, flags) {
}

DkNoMacsSync::~DkNoMacsSync() {
}

void DkNoMacsSync::createActions() {

	DkNoMacs::createActions();

	DkActionManager& am = DkActionManager::instance();
	
	// sync menu
	connect(am.action(DkActionManager::menu_sync_pos), SIGNAL(triggered()), this, SLOT(tcpSendWindowRect()));
	connect(am.action(DkActionManager::menu_sync_arrange), SIGNAL(triggered()), this, SLOT(tcpSendArrange()));

	auto cm = DkSyncManager::inst().client();
	assert(cm);

	// just for local client
	connect(this, SIGNAL(sendArrangeSignal(bool)), cm, SLOT(sendArrangeInstances(bool)));
	connect(this, SIGNAL(sendQuitLocalClientsSignal()), cm, SLOT(sendQuitMessageToPeers()));
	
	connect(cm, SIGNAL(clientConnectedSignal(bool)), this, SLOT(newClientConnected(bool)));
	connect(cm, SIGNAL(receivedPosition(QRect, bool, bool)), this, SLOT(tcpSetWindowRect(QRect, bool, bool)));
}

// mouse events
void DkNoMacsSync::mouseMoveEvent(QMouseEvent *event) {

	int dist = QPoint(event->pos()-mMousePos).manhattanLength();

	// create drag sync action
	if (event->buttons() == Qt::LeftButton && dist > QApplication::startDragDistance() &&
		event->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {

			qDebug() << "generating a drag event...";

			auto cm = dynamic_cast<DkLocalClientManager*>(DkSyncManager::inst().client());
			assert(cm);
			auto md = cm->mimeData();

			QDrag* drag = new QDrag(this);
			drag->setMimeData(md);
			drag->exec(Qt::CopyAction | Qt::MoveAction);
	}
	else
		DkNoMacs::mouseMoveEvent(event);

}

void DkNoMacsSync::dragEnterEvent(QDragEnterEvent *event) {

	if (event->mimeData()->hasFormat("network/sync-dir")) {
		event->accept();
	}

	QMainWindow::dragEnterEvent(event);
}

void DkNoMacsSync::dropEvent(QDropEvent *event) {

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
	}
	else
		QMainWindow::dropEvent(event);
}

DkNoMacsIpl::DkNoMacsIpl(QWidget *parent, Qt::WindowFlags flags) : DkNoMacsSync(parent, flags) {

	// init members
	DkCentralWidget* cw = new DkCentralWidget(this);
	setCentralWidget(cw);
	
	init();
	setAcceptDrops(true);
	setMouseTracking (true);	//receive mouse event everytime

	DkSettingsManager::param().app().appMode = 0;
	DkSettingsManager::param().app().appMode = DkSettings::mode_default;
}

// FramelessNoMacs --------------------------------------------------------------------
DkNoMacsFrameless::DkNoMacsFrameless(QWidget *parent, Qt::WindowFlags flags)
	: DkNoMacs(parent, flags) {

		setObjectName("DkNoMacsFrameless");
		DkSettingsManager::param().app().appMode = DkSettings::mode_frameless;
		
		setWindowFlags(Qt::FramelessWindowHint);
		setAttribute(Qt::WA_TranslucentBackground, true);

		// init members
		DkCentralWidget* cw = new DkCentralWidget(this);
		setCentralWidget(cw);

		init();
		
		setAcceptDrops(true);
		setMouseTracking(true);	//receive mouse event everytime

		// in frameless, you cannot control if menu is visible...
		DkActionManager& am = DkActionManager::instance();
		am.action(DkActionManager::menu_panel_menu)->setEnabled(false);
		am.action(DkActionManager::menu_panel_statusbar)->setEnabled(false);
		am.action(DkActionManager::menu_panel_statusbar)->setChecked(false);
		am.action(DkActionManager::menu_panel_toolbar)->setChecked(false);

		mMenu->setTimeToShow(5000);
		mMenu->hide();
		
		am.action(DkActionManager::menu_view_frameless)->blockSignals(true);
		am.action(DkActionManager::menu_view_frameless)->setChecked(true);
		am.action(DkActionManager::menu_view_frameless)->blockSignals(false);

		mDesktop = QApplication::desktop();

		chooseMonitor(false);
		show();
        
        connect(mDesktop, SIGNAL(workAreaResized(int)), this, SLOT(chooseMonitor()));
		connect(am.action(DkActionManager::menu_view_monitors), SIGNAL(triggered()), this, SLOT(chooseMonitor()));

		setObjectName("DkNoMacsFrameless");
		DkStatusBarManager::instance().show(false);	// fix

		// actions that should always be disabled
		DkActionManager::instance().action(DkActionManager::menu_view_fit_frame)->setEnabled(false);
}

DkNoMacsFrameless::~DkNoMacsFrameless() {
}

void DkNoMacsFrameless::createContextMenu() {

	DkNoMacs::createContextMenu();

	DkActionManager& am = DkActionManager::instance();
	am.contextMenu()->addSeparator();
	am.contextMenu()->addAction(am.action(DkActionManager::menu_file_exit));
}

void DkNoMacsFrameless::chooseMonitor(bool force) {

	if (!mDesktop)
		return;

	QRect screenRect = mDesktop->availableGeometry();

	// ask the user which monitor to use
	if (mDesktop->screenCount() > 1) {
		DkChooseMonitorDialog* cmd = new DkChooseMonitorDialog(this);
		cmd->setWindowTitle(tr("Choose a Monitor"));

		if (force || cmd->showDialog()) {
			int answer = cmd->exec();
			if (answer == QDialog::Accepted) {
				screenRect = cmd->screenRect();
			}
		}
		else {
			screenRect = cmd->screenRect();
		}
	}

	setGeometry(screenRect);
}

// >DIR diem: eating shortcut overrides
bool DkNoMacsFrameless::eventFilter(QObject* , QEvent* event) {

	if (event->type() == QEvent::ShortcutOverride) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

		// consume esc key if fullscreen is on
		if (keyEvent->key() == Qt::Key_Escape && isFullScreen()) {
			exitFullScreen();
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Escape) {
			close();
			return true;
		}
	}
	//if (event->type() == QEvent::Gesture) {
	//	return gestureEvent(static_cast<QGestureEvent*>(event));
	//}

	return false;
}

void DkNoMacsFrameless::closeEvent(QCloseEvent *event) {

	// do not save the window size
	if (mSaveSettings)
		DkSettingsManager::param().save();

	mSaveSettings = false;

	DkNoMacs::closeEvent(event);
}

// Transfer function:

DkNoMacsContrast::DkNoMacsContrast(QWidget *parent, Qt::WindowFlags flags)
	: DkNoMacsSync(parent, flags) {

		setObjectName("DkNoMacsContrast");

		// init members
		DkCentralWidget* cw = new DkCentralWidget(this);
		setCentralWidget(cw);

		init();

		DkToolBarManager::inst().createTransferToolBar();

		setAcceptDrops(true);
		setMouseTracking (true);	//receive mouse event everytime

		DkSettingsManager::param().app().appMode = DkSettings::mode_contrast;
		setObjectName("DkNoMacsContrast");

		// show it...
		show();

		// TODO: this should be checked but no event should be called
		DkActionManager& am = DkActionManager::instance();
		am.action(DkActionManager::menu_panel_transfertoolbar)->blockSignals(true);
		am.action(DkActionManager::menu_panel_transfertoolbar)->setChecked(true);
		am.action(DkActionManager::menu_panel_transfertoolbar)->blockSignals(false);
}

DkNoMacsContrast::~DkNoMacsContrast() {
}

}
