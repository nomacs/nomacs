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
#include "DkToolbars.h"
#include "DkManipulationWidgets.h"
#include "DkSettingsWidgets.h"
#include "DkMessageBox.h"
#include "DkMetaDataWidgets.h"
#include "DkThumbsWidgets.h"
#include "DkBatch.h"
#include "DkCentralWidget.h"
#include "DkMetaData.h"
#include "DkImageContainer.h"
#include "DkQuickAccess.h"
#include "DkError.h"
#include "DkUtils.h"
#include "DkControlWidget.h"
#include "DkImageLoader.h"
#include "DkTimer.h"
#include "DkActionManager.h"

#ifdef  WITH_PLUGINS
#include "DkPluginInterface.h"
#include "DkPluginManager.h"
#endif //  WITH_PLUGINS

#pragma warning(push, 0)	// no warnings from includes - begin
#ifdef WITH_UPNP
#include "DkUpnp.h"
#endif // WITH_UPNP

#include <QBoxLayout>
#include <QShortcut>
#include <QResizeEvent>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QStatusBar>
#include <QPanGesture>
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
#pragma warning(pop)		// no warnings from includes - end

#if defined(WIN32) && !defined(SOCK_STREAM)
#include <winsock2.h>	// needed since libraw 0.16
#endif

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

	DkActionManager::instance().createActions(this);
	registerFileVersion();

	mSaveSettings = true;

	// load settings
	//DkSettings::load();
	
	mOpenDialog = 0;
	mSaveDialog = 0;
	mThumbSaver = 0;
	mResizeDialog = 0;
	mOpacityDialog = 0;
	mUpdater = 0;
	mTranslationUpdater = 0;
	mImgManipulationDialog = 0;
	mExportTiffDialog = 0;
	mUpdateDialog = 0;
	mProgressDialog = 0;
	mProgressDialogTranslations = 0;
	mForceDialog = 0;
	mTrainDialog = 0;
	mExplorer = 0;
	mMetaDataDock = 0;
	mSettingsDialog = 0;
	mPrintPreviewDialog = 0;
	mThumbsDock = 0;
	mQuickAccess = 0;
#ifdef WITH_QUAZIP
	mArchiveExtractionDialog = 0;
#endif 

	// start localhost client/server
	//localClientManager = new DkLocalClientManager(windowTitle());
	//localClientManger->start();

	mOldGeometry = geometry();
	mOverlaid = false;

	mMenu = new DkMenuBar(this, -1);

	resize(850, 504);
	setMinimumSize(20, 20);

	double an = std::pow(3987, 12);
	double bn = std::pow(4365, 12);

	qDebug() << "3987 ^ 12 + 4365 ^ 12 = " << std::pow(an + bn, 1/12.0) << "^ 12";
	qDebug() << "Sorry Fermat, but the Simpsons are right.";

}

DkNoMacs::~DkNoMacs() {
	release();
}

void DkNoMacs::release() {
}

void DkNoMacs::init() {

// assign icon -> in windows the 32px version
#ifdef WIN32
	QString iconPath = ":/nomacs/img/nomacs32.png";
#else
	QString iconPath = ":/nomacs/img/nomacs.png";
#endif

	loadStyleSheet();

	QIcon nmcIcon = QIcon(iconPath);
	setObjectName("DkNoMacs");
	
	if (!nmcIcon.isNull())
		setWindowIcon(nmcIcon);

	// shortcuts and actions
	createActions();
	createMenu();
	createContextMenu();
	createToolbar();
	createStatusbar();
	enableNoImageActions(false);

	// TODO - just for android register me as a gesture recognizer
	grabGesture(Qt::PanGesture);
	grabGesture(Qt::PinchGesture);
	grabGesture(Qt::SwipeGesture);

	// load the window at the same position as last time
	readSettings();
	installEventFilter(this);

	showMenuBar(DkSettings::app.showMenuBar);
	showToolbar(DkSettings::app.showToolBar);
	showStatusBar(DkSettings::app.showStatusBar);

	// connects that are needed in all viewers
	connect(viewport(), SIGNAL(showStatusBar(bool, bool)), this, SLOT(showStatusBar(bool, bool)));
	connect(viewport(), SIGNAL(statusInfoSignal(const QString&, int)), this, SLOT(showStatusMessage(const QString&, int)));
	connect(viewport()->getController()->getCropWidget(), SIGNAL(statusInfoSignal(const QString&)), this, SLOT(showStatusMessage(const QString&)));
	connect(viewport(), SIGNAL(enableNoImageSignal(bool)), this, SLOT(enableNoImageActions(bool)));

	// connections to the image loader
	connect(getTabWidget(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), this, SLOT(setWindowTitle(QSharedPointer<DkImageContainerT>)));

	connect(viewport()->getController()->getCropWidget(), SIGNAL(showToolbar(QToolBar*, bool)), this, SLOT(showToolbar(QToolBar*, bool)));
	connect(viewport(), SIGNAL(movieLoadedSignal(bool)), this, SLOT(enableMovieActions(bool)));
	connect(viewport()->getController()->getFilePreview(), SIGNAL(showThumbsDockSignal(bool)), this, SLOT(showThumbsDock(bool)));
	connect(centralWidget(), SIGNAL(statusInfoSignal(const QString&, int)), this, SLOT(showStatusMessage(const QString&, int)));

	enableMovieActions(false);

// clean up nomacs
#ifdef WIN32
	if (!nmc::DkSettings::global.setupPath.isEmpty() && QApplication::applicationVersion() == nmc::DkSettings::global.setupVersion) {

		// ask for exists - otherwise we always try to delete it if the user deleted it
		if (!QFileInfo(nmc::DkSettings::global.setupPath).exists() || QFile::remove(nmc::DkSettings::global.setupPath)) {
			nmc::DkSettings::global.setupPath = "";
			nmc::DkSettings::global.setupVersion = "";
			DkSettings::save();
		}
	}
#endif // Q_WS_WIN

	//QTimer::singleShot(0, this, SLOT(onWindowLoaded()));
}

#ifdef WIN32	// windows specific versioning

void DkNoMacs::registerFileVersion() {
	
	// this function is based on code from:
	// http://stackoverflow.com/questions/316626/how-do-i-read-from-a-version-resource-in-visual-c

	QString version(NOMACS_VERSION);	// default version (we do not know the build)

	try {
		// get the filename of the executable containing the version resource
		TCHAR szFilename[MAX_PATH + 1] = {0};
		if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0) {
			DkFileException("Sorry, I can't read the module fileInfo name\n", __LINE__, __FILE__);
		}

		// allocate a block of memory for the version info
		DWORD dummy;
		DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
		if (dwSize == 0) {
			throw DkFileException("The version info size is zero\n", __LINE__, __FILE__);
		}
		std::vector<BYTE> bytes(dwSize);

		if (bytes.empty())
			throw DkFileException("The version info is empty\n", __LINE__, __FILE__);

		// load the version info
		if (!bytes.empty() && !GetFileVersionInfo(szFilename, NULL, dwSize, &bytes[0])) {
			throw DkFileException("Sorry, I can't read the version info\n", __LINE__, __FILE__);
		}

		// get the name and version strings
		UINT                uiVerLen = 0;
		VS_FIXEDFILEINFO*   pFixedInfo = 0;     // pointer to fixed file info structure

		if (!bytes.empty() && !VerQueryValue(&bytes[0], TEXT("\\"), (void**)&pFixedInfo, (UINT *)&uiVerLen)) {
			throw DkFileException("Sorry, I can't get the version values...\n", __LINE__, __FILE__);
		}

		// pFixedInfo contains a lot more information...
		version = QString::number(HIWORD(pFixedInfo->dwFileVersionMS)) % "."
			% QString::number(LOWORD(pFixedInfo->dwFileVersionMS)) % "."
			% QString::number(HIWORD(pFixedInfo->dwFileVersionLS)) % "."
			% QString::number(LOWORD(pFixedInfo->dwFileVersionLS));

	} catch (DkFileException dfe) {
		qDebug() << QString::fromStdString(dfe.Msg());
	}

	QApplication::setApplicationVersion(version);


}
#else
	void DkNoMacs::registerFileVersion() {
		QString version(NOMACS_VERSION);	// default version (we do not know the build)
		QApplication::setApplicationVersion(version);
	}
#endif

void DkNoMacs::createToolbar() {

	mToolbar = new DkMainToolBar(tr("Edit"), this);
	mToolbar->setObjectName("EditToolBar");

	if (DkSettings::display.smallIcons)
		mToolbar->setIconSize(QSize(16, 16));
	else
		mToolbar->setIconSize(QSize(32, 32));
	
	qDebug() << mToolbar->styleSheet();

	if (DkSettings::display.toolbarGradient) {
		mToolbar->setObjectName("toolBarWithGradient");
	}

	DkActionManager& am = DkActionManager::instance();

	mToolbar->addAction(am.action(DkActionManager::menu_file_prev));
	mToolbar->addAction(am.action(DkActionManager::menu_file_next));
	mToolbar->addSeparator();

	mToolbar->addAction(am.action(DkActionManager::menu_file_open));
	mToolbar->addAction(am.action(DkActionManager::menu_file_open_dir));
	mToolbar->addAction(am.action(DkActionManager::menu_file_save));
	mToolbar->addAction(am.action(DkActionManager::menu_tools_filter));
	mToolbar->addSeparator();

	// edit
	mToolbar->addAction(am.action(DkActionManager::menu_edit_copy));
	mToolbar->addAction(am.action(DkActionManager::menu_edit_paste));
	mToolbar->addSeparator();

	// edit
	mToolbar->addAction(am.action(DkActionManager::menu_edit_rotate_ccw));
	mToolbar->addAction(am.action(DkActionManager::menu_edit_rotate_cw));
	mToolbar->addSeparator();

	mToolbar->addAction(am.action(DkActionManager::menu_edit_crop));
	mToolbar->addAction(am.action(DkActionManager::menu_edit_transform));
	mToolbar->addSeparator();

	// view
	mToolbar->addAction(am.action(DkActionManager::menu_view_fullscreen));
	mToolbar->addAction(am.action(DkActionManager::menu_view_reset));
	mToolbar->addAction(am.action(DkActionManager::menu_view_100));
	mToolbar->addSeparator();

	mToolbar->addAction(am.action(DkActionManager::menu_view_gps_map));

	mMovieToolbar = addToolBar(tr("Movie Toolbar"));
	mMovieToolbar->setObjectName("movieToolbar");
	mMovieToolbar->addAction(am.action(DkActionManager::menu_view_movie_prev));
	mMovieToolbar->addAction(am.action(DkActionManager::menu_view_movie_pause));
	mMovieToolbar->addAction(am.action(DkActionManager::menu_view_movie_next));

	if (DkSettings::display.toolbarGradient)
		mMovieToolbar->setObjectName("toolBarWithGradient");

	if (DkSettings::display.smallIcons)
		mMovieToolbar->setIconSize(QSize(16, 16));
	else
		mMovieToolbar->setIconSize(QSize(32, 32));

	mToolbar->allActionsAdded();

	addToolBar(mToolbar);
}


void DkNoMacs::createStatusbar() {

	mStatusbarLabels.resize(status_end);

	mStatusbarLabels[status_pixel_info] = new QLabel();
	mStatusbarLabels[status_pixel_info]->hide();
	mStatusbarLabels[status_pixel_info]->setToolTip(tr("CTRL activates the crosshair cursor"));

	mStatusbar = new QStatusBar(this);
	mStatusbar->setObjectName("DkStatusBar");
	QColor col = QColor(200, 200, 230, 100);

	if (DkSettings::display.toolbarGradient)
		mStatusbar->setObjectName("statusBarWithGradient");	

	mStatusbar->addWidget(mStatusbarLabels[status_pixel_info]);
	mStatusbar->hide();

	for (int idx = 1; idx < mStatusbarLabels.size(); idx++) {
		mStatusbarLabels[idx] = new QLabel(this);
		mStatusbarLabels[idx]->setObjectName("statusBarLabel");
		mStatusbarLabels[idx]->hide();
		mStatusbar->addPermanentWidget(mStatusbarLabels[idx]);
	}

	//statusbar->addPermanentWidget()
	this->setStatusBar(mStatusbar);
}

