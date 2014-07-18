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
#include "DkImage.h"
#include "DkWidgets.h"
#include "DkDialog.h"
#include "DkSaveDialog.h"
#include "DkSettings.h"
#include "DkMenu.h"
#include "DkToolbars.h"
#include "DkManipulationWidgets.h"
#include "DkSettingsWidgets.h"
#include "DkMessageBox.h"
#include "DkPluginInterface.h"
#include "DkPluginManager.h"

#ifdef WITH_UPNP
#include "DkUpnp.h"
#endif // WITH_UPNP


namespace nmc {

DkNomacsOSXEventFilter::DkNomacsOSXEventFilter(QObject *parent) : QObject(parent) {
}

/*! Handle QFileOpenEvent for mac here */
bool DkNomacsOSXEventFilter::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::FileOpen) {
		emit loadFile(QFileInfo(static_cast<QFileOpenEvent*>(event)->file()));
		return true;
	}
	return QObject::eventFilter(obj, event);
}

DkNoMacs::DkNoMacs(QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags) {

	QMainWindow::setWindowTitle("nomacs - Image Lounge");
	setObjectName("DkNoMacs");

	registerFileVersion();

	saveSettings = true;

	// load settings
	DkSettings* settings = new DkSettings();
	settings->load();
	
	openDialog = 0;
	saveDialog = 0;
	jpgDialog = 0;
	thumbSaver = 0;
	tifDialog = 0;
	resizeDialog = 0;
	opacityDialog = 0;
	updater = 0;
	translationUpdater = 0;
	imgManipulationDialog = 0;
	exportTiffDialog = 0;
	updateDialog = 0;
	progressDialog = 0;
	forceDialog = 0;
	trainDialog = 0;
	pluginManager = 0;
	explorer = 0;
	appManager = 0;
	settingsDialog = 0;
	printPreviewDialog = 0;

	currRunningPlugin = QString();

	// start localhost client/server
	//localClientManager = new DkLocalClientManager(windowTitle());
	//localClientManger->start();

	oldGeometry = geometry();
	overlaid = false;

	menu = new DkMenuBar(this, -1);

	resize(850, 504);
	setMinimumSize(20, 20);

	if (settings)
		delete settings;

}

DkNoMacs::~DkNoMacs() {
	release();
}

void DkNoMacs::release() {

	if (progressDialog) {
		delete progressDialog;
		progressDialog = 0;
	}

	if (appManager) {
		delete appManager;
		appManager = 0;
	}
}

void DkNoMacs::init() {

	//setStyleSheet( "QMainWindow { border-style: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }" );

// assign icon -> in windows the 32px version
#ifdef WIN32
	QString iconPath = ":/nomacs/img/nomacs32.png";
#else
	QString iconPath = ":/nomacs/img/nomacs.png";
#endif

	QIcon dirIcon = QIcon(iconPath);
	setObjectName("DkNoMacs");

	if (!dirIcon.isNull())
		setWindowIcon(dirIcon);

	appManager = new DkAppManager(this);
	connect(appManager, SIGNAL(openFileSignal(QAction*)), this, SLOT(openFileWith(QAction*)));

	// shortcuts and actions
	createIcons();
	createActions();
	createShortcuts();
	createMenu();
	createContextMenu();
	createToolbar();
	createStatusbar();
	enableNoImageActions(false);

	// add actions since they are ignored otherwise if the menu is hidden
	viewport()->addActions(fileActions.toList());
	viewport()->addActions(sortActions.toList());
	viewport()->addActions(editActions.toList());
	viewport()->addActions(toolsActions.toList());
	viewport()->addActions(panelActions.toList());
	viewport()->addActions(viewActions.toList());
	viewport()->addActions(syncActions.toList());
	viewport()->addActions(pluginsActions.toList());
	viewport()->addActions(helpActions.toList());



	// automatically add status tip as tool tip
	for (int idx = 0; idx < fileActions.size(); idx++)
		fileActions[idx]->setToolTip(fileActions[idx]->statusTip());
	// automatically add status tip as tool tip
	for (int idx = 0; idx < sortActions.size(); idx++)
		sortActions[idx]->setToolTip(sortActions[idx]->statusTip());
	for (int idx = 0; idx < editActions.size(); idx++)
		editActions[idx]->setToolTip(editActions[idx]->statusTip());
	for (int idx = 0; idx < toolsActions.size(); idx++)
		toolsActions[idx]->setToolTip(toolsActions[idx]->statusTip());
	for (int idx = 0; idx < panelActions.size(); idx++)
		panelActions[idx]->setToolTip(panelActions[idx]->statusTip());
	for (int idx = 0; idx < viewActions.size(); idx++)
		viewActions[idx]->setToolTip(viewActions[idx]->statusTip());
	for (int idx = 0; idx < syncActions.size(); idx++)
		syncActions[idx]->setToolTip(syncActions[idx]->statusTip());
	for (int idx = 0; idx < pluginsActions.size(); idx++)
		pluginsActions[idx]->setToolTip(pluginsActions[idx]->statusTip());
	for (int idx = 0; idx < helpActions.size(); idx++)
		helpActions[idx]->setToolTip(helpActions[idx]->statusTip());


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
	connect(viewport(), SIGNAL(statusInfoSignal(QString, int)), this, SLOT(showStatusMessage(QString, int)));
	connect(viewport()->getController()->getCropWidget(), SIGNAL(statusInfoSignal(QString)), this, SLOT(showStatusMessage(QString)));
	connect(viewport()->getController()->getThumbWidget()->getThumbWidget(), SIGNAL(statusInfoSignal(QString, int)), this, SLOT(showStatusMessage(QString, int)));
	connect(this, SIGNAL(saveTempFileSignal(QImage)), viewport()->getImageLoader(), SLOT(saveTempFile(QImage)));
	connect(viewport(), SIGNAL(enableNoImageSignal(bool)), this, SLOT(enableNoImageActions(bool)));

	connect(viewport()->getImageLoader(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), this, SLOT(setWindowTitle(QSharedPointer<DkImageContainerT>)));
	//connect(viewport()->getImageLoader(), SIGNAL(newErrorDialog(QString, QString)), this, SLOT(errorDialog(QString, QString)));
	connect(viewport()->getController()->getMetaDataWidget(), SIGNAL(enableGpsSignal(bool)), viewActions[menu_view_gps_map], SLOT(setEnabled(bool)));
	connect(viewport()->getImageLoader(), SIGNAL(folderFiltersChanged(QStringList)), this, SLOT(updateFilterState(QStringList)));
	connect(viewport()->getController()->getCropWidget(), SIGNAL(showToolbar(QToolBar*, bool)), this, SLOT(showToolbar(QToolBar*, bool)));
	connect(viewport(), SIGNAL(movieLoadedSignal(bool)), this, SLOT(enableMovieActions(bool)));
	connect(viewport()->getImageLoader(), SIGNAL(errorDialogSignal(const QString&)), this, SLOT(errorDialog(const QString&)));

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

	QTimer::singleShot(0, this, SLOT(onWindowLoaded()));
}

#ifdef WIN32	// windows specific versioning
//#include <windows.h>
//#undef min
//#undef max
//#include <stdio.h>
//#include <string>

void DkNoMacs::registerFileVersion() {
	
	// this function is based on code from:
	// http://stackoverflow.com/questions/316626/how-do-i-read-from-a-version-resource-in-visual-c

	QString version(NOMACS_VERSION);	// default version (we do not know the build)

	try {
		// get the filename of the executable containing the version resource
		TCHAR szFilename[MAX_PATH + 1] = {0};
		if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0) {
			DkFileException("Sorry, I can't read the module file name\n", __LINE__, __FILE__);
		}

		// allocate a block of memory for the version info
		DWORD dummy;
		DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
		if (dwSize == 0) {
			throw DkFileException("The version info size is zero\n", __LINE__, __FILE__);
		}
		std::vector<BYTE> data(dwSize);

		if (data.empty())
			throw DkFileException("The version info is empty\n", __LINE__, __FILE__);

		// load the version info
		if (!data.empty() && !GetFileVersionInfo(szFilename, NULL, dwSize, &data[0])) {
			throw DkFileException("Sorry, I can't read the version info\n", __LINE__, __FILE__);
		}

		// get the name and version strings
		UINT                uiVerLen = 0;
		VS_FIXEDFILEINFO*   pFixedInfo = 0;     // pointer to fixed file info structure

		if (!data.empty() && !VerQueryValue(&data[0], TEXT("\\"), (void**)&pFixedInfo, (UINT *)&uiVerLen)) {
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

	toolbar = addToolBar(tr("Edit"));
	toolbar->setObjectName("EditToolBar");

	if (DkSettings::display.smallIcons)
		toolbar->setIconSize(QSize(16, 16));
	else
		toolbar->setIconSize(QSize(32, 32));
	
	qDebug() << toolbar->styleSheet();

	if (DkSettings::display.toolbarGradient) {

		QColor hCol = DkSettings::display.highlightColor;
		hCol.setAlpha(80);

		toolbar->setStyleSheet(
			//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
			QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }")
			+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
			//+ QString("QToolButton:disabled{background-color: rgba(0,0,0,10);}")
			+ QString("QToolButton:hover{border: none; background-color: rgba(255,255,255,80);} QToolButton:pressed{margin: 0px; border: none; background-color: " + DkUtils::colorToString(hCol) + ";}")
			);
	}
	//else if (!DkSettings::display.useDefaultColor)
	//	toolbar->setStyleSheet("QToolBar#EditToolBar{background-color: " + DkUtils::colorToString(DkSettings::display.bgColor) + ";}" );

	//// file
	//DkButton* test = new DkButton(fileIcons[icon_file_prev], tr("Pre&vious File"), this);
	//test->setFixedSize(QSize(16,16));
	//test->addAction(fileActions[menu_file_prev]);
	//toolbar->addWidget(test);


	//// add this to get auto-repeat (but it is triggered twice then?)
	//QToolButton *prevButton = static_cast<QToolButton *>(toolbar->widgetForAction(fileActions[menu_file_prev]));
	//prevButton->setAutoRepeat(true);
	////prevButton->setAutoRepeatInterval(100);
	////prevButton->setAutoRepeatDelay(200);
	//connect(prevButton, SIGNAL(pressed()), viewport(), SLOT(loadPrevFileFast()));

	//QToolButton *nextButton = static_cast<QToolButton *>(toolbar->widgetForAction(fileActions[menu_file_next]));
	//nextButton->setAutoRepeat(true);
	////nextButton->setAutoRepeatInterval(100);
	////nextButton->setAutoRepeatDelay(200);
	//connect(nextButton, SIGNAL(pressed()), viewport(), SLOT(loadNextFileFast()));

	toolbar->addAction(fileActions[menu_file_prev]);
	toolbar->addAction(fileActions[menu_file_next]);
	toolbar->addSeparator();

	toolbar->addAction(fileActions[menu_file_open]);
	toolbar->addAction(fileActions[menu_file_open_dir]);
	toolbar->addAction(fileActions[menu_file_save]);
	toolbar->addAction(toolsActions[menu_tools_filter]);
	toolbar->addSeparator();

	// edit
	toolbar->addAction(editActions[menu_edit_rotate_ccw]);
	toolbar->addAction(editActions[menu_edit_rotate_cw]);
	toolbar->addSeparator();

	toolbar->addAction(editActions[menu_edit_crop]);
	toolbar->addAction(editActions[menu_edit_transform]);
	//toolbar->addAction(editActions[menu_edit_image_manipulation]);
	toolbar->addSeparator();

	// view
	toolbar->addAction(viewActions[menu_view_fullscreen]);
	toolbar->addAction(viewActions[menu_view_reset]);
	toolbar->addAction(viewActions[menu_view_100]);
	toolbar->addSeparator();

	toolbar->addAction(viewActions[menu_view_gps_map]);

	movieToolbar = addToolBar(tr("Movie Toolbar"));
	movieToolbar->setObjectName("movieToolbar");
	//movieToolbar->addSeparator();
	movieToolbar->addAction(viewActions[menu_view_movie_prev]);
	movieToolbar->addAction(viewActions[menu_view_movie_pause]);
	movieToolbar->addAction(viewActions[menu_view_movie_next]);

	if (DkSettings::display.toolbarGradient) {

		QColor hCol = DkSettings::display.highlightColor;
		hCol.setAlpha(80);

		movieToolbar->setStyleSheet(
			//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
			QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }")
			+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
			//+ QString("QToolButton:disabled{background-color: rgba(0,0,0,10);}")
			+ QString("QToolButton:hover{border: none; background-color: rgba(255,255,255,80);} QToolButton:pressed{margin: 0px; border: none; background-color: " + DkUtils::colorToString(hCol) + ";}")
			);
	}

	if (DkSettings::display.smallIcons)
		movieToolbar->setIconSize(QSize(16, 16));
	else
		movieToolbar->setIconSize(QSize(32, 32));
}


void DkNoMacs::createStatusbar() {

	statusbarLabels.resize(status_end);

	statusbarLabels[status_pixel_info] = new QLabel();
	statusbarLabels[status_pixel_info]->hide();
	statusbarLabels[status_pixel_info]->setToolTip(tr("CTRL activates the crosshair cursor"));

	statusbar = new QStatusBar(this);
	statusbar->setObjectName("DkStatusBar");
	QColor col = QColor(200, 200, 230, 100);

	if (DkSettings::display.toolbarGradient)
		statusbar->setStyleSheet(QString("QStatusBar {border-top: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }"));	
	//else if (!DkSettings::display.useDefaultColor)
	//	statusbar->setStyleSheet("QStatusBar#DkStatusBar{background-color: " + DkUtils::colorToString(DkSettings::display.bgColor) + ";}");



	statusbar->addWidget(statusbarLabels[status_pixel_info]);
	statusbar->hide();

	for (int idx = 1; idx < statusbarLabels.size(); idx++) {
		statusbarLabels[idx] = new QLabel();
		statusbarLabels[idx]->hide();
		statusbarLabels[idx]->setStyleSheet("QLabel{color: #555555;}");
		statusbar->addPermanentWidget(statusbarLabels[idx]);

	}

	//statusbar->addPermanentWidget()
	this->setStatusBar(statusbar);
}


void DkNoMacs::createIcons() {

	// this is unbelievable dirty - but for now the quickest way to turn themes off if someone uses customized icons...
	if (DkSettings::display.defaultIconColor) {
		#define ICON(theme, backup) QIcon::fromTheme((theme), QIcon((backup)))
	}
	else {
		#undef ICON
		#define ICON(theme, backup) QIcon(backup), QIcon(backup)
	}

	fileIcons.resize(icon_file_end);
	fileIcons[icon_file_dir] = ICON("document-open-folder", ":/nomacs/img/dir.png");
	fileIcons[icon_file_open] = ICON("document-open", ":/nomacs/img/open.png");
	fileIcons[icon_file_save] = ICON("document-save", ":/nomacs/img/save.png");
	fileIcons[icon_file_print] = ICON("document-print", ":/nomacs/img/printer.png");
	fileIcons[icon_file_open_large] = ICON("document-open-large", ":/nomacs/img/open-large.png");
	fileIcons[icon_file_dir_large] = ICON("document-open-folder-large", ":/nomacs/img/dir-large.png");
	fileIcons[icon_file_prev] = ICON("go-previous", ":/nomacs/img/previous.png");
	fileIcons[icon_file_next] = ICON("go-next", ":/nomacs/img/next.png");
	fileIcons[icon_file_filter] = QIcon();
	fileIcons[icon_file_filter].addPixmap(QPixmap(":/nomacs/img/filter.png"), QIcon::Normal, QIcon::On);
	fileIcons[icon_file_filter].addPixmap(QPixmap(":/nomacs/img/nofilter.png"), QIcon::Normal, QIcon::Off);
	
	editIcons.resize(icon_edit_end);
	editIcons[icon_edit_rotate_cw] = ICON("object-rotate-right", ":/nomacs/img/rotate-cw.png");
	editIcons[icon_edit_rotate_ccw] = ICON("object-rotate-left", ":/nomacs/img/rotate-cc.png");
	editIcons[icon_edit_crop] = ICON("object-edit-crop", ":/nomacs/img/crop.png");
	editIcons[icon_edit_resize] = ICON("object-edit-resize", ":/nomacs/img/resize.png");

	viewIcons.resize(icon_view_end);
	viewIcons[icon_view_fullscreen] = ICON("view-fullscreen", ":/nomacs/img/fullscreen.png");
	viewIcons[icon_view_reset] = ICON("zoom-draw", ":/nomacs/img/zoomReset.png");
	viewIcons[icon_view_100] = ICON("zoom-original", ":/nomacs/img/zoom100.png");
	viewIcons[icon_view_gps] = ICON("", ":/nomacs/img/gps-globe.png");
	viewIcons[icon_view_movie_play] = QIcon();
	viewIcons[icon_view_movie_play].addPixmap(QPixmap(":/nomacs/img/movie-play.png"), QIcon::Normal, QIcon::On);
	viewIcons[icon_view_movie_play].addPixmap(QPixmap(":/nomacs/img/movie-pause.png"), QIcon::Normal, QIcon::Off);
	viewIcons[icon_view_movie_prev] = ICON("", ":/nomacs/img/movie-prev.png");
	viewIcons[icon_view_movie_next] = ICON("", ":/nomacs/img/movie-next.png");

	toolsIcons.resize(icon_tools_end);
	toolsIcons[icon_tools_manipulation] = ICON("", ":/nomacs/img/manipulation.png");

	if (!DkSettings::display.defaultIconColor) {
		// now colorize all icons
		for (int idx = 0; idx < fileIcons.size(); idx++) {

			// never colorize these large icons
			if (idx == icon_file_open_large || idx == icon_file_dir_large)
				continue;

			fileIcons[idx].addPixmap(DkImage::colorizePixmap(fileIcons[idx].pixmap(100, QIcon::Normal, QIcon::On), DkSettings::display.iconColor), QIcon::Normal, QIcon::On);
			fileIcons[idx].addPixmap(DkImage::colorizePixmap(fileIcons[idx].pixmap(100, QIcon::Normal, QIcon::Off), DkSettings::display.iconColor), QIcon::Normal, QIcon::Off);
		}

		// now colorize all icons
		for (int idx = 0; idx < editIcons.size(); idx++)
			editIcons[idx].addPixmap(DkImage::colorizePixmap(editIcons[idx].pixmap(100), DkSettings::display.iconColor));

		for (int idx = 0; idx < viewIcons.size(); idx++)
			viewIcons[idx].addPixmap(DkImage::colorizePixmap(viewIcons[idx].pixmap(100), DkSettings::display.iconColor));

		for (int idx = 0; idx < toolsIcons.size(); idx++)
			toolsIcons[idx].addPixmap(DkImage::colorizePixmap(toolsIcons[idx].pixmap(100), DkSettings::display.iconColor));

	}
}

