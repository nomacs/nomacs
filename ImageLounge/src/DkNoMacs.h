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
#include <QMainWindow>
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

// OpenCV
#ifdef WITH_OPENCV

#ifdef WIN32
	#pragma warning(disable: 4996)
#endif


#ifdef DISABLE_LANCZOS // opencv 2.1.0 is used, does not have opencv2 includes
#include "opencv/cv.h"
#else
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#endif

using namespace cv;
#endif


//#ifdef Q_WS_WIN
//	#include <dwmapi.h>	// needed to see if aero is on
//	#pragma comment (lib, "dwmapi.lib")
//#endif

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

namespace nmc {

class DkTcpMenu;
class DkCompressDialog;
class DkSettingsDialog;
class DkTifDialog;
class DkOpacityDialog;
class DkResizeDialog;
class DkUpdateDialog;
class DkForceThumbDialog;
class DkTrainDialog;
#ifdef WITH_QUAZIP
class DkArchiveExtractionDialog;
#endif
class DkExplorer;
class DkMetaDataDock;
class DkExportTiffDialog;
class DkImageManipulationDialog;
class DkUpdater;
class DkTranslationUpdater;
class DkLocalManagerThread;
class DkLanManagerThread;
class DkRCManagerThread;
class DkTransferToolBar;
class DkPluginManager;
class DkAppManager;
class DkImageContainerT;	// TODO: add include to suppress warning C4150
class DkThumbsSaver;
class DkPrintPreviewDialog;
class DkBatchDialog;
class DkViewPort;
class DkCentralWidget;

#ifdef WITH_UPNP
class DkUpnpControlPoint;
class DkUpnpDeviceHost;
class DkUpnpRendererDeviceHost;
#endif // WITH_UPNP



// keyboard shortcuts
//we can change the keyboard shortcuts from here !
enum {	

	// general
	shortcut_esc			= Qt::Key_Escape,
	
	// file
	shortcut_show_scroller	= Qt::Key_F,
	shortcut_open_preview	= Qt::Key_T,
	shortcut_open_thumbview	= Qt::SHIFT + Qt::Key_T,
	shortcut_open_dir		= Qt::CTRL + Qt::SHIFT + Qt::Key_O,
	shortcut_app_manager	= Qt::CTRL + Qt::Key_M,
	shortcut_save_as		= Qt::CTRL + Qt::SHIFT + Qt::Key_S,
	shortcut_first_file		= Qt::Key_Home, 
	shortcut_last_file		= Qt::Key_End,
	shortcut_skip_prev		= Qt::Key_PageUp,
	shortcut_skip_next		= Qt::Key_PageDown,
	shortcut_prev_file		= Qt::Key_Left,
	shortcut_next_file		= Qt::Key_Right,
	shortcut_rename			= Qt::Key_F2,
	shortcut_goto			= Qt::CTRL + Qt::Key_G,
	shortcut_extract		= Qt::CTRL + Qt::Key_E,

	shortcut_first_file_sync= Qt::ALT + Qt::Key_Home, 
	shortcut_last_file_sync	= Qt::ALT + Qt::Key_End,
	shortcut_prev_file_sync	= Qt::ALT + Qt::Key_Left,
	shortcut_next_file_sync	= Qt::ALT + Qt::Key_Right,