void DkNoMacs::loadStyleSheet() {
	
	// TODO: if we first load from disk, people can style nomacs themselves
	QFileInfo cssInfo(QCoreApplication::applicationDirPath(), "nomacs.css");

	if (!cssInfo.exists())
		cssInfo = QFileInfo(":/nomacs/stylesheet.css");

	QFile file(cssInfo.absoluteFilePath());

	if (file.open(QFile::ReadOnly)) {

		QString cssString = file.readAll();

		QColor hc = DkSettings::display.highlightColor;
		hc.setAlpha(150);

		// replace color placeholders
		cssString.replace("HIGHLIGHT_COLOR", DkUtils::colorToString(DkSettings::display.highlightColor));
		cssString.replace("HIGHLIGHT_LIGHT", DkUtils::colorToString(hc));
		cssString.replace("HUD_BACKGROUND_COLOR", DkUtils::colorToString(DkSettings::display.bgColorWidget));
		cssString.replace("HUD_FONT_COLOR", DkUtils::colorToString(QColor(255,255,255)));
		cssString.replace("BACKGROUND_COLOR", DkUtils::colorToString(DkSettings::display.bgColor));
		cssString.replace("WINDOW_COLOR", DkUtils::colorToString(QPalette().color(QPalette::Window)));

		qApp->setStyleSheet(cssString);
		file.close();

		qDebug() << "CSS loaded from: " << cssInfo.absoluteFilePath();
		//qDebug() << "style: \n" << cssString;
	}
}

void DkNoMacs::createMenu() {

	setMenuBar(mMenu);
	DkActionManager& am = DkActionManager::instance();
	am.createMenus(mMenu);
	mMenu->addMenu(am.fileMenu());
	mMenu->addMenu(am.editMenu());
	mMenu->addMenu(am.viewMenu());
	mMenu->addMenu(am.panelMenu());
	mMenu->addMenu(am.toolsMenu());

	// no sync menu in frameless view
	if (DkSettings::app.appMode != DkSettings::mode_frameless)
		mSyncMenu = mMenu->addMenu(tr("&Sync"));

#ifdef WITH_PLUGINS
	// plugin menu
	mPluginsMenu = mMenu->addMenu(tr("Pl&ugins"));
	am.pluginActionManager()->setMenu(mPluginsMenu);
#endif // WITH_PLUGINS

	mMenu->addMenu(am.helpMenu());
}

//void DkNoMacs::createOpenWithMenu(QMenu*) {
//
//	QList<QAction* > oldActions = mOpenWithMenu->findChildren<QAction* >();
//
//	// remove old actions
//	for (int idx = 0; idx < oldActions.size(); idx++)
//		viewport()->removeAction(oldActions.at(idx));
//
//	QVector<QAction* > appActions = mAppManager->getActions();
//
//	for (int idx = 0; idx < appActions.size(); idx++)
//		qDebug() << "adding action: " << appActions[idx]->text() << " " << appActions[idx]->toolTip();
//
//	assignCustomShortcuts(appActions);
//	mOpenWithMenu->addActions(appActions.toList());
//	
//	if (!appActions.empty())
//		mOpenWithMenu->addSeparator();
//	mOpenWithMenu->addAction(mFileActions[menu_file_app_manager]);
//	centralWidget()->addActions(appActions.toList());
//}

void DkNoMacs::createContextMenu() {
}

void DkNoMacs::createActions() {
	
	DkViewPort* vp = viewport();

	DkActionManager& am = DkActionManager::instance();

	connect(am.action(DkActionManager::menu_file_open), SIGNAL(triggered()), this, SLOT(openFile()));
	connect(am.action(DkActionManager::menu_file_open_dir), SIGNAL(triggered()), this, SLOT(openDir()));
	connect(am.action(DkActionManager::menu_file_quick_launch), SIGNAL(triggered()), this, SLOT(openQuickLaunch()));
	connect(am.action(DkActionManager::menu_file_app_manager), SIGNAL(triggered()), this, SLOT(openAppManager()));
	connect(am.action(DkActionManager::menu_file_rename), SIGNAL(triggered()), this, SLOT(renameFile()));
	connect(am.action(DkActionManager::menu_file_goto), SIGNAL(triggered()), this, SLOT(goTo()));
	connect(am.action(DkActionManager::menu_file_save), SIGNAL(triggered()), this, SLOT(saveFile()));
	connect(am.action(DkActionManager::menu_file_save_as), SIGNAL(triggered()), this, SLOT(saveFileAs()));
	connect(am.action(DkActionManager::menu_file_save_web), SIGNAL(triggered()), this, SLOT(saveFileWeb()));
	connect(am.action(DkActionManager::menu_file_print), SIGNAL(triggered()), this, SLOT(printDialog()));
	connect(am.action(DkActionManager::menu_file_show_recent), SIGNAL(triggered(bool)), centralWidget(), SLOT(showRecentFiles(bool)));	
	connect(am.action(DkActionManager::menu_file_train_format), SIGNAL(triggered()), this, SLOT(trainFormat()));
	connect(am.action(DkActionManager::menu_file_new_instance), SIGNAL(triggered()), this, SLOT(newInstance()));
	connect(am.action(DkActionManager::menu_file_private_instance), SIGNAL(triggered()), this, SLOT(newInstance()));
	connect(am.action(DkActionManager::menu_file_find), SIGNAL(triggered()), this, SLOT(find()));
	connect(am.action(DkActionManager::menu_file_recursive), SIGNAL(triggered(bool)), this, SLOT(setRecursiveScan(bool)));
	connect(am.action(DkActionManager::menu_file_exit), SIGNAL(triggered()), this, SLOT(close()));
	
	// TODO: move to viewport
	connect(am.action(DkActionManager::menu_file_reload), SIGNAL(triggered()), vp, SLOT(reloadFile()));
	connect(am.action(DkActionManager::menu_file_next), SIGNAL(triggered()), vp, SLOT(loadNextFileFast()));
	connect(am.action(DkActionManager::menu_file_prev), SIGNAL(triggered()), vp, SLOT(loadPrevFileFast()));
	connect(am.action(DkActionManager::menu_edit_rotate_cw), SIGNAL(triggered()), vp, SLOT(rotateCW()));
	connect(am.action(DkActionManager::menu_edit_rotate_ccw), SIGNAL(triggered()), vp, SLOT(rotateCCW()));
	connect(am.action(DkActionManager::menu_edit_rotate_180), SIGNAL(triggered()), vp, SLOT(rotate180()));
	connect(am.action(DkActionManager::menu_edit_copy), SIGNAL(triggered()), vp, SLOT(copyImage()));
	connect(am.action(DkActionManager::menu_edit_copy_buffer), SIGNAL(triggered()), vp, SLOT(copyImageBuffer()));
	connect(am.action(DkActionManager::menu_edit_copy_color), SIGNAL(triggered()), vp, SLOT(copyPixelColorValue()));
	connect(am.action(DkActionManager::menu_edit_paste), SIGNAL(triggered()), centralWidget(), SLOT(pasteImage()));
	connect(am.action(DkActionManager::menu_edit_crop), SIGNAL(triggered(bool)), vp->getController(), SLOT(showCrop(bool)));

	connect(am.action(DkActionManager::menu_panel_overview), SIGNAL(toggled(bool)), vp->getController(), SLOT(showOverview(bool)));
	connect(am.action(DkActionManager::menu_panel_player), SIGNAL(toggled(bool)), vp->getController(), SLOT(showPlayer(bool)));
	connect(am.action(DkActionManager::menu_panel_preview), SIGNAL(toggled(bool)), vp->getController(), SLOT(showPreview(bool)));
	connect(am.action(DkActionManager::menu_panel_thumbview), SIGNAL(toggled(bool)), getTabWidget(), SLOT(showThumbView(bool)));
	connect(am.action(DkActionManager::menu_panel_scroller), SIGNAL(toggled(bool)), vp->getController(), SLOT(showScroller(bool)));
	connect(am.action(DkActionManager::menu_panel_exif), SIGNAL(toggled(bool)), vp->getController(), SLOT(showMetaData(bool)));
	connect(am.action(DkActionManager::menu_panel_info), SIGNAL(toggled(bool)), vp->getController(), SLOT(showFileInfo(bool)));
	connect(am.action(DkActionManager::menu_panel_histogram), SIGNAL(toggled(bool)), vp->getController(), SLOT(showHistogram(bool)));
	connect(am.action(DkActionManager::menu_panel_comment), SIGNAL(toggled(bool)), vp->getController(), SLOT(showCommentWidget(bool)));

	connect(am.action(DkActionManager::menu_view_reset), SIGNAL(triggered()), vp, SLOT(zoomToFit()));
	connect(am.action(DkActionManager::menu_view_100), SIGNAL(triggered()), vp, SLOT(fullView()));
	connect(am.action(DkActionManager::menu_view_zoom_in), SIGNAL(triggered()), vp, SLOT(zoomIn()));
	connect(am.action(DkActionManager::menu_view_zoom_out), SIGNAL(triggered()), vp, SLOT(zoomOut()));
	connect(am.action(DkActionManager::menu_view_anti_aliasing), SIGNAL(toggled(bool)), vp->getImageStorage(), SLOT(antiAliasingChanged(bool)));
	connect(am.action(DkActionManager::menu_view_tp_pattern), SIGNAL(toggled(bool)), vp, SLOT(togglePattern(bool)));
	connect(am.action(DkActionManager::menu_view_new_tab), SIGNAL(triggered()), centralWidget(), SLOT(addTab()));
	connect(am.action(DkActionManager::menu_view_close_tab), SIGNAL(triggered()), centralWidget(), SLOT(removeTab()));
	connect(am.action(DkActionManager::menu_view_previous_tab), SIGNAL(triggered()), centralWidget(), SLOT(previousTab()));
	connect(am.action(DkActionManager::menu_view_next_tab), SIGNAL(triggered()), centralWidget(), SLOT(nextTab()));
	connect(am.action(DkActionManager::menu_view_movie_pause), SIGNAL(triggered(bool)), vp, SLOT(pauseMovie(bool)));
	connect(am.action(DkActionManager::menu_view_movie_prev), SIGNAL(triggered()), vp, SLOT(previousMovieFrame()));
	connect(am.action(DkActionManager::menu_view_movie_next), SIGNAL(triggered()), vp, SLOT(nextMovieFrame()));

	connect(am.action(DkActionManager::menu_tools_batch), SIGNAL(triggered()), getTabWidget(), SLOT(startBatchProcessing()));

	// TODO end

	connect(am.action(DkActionManager::menu_sort_filename), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));
	connect(am.action(DkActionManager::menu_sort_date_created), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));
	connect(am.action(DkActionManager::menu_sort_date_modified), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));
	connect(am.action(DkActionManager::menu_sort_random), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));
	connect(am.action(DkActionManager::menu_sort_ascending), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));
	connect(am.action(DkActionManager::menu_sort_descending), SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));

	connect(am.action(DkActionManager::menu_edit_transform), SIGNAL(triggered()), this, SLOT(resizeImage()));
	connect(am.action(DkActionManager::menu_edit_flip_h), SIGNAL(triggered()), this, SLOT(flipImageHorizontal()));
	connect(am.action(DkActionManager::menu_edit_flip_v), SIGNAL(triggered()), this, SLOT(flipImageVertical()));
	connect(am.action(DkActionManager::menu_edit_norm), SIGNAL(triggered()), this, SLOT(normalizeImage()));
	connect(am.action(DkActionManager::menu_edit_auto_adjust), SIGNAL(triggered()), this, SLOT(autoAdjustImage()));
	connect(am.action(DkActionManager::menu_edit_invert), SIGNAL(triggered()), this, SLOT(invertImage()));
	connect(am.action(DkActionManager::menu_edit_gray_convert), SIGNAL(triggered()), this, SLOT(convert2gray()));
	connect(am.action(DkActionManager::menu_edit_unsharp), SIGNAL(triggered()), this, SLOT(unsharpMask()));
	connect(am.action(DkActionManager::menu_edit_tiny_planet), SIGNAL(triggered()), this, SLOT(tinyPlanet()));
	connect(am.action(DkActionManager::menu_edit_delete), SIGNAL(triggered()), this, SLOT(deleteFile()));
	connect(am.action(DkActionManager::menu_edit_wallpaper), SIGNAL(triggered()), this, SLOT(setWallpaper()));
	connect(am.action(DkActionManager::menu_edit_preferences), SIGNAL(triggered()), this, SLOT(openSettings()));

	connect(am.action(DkActionManager::menu_panel_menu), SIGNAL(toggled(bool)), this, SLOT(showMenuBar(bool)));
	connect(am.action(DkActionManager::menu_panel_toolbar), SIGNAL(toggled(bool)), this, SLOT(showToolbar(bool)));
	connect(am.action(DkActionManager::menu_panel_statusbar), SIGNAL(toggled(bool)), this, SLOT(showStatusBar(bool)));
	connect(am.action(DkActionManager::menu_panel_transfertoolbar), SIGNAL(toggled(bool)), this, SLOT(setContrast(bool)));
	connect(am.action(DkActionManager::menu_panel_explorer), SIGNAL(toggled(bool)), this, SLOT(showExplorer(bool)));
	connect(am.action(DkActionManager::menu_panel_metadata_dock), SIGNAL(toggled(bool)), this, SLOT(showMetaDataDock(bool)));
	connect(am.action(DkActionManager::menu_panel_preview), SIGNAL(toggled(bool)), this, SLOT(showThumbsDock(bool)));

	connect(am.action(DkActionManager::menu_view_fit_frame), SIGNAL(triggered()), this, SLOT(fitFrame()));
	connect(am.action(DkActionManager::menu_view_fullscreen), SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
	connect(am.action(DkActionManager::menu_view_frameless), SIGNAL(toggled(bool)), this, SLOT(setFrameless(bool)));
	connect(am.action(DkActionManager::menu_view_opacity_change), SIGNAL(triggered()), this, SLOT(showOpacityDialog()));
	connect(am.action(DkActionManager::menu_view_opacity_up), SIGNAL(triggered()), this, SLOT(opacityUp()));
	connect(am.action(DkActionManager::menu_view_opacity_down), SIGNAL(triggered()), this, SLOT(opacityDown()));
	connect(am.action(DkActionManager::menu_view_opacity_an), SIGNAL(triggered()), this, SLOT(animateChangeOpacity()));
	connect(am.action(DkActionManager::menu_view_lock_window), SIGNAL(triggered(bool)), this, SLOT(lockWindow(bool)));
	connect(am.action(DkActionManager::menu_view_gps_map), SIGNAL(triggered()), this, SLOT(showGpsCoordinates()));
	
	connect(am.action(DkActionManager::menu_tools_thumbs), SIGNAL(triggered()), this, SLOT(computeThumbsBatch()));
	connect(am.action(DkActionManager::menu_tools_filter), SIGNAL(triggered(bool)), this, SLOT(find(bool)));
	connect(am.action(DkActionManager::menu_tools_manipulation), SIGNAL(triggered()), this, SLOT(openImgManipulationDialog()));
	connect(am.action(DkActionManager::menu_tools_export_tiff), SIGNAL(triggered()), this, SLOT(exportTiff()));
	connect(am.action(DkActionManager::menu_tools_extract_archive), SIGNAL(triggered()), this, SLOT(extractImagesFromArchive()));
	connect(am.action(DkActionManager::menu_tools_mosaic), SIGNAL(triggered()), this, SLOT(computeMosaic()));

	connect(am.action(DkActionManager::sc_test_img), SIGNAL(triggered()), vp, SLOT(loadLena()));
	connect(am.action(DkActionManager::sc_test_rec), SIGNAL(triggered()), this, SLOT(loadRecursion()));
	connect(am.action(DkActionManager::sc_test_pong), SIGNAL(triggered()), this, SLOT(startPong()));
	
	connect(am.action(DkActionManager::menu_plugin_manager), SIGNAL(triggered()), this, SLOT(openPluginManager()));

	// help menu
	connect(am.action(DkActionManager::menu_help_about), SIGNAL(triggered()), this, SLOT(aboutDialog()));
	connect(am.action(DkActionManager::menu_help_documentation), SIGNAL(triggered()), this, SLOT(openDocumentation()));
	connect(am.action(DkActionManager::menu_help_bug), SIGNAL(triggered()), this, SLOT(bugReport()));
	connect(am.action(DkActionManager::menu_help_feature), SIGNAL(triggered()), this, SLOT(featureRequest()));
	connect(am.action(DkActionManager::menu_help_update), SIGNAL(triggered()), this, SLOT(checkForUpdate()));
	connect(am.action(DkActionManager::menu_help_update_translation), SIGNAL(triggered()), this, SLOT(updateTranslations()));

	//// add sort actions to the thumbscene
	//getTabWidget()->getThumbScrollWidget()->addContextMenuActions(mSortActions, tr("&Sort"));
}