void DkNoMacs::createMenu() {

	this->setMenuBar(menu);
	fileMenu = menu->addMenu(tr("&File"));
	fileMenu->addAction(fileActions[menu_file_open]);
	fileMenu->addAction(fileActions[menu_file_open_dir]);
	
	openWithMenu = new QMenu(tr("Open &With"), fileMenu);
	createOpenWithMenu(openWithMenu);
	fileMenu->addMenu(openWithMenu);

	fileMenu->addSeparator();
	fileMenu->addAction(fileActions[menu_file_save]);
	fileMenu->addAction(fileActions[menu_file_save_as]);
	fileMenu->addAction(fileActions[menu_file_save_web]);
	fileMenu->addAction(fileActions[menu_file_rename]);
	fileMenu->addSeparator();

	//fileFilesMenu = new DkHistoryMenu(tr("Recent &Files"), fileMenu, &DkSettings::global.recentFiles);
	//connect(fileFilesMenu, SIGNAL(loadFileSignal(QFileInfo)), viewport(), SLOT(loadFile(QFileInfo)));
	//connect(fileFilesMenu, SIGNAL(clearHistory()), this, SLOT(clearFileHistory()));

	//fileFoldersMenu = new DkHistoryMenu(tr("Recent Fo&lders"), fileMenu, &DkSettings::global.recentFolders);
	//connect(fileFoldersMenu, SIGNAL(loadFileSignal(QFileInfo)), viewport(), SLOT(loadFile(QFileInfo)));
	//connect(fileFoldersMenu, SIGNAL(clearHistory()), this, SLOT(clearFolderHistory()));

	//fileMenu->addMenu(fileFilesMenu);
	//fileMenu->addMenu(fileFoldersMenu);
	fileMenu->addAction(fileActions[menu_file_show_recent]);

	fileMenu->addSeparator();
	fileMenu->addAction(fileActions[menu_file_print]);
	fileMenu->addSeparator();
	
	sortMenu = new QMenu(tr("S&ort"), fileMenu);
	sortMenu->addAction(sortActions[menu_sort_filename]);
	sortMenu->addAction(sortActions[menu_sort_date_created]);
	sortMenu->addAction(sortActions[menu_sort_date_modified]);
	sortMenu->addAction(sortActions[menu_sort_random]);
	sortMenu->addSeparator();
	sortMenu->addAction(sortActions[menu_sort_ascending]);
	sortMenu->addAction(sortActions[menu_sort_descending]);

	fileMenu->addMenu(sortMenu);
	fileMenu->addAction(fileActions[menu_file_recursive]);

	fileMenu->addAction(fileActions[menu_file_goto]);
	fileMenu->addAction(fileActions[menu_file_find]);
	fileMenu->addAction(fileActions[menu_file_reload]);
	fileMenu->addAction(fileActions[menu_file_prev]);
	fileMenu->addAction(fileActions[menu_file_next]);
	fileMenu->addSeparator();
	//fileMenu->addAction(fileActions[menu_file_share_fb]);
	//fileMenu->addSeparator();
	fileMenu->addAction(fileActions[menu_file_train_format]);
	fileMenu->addSeparator();
	fileMenu->addAction(fileActions[menu_file_new_instance]);
	fileMenu->addAction(fileActions[menu_file_exit]);

	editMenu = menu->addMenu(tr("&Edit"));
	editMenu->addAction(editActions[menu_edit_copy]);
	editMenu->addAction(editActions[menu_edit_copy_buffer]);
	editMenu->addAction(editActions[menu_edit_paste]);
	editMenu->addAction(editActions[menu_edit_delete]);
	editMenu->addSeparator();
	editMenu->addAction(editActions[menu_edit_rotate_ccw]);
	editMenu->addAction(editActions[menu_edit_rotate_cw]);
	editMenu->addAction(editActions[menu_edit_rotate_180]);
	editMenu->addSeparator();

	editMenu->addAction(editActions[menu_edit_transform]);
	editMenu->addAction(editActions[menu_edit_crop]);
	editMenu->addAction(editActions[menu_edit_flip_h]);
	editMenu->addAction(editActions[menu_edit_flip_v]);
	editMenu->addSeparator();
	editMenu->addAction(editActions[menu_edit_auto_adjust]);
	editMenu->addAction(editActions[menu_edit_norm]);
	editMenu->addAction(editActions[menu_edit_invert]);
#ifdef WITH_OPENCV
	editMenu->addAction(editActions[menu_edit_unsharp]);
#endif
	editMenu->addSeparator();
#ifdef WIN32
	editMenu->addAction(editActions[menu_edit_wallpaper]);
	editMenu->addSeparator();
#endif
	editMenu->addAction(editActions[menu_edit_shortcuts]);
	editMenu->addAction(editActions[menu_edit_preferences]);

	viewMenu = menu->addMenu(tr("&View"));
	
	viewMenu->addAction(viewActions[menu_view_frameless]);	
	viewMenu->addAction(viewActions[menu_view_fullscreen]);
	viewMenu->addSeparator();

	viewMenu->addAction(viewActions[menu_view_reset]);
	viewMenu->addAction(viewActions[menu_view_100]);
	viewMenu->addAction(viewActions[menu_view_fit_frame]);
	viewMenu->addAction(viewActions[menu_view_zoom_in]);
	viewMenu->addAction(viewActions[menu_view_zoom_out]);
	viewMenu->addSeparator();

	viewMenu->addAction(viewActions[menu_view_tp_pattern]);
	viewMenu->addAction(viewActions[menu_view_anti_aliasing]);
	viewMenu->addSeparator();

	viewMenu->addAction(viewActions[menu_view_opacity_change]);
	viewMenu->addAction(viewActions[menu_view_opacity_up]);
	viewMenu->addAction(viewActions[menu_view_opacity_down]);
	viewMenu->addAction(viewActions[menu_view_opacity_an]);
#ifdef WIN32
	viewMenu->addAction(viewActions[menu_view_lock_window]);
#endif
	viewMenu->addSeparator();

	viewMenu->addAction(viewActions[menu_view_movie_pause]);
	viewMenu->addAction(viewActions[menu_view_movie_prev]);
	viewMenu->addAction(viewActions[menu_view_movie_next]);

	viewMenu->addSeparator();
	viewMenu->addAction(viewActions[menu_view_gps_map]);

	panelMenu = menu->addMenu(tr("&Panels"));
	panelToolsMenu = panelMenu->addMenu(tr("Tool&bars"));
	panelToolsMenu->addAction(panelActions[menu_panel_menu]);
	panelToolsMenu->addAction(panelActions[menu_panel_toolbar]);
	panelToolsMenu->addAction(panelActions[menu_panel_statusbar]);
	panelToolsMenu->addAction(panelActions[menu_panel_transfertoolbar]);
	panelMenu->addAction(panelActions[menu_panel_explorer]);
	panelMenu->addAction(panelActions[menu_panel_preview]);
	panelMenu->addAction(panelActions[menu_panel_thumbview]);
	panelMenu->addAction(panelActions[menu_panel_scroller]);
	panelMenu->addAction(panelActions[menu_panel_exif]);
	
	panelMenu->addSeparator();
	
	panelMenu->addAction(panelActions[menu_panel_overview]);
	panelMenu->addAction(panelActions[menu_panel_player]);
	panelMenu->addAction(panelActions[menu_panel_info]);
	panelMenu->addAction(panelActions[menu_panel_histogram]);

	toolsMenu = menu->addMenu(tr("&Tools"));
	toolsMenu->addAction(toolsActions[menu_tools_thumbs]);
	toolsMenu->addAction(toolsActions[menu_tools_filter]);
#ifdef WITH_OPENCV
	toolsMenu->addAction(toolsActions[menu_tools_manipulation]);
#endif
#ifdef WITH_LIBTIFF
	toolsMenu->addAction(toolsActions[menu_tools_export_tiff]);
#endif
#ifdef WITH_OPENCV
	toolsMenu->addAction(toolsActions[menu_tools_mosaic]);
#endif

	// no sync menu in frameless view
	if (DkSettings::app.appMode != DkSettings::mode_frameless)
		syncMenu = menu->addMenu(tr("&Sync"));
	else 
		syncMenu = 0;

#ifdef WITH_PLUGINS
	// plugin menu
	pluginsMenu = menu->addMenu(tr("Pl&ugins"));
	connect(pluginsMenu, SIGNAL(aboutToShow()), this, SLOT(initPluginManager()));
#endif // WITH_PLUGINS

	helpMenu = menu->addMenu(tr("&?"));
#ifndef Q_WS_X11
	helpMenu->addAction(helpActions[menu_help_update]);
#endif // !Q_WS_X11
	helpMenu->addAction(helpActions[menu_help_update_translation]);
	helpMenu->addSeparator();
	helpMenu->addAction(helpActions[menu_help_bug]);
	helpMenu->addAction(helpActions[menu_help_feature]);
	helpMenu->addSeparator();
	helpMenu->addAction(helpActions[menu_help_documentation]);
	helpMenu->addAction(helpActions[menu_help_about]);

}

void DkNoMacs::createOpenWithMenu(QMenu* menu) {

	QList<QAction* > oldActions = openWithMenu->findChildren<QAction* >();

	// remove old actions
	for (int idx = 0; idx < oldActions.size(); idx++)
		viewport()->removeAction(oldActions.at(idx));

	QVector<QAction* > appActions = appManager->getActions();

	for (int idx = 0; idx < appActions.size(); idx++)
		qDebug() << "adding action: " << appActions[idx]->text() << " " << appActions[idx]->toolTip();

	assignCustomShortcuts(appActions);
	openWithMenu->addActions(appActions.toList());
	
	if (!appActions.empty())
		openWithMenu->addSeparator();
	openWithMenu->addAction(fileActions[menu_file_app_manager]);
	viewport()->addActions(appActions.toList());
}

void DkNoMacs::createContextMenu() {

	contextMenu = new QMenu(this);

	contextMenu->addAction(panelActions[menu_panel_explorer]);
	contextMenu->addAction(panelActions[menu_panel_preview]);
	contextMenu->addAction(panelActions[menu_panel_thumbview]);
	contextMenu->addAction(panelActions[menu_panel_scroller]);
	contextMenu->addAction(panelActions[menu_panel_exif]);
	contextMenu->addAction(panelActions[menu_panel_overview]);
	contextMenu->addAction(panelActions[menu_panel_player]);
	contextMenu->addAction(panelActions[menu_panel_info]);
	contextMenu->addAction(panelActions[menu_panel_histogram]);
	contextMenu->addSeparator();
	
	contextMenu->addAction(editActions[menu_edit_copy_buffer]);
	contextMenu->addAction(editActions[menu_edit_copy]);
	contextMenu->addAction(editActions[menu_edit_copy_color]);
	contextMenu->addAction(editActions[menu_edit_paste]);
	contextMenu->addSeparator();
	
	contextMenu->addAction(viewActions[menu_view_frameless]);
	contextMenu->addSeparator();

	contextMenu->addMenu(sortMenu);

	QMenu* viewContextMenu = contextMenu->addMenu(tr("&View"));
	viewContextMenu->addAction(viewActions[menu_view_fullscreen]);
	viewContextMenu->addAction(viewActions[menu_view_reset]);
	viewContextMenu->addAction(viewActions[menu_view_100]);
	viewContextMenu->addAction(viewActions[menu_view_fit_frame]);

	QMenu* editContextMenu = contextMenu->addMenu(tr("&Edit"));
	editContextMenu->addAction(editActions[menu_edit_rotate_cw]);
	editContextMenu->addAction(editActions[menu_edit_rotate_ccw]);
	editContextMenu->addAction(editActions[menu_edit_rotate_180]);
	editContextMenu->addSeparator();
	editContextMenu->addAction(editActions[menu_edit_transform]);
	editContextMenu->addAction(editActions[menu_edit_crop]);
	editContextMenu->addAction(editActions[menu_edit_delete]);

	if (syncMenu)
		contextMenu->addMenu(syncMenu);

	contextMenu->addSeparator();
	contextMenu->addAction(editActions[menu_edit_preferences]);

}

