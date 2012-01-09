/*******************************************************************************************************
 DkNoMacs.h
 Created on:	21.04.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011 Florian Kleber <florian@nomacs.org>

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
#include <QtGui/QMainWindow>
#include <QtGui/QBoxLayout>
#include <QtGui/QShortcut>
#include <QtGui/QResizeEvent>
#include <QtGui/QAction>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>
#include <QtGui/QPanGesture>
#include <QtGui/QSplashScreen>
#include <QDesktopServices>
#include <QClipboard>
#include <QEvent>
#include <QSettings>
#include <QFileInfo>
#include <QTimer>
#include <QProcess>
#include <QStringBuilder>

// OpenCV
#ifdef WITH_OPENCV
	#include <opencv/cv.h>
#endif

// my stuff
#include "DkNetwork.h"
#include "DkViewPort.h"
#include "DkImage.h"
#include "DkWidgets.h"
#include "DkSettings.h"
#include "DkMenu.h"

#ifdef DK_DLL
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

// keyboard shortcuts
//we can change the keyboard shortcuts from here !
enum {	

	// general
	shortcut_esc			= Qt::Key_Escape,
	
	// file
	shortcut_open_preview	= Qt::Key_T,
	shortcut_open_dir		= Qt::CTRL + Qt::SHIFT + Qt::Key_O,
	shortcut_open_with		= Qt::CTRL + Qt::Key_M,
	shortcut_first_file		= Qt::Key_Home, 
	shortcut_last_file		= Qt::Key_End,
	shortcut_skip_prev		= Qt::Key_PageUp,
	shortcut_skip_next		= Qt::Key_PageDown,
	shortcut_prev_file		= Qt::Key_Left,
	shortcut_next_file		= Qt::Key_Right,

	shortcut_first_file_sync= Qt::ALT + Qt::Key_Home, 
	shortcut_last_file_sync	= Qt::ALT + Qt::Key_End,
	shortcut_prev_file_sync	= Qt::ALT + Qt::Key_Left,
	shortcut_next_file_sync	= Qt::ALT + Qt::Key_Right,

	// view
	shortcut_show_toolbar	= Qt::CTRL + Qt::Key_B,
	shortcut_show_statusbar	= Qt::CTRL + Qt::Key_I,
	shortcut_full_screen_ad	= Qt::CTRL + Qt::Key_L,
	shortcut_full_screen_ff	= Qt::Key_F11,
	shortcut_frameless		= Qt::Key_F10,
	shortcut_reset_view 	= Qt::CTRL + Qt::Key_0,
	shortcut_zoom_full		= Qt::CTRL + Qt::Key_1,
	shortcut_zoom_in 		= Qt::Key_Plus,
	shortcut_zoom_out		= Qt::Key_Minus,
	shortcut_panning_left 	= Qt::CTRL + Qt::Key_Left,
	shortcut_panning_right 	= Qt::CTRL + Qt::Key_Right,
	shortcut_panning_up 	= Qt::CTRL + Qt::Key_Up,
	shortcut_panning_down 	= Qt::CTRL + Qt::Key_Down,
	shortcut_show_overview	= Qt::Key_O,
	shortcut_show_player	= Qt::Key_P,
	shortcut_show_exif		= Qt::Key_M,
	shortcut_show_info		= Qt::Key_I,
	shortcut_opacity_down	= Qt::CTRL + Qt::Key_H,
	shortcut_opacity_up		= Qt::CTRL + Qt::SHIFT + Qt::Key_H,
	shortcut_an_opacity		= Qt::ALT  + Qt::Key_H,
	shortcut_new_instance	= Qt::CTRL + Qt::Key_N,
	//shortcut_play			= Qt::Key_Space,

	// edit
	shortcut_rotate_cw		= Qt::Key_R,
	shortcut_rotate_ccw		= Qt::SHIFT + Qt::Key_R,
	shortcut_transform		= Qt::CTRL + Qt::Key_T,
	shortcut_paste			= Qt::Key_Insert,
	shortcut_delete_silent	= Qt::SHIFT + Qt::Key_Delete,

	// tcp
	shortcut_settings		= Qt::CTRL + Qt::SHIFT + Qt::Key_P,
	shortcut_sync			= Qt::CTRL + Qt::Key_D,
	shortcut_tab			= Qt::CTRL + Qt::Key_Tab,
	shortcut_arrange		= Qt::CTRL + Qt::SHIFT + Qt::Key_Tab,
	shortcut_send_img		= Qt::ALT + Qt::Key_I,
	shortcut_connect_all	= Qt::CTRL + Qt::Key_A,

	// help
	shortcut_show_help		= Qt::Key_F1,

	// eggs
	shortcut_test_img		= Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_L,
	shortcut_test_rec		= Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_R,
	shortcut_shiver			= Qt::CTRL + Qt::Key_W,
};

enum fileActions {
	menu_file_open,
	menu_file_open_dir,
	menu_file_open_with,
	menu_file_save,
	menu_file_print,
	menu_file_reload,
	menu_file_next,
	menu_file_prev,
	menu_file_new_instance,
	menu_file_exit,

	menu_file_end,	// nothing beyond this point
};

enum editActions {
	menu_edit_rotate_cw,
	menu_edit_rotate_ccw,
	menu_edit_rotate_180,
	menu_edit_copy,
	menu_edit_paste,
	menu_edit_preferences,
	menu_edit_transfrom,
	menu_edit_delete,

	menu_edit_end,	// nothing beyond this point
};

enum viewActions {
	menu_view_show_menu,
	menu_view_show_toolbar,
	menu_view_show_statusbar,
	menu_view_fullscreen,
	menu_view_reset,
	menu_view_100,
	menu_view_zoom_in,
	menu_view_zoom_out,
	menu_view_show_overview,
	menu_view_show_player,
	menu_view_show_preview,
	menu_view_show_exif,
	menu_view_show_info,
	menu_view_frameless,
	menu_view_opacity_up,
	menu_view_opacity_down,
	menu_view_opacity_an,
	menu_view_gps_map,

	menu_view_end,	// nothing beyond this point
};

enum syncActions {
	menu_sync,
	menu_sync_pos,
	menu_sync_arrange,
	menu_sync_connect_all,

	menu_sync_end,	// nothing beyond this point
};

enum helpActions {
	menu_help_update,
	menu_help_bug,
	menu_help_feature,
	menu_help_about,
	
	menu_help_end,	// nothing beyond this point
};

enum shortcuts {
	sc_pan_up,
	sc_pan_down,
	sc_pan_left,
	sc_pan_right,
	sc_first_file,
	sc_last_file,
	sc_skip_prev,
	sc_skip_next,
	sc_test_img,
	sc_test_rec,
	sc_next_sync,
	sc_prev_sync,
	sc_first_sync,
	sc_last_sync,
	sc_zoom_in,
	sc_zoom_out,
	sc_send_img,
	sc_delete_silent,
	//sc_play,

	sc_end,	// nothing beyond this point
};

enum fileIcons {
	icon_file_prev,
	icon_file_next,
	icon_file_dir,
	icon_file_open,
	icon_file_save,

	icon_file_end,	// nothing beyond this point
};

enum editIcons {
	icon_edit_rotate_cw,
	icon_edit_rotate_ccw,

	icon_edit_end,	// nothing beyond this point
};

enum viewIcons {
	icon_view_fullscreen,
	icon_view_reset,
	icon_view_100,
	icon_view_gps,

	icon_view_end,	// nothing beyond this point
};


/*! QApplication wrapper.
Its main purpose is to provide Mac OS X "open file from finder"
functionality for nomacs. It *has* to be catched on the
QApplication level in the event().

Also QSettings presets are handled here
*/
class DllExport DkNoMacsApp : public QApplication {
	Q_OBJECT

public:
	DkNoMacsApp(int argc, char** argv);

signals:
	void loadFile(const QFileInfo &fi);

private:
#ifdef Q_WS_MAC
	/*! Handle QFileOpenEvent for mac here */
	bool event(QEvent *event);
#endif

};