	// view
	shortcut_new_tab		= Qt::CTRL + Qt::Key_T,
	shortcut_close_tab		= Qt::CTRL + Qt::Key_W,
	shortcut_next_tab		= Qt::CTRL + Qt::Key_Tab,
	shortcut_previous_tab	= Qt::CTRL + Qt::SHIFT + Qt::Key_Tab,
	shortcut_show_toolbar	= Qt::CTRL + Qt::Key_B,
	shortcut_show_statusbar	= Qt::CTRL + Qt::Key_I,
	shortcut_full_screen_ad	= Qt::CTRL + Qt::Key_L,
	shortcut_show_transfer	= Qt::CTRL + Qt::Key_U,
#ifdef Q_WS_MAC
	shortcut_full_screen_ff	= Qt::CTRL + Qt::Key_F,
	shortcut_frameless		= Qt::CTRL + Qt::Key_R,
#else
	shortcut_full_screen_ff	= Qt::Key_F11,
	shortcut_frameless		= Qt::Key_F10,
#endif
	shortcut_reset_view 	= Qt::CTRL + Qt::Key_0,
	shortcut_zoom_full		= Qt::CTRL + Qt::Key_1,
	shortcut_fit_frame		= Qt::CTRL + Qt::Key_2,
	shortcut_show_overview	= Qt::Key_O,
	shortcut_show_explorer	= Qt::Key_E,
	shortcut_show_metadata_dock = Qt::ALT + Qt::Key_M,
	shortcut_show_player	= Qt::Key_P,
	shortcut_show_exif		= Qt::Key_M,
	shortcut_show_info		= Qt::Key_I,
	shortcut_show_histogram	= Qt::Key_H,
	shortcut_show_comment	= Qt::Key_N,
	shortcut_opacity_down	= Qt::CTRL + Qt::Key_J,
	shortcut_opacity_up		= Qt::CTRL + Qt::SHIFT + Qt::Key_J,
	shortcut_opacity_change	= Qt::ALT + Qt::SHIFT + Qt::Key_J,
	shortcut_an_opacity		= Qt::ALT  + Qt::Key_J,
	shortcut_new_instance	= Qt::CTRL + Qt::Key_N,
	shortcut_private_instance = Qt::CTRL + Qt::ALT + Qt::Key_N,
	shortcut_tp_pattern		= Qt::Key_B,
	shortcut_anti_aliasing	= Qt::Key_A,
	shortcut_lock_window	= Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_B,
	shortcut_recent_files	= Qt::CTRL + Qt::Key_H,
	//shortcut_play			= Qt::Key_Space,

	// edit
	shortcut_rotate_cw		= Qt::Key_R,
	shortcut_rotate_ccw		= Qt::SHIFT + Qt::Key_R,
	shortcut_transform		= Qt::CTRL + Qt::Key_R,
	shortcut_manipulation   = Qt::CTRL + Qt::SHIFT + Qt::Key_M,
	shortcut_paste			= Qt::Key_Insert,
	shortcut_delete_silent	= Qt::SHIFT + Qt::Key_Delete,
	shortcut_crop			= Qt::Key_C,
	shortcut_copy_buffer	= Qt::CTRL + Qt::SHIFT + Qt::Key_C,
	shortcut_copy_color		= Qt::CTRL + Qt::ALT + Qt::Key_C,
	shortcut_auto_adjust	= Qt::CTRL + Qt::SHIFT + Qt::Key_L,
	shortcut_norm_image		= Qt::CTRL + Qt::SHIFT + Qt::Key_N,

	// tcp
	shortcut_shortcuts		= Qt::CTRL + Qt::Key_K,
	shortcut_settings		= Qt::CTRL + Qt::SHIFT + Qt::Key_P,
	shortcut_sync			= Qt::CTRL + Qt::Key_D,
	shortcut_tab			= Qt::ALT + Qt::Key_O,
	shortcut_arrange		= Qt::ALT + Qt::Key_A,
	shortcut_send_img		= Qt::ALT + Qt::Key_I,
	shortcut_connect_all	= Qt::CTRL + Qt::Key_A,

	// help
	shortcut_show_help		= Qt::Key_F1,

	// eggs
	shortcut_pong			= Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_P,
	shortcut_test_img		= Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_L,
	shortcut_test_rec		= Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_R,
	shortcut_shiver			= Qt::CTRL + Qt::Key_W,
};

enum fileActions {
	menu_file_open,
	menu_file_open_dir,
	menu_file_app_manager,
	menu_file_save,
	menu_file_save_as,
	menu_file_save_web,
	menu_file_rename,
	menu_file_goto,
	menu_file_find,
	menu_file_recursive,
	menu_file_show_recent,
	menu_file_print,
	menu_file_reload,
	menu_file_next,
	menu_file_prev,
	menu_file_train_format,
	menu_file_new_instance,
	menu_file_private_instance,
	menu_file_exit,
	//menu_file_share_fb,