void DkNoMacs::createActions() {

	DkViewPort* vp = viewport();

	fileActions.resize(menu_file_end);
	
	fileActions[menu_file_open] = new QAction(fileIcons[icon_file_open], tr("&Open"), this);
	fileActions[menu_file_open]->setShortcuts(QKeySequence::Open);
	fileActions[menu_file_open]->setStatusTip(tr("Open an image"));
	connect(fileActions[menu_file_open], SIGNAL(triggered()), this, SLOT(openFile()));

	fileActions[menu_file_open_dir] = new QAction(fileIcons[icon_file_dir], tr("Open &Directory"), this);
	fileActions[menu_file_open_dir]->setShortcut(QKeySequence(shortcut_open_dir));
	fileActions[menu_file_open_dir]->setStatusTip(tr("Open a directory and load its first image"));
	connect(fileActions[menu_file_open_dir], SIGNAL(triggered()), this, SLOT(openDir()));

	fileActions[menu_file_app_manager] = new QAction(tr("&Manage Applications"), this);
	fileActions[menu_file_app_manager]->setStatusTip(tr("Manage Applications which are Automatically Opened"));
	fileActions[menu_file_app_manager]->setShortcut(QKeySequence(shortcut_app_manager));
	connect(fileActions[menu_file_app_manager], SIGNAL(triggered()), this, SLOT(openAppManager()));

	fileActions[menu_file_rename] = new QAction(tr("Re&name"), this);
	fileActions[menu_file_rename]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	fileActions[menu_file_rename]->setShortcut(QKeySequence(shortcut_rename));
	fileActions[menu_file_rename]->setStatusTip(tr("Rename an image"));
	connect(fileActions[menu_file_rename], SIGNAL(triggered()), this, SLOT(renameFile()));

	fileActions[menu_file_goto] = new QAction(tr("&Go To"), this);
	fileActions[menu_file_goto]->setShortcut(QKeySequence(shortcut_goto));
	fileActions[menu_file_goto]->setStatusTip(tr("Go To an image"));
	connect(fileActions[menu_file_goto], SIGNAL(triggered()), this, SLOT(goTo()));

	fileActions[menu_file_save] = new QAction(fileIcons[icon_file_save], tr("&Save"), this);
	fileActions[menu_file_save]->setShortcuts(QKeySequence::Save);
	fileActions[menu_file_save]->setStatusTip(tr("Save an image"));
	connect(fileActions[menu_file_save], SIGNAL(triggered()), this, SLOT(saveFile()));

	fileActions[menu_file_save_as] = new QAction(tr("&Save As"), this);
	fileActions[menu_file_save_as]->setShortcut(QKeySequence(shortcut_save_as));
	fileActions[menu_file_save_as]->setStatusTip(tr("Save an image as"));
	connect(fileActions[menu_file_save_as], SIGNAL(triggered()), this, SLOT(saveFileAs()));

	fileActions[menu_file_save_web] = new QAction(tr("&Save for Web"), this);
	fileActions[menu_file_save_web]->setStatusTip(tr("Save an Image for Web Applications"));
	connect(fileActions[menu_file_save_web], SIGNAL(triggered()), this, SLOT(saveFileWeb()));

	fileActions[menu_file_print] = new QAction(fileIcons[icon_file_print], tr("&Print"), this);
	fileActions[menu_file_print]->setShortcuts(QKeySequence::Print);
	fileActions[menu_file_print]->setStatusTip(tr("Print an image"));
	connect(fileActions[menu_file_print], SIGNAL(triggered()), this, SLOT(printDialog()));

	fileActions[menu_file_show_recent] = new QAction(tr("&Recent Files and Folders"), this);
	fileActions[menu_file_show_recent]->setShortcut(QKeySequence(shortcut_recent_files));
	fileActions[menu_file_show_recent]->setCheckable(true);
	fileActions[menu_file_show_recent]->setChecked(false);
	fileActions[menu_file_show_recent]->setStatusTip(tr("Show Recent Files and Folders"));
	connect(fileActions[menu_file_show_recent], SIGNAL(triggered(bool)), vp->getController(), SLOT(showRecentFiles(bool)));

	fileActions[menu_file_reload] = new QAction(tr("&Reload File"), this);
	fileActions[menu_file_reload]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	fileActions[menu_file_reload]->setShortcuts(QKeySequence::Refresh);
	fileActions[menu_file_reload]->setStatusTip(tr("Reload File"));
	connect(fileActions[menu_file_reload], SIGNAL(triggered()), vp, SLOT(reloadFile()));

	fileActions[menu_file_next] = new QAction(fileIcons[icon_file_next], tr("Ne&xt File"), viewport());
	fileActions[menu_file_next]->setShortcutContext(Qt::WidgetShortcut);
	fileActions[menu_file_next]->setShortcut(QKeySequence(shortcut_next_file));
	fileActions[menu_file_next]->setStatusTip(tr("Load next image"));
	connect(fileActions[menu_file_next], SIGNAL(triggered()), vp, SLOT(loadNextFileFast()));

	fileActions[menu_file_prev] = new QAction(fileIcons[icon_file_prev], tr("Pre&vious File"), this);
	fileActions[menu_file_prev]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	fileActions[menu_file_prev]->setShortcut(QKeySequence(shortcut_prev_file));
	fileActions[menu_file_prev]->setStatusTip(tr("Load previous file"));
	connect(fileActions[menu_file_prev], SIGNAL(triggered()), vp, SLOT(loadPrevFileFast()));

	fileActions[menu_file_train_format] = new QAction(tr("Add Image Format"), this);
	fileActions[menu_file_train_format]->setStatusTip(tr("Add a new image format to nomacs"));
	connect(fileActions[menu_file_train_format], SIGNAL(triggered()), this, SLOT(trainFormat()));

	fileActions[menu_file_new_instance] = new QAction(tr("St&art New Instance"), this);
	fileActions[menu_file_new_instance]->setShortcut(QKeySequence(shortcut_new_instance));
	fileActions[menu_file_new_instance]->setStatusTip(tr("Open file in new instance"));
	connect(fileActions[menu_file_new_instance], SIGNAL(triggered()), this, SLOT(newInstance()));

	fileActions[menu_file_find] = new QAction(tr("&Find && Filter"), this);
	fileActions[menu_file_find]->setShortcut(QKeySequence::Find);
	fileActions[menu_file_find]->setStatusTip(tr("Find an image"));
	connect(fileActions[menu_file_find], SIGNAL(triggered()), this, SLOT(find()));

	fileActions[menu_file_recursive] = new QAction(tr("Scan Folder Re&cursive"), this);
	fileActions[menu_file_recursive]->setStatusTip(tr("Step through Folder and Sub Folders"));
	fileActions[menu_file_recursive]->setCheckable(true);
	fileActions[menu_file_recursive]->setChecked(DkSettings::global.scanSubFolders);
	connect(fileActions[menu_file_recursive], SIGNAL(triggered(bool)), this, SLOT(setRecursiveScan(bool)));

	//fileActions[menu_file_share_fb] = new QAction(tr("Share on &Facebook"), this);
	////fileActions[menu_file_share_fb]->setShortcuts(QKeySequence::Close);
	//fileActions[menu_file_share_fb]->setStatusTip(tr("Shares the image on facebook"));
	//connect(fileActions[menu_file_share_fb], SIGNAL(triggered()), this, SLOT(shareFacebook()));

	fileActions[menu_file_exit] = new QAction(tr("&Exit"), this);
	fileActions[menu_file_exit]->setShortcuts(QKeySequence::Close);
	fileActions[menu_file_exit]->setStatusTip(tr("Exit"));
	connect(fileActions[menu_file_exit], SIGNAL(triggered()), this, SLOT(close()));

	sortActions.resize(menu_sort_end);

	sortActions[menu_sort_filename] = new QAction(tr("by &Filename"), this);
	sortActions[menu_sort_filename]->setObjectName("menu_sort_filename");
	sortActions[menu_sort_filename]->setStatusTip(tr("Sort by Filename"));
	sortActions[menu_sort_filename]->setCheckable(true);
	sortActions[menu_sort_filename]->setChecked(DkSettings::global.sortMode == DkSettings::sort_filename);
	connect(sortActions[menu_sort_filename], SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));

	sortActions[menu_sort_date_created] = new QAction(tr("by Date &Created"), this);
	sortActions[menu_sort_date_created]->setObjectName("menu_sort_date_created");
	sortActions[menu_sort_date_created]->setStatusTip(tr("Sort by Date Created"));
	sortActions[menu_sort_date_created]->setCheckable(true);
	sortActions[menu_sort_date_created]->setChecked(DkSettings::global.sortMode == DkSettings::sort_date_created);
	connect(sortActions[menu_sort_date_created], SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));

	sortActions[menu_sort_date_modified] = new QAction(tr("by Date Modified"), this);
	sortActions[menu_sort_date_modified]->setObjectName("menu_sort_date_modified");
	sortActions[menu_sort_date_modified]->setStatusTip(tr("Sort by Date Last Modified"));
	sortActions[menu_sort_date_modified]->setCheckable(true);
	sortActions[menu_sort_date_modified]->setChecked(DkSettings::global.sortMode == DkSettings::sort_date_modified);
	connect(sortActions[menu_sort_date_modified], SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));

	sortActions[menu_sort_random] = new QAction(tr("Random"), this);
	sortActions[menu_sort_random]->setObjectName("menu_sort_random");
	sortActions[menu_sort_random]->setStatusTip(tr("Sort in Random Order"));
	sortActions[menu_sort_random]->setCheckable(true);
	sortActions[menu_sort_random]->setChecked(DkSettings::global.sortMode == DkSettings::sort_random);
	connect(sortActions[menu_sort_random], SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));

	sortActions[menu_sort_ascending] = new QAction(tr("&Ascending"), this);
	sortActions[menu_sort_ascending]->setObjectName("menu_sort_ascending");
	sortActions[menu_sort_ascending]->setStatusTip(tr("Sort in Ascending Order"));
	sortActions[menu_sort_ascending]->setCheckable(true);
	sortActions[menu_sort_ascending]->setChecked(DkSettings::global.sortDir == Qt::AscendingOrder);
	connect(sortActions[menu_sort_ascending], SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));

	sortActions[menu_sort_descending] = new QAction(tr("&Descending"), this);
	sortActions[menu_sort_descending]->setObjectName("menu_sort_descending");
	sortActions[menu_sort_descending]->setStatusTip(tr("Sort in Descending Order"));
	sortActions[menu_sort_descending]->setCheckable(true);
	sortActions[menu_sort_descending]->setChecked(DkSettings::global.sortDir == Qt::DescendingOrder);
	connect(sortActions[menu_sort_descending], SIGNAL(triggered(bool)), this, SLOT(changeSorting(bool)));

	editActions.resize(menu_edit_end);

	editActions[menu_edit_rotate_cw] = new QAction(editIcons[icon_edit_rotate_cw], tr("9&0° Clockwise"), this);
	editActions[menu_edit_rotate_cw]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	editActions[menu_edit_rotate_cw]->setShortcut(QKeySequence(shortcut_rotate_cw));
	editActions[menu_edit_rotate_cw]->setStatusTip(tr("rotate the image 90° clockwise"));
	connect(editActions[menu_edit_rotate_cw], SIGNAL(triggered()), vp, SLOT(rotateCW()));

	editActions[menu_edit_rotate_ccw] = new QAction(editIcons[icon_edit_rotate_ccw], tr("&90° Counter Clockwise"), this);
	editActions[menu_edit_rotate_ccw]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	editActions[menu_edit_rotate_ccw]->setShortcut(QKeySequence(shortcut_rotate_ccw));
	editActions[menu_edit_rotate_ccw]->setStatusTip(tr("rotate the image 90° counter clockwise"));
	connect(editActions[menu_edit_rotate_ccw], SIGNAL(triggered()), vp, SLOT(rotateCCW()));

	editActions[menu_edit_rotate_180] = new QAction(tr("1&80°"), this);
	editActions[menu_edit_rotate_180]->setStatusTip(tr("rotate the image by 180°"));
	connect(editActions[menu_edit_rotate_180], SIGNAL(triggered()), vp, SLOT(rotate180()));

	editActions[menu_edit_copy] = new QAction(tr("&Copy"), this);
	editActions[menu_edit_copy]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	editActions[menu_edit_copy]->setShortcut(QKeySequence::Copy);
	editActions[menu_edit_copy]->setStatusTip(tr("copy image"));
	connect(editActions[menu_edit_copy], SIGNAL(triggered()), this, SLOT(copyImage()));

	editActions[menu_edit_copy_buffer] = new QAction(tr("Copy &Buffer"), this);
	editActions[menu_edit_copy_buffer]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	editActions[menu_edit_copy_buffer]->setShortcut(shortcut_copy_buffer);
	editActions[menu_edit_copy_buffer]->setStatusTip(tr("copy image"));
	connect(editActions[menu_edit_copy_buffer], SIGNAL(triggered()), this, SLOT(copyImageBuffer()));

	editActions[menu_edit_copy_color] = new QAction(tr("Copy Co&lor"), this);
	editActions[menu_edit_copy_color]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	editActions[menu_edit_copy_color]->setShortcut(shortcut_copy_color);
	editActions[menu_edit_copy_color]->setStatusTip(tr("copy pixel color value as HEX"));
	connect(editActions[menu_edit_copy_color], SIGNAL(triggered()), this, SLOT(copyPixelColorValue()));

	QList<QKeySequence> pastScs;
	pastScs.append(QKeySequence::Paste);
	pastScs.append(shortcut_paste);
	editActions[menu_edit_paste] = new QAction(tr("&Paste"), this);
	editActions[menu_edit_paste]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	editActions[menu_edit_paste]->setShortcuts(pastScs);
	editActions[menu_edit_paste]->setStatusTip(tr("paste image"));
	connect(editActions[menu_edit_paste], SIGNAL(triggered()), this, SLOT(pasteImage()));

	editActions[menu_edit_transform] = new QAction(editIcons[icon_edit_resize], tr("R&esize Image"), this);
	editActions[menu_edit_transform]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	editActions[menu_edit_transform]->setShortcut(shortcut_transform);
	editActions[menu_edit_transform]->setStatusTip(tr("resize the current image"));
	connect(editActions[menu_edit_transform], SIGNAL(triggered()), this, SLOT(resizeImage()));

	editActions[menu_edit_crop] = new QAction(editIcons[icon_edit_crop], tr("Cr&op Image"), this);
	editActions[menu_edit_crop]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	editActions[menu_edit_crop]->setShortcut(shortcut_crop);
	editActions[menu_edit_crop]->setStatusTip(tr("cut the current image"));
	editActions[menu_edit_crop]->setCheckable(true);
	editActions[menu_edit_crop]->setChecked(false);
	connect(editActions[menu_edit_crop], SIGNAL(triggered(bool)), vp->getController(), SLOT(showCrop(bool)));

	editActions[menu_edit_flip_h] = new QAction(tr("Flip &Horizontal"), this);
	//editActions[menu_edit_flip_h]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	//editActions[menu_edit_flip_h]->setShortcut();
	editActions[menu_edit_flip_h]->setStatusTip(tr("Flip Image Horizontally"));
	connect(editActions[menu_edit_flip_h], SIGNAL(triggered()), this, SLOT(flipImageHorizontal()));

	editActions[menu_edit_flip_v] = new QAction(tr("Flip &Vertical"), this);
	//editActions[menu_edit_flip_v]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	//editActions[menu_edit_flip_v]->setShortcut();
	editActions[menu_edit_flip_v]->setStatusTip(tr("Flip Image Vertically"));
	connect(editActions[menu_edit_flip_v], SIGNAL(triggered()), this, SLOT(flipImageVertical()));

	editActions[menu_edit_norm] = new QAction(tr("Nor&malize Image"), this);
	editActions[menu_edit_norm]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	editActions[menu_edit_norm]->setShortcut(shortcut_norm_image);
	editActions[menu_edit_norm]->setStatusTip(tr("Normalize the Image"));
	connect(editActions[menu_edit_norm], SIGNAL(triggered()), this, SLOT(normalizeImage()));

	editActions[menu_edit_auto_adjust] = new QAction(tr("&Auto Adjust"), this);
	editActions[menu_edit_auto_adjust]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	editActions[menu_edit_auto_adjust]->setShortcut(shortcut_auto_adjust);
	editActions[menu_edit_auto_adjust]->setStatusTip(tr("Auto Adjust Image Contrast and Color Balance"));
	connect(editActions[menu_edit_auto_adjust], SIGNAL(triggered()), this, SLOT(autoAdjustImage()));
	
	editActions[menu_edit_invert] = new QAction(tr("&Invert Image"), this);
	//editActions[menu_edit_invert]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	//editActions[menu_edit_invert]->setShortcut();
	editActions[menu_edit_invert]->setStatusTip(tr("Invert the Image"));
	connect(editActions[menu_edit_invert], SIGNAL(triggered()), this, SLOT(invertImage()));

	editActions[menu_edit_unsharp] = new QAction(tr("&Unsharp Mask"), this);
	editActions[menu_edit_unsharp]->setStatusTip(tr("Stretches the Local Contrast of an Image"));
	connect(editActions[menu_edit_unsharp], SIGNAL(triggered()), this, SLOT(unsharpMask()));

	editActions[menu_edit_delete] = new QAction(tr("&Delete"), this);
	editActions[menu_edit_delete]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	editActions[menu_edit_delete]->setShortcut(QKeySequence::Delete);
	editActions[menu_edit_delete]->setStatusTip(tr("delete current file"));
	connect(editActions[menu_edit_delete], SIGNAL(triggered()), this, SLOT(deleteFile()));

	editActions[menu_edit_wallpaper] = new QAction(tr("&Wallpaper"), this);
	editActions[menu_edit_wallpaper]->setStatusTip(tr("set the current image as wallpaper"));
	connect(editActions[menu_edit_wallpaper], SIGNAL(triggered()), this, SLOT(setWallpaper()));

	editActions[menu_edit_shortcuts] = new QAction(tr("&Keyboard Shortcuts"), this);
	editActions[menu_edit_shortcuts]->setShortcut(QKeySequence(shortcut_shortcuts));
	editActions[menu_edit_shortcuts]->setStatusTip(tr("lets you customize your keyboard shortcuts"));
	connect(editActions[menu_edit_shortcuts], SIGNAL(triggered()), this, SLOT(openKeyboardShortcuts()));

	editActions[menu_edit_preferences] = new QAction(tr("&Settings"), this);
	editActions[menu_edit_preferences]->setShortcut(QKeySequence(shortcut_settings));
	editActions[menu_edit_preferences]->setStatusTip(tr("settings"));
	connect(editActions[menu_edit_preferences], SIGNAL(triggered()), this, SLOT(openSettings()));

	// view menu
	panelActions.resize(menu_panel_end);
	panelActions[menu_panel_menu] = new QAction(tr("&Menu"), this);
	panelActions[menu_panel_menu]->setStatusTip(tr("Hides the Menu and Shows it Again on ALT"));
	panelActions[menu_panel_menu]->setCheckable(true);
	connect(panelActions[menu_panel_menu], SIGNAL(toggled(bool)), this, SLOT(showMenuBar(bool)));

	panelActions[menu_panel_toolbar] = new QAction(tr("Tool&bar"), this);
	panelActions[menu_panel_toolbar]->setShortcut(QKeySequence(shortcut_show_toolbar));
	panelActions[menu_panel_toolbar]->setStatusTip(tr("Show Toolbar"));
	panelActions[menu_panel_toolbar]->setCheckable(true);
	connect(panelActions[menu_panel_toolbar], SIGNAL(toggled(bool)), this, SLOT(showToolbar(bool)));

	panelActions[menu_panel_statusbar] = new QAction(tr("&Statusbar"), this);
	panelActions[menu_panel_statusbar]->setShortcut(QKeySequence(shortcut_show_statusbar));
	panelActions[menu_panel_statusbar]->setStatusTip(tr("Show Statusbar"));
	panelActions[menu_panel_statusbar]->setCheckable(true);
	connect(panelActions[menu_panel_statusbar], SIGNAL(toggled(bool)), this, SLOT(showStatusBar(bool)));

	// Added by fabian - for transferfunction:
	panelActions[menu_panel_transfertoolbar] = new QAction(tr("&Pseudocolor Function"), this);
	panelActions[menu_panel_transfertoolbar]->setShortcut(QKeySequence(shortcut_show_transfer));
	panelActions[menu_panel_transfertoolbar]->setStatusTip(tr("Show Pseudocolor Function"));
	panelActions[menu_panel_transfertoolbar]->setCheckable(true);
	panelActions[menu_panel_transfertoolbar]->setChecked(false);
	connect(panelActions[menu_panel_transfertoolbar], SIGNAL(toggled(bool)), this, SLOT(setContrast(bool)));

	panelActions[menu_panel_overview] = new QAction(tr("O&verview"), this);
	panelActions[menu_panel_overview]->setShortcut(QKeySequence(shortcut_show_overview));
	panelActions[menu_panel_overview]->setStatusTip(tr("Shows the Zoom Overview"));
	panelActions[menu_panel_overview]->setCheckable(true);
	panelActions[menu_panel_overview]->setChecked(DkSettings::app.showOverview.testBit(DkSettings::app.currentAppMode));
	connect(panelActions[menu_panel_overview], SIGNAL(toggled(bool)), vp->getController(), SLOT(showOverview(bool)));

	panelActions[menu_panel_player] = new QAction(tr("Pla&yer"), this);
	panelActions[menu_panel_player]->setShortcut(QKeySequence(shortcut_show_player));
	panelActions[menu_panel_player]->setStatusTip(tr("Shows the Slide Show Player"));
	panelActions[menu_panel_player]->setCheckable(true);
	connect(panelActions[menu_panel_player], SIGNAL(toggled(bool)), vp->getController(), SLOT(showPlayer(bool)));

	panelActions[menu_panel_explorer] = new QAction(tr("File &Explorer"), this);
	panelActions[menu_panel_explorer]->setShortcut(QKeySequence(shortcut_show_explorer));
	panelActions[menu_panel_explorer]->setStatusTip(tr("Show File Explorer"));
	panelActions[menu_panel_explorer]->setCheckable(true);
	connect(panelActions[menu_panel_explorer], SIGNAL(toggled(bool)), this, SLOT(showExplorer(bool)));

	panelActions[menu_panel_preview] = new QAction(tr("&Thumbnails"), this);
	panelActions[menu_panel_preview]->setShortcut(QKeySequence(shortcut_open_preview));
	panelActions[menu_panel_preview]->setStatusTip(tr("Show Thumbnails"));
	panelActions[menu_panel_preview]->setCheckable(true);
	connect(panelActions[menu_panel_preview], SIGNAL(toggled(bool)), vp->getController(), SLOT(showPreview(bool)));

	panelActions[menu_panel_thumbview] = new QAction(tr("&Thumbnail Preview"), this);
	panelActions[menu_panel_thumbview]->setShortcut(QKeySequence(shortcut_open_thumbview));
	panelActions[menu_panel_thumbview]->setStatusTip(tr("Show Thumbnails Preview"));
	panelActions[menu_panel_thumbview]->setCheckable(true);
	connect(panelActions[menu_panel_thumbview], SIGNAL(toggled(bool)), vp->getController(), SLOT(showThumbView(bool)));

	panelActions[menu_panel_scroller] = new QAction(tr("&Folder Scrollbar"), this);
	panelActions[menu_panel_scroller]->setShortcut(QKeySequence(shortcut_show_scroller));
	panelActions[menu_panel_scroller]->setStatusTip(tr("Show Folder Scrollbar"));
	panelActions[menu_panel_scroller]->setCheckable(true);
	connect(panelActions[menu_panel_scroller], SIGNAL(toggled(bool)), vp->getController(), SLOT(showScroller(bool)));

	panelActions[menu_panel_exif] = new QAction(tr("&Metadata"), this);
	panelActions[menu_panel_exif]->setShortcut(QKeySequence(shortcut_show_exif));
	panelActions[menu_panel_exif]->setStatusTip(tr("Shows the Metadata Panel"));
	panelActions[menu_panel_exif]->setCheckable(true);
	connect(panelActions[menu_panel_exif], SIGNAL(toggled(bool)), vp->getController(), SLOT(showMetaData(bool)));

	panelActions[menu_panel_info] = new QAction(tr("File &Info"), this);
	panelActions[menu_panel_info]->setShortcut(QKeySequence(shortcut_show_info));
	panelActions[menu_panel_info]->setStatusTip(tr("Shows the Info Panel"));
	panelActions[menu_panel_info]->setCheckable(true);
	connect(panelActions[menu_panel_info], SIGNAL(toggled(bool)), vp->getController(), SLOT(showFileInfo(bool)));

	panelActions[menu_panel_histogram] = new QAction(tr("&Histogram"), this);
	panelActions[menu_panel_histogram]->setShortcut(QKeySequence(shortcut_show_histogram));
	panelActions[menu_panel_histogram]->setStatusTip(tr("Shows the Histogram Panel"));
	panelActions[menu_panel_histogram]->setCheckable(true);
	connect(panelActions[menu_panel_histogram], SIGNAL(toggled(bool)), vp->getController(), SLOT(showHistogram(bool)));

	viewActions.resize(menu_view_end);
	viewActions[menu_view_fit_frame] = new QAction(tr("&Fit Window"), this);
	viewActions[menu_view_fit_frame]->setShortcut(QKeySequence(shortcut_fit_frame));
	viewActions[menu_view_fit_frame]->setStatusTip(tr("Fit window to the image"));
	connect(viewActions[menu_view_fit_frame], SIGNAL(triggered()), this, SLOT(fitFrame()));

	QList<QKeySequence> scs;
	scs.append(shortcut_full_screen_ff);
	scs.append(shortcut_full_screen_ad);
	viewActions[menu_view_fullscreen] = new QAction(viewIcons[icon_view_fullscreen], tr("Fu&ll Screen"), this);
	viewActions[menu_view_fullscreen]->setShortcuts(scs);
	viewActions[menu_view_fullscreen]->setStatusTip(tr("Full Screen"));
	connect(viewActions[menu_view_fullscreen], SIGNAL(triggered()), this, SLOT(enterFullScreen()));

	viewActions[menu_view_reset] = new QAction(viewIcons[icon_view_reset], tr("&Reset Canvas"), this);
	viewActions[menu_view_reset]->setShortcut(QKeySequence(shortcut_reset_view));
	viewActions[menu_view_reset]->setStatusTip(tr("Shows the initial view (no zooming)"));
	connect(viewActions[menu_view_reset], SIGNAL(triggered()), vp, SLOT(resetView()));

	viewActions[menu_view_100] = new QAction(viewIcons[icon_view_100], tr("Show &100%"), this);
	viewActions[menu_view_100]->setShortcut(QKeySequence(shortcut_zoom_full));
	viewActions[menu_view_100]->setStatusTip(tr("Shows the image at 100%"));
	connect(viewActions[menu_view_100], SIGNAL(triggered()), vp, SLOT(fullView()));

	viewActions[menu_view_zoom_in] = new QAction(tr("Zoom &In"), this);
	viewActions[menu_view_zoom_in]->setShortcut(QKeySequence::ZoomIn);
	viewActions[menu_view_zoom_in]->setStatusTip(tr("zoom in"));
	connect(viewActions[menu_view_zoom_in], SIGNAL(triggered()), vp, SLOT(zoomIn()));

	viewActions[menu_view_zoom_out] = new QAction(tr("&Zoom Out"), this);
	viewActions[menu_view_zoom_out]->setShortcut(QKeySequence::ZoomOut);
	viewActions[menu_view_zoom_out]->setStatusTip(tr("zoom out"));
	connect(viewActions[menu_view_zoom_out], SIGNAL(triggered()), vp, SLOT(zoomOut()));

	viewActions[menu_view_anti_aliasing] = new QAction(tr("&Anti Aliasing"), this);
	viewActions[menu_view_anti_aliasing]->setShortcut(QKeySequence(shortcut_anti_aliasing));
	viewActions[menu_view_anti_aliasing]->setStatusTip(tr("if checked images are smoother"));
	viewActions[menu_view_anti_aliasing]->setCheckable(true);
	viewActions[menu_view_anti_aliasing]->setChecked(DkSettings::display.antiAliasing);
	connect(viewActions[menu_view_anti_aliasing], SIGNAL(toggled(bool)), vp->getImageStorage(), SLOT(antiAliasingChanged(bool)));

	viewActions[menu_view_tp_pattern] = new QAction(tr("&Transparency Pattern"), this);
	viewActions[menu_view_tp_pattern]->setShortcut(QKeySequence(shortcut_tp_pattern));
	viewActions[menu_view_tp_pattern]->setStatusTip(tr("if checked, a pattern will be displayed for transparent objects"));
	viewActions[menu_view_tp_pattern]->setCheckable(true);
	viewActions[menu_view_tp_pattern]->setChecked(DkSettings::display.tpPattern);
	connect(viewActions[menu_view_tp_pattern], SIGNAL(toggled(bool)), vp, SLOT(togglePattern(bool)));

	viewActions[menu_view_frameless] = new QAction(tr("&Frameless"), this);
	viewActions[menu_view_frameless]->setShortcut(QKeySequence(shortcut_frameless));
	viewActions[menu_view_frameless]->setStatusTip(tr("shows a frameless window"));
	viewActions[menu_view_frameless]->setCheckable(true);
	viewActions[menu_view_frameless]->setChecked(false);
	connect(viewActions[menu_view_frameless], SIGNAL(toggled(bool)), this, SLOT(setFrameless(bool)));

	viewActions[menu_view_opacity_change] = new QAction(tr("&Change Opacity"), this);
	viewActions[menu_view_opacity_change]->setShortcut(QKeySequence(shortcut_opacity_change));
	viewActions[menu_view_opacity_change]->setStatusTip(tr("change the window opacity"));
	connect(viewActions[menu_view_opacity_change], SIGNAL(triggered()), this, SLOT(showOpacityDialog()));

	viewActions[menu_view_opacity_up] = new QAction(tr("Opacity &Up"), this);
	viewActions[menu_view_opacity_up]->setShortcut(QKeySequence(shortcut_opacity_up));
	viewActions[menu_view_opacity_up]->setStatusTip(tr("changes the window opacity"));
	connect(viewActions[menu_view_opacity_up], SIGNAL(triggered()), this, SLOT(opacityUp()));

	viewActions[menu_view_opacity_down] = new QAction(tr("Opacity &Down"), this);
	viewActions[menu_view_opacity_down]->setShortcut(QKeySequence(shortcut_opacity_down));
	viewActions[menu_view_opacity_down]->setStatusTip(tr("changes the window opacity"));
	connect(viewActions[menu_view_opacity_down], SIGNAL(triggered()), this, SLOT(opacityDown()));

	viewActions[menu_view_opacity_an] = new QAction(tr("To&ggle Opacity"), this);
	viewActions[menu_view_opacity_an]->setShortcut(QKeySequence(shortcut_an_opacity));
	viewActions[menu_view_opacity_an]->setStatusTip(tr("toggle the window opacity"));
	connect(viewActions[menu_view_opacity_an], SIGNAL(triggered()), this, SLOT(animateChangeOpacity()));

	viewActions[menu_view_lock_window] = new QAction(tr("Lock &Window"), this);
	viewActions[menu_view_lock_window]->setShortcut(QKeySequence(shortcut_lock_window));
	viewActions[menu_view_lock_window]->setStatusTip(tr("lock the window"));
	viewActions[menu_view_lock_window]->setCheckable(true);
	viewActions[menu_view_lock_window]->setChecked(false);
	connect(viewActions[menu_view_lock_window], SIGNAL(triggered(bool)), this, SLOT(lockWindow(bool)));

	viewActions[menu_view_movie_pause] = new QAction(viewIcons[icon_view_movie_play], tr("&Pause Movie"), this);
	viewActions[menu_view_movie_pause]->setStatusTip(tr("pause the current movie"));
	viewActions[menu_view_movie_pause]->setCheckable(true);
	viewActions[menu_view_movie_pause]->setChecked(false);
	connect(viewActions[menu_view_movie_pause], SIGNAL(triggered(bool)), vp, SLOT(pauseMovie(bool)));

	viewActions[menu_view_movie_prev] = new QAction(viewIcons[icon_view_movie_prev], tr("P&revious Frame"), this);
	viewActions[menu_view_movie_prev]->setStatusTip(tr("show previous frame"));
	connect(viewActions[menu_view_movie_prev], SIGNAL(triggered()), vp, SLOT(previousMovieFrame()));

	viewActions[menu_view_movie_next] = new QAction(viewIcons[icon_view_movie_next], tr("&Next Frame"), this);
	viewActions[menu_view_movie_next]->setStatusTip(tr("show next frame"));
	connect(viewActions[menu_view_movie_next], SIGNAL(triggered()), vp, SLOT(nextMovieFrame()));

	viewActions[menu_view_gps_map] = new QAction(viewIcons[icon_view_gps], tr("Show G&PS Coordinates"), this);
	viewActions[menu_view_gps_map]->setStatusTip(tr("shows the GPS coordinates"));
	viewActions[menu_view_gps_map]->setEnabled(false);
	connect(viewActions[menu_view_gps_map], SIGNAL(triggered()), this, SLOT(showGpsCoordinates()));
	
	// batch actions
	toolsActions.resize(menu_tools_end);

	toolsActions[menu_tools_thumbs] = new QAction(tr("Compute &Thumbnails"), this);
	toolsActions[menu_tools_thumbs]->setStatusTip(tr("compute all thumbnails of the current folder"));
	toolsActions[menu_tools_thumbs]->setEnabled(false);
	connect(toolsActions[menu_tools_thumbs], SIGNAL(triggered()), this, SLOT(computeThumbsBatch()));

	toolsActions[menu_tools_filter] = new QAction(fileIcons[icon_file_filter], tr("&Filter"), this);
	toolsActions[menu_tools_filter]->setStatusTip(tr("Find an image"));
	toolsActions[menu_tools_filter]->setCheckable(true);
	toolsActions[menu_tools_filter]->setChecked(false);
	connect(toolsActions[menu_tools_filter], SIGNAL(triggered(bool)), this, SLOT(find(bool)));

	toolsActions[menu_tools_manipulation] = new QAction(toolsIcons[icon_tools_manipulation], tr("Image &Manipulation"), this);
	toolsActions[menu_tools_manipulation]->setShortcut(shortcut_manipulation);
	toolsActions[menu_tools_manipulation]->setStatusTip(tr("modify the current image"));
	connect(toolsActions[menu_tools_manipulation], SIGNAL(triggered()), this, SLOT(openImgManipulationDialog()));

	toolsActions[menu_tools_export_tiff] = new QAction(tr("Export Multipage &TIFF"), this);
	toolsActions[menu_tools_export_tiff]->setStatusTip(tr("Export TIFF pages to multiple tiff files"));
	connect(toolsActions[menu_tools_export_tiff], SIGNAL(triggered()), this, SLOT(exportTiff()));

	toolsActions[menu_tools_mosaic] = new QAction(tr("&Mosaic Image"), this);
	toolsActions[menu_tools_mosaic]->setStatusTip(tr("Create a Mosaic Image"));
	connect(toolsActions[menu_tools_mosaic], SIGNAL(triggered()), this, SLOT(computeMosaic()));
	// plugins menu
	pluginsActions.resize(menu_plugins_end);
	pluginsActions[menu_plugin_manager] = new QAction(tr("&Plugin manager"), this);
	pluginsActions[menu_plugin_manager]->setStatusTip(tr("manage installed plugins and download new ones"));
	connect(pluginsActions[menu_plugin_manager], SIGNAL(triggered()), this, SLOT(openPluginManager()));
	// help menu
	helpActions.resize(menu_help_end);
	helpActions[menu_help_about] = new QAction(tr("&About Nomacs"), this);
	helpActions[menu_help_about]->setShortcut(QKeySequence(shortcut_show_help));
	helpActions[menu_help_about]->setStatusTip(tr("about"));
	connect(helpActions[menu_help_about], SIGNAL(triggered()), this, SLOT(aboutDialog()));

	helpActions[menu_help_documentation] = new QAction(tr("&Documentation"), this);
	helpActions[menu_help_documentation]->setStatusTip(tr("Online Documentation"));
	connect(helpActions[menu_help_documentation], SIGNAL(triggered()), this, SLOT(openDocumentation()));

	helpActions[menu_help_bug] = new QAction(tr("&Report a Bug"), this);
	helpActions[menu_help_bug]->setStatusTip(tr("Report a Bug"));
	connect(helpActions[menu_help_bug], SIGNAL(triggered()), this, SLOT(bugReport()));

	helpActions[menu_help_feature] = new QAction(tr("&Feature Request"), this);
	helpActions[menu_help_feature]->setStatusTip(tr("Feature Request"));
	connect(helpActions[menu_help_feature], SIGNAL(triggered()), this, SLOT(featureRequest()));

	helpActions[menu_help_update] = new QAction(tr("&Check for Updates"), this);
	helpActions[menu_help_update]->setStatusTip(tr("check for updates"));
	connect(helpActions[menu_help_update], SIGNAL(triggered()), this, SLOT(checkForUpdate()));

	helpActions[menu_help_update_translation] = new QAction(tr("&Update Translation"), this);
	helpActions[menu_help_update_translation]->setStatusTip(tr("Checks for a new version of the translations of the current language"));
	connect(helpActions[menu_help_update_translation], SIGNAL(triggered()), this, SLOT(updateTranslations()));

	assignCustomShortcuts(fileActions);
	assignCustomShortcuts(editActions);
	assignCustomShortcuts(viewActions);
	assignCustomShortcuts(panelActions);
	assignCustomShortcuts(toolsActions);
	assignCustomShortcuts(helpActions);
}

