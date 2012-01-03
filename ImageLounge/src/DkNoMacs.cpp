/*******************************************************************************************************
 DkNoMacs.cpp
 Created on:	21.04.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011 Florian Kleber <florian@nomacs.org>

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


DkNoMacsApp::DkNoMacsApp(int argc, char** argv)
	: QApplication(argc, argv)
{
}

#ifdef Q_WS_MAC
bool DkNoMacsApp::event(QEvent *event) {
	if (event->type() == QEvent::FileOpen) {
	   	emit loadFile(QFileInfo(static_cast<QFileOpenEvent*>(event)->file()));
		return true;
	}
	return QApplication::event(event);
}
#endif



DkNoMacs::DkNoMacs(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags) {

	QMainWindow::setWindowTitle("nomacs - Image Lounge");

	qDebug() << "This is nomacs - Image Lounge " << QApplication::applicationVersion();
	registerFileVersion();

	saveSettings = true;

	// load settings
	DkSettings* settings = new DkSettings();
	settings->load();
	
	saveDialog = 0;
	jpgDialog = 0;
	tifDialog = 0;
	resizeDialog = 0;
	updater = 0;
	openWithDialog = 0;

	// start localhost client/server
	//localClientManager = new DkLocalClientManager(windowTitle());
	//localClientManger->start();

	oldGeometry = geometry();
	overlaid = false;

	isFrameless = false;
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
}

void DkNoMacs::init() {

	setStyleSheet( "QMainWindow { border-style: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }" );

	// assign icon
	QString iconPath = ":/nomacs/img/nomacs.png";
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
	addActions(viewActions.toList());
	addActions(syncActions.toList());
	addActions(helpActions.toList());

	// TODO - just for android register me as a gesture recognizer
	grabGesture(Qt::PanGesture);
	grabGesture(Qt::PinchGesture);
	grabGesture(Qt::SwipeGesture);

	// load the window at the same position as last time
	readSettings();
	installEventFilter(this);

	showMenuBar(DkSettings::AppSettings::showMenuBar);
	showToolbar(DkSettings::AppSettings::showToolBar);
	showStatusBar(DkSettings::AppSettings::showStatusBar);

}

#ifdef WIN32	// windows specific versioning
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

// play with themes only on windows - users used to look at it there ;)
// all other platforms have "native look and feel"
#ifdef Q_WS_WIN
	toolbar->setIconSize(QSize(16, 16));
	toolbar->setStyleSheet(
					//QString("QToolBar {border-bottom: 1px solid #b6bccc;") +
					QString("QToolBar {border: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }")
					+ QString("QToolBar::separator {background: #656565; width: 1px; height: 1px; margin: 3px;}")
					//+ QString("QToolButton{border: none; margin: 3px;}")
					//+ QString("QToolButton:hover{border: 1px solid gray; color: rgba(0,0,0,127);} QToolButton:pressed{left: 1px; top: 1px; border: 1px;}")
					);
#endif
	// file
	//DkButton* test = new DkButton(fileIcons[icon_file_prev], tr("Pre&vious File"), this);
	//test->addAction(fileActions[menu_file_prev]);
	//toolbar->addWidget(test);
	toolbar->addAction(fileActions[menu_file_prev]);
	toolbar->addAction(fileActions[menu_file_next]);
	toolbar->addSeparator();

	toolbar->addAction(fileActions[menu_file_open_dir]);
	toolbar->addAction(fileActions[menu_file_open]);
	toolbar->addAction(fileActions[menu_file_save]);
	toolbar->addSeparator();

	// edit
	toolbar->addAction(editActions[menu_edit_rotate_ccw]);
	toolbar->addAction(editActions[menu_edit_rotate_cw]);
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

	statusbar = new QStatusBar(this);
	QColor col = QColor(200, 200, 230, 100);
	statusbar->setStyleSheet(QString("QStatusBar {border-top: none; background: QLinearGradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #edeff9, stop: 1 #bebfc7); }"));
	statusbar->addWidget(statusbarMsg);
	statusbar->show();
	this->setStatusBar(statusbar);
}

#define ICON(theme, backup) QIcon::fromTheme((theme), QIcon((backup)))

void DkNoMacs::createIcons() {

	fileIcons.resize(icon_file_end);
	fileIcons[icon_file_dir] = ICON("document-open-folder", ":/nomacs/img/dir.png");
	fileIcons[icon_file_open] = ICON("document-open", ":/nomacs/img/open.png");
	fileIcons[icon_file_save] = ICON("document-save", ":/nomacs/img/save.png");
	fileIcons[icon_file_prev] = ICON("go-previous", ":/nomacs/img/previous.png");
	fileIcons[icon_file_next] = ICON("go-next", ":/nomacs/img/next.png");

	editIcons.resize(icon_edit_end);
	editIcons[icon_edit_rotate_cw] = ICON("object-rotate-right", ":/nomacs/img/rotate-cw.png");
	editIcons[icon_edit_rotate_ccw] = ICON("object-rotate-left", ":/nomacs/img/rotate-cc.png");

	viewIcons.resize(icon_view_end);
	viewIcons[icon_view_fullscreen] = ICON("view-fullscreen", ":/nomacs/img/fullscreen.png");
	viewIcons[icon_view_reset] = ICON("zoom-draw", ":/nomacs/img/zoomReset.png");
	viewIcons[icon_view_100] = ICON("zoom-original", ":/nomacs/img/zoom100.png");
	viewIcons[icon_view_gps] = ICON("", ":/nomacs/img/gps-globe.png");

}

void DkNoMacs::createMenu() {

	this->setMenuBar(menu);
	fileMenu = menu->addMenu(tr("&File"));
	fileMenu->addAction(fileActions[menu_file_open]);
	fileMenu->addAction(fileActions[menu_file_open_dir]);
	fileMenu->addAction(fileActions[menu_file_open_with]);
	fileMenu->addAction(fileActions[menu_file_save]);

	fileFilesMenu = new DkHistoryMenu(tr("Recent &Files"), fileMenu, &DkSettings::GlobalSettings::recentFiles);
	connect(fileFilesMenu, SIGNAL(loadFileSignal(QFileInfo)), viewport(), SLOT(loadFile(QFileInfo)));

	fileFoldersMenu = new DkHistoryMenu(tr("Recent Fo&lders"), fileMenu, &DkSettings::GlobalSettings::recentFolders);
	connect(fileFoldersMenu, SIGNAL(loadFileSignal(QFileInfo)), viewport(), SLOT(loadFile(QFileInfo)));

	fileMenu->addMenu(fileFilesMenu);
	fileMenu->addMenu(fileFoldersMenu);

	fileMenu->addSeparator();
	fileMenu->addAction(fileActions[menu_file_print]);
	fileMenu->addSeparator();
	fileMenu->addAction(fileActions[menu_file_reload]);
	fileMenu->addAction(fileActions[menu_file_prev]);
	fileMenu->addAction(fileActions[menu_file_next]);
	fileMenu->addSeparator();
	fileMenu->addAction(fileActions[menu_file_new_instance]);
	fileMenu->addAction(fileActions[menu_file_exit]);

	editMenu = menu->addMenu(tr("&Edit"));
	editMenu->addAction(editActions[menu_edit_copy]);
	editMenu->addAction(editActions[menu_edit_paste]);
	editMenu->addSeparator();
	editMenu->addAction(editActions[menu_edit_rotate_ccw]);
	editMenu->addAction(editActions[menu_edit_rotate_cw]);
	editMenu->addAction(editActions[menu_edit_rotate_180]);
	editMenu->addSeparator();
	editMenu->addAction(editActions[menu_edit_transfrom]);
	editMenu->addAction(editActions[menu_edit_delete]);
	editMenu->addSeparator();
	editMenu->addAction(editActions[menu_edit_preferences]);

	viewMenu = menu->addMenu(tr("&View"));
	viewToolsMenu = viewMenu->addMenu(tr("Tool&bars"));
	viewToolsMenu->addAction(viewActions[menu_view_show_menu]);
	viewToolsMenu->addAction(viewActions[menu_view_show_toolbar]);
	viewToolsMenu->addAction(viewActions[menu_view_show_statusbar]);
	viewMenu->addAction(viewActions[menu_view_show_preview]);
	viewMenu->addAction(viewActions[menu_view_show_exif]);
	viewMenu->addAction(viewActions[menu_view_show_overview]);
	viewMenu->addAction(viewActions[menu_view_show_player]);
	viewMenu->addAction(viewActions[menu_view_show_info]);
	viewMenu->addSeparator();

	viewMenu->addAction(viewActions[menu_view_fullscreen]);
	viewMenu->addAction(viewActions[menu_view_reset]);
	viewMenu->addAction(viewActions[menu_view_100]);
	viewMenu->addAction(viewActions[menu_view_zoom_in]);
	viewMenu->addAction(viewActions[menu_view_zoom_out]);
	viewMenu->addSeparator();

	viewMenu->addAction(viewActions[menu_view_opacity_up]);
	viewMenu->addAction(viewActions[menu_view_opacity_down]);
	viewMenu->addAction(viewActions[menu_view_opacity_an]);
	viewMenu->addSeparator();
	
	viewMenu->addAction(viewActions[menu_view_gps_map]);


	syncMenu = menu->addMenu(tr("&Sync"));

	helpMenu = menu->addMenu(tr("&?"));
	helpMenu->addAction(helpActions[menu_help_update]);
	helpMenu->addSeparator();
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
	contextMenu->addSeparator();

	QMenu* viewContextMenu = contextMenu->addMenu(tr("&View"));
	viewContextMenu->addAction(viewActions[menu_view_fullscreen]);
	viewContextMenu->addAction(viewActions[menu_view_reset]);
	viewContextMenu->addAction(viewActions[menu_view_100]);

	QMenu* editContextMenu = contextMenu->addMenu(tr("&Edit"));
	editContextMenu->addAction(editActions[menu_edit_rotate_cw]);
	editContextMenu->addAction(editActions[menu_edit_rotate_ccw]);
	editContextMenu->addAction(editActions[menu_edit_rotate_180]);
	editContextMenu->addSeparator();
	editContextMenu->addAction(editActions[menu_edit_transfrom]);
	editContextMenu->addAction(editActions[menu_edit_delete]);
	contextMenu->addSeparator();
	contextMenu->addAction(editActions[menu_edit_preferences]);

	contextMenu->addMenu(syncMenu);
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

	fileActions[menu_file_save] = new QAction(fileIcons[icon_file_save], tr("&Save"), this);
	fileActions[menu_file_save]->setShortcuts(QKeySequence::Save);
	fileActions[menu_file_save]->setStatusTip(tr("Save an image"));
	connect(fileActions[menu_file_save], SIGNAL(triggered()), this, SLOT(saveFile()));

	fileActions[menu_file_print] = new QAction(tr("&Print"), this);
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
	connect(fileActions[menu_file_next], SIGNAL(triggered()), vp, SLOT(loadNextFile()));

	fileActions[menu_file_prev] = new QAction(fileIcons[icon_file_prev], tr("Pre&vious File"), this);
	fileActions[menu_file_prev]->setShortcut(QKeySequence(shortcut_prev_file));
	fileActions[menu_file_prev]->setStatusTip(tr("Load previous file"));
	connect(fileActions[menu_file_prev], SIGNAL(triggered()), vp, SLOT(loadPrevFile()));

	fileActions[menu_file_new_instance] = new QAction(tr("St&art New Instance"), this);
	fileActions[menu_file_new_instance]->setShortcut(QKeySequence(shortcut_new_instance));
	fileActions[menu_file_new_instance]->setStatusTip(tr("Open file in new instance"));
	connect(fileActions[menu_file_new_instance], SIGNAL(triggered()), this, SLOT(newInstance()));

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

	QList<QKeySequence> pastScs;
	pastScs.append(QKeySequence::Paste);
	pastScs.append(shortcut_paste);
	editActions[menu_edit_paste] = new QAction(tr("&Paste"), this);
	editActions[menu_edit_paste]->setShortcuts(pastScs);
	editActions[menu_edit_paste]->setStatusTip(tr("paste image"));
	connect(editActions[menu_edit_paste], SIGNAL(triggered()), this, SLOT(pasteImage()));

	editActions[menu_edit_transfrom] = new QAction(tr("R&esize Image"), this);
	editActions[menu_edit_transfrom]->setShortcut(shortcut_transform);
	editActions[menu_edit_transfrom]->setStatusTip(tr("resize the current image"));
	connect(editActions[menu_edit_transfrom], SIGNAL(triggered()), this, SLOT(resizeImage()));

	editActions[menu_edit_delete] = new QAction(tr("&Delete"), this);
	editActions[menu_edit_delete]->setShortcut(QKeySequence::Delete);
	editActions[menu_edit_delete]->setStatusTip(tr("delete current file"));
	connect(editActions[menu_edit_delete], SIGNAL(triggered()), this, SLOT(deleteFile()));

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

	viewActions[menu_view_show_overview] = new QAction(tr("Show O&verview"), this);
	viewActions[menu_view_show_overview]->setShortcut(QKeySequence(shortcut_show_overview));
	viewActions[menu_view_show_overview]->setStatusTip(tr("shows the overview or not"));
	viewActions[menu_view_show_overview]->setCheckable(true);
	viewActions[menu_view_show_overview]->setChecked(DkSettings::GlobalSettings::showOverview);
	connect(viewActions[menu_view_show_overview], SIGNAL(triggered()), vp, SLOT(toggleShowOverview()));

	viewActions[menu_view_show_player] = new QAction(tr("Show Pla&yer"), this);
	viewActions[menu_view_show_player]->setShortcut(QKeySequence(shortcut_show_player));
	viewActions[menu_view_show_player]->setStatusTip(tr("shows the player or not"));
	viewActions[menu_view_show_player]->setCheckable(true);
	connect(viewActions[menu_view_show_player], SIGNAL(triggered()), vp, SLOT(toggleShowPlayer()));

	viewActions[menu_view_show_preview] = new QAction(tr("Sho&w Thumbnails"), this);
	viewActions[menu_view_show_preview]->setShortcut(QKeySequence(shortcut_open_preview));
	viewActions[menu_view_show_preview]->setStatusTip(tr("Show thumbnails"));
	viewActions[menu_view_show_preview]->setCheckable(true);
	connect(viewActions[menu_view_show_preview], SIGNAL(triggered()), vp, SLOT(showPreview()));

	viewActions[menu_view_show_exif] = new QAction(tr("Show &Metadata"), this);
	viewActions[menu_view_show_exif]->setShortcut(QKeySequence(shortcut_show_exif));
	viewActions[menu_view_show_exif]->setStatusTip(tr("shows the metadata panel"));
	viewActions[menu_view_show_exif]->setCheckable(true);
	connect(viewActions[menu_view_show_exif], SIGNAL(triggered()), vp, SLOT(showExif()));

	viewActions[menu_view_show_info] = new QAction(tr("Show File &Info"), this);
	viewActions[menu_view_show_info]->setShortcut(QKeySequence(shortcut_show_info));
	viewActions[menu_view_show_info]->setStatusTip(tr("shows the info panel"));
	viewActions[menu_view_show_info]->setCheckable(true);
	connect(viewActions[menu_view_show_info], SIGNAL(triggered()), vp, SLOT(showInfo()));

	viewActions[menu_view_frameless] = new QAction(tr("&Frameless"), this);
	viewActions[menu_view_frameless]->setShortcut(QKeySequence(shortcut_frameless));
	viewActions[menu_view_frameless]->setStatusTip(tr("shows a frameless window"));
	viewActions[menu_view_frameless]->setCheckable(true);
	connect(viewActions[menu_view_frameless], SIGNAL(triggered()), this, SLOT(setFrameless()));

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

	viewActions[menu_view_gps_map] = new QAction(viewIcons[icon_view_gps], tr("Show G&PS Coordinates"), this);
	viewActions[menu_view_gps_map]->setStatusTip(tr("shows the GPS coordinates"));
	viewActions[menu_view_gps_map]->setEnabled(false);
	connect(viewActions[menu_view_gps_map], SIGNAL(triggered()), this, SLOT(showGpsCoordinates()));

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
	QObject::connect(shortcuts[sc_next_sync], SIGNAL( activated ()), vp, SLOT( loadNextFile() ));

	shortcuts[sc_prev_sync] = new QShortcut(shortcut_prev_file_sync, this);
	QObject::connect(shortcuts[sc_prev_sync], SIGNAL( activated ()), vp, SLOT( loadPrevFile() ));

	shortcuts[sc_zoom_in] = new QShortcut(shortcut_zoom_in, this);
	connect(shortcuts[sc_zoom_in], SIGNAL(activated()), vp, SLOT(zoomIn()));

	shortcuts[sc_zoom_out] = new QShortcut(shortcut_zoom_out, this);
	connect(shortcuts[sc_zoom_out], SIGNAL(activated()), vp, SLOT(zoomOut()));

	shortcuts[sc_send_img] = new QShortcut(shortcut_send_img, this);
	connect(shortcuts[sc_send_img], SIGNAL(activated()), vp, SLOT(tcpSendImage()));

	shortcuts[sc_delete_silent] = new QShortcut(shortcut_delete_silent, this);
	connect(shortcuts[sc_delete_silent], SIGNAL(activated()), vp->getImageLoader(), SLOT(deleteFile()));
}

void DkNoMacs::enableNoImageActions(bool enable) {

	fileActions[menu_file_save]->setEnabled(enable);
	fileActions[menu_file_open_with]->setEnabled(enable);
	fileActions[menu_file_print]->setEnabled(enable);
	fileActions[menu_file_reload]->setEnabled(enable);
	fileActions[menu_file_prev]->setEnabled(enable);
	fileActions[menu_file_next]->setEnabled(enable);

	editActions[menu_edit_rotate_cw]->setEnabled(enable);
	editActions[menu_edit_rotate_ccw]->setEnabled(enable);
	editActions[menu_edit_rotate_180]->setEnabled(enable);
	editActions[menu_edit_delete]->setEnabled(enable);
	editActions[menu_edit_transfrom]->setEnabled(enable);
	editActions[menu_edit_copy]->setEnabled(enable);

	viewActions[menu_view_show_info]->setEnabled(enable);
	viewActions[menu_view_show_preview]->setEnabled(enable);
	viewActions[menu_view_show_exif]->setEnabled(enable);
	viewActions[menu_view_show_overview]->setEnabled(enable);
	viewActions[menu_view_show_player]->setEnabled(enable);
	viewActions[menu_view_fullscreen]->setEnabled(enable);
	viewActions[menu_view_reset]->setEnabled(enable);
	viewActions[menu_view_100]->setEnabled(enable);
	viewActions[menu_view_zoom_in]->setEnabled(enable);
	viewActions[menu_view_zoom_out]->setEnabled(enable);

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

	if (event->button() != Qt::LeftButton)
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
			&& viewport()->getWorldMatrix().m11() == 1.0f
			&& !viewport()->getImage().isNull()
			&& viewport()->getImageLoader()) {

			QUrl fileUrl = QUrl("file:///" + viewport()->getImageLoader()->getFile().absoluteFilePath());

			QList<QUrl> urls;
			urls.append(fileUrl);

			QMimeData* mimeData = new QMimeData;
			
			if (viewport()->getImageLoader()->getFile().exists())
				mimeData->setUrls(urls);
			
			if (!viewport()->getImage().isNull())
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
				vp->loadNextFile();
			else if (swipeG->horizontalDirection() == QSwipeGesture::Right)
				vp->loadPrevFile();

			// TODO: recognize some other gestures please
		}

	}
	else if (QGesture *pan = event->gesture(Qt::PanGesture)) {
		
		QPanGesture* panG = static_cast<QPanGesture *>(pan);

		qDebug() << "you're speedy: " << panG->acceleration();

		QPointF delta = panG->delta();

		if (panG->acceleration() > 10 && delta.x() && fabs(delta.y()/delta.x()) < 0.2) {
			
			if (delta.x() < 0)
				vp->loadNextFile();
			else
				vp->loadPrevFile();
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

	}
	else if (event->mimeData()->hasImage()) {

		QImage dropImg = qvariant_cast<QImage>(event->mimeData()->imageData());

		// delete current information
		if (viewport()->getImageLoader()) {
			viewport()->getImageLoader()->clearPath();
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
	
	if (viewport()->getImageLoader()->getFile().exists())
		mimeData->setUrls(urls);

	if (!viewport()->getImage().isNull())
		mimeData->setImageData(viewport()->getImage());

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);

	qDebug() << "copying: " << fileUrl;
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
				viewport()->getImageLoader()->clearPath();
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
			viewport()->setCenterInfo("Could not find a valid file url, sorry");
	}
	
	else if (viewport())
		viewport()->setCenterInfo("Clipboard has no image...");

}

void DkNoMacs::readSettings() {
	
	qDebug() << "reading settings...";
	QSettings settings;
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
}

void DkNoMacs::enterFullScreen() {

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
		
		if (!isFrameless) {
			if (DkSettings::AppSettings::showMenuBar) menu->show();
			if (DkSettings::AppSettings::showToolBar) toolbar->show();
			if (DkSettings::AppSettings::showStatusBar) statusbar->show();
		}
		showNormal();
	}

	if (viewport())
		viewport()->setFullScreen(false);
}

void DkNoMacs::setFrameless() {

	isFrameless = viewActions[menu_view_frameless]->isChecked();
	setFrameless(isFrameless);

}

void DkNoMacs::setFrameless(bool frameless) {

	isFrameless = frameless;

	if (isFrameless) {
		setWindowFlags(Qt::FramelessWindowHint);
		setAttribute(Qt::WA_TranslucentBackground, true);
		menu->hide();
		toolbar->hide();
		statusbar->hide();
		show();
	}
	else {

		if (DkSettings::AppSettings::showMenuBar)
			menu->show();
		if (DkSettings::AppSettings::showToolBar)
			toolbar->show();
		if (DkSettings::AppSettings::showStatusBar)
			statusbar->show();

		//setAttribute(Qt::WA_TranslucentBackground, false);
		//setAttribute(Qt::WA_OpaquePaintEvent, true);
		setWindowFlags(Qt::Widget);
		show();
		//showNormal();
	}

	viewport()->setFrameless(isFrameless);
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

		//this->showNormal();
		//this->raise();
		//this->activateWindow();

		showMinimized();
		setWindowState(Qt::WindowActive);
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

	// load system default open dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"),
		loader->getDir().absolutePath(), 
		DkImageLoader::openFilter);

	if (fileName.isEmpty())
		return;

	QFileInfo oFile = QFileInfo(fileName);
	viewport()->loadFile(QFileInfo(fileName));
}

void DkNoMacs::saveFile() {
	
	qDebug() << "saving...";

	QStringList fileNames;
	DkImageLoader* loader = viewport()->getImageLoader();

	//QString saveName;
	//QString saveDir;

	//if (loader->hasFile()) {
	//	saveName = loader->getFile().fileName();
	//	saveDir = loader->getSaveDir().absolutePath();
	//}

	//QString fileName = QFileDialog::getSaveFileName(this, tr("Save File %1").arg(saveName),
	//	saveName,
	//	//saveDir,
	//	DkImageLoader::saveFilter);

	if (!saveDialog)
		saveDialog = new QFileDialog(this);

	saveDialog->setWindowTitle("Save Image");
	saveDialog->setAcceptMode(QFileDialog::AcceptSave);
	saveDialog->setFilters(DkImageLoader::saveFilters);
	
	if (loader->hasFile()) {
		saveDialog->selectNameFilter(loader->getCurrentFilter());	// select the current file filter
		saveDialog->setDirectory(loader->getSaveDir());
		saveDialog->selectFile(loader->getFile().fileName());
	}

	//saveDialog->show();

	// show the dialog
	if(saveDialog->exec())
		fileNames = saveDialog->selectedFiles();

	if (fileNames.empty())
		return;

	// TODO: if more than one file is opened -> open new threads
	QFileInfo sFile = QFileInfo(fileNames[0]);
	QImage saveImg = viewport()->getImage();
	int compression = -1;	// default value

	if (saveDialog->selectedNameFilter().contains("jpg")) {
		
		if (!jpgDialog)
			jpgDialog = new DkJpgDialog(this);

		jpgDialog->imageHasAlpha(saveImg.hasAlphaChannel());
		jpgDialog->show();
		jpgDialog->setImage(&saveImg);
		bool ret = jpgDialog->exec();
		
		if (!jpgDialog->wasOkPressed())
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

	if (saveDialog->selectedNameFilter().contains("tif")) {

		if (!tifDialog)
			tifDialog = new DkTifDialog(this);

		bool ret = tifDialog->exec();

		if (!tifDialog->wasOkPressed())
			return;

		compression = tifDialog->getCompression();
	}


	if (loader)
		loader->saveFile(sFile, saveDialog->selectedNameFilter(), saveImg, compression);

}

void DkNoMacs::resizeImage() {

	if (!viewport() || viewport()->getImage().isNull())
		return;

	if (!resizeDialog)
		resizeDialog = new DkResizeDialog(this);


	DkMetaDataInfo* metaData = viewport()->getMetaDataWidget();
	
	if (metaData) {
		float xDpi, yDpi;
		metaData->getResolution(xDpi, yDpi);
		resizeDialog->setExifDpi(xDpi);
	}

	resizeDialog->setImage(&viewport()->getImage());

	bool done = resizeDialog->exec();

	if (resizeDialog->wasOkPressed()) {

		if (resizeDialog->resample()) {

			// do not load the old image -> as the transformed image is not the same anymore
			viewport()->getImageLoader()->enableWatcher(false);

			// this reloads the image -> that's not what we want!
			if (metaData)
				metaData->setResolution((int)resizeDialog->getExifDpi(), (int)resizeDialog->getExifDpi());

			QImage rImg = resizeDialog->getResizedImage();

			if (!rImg.isNull())
				viewport()->setImage(rImg);
			else {
				errorDialog("Sorry, I cannot transform the image.");
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

	if (infoDialog("Do you want to permanently delete " % file.fileName(), this) == QMessageBox::Yes)
		viewport()->getImageLoader()->deleteFile();

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

void DkNoMacs::newInstance() {

	if (!viewport()) 
		return;

	QString exe = QApplication::applicationFilePath();
	QStringList args;
	args.append(viewport()->getImageLoader()->getFile().absoluteFilePath());
	QProcess::startDetached(exe, args);
}

void DkNoMacs::loadRecursion() {

	QImage img = QPixmap::grabWindow(this->winId()).toImage();
	viewport()->setImage(img);
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


// >DIR diem: eating shortcut overrides (this allows us to use navigation keys like arrows)
bool DkNoMacs::eventFilter(QObject *obj, QEvent *event) {

	if (event->type() == QEvent::ShortcutOverride) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

		if (keyEvent->key() == Qt::Key_Alt && !isFrameless && !isFullScreen()) {
			menu->showMenu();
			return true;
		}

		// consume esc key if fullscreen is on
		if (keyEvent->key() == Qt::Key_Escape && isFullScreen()) {
			exitFullScreen();
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Escape && isFrameless) {
			close();
			return true;
		}
	}
	if (event->type() == QEvent::Gesture) {
		return gestureEvent(static_cast<QGestureEvent*>(event));
	}

	return false;
}

void DkNoMacs::showMenuBar(bool show) {

	DkSettings::AppSettings::showMenuBar = show;
	int tts = (DkSettings::AppSettings::showMenuBar) ? -1 : 5000;
	viewActions[menu_view_show_menu]->setChecked(DkSettings::AppSettings::showMenuBar);
	menu->setTimeToShow(tts);
	menu->showMenu();
}

void DkNoMacs::showToolbar(bool show) {

	DkSettings::AppSettings::showToolBar = show;
	viewActions[menu_view_show_toolbar]->setChecked(DkSettings::AppSettings::showToolBar);
	
	if (DkSettings::AppSettings::showToolBar)
		toolbar->show();
	else
		toolbar->hide();
}

void DkNoMacs::showStatusBar(bool show) {

	DkSettings::AppSettings::showStatusBar = show;
	viewActions[menu_view_show_statusbar]->setChecked(DkSettings::AppSettings::showStatusBar);

	if (DkSettings::AppSettings::showStatusBar)
		statusbar->show();
	else
		statusbar->hide();

	viewport()->setVisibleStatusbar(show);
}

void DkNoMacs::showStatusMessage(QString msg) {

	statusbarMsg->setText(msg);
}

void DkNoMacs::openFileWith() {

	//QString org = QString("Adobe");
	//QString ps = QString("Photoshop");
	//QSettings* adSettings = new QSettings(QSettings::UserScope, org, ps);

	//QStringList keys = adSettings->allKeys();

	//qDebug() << keys;

	//QString appPath;

	//for (int idx = 0; idx < keys.length(); idx++) {
	//	
	//	if (keys[idx].contains("ApplicationPath")) {
	//		appPath = adSettings->value(keys[idx]).toString() % "Photoshop";
	//		break;
	//	}
	//}

	//appPath.replace("/", "\\");	// photoshop needs backslashes

	if (DkSettings::GlobalSettings::showDefaultAppDialog) {
		if (!openWithDialog) openWithDialog = new DkOpenWithDialog(this);
		openWithDialog->exec();

		if (!openWithDialog->wasOkClicked())
			return;
	}

	QStringList args;
	args << QDir::toNativeSeparators(viewport()->getImageLoader()->getFile().absoluteFilePath());

	//bool started = process.startDetached("psOpenImages.exe", args);	// already deprecated
	bool started = process.startDetached(DkSettings::GlobalSettings::defaultAppPath, args);

	if (started)
		qDebug() << "starting: " << DkSettings::GlobalSettings::defaultAppPath;
	else {
		viewport()->setCenterInfo("Sorry, I could not start: " % DkSettings::GlobalSettings::defaultAppPath);
		DkSettings::GlobalSettings::showDefaultAppDialog = true;
	}

	qDebug() << "I'm trying to execute: " << args[0];

}

void DkNoMacs::showGpsCoordinates() {

	DkMetaDataInfo* exifData = viewport()->getMetaDataWidget();

	if (!exifData || exifData->getGPSCoordinates().isEmpty()) {
		viewport()->setInfo("Sorry, I could not find the GPS coordinates...");
		return;
	}

	qDebug() << "gps: " << exifData->getGPSCoordinates();

	QDesktopServices::openUrl(QUrl(exifData->getGPSCoordinates()));  
}

QVector <QAction* > DkNoMacs::getFileActions() {

	return fileActions;
}

QVector <QAction* > DkNoMacs::getViewActions() {

	return viewActions;
}

QVector <QAction* > DkNoMacs::getSyncActions() {

	return syncActions;
}

void DkNoMacs::setWindowTitle(QFileInfo file, QSize size) {

	// tell the viewport
	viewport()->setTitleLabel(file, -1);

	QString title = file.fileName();

	if (!file.exists())
		title = "nomacs - Image Lounge";

	QString attributes;

	if (!size.isEmpty())
		attributes.sprintf(" - %i x %i", size.width(), size.height());
	if (size.isEmpty() && viewport())
		attributes.sprintf(" - %i x %i", viewport()->getImage().width(), viewport()->getImage().height());

	QMainWindow::setWindowTitle(title.append(attributes));

	emit sendTitleSignal(windowTitle());

}

void DkNoMacs::openSettings() {

	DkSettingsDialog dsd = DkSettingsDialog(this);
	connect(&dsd, SIGNAL(setToDefaultSignal()), this, SLOT(cleanSettings()));
	connect(&dsd, SIGNAL(settingsChanged()), viewport(), SLOT(settingsChanged()));
	connect(&dsd, SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));
	dsd.exec();

	qDebug() << "hier könnte ihre werbung stehen...";
}

void DkNoMacs::settingsChanged() {
	
	showMenuBar(DkSettings::AppSettings::showMenuBar);
	showToolbar(DkSettings::AppSettings::showToolBar);
	showStatusBar(DkSettings::AppSettings::showStatusBar);
}

void DkNoMacs::checkForUpdate() {

	if (!updater) {
		updater = new DkUpdater();
		connect(updater, SIGNAL(displayUpdateDialog(QString, QString)), this, SLOT(showUpdateDialog(QString, QString)));
	}
	updater->silent = false;
	updater->checkForUpdated();

}

void DkNoMacs::showUpdateDialog(QString msg, QString title) {

	DkSettings::SynchronizeSettings::updateDialogShown = true;

	DkSettings settings;
	settings.save();
	
	QMessageBox infoDialog(this);
	infoDialog.setWindowTitle(title);
	infoDialog.setIcon(QMessageBox::Information);
	infoDialog.setText(msg);
	infoDialog.show();

	infoDialog.exec();

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

// DkNoMacsIpl --------------------------------------------------------------------
DkNoMacsIpl::DkNoMacsIpl(QWidget *parent, Qt::WFlags flags) : DkNoMacs(parent, flags) {

	// init members
	DkViewPort* vp = new DkViewPort(this);
	vp->setAlignment(Qt::AlignHCenter);
	setCentralWidget(vp);

	localClient = new DkLocalManagerThread(this);
	localClient->start();

	lanClient = 0;

	init();
	setFrameless(isFrameless);
	setAcceptDrops(true);
	setMouseTracking (true);	//receive mouse event everytime

	vp->setVisibleStatusbar(statusbar->isVisible());

	updater = new DkUpdater();
	connect(updater, SIGNAL(displayUpdateDialog(QString, QString)), this, SLOT(showUpdateDialog(QString, QString)));
	if (!DkSettings::SynchronizeSettings::updateDialogShown && QDate::currentDate() > DkSettings::SynchronizeSettings::lastUpdateCheck)
		updater->checkForUpdated();	// TODO: is threaded??
	
	// title signals
	connect(vp, SIGNAL(windowTitleSignal(QFileInfo, QSize)), this, SLOT(setWindowTitle(QFileInfo, QSize)));
	connect(vp->getImageLoader(), SIGNAL(updateFileSignal(QFileInfo, QSize)), this, SLOT(setWindowTitle(QFileInfo, QSize)));
	connect(vp->getImageLoader(), SIGNAL(newErrorDialog(QString, QString)), this, SLOT(errorDialog(QString, QString)));
	connect(this, SIGNAL(saveTempFileSignal(QImage)), vp->getImageLoader(), SLOT(saveTempFile(QImage)));
	connect(vp, SIGNAL(statusInfoSignal(QString)), this, SLOT(showStatusMessage(QString)));
	connect(vp, SIGNAL(enableNoImageSignal(bool)), this, SLOT(enableNoImageActions(bool)));
	connect(vp, SIGNAL(newClientConnectedSignal()), this, SLOT(newClientConnected()));
	connect(viewport()->getMetaDataWidget(), SIGNAL(enableGpsSignal(bool)), viewActions[menu_view_gps_map], SLOT(setEnabled(bool)));

	//connect(vp, SIGNAL(previewVisibleSignal(bool)), viewActions[menu_view_show_preview], SLOT(setChecked(bool)));
	//connect(vp, SIGNAL(exifVisibleSignal(bool)), viewActions[menu_view_show_exif], SLOT(setChecked(bool)));
	//connect(vp, SIGNAL(overviewVisibleSignal(bool)), viewActions[menu_view_show_overview], SLOT(setChecked(bool)));
	//connect(vp, SIGNAL(playerVisibleSignal(bool)), viewActions[menu_view_show_player], SLOT(setChecked(bool)));

	vp->getFilePreview()->registerAction(viewActions[menu_view_show_preview]);
	vp->getPlayer()->registerAction(viewActions[menu_view_show_player]);
	vp->getMetaDataWidget()->registerAction(viewActions[menu_view_show_exif]);
	vp->getFileInfoWidget()->registerAction(viewActions[menu_view_show_info]);

	initLanClient();
	qDebug() << "lan client initialized...";
	//emit sendTitleSignal(windowTitle());

	// show it...
	show();

	qDebug() << "viewport (normal) created...";
}

DkNoMacsIpl::~DkNoMacsIpl() {

	if (localClient) {

		// terminate local client
		localClient->quit();
		localClient->wait();

		delete localClient;
		localClient = 0;
	}

}

void DkNoMacsIpl::connectClient(DkClientManager* client) {

	DkViewPort* vp = viewport();

	//// TCP communication
	//connect(vp, SIGNAL(sendTransformSignal(QTransform, QTransform, QPointF)), client, SLOT(sendTransform(QTransform, QTransform, QPointF)));
	//connect(this, SIGNAL(sendPositionSignal(QRect)), client, SLOT(sendPosition(QRect)));
	//connect(this, SIGNAL(synchronizeWithSignal(quint16)), client, SLOT(synchronizeWith(quint16)));
	//connect(this, SIGNAL(synchronizeWithServerPortSignal(quint16)), client, SLOT(synchronizeWithServerPort(quint16)));

	//connect(this, SIGNAL(sendTitleSignal(QString)), client, SLOT(sendTitle(QString)));
	//connect(vp, SIGNAL(sendNewFileSignal(qint16, QString)), client, SLOT(sendNewFile(qint16, QString)));
	//connect(client, SIGNAL(receivedNewFile(qint16, QString)), vp, SLOT(tcpLoadFile(qint16, QString)));
	//connect(client, SIGNAL(updateConnectionSignal(QList<DkPeer>)), vp, SLOT(tcpShowConnections(QList<DkPeer>)));

	//connect(client, SIGNAL(receivedTransformation(QTransform, QTransform, QPointF)), vp, SLOT(tcpSetTransforms(QTransform, QTransform, QPointF)));
	//connect(client, SIGNAL(receivedPosition(QRect, bool)), this, SLOT(tcpSetWindowRect(QRect, bool)));
}

void DkNoMacsIpl::initLanClient() {

	if (lanClient) {

		lanClient->quit();
		lanClient->wait();

		delete lanClient;
	}

	if (!DkSettings::SynchronizeSettings::enableNetworkSync) {
		
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
	
	tcpLanMenu->setClientManager(lanClient);
	tcpLanMenu->addTcpAction(startServer);
	tcpLanMenu->setEnabled(true);
}

void DkNoMacsIpl::createMenu() {

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
void DkNoMacsIpl::mouseMoveEvent(QMouseEvent *event) {

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
	
void DkNoMacsIpl::dropEvent(QDropEvent *event) {

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

qint16 DkNoMacsIpl::getServerPort() {

	return (localClient) ? localClient->getServerPort() : 0;
}

void DkNoMacsIpl::syncWith(qint16 port) {
	emit synchronizeWithServerPortSignal(port);
}

// slots
void DkNoMacsIpl::tcpConnectAll() {

	QList<DkPeer> peers = localClient->getPeerList();

	for (int idx = 0; idx < peers.size(); idx++)
		emit synchronizeWithSignal(peers.at(idx).peerId);

}

void DkNoMacsIpl::settingsChanged() {
	initLanClient();

	DkNoMacs::settingsChanged();
}

void DkNoMacsIpl::clientInitialized() {
	//TODO: things that need to be done after the clientManager has finished initialization
	emit clientInitializedSignal();
}