void DkNoMacs::enableNoImageActions(bool enable) {

	DkActionManager& am = DkActionManager::instance();

	am.action(DkActionManager::menu_file_save)->setEnabled(enable);
	am.action(DkActionManager::menu_file_save_as)->setEnabled(enable);
	am.action(DkActionManager::menu_file_save_web)->setEnabled(enable);
	am.action(DkActionManager::menu_file_rename)->setEnabled(enable);
	am.action(DkActionManager::menu_file_print)->setEnabled(enable);
	am.action(DkActionManager::menu_file_reload)->setEnabled(enable);
	am.action(DkActionManager::menu_file_prev)->setEnabled(enable);
	am.action(DkActionManager::menu_file_next)->setEnabled(enable);
	am.action(DkActionManager::menu_file_goto)->setEnabled(enable);
	am.action(DkActionManager::menu_file_find)->setEnabled(enable);

	am.action(DkActionManager::menu_edit_rotate_cw)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_rotate_ccw)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_rotate_180)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_delete)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_transform)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_crop)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_copy)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_copy_buffer)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_copy_color)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_wallpaper)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_flip_h)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_flip_v)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_norm)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_auto_adjust)->setEnabled(enable);
#ifdef WITH_OPENCV
	am.action(DkActionManager::menu_edit_unsharp)->setEnabled(enable);
#else
	editActions[menu_edit_unsharp)->setEnabled(false);
#endif
#ifdef WITH_OPENCV
	am.action(DkActionManager::menu_edit_tiny_planet)->setEnabled(enable);
#else
	editActions[menu_edit_tiny_planet)->setEnabled(false);
#endif

	am.action(DkActionManager::menu_edit_invert)->setEnabled(enable);
	am.action(DkActionManager::menu_edit_gray_convert)->setEnabled(enable);	

	am.action(DkActionManager::menu_tools_thumbs)->setEnabled(enable);
	
	am.action(DkActionManager::menu_panel_info)->setEnabled(enable);
#ifdef WITH_OPENCV
	am.action(DkActionManager::menu_panel_histogram)->setEnabled(enable);
#else
	panelActions[menu_panel_histogram)->setEnabled(false);
#endif
	am.action(DkActionManager::menu_panel_scroller)->setEnabled(enable);
	am.action(DkActionManager::menu_panel_comment)->setEnabled(enable);
	am.action(DkActionManager::menu_panel_preview)->setEnabled(enable);
	am.action(DkActionManager::menu_panel_exif)->setEnabled(enable);
	am.action(DkActionManager::menu_panel_overview)->setEnabled(enable);
	am.action(DkActionManager::menu_panel_player)->setEnabled(enable);
	
	am.action(DkActionManager::menu_view_fullscreen)->setEnabled(enable);
	am.action(DkActionManager::menu_view_reset)->setEnabled(enable);
	am.action(DkActionManager::menu_view_100)->setEnabled(enable);
	am.action(DkActionManager::menu_view_fit_frame)->setEnabled(enable);
	am.action(DkActionManager::menu_view_zoom_in)->setEnabled(enable);
	am.action(DkActionManager::menu_view_zoom_out)->setEnabled(enable);

#ifdef WITH_OPENCV
	am.action(DkActionManager::menu_tools_manipulation)->setEnabled(enable);
#else
	toolsActions[menu_tools_manipulation)->setEnabled(false);
#endif

	// disable open with actions
	for (QAction* a : DkActionManager::instance().appManager()->getActions())
		a->setEnabled(enable);

}

void DkNoMacs::enableMovieActions(bool enable) {

	DkSettings::app.showMovieToolBar = enable;
	
	DkActionManager& am = DkActionManager::instance();

	am.action(DkActionManager::menu_view_movie_pause)->setEnabled(enable);
	am.action(DkActionManager::menu_view_movie_prev)->setEnabled(enable);
	am.action(DkActionManager::menu_view_movie_next)->setEnabled(enable);

	am.action(DkActionManager::menu_view_movie_pause)->setChecked(false);
	
	if (enable)
		addToolBar(mMovieToolbar);
	else
		removeToolBar(mMovieToolbar);
	
	if (mToolbar->isVisible())
		mMovieToolbar->setVisible(enable);
}

void DkNoMacs::clearFileHistory() {
	DkSettings::global.recentFiles.clear();
}

void DkNoMacs::clearFolderHistory() {
	DkSettings::global.recentFolders.clear();
}


DkViewPort* DkNoMacs::viewport() const {

	DkCentralWidget* cw = dynamic_cast<DkCentralWidget*>(centralWidget());

	if (!cw)
		return 0;

	return cw->getViewPort();
}

DkCentralWidget* DkNoMacs::getTabWidget() const {

	DkCentralWidget* cw = dynamic_cast<DkCentralWidget*>(centralWidget());
	return cw;
}

void DkNoMacs::updateAll() {

	QWidgetList w = QApplication::topLevelWidgets();
	for (int idx = 0; idx < w.size(); idx++) {
		if (w[idx]->objectName().contains(QString("DkNoMacs")))
			w[idx]->update();
	}
}