void DkNoMacs::assignCustomShortcuts(QVector<QAction*> actions) {

	QSettings settings;
	settings.beginGroup("CustomShortcuts");

	for (int idx = 0; idx < actions.size(); idx++) {
		QString val = settings.value(actions[idx]->text(), "no-shortcut").toString();

		if (val != "no-shortcut")
			actions[idx]->setShortcut(val);

		// assign widget shortcuts to all of them
		actions[idx]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	}
}

void DkNoMacs::createShortcuts() {

	DkViewPort* vp = viewport();

	shortcuts.resize(sc_end);

	shortcuts[sc_test_img] = new QShortcut(shortcut_test_img, this);
	QObject::connect(shortcuts[sc_test_img], SIGNAL( activated ()), vp, SLOT( loadLena() ));

	shortcuts[sc_test_rec] = new QShortcut(shortcut_test_rec, this);
	QObject::connect(shortcuts[sc_test_rec], SIGNAL( activated ()), this, SLOT( loadRecursion() ));

	for (int idx = 0; idx < shortcuts.size(); idx++) {

		// assign widget shortcuts to all of them
		shortcuts[idx]->setContext(Qt::WidgetWithChildrenShortcut);
	}
}

void DkNoMacs::enableNoImageActions(bool enable) {

	fileActions[menu_file_save]->setEnabled(enable);
	fileActions[menu_file_save_as]->setEnabled(enable);
	fileActions[menu_file_save_web]->setEnabled(enable);
	fileActions[menu_file_rename]->setEnabled(enable);
	fileActions[menu_file_print]->setEnabled(enable);
	fileActions[menu_file_reload]->setEnabled(enable);
	fileActions[menu_file_prev]->setEnabled(enable);
	fileActions[menu_file_next]->setEnabled(enable);
	fileActions[menu_file_goto]->setEnabled(enable);

	editActions[menu_edit_rotate_cw]->setEnabled(enable);
	editActions[menu_edit_rotate_ccw]->setEnabled(enable);
	editActions[menu_edit_rotate_180]->setEnabled(enable);
	editActions[menu_edit_delete]->setEnabled(enable);
	editActions[menu_edit_transform]->setEnabled(enable);
	editActions[menu_edit_crop]->setEnabled(enable);
	editActions[menu_edit_copy]->setEnabled(enable);
	editActions[menu_edit_copy_buffer]->setEnabled(enable);
	editActions[menu_edit_copy_color]->setEnabled(enable);
	editActions[menu_edit_wallpaper]->setEnabled(enable);
	editActions[menu_edit_flip_h]->setEnabled(enable);
	editActions[menu_edit_flip_v]->setEnabled(enable);
	editActions[menu_edit_norm]->setEnabled(enable);
	editActions[menu_edit_auto_adjust]->setEnabled(enable);
#ifdef WITH_OPENCV
	editActions[menu_edit_unsharp]->setEnabled(enable);
#else
	editActions[menu_edit_unsharp]->setEnabled(false);
#endif
	editActions[menu_edit_invert]->setEnabled(enable);

	toolsActions[menu_tools_thumbs]->setEnabled(enable);
	
	panelActions[menu_panel_info]->setEnabled(enable);
#ifdef WITH_OPENCV
	panelActions[menu_panel_histogram]->setEnabled(enable);
#else
	panelActions[menu_panel_histogram]->setEnabled(false);
#endif
	panelActions[menu_panel_preview]->setEnabled(enable);
	panelActions[menu_panel_scroller]->setEnabled(enable);
	panelActions[menu_panel_exif]->setEnabled(enable);
	panelActions[menu_panel_overview]->setEnabled(enable);
	panelActions[menu_panel_player]->setEnabled(enable);
	
	viewActions[menu_view_fullscreen]->setEnabled(enable);
	viewActions[menu_view_reset]->setEnabled(enable);
	viewActions[menu_view_100]->setEnabled(enable);
	viewActions[menu_view_fit_frame]->setEnabled(enable);
	viewActions[menu_view_zoom_in]->setEnabled(enable);
	viewActions[menu_view_zoom_out]->setEnabled(enable);

#ifdef WITH_OPENCV
	toolsActions[menu_tools_manipulation]->setEnabled(enable);
#else
	toolsActions[menu_tools_manipulation]->setEnabled(false);
#endif

	QList<QAction* > actions = openWithMenu->actions();
	for (int idx = 0; idx < actions.size()-1; idx++)
		actions.at(idx)->setEnabled(enable);

}

void DkNoMacs::enableMovieActions(bool enable) {

	viewActions[menu_view_movie_pause]->setEnabled(enable);
	viewActions[menu_view_movie_prev]->setEnabled(enable);
	viewActions[menu_view_movie_next]->setEnabled(enable);

	viewActions[menu_view_movie_pause]->setChecked(false);
	
	if (enable)
		addToolBar(movieToolbar);
	else
		removeToolBar(movieToolbar);
	
	if (toolbar->isVisible())
		movieToolbar->setVisible(enable);
}

void DkNoMacs::clearFileHistory() {
	DkSettings::global.recentFiles.clear();
}

void DkNoMacs::clearFolderHistory() {
	DkSettings::global.recentFolders.clear();
}


DkViewPort* DkNoMacs::viewport() {
	return (DkViewPort*)centralWidget();
}

void DkNoMacs::updateAll() {

	QWidgetList w = QApplication::topLevelWidgets();
	for (int idx = 0; idx < w.size(); idx++) {
		if (w[idx]->objectName().contains(QString("DkNoMacs")))
			w[idx]->update();
	}
}

QWidget* DkNoMacs::getDialogParent() {

	QWidget* w = 0;

	QWidgetList wList = QApplication::topLevelWidgets();
	for (int idx = 0; idx < wList.size(); idx++) {
		if (wList[idx]->objectName().contains(QString("DkNoMacs")))
			return wList[idx];
	}

	return 0;
}


// Qt how-to
void DkNoMacs::closeEvent(QCloseEvent *event) {

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

	if (saveSettings) {
		QSettings settings;
		settings.setValue("geometry", saveGeometry());
		settings.setValue("windowState", saveState());
		
		if (explorer)
			settings.setValue("explorerLocation", QMainWindow::dockWidgetArea(explorer));
		
		DkSettings::save();
	}

	QMainWindow::closeEvent(event);
}

void DkNoMacs::resizeEvent(QResizeEvent *event) {

	QMainWindow::resizeEvent(event);
	
	if (!overlaid)
		oldGeometry = geometry();
	else if (windowOpacity() < 1.0f) {
		animateChangeOpacity();
		overlaid = false;
	}

}

void DkNoMacs::moveEvent(QMoveEvent *event) {

	QMainWindow::moveEvent(event);

	if (!overlaid)
		oldGeometry = geometry();
	else if (windowOpacity() < 1.0f) {
		animateChangeOpacity();
		overlaid = false;
	}
}

void DkNoMacs::mouseDoubleClickEvent(QMouseEvent* event) {

	if (event->button() != Qt::LeftButton || viewport() && viewport()->getImage().isNull())
		return;

	if (isFullScreen())
		exitFullScreen();
	else
		enterFullScreen();

	//QMainWindow::mouseDoubleClickEvent(event);
}


void DkNoMacs::mousePressEvent(QMouseEvent* event) {

	mousePos = event->pos();

	//QMainWindow::mousePressEvent(event);
}

void DkNoMacs::mouseReleaseEvent(QMouseEvent *event) {

	//QMainWindow::mouseReleaseEvent(event);
}

void DkNoMacs::contextMenuEvent(QContextMenuEvent *event) {

	QMainWindow::contextMenuEvent(event);

	if (!event->isAccepted())
		contextMenu->exec(event->globalPos());
}

void DkNoMacs::mouseMoveEvent(QMouseEvent *event) {

	int dist = QPoint(event->pos()-mousePos).manhattanLength();

	if (event->buttons() == Qt::LeftButton 
			&& dist > QApplication::startDragDistance()
			&& viewport() 
			&& viewport()->imageInside()
			&& !viewport()->getImage().isNull()
			&& viewport()->getImageLoader()) {


			qDebug() << viewport()->getImageLoader()->file().absoluteFilePath();

			// TODO: check if we do it correct (network locations that are not mounted)
			QUrl fileUrl = QUrl::fromLocalFile(viewport()->getImageLoader()->file().absoluteFilePath());

			// TODO: we cannot drag&drop files with # in them because # starts a new fragment
			QList<QUrl> urls;
			urls.append(fileUrl);
			
			// who deletes me?
			QMimeData* mimeData = new QMimeData;
			
			if (viewport()->getImageLoader()->file().exists() && !viewport()->getImageLoader()->isEdited())
				mimeData->setUrls(urls);
			else if (!viewport()->getImage().isNull())
				mimeData->setImageData(viewport()->getImage());

			QDrag* drag = new QDrag(this);
			drag->setMimeData(mimeData);
			Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
			qDebug() << "creating drag: " << fileUrl;
	}

	//QMainWindow::mouseMoveEvent(event);
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
			vp->zoom(pinchG->scaleFactor());
		}
		else if (pinchG->rotationAngle() != 0 && vp) {

			float angle = pinchG->rotationAngle();

			
			qDebug() << "angle: " << angle;

			//vp->rotate(angle);

		}
	}

	qDebug() << "gesture event (NoMacs)";

	//	pinchTriggered(static_cast<QPinchGesture *>(pinch));
	return true;
}

void DkNoMacs::dragMoveEvent(QDragMoveEvent *event) {
}

void DkNoMacs::dragLeaveEvent(QDragLeaveEvent *event) {

	qDebug() << "";
	event->accept();	
}

void DkNoMacs::dragEnterEvent(QDragEnterEvent *event) {

	printf("drag enter event\n");

	//if (event->source() == this)
	//	return;

	if (event->mimeData()->hasFormat("network/sync-dir")) {
		event->accept();
	}
	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
		
		// TODO: check if we accept appropriately (network drives that are not mounted)
		qDebug() << url;
		QFileInfo file = QFileInfo(url.toString());

		// just accept image files
		if (DkImageLoader::isValid(file))
			event->acceptProposedAction();
		else if (file.isDir())
			event->acceptProposedAction();
	}
	if (event->mimeData()->hasImage()) {
		event->acceptProposedAction();
	}

	QMainWindow::dragEnterEvent(event);

}

void DkNoMacs::dropEvent(QDropEvent *event) {

	if (event->source() == this) {
		event->accept();
		return;
	}

	if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
		QUrl url = event->mimeData()->urls().at(0);
		qDebug() << "dropping: " << url;
		url = url.toLocalFile();
		
		viewport()->loadFile(QFileInfo(url.toString()));

		QList<QUrl> urls = event->mimeData()->urls();
		for (int idx = 1; idx < urls.size() && idx < 20; idx++)
			newInstance(QFileInfo(urls[idx].toLocalFile()));
		
	}
	else if (event->mimeData()->hasImage()) {

		QImage dropImg = qvariant_cast<QImage>(event->mimeData()->imageData());
		viewport()->loadImage(dropImg);
	}

	qDebug() << "drop event...";
}

void DkNoMacs::copyImage() {
	
	qDebug() << "copying...";

	if (!viewport() || viewport()->getImage().isNull() || !viewport()->getImageLoader())
		return;

	QUrl fileUrl = QUrl("file:///" + viewport()->getImageLoader()->file().absoluteFilePath());
	
	QList<QUrl> urls;
	urls.append(fileUrl);

	QMimeData* mimeData = new QMimeData;
	
	if (viewport()->getImageLoader()->file().exists() && !viewport()->getImageLoader()->isEdited())
		mimeData->setUrls(urls);
	else if (!viewport()->getImage().isNull())
		mimeData->setImageData(viewport()->getImage());
	
	mimeData->setText(viewport()->getImageLoader()->file().absoluteFilePath());

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);

	qDebug() << "copying: " << fileUrl;
}

void DkNoMacs::copyPixelColorValue() {
	
	if (!viewport() || viewport()->getImage().isNull())
		return;

	QMimeData* mimeData = new QMimeData;

	if (!viewport()->getImage().isNull())
		mimeData->setText(viewport()->getCurrentPixelHexValue());

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);
}

void DkNoMacs::copyImageBuffer() {

	qDebug() << "copying...";

	if (!viewport() || viewport()->getImage().isNull())
		return;

	QMimeData* mimeData = new QMimeData;

	if (!viewport()->getImage().isNull())
		mimeData->setImageData(viewport()->getImage());

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);

}