	menu_file_end,	// nothing beyond this point
};

enum sortActions {

	menu_sort_filename,
	menu_sort_date_created,
	menu_sort_date_modified,
	menu_sort_random,
	menu_sort_ascending,
	menu_sort_descending,

	menu_sort_end,
};

enum editActions {
	menu_edit_rotate_cw,
	menu_edit_rotate_ccw,
	menu_edit_rotate_180,
	menu_edit_copy,
	menu_edit_copy_buffer,
	menu_edit_copy_color,
	menu_edit_paste,
	menu_edit_shortcuts,
	menu_edit_preferences,
	menu_edit_transform,
	menu_edit_delete,
	menu_edit_crop,
	menu_edit_flip_h,
	menu_edit_flip_v,
	menu_edit_invert,
	menu_edit_norm,
	menu_edit_auto_adjust,
	menu_edit_unsharp,
	menu_edit_wallpaper,

	menu_edit_end,	// nothing beyond this point
};

enum toolsActions {
	menu_tools_thumbs,
	menu_tools_filter,
	menu_tools_manipulation,
	menu_tools_export_tiff,
	menu_tools_extract_archive,
	menu_tools_mosaic,
	menu_tools_batch,

	menu_tools_end,
};

enum panelActions {
	menu_panel_menu,
	menu_panel_toolbar,
	menu_panel_statusbar,
	menu_panel_transfertoolbar,

	menu_panel_player,
	menu_panel_preview,
	menu_panel_thumbview,
	menu_panel_scroller,
	menu_panel_exif,
	menu_panel_info,
	menu_panel_histogram,
	menu_panel_overview,
	menu_panel_explorer,
	menu_panel_metadata_dock,
	menu_panel_comment,

	menu_panel_end,
};

enum viewActions {
	menu_view_fullscreen,
	menu_view_reset,
	menu_view_100,
	menu_view_fit_frame,
	menu_view_zoom_in,
	menu_view_zoom_out,
	menu_view_anti_aliasing,
	menu_view_tp_pattern,
	menu_view_frameless,
	menu_view_new_tab,
	menu_view_close_tab,
	menu_view_previous_tab,
	menu_view_next_tab,
	menu_view_opacity_up,
	menu_view_opacity_down,
	menu_view_opacity_an,
	menu_view_opacity_change,
	menu_view_lock_window,
	menu_view_gps_map,
	menu_view_movie_pause,
	menu_view_movie_next,
	menu_view_movie_prev,

	menu_view_end,	// nothing beyond this point
};

enum syncActions {
	menu_sync,
	menu_sync_pos,
	menu_sync_arrange,
	menu_sync_connect_all,
	menu_sync_all_actions,
	menu_sync_start_upnp,

	menu_sync_remote_control,
	menu_sync_remote_display,

	menu_sync_end,	// nothing beyond this point
};

enum pluginsActions {
	menu_plugin_manager,
	
	menu_plugins_end,	// nothing beyond this point
};

enum lanSyncActions {
	menu_lan_server,
	menu_lan_image,

	menu_lan_end,
};

enum helpActions {
	menu_help_update,
	menu_help_update_translation,
	menu_help_bug,
	menu_help_feature,
	menu_help_documentation,
	menu_help_about,
	
	menu_help_end,	// nothing beyond this point
};

enum shortcuts {
	sc_test_img,
	sc_test_rec,

	sc_end,	// nothing beyond this point
};

enum fileIcons {
	icon_file_prev,
	icon_file_next,
	icon_file_dir,
	icon_file_open,
	icon_file_open_large,
	icon_file_dir_large,
	icon_file_save,
	icon_file_print,
	icon_file_filter,

	icon_file_end,	// nothing beyond this point
};

enum editIcons {
	icon_edit_rotate_cw,
	icon_edit_rotate_ccw,
	icon_edit_crop,
	icon_edit_resize,

