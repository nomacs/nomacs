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



DkNoMacs::DkNoMacs(QWidget *parent, Qt::WFlags flags)
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
	tifDialog = 0;
	resizeDialog = 0;
	opacityDialog = 0;
	updater = 0;
	openWithDialog = 0;
	imgManipulationDialog = 0;
	updateDialog = 0;
	progressDialog = 0;

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
}

void DkNoMacs::init() {

	//setStyleSheet( "QMainWindow { border-style: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }" );

// assign icon -> in windows the 32px version
#ifdef Q_WS_WIN
	QString iconPath = ":/nomacs/img/nomacs32.png";
#else
	QString iconPath = ":/nomacs/img/nomacs.png";
#endif

	QIcon dirIcon = QIcon(iconPath);
	setObjectName("DkNoMacs");

	if (!dirIcon.isNull())
		setWindowIcon(dirIcon);

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
	addActions(fileActions.toList());
	addActions(editActions.toList());
	addActions(toolsActions.toList());
	addActions(viewActions.toList());
	addActions(syncActions.toList());
	addActions(helpActions.toList());

	// automatically add status tip as tool tip
	for (int idx = 0; idx < fileActions.size(); idx++)
		fileActions[idx]->setToolTip(fileActions[idx]->statusTip());
	for (int idx = 0; idx < editActions.size(); idx++)
		editActions[idx]->setToolTip(editActions[idx]->statusTip());
	for (int idx = 0; idx < toolsActions.size(); idx++)
		toolsActions[idx]->setToolTip(toolsActions[idx]->statusTip());
	for (int idx = 0; idx < viewActions.size(); idx++)
		viewActions[idx]->setToolTip(viewActions[idx]->statusTip());
	for (int idx = 0; idx < syncActions.size(); idx++)
		syncActions[idx]->setToolTip(syncActions[idx]->statusTip());
	for (int idx = 0; idx < helpActions.size(); idx++)
		helpActions[idx]->setToolTip(helpActions[idx]->statusTip());


	// TODO - just for android register me as a gesture recognizer
	grabGesture(Qt::PanGesture);
	grabGesture(Qt::PinchGesture);
	grabGesture(Qt::SwipeGesture);

	// load the window at the same position as last time
	readSettings();
	installEventFilter(this);

	showMenuBar(DkSettings::App::showMenuBar);
	showToolbar(DkSettings::App::showToolBar);
	showStatusBar(DkSettings::App::showStatusBar);

	// connects that are needed in all viewers
	connect(viewport(), SIGNAL(showStatusBar(bool, bool)), this, SLOT(showStatusBar(bool, bool)));
}

#ifdef Q_WS_WIN	// windows specific versioning
#include <windows.h>
#undef min
#undef max
#include <stdio.h>
#include <string>

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
			DkFileException("The version info size is zero\n", __LINE__, __FILE__);
		}
		std::vector<BYTE> data(dwSize);

		// load the version info
		if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0])) {
			DkFileException("Sorry, I can't read the version info\n", __LINE__, __FILE__);
		}

		// get the name and version strings
		UINT                uiVerLen = 0;
		VS_FIXEDFILEINFO*   pFixedInfo = 0;     // pointer to fixed file info structure

		if (!VerQueryValue(&data[0], TEXT("\\"), (void**)&pFixedInfo, (UINT *)&uiVerLen)) {
			DkFileException("Sorry, I can't get the version values...\n", __LINE__, __FILE__);
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

	if (DkSettings::Display::smallIcons)
		toolbar->setIconSize(QSize(16, 16));
	else
		toolbar->setIconSize(QSize(32, 32));
	
	qDebug() << toolbar->styleSheet();

	if (DkSettings::Display::toolbarGradient) {

		QColor hCol = DkSettings::Display::highlightColor;
		hCol.setAlpha(80);

		toolbar->setStyleSheet(
			//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
			QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }")
			+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
			//+ QString("QToolButton:disabled{background-color: rgba(0,0,0,10);}")
			+ QString("QToolButton:hover{border: none; background-color: rgba(255,255,255,80);} QToolButton:pressed{margin: 0px; border: none; background-color: " + DkUtils::colorToString(hCol) + ";}")
			);
	}
	//else if (!DkSettings::Display::useDefaultColor)
	//	toolbar->setStyleSheet("QToolBar#EditToolBar{background-color: " + DkUtils::colorToString(DkSettings::Display::bgColor) + ";}" );

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
	toolbar->addAction(editActions[menu_edit_transfrom]);
	//toolbar->addAction(editActions[menu_edit_image_manipulation]);
	toolbar->addSeparator();

	// view
	toolbar->addAction(viewActions[menu_view_fullscreen]);
	toolbar->addAction(viewActions[menu_view_reset]);
	toolbar->addAction(viewActions[menu_view_100]);
	toolbar->addSeparator();

	toolbar->addAction(viewActions[menu_view_gps_map]);
}


void DkNoMacs::createStatusbar() {

	statusbarMsg = new QLabel();
	statusbarMsg->setToolTip(tr("CTRL activates the crosshair cursor"));

	statusbar = new QStatusBar(this);
	statusbar->setObjectName("DkStatusBar");
	QColor col = QColor(200, 200, 230, 100);

	if (DkSettings::Display::toolbarGradient)
		statusbar->setStyleSheet(QString("QStatusBar {border-top: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }"));	
	//else if (!DkSettings::Display::useDefaultColor)
	//	statusbar->setStyleSheet("QStatusBar#DkStatusBar{background-color: " + DkUtils::colorToString(DkSettings::Display::bgColor) + ";}");

	statusbar->addWidget(statusbarMsg);
	statusbar->hide();
	//statusbar->addPermanentWidget()
	this->setStatusBar(statusbar);
}