void DkNoMacs::pasteImage() {
	
	qDebug() << "pasting...";

	if(!getCurrRunningPlugin().isEmpty()) applyPluginChanges(true, false);

	QClipboard* clipboard = QApplication::clipboard();

	if (clipboard->mimeData()->hasUrls() && clipboard->mimeData()->urls().size() > 0) {
		QUrl url = clipboard->mimeData()->urls().at(0);
		url = url.toLocalFile();
		qDebug() << "pasting: " << url.toString();
		viewport()->loadFile(QFileInfo(url.toString()));

	}
	else if (clipboard->mimeData()->hasImage()) {

		QImage dropImg = qvariant_cast<QImage>(clipboard->mimeData()->imageData());

		if (viewport())
			viewport()->loadImage(dropImg);

	} 
	else if (clipboard->mimeData()->hasText()) {

		QString msg = clipboard->mimeData()->text();
		QFileInfo file = QFileInfo(msg);

		if (file.exists()) {
			viewport()->loadFile(file);
		}
		else
			viewport()->getController()->setInfo("Could not find a valid file url, sorry");
	}
	else if (viewport())
		viewport()->getController()->setInfo("Clipboard has no image...");

}

void DkNoMacs::flipImageHorizontal() {

	DkViewPort* vp = viewport();

	if (!vp)
		return;

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

	QImage img = vp->getImage();
	img.invertPixels();

	if (img.isNull())
		vp->getController()->setInfo(tr("Sorry, I cannot Invert the Image..."));
	else
		vp->setEditedImage(img);

}

void DkNoMacs::normalizeImage() {

	DkViewPort* vp = viewport();

	if (!vp)
		return;

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

	QImage img = vp->getImage();

	bool normalized = DkImage::autoAdjustImage(img);

	if (!normalized || img.isNull())
		vp->getController()->setInfo(tr("Sorry, I cannot Auto Adjust"));
	else
		vp->setEditedImage(img);
}

void DkNoMacs::unsharpMask() {
#ifdef WITH_OPENCV
	DkUnsharpDialog* unsharpDialog = new DkUnsharpDialog(this);
	unsharpDialog->setImage(viewport()->getImage());
	bool answer = unsharpDialog->exec();
	if (answer == QDialog::Accepted) {
		QImage editedImage = unsharpDialog->getImage();
		viewport()->setEditedImage(editedImage);
	}

	unsharpDialog->deleteLater();
#endif
}

void DkNoMacs::readSettings() {
	
	qDebug() << "reading settings...";
	QSettings settings;
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
}

void DkNoMacs::restart() {
	
	if (!viewport()) 
		return;

	QString exe = QApplication::applicationFilePath();
	QStringList args;
	args.append(viewport()->getImageLoader()->file().absoluteFilePath());

	bool started = process.startDetached(exe, args);

	// close me if the new instance started
	if (started)
		close();
}

void DkNoMacs::enterFullScreen() {
	
	// switch off fullscreen if it's in it already
	if (isFullScreen()) {
		exitFullScreen();
		return;
	}

	DkSettings::app.currentAppMode += DkSettings::mode_end*0.5f;
	if (DkSettings::app.currentAppMode < 0) {
		qDebug() << "illegal state: " << DkSettings::app.currentAppMode;
		DkSettings::app.currentAppMode = DkSettings::mode_default;
	}
	
	menuBar()->hide();
	toolbar->hide();
	movieToolbar->hide();
	statusbar->hide();
	showFullScreen();

	if (viewport())
		viewport()->setFullScreen(true);

	update();
}

void DkNoMacs::exitFullScreen() {

	if (isFullScreen()) {
		DkSettings::app.currentAppMode -= DkSettings::mode_end*0.5f;
		if (DkSettings::app.currentAppMode < 0) {
			qDebug() << "illegal state: " << DkSettings::app.currentAppMode;
			DkSettings::app.currentAppMode = DkSettings::mode_default;
		}

		if (DkSettings::app.showMenuBar) menu->show();
		if (DkSettings::app.showToolBar) toolbar->show();
		if (DkSettings::app.showStatusBar) statusbar->show();
		showNormal();
		update();	// if no resize is triggered, the viewport won't change its color
	}

	if (viewport())
		viewport()->setFullScreen(false);
}

void DkNoMacs::setFrameless(bool frameless) {

	if (!viewport()) 
		return;

	QString exe = QApplication::applicationFilePath();
	QStringList args;
	args.append(viewport()->getImageLoader()->file().absoluteFilePath());
	
	if (objectName() != "DkNoMacsFrameless") {
		DkSettings::app.appMode = DkSettings::mode_frameless;
        //args.append("-graphicssystem");
        //args.append("native");
    } else {
		DkSettings::app.appMode = DkSettings::mode_default;
    }
	bool started = process.startDetached(exe, args);

	// close me if the new instance started
	if (started)
		close();

	qDebug() << "frameless arguments: " << args;
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

	// reset viewport if we did not clip -> compensates round-off errors
	if (screenRect.contains(nmRect.toRect()))
		viewport()->resetView();

}

void DkNoMacs::setRecursiveScan(bool recursive) {

	DkSettings::global.scanSubFolders = recursive;
	//viewport()->reloadFile();
	DkImageLoader* loader = viewport()->getImageLoader();
	
	if (!loader)
		return;

	if (recursive)
		viewport()->getController()->setInfo(tr("Recursive Folder Scan is Now Enabled"));
	else
		viewport()->getController()->setInfo(tr("Recursive Folder Scan is Now Disabled"));

	loader->updateSubFolders(loader->getDir());

}

void DkNoMacs::showOpacityDialog() {

	if (!opacityDialog) {
		opacityDialog = new DkOpacityDialog(this);
		opacityDialog->setWindowTitle(tr("Change Opacity"));
	}
	
	if (opacityDialog->exec())
		setWindowOpacity(opacityDialog->value()/100.0f);
}

void DkNoMacs::opacityDown() {

	changeOpacity(-0.3f);
}

void DkNoMacs::opacityUp() {
	
	changeOpacity(0.3f);
}

void DkNoMacs::changeOpacity(float change) {

	float newO = windowOpacity() + change;
	if (newO > 1) newO = 1.0f;
	if (newO < 0.1) newO = 0.1f;
	setWindowOpacity(newO);
}

void DkNoMacs::animateOpacityDown() {

	float newO = windowOpacity() - 0.03f;

	if (newO < 0.3f) {
		setWindowOpacity(0.3f);
		return;
	}

	setWindowOpacity(newO);
	QTimer::singleShot(20, this, SLOT(animateOpacityDown()));
}

void DkNoMacs::animateOpacityUp() {

	float newO = windowOpacity() + 0.03f;

	if (newO > 1.0f) {
		setWindowOpacity(1.0f);
		return;
	}

	setWindowOpacity(newO);
	QTimer::singleShot(20, this, SLOT(animateOpacityUp()));
}

// >DIR: diem - why can't we put it in viewport?
void DkNoMacs::animateChangeOpacity() {

	float newO = windowOpacity();

	if (newO >= 1.0f)
		animateOpacityDown();
	else
		animateOpacityUp();
}

void DkNoMacs::lockWindow(bool lock) {

	
#ifdef WIN32
	
	qDebug() << "locking: " << lock;

	if (lock && windowOpacity() < 1.0f) {
		//setAttribute(Qt::WA_TransparentForMouseEvents);
		HWND hwnd = (HWND) winId(); // get handle of the widget
		LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
		SetWindowLong(hwnd, GWL_EXSTYLE, styles | WS_EX_TRANSPARENT); 
		SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		viewport()->getController()->setInfo(tr("Window Locked\nTo unlock: gain focus (ALT+Tab),\nthen press CTRL+SHIFT+ALT+B"), 5000);
	}
	else if (lock && windowOpacity() == 1.0f) {
		viewport()->getController()->setInfo(tr("You should first reduce opacity\n before working through the window."));
		viewActions[menu_view_lock_window]->setChecked(false);
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

void DkNoMacs::newClientConnected(bool connected, bool local) {
	overlaid = false;
	// add methods if clients are connected

	syncActions[menu_sync]->setEnabled(connected);
	syncActions[menu_sync_pos]->setEnabled(connected);
	syncActions[menu_sync_arrange]->setEnabled(connected);

}

void DkNoMacs::tcpSetWindowRect(QRect newRect, bool opacity, bool overlaid) {

	this->overlaid = overlaid;

	DkUtils::printDebug(DK_MODULE, "arranging...\n");

	// we are currently overlaid...
	if (!overlaid) {

		setGeometry(oldGeometry);
		if (opacity)
			animateOpacityUp();
		oldGeometry = geometry();
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

		oldGeometry = geometry();
		
		this->move(newRect.topLeft());
		this->resize(newRect.size() - (frameGeometry().size() - geometry().size()));

		//setGeometry(newRect);
		if (opacity)
			animateOpacityDown();
		
		//this->setActiveWindow();
	}
};

void DkNoMacs::tcpSendWindowRect() {

	overlaid = !overlaid;

	qDebug() << "overlaying";
	// change my geometry
	tcpSetWindowRect(this->frameGeometry(), !overlaid, overlaid);

	emit sendPositionSignal(frameGeometry(), overlaid);

};

void DkNoMacs::tcpSendArrange() {
	
	overlaid = !overlaid;
	emit sendArrangeSignal(overlaid);
}

void DkNoMacs::showExplorer(bool show) {

	if (!explorer) {

		// get last location
		QSettings settings;
		int dockLocation = settings.value("explorerLocation", Qt::LeftDockWidgetArea).toInt();
		
		explorer = new DkExplorer(tr("File Explorer"));
		addDockWidget((Qt::DockWidgetArea)dockLocation, explorer);
		connect(explorer, SIGNAL(openFile(QFileInfo)), viewport()->getImageLoader(), SLOT(load(QFileInfo)));
		connect(explorer, SIGNAL(openDir(QFileInfo)), viewport()->getController()->getThumbWidget(), SLOT(setDir(QFileInfo)));
		connect(viewport()->getImageLoader(), SIGNAL(updateFileSignal(QFileInfo)), explorer, SLOT(setCurrentPath(QFileInfo)));
	}

	explorer->setVisible(show);

	if (viewport()->getImageLoader()->hasFile()) {
		explorer->setCurrentPath(viewport()->getImageLoader()->file());
	}
	else {
		QStringList folders = DkSettings::global.recentFiles;

		if (folders.size() > 0)
			explorer->setCurrentPath(folders[0]);
	}

}

void DkNoMacs::openDir() {

	DkImageLoader* loader = viewport()->getImageLoader();

	// load system default open dialog
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),
		loader->getDir().absolutePath());

	if (dirName.isEmpty())
		return;

	qDebug() << "loading directory: " << dirName;
	
	viewport()->loadFile(QFileInfo(dirName));
}

void DkNoMacs::openFile() {

	if (!viewport() || !viewport()->getImageLoader())
		return;

	DkImageLoader* loader = viewport()->getImageLoader();
	//QStringList fileNames;

	//if (!openDialog)
	//	openDialog = new QFileDialog(this);

	//openDialog->setWindowTitle(tr("Open Image"));
	//openDialog->setFilters(DkImageLoader::openFilters);
	//openDialog->setDirectory(loader->getDir());
	//openDialog->setOption(QFileDialog::DontResolveSymlinks);


	// load system default open dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"),
		loader->getDir().absolutePath(), 
		DkSettings::openFilters.join(";;"));

	//// show the dialog
	//if(openDialog->exec())
	//	fileNames = openDialog->selectedFiles();

	if (fileName.isEmpty())
		return;

	qDebug() << "os filename: " << fileName;
	viewport()->loadFile(QFileInfo(fileName));


	//if (fileNames[0].isEmpty())
	//	return;

	//qDebug() << "os filename: " << fileNames[0];
	//viewport()->loadFile(QFileInfo(fileNames[0]));
}

void DkNoMacs::loadFile(const QFileInfo& file) {

	if (!viewport())
		return;

	viewport()->loadFile(file);
}

void DkNoMacs::renameFile() {

	DkImageLoader* loader = viewport()->getImageLoader();

	if (!loader)
		return;

	QFileInfo file = loader->file();

	if (!file.absoluteDir().exists()) {
		viewport()->getController()->setInfo(tr("Sorry, the directory: %1  does not exist\n").arg(file.absolutePath()));
		return;
	}
	if (file.exists() && !file.isWritable()) {
		viewport()->getController()->setInfo(tr("Sorry, I can't write to the file: %1").arg(file.fileName()));
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
			infoDialog.setText(tr("The file: %1  already exists.\n Do you want to replace it?").arg(filename));
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
			viewport()->loadFile(renamedFile.absoluteFilePath());
		
	}

}

void DkNoMacs::find(bool filterAction) {

	if(!getCurrRunningPlugin().isEmpty()) applyPluginChanges(true, false);

	if (!viewport() || !viewport()->getImageLoader())
		return;

	if (filterAction) {

		int db = (QObject::sender() == toolsActions[menu_tools_filter]) ? DkSearchDialog::filter_button : DkSearchDialog::find_button;
		
		qDebug() << "default button: " << db;
		DkSearchDialog* searchDialog = new DkSearchDialog(this);
		searchDialog->setDefaultButton(db);
		searchDialog->setFiles(viewport()->getImageLoader()->getFileNames());
		searchDialog->setPath(viewport()->getImageLoader()->getDir());

		connect(searchDialog, SIGNAL(filterSignal(QStringList)), viewport()->getImageLoader(), SLOT(setFolderFilters(QStringList)));
		connect(searchDialog, SIGNAL(loadFileSignal(QFileInfo)), viewport(), SLOT(loadFile(QFileInfo)));
		int answer = searchDialog->exec();

		toolsActions[menu_tools_filter]->setChecked(answer == DkSearchDialog::filter_button);		
	}
	else {
		// remove the filter 
		viewport()->getImageLoader()->setFolderFilters(QStringList());
	}


}

void DkNoMacs::updateFilterState(QStringList filters) {
	
	// TODO: remove
	//qDebug() << "filters: " << filters;
	//toolsActions[menu_tools_filter]->blockSignals(true);
	//toolsActions[menu_tools_filter]->setChecked(!filters.empty());
	//toolsActions[menu_tools_filter]->blockSignals(false);
}

void DkNoMacs::changeSorting(bool change) {

	if (!change)
		return;

	
	QString senderName = QObject::sender()->objectName();
	bool modeChange = true;

	if (senderName == "menu_sort_filename")
		DkSettings::global.sortMode = DkSettings::sort_filename;
	else if (senderName == "menu_sort_date_created")
		DkSettings::global.sortMode = DkSettings::sort_date_created;
	else if (senderName == "menu_sort_date_modified")
		DkSettings::global.sortMode = DkSettings::sort_date_modified;
	else if (senderName == "menu_sort_random")
		DkSettings::global.sortMode = DkSettings::sort_random;
	else if (senderName == "menu_sort_ascending") {
		DkSettings::global.sortDir = DkSettings::sort_ascending;
		modeChange = false;
	}
	else if (senderName == "menu_sort_descending") {
		DkSettings::global.sortDir = DkSettings::sort_descending;
		modeChange = false;
	}

	if (viewport() && viewport()->getImageLoader()) 
		viewport()->getImageLoader()->sort();

	for (int idx = 0; idx < sortActions.size(); idx++) {

		if (modeChange && idx < menu_sort_ascending && sortActions.at(idx) != QObject::sender())
			sortActions[idx]->setChecked(false);
		else if (!modeChange && idx >= menu_sort_ascending && sortActions.at(idx) != QObject::sender())
			sortActions[idx]->setChecked(false);
	}
}

void DkNoMacs::goTo() {

	if(!getCurrRunningPlugin().isEmpty()) applyPluginChanges(true, false);

	if (!viewport() || !viewport()->getImageLoader())
		return;

	DkImageLoader* loader = viewport()->getImageLoader();
	
	bool ok = false;
	int fileIdx = QInputDialog::getInt(this, tr("Go To Image"), tr("Image Index:"), 0, 0, loader->numFiles()-1, 1, &ok);

	if (ok)
		loader->loadFileAt(fileIdx);

}

void DkNoMacs::trainFormat() {

	if (!viewport())
		return;

	if (!trainDialog)
		trainDialog = new DkTrainDialog(this);

	trainDialog->setCurrentFile(viewport()->getImageLoader()->file());
	bool okPressed = trainDialog->exec();

	if (okPressed) {
		viewport()->getImageLoader()->load(trainDialog->getAcceptedFile());
		restart();	// quick & dirty, but currently he messes up the filteredFileList if the same folder was already loaded
	}


}

void DkNoMacs::saveFile() {

	saveFileAs(true);
}

void DkNoMacs::saveFileAs(bool silent) {
	
	qDebug() << "saving...";

	if(!currRunningPlugin.isEmpty()) applyPluginChanges(true, true);

	DkImageLoader* loader = viewport()->getImageLoader();

	QString selectedFilter;
	QString saveName;
	QFileInfo saveFile;

	if (loader->hasFile()) {
		saveFile = loader->file();
		saveName = saveFile.fileName();
		
		qDebug() << "save dir: " << loader->getSaveDir();

		if (loader->getSaveDir() != saveFile.absoluteDir())
			saveFile = QFileInfo(loader->getSaveDir(), saveName);

		int filterIdx = -1;

		QStringList sF = DkSettings::saveFilters;
		//qDebug() << sF;

		QRegExp exp = QRegExp("*." + saveFile.suffix() + "*", Qt::CaseInsensitive);
		exp.setPatternSyntax(QRegExp::Wildcard);

		for (int idx = 0; idx < sF.size(); idx++) {

			if (exp.exactMatch(sF.at(idx))) {
				selectedFilter = sF.at(idx);
				filterIdx = idx;
				break;
			}
		}

		if (filterIdx == -1)
			saveName.remove("." + saveFile.suffix());
	}

	QString fileName;

	int answer = QDialog::Rejected;

	// don't ask the user if save was hit & the file format is supported for saving
	if (silent && !selectedFilter.isEmpty() && viewport()->getImageLoader()->isEdited()) {
		fileName = loader->file().absoluteFilePath();
		DkMessageBox* msg = new DkMessageBox(QMessageBox::Question, tr("Overwrite File"), 
			tr("Do you want to overwrite:\n%1?").arg(fileName), 
			(QMessageBox::Yes | QMessageBox::No), this);
		msg->setObjectName("overwriteDialog");

		//msg->show();
		answer = msg->exec();

	}
	if (answer == QDialog::Rejected || answer == QMessageBox::No) {
		// note: basename removes the whole file name from the first dot...
		QString savePath = (!selectedFilter.isEmpty()) ? saveFile.absoluteFilePath() : QFileInfo(saveFile.absoluteDir(), saveName).absoluteFilePath();

		fileName = QFileDialog::getSaveFileName(this, tr("Save File %1").arg(saveName),
			savePath, DkSettings::saveFilters.join(";;"), &selectedFilter);
	}


	//qDebug() << "selected Filter: " << selectedFilter;


	// uncomment if you want to use native Qt dialog -> slow! ------------------------------------	
	//if (!saveDialog)
	//	saveDialog = new QFileDialog(this);

	//saveDialog->setWindowTitle(tr("Save Image"));
	//saveDialog->setAcceptMode(QFileDialog::AcceptSave);
	//saveDialog->setFilters(DkImageLoader::saveFilters);
	//
	//if (loader->hasFile()) {
	//	saveDialog->selectNameFilter(loader->getCurrentFilter());	// select the current file filter
	//	saveDialog->setDirectory(loader->getSaveDir());
	//	saveDialog->selectFile(loader->getFile().fileName());
	//}

	//saveDialog->show();

	//// show the dialog
	//if(saveDialog->exec())
	//	fileNames = saveDialog->selectedFiles();

	//if (fileNames.empty())
	//	return;
	// Uncomment if you want to use native Qt dialog -> slow! ------------------------------------

	if (fileName.isEmpty())
		return;


	QString ext = QFileInfo(fileName).suffix();

	if (!ext.isEmpty() && !selectedFilter.contains(ext)) {

		QStringList sF = DkSettings::saveFilters;

		for (int idx = 0; idx < sF.size(); idx++) {

			if (sF.at(idx).contains(ext)) {
				selectedFilter = sF.at(idx);
				break;
			}
		}
	}


	// TODO: if more than one file is opened -> open new threads
	QFileInfo sFile = QFileInfo(fileName);
	QImage saveImg = viewport()->getImage();
	int compression = -1;	// default value

	//if (saveDialog->selectedNameFilter().contains("jpg")) {
	if (selectedFilter.contains(QRegExp("(jpg|jpeg|j2k|jp2|jpf|jpx)", Qt::CaseInsensitive))) {

		if (!jpgDialog)
			jpgDialog = new DkCompressDialog(this);

		if (selectedFilter.contains(QRegExp("(j2k|jp2|jpf|jpx)")))
			jpgDialog->setDialogMode(DkCompressDialog::j2k_dialog);
		else
			jpgDialog->setDialogMode(DkCompressDialog::jpg_dialog);

		jpgDialog->imageHasAlpha(saveImg.hasAlphaChannel());
		//jpgDialog->show();
		jpgDialog->setImage(&saveImg);
		

		if (!jpgDialog->exec())
			return;

		compression = jpgDialog->getCompression();

		if (saveImg.hasAlphaChannel()) {

			QRect imgRect = QRect(QPoint(), saveImg.size());
			QImage tmpImg = QImage(saveImg.size(), QImage::Format_RGB32);
			QPainter painter(&tmpImg);
			painter.fillRect(imgRect, jpgDialog->getBackgroundColor());
			painter.drawImage(imgRect, saveImg, imgRect);

			saveImg = tmpImg;
		}

	//	qDebug() << "returned: " << ret;
	}

	if (selectedFilter.contains("webp")) {

		if (!jpgDialog)
			jpgDialog = new DkCompressDialog(this);

		jpgDialog->setDialogMode(DkCompressDialog::webp_dialog);

		jpgDialog->setImage(&saveImg);

		if (!jpgDialog->exec())
			return;

		compression = jpgDialog->getCompression();
	}

	//if (saveDialog->selectedNameFilter().contains("tif")) {
	if (selectedFilter.contains("tif")) {

		if (!tifDialog)
			tifDialog = new DkTifDialog(this);

		if (!tifDialog->exec())
			return;

		compression = tifDialog->getCompression();
	}

	if (loader)
		loader->saveFile(sFile, saveImg, selectedFilter, compression);

}