	icon_edit_end,	// nothing beyond this point
};

enum viewIcons {
	icon_view_fullscreen,
	icon_view_reset,
	icon_view_100,
	icon_view_gps,
	icon_view_movie_play,
	icon_view_movie_prev,
	icon_view_movie_next,

	icon_view_end,	// nothing beyond this point
};

enum toolsIcons {
	icon_tools_manipulation,

	icon_tools_end,
};

enum statusbarLabels {
	status_pixel_info,
	status_filesize_info,
	status_time_info,

	status_end,

};

/*! A custom event filter for OSX.
Its main purpose is to provide Mac OS X "open file from finder"
functionality for nomacs. It *has* to be catched on the
QApplication level in the event().
*/
class DkNomacsOSXEventFilter : public QObject {
	Q_OBJECT

public:
	DkNomacsOSXEventFilter(QObject *parent = 0);

signals:
	void loadFile(const QFileInfo &fi);

protected:
	/*! Handle QFileOpenEvent for mac here */
	bool eventFilter(QObject *obj, QEvent *event);

};

class DkMenuBar;

class DllExport DkNoMacs : public QMainWindow {
	Q_OBJECT

public:

	virtual ~DkNoMacs();

	void release();
	
	static int infoDialog(QString msg, QWidget* parent = 0, QString title = "Question");
	static int dialog(QString msg, QWidget* parent = 0, QString title = "Error");
	static QWidget* getDialogParent();

	virtual DkViewPort* viewport() const;
	virtual DkCentralWidget* getTabWidget() const;
	
	QVector<QAction* > getFileActions();
	QVector<QAction* > getBatchActions();
	QVector<QAction* > getPanelActions();
	QVector<QAction* > getViewActions();
	QVector<QAction* > getSyncActions();
	void loadFile(const QFileInfo& file);

	static void updateAll();

	bool saveSettings;

	QString getCurrRunningPlugin() {return currRunningPlugin;};
	void colorizeIcons(QColor col);

signals:
	void sendTitleSignal(QString newTitle);
	void sendPositionSignal(QRect newRect, bool overlaid);
	void sendArrangeSignal(bool overlaid);
	void synchronizeWithSignal(quint16);
	void stopSynchronizeWithSignal();
	void synchronizeWithServerPortSignal(quint16);
	void synchronizeRemoteControl(quint16);
	void closeSignal();
	void saveTempFileSignal(QImage img);
	void sendQuitLocalClientsSignal();

public slots:
	void restart();
	void toggleFullScreen();
	void enterFullScreen();
	void exitFullScreen();
	void openKeyboardShortcuts();
	void openSettings();
	void showExplorer(bool show);
	void showMetaDataDock(bool show);
	void showThumbsDock(bool show);
	void thumbsDockAreaChanged();
	void showRecentFiles(bool show = true);
	void openDir();
	void openFile();
	void renameFile();
	void changeSorting(bool change);
	void goTo();
	void find(bool filterAction = true);
	void updateFilterState(QStringList filters);
	void saveFile();
	void saveFileAs(bool silent = false);
	void saveFileWeb();
	void extractImagesFromArchive();
	void trainFormat();
	void resizeImage();
	void openImgManipulationDialog();
	void exportTiff();
	void computeMosaic();
	void computeBatch();
	void deleteFile();
	void openAppManager();
	void setWallpaper();
	void printDialog();
	void cleanSettings();
	void newInstance(QFileInfo file = QFileInfo());
	void showStatusBar(bool show, bool permanent = true);
	void showMenuBar(bool show);
	void showToolbarsTemporarily(bool show);
	void showToolbar(bool show);
	void showToolbar(QToolBar* toolbar, bool show);
	void showGpsCoordinates();
	void openFileWith(QAction* action);
	void aboutDialog();
	void openDocumentation();
	void bugReport();
	void featureRequest();
	//void errorDialog(QString msg, QString title = "Error");
	void errorDialog(const QString& msg);
	void loadRecursion();
	void setWindowTitle(QSharedPointer<DkImageContainerT> imgC);
	void setWindowTitle(QFileInfo file, QSize size = QSize(), bool edited = false, QString attr = QString());
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
	virtual void newClientConnected(bool connected, bool local);
	void showStatusMessage(QString msg, int which = status_pixel_info);
	void flipImageHorizontal();
	void flipImageVertical();
	void normalizeImage();
	void autoAdjustImage();
	void unsharpMask();
	void invertImage();
	virtual void settingsChanged();
	void showUpdaterMessage(QString msg, QString title);
	void showUpdateDialog(QString msg, QString title);
	void performUpdate();
	void updateProgress(qint64 received, qint64 total);
	void startSetup(QString);
	void updateTranslations();
	virtual void enableNoImageActions(bool enable = true);
	void checkForUpdate();
	void setFrameless(bool frameless);
	void fitFrame();
	void setRecursiveScan(bool recursive);
	void setContrast(bool contrast);
	void enableMovieActions(bool enable);
	void runLoadedPlugin();
	void openPluginManager();
	void initPluginManager();
	void runPluginFromShortcut();
	void closePlugin(bool askForSaving, bool alreadySaving);
	void applyPluginChanges(bool askForSaving, bool alreadySaving);
	void clearFileHistory();
	void clearFolderHistory();
	//void shareFacebook();