class DkViewPort;
class DkViewPortGt;
class DkMenuBar;

class DllExport DkNoMacs : public QMainWindow {
	Q_OBJECT

public:
	virtual ~DkNoMacs();

	void release();
	
	static int infoDialog(QString msg, QWidget* parent = 0, QString title = "Question");
	static int dialog(QString msg, QWidget* parent = 0, QString title = "Error");

	virtual DkViewPort* viewport();
	
	QVector<QAction* > getFileActions();
	QVector<QAction* > getViewActions();
	QVector<QAction* > getSyncActions();

	static void updateAll();

	bool saveSettings;

signals:
	void sendTitleSignal(QString newTitle);
	void sendPositionSignal(QRect newRect, bool overlaid);
	void sendArrangeSignal(bool overlaid);
	void synchronizeWithSignal(quint16);
	void synchronizeWithServerPortSignal(quint16);
	void closeSignal();
	void saveTempFileSignal(QImage img);

public slots:
	void enterFullScreen();
	void exitFullScreen();
	void openSettings();
	void openDir();
	void openFile();
	void saveFile();
	void resizeImage();
	void deleteFile();
	void cleanSettings();
	void newInstance();
	void showStatusBar(bool show);
	void showMenuBar(bool show);
	void showToolbar(bool show);
	void showGpsCoordinates();
	void openFileWith();
	void aboutDialog();
	void bugReport();
	void featureRequest();
	void errorDialog(QString msg, QString title = "Error");
	void loadRecursion();
	void setWindowTitle(QFileInfo file, QSize size = QSize());
	void opacityUp();
	void opacityDown();
	void changeOpacity(float change);
	void animateChangeOpacity();
	void animateOpacityUp();
	void animateOpacityDown();
	void tcpSetWindowRect(QRect newRect, bool opacity, bool overlaid);
	void tcpSendWindowRect();
	void tcpSendArrange();
	void newClientConnected();
	void showStatusMessage(QString msg);
	void copyImage();
	void pasteImage();
	virtual void settingsChanged();
	void showUpdateDialog(QString msg, QString title);
	void enableNoImageActions(bool enable = true);
	void checkForUpdate();
	void setFrameless(bool frameless);


protected:
	