void DkNoMacs::saveFileWeb() {

	DkImageLoader* loader = viewport()->getImageLoader();

	if (!loader)
		return;

	
	QString saveName;
	QFileInfo saveFile;

	if (loader->hasFile()) {
		saveFile = loader->file();
		saveName = saveFile.fileName();

		qDebug() << "save dir: " << loader->getSaveDir();

		if (loader->getSaveDir() != saveFile.absoluteDir())
			saveFile = QFileInfo(loader->getSaveDir(), saveName);
	}

	QImage img = viewport()->getImage();
	bool imgHasAlpha = DkImage::alphaChannelUsed(img);

	QString suffix = imgHasAlpha ? ".png" : ".jpg";
	QString saveFilterGui;

	for (int idx = 0; idx < DkSettings::saveFilters.size(); idx++) {

		if (DkSettings::saveFilters.at(idx).contains(suffix)) {
			saveFilterGui = DkSettings::saveFilters.at(idx);
			break;
		}
	}

	if (saveFile.exists())
		saveFile = QFileInfo(saveFile.absolutePath(), saveFile.baseName() + suffix);

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File %1").arg(saveName),
						saveFile.absoluteFilePath(), saveFilterGui);

	if (fileName.isEmpty())
		return;

	if (!jpgDialog)
		jpgDialog = new DkCompressDialog(this);

	jpgDialog->setDialogMode(DkCompressDialog::web_dialog);

	jpgDialog->imageHasAlpha(imgHasAlpha);
	jpgDialog->setImage(&img);

	if (!jpgDialog->exec())
		return;

	float factor = jpgDialog->getResizeFactor();
	if (factor != -1)
		img = DkImage::resizeImage(img, QSize(), factor, DkImage::ipl_area);

	loader->saveFile(fileName, img, suffix, jpgDialog->getCompression());

}

void DkNoMacs::resizeImage() {

	if(!getCurrRunningPlugin().isEmpty()) applyPluginChanges(true, false);

	if (!viewport() || viewport()->getImage().isNull())
		return;

	if (!resizeDialog)
		resizeDialog = new DkResizeDialog(this);


	QSharedPointer<DkImageContainerT> imgC = viewport()->getImageLoader()->getCurrentImage();
	QSharedPointer<DkMetaDataT> metaData;

	if (imgC) {
		metaData = imgC->getMetaData();
		QVector2D res = metaData->getResolution();
		resizeDialog->setExifDpi(res.x());
	}

	qDebug() << "resize image: " << viewport()->getImage().size();

	resizeDialog->setImage(viewport()->getImage());

	if (!resizeDialog->exec())
		return;

	if (resizeDialog->resample()) {

		//// do not load the old image -> as the transformed image is not the same anymore
		//viewport()->getImageLoader()->enableWatcher(false);
		//viewport()->getImageLoader()->clearFileWatcher();

		// TODO: redirect resize to basic loader here
		QImage rImg = resizeDialog->getResizedImage();

		if (!rImg.isNull()) {

			// this reloads the image -> that's not what we want!
			if (metaData)
				metaData->setResolution(QVector2D(resizeDialog->getExifDpi(), resizeDialog->getExifDpi()));

			viewport()->setEditedImage(rImg);
		}
	}
	else if (metaData) {
		// ok, user just wants to change the resolution
		metaData->setResolution(QVector2D(resizeDialog->getExifDpi(), resizeDialog->getExifDpi()));
	}
}

void DkNoMacs::deleteFile() {

	if(!getCurrRunningPlugin().isEmpty()) applyPluginChanges(false, false);

	if (!viewport() || viewport()->getImage().isNull())
		return;

	qDebug() << "yep deleting...";

	QFileInfo file = viewport()->getImageLoader()->file();

	if (infoDialog(tr("Do you want to permanently delete %1").arg(file.fileName()), this) == QMessageBox::Yes)
		viewport()->getImageLoader()->deleteFile();
}

void DkNoMacs::openAppManager() {

	DkAppManagerDialog* appManagerDialog = new DkAppManagerDialog(appManager, this, windowFlags());
	connect(appManagerDialog, SIGNAL(openWithSignal(QAction*)), this, SLOT(openFileWith(QAction*)));
	appManagerDialog->exec();

	appManagerDialog->deleteLater();

	openWithMenu->clear();
	createOpenWithMenu(openWithMenu);
}

void DkNoMacs::exportTiff() {

#ifdef WITH_LIBTIFF
	if (!exportTiffDialog)
		exportTiffDialog = new DkExportTiffDialog(this);

	exportTiffDialog->setFile(viewport()->getImageLoader()->file());
	
	exportTiffDialog->exec();
#endif
}

void DkNoMacs::computeMosaic() {
#ifdef WITH_OPENCV

	if(!getCurrRunningPlugin().isEmpty()) applyPluginChanges(true, false);

	//if (!mosaicDialog)
	DkMosaicDialog* mosaicDialog = new DkMosaicDialog(this, Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

	mosaicDialog->setFile(viewport()->getImageLoader()->file());

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

	if(!getCurrRunningPlugin().isEmpty()) applyPluginChanges(true, false);

	if (!viewport() || viewport()->getImage().isNull())
		return;

	if (!imgManipulationDialog)
		imgManipulationDialog = new DkImageManipulationDialog(this);
	else 
		imgManipulationDialog->resetValues();

	QImage tmpImg = viewport()->getImage();
	imgManipulationDialog->setImage(&tmpImg);

	bool ok = imgManipulationDialog->exec();

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

	//// for now - we just save the image as is
	//QRect screenRect = QApplication::desktop()->screenGeometry();

	//qDebug() << "screen rect size: " << screenRect;

	//QImage dImg;

	//if (img.size().width() > screenRect.size().width() && img.size().height() > screenRect.size().height()) {
	//	
	//	dImg = QImage(screenRect.size(), QImage::Format_ARGB32);
	//	dImg.fill(QColor(0,0,0,0).rgba());

	//	// do some scaling here - or just save the image as is?

	//	// render the image into the new coordinate system
	//	QPainter painter(&dImg);
	//	painter.drawImage(QRect(QPoint(), screenRect.size()), img, QRect(QPoint(), screenRect.size()));
	//	painter.end();
	//	qDebug() << "resizing to: " << screenRect;
	//	// TODO: rescale image here
	//	//viewport()->setImage(dImg);
	//}
	//else
	//	dImg = img;

	QImage dImg = img;
	QFileInfo tmpPath = viewport()->getImageLoader()->saveTempFile(dImg, "wallpaper", ".jpg", true);

	// is there a more elegant way to see if saveTempFile returned an empty path
	if (tmpPath.absoluteFilePath() == QFileInfo().absoluteFilePath()) {
		errorDialog(tr("Sorry, I could not create a wallpaper..."));
		return;
	}

#ifdef WIN32

//#include <stdio.h>
//#include <windows.h>

	//Read current windows background image path
	QSettings appSettings( "HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
	appSettings.setValue("Wallpaper", tmpPath.absoluteFilePath());

	QByteArray ba = tmpPath.absoluteFilePath().toLatin1();
	SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)ba.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#endif
	// TODO: add functionality for unix based systems

}

void DkNoMacs::printDialog() {

	if(!getCurrRunningPlugin().isEmpty()) applyPluginChanges(true, false);

	QPrinter printer;

	QVector2D res(72,72);
	QSharedPointer<DkImageContainerT> imgC = viewport()->getImageLoader()->getCurrentImage();
	
	if (imgC)
		res = imgC->getMetaData()->getResolution();

	//QPrintPreviewDialog* previewDialog = new QPrintPreviewDialog();
	QImage img = viewport()->getImage();
	if (!printPreviewDialog)
		printPreviewDialog = new DkPrintPreviewDialog(img, res.x(), 0, this);
	else
		printPreviewDialog->setImage(img, res.x());

	printPreviewDialog->show();
	printPreviewDialog->updateZoomFactor(); // otherwise the initial zoom factor is wrong

}

void DkNoMacs::computeThumbsBatch() {

	if (!viewport() || !viewport()->getImageLoader())
		return;

	if (!forceDialog)
		forceDialog = new DkForceThumbDialog(this);
	forceDialog->setWindowTitle(tr("Save Thumbnails"));
	forceDialog->setDir(viewport()->getImageLoader()->getDir());

	if (!forceDialog->exec())
		return;

	if (!thumbSaver)
		thumbSaver = new DkThumbsSaver(this);
	thumbSaver->processDir(viewport()->getImageLoader()->getImages(), forceDialog->forceSave());
}

void DkNoMacs::aboutDialog() {

	DkSplashScreen* spScreen = new DkSplashScreen(this, 0/*, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint*/);
	spScreen->exec();
	delete spScreen;
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
	url += "Windows XP";
#elif defined WIN32 && _WIN64
	url += "Windows Vista/7/8 64bit";
#elif defined WIN32 && _WIN32
	url += "Windows Vista/7/8 32bit";
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

	QSettings settings;
	settings.clear();

	readSettings();
	resize(400, 225);
	move(100, 100);
}

void DkNoMacs::newInstance(QFileInfo file) {

	if (!viewport()) 
		return;

	QString exe = QApplication::applicationFilePath();
	QStringList args;

	if (!file.exists())
		args.append(viewport()->getImageLoader()->file().absoluteFilePath());
	else
		args.append(file.absoluteFilePath());

	if (objectName() == "DkNoMacsFrameless")
		args.append("1");	
	
	QProcess::startDetached(exe, args);
}

//void DkNoMacs::shareFacebook() {
//	
//	DkFacebook fb;
//
//	fb.login(QString());
//	qDebug() << "login called...";
//
//}

void DkNoMacs::loadRecursion() {

	QImage img = QPixmap::grabWindow(this->winId()).toImage();
	viewport()->setImage(img);
}

// Added by fabian for transfer function:

void DkNoMacs::setContrast(bool contrast) {

	qDebug() << "contrast: " << contrast;

	if (!viewport()) 
		return;

	QString exe = QApplication::applicationFilePath();
	QStringList args;
	args.append(viewport()->getImageLoader()->file().absoluteFilePath());
	
	if (contrast)
		DkSettings::app.appMode = DkSettings::mode_contrast;
	else
		DkSettings::app.appMode = DkSettings::mode_default;

	bool started = process.startDetached(exe, args);

	// close me if the new instance started
	if (started)
		close();

	qDebug() << "contrast arguments: " << args;
}

void DkNoMacs::showRecentFiles(bool show) {

	// TODO: add setting
	if (DkSettings::app.appMode != DkSettings::mode_frameless && !DkSettings::global.recentFiles.empty())
		viewport()->getController()->showRecentFiles(show);

}

void DkNoMacs::onWindowLoaded() {

	QSettings s;
	bool firstTime = s.value("AppSettings/firstTime", true).toBool();

	if (!firstTime)
		return;

	// here are some first time requests
	DkWelcomeDialog* wecomeDialog = new DkWelcomeDialog(this);
	wecomeDialog->exec();

	s.setValue("AppSettings/firstTime", false);

	if (wecomeDialog->isLanguageChanged())
		restart();
}

void DkNoMacs::keyPressEvent(QKeyEvent *event) {
	
	if (event->key() == Qt::Key_Alt) {
		posGrabKey = QCursor::pos();
		otherKeyPressed = false;
	}
	else
		otherKeyPressed = true;

}

void DkNoMacs::keyReleaseEvent(QKeyEvent* event) {

	if (event->key() == Qt::Key_Alt && !otherKeyPressed && (posGrabKey - QCursor::pos()).manhattanLength() == 0)
		menu->showMenu();
	
}

// >DIR diem: eating shortcut overrides (this allows us to use navigation keys like arrows)
bool DkNoMacs::eventFilter(QObject *obj, QEvent *event) {

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
	panelActions[menu_panel_menu]->setChecked(DkSettings::app.showMenuBar);
	menu->setTimeToShow(tts);
	menu->showMenu();
}

void DkNoMacs::showToolbar(QToolBar* toolbar, bool show) {

	if (!toolbar)
		return;

	showToolbarsTemporarily(!show);

	if (show)
		addToolBar(toolbar);
	else
		removeToolBar(toolbar);

	toolbar->setVisible(show);
}

void DkNoMacs::showToolbarsTemporarily(bool show) {

	if (show) {
		for (int idx = 0; idx < hiddenToolbars.size(); idx++)
			hiddenToolbars.at(idx)->show();
	}
	else {

		hiddenToolbars.clear();
		QList<QToolBar *> toolbars = findChildren<QToolBar *>();

		for (int idx = 0; idx < toolbars.size(); idx++) {
			
			if (toolbars.at(idx)->isVisible()) {
				toolbars.at(idx)->hide();
				hiddenToolbars.append(toolbars.at(idx));
			}
		}
	}
}

void DkNoMacs::showToolbar(bool show) {

	DkSettings::app.showToolBar = show;
	panelActions[menu_panel_toolbar]->setChecked(DkSettings::app.showToolBar);
	
	if (DkSettings::app.showToolBar)
		toolbar->show();
	else
		toolbar->hide();
}

void DkNoMacs::showStatusBar(bool show, bool permanent) {

	if (statusbar->isVisible() == show)
		return;

	if (permanent)
		DkSettings::app.showStatusBar = show;
	panelActions[menu_panel_statusbar]->setChecked(DkSettings::app.showStatusBar);

	statusbar->setVisible(show);

	viewport()->setVisibleStatusbar(show);
}

void DkNoMacs::showStatusMessage(QString msg, int which) {

	if (which < 0 || which >= statusbarLabels.size())
		return;

	statusbarLabels[which]->setVisible(!msg.isEmpty());
	statusbarLabels[which]->setText(msg);
}

void DkNoMacs::openFileWith(QAction* action) {

	if (!action)
		return;

	QFileInfo app(action->toolTip());

	if (!app.exists())
		viewport()->getController()->setInfo("Sorry, " % app.fileName() % " does not exist");

	QStringList args;
	
	if (app.fileName() == "explorer.exe")
		args << "/select," << QDir::toNativeSeparators(viewport()->getImageLoader()->file().absoluteFilePath());
	else
		args << QDir::toNativeSeparators(viewport()->getImageLoader()->file().absoluteFilePath());

	//bool started = process.startDetached("psOpenImages.exe", args);	// already deprecated
	bool started = process.startDetached(app.absoluteFilePath(), args);

	if (started)
		qDebug() << "starting: " << app.fileName();
	else if (viewport())
		viewport()->getController()->setInfo("Sorry, I could not start: " % app.absoluteFilePath());
}

void DkNoMacs::showGpsCoordinates() {

	DkMetaDataInfo* exifData = viewport()->getController()->getMetaDataWidget();

	if (!exifData || exifData->getGPSCoordinates().isEmpty()) {
		viewport()->getController()->setInfo("Sorry, I could not find the GPS coordinates...");
		return;
	}

	qDebug() << "gps: " << exifData->getGPSCoordinates();

	QDesktopServices::openUrl(QUrl(exifData->getGPSCoordinates()));  
}

QVector <QAction* > DkNoMacs::getFileActions() {

	return fileActions;
}

QVector <QAction* > DkNoMacs::getBatchActions() {

	return toolsActions;
}

QVector <QAction* > DkNoMacs::getPanelActions() {

	return panelActions;
}

QVector <QAction* > DkNoMacs::getViewActions() {

	return viewActions;
}

QVector <QAction* > DkNoMacs::getSyncActions() {

	return syncActions;
}

void DkNoMacs::setWindowTitle(QSharedPointer<DkImageContainerT> imgC) {

	if (!imgC) {
		setWindowTitle(QFileInfo());
		return;
	}

	setWindowTitle(imgC->file(), imgC->image().size(), imgC->isEdited(), imgC->getTitleAttribute());
}

void DkNoMacs::setWindowTitle(QFileInfo file, QSize size, bool edited, QString attr) {

	// TODO: rename!

	////  do not tell the viewport (he should know it)
	//viewport()->setTitleLabel(file, -1);

	QString title = file.fileName();
	title = title.remove(".lnk");
	
	if (title.isEmpty())
		title = "nomacs - Image Lounge";

	if (edited)
		title.append("[*]");

	title.append(" ");
	title.append(attr);	// append some attributes

	QString attributes;

	if (!size.isEmpty())
		attributes.sprintf(" - %i x %i", size.width(), size.height());
	if (size.isEmpty() && viewport())
		attributes.sprintf(" - %i x %i", viewport()->getImage().width(), viewport()->getImage().height());

	QMainWindow::setWindowTitle(title.append(attributes));
	setWindowFilePath(file.absoluteFilePath());
	emit sendTitleSignal(windowTitle());
	setWindowModified(edited);

	if ((!viewport()->getController()->getFileInfoLabel()->isVisible() || 
		!DkSettings::slideShow.display.testBit(DkSettings::display_creation_date)) && viewport()->getImageLoader()->getCurrentImage()) {
		
		// create statusbar info
		QSharedPointer<DkMetaDataT> metaData = viewport()->getImageLoader()->getCurrentImage()->getMetaData();
		QString dateString = metaData->getExifValue("DateTimeOriginal");
		dateString = DkUtils::convertDateString(dateString, file);
		showStatusMessage(dateString, status_time_info);
	}
	else 
		showStatusMessage("", status_time_info);	// hide label

	if (file.exists())
		showStatusMessage(DkUtils::readableByte(file.size()), status_filesize_info);
	else 
		showStatusMessage("", status_filesize_info);

}

void DkNoMacs::openKeyboardShortcuts() {

	QList<QAction* > openWithActionList = openWithMenu->actions();
	QVector<QAction* > openWithActions;

	for (int idx = 0; idx < openWithActionList.size(); idx++) {
		if (!openWithActionList.at(idx)->text().isEmpty())
			openWithActions.append(openWithActionList.at(idx));
		else
			qDebug() << "Empty action detected!";
	}

	DkShortcutsDialog* shortcutsDialog = new DkShortcutsDialog(this);
	shortcutsDialog->addActions(fileActions, fileMenu->title());
	shortcutsDialog->addActions(openWithActions, openWithMenu->title());
	shortcutsDialog->addActions(sortActions, sortMenu->title());
	shortcutsDialog->addActions(editActions, editMenu->title());
	shortcutsDialog->addActions(viewActions, viewMenu->title());
	shortcutsDialog->addActions(panelActions, panelMenu->title());
	shortcutsDialog->addActions(toolsActions, toolsMenu->title());
	shortcutsDialog->addActions(syncActions, syncMenu->title());
#ifdef WITH_PLUGINS
	shortcutsDialog->addActions(pluginsActions, pluginsMenu->title());
#endif // WITH_PLUGINS
	shortcutsDialog->addActions(helpActions, helpMenu->title());

	shortcutsDialog->exec();
}

void DkNoMacs::openSettings() {

	if (!settingsDialog) {
		settingsDialog = new DkSettingsDialog(this);
		connect(settingsDialog, SIGNAL(setToDefaultSignal()), this, SLOT(cleanSettings()));
		connect(settingsDialog, SIGNAL(settingsChanged()), viewport(), SLOT(settingsChanged()));
		connect(settingsDialog, SIGNAL(languageChanged()), this, SLOT(restart()));
		connect(settingsDialog, SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));
	}

	settingsDialog->exec();

	qDebug() << "hier könnte ihre werbung stehen...";
}