	// batch actions
	void computeThumbsBatch();
	void onWindowLoaded();

protected:
	
	// mouse events
	void mouseDoubleClickEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

	// mouse events
	void moveEvent(QMoveEvent *event);

	// window events
	void contextMenuEvent(QContextMenuEvent *event);
	bool eventFilter(QObject *obj, QEvent *event);
	void resizeEvent(QResizeEvent *event);
	void closeEvent(QCloseEvent *event);

	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent* event);

	// TODO: put to android class
	bool gestureEvent(QGestureEvent *event);

	void assignCustomShortcuts(QVector<QAction*> actions);
	void assignCustomPluginShortcuts();
	void savePluginActions(QVector<QAction *> actions);

	bool otherKeyPressed;
	QPoint posGrabKey;
	bool overlaid;

	// vars
	QWidget *parent;
	DkPluginManager* pluginManager;
	QString currRunningPlugin;

	QVector<QShortcut*> shortcuts;	
	QVector<QAction *> fileActions;
	QVector<QAction *> sortActions;
	QVector<QAction *> openWithActions;
	QVector<QAction *> editActions;
	QVector<QAction *> toolsActions;
	QVector<QAction *> panelActions;
	QVector<QAction *> viewActions;
	QVector<QAction *> syncActions;
	QVector<QAction *> pluginsActions;
	QVector<QAction *> pluginsDummyActions;
	QVector<QAction *> lanActions;
	QVector<QAction *> helpActions;
	//QVector<QAction *> tcpViewerActions;
	
	// icons
	QVector<QIcon> fileIcons;
	QVector<QIcon> editIcons;
	QVector<QIcon> viewIcons;
	QVector<QIcon> toolsIcons;

	// menu
	DkMenuBar* menu;
	QMenu* fileMenu;
	QMenu* sortMenu;
	QMenu* openWithMenu;
	QMenu* editMenu;
	QMenu* toolsMenu;
	QMenu* panelMenu;
	QMenu* viewMenu;
	QMenu* syncMenu;
	QMenu* pluginsMenu;
	QMenu* helpMenu;
	QMenu* contextMenu;

	// sub menus
	//QMenu* fileFilesMenu;
	//QMenu* fileFoldersMenu;
	QMenu* panelToolsMenu;
	DkTcpMenu* tcpViewerMenu;
	DkTcpMenu* tcpLanMenu;
	
	QPoint mousePos;
	
	// toolbar
	QToolBar* toolbar;
	QToolBar* movieToolbar;
	QStatusBar* statusbar;
	QVector<QLabel*> statusbarLabels;
	