	// mouse events
	void mouseDoubleClickEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

	// mouse events
	void moveEvent(QMoveEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

	// window events
	void contextMenuEvent(QContextMenuEvent *event);
	bool eventFilter(QObject *obj, QEvent *event);
	void resizeEvent(QResizeEvent *event);
	void closeEvent(QCloseEvent *event);

	// TODO: put to android class
	bool gestureEvent(QGestureEvent *event);

	bool overlaid;

	// vars
	QWidget *parent;

	QVector<QShortcut*> shortcuts;	
	QVector<QAction *> fileActions;
	QVector<QAction *> editActions;
	QVector<QAction *> viewActions;
	QVector<QAction *> syncActions;
	QVector<QAction *> helpActions;
	//QVector<QAction *> tcpViewerActions;
	
	// icons
	QVector<QIcon> fileIcons;
	QVector<QIcon> editIcons;
	QVector<QIcon> viewIcons;

	// menu
	DkMenuBar* menu;
	QMenu* fileMenu;	// TODO: release ?!
	QMenu* editMenu;
	QMenu* viewMenu;
	QMenu* syncMenu;
	QMenu* helpMenu;
	QMenu* contextMenu;

	// sub menus
	QMenu* fileFilesMenu;
	QMenu* fileFoldersMenu;
	QMenu* viewToolsMenu;
	DkTcpMenu* tcpViewerMenu;
	DkTcpMenu* tcpLanMenu;
	
	QPoint mousePos;
	
	// toolbar
	QToolBar* toolbar;
	QStatusBar* statusbar;
	QLabel* statusbarMsg;

	// file dialog
	QFileDialog* saveDialog;
	DkJpgDialog* jpgDialog;
	DkTifDialog* tifDialog;
	DkOpenWithDialog* openWithDialog;

	DkResizeDialog* resizeDialog;

	// client managers
	//DkLocalClientManager* localClientManager;
	//DkLANClientManager* lanClientManager;
	DkUpdater* updater;

	QRect oldGeometry;

	QProcess process;

	// functions
	DkNoMacs(QWidget *parent = 0, Qt::WFlags flags = 0);

	virtual void init();
	
	void registerFileVersion();
	virtual void createIcons();
	virtual void createToolbar();
	virtual void createShortcuts();
	virtual void createActions();
	virtual void createMenu();
	virtual void createContextMenu();
	virtual void createStatusbar();

	virtual void readSettings();

};

class DllExport DkNoMacsIpl : public DkNoMacs {
	Q_OBJECT

public:
	DkNoMacsIpl(QWidget *parent = 0, Qt::WFlags flags = 0);
	~DkNoMacsIpl();

	DkViewPortGt* viewportGt();
	qint16 getServerPort();
	void syncWith(qint16 port);
	
signals:
	void clientInitializedSignal();

public slots:
	void tcpConnectAll();
	void settingsChanged();
	void clientInitialized();

protected:

	// mouse events
	void dropEvent(QDropEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

	// functions
	void initLanClient();

	// gui
	void createMenu();

	// network layer
	DkLocalManagerThread* localClient;
	DkLanManagerThread* lanClient;

};


class DllExport DkNoMacsFrameless : public DkNoMacs {
	Q_OBJECT

public:
	DkNoMacsFrameless(QWidget *parent = 0, Qt::WFlags flags = 0);
	
	virtual ~DkNoMacsFrameless();

	void release();
	
public slots:
	void exitFullScreen();

protected:
	bool eventFilter(QObject *obj, QEvent *event);

};