void DkNoMacs::settingsChanged() {
	
	if (!isFullScreen()) {
		showMenuBar(DkSettings::app.showMenuBar);
		showToolbar(DkSettings::app.showToolBar);
		showStatusBar(DkSettings::app.showStatusBar);
	}
}

void DkNoMacs::checkForUpdate() {

	if (!updater) {
		updater = new DkUpdater();
		connect(updater, SIGNAL(displayUpdateDialog(QString, QString)), this, SLOT(showUpdateDialog(QString, QString)));
	}
	updater->silent = false;
	updater->checkForUpdates();

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
	if (progressDialog != 0 && !progressDialog->isHidden()) { // check if the progress bar is already open 
		showUpdaterMessage(tr("Already downloading update"), "update");
		return;
	}

	DkSettings::sync.updateDialogShown = true;

	DkSettings::save();
	
	if (!updateDialog) {
		updateDialog = new DkUpdateDialog(this);
		updateDialog->setWindowTitle(title);
		updateDialog->upperLabel->setText(msg);
		connect(updateDialog, SIGNAL(startUpdate()), this, SLOT(performUpdate()));
	}

	updateDialog->exec();
}

void DkNoMacs::performUpdate() {
	updater->performUpdate();

	if (!progressDialog) {
		progressDialog = new QProgressDialog(tr("Downloading update..."), tr("Cancel Update"), 0, 100);
		progressDialog->setWindowIcon(windowIcon());
		connect(progressDialog, SIGNAL(canceled()), updater, SLOT(cancelUpdate()));
		connect(updater, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateProgress(qint64, qint64)));
		connect(updater, SIGNAL(downloadFinished(QString)), progressDialog, SLOT(close()));
		//connect(updater, SIGNAL(downloadFinished(QString)), progressDialog, SLOT(deleteLater()));
		connect(updater, SIGNAL(downloadFinished(QString)), this, SLOT(startSetup(QString)));
	}
	progressDialog->setWindowModality(Qt::ApplicationModal);

	progressDialog->show();
	//progressDialog->raise();
	//progressDialog->activateWindow();
	progressDialog->setWindowModality(Qt::NonModal);
}

void DkNoMacs::updateProgress(qint64 received, qint64 total) {
	progressDialog->setMaximum(total);
	progressDialog->setValue(received);
}

void DkNoMacs::startSetup(QString filePath) {
	
	qDebug() << "starting setup filePath:" << filePath;
	
	if (!QFile::exists(filePath))
		qDebug() << "file does not exist";
	if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filePath))) {
		QString msg = tr("Unable to install new version<br>") +
			tr("You can download the new version from our web page") +
			"<br><a href=\"http://www.nomacs.org/download/\">www.nomacs.org</a><br>";
		showUpdaterMessage(msg, "update");
	}
}

void DkNoMacs::updateTranslations() {
	translationUpdater->checkForUpdates();
}

void DkNoMacs::errorDialog(const QString& msg) {
	dialog(msg, this, tr("Error"));
}

//void DkNoMacs::errorDialog(QString msg, QString title) {
//
//	dialog(msg, this, title);
//}

int DkNoMacs::dialog(QString msg, QWidget* parent, QString title) {

	if (!parent) {
		QWidgetList w = QApplication::topLevelWidgets();

		for (int idx = 0; idx < w.size(); idx++) {

			if (w[idx]->objectName().contains(QString("DkNoMacs"))) {
				parent = w[idx];
				break;
			}
		}
	}

	QMessageBox errorDialog(parent);
	errorDialog.setWindowTitle(title);
	errorDialog.setIcon(QMessageBox::Critical);
	errorDialog.setText(msg);
	errorDialog.show();

	return errorDialog.exec();

}

int DkNoMacs::infoDialog(QString msg, QWidget* parent, QString title) {

	QMessageBox errorDialog(parent);
	errorDialog.setWindowTitle(title);
	errorDialog.setIcon(QMessageBox::Question);
	errorDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	errorDialog.setText(msg);
	errorDialog.show();

	return errorDialog.exec();
}

/**
* Creates the plugin menu when it is not empty
* called in DkNoMacs::createPluginsMenu()
**/
void DkNoMacs::addPluginsToMenu() {
	
#ifdef WITH_PLUGINS

	QMap<QString, DkPluginInterface *> loadedPlugins = pluginManager->getPlugins();
	QList<QString> pluginIdList = pluginManager->getPluginIdList();

	QMap<QString, QString> runId2PluginId = QMap<QString, QString>();
	QList<QPair<QString, QString> > sortedNames = QList<QPair<QString, QString> >();
	QStringList pluginMenu = QStringList();

	QVector<QMenu*> pluginSubMenus = QVector<QMenu*>();

	for (int i = 0; i < pluginIdList.size(); i++) {

		DkPluginInterface* cPlugin = loadedPlugins.value(pluginIdList.at(i));

		if (cPlugin) {

			QStringList runID = cPlugin->runID();
			QList<QAction*> actions = cPlugin->pluginActions(this);

			if (!actions.empty()) {
				/*
				QAction* runPlugin = new QAction(cPlugin->pluginName(), this);
				connect(runPlugin, SIGNAL(triggered()), this, SLOT(runLoadedPlugin()));
				runPlugin->setData(cPlugin->pluginID());
				actions.prepend(runPlugin);
				*/
				for (int iAction = 0; iAction < actions.size(); iAction++) {
					connect(actions.at(iAction), SIGNAL(triggered()), this, SLOT(runLoadedPlugin()));
					runId2PluginId.insert(actions.at(iAction)->data().toString(), pluginIdList.at(i));
				}

				QMenu* sm = new QMenu(cPlugin->pluginMenuName(),this);
				sm->setStatusTip(cPlugin->pluginStatusTip());
				sm->addActions(actions);
				runId2PluginId.insert(cPlugin->pluginMenuName(), pluginIdList.at(i));
			
				pluginSubMenus.append(sm);
			}
			else {
		
				//QList<QPair<QString, QString> > sortedNames;


				for (int j = 0; j < runID.size(); j++) {
				
					runId2PluginId.insert(runID.at(j), pluginIdList.at(i));
					sortedNames.append(qMakePair(runID.at(j), cPlugin->pluginMenuName(runID.at(j))));
				}
			}
		}
	}

	/*
	QList<QPair<QString, QString> > sortedNames;
	QMapIterator<QString, QString> iter(pluginNames);		
	while(iter.hasNext()) {
		iter.next();
		sortedNames.append(qMakePair(iter.key(), loadedPlugins.value(iter.value())->pluginNames(iter.key())));
	}
	// Ordering ascending
	qSort(sortedNames.begin(), sortedNames.end(), QPairSecondComparer());
	*/

	//pluginsActions.resize(menu_plugins_end + sortedNames.size() + pluginSubMenus.size());
	
	pluginsMenu->addAction(pluginsActions[menu_plugin_manager]);
	pluginsMenu->addSeparator();

	QMap<QString, bool> pluginsEnabled = QMap<QString, bool>();

	QSettings settings;
	int size = settings.beginReadArray("PluginSettings/disabledPlugins");
	for (int i = 0; i < size; ++i) {
		settings.setArrayIndex(i);
		if (pluginIdList.contains(settings.value("pluginId").toString())) pluginsEnabled.insert(settings.value("pluginId").toString(), false);
	}
	settings.endArray();

	for(int i = 0; i < sortedNames.size(); i++) {


		if (pluginsEnabled.value(runId2PluginId.value(sortedNames.at(i).first), true)) {

			QAction* pluginAction = new QAction(sortedNames.at(i).second, this);
			pluginAction->setStatusTip(loadedPlugins.value(runId2PluginId.value(sortedNames.at(i).first))->pluginStatusTip(sortedNames.at(i).first));
			pluginAction->setData(sortedNames.at(i).first);
			connect(pluginAction, SIGNAL(triggered()), this, SLOT(runLoadedPlugin()));

			viewport()->addAction(pluginAction);
			pluginsMenu->addAction(pluginAction);
			pluginAction->setToolTip(pluginAction->statusTip());

			pluginsActions.append(pluginAction);
		}		
	}

	for (int idx = 0; idx < pluginSubMenus.size(); idx++) {

		if (pluginsEnabled.value(runId2PluginId.value(pluginSubMenus.at(idx)->title()), true))
			pluginsMenu->addMenu(pluginSubMenus.at(idx));

	}

	pluginManager->setRunId2PluginId(runId2PluginId);

	assignCustomShortcuts(pluginsActions);

#endif // WITH_PLUGINS
}

/**
* Creates the plugins menu 
**/
void DkNoMacs::createPluginsMenu() {
#ifdef WITH_PLUGINS

	QList<QString> pluginIdList = pluginManager->getPluginIdList();

	if(pluginIdList.isEmpty()) { // no  plugins
		pluginsMenu->clear();
		pluginsActions.resize(menu_plugins_end);
		pluginsMenu->addAction(pluginsActions[menu_plugin_manager]);
	}
	else {
		pluginsMenu->clear();
		// delete old plugin actions	
		for (int idx = pluginsActions.size(); idx > menu_plugins_end; idx--) {
			pluginsActions.last()->deleteLater();
			pluginsActions.last() = 0;
			pluginsActions.pop_back();
		}
		pluginsActions.resize(menu_plugins_end);
		addPluginsToMenu();
	}
#endif // WITH_PLUGINS
}

void DkNoMacs::openPluginManager() {
#ifdef WITH_PLUGINS

	if (!currRunningPlugin.isEmpty()) {
		closePlugin(true, false);
	}

	if (!currRunningPlugin.isEmpty()) {
	   	   
		QMessageBox infoDialog(this);
		infoDialog.setWindowTitle("Close plugin");
		infoDialog.setIcon(QMessageBox::Information);
		infoDialog.setText("Please close the currently opened plugin first.");
		infoDialog.show();

		infoDialog.exec();
		return;
	}

	bool done = pluginManager->exec();
	createPluginsMenu();
#endif // WITH_PLUGINS
}