//QWidget* DkNoMacs::getDialogParent() {
//
//	QWidgetList wList = QApplication::topLevelWidgets();
//	for (int idx = 0; idx < wList.size(); idx++) {
//		if (wList[idx]->objectName().contains(QString("DkNoMacs")))
//			return wList[idx];
//	}
//
//	return 0;
//}

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

	if (viewport()) {
		if (!viewport()->unloadImage(true)) {
			// do not close if the user hit cancel in the save changes dialog
			event->ignore();
			return;
		}
	}

	emit closeSignal();
	qDebug() << "saving window settings...";
	setVisible(false);
	//showNormal();

	if (mSaveSettings) {
		QSettings& settings = Settings::instance().getSettings();
		settings.setValue("geometryNomacs", geometry());
		settings.setValue("geometry", saveGeometry());
		settings.setValue("windowState", saveState());
		
		if (mExplorer)
			settings.setValue(mExplorer->objectName(), QMainWindow::dockWidgetArea(mExplorer));
		if (mMetaDataDock)
			settings.setValue(mMetaDataDock->objectName(), QMainWindow::dockWidgetArea(mMetaDataDock));
		if (mThumbsDock)
			settings.setValue(mThumbsDock->objectName(), QMainWindow::dockWidgetArea(mThumbsDock));

		DkSettings::save();
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

	if (event->button() != Qt::LeftButton || (viewport() && viewport()->getImage().isNull()))
		return;

	if (isFullScreen())
		exitFullScreen();
	else
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

bool DkNoMacs::gestureEvent(QGestureEvent *event) {
	
	DkViewPort* vp = viewport();

	if (QGesture *swipe = event->gesture(Qt::SwipeGesture)) {
		QSwipeGesture* swipeG = static_cast<QSwipeGesture *>(swipe);

		qDebug() << "swipe detected\n";
		if (vp) {
			
			if (swipeG->horizontalDirection() == QSwipeGesture::Left)
				vp->loadNextFileFast();
			else if (swipeG->horizontalDirection() == QSwipeGesture::Right)
				vp->loadPrevFileFast();

			// TODO: recognize some other gestures please
		}

	}
	else if (QGesture *pan = event->gesture(Qt::PanGesture)) {
		
		QPanGesture* panG = static_cast<QPanGesture *>(pan);

		qDebug() << "you're speedy: " << panG->acceleration();

		QPointF delta = panG->delta();

		if (panG->acceleration() > 10 && delta.x() && fabs(delta.y()/delta.x()) < 0.2) {
			
			if (delta.x() < 0)
				vp->loadNextFileFast();
			else
				vp->loadPrevFileFast();
		}

		if (vp)
			vp->moveView(panG->delta());
	}
	else if (QGesture *pinch = event->gesture(Qt::PinchGesture)) {

		QPinchGesture* pinchG = static_cast<QPinchGesture *>(pinch);

		//if (pinchG->changeFlags() == QPinchGesture::ChangeFlag.ScaleFactorChanged) {
		qDebug() << "scale Factor: " << pinchG->scaleFactor();
		if (pinchG->scaleFactor() != 0 && vp) {
			vp->zoom((float)pinchG->scaleFactor());
		}
		else if (pinchG->rotationAngle() != 0 && vp) {

			float angle = (float)pinchG->rotationAngle();
			qDebug() << "angle: " << angle;
			//vp->rotate(angle);
		}
	}

	qDebug() << "gesture event (NoMacs)";

	//	pinchTriggered(static_cast<QPinchGesture *>(pinch));
	return true;
}

void DkNoMacs::flipImageHorizontal() {

	DkViewPort* vp = viewport();

	if (!vp)
		return;

	viewport()->getController()->applyPluginChanges(true);

	QImage img = vp->getImage();
	img = img.mirrored(true, false);

	if (img.isNull())
		vp->getController()->setInfo(tr("Sorry, I cannot Flip the Image..."));
	else
		vp->setEditedImage(img);
}

void DkNoMacs::flipImageVertical() {

	DkViewPort* vp = viewport();

	if (!vp)
		return;

	viewport()->getController()->applyPluginChanges(true);

	QImage img = vp->getImage();
	img = img.mirrored(false, true);

	if (img.isNull())
		vp->getController()->setInfo(tr("Sorry, I cannot Flip the Image..."));
	else
		vp->setEditedImage(img);

}

void DkNoMacs::invertImage() {

	DkViewPort* vp = viewport();

	if (!vp)
		return;

	viewport()->getController()->applyPluginChanges(true);

	QImage img = vp->getImage();
	img.invertPixels();

	if (img.isNull())
		vp->getController()->setInfo(tr("Sorry, I cannot Invert the Image..."));
	else
		vp->setEditedImage(img);

}

void DkNoMacs::convert2gray() {

	DkViewPort* vp = viewport();

	if (!vp)
		return;

	viewport()->getController()->applyPluginChanges(true);

	QImage img = vp->getImage();

	QVector<QRgb> table(256);
	for(int i=0;i<256;++i)
		table[i]=qRgb(i,i,i);

	img = img.convertToFormat(QImage::Format_Indexed8,table);

	if (img.isNull())
		vp->getController()->setInfo(tr("Sorry, I cannot convert the Image..."));
	else
		vp->setEditedImage(img);
}

void DkNoMacs::normalizeImage() {

	DkViewPort* vp = viewport();

	if (!vp)
		return;

	viewport()->getController()->applyPluginChanges(true);

	QImage img = vp->getImage();
	
	bool normalized = DkImage::normImage(img);

	if (!normalized || img.isNull())
		vp->getController()->setInfo(tr("The Image is Already Normalized..."));
	else
		vp->setEditedImage(img);
}

void DkNoMacs::autoAdjustImage() {

	DkViewPort* vp = viewport();

	if (!vp)
		return;

	viewport()->getController()->applyPluginChanges(true);

	QImage img = vp->getImage();

	bool normalized = DkImage::autoAdjustImage(img);

	if (!normalized || img.isNull())
		vp->getController()->setInfo(tr("Sorry, I cannot Auto Adjust"));
	else
		vp->setEditedImage(img);
}

void DkNoMacs::unsharpMask() {
#ifdef WITH_OPENCV
	viewport()->getController()->applyPluginChanges(true);

	DkUnsharpDialog* unsharpDialog = new DkUnsharpDialog(this);
	unsharpDialog->setImage(viewport()->getImage());
	int answer = unsharpDialog->exec();
	if (answer == QDialog::Accepted) {
		QImage editedImage = unsharpDialog->getImage();
		viewport()->setEditedImage(editedImage);
	}

	unsharpDialog->deleteLater();
#endif
}

void DkNoMacs::tinyPlanet() {

#ifdef WITH_OPENCV
	
	viewport()->getController()->applyPluginChanges(true);

	DkTinyPlanetDialog* tinyPlanetDialog = new DkTinyPlanetDialog(this);
	tinyPlanetDialog->setImage(viewport()->getImage());
	
	int answer = tinyPlanetDialog->exec();

	if (answer == QDialog::Accepted) {
		QImage editedImage = tinyPlanetDialog->getImage();
		viewport()->setEditedImage(editedImage);
	}

	tinyPlanetDialog->deleteLater();
#endif
}

void DkNoMacs::readSettings() {
	
	qDebug() << "reading settings...";
	QSettings& settings = Settings::instance().getSettings();

#ifdef Q_WS_WIN
	// fixes #392 - starting maximized on 2nd screen - tested on win8 only
	QRect r = settings.value("geometryNomacs", QRect()).toRect();

	if (r.width() && r.height())	// do not set the geometry if nomacs is loaded the first time
		setGeometry(r);
#endif

	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
}

void DkNoMacs::restart() {
	
	if (!viewport()) 
		return;

	QString exe = QApplication::applicationFilePath();
	QStringList args;

	if (getTabWidget()->getCurrentImage())
		args.append(getTabWidget()->getCurrentImage()->filePath());

	bool started = mProcess.startDetached(exe, args);

	// close me if the new instance started
	if (started)
		close();
}

void DkNoMacs::toggleFullScreen() {

	if (isFullScreen())
		exitFullScreen();
	else
		enterFullScreen();
}

void DkNoMacs::enterFullScreen() {
	
	DkSettings::app.currentAppMode += qFloor(DkSettings::mode_end*0.5f);
	if (DkSettings::app.currentAppMode < 0) {
		qDebug() << "illegal state: " << DkSettings::app.currentAppMode;
		DkSettings::app.currentAppMode = DkSettings::mode_default;
	}
	
	menuBar()->hide();
	mToolbar->hide();
	mMovieToolbar->hide();
	mStatusbar->hide();
	getTabWidget()->showTabs(false);

	showExplorer(DkDockWidget::testDisplaySettings(DkSettings::app.showExplorer), false);
	showMetaDataDock(DkDockWidget::testDisplaySettings(DkSettings::app.showMetaDataDock), false);

	DkSettings::app.maximizedMode = isMaximized();
	setWindowState(Qt::WindowFullScreen);
	
	if (viewport())
		viewport()->setFullScreen(true);

	update();
}

void DkNoMacs::exitFullScreen() {

	if (isFullScreen()) {
		DkSettings::app.currentAppMode -= qFloor(DkSettings::mode_end*0.5f);
		if (DkSettings::app.currentAppMode < 0) {
			qDebug() << "illegal state: " << DkSettings::app.currentAppMode;
			DkSettings::app.currentAppMode = DkSettings::mode_default;
		}

		if (DkSettings::app.showMenuBar) mMenu->show();
		if (DkSettings::app.showToolBar) mToolbar->show();
		if (DkSettings::app.showStatusBar) mStatusbar->show();
		if (DkSettings::app.showMovieToolBar) mMovieToolbar->show();
		showExplorer(DkDockWidget::testDisplaySettings(DkSettings::app.showExplorer), false);
		showMetaDataDock(DkDockWidget::testDisplaySettings(DkSettings::app.showMetaDataDock), false);

		if(DkSettings::app.maximizedMode) 
			setWindowState(Qt::WindowMaximized);
		else 
			setWindowState(Qt::WindowNoState);
		
		if (getTabWidget())
			getTabWidget()->showTabs(true);

		update();	// if no resize is triggered, the mViewport won't change its color
	}

	if (viewport())
		viewport()->setFullScreen(false);
}

void DkNoMacs::setFrameless(bool) {

	if (!viewport()) 
		return;

	QString exe = QApplication::applicationFilePath();
	QStringList args;

	if (getTabWidget()->getCurrentImage())
		args.append(getTabWidget()->getCurrentImage()->filePath());
	
	if (objectName() != "DkNoMacsFrameless") {
		DkSettings::app.appMode = DkSettings::mode_frameless;
        //args.append("-graphicssystem");
        //args.append("native");
    } else {
		DkSettings::app.appMode = DkSettings::mode_default;
    }
	
	DkSettings::save();
	
	bool started = mProcess.startDetached(exe, args);

	// close me if the new instance started
	if (started)
		close();

	qDebug() << "frameless arguments: " << args;
}

void DkNoMacs::startPong() const {

	QString exe = QApplication::applicationFilePath();
	QStringList args;

	args.append("-pong");

	bool started = mProcess.startDetached(exe, args);
	qDebug() << "pong started: " << started;
}

void DkNoMacs::fitFrame() {

	QRectF viewRect = viewport()->getImageViewRect();
	QRectF vpRect = viewport()->geometry();
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

	// reset mViewport if we did not clip -> compensates round-off errors
	if (screenRect.contains(nmRect.toRect()))
		viewport()->resetView();

}

void DkNoMacs::setRecursiveScan(bool recursive) {

	DkSettings::global.scanSubFolders = recursive;

	QSharedPointer<DkImageLoader> loader = getTabWidget()->getCurrentImageLoader();
	
	if (!loader)
		return;

	if (recursive)
		viewport()->getController()->setInfo(tr("Recursive Folder Scan is Now Enabled"));
	else
		viewport()->getController()->setInfo(tr("Recursive Folder Scan is Now Disabled"));

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

	
#ifdef WIN32
	
	qDebug() << "locking: " << lock;

	if (lock) {
		//setAttribute(Qt::WA_TransparentForMouseEvents);
		HWND hwnd = (HWND) winId(); // get handle of the widget
		LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
		SetWindowLong(hwnd, GWL_EXSTYLE, styles | WS_EX_TRANSPARENT); 
		SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		viewport()->getController()->setInfo(tr("Window Locked\nTo unlock: gain focus (ALT+Tab),\nthen press CTRL+SHIFT+ALT+B"), 5000);
	}
	else if (lock && windowOpacity() == 1.0f) {
		viewport()->getController()->setInfo(tr("You should first reduce opacity\n before working through the window."));
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

void DkNoMacs::newClientConnected(bool connected, bool) {
	
	mOverlaid = false;
	// add methods if clients are connected

	DkActionManager& am = DkActionManager::instance();
	am.action(DkActionManager::menu_sync)->setEnabled(connected);
	am.action(DkActionManager::menu_sync_pos)->setEnabled(connected);
	am.action(DkActionManager::menu_sync_arrange)->setEnabled(connected);

}

void DkNoMacs::tcpSetWindowRect(QRect newRect, bool opacity, bool overlaid) {

	this->mOverlaid = overlaid;

	DkUtils::printDebug(DK_MODULE, "arranging...\n");

	// we are currently overlaid...
	if (!overlaid) {

		setGeometry(mOldGeometry);
		if (opacity)
			animateOpacityUp();
		mOldGeometry = geometry();
	}
	else {

#ifdef WIN32
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
};

void DkNoMacs::tcpSendWindowRect() {

	mOverlaid = !mOverlaid;

	qDebug() << "overlaying";
	// change my geometry
	tcpSetWindowRect(this->frameGeometry(), !mOverlaid, mOverlaid);

	emit sendPositionSignal(frameGeometry(), mOverlaid);

};

void DkNoMacs::tcpSendArrange() {
	
	mOverlaid = !mOverlaid;
	emit sendArrangeSignal(mOverlaid);
}

void DkNoMacs::showExplorer(bool show, bool saveSettings) {

	if (!mExplorer) {

		// get last location
		mExplorer = new DkExplorer(tr("File Explorer"));
		mExplorer->registerAction(DkActionManager::instance().action(DkActionManager::menu_panel_explorer));
		mExplorer->setDisplaySettings(&DkSettings::app.showExplorer);
		addDockWidget(mExplorer->getDockLocationSettings(Qt::LeftDockWidgetArea), mExplorer);

		connect(mExplorer, SIGNAL(openFile(const QString&)), getTabWidget(), SLOT(loadFile(const QString&)));
		connect(mExplorer, SIGNAL(openDir(const QString&)), getTabWidget()->getThumbScrollWidget(), SLOT(setDir(const QString&)));
		connect(getTabWidget(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mExplorer, SLOT(setCurrentImage(QSharedPointer<DkImageContainerT>)));
	}

	mExplorer->setVisible(show, saveSettings);

	if (getTabWidget()->getCurrentImage() && QFileInfo(getTabWidget()->getCurrentFilePath()).exists()) {
		mExplorer->setCurrentPath(getTabWidget()->getCurrentFilePath());
	}
	else {
		QStringList folders = DkSettings::global.recentFiles;

		if (folders.size() > 0)
			mExplorer->setCurrentPath(folders[0]);
	}

}

void DkNoMacs::showMetaDataDock(bool show, bool saveSettings) {

	if (!mMetaDataDock) {

		mMetaDataDock = new DkMetaDataDock(tr("Meta Data Info"), this);
		mMetaDataDock->registerAction(DkActionManager::instance().action(DkActionManager::menu_panel_metadata_dock));
		mMetaDataDock->setDisplaySettings(&DkSettings::app.showMetaDataDock);
		addDockWidget(mMetaDataDock->getDockLocationSettings(Qt::RightDockWidgetArea), mMetaDataDock);

		connect(getTabWidget(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), mMetaDataDock, SLOT(setImage(QSharedPointer<DkImageContainerT>)));
	}

	mMetaDataDock->setVisible(show, saveSettings);

	if (getTabWidget()->getCurrentImage())
		mMetaDataDock->setImage(getTabWidget()->getCurrentImage());
}

void DkNoMacs::showThumbsDock(bool show) {

	
	// nothing todo here
	if (mThumbsDock && mThumbsDock->isVisible() && show)
		return;
	
	int winPos = viewport()->getController()->getFilePreview()->getWindowPosition();

	if (winPos != DkFilePreview::cm_pos_dock_hor && winPos != DkFilePreview::cm_pos_dock_ver) {
		if (mThumbsDock) {

			//DkSettings::display.thumbDockSize = qMin(thumbsDock->width(), thumbsDock->height());
			QSettings& settings = Settings::instance().getSettings();
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
		mThumbsDock->setDisplaySettings(&DkSettings::app.showFilePreview);
		mThumbsDock->setWidget(viewport()->getController()->getFilePreview());
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

	viewport()->getController()->getFilePreview()->setWindowPosition(thumbsOrientation);

}

void DkNoMacs::openDir() {

	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),
		getTabWidget()->getCurrentDir());

	if (dirName.isEmpty())
		return;

	qDebug() << "loading directory: " << dirName;
	
	getTabWidget()->loadFile(dirName);
}

void DkNoMacs::openFile() {

	if (!viewport())
		return;

	QStringList openFilters = DkSettings::app.openFilters;
	openFilters.pop_front();
	openFilters.prepend(tr("All Files (*.*)"));

	// load system default open dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"),
		getTabWidget()->getCurrentDir(), 
		openFilters.join(";;"));

	if (fileName.isEmpty())
		return;

	qDebug() << "os filename: " << fileName;
	getTabWidget()->loadFile(fileName);
}

void DkNoMacs::openQuickLaunch() {

	// create new model
	if (!mQuickAccess) {
		mQuickAccess = new DkQuickAccess(this);
		
		// add all actions
		mQuickAccess->addActions(DkActionManager::instance().allActions());
		
		connect(mToolbar->getCompleter(), SIGNAL(activated(const QModelIndex&)), mQuickAccess, SLOT(fireAction(const QModelIndex&)));
		connect(mQuickAccess, SIGNAL(loadFileSignal(const QString&)), getTabWidget(), SLOT(loadFile(const QString&)));
		//connect(toolbar, SIGNAL(quickAccessFinishedSignal(const QModelIndex&)), quickAccess, SLOT(fireAction(const QModelIndex&)));
	}
	
	mQuickAccess->addDirs(DkSettings::global.recentFolders);
	mQuickAccess->addFiles(DkSettings::global.recentFiles);

	mToolbar->setQuickAccessModel(mQuickAccess->getModel());
}

void DkNoMacs::loadFile(const QString& filePath) {

	if (!viewport())
		return;


	if (QFileInfo(filePath).isDir())
		getTabWidget()->loadDirToTab(filePath);
	else
		getTabWidget()->loadFileToTab(filePath);

}

void DkNoMacs::renameFile() {

	QFileInfo file = getTabWidget()->getCurrentFilePath();

	if (!file.absoluteDir().exists()) {
		viewport()->getController()->setInfo(tr("Sorry, the directory: %1  does not exist\n").arg(file.absolutePath()));
		return;
	}
	if (file.exists() && !file.isWritable()) {
		viewport()->getController()->setInfo(tr("Sorry, I can't write to the fileInfo: %1").arg(file.fileName()));
		return;
	}

	bool ok;
	QString filename = QInputDialog::getText(this, file.baseName(), tr("Rename:"), QLineEdit::Normal, file.baseName(), &ok);

	if (ok && !filename.isEmpty() && filename != file.baseName()) {
		
		if (!file.suffix().isEmpty())
			filename.append("." + file.suffix());
		
		qDebug() << "renaming: " << file.fileName() << " -> " << filename;
		QFileInfo renamedFile = QFileInfo(file.absoluteDir(), filename);

		// overwrite file?
		if (renamedFile.exists()) {

			QMessageBox infoDialog(this);
			infoDialog.setWindowTitle(tr("Question"));
			infoDialog.setText(tr("The fileInfo: %1  already exists.\n Do you want to replace it?").arg(filename));
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
					viewport()->getController()->setInfo(tr("Sorry, I can't delete: %1").arg(file.fileName()));
					return;
				}
			}
			else
				return;		// cancel renaming
		}

		viewport()->unloadImage();

		QFile newFile(file.absoluteFilePath());
		bool renamed = newFile.rename(renamedFile.absoluteFilePath());
		
		// tell user that deleting went wrong, and stop the renaming
		if (!renamed)
			viewport()->getController()->setInfo(tr("Sorry, I can't rename: %1").arg(file.fileName()));
		else
			getTabWidget()->loadFile(renamedFile.absoluteFilePath());
		
	}

}

void DkNoMacs::find(bool filterAction) {

	if (!viewport() || !getTabWidget()->getCurrentImageLoader())
		return;

	if (filterAction) {

		int db = (QObject::sender() == DkActionManager::instance().action(DkActionManager::menu_tools_filter)) ? DkSearchDialog::filter_button : DkSearchDialog::find_button;
		
		qDebug() << "default button: " << db;
		DkSearchDialog* searchDialog = new DkSearchDialog(this);
		searchDialog->setDefaultButton(db);

		searchDialog->setFiles(getTabWidget()->getCurrentImageLoader()->getFileNames());
		searchDialog->setPath(getTabWidget()->getCurrentImageLoader()->getDirPath());

		connect(searchDialog, SIGNAL(filterSignal(const QStringList&)), getTabWidget()->getCurrentImageLoader().data(), SLOT(setFolderFilters(const QStringList&)));
		connect(searchDialog, SIGNAL(loadFileSignal(const QString&)), getTabWidget(), SLOT(loadFile(const QString&)));
		int answer = searchDialog->exec();

		DkActionManager::instance().action(DkActionManager::menu_tools_filter)->setChecked(answer == DkSearchDialog::filter_button);		
	}
	else {
		// remove the filter 
		getTabWidget()->getCurrentImageLoader()->setFolderFilters(QStringList());
	}

}

void DkNoMacs::changeSorting(bool change) {

	// TODO: move to image loader?!

	if (change) {
	
		QString senderName = QObject::sender()->objectName();

		if (senderName == "menu_sort_filename")
			DkSettings::global.sortMode = DkSettings::sort_filename;
		else if (senderName == "menu_sort_date_created")
			DkSettings::global.sortMode = DkSettings::sort_date_created;
		else if (senderName == "menu_sort_date_modified")
			DkSettings::global.sortMode = DkSettings::sort_date_modified;
		else if (senderName == "menu_sort_random")
			DkSettings::global.sortMode = DkSettings::sort_random;
		else if (senderName == "menu_sort_ascending")
			DkSettings::global.sortDir = DkSettings::sort_ascending;
		else if (senderName == "menu_sort_descending")
			DkSettings::global.sortDir = DkSettings::sort_descending;

		if (getTabWidget()->getCurrentImageLoader()) 
			getTabWidget()->getCurrentImageLoader()->sort();
	}

	QVector<QAction*> sortActions = DkActionManager::instance().sortActions();
	for (int idx = 0; idx < sortActions.size(); idx++) {

		if (idx < DkActionManager::menu_sort_ascending)
			sortActions[idx]->setChecked(idx == DkSettings::global.sortMode);
		else if (idx >= DkActionManager::menu_sort_ascending)
			sortActions[idx]->setChecked(idx-DkActionManager::menu_sort_ascending == DkSettings::global.sortDir);
	}
}

void DkNoMacs::goTo() {

	if (!viewport() || !getTabWidget()->getCurrentImageLoader())
		return;

	QSharedPointer<DkImageLoader> loader = getTabWidget()->getCurrentImageLoader();
	
	bool ok = false;
	int fileIdx = QInputDialog::getInt(this, tr("Go To Image"), tr("Image Index:"), 0, 0, loader->numFiles()-1, 1, &ok);

	if (ok)
		loader->loadFileAt(fileIdx);

}

void DkNoMacs::trainFormat() {

	if (!viewport())
		return;

	if (!mTrainDialog)
		mTrainDialog = new DkTrainDialog(this);

	mTrainDialog->setCurrentFile(getTabWidget()->getCurrentFilePath());
	bool okPressed = mTrainDialog->exec() != 0;

	if (okPressed && getTabWidget()->getCurrentImageLoader()) {
		getTabWidget()->getCurrentImageLoader()->load(mTrainDialog->getAcceptedFile());
		restart();	// quick & dirty, but currently he messes up the filteredFileList if the same folder was already loaded
	}


}

void DkNoMacs::extractImagesFromArchive() {
#ifdef WITH_QUAZIP
	if (!viewport())
		return;

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


void DkNoMacs::saveFile() {

	saveFileAs(true);
}

void DkNoMacs::saveFileAs(bool silent) {
	
	qDebug() << "saving...";

	if (getTabWidget()->getCurrentImageLoader())
		getTabWidget()->getCurrentImageLoader()->saveUserFileAs(getTabWidget()->getViewPort()->getImage(), silent);
}

void DkNoMacs::saveFileWeb() {

	if (getTabWidget()->getCurrentImageLoader())
		getTabWidget()->getCurrentImageLoader()->saveFileWeb(getTabWidget()->getViewPort()->getImage());
}

void DkNoMacs::resizeImage() {

	if (!viewport() || viewport()->getImage().isNull())
		return;

	viewport()->getController()->applyPluginChanges(true);

	if (!mResizeDialog)
		mResizeDialog = new DkResizeDialog(this);

	QSharedPointer<DkImageContainerT> imgC = getTabWidget()->getCurrentImage();
	QSharedPointer<DkMetaDataT> metaData;

	if (imgC) {
		metaData = imgC->getMetaData();
		QVector2D res = metaData->getResolution();
		mResizeDialog->setExifDpi((float)res.x());
	}

	qDebug() << "resize image: " << viewport()->getImage().size();


	mResizeDialog->setImage(viewport()->getImage());

	if (!mResizeDialog->exec())
		return;

	if (mResizeDialog->resample()) {

		QImage rImg = mResizeDialog->getResizedImage();

		if (!rImg.isNull()) {

			// this reloads the image -> that's not what we want!
			if (metaData)
				metaData->setResolution(QVector2D(mResizeDialog->getExifDpi(), mResizeDialog->getExifDpi()));

			imgC->setImage(rImg);
			viewport()->setEditedImage(imgC);
		}
	}
	else if (metaData) {
		// ok, user just wants to change the resolution
		metaData->setResolution(QVector2D(mResizeDialog->getExifDpi(), mResizeDialog->getExifDpi()));
		qDebug() << "setting resolution to: " << mResizeDialog->getExifDpi();
		//mViewport()->setEditedImage(mViewport()->getImage());
	}
}

void DkNoMacs::deleteFile() {

	if (!viewport() || viewport()->getImage().isNull() || !getTabWidget()->getCurrentImageLoader())
		return;
	
	viewport()->getController()->applyPluginChanges(true);

	QFileInfo fileInfo = getTabWidget()->getCurrentFilePath();

	if (QMessageBox::question(this, tr("Info"), tr("Do you want to permanently delete %1").arg(fileInfo.fileName())) == QMessageBox::Yes) {
		viewport()->stopMovie();	// movies keep file handles so stop it before we can delete files
		
		if (!getTabWidget()->getCurrentImageLoader()->deleteFile())
			viewport()->loadMovie();	// load the movie again, if we could not delete it
	}
}

void DkNoMacs::openAppManager() {

	DkAppManagerDialog* appManagerDialog = new DkAppManagerDialog(DkActionManager::instance().appManager(), this, windowFlags());
	connect(appManagerDialog, SIGNAL(openWithSignal(QAction*)), this, SLOT(openFileWith(QAction*)));
	appManagerDialog->exec();

	appManagerDialog->deleteLater();

	DkActionManager::instance().openWithMenu();	// update
}

void DkNoMacs::exportTiff() {

#ifdef WITH_LIBTIFF
	if (!mExportTiffDialog)
		mExportTiffDialog = new DkExportTiffDialog(this);

	mExportTiffDialog->setFile(getTabWidget()->getCurrentFilePath());
	mExportTiffDialog->exec();
#endif
}

void DkNoMacs::computeMosaic() {
#ifdef WITH_OPENCV

	DkMosaicDialog* mosaicDialog = new DkMosaicDialog(this, Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

	mosaicDialog->setFile(getTabWidget()->getCurrentFilePath());

	int response = mosaicDialog->exec();

	if (response == QDialog::Accepted && !mosaicDialog->getImage().isNull()) {
		QImage editedImage = mosaicDialog->getImage();
		viewport()->setEditedImage(editedImage);
		saveFileAs();
	}

	mosaicDialog->deleteLater();
#endif
}

void DkNoMacs::openImgManipulationDialog() {

	if (!viewport() || viewport()->getImage().isNull())
		return;

	if (!mImgManipulationDialog)
		mImgManipulationDialog = new DkImageManipulationDialog(this);
	else 
		mImgManipulationDialog->resetValues();

	QImage tmpImg = viewport()->getImage();
	mImgManipulationDialog->setImage(&tmpImg);

	bool ok = mImgManipulationDialog->exec() != 0;

	if (ok) {

#ifdef WITH_OPENCV

		QImage mImg = DkImage::mat2QImage(DkImageManipulationWidget::manipulateImage(DkImage::qImage2Mat(viewport()->getImage())));

		if (!mImg.isNull())
			viewport()->setEditedImage(mImg);

#endif
	}
}


void DkNoMacs::setWallpaper() {

	// based on code from: http://qtwiki.org/Set_windows_background_using_QT

	QImage img = viewport()->getImage();

	QImage dImg = img;

	QSharedPointer<DkImageLoader> loader = QSharedPointer<DkImageLoader>(new DkImageLoader());
	QFileInfo tmpPath = loader->saveTempFile(dImg, "wallpaper", ".jpg", true, false);
	
	// is there a more elegant way to see if saveTempFile returned an empty path
	if (tmpPath.absoluteFilePath() == QFileInfo().absoluteFilePath()) {
		QMessageBox::critical(this, tr("Error"), tr("Sorry, I could not create a wallpaper..."));
		return;
	}

#ifdef WIN32

	//Read current windows background image path
	QSettings appSettings( "HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
	appSettings.setValue("Wallpaper", tmpPath.absoluteFilePath());

	QByteArray ba = tmpPath.absoluteFilePath().toLatin1();
	SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)ba.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#endif
	// TODO: add functionality for unix based systems
}

void DkNoMacs::printDialog() {

	QPrinter printer;

	QVector2D res(150,150);
	QSharedPointer<DkImageContainerT> imgC = getTabWidget()->getCurrentImage();
	
	if (imgC)
		res = imgC->getMetaData()->getResolution();

	//QPrintPreviewDialog* previewDialog = new QPrintPreviewDialog();
	QImage img = viewport()->getImage();
	if (!mPrintPreviewDialog)
		mPrintPreviewDialog = new DkPrintPreviewDialog(img, (float)res.x(), 0, this);
	else
		mPrintPreviewDialog->setImage(img, (float)res.x());

	mPrintPreviewDialog->show();
	mPrintPreviewDialog->updateZoomFactor(); // otherwise the initial zoom factor is wrong

}

void DkNoMacs::computeThumbsBatch() {

	if (!viewport())
		return;

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

	QString url = QString("http://www.nomacs.org/documentation/");

	QDesktopServices::openUrl(QUrl(url));
}

void DkNoMacs::bugReport() {

	QString url = QString("http://www.nomacs.org/redmine/projects/nomacs/")
		% QString("issues/new?issue[tracker_id]=1&issue[custom_field_values][1]=")
		% QApplication::applicationVersion();

	url += "&issue[custom_field_values][4]=";
#if defined WIN32 &&	_MSC_VER == 1600
	url += "Windows XP";
#elif defined WIN32 && _WIN64
	url += "Windows Vista/7/8 64bit";
#elif defined WIN32 && _WIN32
	url += "Windows Vista/7/8 32bit";
#elif defined Q_WS_X11 && __x86_64__	// >DIR: check if qt5 still supports these flags [19.2.2014 markus]
	url += "Linux 64bit";
#elif defined Q_WS_X11 && __i386__
	url += "Linux 32bit";
#elif defined Q_WS_MAC
	url += "Mac OS";
#else
	url += "";
#endif

	
	QDesktopServices::openUrl(QUrl(url));
}

void DkNoMacs::featureRequest() {
	
	QString url = QString("http://www.nomacs.org/redmine/projects/nomacs/")
		% QString("issues/new?issue[tracker_id]=2&issue[custom_field_values][1]=")
		% QApplication::applicationVersion();

	url += "&issue[custom_field_values][4]=";
#if defined WIN32 &&	_MSC_VER == 1600
	url += "Windows Vista/XP";
#elif defined WIN32 && _WIN64
	url += "Windows 7/8/10 64bit";
#elif defined WIN32 && _WIN32
	url += "Windows 7/8/10 32bit";
#elif defined Q_WS_X11 && __x86_64__
	url += "Linux 64bit";
#elif defined Q_WS_X11 && __i386__
	url += "Linux 32bit";
#elif defined Q_WS_MAC
	url += "Mac OS";
#else
	url += "";
#endif

	QDesktopServices::openUrl(QUrl(url));
}

void DkNoMacs::cleanSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.clear();

	readSettings();
	resize(400, 225);
	move(100, 100);
}

void DkNoMacs::newInstance(const QString& filePath) {

	if (!viewport()) 
		return;

	QString exe = QApplication::applicationFilePath();
	QStringList args;

	QAction* a = static_cast<QAction*>(sender());

	if (a && a == DkActionManager::instance().action(DkActionManager::menu_file_private_instance))
		args.append("-p");

	if (filePath.isEmpty())
		args.append(getTabWidget()->getCurrentFilePath());
	else
		args.append(filePath);

	if (objectName() == "DkNoMacsFrameless")
		args.append("1");	
	
	QProcess::startDetached(exe, args);
}

void DkNoMacs::loadRecursion() {

	if (!getTabWidget()->getCurrentImage())
		return;

	viewport()->toggleDissolve();


	//QImage img = getTabWidget()->getCurrentImage()->image();

	//while (DkImage::addToImage(img, 1)) {
	//	mViewport()->setEditedImage(img);
	//	QApplication::sendPostedEvents();
	//}

	//QImage img = QPixmap::grabWindow(this->winId()).toImage();
	//mViewport()->setImage(img);
}

// Added by fabian for transfer function:

void DkNoMacs::setContrast(bool contrast) {

	qDebug() << "contrast: " << contrast;

	if (!viewport()) 
		return;

	QString exe = QApplication::applicationFilePath();
	QStringList args;
	args.append(getTabWidget()->getCurrentFilePath());
	
	if (contrast)
		DkSettings::app.appMode = DkSettings::mode_contrast;
	else
		DkSettings::app.appMode = DkSettings::mode_default;

	bool started = mProcess.startDetached(exe, args);

	// close me if the new instance started
	if (started)
		close();

	qDebug() << "contrast arguments: " << args;
}

void DkNoMacs::showRecentFiles(bool show) {

	if (DkSettings::app.appMode != DkSettings::mode_frameless && !DkSettings::global.recentFiles.empty())
		getTabWidget()->showRecentFiles(show);

}

void DkNoMacs::onWindowLoaded() {

	QSettings& settings = Settings::instance().getSettings();
	bool firstTime = settings.value("AppSettings/firstTime.nomacs.3", true).toBool();

	if (DkDockWidget::testDisplaySettings(DkSettings::app.showExplorer))
		showExplorer(true);
	if (DkDockWidget::testDisplaySettings(DkSettings::app.showMetaDataDock))
		showMetaDataDock(true);

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
		else if (keyEvent->key() == Qt::Key_Escape && DkSettings::app.closeOnEsc)
			close();
	}
	if (event->type() == QEvent::Gesture) {
		return gestureEvent(static_cast<QGestureEvent*>(event));
	}

	return false;
}

void DkNoMacs::showMenuBar(bool show) {

	DkSettings::app.showMenuBar = show;
	int tts = (DkSettings::app.showMenuBar) ? -1 : 5000;
	DkActionManager::instance().action(DkActionManager::menu_panel_menu)->setChecked(DkSettings::app.showMenuBar);
	mMenu->setTimeToShow(tts);
	mMenu->showMenu();

	if (!show)
		mMenu->hide();
}

void DkNoMacs::showToolbar(QToolBar* toolbar, bool show) {

	if (!toolbar)
		return;

	showToolbarsTemporarily(!show);

	if (show) {
		addToolBar(toolBarArea(this->mToolbar), toolbar);
	}
	else
		removeToolBar(toolbar);

	toolbar->setVisible(show);
}

void DkNoMacs::showToolbarsTemporarily(bool show) {

	if (show) {
		for (int idx = 0; idx < mHiddenToolbars.size(); idx++)
			mHiddenToolbars.at(idx)->show();
	}
	else {

		mHiddenToolbars.clear();
		QList<QToolBar *> toolbars = findChildren<QToolBar *>();

		for (int idx = 0; idx < toolbars.size(); idx++) {
			
			if (toolbars.at(idx)->isVisible()) {
				toolbars.at(idx)->hide();
				mHiddenToolbars.append(toolbars.at(idx));
			}
		}
	}
}

void DkNoMacs::showToolbar(bool show) {

	DkSettings::app.showToolBar = show;
	DkActionManager::instance().action(DkActionManager::menu_panel_toolbar)->setChecked(DkSettings::app.showToolBar);
	
	if (DkSettings::app.showToolBar)
		mToolbar->show();
	else
		mToolbar->hide();
}

void DkNoMacs::showStatusBar(bool show, bool permanent) {

	if (mStatusbar->isVisible() == show)
		return;

	if (permanent)
		DkSettings::app.showStatusBar = show;
	DkActionManager::instance().action(DkActionManager::menu_panel_statusbar)->setChecked(DkSettings::app.showStatusBar);

	mStatusbar->setVisible(show);

	viewport()->setVisibleStatusbar(show);
}

void DkNoMacs::showStatusMessage(QString msg, int which) {

	if (which < 0 || which >= mStatusbarLabels.size())
		return;

	mStatusbarLabels[which]->setVisible(!msg.isEmpty());
	mStatusbarLabels[which]->setText(msg);
}

void DkNoMacs::openFileWith(QAction* action) {

	if (!action)
		return;

	QFileInfo app(action->toolTip());

	if (!app.exists())
		viewport()->getController()->setInfo("Sorry, " % app.fileName() % " does not exist");

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
	else if (viewport())
		viewport()->getController()->setInfo("Sorry, I could not start: " % app.absoluteFilePath());
}

void DkNoMacs::showGpsCoordinates() {

	QSharedPointer<DkMetaDataT> metaData = getTabWidget()->getCurrentImage()->getMetaData();

	if (!DkMetaDataHelper::getInstance().hasGPS(metaData)) {
		viewport()->getController()->setInfo("Sorry, I could not find the GPS coordinates...");
		return;
	}

	qDebug() << "gps: " << DkMetaDataHelper::getInstance().getGpsCoordinates(metaData);

	QDesktopServices::openUrl(QUrl(DkMetaDataHelper::getInstance().getGpsCoordinates(metaData)));  
}

void DkNoMacs::setWindowTitle(QSharedPointer<DkImageContainerT> imgC) {

	if (!imgC) {
		setWindowTitle(QString());
		return;
	}

	setWindowTitle(imgC->filePath(), imgC->image().size(), imgC->isEdited(), imgC->getTitleAttribute());
}

void DkNoMacs::setWindowTitle(const QString& filePath, const QSize& size, bool edited, const QString& attr) {

	// TODO: rename!

	QFileInfo fInfo = filePath;
	QString title = QFileInfo(filePath).fileName();
	title = title.remove(".lnk");
	
	if (title.isEmpty()) {
		title = "nomacs - Image Lounge";
		if (DkSettings::app.privateMode) 
			title.append(tr(" [Private Mode]"));
	}

	if (edited)
		title.append("[*]");

	title.append(" ");
	title.append(attr);	// append some attributes

	QString attributes;

	if (!size.isEmpty())
		attributes.sprintf(" - %i x %i", size.width(), size.height());
	if (size.isEmpty() && viewport())
		attributes.sprintf(" - %i x %i", viewport()->getImage().width(), viewport()->getImage().height());
	if (DkSettings::app.privateMode) 
		attributes.append(tr(" [Private Mode]"));

	QMainWindow::setWindowTitle(title.append(attributes));
	setWindowFilePath(filePath);
	emit sendTitleSignal(windowTitle());
	setWindowModified(edited);

	if ((!viewport()->getController()->getFileInfoLabel()->isVisible() || 
		!DkSettings::slideShow.display.testBit(DkSettings::display_creation_date)) && getTabWidget()->getCurrentImage()) {
		
		// create statusbar info
		QSharedPointer<DkMetaDataT> metaData = getTabWidget()->getCurrentImage()->getMetaData();
		QString dateString = metaData->getExifValue("DateTimeOriginal");
		dateString = DkUtils::convertDateString(dateString, fInfo);
		showStatusMessage(dateString, status_time_info);
	}
	else 
		showStatusMessage("", status_time_info);	// hide label

	if (fInfo.exists())
		showStatusMessage(DkUtils::readableByte((float)fInfo.size()), status_filesize_info);
	else 
		showStatusMessage("", status_filesize_info);

}

void DkNoMacs::openSettings() {

	if (!mSettingsDialog) {
		mSettingsDialog = new DkSettingsDialog(this);
		connect(mSettingsDialog, SIGNAL(setToDefaultSignal()), this, SLOT(cleanSettings()));
		connect(mSettingsDialog, SIGNAL(settingsChanged()), viewport(), SLOT(settingsChanged()));
		connect(mSettingsDialog, SIGNAL(languageChanged()), this, SLOT(restartWithTranslationUpdate()));
		connect(mSettingsDialog, SIGNAL(settingsChangedRestart()), this, SLOT(restart()));
		connect(mSettingsDialog, SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));
	}

	mSettingsDialog->exec();

	qDebug() << "hier könnte ihre werbung stehen...";
}

void DkNoMacs::settingsChanged() {
	
	if (!isFullScreen()) {
		showMenuBar(DkSettings::app.showMenuBar);
		showToolbar(DkSettings::app.showToolBar);
		showStatusBar(DkSettings::app.showStatusBar);
	}
}

void DkNoMacs::checkForUpdate(bool silent) {

	// updates are supported on windows only
#ifndef Q_WS_X11

	// do we really need to check for update?
	if (!silent || (!DkSettings::sync.updateDialogShown && QDate::currentDate() > DkSettings::sync.lastUpdateCheck && DkSettings::sync.checkForUpdates)) {

		DkTimer dt;

		if (!mUpdater) {
			mUpdater = new DkUpdater(this);
			connect(mUpdater, SIGNAL(displayUpdateDialog(QString, QString)), this, SLOT(showUpdateDialog(QString, QString)));
			connect(mUpdater, SIGNAL(showUpdaterMessage(QString, QString)), this, SLOT(showUpdaterMessage(QString, QString)));
		}
		mUpdater->silent = silent;
		mUpdater->checkForUpdates();

		qDebug() << "checking for updates takes: " << dt.getTotal();
	}
#endif // !#ifndef Q_WS_X11
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

	DkSettings::sync.updateDialogShown = true;

	DkSettings::save();
	
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
	mProgressDialog->setWindowModality(Qt::ApplicationModal);

	mProgressDialog->show();
	//progressDialog->raise();
	//progressDialog->activateWindow();
	mProgressDialog->setWindowModality(Qt::NonModal);
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
		qDebug() << "fileInfo does not exist";
	if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filePath))) {
		QString msg = tr("Unable to install new version<br>") +
			tr("You can download the new version from our web page") +
			"<br><a href=\"http://www.nomacs.org/download/\">www.nomacs.org</a><br>";
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
	mProgressDialogTranslations->setWindowModality(Qt::ApplicationModal);

	mProgressDialogTranslations->show();
	//progressDialog->raise();
	//progressDialog->activateWindow();
	mProgressDialogTranslations->setWindowModality(Qt::NonModal);

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

//void DkNoMacs::errorDialog(const QString& msg) {
//	dialog(msg, this, tr("Error"));
//}

//void DkNoMacs::errorDialog(QString msg, QString title) {
//
//	dialog(msg, this, title);
//}

//int DkNoMacs::dialog(QString msg, QWidget* parent, QString title) {
//
//	if (!parent) {
//		QWidgetList w = QApplication::topLevelWidgets();
//
//		for (int idx = 0; idx < w.size(); idx++) {
//
//			if (w[idx]->objectName().contains(QString("DkNoMacs"))) {
//				parent = w[idx];
//				break;
//			}
//		}
//	}
//
//	QMessageBox errorDialog(parent);
//	errorDialog.setWindowTitle(title);
//	errorDialog.setIcon(QMessageBox::Critical);
//	errorDialog.setText(msg);
//	errorDialog.show();
//
//	return errorDialog.exec();
//
//}

//int DkNoMacs::infoDialog(QString msg, QWidget* parent, QString title) {
//
//	QMessageBox errorDialog(parent);
//	errorDialog.setWindowTitle(title);
//	errorDialog.setIcon(QMessageBox::Question);
//	errorDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//	errorDialog.setText(msg);
//	errorDialog.show();
//
//	return errorDialog.exec();
//}

void DkNoMacs::openPluginManager() {
#ifdef WITH_PLUGINS

	viewport()->getController()->closePlugin(true);

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

	if (mLocalClient) {

		// terminate local client
		mLocalClient->quit();
		mLocalClient->wait();

		delete mLocalClient;
		mLocalClient = 0;
	}

	if (mRcClient) {

		if (DkSettings::sync.syncMode == DkSettings::sync_mode_remote_control)
			mRcClient->sendNewMode(DkSettings::sync_mode_remote_control);	// TODO: if we need this threaded emit a signal here

		emit stopSynchronizeWithSignal();

		mRcClient->quit();
		mRcClient->wait();

		delete mRcClient;
		mRcClient = 0;

	}

}

void DkNoMacsSync::initLanClient() {

	DkTimer dt;
	if (mLanClient) {

		mLanClient->quit();
		mLanClient->wait();

		delete mLanClient;
	}


	// remote control server
	if (mRcClient) {
		mRcClient->quit();
		mRcClient->wait();

		delete mRcClient;
	}

	qDebug() << "client clearing takes: " << dt.getTotal();

	if (!DkSettings::sync.enableNetworkSync) {

		mLanClient = 0;
		mRcClient = 0;

		DkActionManager::instance().lanMenu()->setEnabled(false);
		DkActionManager::instance().action(DkActionManager::menu_sync_remote_control)->setEnabled(false);
		DkActionManager::instance().action(DkActionManager::menu_sync_remote_display)->setEnabled(false);
		return;
	}

	DkTcpMenu* lanMenu = DkActionManager::instance().lanMenu();
	lanMenu->clear();

	// start lan client/server
	mLanClient = new DkLanManagerThread(this);
	mLanClient->setObjectName("lanClient");
#ifdef WITH_UPNP
	if (!upnpControlPoint) {
		upnpControlPoint = QSharedPointer<DkUpnpControlPoint>(new DkUpnpControlPoint());
	}
	mLanClient->upnpControlPoint = upnpControlPoint;
	if (!upnpDeviceHost) {
		upnpDeviceHost = QSharedPointer<DkUpnpDeviceHost>(new DkUpnpDeviceHost());
	}
	mLanClient->upnpDeviceHost = upnpDeviceHost;
#endif // WITH_UPNP
	mLanClient->start();

	lanMenu->setClientManager(mLanClient);
	lanMenu->addTcpAction(DkActionManager::instance().action(DkActionManager::menu_lan_server));
	lanMenu->addTcpAction(DkActionManager::instance().action(DkActionManager::menu_lan_image));	// well this is a bit nasty... we only add it here to have correct enable/disable behavior...
	lanMenu->setEnabled(true);
	lanMenu->enableActions(false, false);

	mRcClient = new DkRCManagerThread(this);
	mRcClient->setObjectName("rcClient");
#ifdef WITH_UPNP
	if (!upnpControlPoint) {
		upnpControlPoint = QSharedPointer<DkUpnpControlPoint>(new DkUpnpControlPoint());
	}
	mRcClient->upnpControlPoint = upnpControlPoint;
	if (!upnpDeviceHost) {
		upnpDeviceHost = QSharedPointer<DkUpnpDeviceHost>(new DkUpnpDeviceHost());
	}
	mRcClient->upnpDeviceHost = upnpDeviceHost;
#endif // WITH_UPNP
	
	mRcClient->start();
	
	connect(this, SIGNAL(startTCPServerSignal(bool)), mLanClient, SLOT(startServer(bool)));
	connect(this, SIGNAL(startRCServerSignal(bool)), mRcClient, SLOT(startServer(bool)), Qt::QueuedConnection);

	if (!DkSettings::sync.syncWhiteList.empty()) {
		qDebug() << "whitelist not empty .... starting server";
#ifdef WITH_UPNP
		upnpDeviceHost->startDevicehost(":/nomacs/descriptions/nomacs-device.xml");
#endif // WITH_UPNP

		// TODO: currently blocking : )
		emit startRCServerSignal(true);
		//rcClient->startServer(true);
	}
	else 
		qDebug() << "whitelist empty!!";



	qDebug() << "start server takes: " << dt.getTotal();
}

void DkNoMacsSync::createActions() {

	DkNoMacs::createActions();

	DkActionManager& am = DkActionManager::instance();
	connect(am.action(DkActionManager::menu_lan_server), SIGNAL(toggled(bool)), this, SLOT(startTCPServer(bool)));	// TODO: something that makes sense...
	
	// TODO: move to viewport
	connect(am.action(DkActionManager::menu_lan_image), SIGNAL(triggered()), viewport(), SLOT(tcpSendImage()));
	connect(am.action(DkActionManager::menu_sync), SIGNAL(triggered()), viewport(), SLOT(tcpForceSynchronize()));

	// sync menu
	connect(am.action(DkActionManager::menu_sync_pos), SIGNAL(triggered()), this, SLOT(tcpSendWindowRect()));
	connect(am.action(DkActionManager::menu_sync_arrange), SIGNAL(triggered()), this, SLOT(tcpSendArrange()));
	connect(am.action(DkActionManager::menu_sync_connect_all), SIGNAL(triggered()), this, SLOT(tcpConnectAll()));
	connect(am.action(DkActionManager::menu_sync_all_actions), SIGNAL(triggered(bool)), this, SLOT(tcpAutoConnect(bool)));
	connect(am.action(DkActionManager::menu_sync_start_upnp), SIGNAL(triggered(bool)), this, SLOT(startUpnpRenderer(bool)));
	connect(am.action(DkActionManager::menu_sync_remote_control), SIGNAL(triggered(bool)), this, SLOT(tcpRemoteControl(bool)));
	connect(am.action(DkActionManager::menu_sync_remote_display), SIGNAL(triggered(bool)), this, SLOT(tcpRemoteDisplay(bool)));
}

void DkNoMacsSync::createMenu() {

	DkNoMacs::createMenu();

	DkActionManager::instance().createSyncMenu(mSyncMenu, mLocalClient, mLanClient);
}

// mouse events
void DkNoMacsSync::mouseMoveEvent(QMouseEvent *event) {

	int dist = QPoint(event->pos()-mMousePos).manhattanLength();

	// create drag sync action
	if (event->buttons() == Qt::LeftButton && dist > QApplication::startDragDistance() &&
		event->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {

			qDebug() << "generating a drag event...";

			QByteArray connectionData;
			QDataStream dataStream(&connectionData, QIODevice::WriteOnly);
			dataStream << mLocalClient->getServerPort();
			qDebug() << "serverport: " << mLocalClient->getServerPort();

			QDrag* drag = new QDrag(this);
			QMimeData* mimeData = new QMimeData;
			mimeData->setData("network/sync-dir", connectionData);

			drag->setMimeData(mimeData);
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

		emit synchronizeWithServerPortSignal(peerId);
		qDebug() << "drop server port: " << peerId;
	}
	else
		QMainWindow::dropEvent(event);

}

void DkNoMacsSync::enableNoImageActions(bool enable /* = true */) {

	DkNoMacs::enableNoImageActions(enable);

	DkActionManager::instance().action(DkActionManager::menu_sync_connect_all)->setEnabled(enable);
}

qint16 DkNoMacsSync::getServerPort() {

	return (mLocalClient) ? mLocalClient->getServerPort() : 0;
}

void DkNoMacsSync::syncWith(qint16 port) {
	emit synchronizeWithServerPortSignal(port);
}

// slots
void DkNoMacsSync::tcpConnectAll() {

	QList<DkPeer*> peers = mLocalClient->getPeerList();

	for (int idx = 0; idx < peers.size(); idx++)
		emit synchronizeWithSignal(peers.at(idx)->peerId);

}

void DkNoMacsSync::tcpChangeSyncMode(int syncMode, bool connectWithWhiteList) {

	if (syncMode == DkSettings::sync.syncMode || !mRcClient)
		return;

	// turn off everything
	if (syncMode == DkSettings::sync_mode_default)
		mRcClient->sendGoodByeToAll();

	DkActionManager::instance().action(DkActionManager::menu_sync_remote_control)->setChecked(false);
	DkActionManager::instance().action(DkActionManager::menu_sync_remote_display)->setChecked(false);

	if (syncMode == DkSettings::sync_mode_default) {
		DkSettings::sync.syncMode = syncMode;
		return;
	}

	// if we do not connect with the white list, the signal came from the rc client
	// so we can easily assume that we are connected
	bool connected = (connectWithWhiteList) ? connectWhiteList(syncMode, DkSettings::sync.syncMode == DkSettings::sync_mode_default) : true;

	if (!connected) {
		DkSettings::sync.syncMode = DkSettings::sync_mode_default;
		viewport()->getController()->setInfo(tr("Sorry, I could not find any clients."));
		return;
	}

	// turn on the new mode
	switch(syncMode) {
		case DkSettings::sync_mode_remote_control: 
			DkActionManager::instance().action(DkActionManager::menu_sync_remote_control)->setChecked(true);	
			break;
		case DkSettings::sync_mode_remote_display: 
			DkActionManager::instance().action(DkActionManager::menu_sync_remote_display)->setChecked(true);
			break;
	//default:
	}

	DkSettings::sync.syncMode = syncMode;
}


void DkNoMacsSync::tcpRemoteControl(bool start) {

	if (!mRcClient)
		return;

	tcpChangeSyncMode((start) ? DkSettings::sync_mode_remote_control : DkSettings::sync_mode_default, true);
}

void DkNoMacsSync::tcpRemoteDisplay(bool start) {

	if (!mRcClient)
		return;

	tcpChangeSyncMode((start) ? DkSettings::sync_mode_remote_display : DkSettings::sync_mode_default, true);

}

void DkNoMacsSync::tcpAutoConnect(bool connect) {

	DkSettings::sync.syncActions = connect;
}

#ifdef WITH_UPNP
void DkNoMacsSync::startUpnpRenderer(bool start) {
	if (!upnpRendererDeviceHost) {
		upnpRendererDeviceHost = QSharedPointer<DkUpnpRendererDeviceHost>(new DkUpnpRendererDeviceHost());
		connect(upnpRendererDeviceHost.data(), SIGNAL(newImage(QImage)), viewport(), SLOT(setImage(QImage)));
	}
	if(start)
		upnpRendererDeviceHost->startDevicehost(":/nomacs/descriptions/nomacs_mediarenderer_description.xml");
	else
		upnpDeviceHost->stopDevicehost();
}
#else
void DkNoMacsSync::startUpnpRenderer(bool) {}	// dummy
#endif // WITH_UPNP

bool DkNoMacsSync::connectWhiteList(int mode, bool connect) {

	if (!mRcClient)
		return false;

	bool couldConnect = false;

	QList<DkPeer*> peers = mRcClient->getPeerList();
	qDebug() << "number of peers in list:" << peers.size();

	// TODO: add gui if idx != 1
	if (connect && !peers.isEmpty()) {
		DkPeer* peer = peers[0];

		emit synchronizeRemoteControl(peer->peerId);
		
		if (mode == DkSettings::sync_mode_remote_control)
			mRcClient->sendNewMode(DkSettings::sync_mode_remote_display);	// TODO: if we need this threaded emit a signal here
		else
			mRcClient->sendNewMode(DkSettings::sync_mode_remote_control);	// TODO: if we need this threaded emit a signal here

		couldConnect = true;
	}
	else if (!connect) {

		if (mode == DkSettings::sync_mode_remote_control)
			mRcClient->sendNewMode(DkSettings::sync_mode_remote_display);	// TODO: if we need this threaded emit a signal here
		else
			mRcClient->sendNewMode(DkSettings::sync_mode_remote_control);	// TODO: if we need this threaded emit a signal here

		emit stopSynchronizeWithSignal();
		couldConnect = true;
	}

	return couldConnect;
}

void DkNoMacsSync::newClientConnected(bool connected, bool local) {

	DkActionManager::instance().lanMenu()->enableActions(connected, local);
	
	DkNoMacs::newClientConnected(connected, local);
}

void DkNoMacsSync::startTCPServer(bool start) {
	
#ifdef WITH_UPNP
	if (!upnpDeviceHost->isStarted())
		upnpDeviceHost->startDevicehost(":/nomacs/descriptions/nomacs-device.xml");
#endif // WITH_UPNP
	emit startTCPServerSignal(start);
}

void DkNoMacsSync::settingsChanged() {
	initLanClient();

	DkNoMacs::settingsChanged();
}

void DkNoMacsSync::clientInitialized() {
	
	//TODO: things that need to be done after the clientManager has finished initialization
#ifdef WITH_UPNP
	QObject* obj = QObject::sender();
	if (obj && (obj->objectName() == "lanClient" || obj->objectName() == "rcClient")) {
		qDebug() << "sender:" << obj->objectName();
		if (!upnpControlPoint->isStarted()) {
			qDebug() << "initializing upnpControlPoint";
			upnpControlPoint->init();
		}
	} 
#endif // WITH_UPNP
	
	emit clientInitializedSignal();
}

DkNoMacsIpl::DkNoMacsIpl(QWidget *parent, Qt::WindowFlags flags) : DkNoMacsSync(parent, flags) {

		// init members
	DkViewPort* vp = new DkViewPort(this);
	vp->setAlignment(Qt::AlignHCenter);

	DkCentralWidget* cw = new DkCentralWidget(vp, this);
	setCentralWidget(cw);

	mLocalClient = new DkLocalManagerThread(this);
	mLocalClient->setObjectName("localClient");
	mLocalClient->start();

	mLanClient = 0;
	mRcClient = 0;


	init();
	setAcceptDrops(true);
	setMouseTracking (true);	//receive mouse event everytime

	DkTimer dt;
		
	// sync signals
	connect(vp, SIGNAL(newClientConnectedSignal(bool, bool)), this, SLOT(newClientConnected(bool, bool)));

	DkSettings::app.appMode = 0;
	initLanClient();
	//emit sendTitleSignal(windowTitle());
	qDebug() << "LAN client created in: " << dt.getTotal();
	// show it...
	show();
	DkSettings::app.appMode = DkSettings::mode_default;

	qDebug() << "mViewport (normal) created in " << dt.getTotal();
}

// FramelessNoMacs --------------------------------------------------------------------
DkNoMacsFrameless::DkNoMacsFrameless(QWidget *parent, Qt::WindowFlags flags)
	: DkNoMacs(parent, flags) {

		setObjectName("DkNoMacsFrameless");
		DkSettings::app.appMode = DkSettings::mode_frameless;
		
		setWindowFlags(Qt::FramelessWindowHint);
		setAttribute(Qt::WA_TranslucentBackground, true);

		// init members
		DkViewPortFrameless* vp = new DkViewPortFrameless(this);
		vp->setAlignment(Qt::AlignHCenter);

		DkCentralWidget* cw = new DkCentralWidget(vp, this);
		setCentralWidget(cw);

		init();
		
		setAcceptDrops(true);
		setMouseTracking (true);	//receive mouse event everytime

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
		updateScreenSize();
		show();
        
        connect(mDesktop, SIGNAL(workAreaResized(int)), this, SLOT(updateScreenSize(int)));

		setObjectName("DkNoMacsFrameless");
		showStatusBar(false);	// fix
}

DkNoMacsFrameless::~DkNoMacsFrameless() {
	release();
}

void DkNoMacsFrameless::release() {
}

void DkNoMacsFrameless::createContextMenu() {

	DkNoMacs::createContextMenu();

	DkActionManager& am = DkActionManager::instance();
	am.contextMenu()->addSeparator();
	am.contextMenu()->addAction(am.action(DkActionManager::menu_file_exit));
}

void DkNoMacsFrameless::enableNoImageActions(bool enable) {

	DkNoMacs::enableNoImageActions(enable);

	// actions that should always be disabled
	DkActionManager::instance().action(DkActionManager::menu_view_fit_frame)->setEnabled(false);

}

void DkNoMacsFrameless::updateScreenSize(int) {

	if (!mDesktop)
		return;

	//TODO: let user choose which screen
	int sc = mDesktop->screenCount();
	QRect screenRects = mDesktop->availableGeometry();

	for (int idx = 0; idx < sc; idx++) {

		qDebug() << "screens: " << mDesktop->availableGeometry(idx);
		QRect curScreen = mDesktop->availableGeometry(idx);
		screenRects.setLeft(qMin(screenRects.left(), curScreen.left()));
		screenRects.setTop(qMin(screenRects.top(), curScreen.top()));
		screenRects.setBottom(qMax(screenRects.bottom(), curScreen.bottom()));
		screenRects.setRight(qMax(screenRects.right(), curScreen.right()));
	}

	qDebug() << "set up geometry: " << screenRects;


	DkViewPortFrameless* vp = static_cast<DkViewPortFrameless*>(viewport());
	vp->setMainGeometry(mDesktop->screenGeometry());
	
	this->setGeometry(screenRects);
	//this->setGeometry(mDesktop->screenGeometry());

}

void DkNoMacsFrameless::exitFullScreen() {

	// TODO: delete this function if we support menu in frameless mode
	if (isFullScreen())
		showNormal();

	if (viewport())
		viewport()->setFullScreen(false);
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
	if (event->type() == QEvent::Gesture) {
		return gestureEvent(static_cast<QGestureEvent*>(event));
	}

	return false;
}

void DkNoMacsFrameless::closeEvent(QCloseEvent *event) {

	// do not save the window size
	if (mSaveSettings)
		DkSettings::save();

	mSaveSettings = false;

	DkNoMacs::closeEvent(event);
}

// Transfer function:

DkNoMacsContrast::DkNoMacsContrast(QWidget *parent, Qt::WindowFlags flags)
	: DkNoMacsSync(parent, flags) {


		setObjectName("DkNoMacsContrast");

		// init members
		DkViewPortContrast* vp = new DkViewPortContrast(this);
		vp->setAlignment(Qt::AlignHCenter);

		DkCentralWidget* cw = new DkCentralWidget(vp, this);
		setCentralWidget(cw);

		mLocalClient = new DkLocalManagerThread(this);
		mLocalClient->setObjectName("localClient");
		mLocalClient->start();

		mLanClient = 0;
		mRcClient = 0;

		init();

		createTransferToolbar();

		setAcceptDrops(true);
		setMouseTracking (true);	//receive mouse event everytime

		// sync signals
		connect(vp, SIGNAL(newClientConnectedSignal(bool, bool)), this, SLOT(newClientConnected(bool, bool)));
		
		initLanClient();
		emit sendTitleSignal(windowTitle());

		DkSettings::app.appMode = DkSettings::mode_contrast;
		setObjectName("DkNoMacsContrast");

		// show it...
		show();

		// TODO: this should be checked but no event should be called
		DkActionManager& am = DkActionManager::instance();
		am.action(DkActionManager::menu_panel_transfertoolbar)->blockSignals(true);
		am.action(DkActionManager::menu_panel_transfertoolbar)->setChecked(true);
		am.action(DkActionManager::menu_panel_transfertoolbar)->blockSignals(false);

		qDebug() << "mViewport (normal) created...";
}

DkNoMacsContrast::~DkNoMacsContrast() {
	release();
}

void DkNoMacsContrast::release() {
}

void DkNoMacsContrast::createTransferToolbar() {

	mTransferToolBar = new DkTransferToolBar(this);

	// add this toolbar below all previous toolbars
	addToolBarBreak();
	addToolBar(mTransferToolBar);
	mTransferToolBar->setObjectName("TransferToolBar");

	//transferToolBar->layout()->setSizeConstraint(QLayout::SetMinimumSize);
	
	connect(mTransferToolBar, SIGNAL(colorTableChanged(QGradientStops)),  viewport(), SLOT(changeColorTable(QGradientStops)));
	connect(mTransferToolBar, SIGNAL(channelChanged(int)),  viewport(), SLOT(changeChannel(int)));
	connect(mTransferToolBar, SIGNAL(pickColorRequest(bool)),  viewport(), SLOT(pickColor(bool)));
	connect(mTransferToolBar, SIGNAL(tFEnabled(bool)), viewport(), SLOT(enableTF(bool)));
	connect((DkViewPortContrast*)viewport(), SIGNAL(tFSliderAdded(qreal)), mTransferToolBar, SLOT(insertSlider(qreal)));
	connect((DkViewPortContrast*)viewport(), SIGNAL(imageModeSet(int)), mTransferToolBar, SLOT(setImageMode(int)));

	if (DkSettings::display.smallIcons)
		mTransferToolBar->setIconSize(QSize(16, 16));
	else
		mTransferToolBar->setIconSize(QSize(32, 32));


	if (DkSettings::display.toolbarGradient)
		mTransferToolBar->setObjectName("toolBarWithGradient");

}
}