	// file dialog
	QFileDialog* openDialog;
	QFileDialog* saveDialog;
	DkCompressDialog* jpgDialog;
	DkTifDialog* tifDialog;
	DkOpacityDialog* opacityDialog;
	DkResizeDialog* resizeDialog;
	DkUpdateDialog* updateDialog;
	QProgressDialog* progressDialog;
	DkForceThumbDialog* forceDialog;
	DkTrainDialog* trainDialog;
#ifdef WITH_QUAZIP
	DkArchiveExtractionDialog* archiveExtractionDialog;
#endif
	DkExplorer* explorer;
	DkMetaDataDock* metaDataDock;
	QDockWidget* thumbsDock;
	DkExportTiffDialog* exportTiffDialog;
	DkSettingsDialog* settingsDialog;
	DkThumbsSaver* thumbSaver;
	DkBatchDialog* batchDialog;
	DkImageManipulationDialog* imgManipulationDialog;

	DkPrintPreviewDialog* printPreviewDialog;

	DkAppManager* appManager;

	// client managers
	//DkLocalClientManager* localClientManager;
	//DkLANClientManager* lanClientManager;
	DkUpdater* updater;
	DkTranslationUpdater* translationUpdater;	
	

	QRect oldGeometry;
	QList<QToolBar *> hiddenToolbars;

	QProcess process;

	// functions
	DkNoMacs(QWidget *parent = 0, Qt::WindowFlags flags = 0);

	virtual void init();
	
	void loadStyleSheet();
	void registerFileVersion();
	virtual void createIcons();
	virtual void createToolbar();
	virtual void createShortcuts();
	virtual void createActions();
	virtual void createMenu();
	virtual void createOpenWithMenu(QMenu* menu);
	virtual void createContextMenu();
	virtual void createStatusbar();

	virtual void readSettings();

	// plugin functions
	void addPluginsToMenu();
	void createPluginsMenu();
};

class DllExport DkNoMacsSync : public DkNoMacs {
	Q_OBJECT

public:
	DkNoMacsSync(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~DkNoMacsSync();
	qint16 getServerPort();
	void syncWith(qint16 port);
	
signals:
	void clientInitializedSignal();
	void startRCServerSignal(bool start);
	void startTCPServerSignal(bool start);

public slots:
	void tcpConnectAll();
	void tcpChangeSyncMode(int syncMode, bool connectWithWhiteList = false);
	void tcpRemoteControl(bool start);
	void tcpRemoteDisplay(bool start);
	void tcpAutoConnect(bool connect);
	void startUpnpRenderer(bool start);
	void settingsChanged();
	void clientInitialized();
	void newClientConnected(bool connected, bool local);
	void startTCPServer(bool start);
	
protected:

	// mouse events
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

	// functions
	void initLanClient();
	bool connectWhiteList(int mode, bool connect = true);

	// gui
	virtual void createActions();
	virtual void createMenu();

	// network layer
	DkLocalManagerThread* localClient;
	DkLanManagerThread* lanClient;
	DkRCManagerThread* rcClient;
#ifdef WITH_UPNP
	QSharedPointer<DkUpnpControlPoint> upnpControlPoint;
	QSharedPointer<DkUpnpDeviceHost> upnpDeviceHost;
	QSharedPointer<DkUpnpRendererDeviceHost> upnpRendererDeviceHost;
#endif // WITH_UPNP

};

class DllExport DkNoMacsIpl : public DkNoMacsSync {
	Q_OBJECT

public:
	DkNoMacsIpl(QWidget *parent = 0, Qt::WindowFlags flags = 0);

};



class DllExport DkNoMacsFrameless : public DkNoMacs {
	Q_OBJECT

public:
	DkNoMacsFrameless(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	
	virtual ~DkNoMacsFrameless();

	void release();
	
public slots:
	virtual void enableNoImageActions(bool enable = true);
	void exitFullScreen();
	void updateScreenSize(int screen = 0);

protected:
	void closeEvent(QCloseEvent *event);
	bool eventFilter(QObject *obj, QEvent *event);
	virtual void createContextMenu();

	QDesktopWidget* dw;
};

class DllExport DkNoMacsContrast : public DkNoMacsSync {
	Q_OBJECT

public:
	DkNoMacsContrast(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	
	virtual ~DkNoMacsContrast();
	void release();	

protected:
	void createTransferToolbar();

	DkTransferToolBar* transferToolBar;

};
};