void DkNoMacs::runLoadedPlugin() {
#ifdef WITH_PLUGINS

   QAction* action = qobject_cast<QAction*>(sender());

   if (!action)
	   return;

   if (!currRunningPlugin.isEmpty())
	   closePlugin(true, false);

   if (!currRunningPlugin.isEmpty()) {
	   
	    // the plugin is not closed in time
	   
	   	QMessageBox infoDialog(this);
		infoDialog.setWindowTitle("Close plugin");
		infoDialog.setIcon(QMessageBox::Information);
		infoDialog.setText("Please first close the currently opened plugin.");
		infoDialog.show();

		infoDialog.exec();
		return;
   }

   QString key = action->data().toString();
   DkPluginInterface* cPlugin = pluginManager->getPlugin(key);

   // something is wrong if no plugin is loaded
   if (!cPlugin) {
	   qDebug() << "plugin is NULL";
	   return;
   }

	if(viewport()->getImageLoader()->getImage().isNull() && cPlugin->closesOnImageChange()){
		QMessageBox msgBox(this);
		msgBox.setText("No image loaded\nThe plugin can't run.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
		return;
	}

   if (cPlugin->interfaceType() == DkPluginInterface::interface_viewport) {
	    
		DkViewPortInterface* vPlugin = dynamic_cast<DkViewPortInterface*>(cPlugin);

	   if(!vPlugin || !vPlugin->getViewPort()) 
		   return;

	   currRunningPlugin = key;
	   	   
	   connect(vPlugin->getViewPort(), SIGNAL(closePlugin(bool, bool)), this, SLOT(closePlugin(bool, bool)));
	   connect(vPlugin->getViewPort(), SIGNAL(showToolbar(QToolBar*, bool)), this, SLOT(showToolbar(QToolBar*, bool)));
	   connect(vPlugin->getViewPort(), SIGNAL(loadFile(QFileInfo)), viewport(), SLOT(loadFile(QFileInfo)));
	   connect(vPlugin->getViewPort(), SIGNAL(loadImage(QImage)), viewport(), SLOT(setImage(QImage)));
	   
	   viewport()->getController()->setPluginWidget(vPlugin, false);
   }
   else if (cPlugin->interfaceType() == DkPluginInterface::interface_basic) {

	    QImage tmpImg = viewport()->getImageLoader()->getImage();
		QImage result = cPlugin->runPlugin(key, tmpImg);
		if(!result.isNull()) viewport()->setEditedImage(result);
   }
#endif // WITH_PLUGINS
}

void DkNoMacs::initPluginManager() {
#ifdef WITH_PLUGINS

	if(!pluginManager) {
		pluginManager = new DkPluginManager(this);
		createPluginsMenu();
	}
#endif // WITH_PLUGINS
}

void DkNoMacs::closePlugin(bool askForSaving, bool alreadySaving) {
#ifdef WITH_PLUGINS

	if (currRunningPlugin.isEmpty())
		return;

	if (!viewport())
		return;

	DkPluginInterface* cPlugin = pluginManager->getPlugin(currRunningPlugin);


	currRunningPlugin = QString();
	bool isSaveNeeded = false;

	if (!cPlugin) 
		return;
	DkViewPortInterface* vPlugin = dynamic_cast<DkViewPortInterface*>(cPlugin);

	if (!vPlugin) 
		return;

	//viewport()->setPluginImageWasApplied(true);

	QImage pluginImage = vPlugin->runPlugin();	// empty vars - viewport plugin doesn't need them

	if (!pluginImage.isNull()) {
		if (askForSaving) {

			QMessageBox msgBox(this);
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			msgBox.setEscapeButton(QMessageBox::No);
			msgBox.setIcon(QMessageBox::Question);
			msgBox.setWindowTitle(tr("Closing plugin..."));

			msgBox.setText(tr("Do you want to apply plugin changes?"));

			if(msgBox.exec() == QMessageBox::Yes) {
				viewport()->setEditedImage(pluginImage);
				isSaveNeeded = true;
			}
			msgBox.deleteLater();
		}				
		else 
			viewport()->setEditedImage(pluginImage);
	}

	disconnect(vPlugin->getViewPort(), SIGNAL(showToolbar(QToolBar*, bool)), this, SLOT(showToolbar(QToolBar*, bool)));

	viewport()->getController()->setPluginWidget(vPlugin, true);

	if(!alreadySaving && isSaveNeeded) saveFileAs();

	//viewport()->setPluginImageWasApplied(true);

#endif // WITH_PLUGINS
}

void DkNoMacs::applyPluginChanges(bool askForSaving, bool alreadySaving) {

#ifdef WITH_PLUGINS
	if (currRunningPlugin.isEmpty())
		return;

	DkPluginInterface* cPlugin = pluginManager->getPlugin(currRunningPlugin);

	// does the plugin want to be closed on image changes?
	if (!cPlugin->closesOnImageChange())
		return;

	closePlugin(askForSaving, alreadySaving);
#endif // WITH_PLUGINS
}

// DkNoMacsSync --------------------------------------------------------------------
DkNoMacsSync::DkNoMacsSync(QWidget *parent, Qt::WindowFlags flags) : DkNoMacs(parent, flags) {

}

DkNoMacsSync::~DkNoMacsSync() {

	if (localClient) {

		// terminate local client
		localClient->quit();
		localClient->wait();

		delete localClient;
		localClient = 0;
	}

	if (rcClient) {

		if (DkSettings::sync.syncMode == DkSettings::sync_mode_remote_control)
			rcClient->sendNewMode(DkSettings::sync_mode_remote_control);	// TODO: if we need this threaded emit a signal here

		emit stopSynchronizeWithSignal();

		rcClient->quit();
		rcClient->wait();

		delete rcClient;
		rcClient = 0;

	}

}

void DkNoMacsSync::initLanClient() {

	if (lanClient) {

		lanClient->quit();
		lanClient->wait();

		delete lanClient;
	}


	// remote control server
	if (rcClient) {
		rcClient->quit();
		rcClient->wait();

		delete rcClient;
	}

	if (!DkSettings::sync.enableNetworkSync) {

		lanClient = 0;
		rcClient = 0;

		tcpLanMenu->setEnabled(false);
		syncActions[menu_sync_remote_control]->setEnabled(false);
		syncActions[menu_sync_remote_display]->setEnabled(false);
		return;
	}

	tcpLanMenu->clear();

	// start lan client/server
	lanClient = new DkLanManagerThread(this);
	lanClient->setObjectName("lanClient");
#ifdef WITH_UPNP
	if (!upnpControlPoint) {
		upnpControlPoint = QSharedPointer<DkUpnpControlPoint>(new DkUpnpControlPoint());
	}
	lanClient->upnpControlPoint = upnpControlPoint;
	if (!upnpDeviceHost) {
		upnpDeviceHost = QSharedPointer<DkUpnpDeviceHost>(new DkUpnpDeviceHost());
	}
	lanClient->upnpDeviceHost = upnpDeviceHost;
#endif // WITH_UPNP
	lanClient->start();

	tcpLanMenu->setClientManager(lanClient);
	tcpLanMenu->addTcpAction(lanActions[menu_lan_server]);
	tcpLanMenu->addTcpAction(lanActions[menu_lan_image]);	// well this is a bit nasty... we only add it here to have correct enable/disable behavior...
	tcpLanMenu->setEnabled(true);
	tcpLanMenu->enableActions(false, false);

	rcClient = new DkRCManagerThread(this);
	rcClient->setObjectName("rcClient");
#ifdef WITH_UPNP
	if (!upnpControlPoint) {
		upnpControlPoint = QSharedPointer<DkUpnpControlPoint>(new DkUpnpControlPoint());
	}
	rcClient->upnpControlPoint = upnpControlPoint;
	if (!upnpDeviceHost) {
		upnpDeviceHost = QSharedPointer<DkUpnpDeviceHost>(new DkUpnpDeviceHost());
	}
	rcClient->upnpDeviceHost = upnpDeviceHost;
#endif // WITH_UPNP
	
	rcClient->start();
	
	connect(lanActions[menu_lan_server], SIGNAL(toggled(bool)), this, SLOT(startTCPServer(bool)));	// TODO: something that makes sense...
	connect(lanActions[menu_lan_image], SIGNAL(triggered()), viewport(), SLOT(tcpSendImage()));

	connect(this, SIGNAL(startTCPServerSignal(bool)), lanClient, SLOT(startServer(bool)));
	connect(this, SIGNAL(startRCServerSignal(bool)), rcClient, SLOT(startServer(bool)), Qt::QueuedConnection);

	DkTimer dt;
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

	DkViewPort* vp = viewport();

	// sync menu
	syncActions.resize(menu_sync_end);
	syncActions[menu_sync] = new QAction(tr("Synchronize &View"), this);
	syncActions[menu_sync]->setShortcut(QKeySequence(shortcut_sync));
	syncActions[menu_sync]->setStatusTip(tr("synchronize the current view"));
	syncActions[menu_sync]->setEnabled(false);
	connect(syncActions[menu_sync], SIGNAL(triggered()), vp, SLOT(tcpForceSynchronize()));

	syncActions[menu_sync_pos] = new QAction(tr("&Window Overlay"), this);
	syncActions[menu_sync_pos]->setShortcut(QKeySequence(shortcut_tab));
	syncActions[menu_sync_pos]->setStatusTip(tr("toggle the window opacity"));
	syncActions[menu_sync_pos]->setEnabled(false);
	connect(syncActions[menu_sync_pos], SIGNAL(triggered()), this, SLOT(tcpSendWindowRect()));

	syncActions[menu_sync_arrange] = new QAction(tr("Arrange Instances"), this);
	syncActions[menu_sync_arrange]->setShortcut(QKeySequence(shortcut_arrange));
	syncActions[menu_sync_arrange]->setStatusTip(tr("arrange connected instances"));
	syncActions[menu_sync_arrange]->setEnabled(false);
	connect(syncActions[menu_sync_arrange], SIGNAL(triggered()), this, SLOT(tcpSendArrange()));

	syncActions[menu_sync_connect_all] = new QAction(tr("Connect &All"), this);
	syncActions[menu_sync_connect_all]->setShortcut(QKeySequence(shortcut_connect_all));
	syncActions[menu_sync_connect_all]->setStatusTip(tr("connect all instances"));
	connect(syncActions[menu_sync_connect_all], SIGNAL(triggered()), this, SLOT(tcpConnectAll()));

	syncActions[menu_sync_all_actions] = new QAction(tr("&Sync All Actions"), this);
	syncActions[menu_sync_all_actions]->setStatusTip(tr("Transmit All Signals Automatically."));
	syncActions[menu_sync_all_actions]->setCheckable(true);
	syncActions[menu_sync_all_actions]->setChecked(DkSettings::sync.syncActions);
	connect(syncActions[menu_sync_all_actions], SIGNAL(triggered(bool)), this, SLOT(tcpAutoConnect(bool)));

	syncActions[menu_sync_start_upnp] = new QAction(tr("&Start Upnp"), this);
	syncActions[menu_sync_start_upnp]->setStatusTip(tr("Starts a Upnp Media Renderer."));
	syncActions[menu_sync_start_upnp]->setCheckable(true);
	connect(syncActions[menu_sync_start_upnp], SIGNAL(triggered(bool)), this, SLOT(startUpnpRenderer(bool)));

	syncActions[menu_sync_remote_control] = new QAction(tr("&Remote Control"), this);
	//syncActions[menu_sync_remote_control]->setShortcut(QKeySequence(shortcut_connect_all));
	syncActions[menu_sync_remote_control]->setStatusTip(tr("Automatically Receive Images From Your Remote Instance."));
	syncActions[menu_sync_remote_control]->setCheckable(true);
	connect(syncActions[menu_sync_remote_control], SIGNAL(triggered(bool)), this, SLOT(tcpRemoteControl(bool)));

	syncActions[menu_sync_remote_display] = new QAction(tr("Remote &Display"), this);
	syncActions[menu_sync_remote_display]->setStatusTip(tr("Automatically Send Images to a Remote Instance."));
	syncActions[menu_sync_remote_display]->setCheckable(true);
	connect(syncActions[menu_sync_remote_display], SIGNAL(triggered(bool)), this, SLOT(tcpRemoteDisplay(bool)));

	lanActions.resize(menu_lan_end);

	// start server action
	lanActions[menu_lan_server] = new QAction(tr("Start &Server"), this);
	lanActions[menu_lan_server]->setObjectName("serverAction");
	lanActions[menu_lan_server]->setCheckable(true);
	lanActions[menu_lan_server]->setChecked(false);

	lanActions[menu_lan_image] = new QAction(tr("Send &Image"), this);
	lanActions[menu_lan_image]->setObjectName("sendImageAction");
	lanActions[menu_lan_image]->setShortcut(QKeySequence(shortcut_send_img));
	//sendImage->setEnabled(false);		// TODO: enable/disable sendImage action as needed
	lanActions[menu_lan_image]->setToolTip(tr("Sends the current image to all clients."));

	assignCustomShortcuts(syncActions);
}

void DkNoMacsSync::createMenu() {

	DkNoMacs::createMenu();

	// local host menu
	tcpViewerMenu = new DkTcpMenu(tr("&Synchronize"), syncMenu, localClient);
	tcpViewerMenu->showNoClientsFound(true);
	syncMenu->addMenu(tcpViewerMenu);

	// connect all action
	tcpViewerMenu->addTcpAction(syncActions[menu_sync_connect_all]);

	// LAN menu
	tcpLanMenu = new DkTcpMenu(tr("&LAN Synchronize"), syncMenu, lanClient);	// TODO: replace
	syncMenu->addMenu(tcpLanMenu);

	syncMenu->addAction(syncActions[menu_sync_remote_control]);
	syncMenu->addAction(syncActions[menu_sync_remote_display]);
	syncMenu->addAction(lanActions[menu_lan_image]);
	syncMenu->addSeparator();

	syncMenu->addAction(syncActions[menu_sync]);
	syncMenu->addAction(syncActions[menu_sync_pos]);
	syncMenu->addAction(syncActions[menu_sync_arrange]);
	syncMenu->addAction(syncActions[menu_sync_all_actions]);
#ifdef WITH_UPNP
	// disable this action since it does not work using herqq
	//syncMenu->addAction(syncActions[menu_sync_start_upnp]);
#endif // WITH_UPNP

}

// mouse events
void DkNoMacsSync::mouseMoveEvent(QMouseEvent *event) {

	int dist = QPoint(event->pos()-mousePos).manhattanLength();

	// create drag sync action
	if (event->buttons() == Qt::LeftButton && dist > QApplication::startDragDistance() &&
		event->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {

			qDebug() << "generating a drag event...";

			QByteArray connectionData;
			QDataStream dataStream(&connectionData, QIODevice::WriteOnly);
			dataStream << localClient->getServerPort();
			qDebug() << "serverport: " << localClient->getServerPort();

			QDrag* drag = new QDrag(this);
			QMimeData *mimeData = new QMimeData;
			mimeData->setData("network/sync-dir", connectionData);

			drag->setMimeData(mimeData);
			drag->exec(Qt::CopyAction | Qt::MoveAction);
	}
	else
		DkNoMacs::mouseMoveEvent(event);

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
		DkNoMacs::dropEvent(event);

}

qint16 DkNoMacsSync::getServerPort() {

	return (localClient) ? localClient->getServerPort() : 0;
}

void DkNoMacsSync::syncWith(qint16 port) {
	emit synchronizeWithServerPortSignal(port);
}

// slots
void DkNoMacsSync::tcpConnectAll() {

	QList<DkPeer> peers = localClient->getPeerList();

	for (int idx = 0; idx < peers.size(); idx++)
		emit synchronizeWithSignal(peers.at(idx).peerId);

}

void DkNoMacsSync::tcpChangeSyncMode(int syncMode, bool connectWithWhiteList) {

	if (syncMode == DkSettings::sync.syncMode || !rcClient)
		return;

	// turn off everything
	if (syncMode == DkSettings::sync_mode_default)
		rcClient->sendGoodByeToAll();

	syncActions[menu_sync_remote_control]->setChecked(false);
	syncActions[menu_sync_remote_display]->setChecked(false);

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
			syncActions[menu_sync_remote_control]->setChecked(true);	
			break;
		case DkSettings::sync_mode_remote_display: 
			syncActions[menu_sync_remote_display]->setChecked(true);
			break;
	//default:
	}

	DkSettings::sync.syncMode = syncMode;
}


void DkNoMacsSync::tcpRemoteControl(bool start) {

	if (!rcClient)
		return;

	tcpChangeSyncMode((start) ? DkSettings::sync_mode_remote_control : DkSettings::sync_mode_default, true);
}

void DkNoMacsSync::tcpRemoteDisplay(bool start) {

	if (!rcClient)
		return;

	tcpChangeSyncMode((start) ? DkSettings::sync_mode_remote_display : DkSettings::sync_mode_default, true);

}

void DkNoMacsSync::tcpAutoConnect(bool connect) {

	DkSettings::sync.syncActions = connect;
}

void DkNoMacsSync::startUpnpRenderer(bool start) {
#ifdef WITH_UPNP
	if (!upnpRendererDeviceHost) {
		upnpRendererDeviceHost = QSharedPointer<DkUpnpRendererDeviceHost>(new DkUpnpRendererDeviceHost());
		connect(upnpRendererDeviceHost.data(), SIGNAL(newImage(QImage)), viewport(), SLOT(setImage(QImage)));
	}
	if(start)
		upnpRendererDeviceHost->startDevicehost(":/nomacs/descriptions/nomacs_mediarenderer_description.xml");
	else
		upnpDeviceHost->stopDevicehost();
#endif // WITH_UPNP
}

bool DkNoMacsSync::connectWhiteList(int mode, bool connect) {

	if (!rcClient)
		return false;

	bool couldConnect = false;

	QList<DkPeer> peers = rcClient->getPeerList();
	qDebug() << "number of peers in list:" << peers.size();

	// TODO: add gui if idx != 1
	if (connect && !peers.isEmpty()) {
		DkPeer peer = peers[0];

		emit synchronizeRemoteControl(peer.peerId);
		
		if (mode == DkSettings::sync_mode_remote_control)
			rcClient->sendNewMode(DkSettings::sync_mode_remote_display);	// TODO: if we need this threaded emit a signal here
		else
			rcClient->sendNewMode(DkSettings::sync_mode_remote_control);	// TODO: if we need this threaded emit a signal here

		couldConnect = true;
	}
	else if (!connect) {

		if (mode == DkSettings::sync_mode_remote_control)
			rcClient->sendNewMode(DkSettings::sync_mode_remote_display);	// TODO: if we need this threaded emit a signal here
		else
			rcClient->sendNewMode(DkSettings::sync_mode_remote_control);	// TODO: if we need this threaded emit a signal here

		emit stopSynchronizeWithSignal();
		couldConnect = true;
	}

	return couldConnect;
}

void DkNoMacsSync::newClientConnected(bool connected, bool local) {

	tcpLanMenu->enableActions(connected, local);
	
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
	setCentralWidget(vp);

	localClient = new DkLocalManagerThread(this);
	localClient->setObjectName("localClient");
	localClient->start();

	lanClient = 0;
	rcClient = 0;


	init();
	setAcceptDrops(true);
	setMouseTracking (true);	//receive mouse event everytime

	updater = new DkUpdater();
	connect(updater, SIGNAL(displayUpdateDialog(QString, QString)), this, SLOT(showUpdateDialog(QString, QString)));
	connect(updater, SIGNAL(showUpdaterMessage(QString, QString)), this, SLOT(showUpdaterMessage(QString, QString)));

	translationUpdater = new DkTranslationUpdater();
	connect(translationUpdater, SIGNAL(showUpdaterMessage(QString, QString)), this, SLOT(showUpdaterMessage(QString, QString)));

#ifndef Q_WS_X11
	if (!DkSettings::sync.updateDialogShown && QDate::currentDate() > DkSettings::sync.lastUpdateCheck)
		updater->checkForUpdates();	// TODO: is threaded??

#endif
	
	// sync signals
	connect(vp, SIGNAL(newClientConnectedSignal(bool, bool)), this, SLOT(newClientConnected(bool, bool)));

	vp->getController()->getFilePreview()->registerAction(panelActions[menu_panel_preview]);
	vp->getController()->getThumbWidget()->registerAction(panelActions[menu_panel_thumbview]);
	vp->getController()->getScroller()->registerAction(panelActions[menu_panel_scroller]);
	vp->getController()->getMetaDataWidget()->registerAction(panelActions[menu_panel_exif]);
	vp->getController()->getPlayer()->registerAction(panelActions[menu_panel_player]);
	vp->getController()->getCropWidget()->registerAction(editActions[menu_edit_crop]);
	vp->getController()->getFileInfoLabel()->registerAction(panelActions[menu_panel_info]);
	vp->getController()->getHistogram()->registerAction(panelActions[menu_panel_histogram]);
	vp->getController()->getRecentFilesWidget()->registerAction(fileActions[menu_file_show_recent]);
	DkSettings::app.appMode = 0;

	initLanClient();
	//emit sendTitleSignal(windowTitle());

	// show it...
	show();
	DkSettings::app.appMode = DkSettings::mode_default;

	qDebug() << "viewport (normal) created...";
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
		setCentralWidget(vp);

		init();
		
		setAcceptDrops(true);
		setMouseTracking (true);	//receive mouse event everytime

		updater = new DkUpdater();
		connect(updater, SIGNAL(displayUpdateDialog(QString, QString)), this, SLOT(showUpdateDialog(QString, QString)));

		translationUpdater = new DkTranslationUpdater();
		connect(translationUpdater, SIGNAL(showUpdaterMessage(QString, QString)), this, SLOT(showUpdaterMessage(QString, QString)));

#ifndef Q_WS_X11
		if (!DkSettings::sync.updateDialogShown && QDate::currentDate() > DkSettings::sync.lastUpdateCheck)
			updater->checkForUpdates();
#endif

		vp->getController()->getFilePreview()->registerAction(panelActions[menu_panel_preview]);
		vp->getController()->getThumbWidget()->registerAction(panelActions[menu_panel_thumbview]);
		vp->getController()->getScroller()->registerAction(panelActions[menu_panel_scroller]);
		vp->getController()->getMetaDataWidget()->registerAction(panelActions[menu_panel_exif]);
		vp->getController()->getPlayer()->registerAction(panelActions[menu_panel_player]);
		vp->getController()->getFileInfoLabel()->registerAction(panelActions[menu_panel_info]);
		vp->getController()->getHistogram()->registerAction(panelActions[menu_panel_histogram]);

		// in frameless, you cannot control if menu is visible...
		panelActions[menu_panel_menu]->setEnabled(false);
		panelActions[menu_panel_statusbar]->setEnabled(false);
		panelActions[menu_panel_statusbar]->setChecked(false);
		panelActions[menu_panel_toolbar]->setChecked(false);

		menu->setTimeToShow(5000);
		menu->hide();
		
		vp->addStartActions(fileActions[menu_file_open], &fileIcons[icon_file_open_large]);
		vp->addStartActions(fileActions[menu_file_open_dir], &fileIcons[icon_file_dir_large]);

		viewActions[menu_view_frameless]->blockSignals(true);
		viewActions[menu_view_frameless]->setChecked(true);
		viewActions[menu_view_frameless]->blockSignals(false);

		dw = QApplication::desktop();
		updateScreenSize();
		show();
        
        connect(dw, SIGNAL(workAreaResized(int)), this, SLOT(updateScreenSize(int)));

		setObjectName("DkNoMacsFrameless");
		showStatusBar(false);	// fix
}

DkNoMacsFrameless::~DkNoMacsFrameless() {
	release();
}

void DkNoMacsFrameless::release() {
}

void DkNoMacsFrameless::enableNoImageActions(bool enable) {

	DkNoMacs::enableNoImageActions(enable);

	// actions that should always be disabled
	viewActions[menu_view_fit_frame]->setEnabled(false);

}

void DkNoMacsFrameless::updateScreenSize(int screen) {

	if (!dw)
		return;

	// for now: set to fullscreen

	int sc = dw->screenCount();
	QRect screenRects = dw->availableGeometry();

	for (int idx = 0; idx < sc; idx++) {

		qDebug() << "screens: " << dw->availableGeometry(idx);
		QRect curScreen = dw->availableGeometry(idx);
		screenRects.setLeft(qMin(screenRects.left(), curScreen.left()));
		screenRects.setTop(qMin(screenRects.top(), curScreen.top()));
		screenRects.setBottom(qMax(screenRects.bottom(), curScreen.bottom()));
		screenRects.setRight(qMax(screenRects.right(), curScreen.right()));
	}

	qDebug() << "set up geometry: " << screenRects;

	this->setGeometry(screenRects);

	DkViewPortFrameless* vp = static_cast<DkViewPortFrameless*>(viewport());
	vp->setMainGeometry(dw->screenGeometry());

}

void DkNoMacsFrameless::exitFullScreen() {

	// TODO: delete this function if we support menu in frameless mode
	if (isFullScreen())
		showNormal();

	if (viewport())
		viewport()->setFullScreen(false);
}


// >DIR diem: eating shortcut overrides
bool DkNoMacsFrameless::eventFilter(QObject *obj, QEvent *event) {

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
	if (saveSettings)
		DkSettings::save();

	saveSettings = false;

	DkNoMacs::closeEvent(event);

}

// Transfer function:

DkNoMacsContrast::DkNoMacsContrast(QWidget *parent, Qt::WindowFlags flags)
	: DkNoMacsSync(parent, flags) {


		setObjectName("DkNoMacsContrast");

		// init members
		DkViewPortContrast* vp = new DkViewPortContrast(this);
		vp->setAlignment(Qt::AlignHCenter);
		setCentralWidget(vp);

		localClient = new DkLocalManagerThread(this);
		localClient->setObjectName("localClient");
		localClient->start();

		lanClient = 0;
		rcClient = 0;

		init();

		createTransferToolbar();

		setAcceptDrops(true);
		setMouseTracking (true);	//receive mouse event everytime

		updater = new DkUpdater();
		connect(updater, SIGNAL(displayUpdateDialog(QString, QString)), this, SLOT(showUpdateDialog(QString, QString)));

		translationUpdater = new DkTranslationUpdater();
		connect(translationUpdater, SIGNAL(showUpdaterMessage(QString, QString)), this, SLOT(showUpdaterMessage(QString, QString)));

#ifndef Q_WS_X11
		if (!DkSettings::sync.updateDialogShown && QDate::currentDate() > DkSettings::sync.lastUpdateCheck)
			updater->checkForUpdates();	// TODO: is threaded??
#endif

		// sync signals
		connect(vp, SIGNAL(newClientConnectedSignal(bool, bool)), this, SLOT(newClientConnected(bool, bool)));
		
		vp->getController()->getFilePreview()->registerAction(panelActions[menu_panel_preview]);
		vp->getController()->getThumbWidget()->registerAction(panelActions[menu_panel_thumbview]);
		vp->getController()->getScroller()->registerAction(panelActions[menu_panel_scroller]);
		vp->getController()->getMetaDataWidget()->registerAction(panelActions[menu_panel_exif]);
		vp->getController()->getPlayer()->registerAction(panelActions[menu_panel_player]);
		vp->getController()->getFileInfoLabel()->registerAction(panelActions[menu_panel_info]);
		vp->getController()->getCropWidget()->registerAction(editActions[menu_edit_crop]);
		vp->getController()->getHistogram()->registerAction(panelActions[menu_panel_histogram]);
		vp->getController()->getRecentFilesWidget()->registerAction(fileActions[menu_file_show_recent]);

		initLanClient();
		emit sendTitleSignal(windowTitle());

		DkSettings::app.appMode = DkSettings::mode_contrast;
		setObjectName("DkNoMacsContrast");

		// show it...
		show();

		// TODO: this should be checked but no event should be called
		panelActions[menu_panel_transfertoolbar]->blockSignals(true);
		panelActions[menu_panel_transfertoolbar]->setChecked(true);
		panelActions[menu_panel_transfertoolbar]->blockSignals(false);

		qDebug() << "viewport (normal) created...";
}

DkNoMacsContrast::~DkNoMacsContrast() {
	release();
}

void DkNoMacsContrast::release() {
}

void DkNoMacsContrast::createTransferToolbar() {

	transferToolBar = new DkTransferToolBar(this);

	// add this toolbar below all previous toolbars
	addToolBarBreak();
	addToolBar(transferToolBar);
	transferToolBar->setObjectName("TransferToolBar");

	//transferToolBar->layout()->setSizeConstraint(QLayout::SetMinimumSize);
	
	connect(transferToolBar, SIGNAL(colorTableChanged(QGradientStops)),  viewport(), SLOT(changeColorTable(QGradientStops)));
	connect(transferToolBar, SIGNAL(channelChanged(int)),  viewport(), SLOT(changeChannel(int)));
	connect(transferToolBar, SIGNAL(pickColorRequest()),  viewport(), SLOT(pickColor()));
	connect(transferToolBar, SIGNAL(tFEnabled(bool)), viewport(), SLOT(enableTF(bool)));
	connect((DkViewPortContrast*)centralWidget(), SIGNAL(tFSliderAdded(qreal)), transferToolBar, SLOT(insertSlider(qreal)));
	connect((DkViewPortContrast*)centralWidget(), SIGNAL(imageModeSet(int)), transferToolBar, SLOT(setImageMode(int)));

	if (DkSettings::display.smallIcons)
		transferToolBar->setIconSize(QSize(16, 16));
	else
		transferToolBar->setIconSize(QSize(32, 32));


	if (DkSettings::display.toolbarGradient) {

		QColor hCol = DkSettings::display.highlightColor;
		hCol.setAlpha(80);

		transferToolBar->setStyleSheet(
			//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
			QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }")
			+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
			//+ QString("QToolButton:disabled{background-color: rgba(0,0,0,10);}")
			+ QString("QToolButton:hover{border: none; background-color: rgba(255,255,255,80);} QToolButton:pressed{margin: 0px; border: none; background-color: " + DkUtils::colorToString(hCol) + ";}")
			);
	}


}
}