void DkNoMacs::createIcons() {

	// this is unbelievable dirty - but for now the quickest way to turn themes off if someone uses customized icons...
	if (DkSettings::Display::defaultIconColor) {
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

	toolsIcons.resize(icon_tools_end);
	toolsIcons[icon_tools_manipulation] = ICON("", ":/nomacs/img/manipulation.png");

	if (!DkSettings::Display::defaultIconColor) {
		// now colorize all icons
		for (int idx = 0; idx < fileIcons.size(); idx++) {

			// never colorize these large icons
			if (idx == icon_file_open_large || idx == icon_file_dir_large)
				continue;

			fileIcons[idx].addPixmap(DkUtils::colorizePixmap(fileIcons[idx].pixmap(100, QIcon::Normal, QIcon::On), DkSettings::Display::iconColor), QIcon::Normal, QIcon::On);
			fileIcons[idx].addPixmap(DkUtils::colorizePixmap(fileIcons[idx].pixmap(100, QIcon::Normal, QIcon::Off), DkSettings::Display::iconColor), QIcon::Normal, QIcon::Off);
		}

		// now colorize all icons
		for (int idx = 0; idx < editIcons.size(); idx++)
			editIcons[idx].addPixmap(DkUtils::colorizePixmap(editIcons[idx].pixmap(100), DkSettings::Display::iconColor));

		for (int idx = 0; idx < viewIcons.size(); idx++)
			viewIcons[idx].addPixmap(DkUtils::colorizePixmap(viewIcons[idx].pixmap(100), DkSettings::Display::iconColor));

		for (int idx = 0; idx < toolsIcons.size(); idx++)
			toolsIcons[idx].addPixmap(DkUtils::colorizePixmap(toolsIcons[idx].pixmap(100), DkSettings::Display::iconColor));

	}
}

void DkNoMacs::createMenu() {

	this->setMenuBar(menu);
	fileMenu = menu->addMenu(tr("&File"));
	fileMenu->addAction(fileActions[menu_file_open]);
	fileMenu->addAction(fileActions[menu_file_open_dir]);
	fileMenu->addAction(fileActions[menu_file_open_with]);
	fileMenu->addAction(fileActions[menu_file_save]);
	fileMenu->addAction(fileActions[menu_file_rename]);
	fileMenu->addSeparator();

	fileFilesMenu = new DkHistoryMenu(tr("Recent &Files"), fileMenu, &DkSettings::Global::recentFiles);
	connect(fileFilesMenu, SIGNAL(loadFileSignal(QFileInfo)), viewport(), SLOT(loadFile(QFileInfo)));

	fileFoldersMenu = new DkHistoryMenu(tr("Recent Fo&lders"), fileMenu, &DkSettings::Global::recentFolders);
	connect(fileFoldersMenu, SIGNAL(loadFileSignal(QFileInfo)), viewport(), SLOT(loadFile(QFileInfo)));

	fileMenu->addMenu(fileFilesMenu);
	fileMenu->addMenu(fileFoldersMenu);

	fileMenu->addSeparator();
	fileMenu->addAction(fileActions[menu_file_print]);
	fileMenu->addSeparator();
	fileMenu->addAction(fileActions[menu_file_goto]);
	fileMenu->addAction(fileActions[menu_file_find]);
	fileMenu->addAction(fileActions[menu_file_reload]);
	fileMenu->addAction(fileActions[menu_file_prev]);
	fileMenu->addAction(fileActions[menu_file_next]);
	fileMenu->addSeparator();
	//fileMenu->addAction(fileActions[menu_file_share_fb]);
	//fileMenu->addSeparator();
	fileMenu->addAction(fileActions[menu_file_new_instance]);
	fileMenu->addAction(fileActions[menu_file_exit]);

	editMenu = menu->addMenu(tr("&Edit"));
	editMenu->addAction(editActions[menu_edit_copy]);
	editMenu->addAction(editActions[menu_edit_copy_buffer]);
	editMenu->addAction(editActions[menu_edit_paste]);
	editMenu->addSeparator();
	editMenu->addAction(editActions[menu_edit_rotate_ccw]);
	editMenu->addAction(editActions[menu_edit_rotate_cw]);
	editMenu->addAction(editActions[menu_edit_rotate_180]);
	editMenu->addSeparator();
	editMenu->addAction(editActions[menu_edit_transfrom]);
	editMenu->addAction(editActions[menu_edit_crop]);
	editMenu->addAction(editActions[menu_edit_delete]);
	editMenu->addSeparator();
#ifdef Q_WS_WIN
	editMenu->addAction(editActions[menu_edit_wallpaper]);
	editMenu->addSeparator();
#endif
	editMenu->addAction(editActions[menu_edit_preferences]);

	viewMenu = menu->addMenu(tr("&View"));
	viewToolsMenu = viewMenu->addMenu(tr("Tool&bars"));
	viewToolsMenu->addAction(viewActions[menu_view_show_menu]);
	viewToolsMenu->addAction(viewActions[menu_view_show_toolbar]);
	viewToolsMenu->addAction(viewActions[menu_view_show_statusbar]);
	viewToolsMenu->addAction(viewActions[menu_view_show_transfertoolbar]);
	viewMenu->addAction(viewActions[menu_view_show_preview]);
	viewMenu->addAction(viewActions[menu_view_show_exif]);
	viewMenu->addAction(viewActions[menu_view_show_overview]);
	viewMenu->addAction(viewActions[menu_view_show_player]);
	viewMenu->addAction(viewActions[menu_view_show_info]);
	viewMenu->addAction(viewActions[menu_view_show_histogram]);
	viewMenu->addSeparator();
	
	viewMenu->addAction(viewActions[menu_view_frameless]);	
	viewMenu->addSeparator();

	viewMenu->addAction(viewActions[menu_view_fullscreen]);
	viewMenu->addAction(viewActions[menu_view_reset]);
	viewMenu->addAction(viewActions[menu_view_100]);
	viewMenu->addAction(viewActions[menu_view_fit_frame]);
	viewMenu->addAction(viewActions[menu_view_zoom_in]);
	viewMenu->addAction(viewActions[menu_view_zoom_out]);
	viewMenu->addAction(viewActions[menu_view_anti_aliasing]);
	viewMenu->addSeparator();

	viewMenu->addAction(viewActions[menu_view_opacity_change]);
	viewMenu->addAction(viewActions[menu_view_opacity_up]);
	viewMenu->addAction(viewActions[menu_view_opacity_down]);
	viewMenu->addAction(viewActions[menu_view_opacity_an]);
#ifdef Q_WS_WIN
	viewMenu->addAction(viewActions[menu_view_lock_window]);
#endif
	viewMenu->addSeparator();
	
	viewMenu->addAction(viewActions[menu_view_gps_map]);

	toolsMenu = menu->addMenu(tr("&Tools"));
	toolsMenu->addAction(toolsActions[menu_tools_thumbs]);
	toolsMenu->addAction(toolsActions[menu_tools_filter]);
	toolsMenu->addAction(toolsActions[menu_tools_manipulation]);

	// no sync menu in frameless view
	if (DkSettings::App::appMode != DkSettings::mode_frameless)
		syncMenu = menu->addMenu(tr("&Sync"));
	else 
		syncMenu = 0;
	
	helpMenu = menu->addMenu(tr("&?"));
#ifndef Q_WS_X11
	helpMenu->addAction(helpActions[menu_help_update]);
	helpMenu->addSeparator();
#endif // !Q_WS_X11
	helpMenu->addAction(helpActions[menu_help_bug]);
	helpMenu->addAction(helpActions[menu_help_feature]);
	helpMenu->addSeparator();
	helpMenu->addAction(helpActions[menu_help_about]);

}

void DkNoMacs::createContextMenu() {

	contextMenu = new QMenu(this);

	contextMenu->addAction(viewActions[menu_view_show_preview]);
	contextMenu->addAction(viewActions[menu_view_show_exif]);
	contextMenu->addAction(viewActions[menu_view_show_overview]);
	contextMenu->addAction(viewActions[menu_view_show_player]);
	contextMenu->addAction(viewActions[menu_view_show_info]);
	contextMenu->addAction(viewActions[menu_view_show_histogram]);
	contextMenu->addSeparator();
	
	contextMenu->addAction(editActions[menu_edit_copy_buffer]);
	contextMenu->addAction(editActions[menu_edit_copy]);
	contextMenu->addAction(editActions[menu_edit_paste]);
	contextMenu->addSeparator();
	
	contextMenu->addAction(viewActions[menu_view_frameless]);
	contextMenu->addSeparator();

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
	editContextMenu->addAction(editActions[menu_edit_transfrom]);
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

	fileActions[menu_file_open_with] = new QAction(tr("Open &With"), this);
	fileActions[menu_file_open_with]->setShortcut(QKeySequence(shortcut_open_with));
	fileActions[menu_file_open_with]->setStatusTip(tr("Open an image in a different Program"));
	connect(fileActions[menu_file_open_with], SIGNAL(triggered()), this, SLOT(openFileWith()));

	fileActions[menu_file_rename] = new QAction(tr("Re&name"), this);
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

	fileActions[menu_file_print] = new QAction(fileIcons[icon_file_print], tr("&Print"), this);
	fileActions[menu_file_print]->setShortcuts(QKeySequence::Print);
	fileActions[menu_file_print]->setStatusTip(tr("Print an image"));
	connect(fileActions[menu_file_print], SIGNAL(triggered()), vp, SLOT(printImage()));

	fileActions[menu_file_reload] = new QAction(tr("&Reload File"), this);
	fileActions[menu_file_reload]->setShortcuts(QKeySequence::Refresh);
	fileActions[menu_file_reload]->setStatusTip(tr("Reload File"));
	connect(fileActions[menu_file_reload], SIGNAL(triggered()), vp, SLOT(reloadFile()));

	fileActions[menu_file_next] = new QAction(fileIcons[icon_file_next], tr("Ne&xt File"), this);
	fileActions[menu_file_next]->setShortcut(QKeySequence(shortcut_next_file));
	fileActions[menu_file_next]->setStatusTip(tr("Load next image"));
	connect(fileActions[menu_file_next], SIGNAL(triggered()), vp, SLOT(loadNextFileFast()));

	fileActions[menu_file_prev] = new QAction(fileIcons[icon_file_prev], tr("Pre&vious File"), this);
	fileActions[menu_file_prev]->setShortcut(QKeySequence(shortcut_prev_file));
	fileActions[menu_file_prev]->setStatusTip(tr("Load previous file"));
	connect(fileActions[menu_file_prev], SIGNAL(triggered()), vp, SLOT(loadPrevFileFast()));

	fileActions[menu_file_new_instance] = new QAction(tr("St&art New Instance"), this);
	fileActions[menu_file_new_instance]->setShortcut(QKeySequence(shortcut_new_instance));
	fileActions[menu_file_new_instance]->setStatusTip(tr("Open file in new instance"));
	connect(fileActions[menu_file_new_instance], SIGNAL(triggered()), this, SLOT(newInstance()));

	fileActions[menu_file_find] = new QAction(tr("&Find && Filter"), this);
	fileActions[menu_file_find]->setShortcut(QKeySequence::Find);
	fileActions[menu_file_find]->setStatusTip(tr("Find an image"));
	connect(fileActions[menu_file_find], SIGNAL(triggered()), this, SLOT(find()));

	//fileActions[menu_file_share_fb] = new QAction(tr("Share on &Facebook"), this);
	////fileActions[menu_file_share_fb]->setShortcuts(QKeySequence::Close);
	//fileActions[menu_file_share_fb]->setStatusTip(tr("Shares the image on facebook"));
	//connect(fileActions[menu_file_share_fb], SIGNAL(triggered()), this, SLOT(shareFacebook()));

	fileActions[menu_file_exit] = new QAction(tr("&Exit"), this);
	fileActions[menu_file_exit]->setShortcuts(QKeySequence::Close);
	fileActions[menu_file_exit]->setStatusTip(tr("Exit"));
	connect(fileActions[menu_file_exit], SIGNAL(triggered()), this, SLOT(close()));

	editActions.resize(menu_edit_end);

	editActions[menu_edit_rotate_cw] = new QAction(editIcons[icon_edit_rotate_cw], tr("9&0° Clockwise"), this);
	editActions[menu_edit_rotate_cw]->setShortcut(QKeySequence(shortcut_rotate_cw));
	editActions[menu_edit_rotate_cw]->setStatusTip(tr("rotate the image 90° clockwise"));
	connect(editActions[menu_edit_rotate_cw], SIGNAL(triggered()), vp, SLOT(rotateCW()));

	editActions[menu_edit_rotate_ccw] = new QAction(editIcons[icon_edit_rotate_ccw], tr("&90° Counter Clockwise"), this);
	editActions[menu_edit_rotate_ccw]->setShortcut(QKeySequence(shortcut_rotate_ccw));
	editActions[menu_edit_rotate_ccw]->setStatusTip(tr("rotate the image 90° counter clockwise"));
	connect(editActions[menu_edit_rotate_ccw], SIGNAL(triggered()), vp, SLOT(rotateCCW()));

	editActions[menu_edit_rotate_180] = new QAction(tr("180°"), this);
	editActions[menu_edit_rotate_180]->setStatusTip(tr("rotate the image by 180°"));
	connect(editActions[menu_edit_rotate_180], SIGNAL(triggered()), vp, SLOT(rotate180()));

	editActions[menu_edit_copy] = new QAction(tr("&Copy"), this);
	editActions[menu_edit_copy]->setShortcut(QKeySequence::Copy);
	editActions[menu_edit_copy]->setStatusTip(tr("copy image"));
	connect(editActions[menu_edit_copy], SIGNAL(triggered()), this, SLOT(copyImage()));

	editActions[menu_edit_copy_buffer] = new QAction(tr("&Copy Buffer"), this);
	editActions[menu_edit_copy_buffer]->setShortcut(shortcut_copy_buffer);
	editActions[menu_edit_copy_buffer]->setStatusTip(tr("copy image"));
	connect(editActions[menu_edit_copy_buffer], SIGNAL(triggered()), this, SLOT(copyImageBuffer()));

	QList<QKeySequence> pastScs;
	pastScs.append(QKeySequence::Paste);
	pastScs.append(shortcut_paste);
	editActions[menu_edit_paste] = new QAction(tr("&Paste"), this);
	editActions[menu_edit_paste]->setShortcuts(pastScs);
	editActions[menu_edit_paste]->setStatusTip(tr("paste image"));
	connect(editActions[menu_edit_paste], SIGNAL(triggered()), this, SLOT(pasteImage()));

	editActions[menu_edit_transfrom] = new QAction(editIcons[icon_edit_resize], tr("R&esize Image"), this);
	editActions[menu_edit_transfrom]->setShortcut(shortcut_transform);
	editActions[menu_edit_transfrom]->setStatusTip(tr("resize the current image"));
	connect(editActions[menu_edit_transfrom], SIGNAL(triggered()), this, SLOT(resizeImage()));

	editActions[menu_edit_crop] = new QAction(editIcons[icon_edit_crop], tr("Cr&op Image"), this);
	editActions[menu_edit_crop]->setShortcut(shortcut_crop);
	editActions[menu_edit_crop]->setStatusTip(tr("cut the current image"));
	editActions[menu_edit_crop]->setCheckable(true);
	editActions[menu_edit_crop]->setChecked(false);
	connect(editActions[menu_edit_crop], SIGNAL(toggled(bool)), vp->getController(), SLOT(showCrop(bool)));

	editActions[menu_edit_delete] = new QAction(tr("&Delete"), this);
	editActions[menu_edit_delete]->setShortcut(QKeySequence::Delete);
	editActions[menu_edit_delete]->setStatusTip(tr("delete current file"));
	connect(editActions[menu_edit_delete], SIGNAL(triggered()), this, SLOT(deleteFile()));

	editActions[menu_edit_wallpaper] = new QAction(tr("&Wallpaper"), this);
	editActions[menu_edit_wallpaper]->setStatusTip(tr("set the current image as wallpaper"));
	connect(editActions[menu_edit_wallpaper], SIGNAL(triggered()), this, SLOT(setWallpaper()));

	editActions[menu_edit_preferences] = new QAction(tr("&Settings"), this);
	editActions[menu_edit_preferences]->setShortcut(QKeySequence(shortcut_settings));
	editActions[menu_edit_preferences]->setStatusTip(tr("settings"));
	connect(editActions[menu_edit_preferences], SIGNAL(triggered()), this, SLOT(openSettings()));

	// view menu
	viewActions.resize(menu_view_end);
	viewActions[menu_view_show_menu] = new QAction(tr("Show &Menu"), this);
	viewActions[menu_view_show_menu]->setStatusTip(tr("hides the menu and shows it again on ALT"));
	viewActions[menu_view_show_menu]->setCheckable(true);
	connect(viewActions[menu_view_show_menu], SIGNAL(toggled(bool)), this, SLOT(showMenuBar(bool)));

	viewActions[menu_view_show_toolbar] = new QAction(tr("Show Tool&bar"), this);
	viewActions[menu_view_show_toolbar]->setShortcut(QKeySequence(shortcut_show_toolbar));
	viewActions[menu_view_show_toolbar]->setStatusTip(tr("Show Toolbar"));
	viewActions[menu_view_show_toolbar]->setCheckable(true);
	connect(viewActions[menu_view_show_toolbar], SIGNAL(toggled(bool)), this, SLOT(showToolbar(bool)));

	viewActions[menu_view_show_statusbar] = new QAction(tr("Show &Statusbar"), this);
	viewActions[menu_view_show_statusbar]->setShortcut(QKeySequence(shortcut_show_statusbar));
	viewActions[menu_view_show_statusbar]->setStatusTip(tr("Show Statusbar"));
	viewActions[menu_view_show_statusbar]->setCheckable(true);
	connect(viewActions[menu_view_show_statusbar], SIGNAL(toggled(bool)), this, SLOT(showStatusBar(bool)));

	// Added by fabian - for transferfunction:
	viewActions[menu_view_show_transfertoolbar] = new QAction(tr("Show &Pseudocolor Function"), this);
	viewActions[menu_view_show_transfertoolbar]->setShortcut(QKeySequence(shortcut_show_transfer));
	viewActions[menu_view_show_transfertoolbar]->setStatusTip(tr("Show Pseudocolor Function"));
	viewActions[menu_view_show_transfertoolbar]->setCheckable(true);
	viewActions[menu_view_show_transfertoolbar]->setChecked(false);
	connect(viewActions[menu_view_show_transfertoolbar], SIGNAL(toggled(bool)), this, SLOT(setContrast(bool)));

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
	viewActions[menu_view_anti_aliasing]->setChecked(DkSettings::Display::antiAliasing);
	connect(viewActions[menu_view_anti_aliasing], SIGNAL(toggled(bool)), vp->getImageStorage(), SLOT(antiAliasingChanged(bool)));

	viewActions[menu_view_show_overview] = new QAction(tr("Show O&verview"), this);
	viewActions[menu_view_show_overview]->setShortcut(QKeySequence(shortcut_show_overview));
	viewActions[menu_view_show_overview]->setStatusTip(tr("shows the overview or not"));
	viewActions[menu_view_show_overview]->setCheckable(true);
	viewActions[menu_view_show_overview]->setChecked(DkSettings::App::showOverview.testBit(DkSettings::App::currentAppMode));
	connect(viewActions[menu_view_show_overview], SIGNAL(toggled(bool)), vp->getController(), SLOT(showOverview(bool)));

	viewActions[menu_view_show_player] = new QAction(tr("Show Pla&yer"), this);
	viewActions[menu_view_show_player]->setShortcut(QKeySequence(shortcut_show_player));
	viewActions[menu_view_show_player]->setStatusTip(tr("shows the player or not"));
	viewActions[menu_view_show_player]->setCheckable(true);
	connect(viewActions[menu_view_show_player], SIGNAL(toggled(bool)), vp->getController(), SLOT(showPlayer(bool)));

	viewActions[menu_view_show_preview] = new QAction(tr("Sho&w Thumbnails"), this);
	viewActions[menu_view_show_preview]->setShortcut(QKeySequence(shortcut_open_preview));
	viewActions[menu_view_show_preview]->setStatusTip(tr("Show thumbnails"));
	viewActions[menu_view_show_preview]->setCheckable(true);
	connect(viewActions[menu_view_show_preview], SIGNAL(toggled(bool)), vp->getController(), SLOT(showPreview(bool)));

	viewActions[menu_view_show_exif] = new QAction(tr("Show &Metadata"), this);
	viewActions[menu_view_show_exif]->setShortcut(QKeySequence(shortcut_show_exif));
	viewActions[menu_view_show_exif]->setStatusTip(tr("shows the metadata panel"));
	viewActions[menu_view_show_exif]->setCheckable(true);
	connect(viewActions[menu_view_show_exif], SIGNAL(toggled(bool)), vp->getController(), SLOT(showMetaData(bool)));

	viewActions[menu_view_show_info] = new QAction(tr("Show File &Info"), this);
	viewActions[menu_view_show_info]->setShortcut(QKeySequence(shortcut_show_info));
	viewActions[menu_view_show_info]->setStatusTip(tr("shows the info panel"));
	viewActions[menu_view_show_info]->setCheckable(true);
	connect(viewActions[menu_view_show_info], SIGNAL(toggled(bool)), vp->getController(), SLOT(showFileInfo(bool)));

	viewActions[menu_view_show_histogram] = new QAction(tr("Show &Histogram"), this);
	viewActions[menu_view_show_histogram]->setShortcut(QKeySequence(shortcut_show_histogram));
	viewActions[menu_view_show_histogram]->setStatusTip(tr("shows the image histogram panel"));
	viewActions[menu_view_show_histogram]->setCheckable(true);
	connect(viewActions[menu_view_show_histogram], SIGNAL(toggled(bool)), vp->getController(), SLOT(showHistogram(bool)));

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

	connect(viewActions[menu_view_lock_window], SIGNAL(toggled(bool)), this, SLOT(lockWindow(bool)));

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

	toolsActions[menu_tools_filter] = new QAction(fileIcons[icon_file_filter], tr("&Find && Filter"), this);
	toolsActions[menu_tools_filter]->setStatusTip(tr("Find an image"));
	toolsActions[menu_tools_filter]->setCheckable(true);
	toolsActions[menu_tools_filter]->setChecked(false);
	connect(toolsActions[menu_tools_filter], SIGNAL(toggled(bool)), this, SLOT(find(bool)));

	toolsActions[menu_tools_manipulation] = new QAction(toolsIcons[icon_tools_manipulation], tr("Image &Manipulation"), this);
	toolsActions[menu_tools_manipulation]->setShortcut(shortcut_manipulation);
	toolsActions[menu_tools_manipulation]->setStatusTip(tr("modify the current image"));
	connect(toolsActions[menu_tools_manipulation], SIGNAL(triggered()), this, SLOT(openImgManipulationDialog()));

	// help menu
	helpActions.resize(menu_help_end);
	helpActions[menu_help_about] = new QAction(tr("&About Nomacs"), this);
	helpActions[menu_help_about]->setShortcut(QKeySequence(shortcut_show_help));
	helpActions[menu_help_about]->setStatusTip(tr("about"));
	connect(helpActions[menu_help_about], SIGNAL(triggered()), this, SLOT(aboutDialog()));

	helpActions[menu_help_bug] = new QAction(tr("&Report a Bug"), this);
	helpActions[menu_help_bug]->setStatusTip(tr("Report a Bug"));
	connect(helpActions[menu_help_bug], SIGNAL(triggered()), this, SLOT(bugReport()));

	helpActions[menu_help_feature] = new QAction(tr("&Feature Request"), this);
	helpActions[menu_help_feature]->setStatusTip(tr("Feature Request"));
	connect(helpActions[menu_help_feature], SIGNAL(triggered()), this, SLOT(featureRequest()));

	helpActions[menu_help_update] = new QAction(tr("&Check for Updates"), this);
	helpActions[menu_help_update]->setStatusTip(tr("check for updates"));
	connect(helpActions[menu_help_update], SIGNAL(triggered()), this, SLOT(checkForUpdate()));

}

void DkNoMacs::createShortcuts() {

	DkViewPort* vp = viewport();

	shortcuts.resize(sc_end);

	// panning
	shortcuts[sc_pan_left] = new QShortcut(shortcut_panning_left, this);
	QObject::connect( shortcuts[sc_pan_left], SIGNAL( activated ()), vp, SLOT(shiftLeft()));
	shortcuts[sc_pan_right] = new QShortcut(shortcut_panning_right, this);
	QObject::connect( shortcuts[sc_pan_right], SIGNAL( activated ()), vp, SLOT( shiftRight() ));
	shortcuts[sc_pan_up] = new QShortcut(shortcut_panning_up, this);
	QObject::connect(shortcuts[sc_pan_up], SIGNAL( activated ()), vp, SLOT( shiftUp() ));
	shortcuts[sc_pan_down] = new QShortcut(shortcut_panning_down, this);
	QObject::connect(shortcuts[sc_pan_down], SIGNAL( activated ()), vp, SLOT( shiftDown() ));

	// files
	shortcuts[sc_first_file] = new QShortcut(shortcut_first_file, this);
	QObject::connect(shortcuts[sc_first_file], SIGNAL( activated ()), vp, SLOT( loadFirst() ));
	shortcuts[sc_last_file] = new QShortcut(shortcut_last_file, this);
	QObject::connect(shortcuts[sc_last_file], SIGNAL( activated ()), vp, SLOT( loadLast() ));

	shortcuts[sc_skip_prev] = new QShortcut(shortcut_skip_prev, this);
	QObject::connect(shortcuts[sc_skip_prev], SIGNAL( activated()), vp, SLOT( loadSkipPrev10() ));
	shortcuts[sc_skip_next] = new QShortcut(shortcut_skip_next, this);
	QObject::connect(shortcuts[sc_skip_next], SIGNAL( activated ()), vp, SLOT( loadSkipNext10() ));


	shortcuts[sc_test_img] = new QShortcut(shortcut_test_img, this);
	QObject::connect(shortcuts[sc_test_img], SIGNAL( activated ()), vp, SLOT( loadLena() ));

	shortcuts[sc_test_rec] = new QShortcut(shortcut_test_rec, this);
	QObject::connect(shortcuts[sc_test_rec], SIGNAL( activated ()), this, SLOT( loadRecursion() ));

	shortcuts[sc_first_sync] = new QShortcut(shortcut_first_file_sync, this);
	QObject::connect(shortcuts[sc_first_sync], SIGNAL( activated ()), vp, SLOT( loadFirst() ));

	shortcuts[sc_last_sync] = new QShortcut(shortcut_last_file_sync, this);
	QObject::connect(shortcuts[sc_last_sync], SIGNAL( activated ()), vp, SLOT( loadLast() ));

	shortcuts[sc_next_sync] = new QShortcut(shortcut_next_file_sync, this);
	QObject::connect(shortcuts[sc_next_sync], SIGNAL( activated ()), vp, SLOT( loadNextFileFast() ));

	shortcuts[sc_prev_sync] = new QShortcut(shortcut_prev_file_sync, this);
	QObject::connect(shortcuts[sc_prev_sync], SIGNAL( activated ()), vp, SLOT( loadPrevFileFast() ));

	shortcuts[sc_zoom_in] = new QShortcut(shortcut_zoom_in, this);
	connect(shortcuts[sc_zoom_in], SIGNAL(activated()), vp, SLOT(zoomIn()));

	shortcuts[sc_zoom_out] = new QShortcut(shortcut_zoom_out, this);
	connect(shortcuts[sc_zoom_out], SIGNAL(activated()), vp, SLOT(zoomOut()));

	shortcuts[sc_zoom_in_alt] = new QShortcut(shortcut_zoom_in_alt, this);
	connect(shortcuts[sc_zoom_in_alt], SIGNAL(activated()), vp, SLOT(zoomIn()));

	shortcuts[sc_zoom_out_alt] = new QShortcut(shortcut_zoom_out_alt, this);
	connect(shortcuts[sc_zoom_out_alt], SIGNAL(activated()), vp, SLOT(zoomOut()));

	//shortcuts[sc_send_img] = new QShortcut(shortcut_send_img, this);
	//connect(shortcuts[sc_send_img], SIGNAL(activated()), vp, SLOT(tcpSendImage()));

	shortcuts[sc_delete_silent] = new QShortcut(shortcut_delete_silent, this);
	connect(shortcuts[sc_delete_silent], SIGNAL(activated()), vp->getImageLoader(), SLOT(deleteFile()));
}

void DkNoMacs::enableNoImageActions(bool enable) {

	fileActions[menu_file_save]->setEnabled(enable);
	fileActions[menu_file_rename]->setEnabled(enable);
	fileActions[menu_file_open_with]->setEnabled(enable);
	fileActions[menu_file_print]->setEnabled(enable);
	fileActions[menu_file_reload]->setEnabled(enable);
	fileActions[menu_file_prev]->setEnabled(enable);
	fileActions[menu_file_next]->setEnabled(enable);
	fileActions[menu_file_goto]->setEnabled(enable);

	editActions[menu_edit_rotate_cw]->setEnabled(enable);
	editActions[menu_edit_rotate_ccw]->setEnabled(enable);
	editActions[menu_edit_rotate_180]->setEnabled(enable);
	editActions[menu_edit_delete]->setEnabled(enable);
	editActions[menu_edit_transfrom]->setEnabled(enable);
	editActions[menu_edit_crop]->setEnabled(enable);
	editActions[menu_edit_copy]->setEnabled(enable);
	editActions[menu_edit_copy_buffer]->setEnabled(enable);
	editActions[menu_edit_wallpaper]->setEnabled(enable);

	toolsActions[menu_tools_thumbs]->setEnabled(enable);
	
	viewActions[menu_view_show_info]->setEnabled(enable);
#ifdef WITH_OPENCV
	viewActions[menu_view_show_histogram]->setEnabled(enable);
#else
	viewActions[menu_view_show_histogram]->setEnabled(false);
#endif
	viewActions[menu_view_show_preview]->setEnabled(enable);
	viewActions[menu_view_show_exif]->setEnabled(enable);
	viewActions[menu_view_show_overview]->setEnabled(enable);
	viewActions[menu_view_show_player]->setEnabled(enable);
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

	emit closeSignal();
	qDebug() << "saving window settings...";
	setVisible(false);
	showNormal();

	if (viewport())
		viewport()->unloadImage();

	if (saveSettings) {
		QSettings settings;
		settings.setValue("geometry", saveGeometry());
		settings.setValue("windowState", saveState());

		DkSettings ourSettings;
		ourSettings.save();
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
}


void DkNoMacs::mousePressEvent(QMouseEvent* event) {

	mousePos = event->pos();
}

void DkNoMacs::mouseReleaseEvent(QMouseEvent *event) {
}

void DkNoMacs::contextMenuEvent(QContextMenuEvent *event) {

	contextMenu->exec(event->globalPos());
}

void DkNoMacs::mouseMoveEvent(QMouseEvent *event) {

	int dist = QPoint(event->pos()-mousePos).manhattanLength();

	if (event->buttons() == Qt::LeftButton 
			&& dist > QApplication::startDragDistance()
			&& viewport() 
			&& viewport()->getWorldMatrix().m11() <= 1.0f
			&& !viewport()->getImage().isNull()
			&& viewport()->getImageLoader()) {

			QUrl fileUrl = QUrl("file:///" + viewport()->getImageLoader()->getFile().absoluteFilePath());

			QList<QUrl> urls;
			urls.append(fileUrl);

			QMimeData* mimeData = new QMimeData;
			
			if (viewport()->getImageLoader()->getFile().exists() && !viewport()->getImageLoader()->isEdited())
				mimeData->setUrls(urls);
			else if (!viewport()->getImage().isNull())
				mimeData->setImageData(viewport()->getImage());

			QDrag* drag = new QDrag(this);
			drag->setMimeData(mimeData);
			Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
			qDebug() << "creating drag...\n";
	}
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

	if (event->mimeData()->hasFormat("network/sync-dir")) {
		event->accept();
	}
	if (event->mimeData()->hasUrls()) {
		QUrl url = event->mimeData()->urls().at(0);
		url = url.toLocalFile();
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
		
		viewport()->loadFile(QFileInfo(url.toString()), true);

		QList<QUrl> urls = event->mimeData()->urls();
		for (int idx = 1; idx < urls.size() && idx < 20; idx++)
			newInstance(QFileInfo(urls[idx].toLocalFile()));
		
	}
	else if (event->mimeData()->hasImage()) {

		QImage dropImg = qvariant_cast<QImage>(event->mimeData()->imageData());

		// delete current information
		if (viewport()->getImageLoader()) {
			//viewport()->getImageLoader()->clearPath();
			viewport()->unloadImage();
			viewport()->getImageLoader()->setImage(dropImg);
			viewport()->setImage(dropImg);

			// save to temp folder
			saveTempFileSignal(dropImg);
		}
	}

	qDebug() << "drop event...";
}

void DkNoMacs::copyImage() {
	
	qDebug() << "copying...";

	if (!viewport() || viewport()->getImage().isNull() || !viewport()->getImageLoader())
		return;

	QUrl fileUrl = QUrl("file:///" + viewport()->getImageLoader()->getFile().absoluteFilePath());
	
	QList<QUrl> urls;
	urls.append(fileUrl);

	QMimeData* mimeData = new QMimeData;
	
	if (viewport()->getImageLoader()->getFile().exists() && !viewport()->getImageLoader()->isEdited())
		mimeData->setUrls(urls);
	else if (!viewport()->getImage().isNull())
		mimeData->setImageData(viewport()->getImage());
	
	mimeData->setText(viewport()->getImageLoader()->getFile().absoluteFilePath());

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);

	qDebug() << "copying: " << fileUrl;
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

	QClipboard* clipboard = QApplication::clipboard();

	if (clipboard->mimeData()->hasUrls() && clipboard->mimeData()->urls().size() > 0) {
		QUrl url = clipboard->mimeData()->urls().at(0);
		url = url.toLocalFile();
		qDebug() << "pasting: " << url.toString();
		viewport()->loadFile(QFileInfo(url.toString()), true);

	}
	else if (clipboard->mimeData()->hasImage()) {

		QImage dropImg = qvariant_cast<QImage>(clipboard->mimeData()->imageData());

		if (viewport()) {
			viewport()->unloadImage();

			// delete current information
			if (viewport()->getImageLoader()) {
				
				//viewport()->getImageLoader()->clearPath();
				viewport()->getImageLoader()->setImage(dropImg);
				viewport()->setImage(dropImg);
				qDebug() << "loader path: " << viewport()->getImageLoader()->getFile().absoluteFilePath();	

				saveTempFileSignal(dropImg);
			}
		}
	} else if (clipboard->mimeData()->hasText()) {

		QString msg = clipboard->mimeData()->text();
		QFileInfo file = QFileInfo(msg);

		if (file.exists()) {
			viewport()->loadFile(file, true);
		}
		else
			viewport()->getController()->setInfo("Could not find a valid file url, sorry");
	}
	
	else if (viewport())
		viewport()->getController()->setInfo("Clipboard has no image...");

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
	args.append(viewport()->getImageLoader()->getFile().absoluteFilePath());

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

	DkSettings::App::currentAppMode += DkSettings::mode_end*0.5f;
	if (DkSettings::App::currentAppMode < 0) {
		qDebug() << "illegal state: " << DkSettings::App::currentAppMode;
		DkSettings::App::currentAppMode = DkSettings::mode_default;
	}
	
	menuBar()->hide();
	toolbar->hide();
	statusbar->hide();
	showFullScreen();

	if (viewport())
		viewport()->setFullScreen(true);

	update();
}

void DkNoMacs::exitFullScreen() {

	if (isFullScreen()) {
		DkSettings::App::currentAppMode -= DkSettings::mode_end*0.5f;
		if (DkSettings::App::currentAppMode < 0) {
			qDebug() << "illegal state: " << DkSettings::App::currentAppMode;
			DkSettings::App::currentAppMode = DkSettings::mode_default;
		}

		if (DkSettings::App::showMenuBar) menu->show();
		if (DkSettings::App::showToolBar) toolbar->show();
		if (DkSettings::App::showStatusBar) statusbar->show();
		showNormal();
	}

	if (viewport())
		viewport()->setFullScreen(false);
}

void DkNoMacs::setFrameless(bool frameless) {

	if (!viewport()) 
		return;

	QString exe = QApplication::applicationFilePath();
	QStringList args;
	args.append(viewport()->getImageLoader()->getFile().absoluteFilePath());
	
	if (objectName() != "DkNoMacsFrameless") {
		DkSettings::App::appMode = DkSettings::mode_frameless;
        //args.append("-graphicssystem");
        //args.append("native");
    } else {
		DkSettings::App::appMode = DkSettings::mode_default;
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

	
#ifdef Q_WS_WIN
	
	qDebug() << "locking: " << lock;

	if (lock && windowOpacity() < 1.0f) {
		//setAttribute(Qt::WA_TransparentForMouseEvents);
		HWND hwnd = (HWND) winId(); // get handle of the widget
		LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
		SetWindowLong(hwnd, GWL_EXSTYLE, styles | WS_EX_TRANSPARENT); 
		SetWindowPos(this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		viewport()->getController()->setInfo(tr("Window Locked\nTo unlock: gain focus (ALT+Tab),\nthen press CTRL+SHIFT+ALT+B"), 5000);
	}
	else if (lock && windowOpacity() == 1.0f) {
		viewport()->getController()->setInfo(tr("You should first reduce opacity\n before working through the window."));
	}
	else {
		qDebug() << "deactivating...";
		HWND hwnd = (HWND) winId(); // get handle of the widget
		LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
		SetWindowLong(hwnd, GWL_EXSTYLE, styles & ~WS_EX_TRANSPARENT); 

		SetWindowPos(this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
#else
	// TODO: find corresponding command for linux etc

	//setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	//show();
#endif
}

void DkNoMacs::newClientConnected() {
	overlaid = false;
	// add methods if clients are connected
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

		Qt::WindowFlags flags = windowFlags();
		setWindowFlags(Qt::WindowStaysOnTopHint);	// we need this to 'generally' (for all OSs) bring the window to front
		setWindowFlags(flags);	// reset flags

		showNormal();

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

	// change my geometry
	tcpSetWindowRect(this->frameGeometry(), !overlaid, overlaid);

	qDebug() << "overlaying";

	emit sendPositionSignal(frameGeometry(), overlaid);

};

void DkNoMacs::tcpSendArrange() {
	
	overlaid = !overlaid;
	emit sendArrangeSignal(overlaid);
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
		DkImageLoader::openFilter);

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

void DkNoMacs::renameFile() {

	DkImageLoader* loader = viewport()->getImageLoader();

	if (!loader)
		return;

	QFileInfo file = loader->getFile();

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

	if (!viewport() || !viewport()->getImageLoader())
		return;

	if (filterAction) {
		DkSearchDialog* searchDialog = new DkSearchDialog(this);
		searchDialog->setFiles(viewport()->getImageLoader()->getFiles());
		searchDialog->setPath(viewport()->getImageLoader()->getDir());

		connect(searchDialog, SIGNAL(filterSignal(QStringList)), viewport()->getImageLoader(), SLOT(setFolderFilters(QStringList)));
		connect(searchDialog, SIGNAL(loadFileSignal(QFileInfo)), viewport()->getImageLoader(), SLOT(loadFile(QFileInfo)));
		searchDialog->exec();
		
	}
	else {
		// remove the filter 
		viewport()->getImageLoader()->setFolderFilters(QStringList());
	}


}

void DkNoMacs::updateFilterState(QStringList filters) {
	
	toolsActions[menu_tools_filter]->blockSignals(true);
	toolsActions[menu_tools_filter]->setChecked(!filters.empty());
	toolsActions[menu_tools_filter]->blockSignals(false);
}

void DkNoMacs::goTo() {

	if (!viewport() || !viewport()->getImageLoader())
		return;

	DkImageLoader* loader = viewport()->getImageLoader();
	
	bool ok = false;
	int fileIdx = QInputDialog::getInt(this, tr("Go To Image"), tr("Image Index:"), 0, 0, loader->numFiles()-1, 1, &ok);

	if (ok)
		loader->loadFileAt(fileIdx);

}

void DkNoMacs::saveFile() {
	
	qDebug() << "saving...";

	DkImageLoader* loader = viewport()->getImageLoader();

	QString selectedFilter;
	QString saveName;
	QFileInfo saveFile;

	if (loader->hasFile()) {
		saveFile = loader->getFile();
		saveName = saveFile.fileName();
		
		qDebug() << "save dir: " << loader->getSaveDir();

		if (loader->getSaveDir() != saveFile.absoluteDir())
			saveFile = QFileInfo(loader->getSaveDir(), saveName);

		int filterIdx = -1;

		QStringList sF = DkImageLoader::saveFilter.split(";;");
		qDebug() << sF;

		QRegExp exp = QRegExp("*." + saveFile.suffix() + "*", Qt::CaseInsensitive);
		exp.setPatternSyntax(QRegExp::Wildcard);

		for (int idx = 0; idx < sF.size(); idx++) {

			
			qDebug() << exp;
			qDebug() << saveFile.suffix();
			qDebug() << sF.at(idx);

			if (exp.exactMatch(sF.at(idx))) {
				selectedFilter = sF.at(idx);
				filterIdx = idx;
				break;
			}
		}

		if (filterIdx == -1)
			saveName.remove("." + saveFile.suffix());
	}

	// note: basename removes the whole file name from the first dot...
	QString savePath = (!selectedFilter.isEmpty()) ? saveFile.absoluteFilePath() : QFileInfo(saveFile.absoluteDir(), saveName).absoluteFilePath();

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File %1").arg(saveName),
		savePath, DkImageLoader::saveFilter, &selectedFilter);

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

		QStringList sF = DkImageLoader::saveFilter.split(";;");

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
	if (selectedFilter.contains("jpg")) {

		if (!jpgDialog)
			jpgDialog = new DkJpgDialog(this);

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

	//if (saveDialog->selectedNameFilter().contains("tif")) {
	if (selectedFilter.contains("tif")) {

		if (!tifDialog)
			tifDialog = new DkTifDialog(this);

		if (!tifDialog->exec())
			return;

		compression = tifDialog->getCompression();
	}


	if (loader)
		loader->saveFile(sFile, selectedFilter, saveImg, compression);

}

void DkNoMacs::resizeImage() {

	if (!viewport() || viewport()->getImage().isNull())
		return;

	if (!resizeDialog)
		resizeDialog = new DkResizeDialog(this);


	DkMetaDataInfo* metaData = viewport()->getController()->getMetaDataWidget();
	
	if (metaData) {
		float xDpi, yDpi;
		metaData->getResolution(xDpi, yDpi);
		resizeDialog->setExifDpi(xDpi);
	}

	resizeDialog->setImage(viewport()->getImageLoader()->getImage());

	if (resizeDialog->exec()) {

		if (resizeDialog->resample()) {

			//// do not load the old image -> as the transformed image is not the same anymore
			//viewport()->getImageLoader()->enableWatcher(false);
			//viewport()->getImageLoader()->clearFileWatcher();

			// TODO: redirect resize to basic loader here
			QImage rImg = resizeDialog->getResizedImage();

			if (!rImg.isNull()) {

				// this reloads the image -> that's not what we want!
				if (metaData)
					metaData->setResolution((int)resizeDialog->getExifDpi(), (int)resizeDialog->getExifDpi());

				viewport()->setEditedImage(rImg);
			}


		}
		else if (metaData) {
			// ok, user just wants to change the resolution
			metaData->setResolution((int)resizeDialog->getExifDpi(), (int)resizeDialog->getExifDpi());
		}
	}
}

void DkNoMacs::deleteFile() {

	if (!viewport() || viewport()->getImage().isNull())
		return;

	qDebug() << "yep deleting...";

	QFileInfo file = viewport()->getImageLoader()->getFile();

	if (infoDialog(tr("Do you want to permanently delete %1").arg(file.fileName()), this) == QMessageBox::Yes)
		viewport()->getImageLoader()->deleteFile();
}

void DkNoMacs::openImgManipulationDialog() {

	if (!viewport() || viewport()->getImage().isNull())
		return;

	if (!imgManipulationDialog)
		imgManipulationDialog = new DkImageManipulationDialog(this);
	else 
		imgManipulationDialog->resetValues();

	QImage tmpImg = viewport()->getImageLoader()->getImage();
	imgManipulationDialog->setImage(&tmpImg);

	bool done = imgManipulationDialog->exec();

	if (imgManipulationDialog->wasOkPressed()) {

#ifdef WITH_OPENCV

		QImage mImg = DkImage::mat2QImage(DkImageManipulationWidget::manipulateImage(DkImage::qImage2Mat(viewport()->getImageLoader()->getImage())));

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
	QFileInfo tmpPath = viewport()->getImageLoader()->saveTempFile(dImg, "wallpaper", ".jpg", true, false);

	// is there a more elegant way to see if saveTempFile returned an empty path
	if (tmpPath.absoluteFilePath() == QFileInfo().absoluteFilePath()) {
		errorDialog(tr("Sorry, I could not create a wallpaper..."));
		return;
	}

#ifdef WIN32

#include <stdio.h>
#include <windows.h>

	//Read current windows background image path
	QSettings appSettings( "HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
	appSettings.setValue("Wallpaper", tmpPath.absoluteFilePath());

	QByteArray ba = tmpPath.absoluteFilePath().toLatin1();
	SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)ba.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#endif
	// TODO: add functionality for unix based systems

}

void DkNoMacs::computeThumbsBatch() {

	if (!viewport() || !viewport()->getImageLoader())
		return;

	// TODO: make private -> so that it gets destroyed
	DkThumbsSaver* saver = new DkThumbsSaver();
	saver->processDir(viewport()->getImageLoader()->getDir());

}

void DkNoMacs::aboutDialog() {

	DkSplashScreen* spScreen = new DkSplashScreen(this, 0/*, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint*/);
	spScreen->exec();
	delete spScreen;
}

void DkNoMacs::bugReport() {

	QString url = QString("http://www.nomacs.org/redmine/projects/nomacs/")
		% QString("issues/new?issue[tracker_id]=1&issue[custom_field_values][1]=")
		% QApplication::applicationVersion();
	
	QDesktopServices::openUrl(QUrl(url));
}

void DkNoMacs::featureRequest() {
	
	QString url = QString("http://www.nomacs.org/redmine/projects/nomacs/")
		% QString("issues/new?issue[tracker_id]=2&issue[custom_field_values][1]=")
		% QApplication::applicationVersion();

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
		args.append(viewport()->getImageLoader()->getFile().absoluteFilePath());
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
	args.append(viewport()->getImageLoader()->getFile().absoluteFilePath());
	
	if (contrast)
		DkSettings::App::appMode = DkSettings::mode_contrast;
	else
		DkSettings::App::appMode = DkSettings::mode_default;

	bool started = process.startDetached(exe, args);

	// close me if the new instance started
	if (started)
		close();

	qDebug() << "contrast arguments: " << args;
}

//bool DkNoMacs::event(QEvent *event) {
//
//	if (event->type() > QEvent::User)
//		qDebug() << "user event??";
//
//	//if (event->type() == DkInfoEvent::type()) {
//
//	//	DkInfoEvent *infoEvent = static_cast<DkInfoEvent*>(event);
//	//	viewport()->setInfo(infoEvent->getMessage(), infoEvent->getTime(), infoEvent->getInfoType());
//	//	return true;
//	//}
//	//if (event->type() == DkLoadImageEvent::type()) {
//
//	//	DkLoadImageEvent *imgEvent = static_cast<DkLoadImageEvent*>(event);
//	//	
//	//	if (!imgEvent->getImage().isNull())
//	//		viewport()->setImage(imgEvent->getImage());
//	//	viewport()->setWindowTitle(imgEvent->getTitle(), imgEvent->getAttr());
//	//	return true;
//	//}
//
//	if (event->type() == QEvent::Gesture)
//		return gestureEvent(static_cast<QGestureEvent*>(event));
//
//	return QMainWindow::event(event);
//}

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
	}
	if (event->type() == QEvent::Gesture) {
		return gestureEvent(static_cast<QGestureEvent*>(event));
	}

	return false;
}

void DkNoMacs::showMenuBar(bool show) {

	DkSettings::App::showMenuBar = show;
	int tts = (DkSettings::App::showMenuBar) ? -1 : 5000;
	viewActions[menu_view_show_menu]->setChecked(DkSettings::App::showMenuBar);
	menu->setTimeToShow(tts);
	menu->showMenu();
}

void DkNoMacs::showToolbar(bool show) {

	DkSettings::App::showToolBar = show;
	viewActions[menu_view_show_toolbar]->setChecked(DkSettings::App::showToolBar);
	
	if (DkSettings::App::showToolBar)
		toolbar->show();
	else
		toolbar->hide();
}

void DkNoMacs::showStatusBar(bool show, bool permanent) {

	if (statusbar->isVisible() == show)
		return;

	if (permanent)
		DkSettings::App::showStatusBar = show;
	viewActions[menu_view_show_statusbar]->setChecked(DkSettings::App::showStatusBar);

	statusbar->setVisible(show);

	viewport()->setVisibleStatusbar(show);
}

void DkNoMacs::showStatusMessage(QString msg) {

	statusbarMsg->setText(msg);
}

void DkNoMacs::openFileWith() {

	if (DkSettings::Global::showDefaultAppDialog) {
		if (!openWithDialog) openWithDialog = new DkOpenWithDialog(this);
		
		if (!openWithDialog->exec())
			return;
	}

	QStringList args;
	
	if (QFileInfo(DkSettings::Global::defaultAppPath).fileName() == "explorer.exe") {
		args << "/select," + QDir::toNativeSeparators(viewport()->getImageLoader()->getFile().absoluteFilePath());
		qDebug() << "explorer.exe started...";
	}
	else
		args << QDir::toNativeSeparators(viewport()->getImageLoader()->getFile().absoluteFilePath());

	//bool started = process.startDetached("psOpenImages.exe", args);	// already deprecated
	bool started = process.startDetached(DkSettings::Global::defaultAppPath, args);

	if (started)
		qDebug() << "starting: " << DkSettings::Global::defaultAppPath;
	else {
		viewport()->getController()->setInfo("Sorry, I could not start: " % DkSettings::Global::defaultAppPath);
		DkSettings::Global::showDefaultAppDialog = true;
	}

	qDebug() << "I'm trying to execute: " << args[0];

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

QVector <QAction* > DkNoMacs::getViewActions() {

	return viewActions;
}

QVector <QAction* > DkNoMacs::getSyncActions() {

	return syncActions;
}

void DkNoMacs::setWindowTitle(QFileInfo file, QSize size, bool edited) {

	////  do not tell the viewport (he should know it)
	//viewport()->setTitleLabel(file, -1);

	QString title = file.fileName();
	title = title.remove(".lnk");
	
	if (!file.exists())
		title = "nomacs";
	else
		setWindowModified(edited);
	title.append("[*]");

	QString attributes;

	if (!size.isEmpty())
		attributes.sprintf(" - %i x %i", size.width(), size.height());
	if (size.isEmpty() && viewport())
		attributes.sprintf(" - %i x %i", viewport()->getImage().width(), viewport()->getImage().height());

	QMainWindow::setWindowTitle(title.append(attributes));
    setWindowFilePath(file.absoluteFilePath());
	emit sendTitleSignal(windowTitle());

}

void DkNoMacs::openSettings() {

	DkSettingsDialog dsd = DkSettingsDialog(this);
	connect(&dsd, SIGNAL(setToDefaultSignal()), this, SLOT(cleanSettings()));
	connect(&dsd, SIGNAL(settingsChanged()), viewport(), SLOT(settingsChanged()));
	connect(&dsd, SIGNAL(languageChanged()), this, SLOT(restart()));
	connect(&dsd, SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));
	dsd.exec();

	qDebug() << "hier könnte ihre werbung stehen...";
}

void DkNoMacs::settingsChanged() {
	
	if (!isFullScreen()) {
		showMenuBar(DkSettings::App::showMenuBar);
		showToolbar(DkSettings::App::showToolBar);
		showStatusBar(DkSettings::App::showStatusBar);
	}
}

void DkNoMacs::checkForUpdate() {

	if (!updater) {
		updater = new DkUpdater();
		connect(updater, SIGNAL(displayUpdateDialog(QString, QString)), this, SLOT(showUpdateDialog(QString, QString)));
	}
	updater->silent = false;
	updater->checkForUpdated();

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

	DkSettings::Sync::updateDialogShown = true;

	DkSettings settings;
	settings.save();
	
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
		QString msg = tr("Unable to install new Version") + "<br><a href=\"file:///" + filePath + "\">"+ filePath +"</a><br>" + tr("Click the file to try install again");
		showUpdaterMessage(msg, "update");
	}
}

void DkNoMacs::errorDialog(QString msg, QString title) {

	dialog(msg, this, title);
}

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


// DkNoMacsSync --------------------------------------------------------------------
DkNoMacsSync::DkNoMacsSync(QWidget *parent, Qt::WFlags flags) : DkNoMacs(parent, flags) {

}

DkNoMacsSync::~DkNoMacsSync() {

	if (localClient) {

		// terminate local client
		localClient->quit();
		localClient->wait();

		delete localClient;
		localClient = 0;
	}

}

void DkNoMacsSync::initLanClient() {

	if (lanClient) {

		lanClient->quit();
		lanClient->wait();

		delete lanClient;
	}

	if (!DkSettings::Sync::enableNetworkSync) {

		lanClient = 0;

		tcpLanMenu->setEnabled(false);
		return;
	}

	tcpLanMenu->clear();

	// start lan client/server
	lanClient = new DkLanManagerThread(this);
	lanClient->start();

	// start server action
	QAction* startServer = new QAction(tr("Start &Server"), this);
	startServer->setCheckable(true);
	startServer->setChecked(false);
	connect(startServer, SIGNAL(toggled(bool)), lanClient, SLOT(startServer(bool)));	// TODO: something that makes sense...

	QAction* sendImage = new QAction(tr("Send &Image"), this);
	sendImage->setShortcut(QKeySequence(shortcut_send_img));
	sendImage->setToolTip(tr("Sends the current image to all clients."));
	connect(sendImage, SIGNAL(triggered()), viewport(), SLOT(tcpSendImage()));

	tcpLanMenu->setClientManager(lanClient);
	tcpLanMenu->addTcpAction(startServer);
	tcpLanMenu->addTcpAction(sendImage);
	tcpLanMenu->setEnabled(true);
}

void DkNoMacsSync::createActions() {

	DkNoMacs::createActions();

	DkViewPort* vp = viewport();

	// sync menu
	syncActions.resize(menu_sync_end);
	syncActions[menu_sync] = new QAction(tr("Synchronize &View"), this);
	syncActions[menu_sync]->setShortcut(QKeySequence(shortcut_sync));
	syncActions[menu_sync]->setStatusTip(tr("synchronize the current view"));
	connect(syncActions[menu_sync], SIGNAL(triggered()), vp, SLOT(tcpSynchronize()));

	syncActions[menu_sync_pos] = new QAction(tr("&Window Overlay"), this);
	syncActions[menu_sync_pos]->setShortcut(QKeySequence(shortcut_tab));
	syncActions[menu_sync_pos]->setStatusTip(tr("toggle the window opacity"));
	connect(syncActions[menu_sync_pos], SIGNAL(triggered()), this, SLOT(tcpSendWindowRect()));

	syncActions[menu_sync_arrange] = new QAction(tr("Arrange Instances"), this);
	syncActions[menu_sync_arrange]->setShortcut(QKeySequence(shortcut_arrange));
	syncActions[menu_sync_arrange]->setStatusTip(tr("arrange connected instances"));
	connect(syncActions[menu_sync_arrange], SIGNAL(triggered()), this, SLOT(tcpSendArrange()));

	syncActions[menu_sync_connect_all] = new QAction(tr("Connect &all"), this);
	syncActions[menu_sync_connect_all]->setShortcut(QKeySequence(shortcut_connect_all));
	syncActions[menu_sync_connect_all]->setStatusTip(tr("connect all instances"));
	connect(syncActions[menu_sync_connect_all], SIGNAL(triggered()), this, SLOT(tcpConnectAll()));
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

	syncMenu->addAction(syncActions[menu_sync]);
	syncMenu->addAction(syncActions[menu_sync_pos]);
	syncMenu->addAction(syncActions[menu_sync_arrange]);

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

void DkNoMacsSync::settingsChanged() {
	initLanClient();

	DkNoMacs::settingsChanged();
}

void DkNoMacsSync::clientInitialized() {
	//TODO: things that need to be done after the clientManager has finished initialization
	emit clientInitializedSignal();
}


DkNoMacsIpl::DkNoMacsIpl(QWidget *parent, Qt::WFlags flags) : DkNoMacsSync(parent, flags) {

		// init members
	DkViewPort* vp = new DkViewPort(this);
	vp->setAlignment(Qt::AlignHCenter);
	setCentralWidget(vp);

	localClient = new DkLocalManagerThread(this);
	localClient->start();

	lanClient = 0;

	init();
	setAcceptDrops(true);
	setMouseTracking (true);	//receive mouse event everytime

	updater = new DkUpdater();
	connect(updater, SIGNAL(displayUpdateDialog(QString, QString)), this, SLOT(showUpdateDialog(QString, QString)));
	connect(updater, SIGNAL(showUpdaterMessage(QString, QString)), this, SLOT(showUpdaterMessage(QString, QString)));

#ifndef Q_WS_X11
	if (!DkSettings::Sync::updateDialogShown && QDate::currentDate() > DkSettings::Sync::lastUpdateCheck)
		updater->checkForUpdated();	// TODO: is threaded??

#endif
	
	// title signals
	connect(vp, SIGNAL(windowTitleSignal(QFileInfo, QSize, bool)), this, SLOT(setWindowTitle(QFileInfo, QSize, bool)));
	connect(vp->getImageLoader(), SIGNAL(updateFileSignal(QFileInfo, QSize, bool)), this, SLOT(setWindowTitle(QFileInfo, QSize, bool)));
	connect(vp->getImageLoader(), SIGNAL(newErrorDialog(QString, QString)), this, SLOT(errorDialog(QString, QString)));
	connect(this, SIGNAL(saveTempFileSignal(QImage)), vp->getImageLoader(), SLOT(saveTempFile(QImage)));
	connect(vp, SIGNAL(statusInfoSignal(QString)), this, SLOT(showStatusMessage(QString)));
	connect(vp, SIGNAL(enableNoImageSignal(bool)), this, SLOT(enableNoImageActions(bool)));
	connect(vp, SIGNAL(newClientConnectedSignal()), this, SLOT(newClientConnected()));
	connect(viewport()->getController()->getMetaDataWidget(), SIGNAL(enableGpsSignal(bool)), viewActions[menu_view_gps_map], SLOT(setEnabled(bool)));
	connect(vp->getImageLoader(), SIGNAL(folderFiltersChanged(QStringList)), this, SLOT(updateFilterState(QStringList)));

	vp->getController()->getFilePreview()->registerAction(viewActions[menu_view_show_preview]);
	vp->getController()->getMetaDataWidget()->registerAction(viewActions[menu_view_show_exif]);
	vp->getController()->getPlayer()->registerAction(viewActions[menu_view_show_player]);
	vp->getController()->getEditRect()->registerAction(editActions[menu_edit_crop]);
	vp->getController()->getFileInfoLabel()->registerAction(viewActions[menu_view_show_info]);
	vp->getController()->getHistogram()->registerAction(viewActions[menu_view_show_histogram]);
	DkSettings::App::appMode = 0;

	initLanClient();
	//emit sendTitleSignal(windowTitle());

	// show it...
	show();
	DkSettings::App::appMode = DkSettings::mode_default;

	qDebug() << "viewport (normal) created...";
}

// FramelessNoMacs --------------------------------------------------------------------
DkNoMacsFrameless::DkNoMacsFrameless(QWidget *parent, Qt::WFlags flags)
	: DkNoMacs(parent, flags) {

		setObjectName("DkNoMacsFrameless");
		DkSettings::App::appMode = DkSettings::mode_frameless;
		
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
#ifndef Q_WS_X11
		if (!DkSettings::Sync::updateDialogShown && QDate::currentDate() > DkSettings::Sync::lastUpdateCheck)
			updater->checkForUpdated();
#endif

		// title signals
		connect(vp, SIGNAL(windowTitleSignal(QFileInfo, QSize)), this, SLOT(setWindowTitle(QFileInfo, QSize)));
		connect(vp->getImageLoader(), SIGNAL(updateFileSignal(QFileInfo, QSize)), this, SLOT(setWindowTitle(QFileInfo, QSize)));
		connect(vp->getImageLoader(), SIGNAL(newErrorDialog(QString, QString)), this, SLOT(errorDialog(QString, QString)));
		connect(this, SIGNAL(saveTempFileSignal(QImage)), vp->getImageLoader(), SLOT(saveTempFile(QImage)));
		connect(vp, SIGNAL(statusInfoSignal(QString)), this, SLOT(showStatusMessage(QString)));
		connect(vp, SIGNAL(enableNoImageSignal(bool)), this, SLOT(enableNoImageActions(bool)));
		connect(viewport()->getController()->getMetaDataWidget(), SIGNAL(enableGpsSignal(bool)), viewActions[menu_view_gps_map], SLOT(setEnabled(bool)));
		connect(vp->getImageLoader(), SIGNAL(folderFiltersChanged(QStringList)), this, SLOT(updateFilterState(QStringList)));

		vp->getController()->getFilePreview()->registerAction(viewActions[menu_view_show_preview]);
		vp->getController()->getMetaDataWidget()->registerAction(viewActions[menu_view_show_exif]);
		vp->getController()->getPlayer()->registerAction(viewActions[menu_view_show_player]);
		vp->getController()->getFileInfoLabel()->registerAction(viewActions[menu_view_show_info]);
		vp->getController()->getHistogram()->registerAction(viewActions[menu_view_show_histogram]);

		// in frameless, you cannot control if menu is visible...
		viewActions[menu_view_show_menu]->setEnabled(false);
		viewActions[menu_view_show_statusbar]->setEnabled(false);
		viewActions[menu_view_show_statusbar]->setChecked(false);
		viewActions[menu_view_show_toolbar]->setChecked(false);

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
	if (saveSettings) {
		DkSettings ourSettings;
		ourSettings.save();
	}

	saveSettings = false;

	DkNoMacs::closeEvent(event);

}

// Transfer function:

DkNoMacsContrast::DkNoMacsContrast(QWidget *parent, Qt::WFlags flags)
	: DkNoMacsSync(parent, flags) {


		setObjectName("DkNoMacsContrast");

		// init members
		DkViewPortContrast* vp = new DkViewPortContrast(this);
		vp->setAlignment(Qt::AlignHCenter);
		setCentralWidget(vp);

		localClient = new DkLocalManagerThread(this);
		localClient->start();

		lanClient = 0;

		init();

		createTransferToolbar();

		setAcceptDrops(true);
		setMouseTracking (true);	//receive mouse event everytime

		updater = new DkUpdater();
		connect(updater, SIGNAL(displayUpdateDialog(QString, QString)), this, SLOT(showUpdateDialog(QString, QString)));
#ifndef Q_WS_X11
		if (!DkSettings::Sync::updateDialogShown && QDate::currentDate() > DkSettings::Sync::lastUpdateCheck)
			updater->checkForUpdated();	// TODO: is threaded??
#endif

		// title signals
		connect(vp, SIGNAL(windowTitleSignal(QFileInfo, QSize)), this, SLOT(setWindowTitle(QFileInfo, QSize)));
		connect(vp->getImageLoader(), SIGNAL(updateFileSignal(QFileInfo, QSize)), this, SLOT(setWindowTitle(QFileInfo, QSize)));
		connect(vp->getImageLoader(), SIGNAL(newErrorDialog(QString, QString)), this, SLOT(errorDialog(QString, QString)));
		connect(this, SIGNAL(saveTempFileSignal(QImage)), vp->getImageLoader(), SLOT(saveTempFile(QImage)));
		connect(vp, SIGNAL(statusInfoSignal(QString)), this, SLOT(showStatusMessage(QString)));
		connect(vp, SIGNAL(enableNoImageSignal(bool)), this, SLOT(enableNoImageActions(bool)));
		//connect(vp, SIGNAL(newClientConnectedSignal()), this, SLOT(newClientConnected()));
		connect(viewport()->getController()->getMetaDataWidget(), SIGNAL(enableGpsSignal(bool)), viewActions[menu_view_gps_map], SLOT(setEnabled(bool)));
		connect(vp->getImageLoader(), SIGNAL(folderFiltersChanged(QStringList)), this, SLOT(updateFilterState(QStringList)));

		vp->getController()->getFilePreview()->registerAction(viewActions[menu_view_show_preview]);
		vp->getController()->getMetaDataWidget()->registerAction(viewActions[menu_view_show_exif]);
		vp->getController()->getPlayer()->registerAction(viewActions[menu_view_show_player]);
		vp->getController()->getFileInfoLabel()->registerAction(viewActions[menu_view_show_info]);
		vp->getController()->getEditRect()->registerAction(editActions[menu_edit_crop]);
		vp->getController()->getHistogram()->registerAction(viewActions[menu_view_show_histogram]);

		initLanClient();
		emit sendTitleSignal(windowTitle());

		DkSettings::App::appMode = DkSettings::mode_contrast;
		setObjectName("DkNoMacsContrast");

		// show it...
		show();

		// TODO: this should be checked but no event should be called
		viewActions[menu_view_show_transfertoolbar]->blockSignals(true);
		viewActions[menu_view_show_transfertoolbar]->setChecked(true);
		viewActions[menu_view_show_transfertoolbar]->blockSignals(false);

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

	transferToolBar->setMinimumHeight(5000);
	transferToolBar->layout()->setSizeConstraint(QLayout::SetMinimumSize);
	
	connect(transferToolBar, SIGNAL(colorTableChanged(QGradientStops)),  viewport(), SLOT(changeColorTable(QGradientStops)));
	connect(transferToolBar, SIGNAL(channelChanged(int)),  viewport(), SLOT(changeChannel(int)));
	connect(transferToolBar, SIGNAL(pickColorRequest()),  viewport(), SLOT(pickColor()));
	connect(transferToolBar, SIGNAL(tFEnabled(bool)), viewport(), SLOT(enableTF(bool)));
	connect((DkViewPortContrast*)centralWidget(), SIGNAL(tFSliderAdded(qreal)), transferToolBar, SLOT(insertSlider(qreal)));
	connect((DkViewPortContrast*)centralWidget(), SIGNAL(imageModeSet(int)), transferToolBar, SLOT(setImageMode(int)));

// play with themes only on windows - users used to look at it there ;)
// all other platforms have "native look and feel"
#ifdef Q_WS_WIN
	transferToolBar->setIconSize(QSize(16, 16));
	transferToolBar->setStyleSheet(
					//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
					QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); spacing: 4px}")
					+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
					//+ QString("QToolButton{border: none; margin: 3px;}")
					//+ QString("QToolButton:hover{border: 1px solid gray; color: rgba(0,0,0,127);} QToolButton:pressed{left: 1px; top: 1px; border: 1px;}")
					);
#endif

}
}
