/*******************************************************************************************************
 DkActionManager.h
 Created on:	28.10.2015

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2015 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2015 Florian Kleber <florian@nomacs.org>

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

#include "DkBaseWidgets.h"
#include "DkManipulators.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAction>
#include <QObject>
#pragma warning(pop) // no warnings from includes - end

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

#pragma warning(disable : 4251) // TODO: remove

// Qt defines
class QMenu;
class QMainWindow;
class QWinTaskbarProgress;
class QProgressDialog;

namespace nmc
{

// nomacs defines
class DkTcpMenu;
class DkPluginActionManager;

class DllCoreExport DkAppManager : public QObject
{
    Q_OBJECT

public:
    DkAppManager(QWidget *parent = nullptr);
    ~DkAppManager();

    void setActions(QVector<QAction *> actions);
    QVector<QAction *> getActions() const;
    QAction *createAction(const QString &filePath);
    QAction *findAction(const QString &appPath) const;

    enum defaultAppIdx {
        app_photohsop,
        app_picasa,
        app_picasa_viewer,
        app_irfan_view,
        app_explorer,

        app_idx_end
    };

public slots:
    void openTriggered() const;

signals:
    void openFileSignal(QAction *action) const;

protected:
    void saveSettings() const;
    void loadSettings();
    void assignIcon(QAction *app) const;
    bool containsApp(QVector<QAction *> apps, const QString &appName) const;

    QString searchForSoftware(const QString &organization,
                              const QString &application,
                              const QString &pathKey = "",
                              const QString &exeName = "") const;
    void findDefaultSoftware();

    QVector<QString> mDefaultNames;
    QVector<QAction *> mApps;
    bool mFirstTime = true;
};

class DllCoreExport DkActionManager
{
public:
    static DkActionManager &instance();
    ~DkActionManager();

    // singleton
    DkActionManager(DkActionManager const &) = delete;
    void operator=(DkActionManager const &) = delete;

    enum FileMenuActions {
        menu_file_open,
        menu_file_open_dir,
        menu_file_open_list,
        menu_file_quick_launch,
        menu_file_app_manager,
        menu_file_save,
        menu_file_save_as,
        menu_file_save_copy,
        menu_file_save_list,
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
        menu_file_new_instance,
        menu_file_private_instance,
        menu_file_exit,

        menu_file_end, // nothing beyond this point
    };

    enum SortMenuActions {
        menu_sort_filename,
        menu_sort_file_size,
        menu_sort_date_created,
        menu_sort_date_modified,
        menu_sort_random,
        menu_sort_ascending,
        menu_sort_descending,

        menu_sort_end,
    };

    enum EditMenuActions {
        menu_edit_image,
        menu_edit_rotate_cw,
        menu_edit_rotate_ccw,
        menu_edit_rotate_180,
        menu_edit_undo,
        menu_edit_redo,
        menu_edit_copy,
        menu_edit_copy_buffer,
        menu_edit_copy_color,
        menu_edit_paste,
        menu_edit_shortcuts,
        menu_edit_preferences,
        menu_edit_transform,
        menu_edit_delete,
        menu_edit_crop,

        menu_edit_end, // nothing beyond this point
    };

    enum ToolsMenuActions {
        menu_tools_thumbs,
        menu_tools_filter,
        menu_tools_export_tiff,
        menu_tools_extract_archive,
        menu_tools_mosaic,
        menu_tools_batch,
        menu_tools_wallpaper,
        menu_tools_train_format,

        menu_tools_end,
    };

    enum PanelMenuActions {
        menu_panel_toggle,

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
        menu_panel_history,
        menu_panel_log,

        menu_panel_end,
    };

    enum ViewMenuActions {
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
        menu_view_close_all_tabs,
        menu_view_first_tab,
        menu_view_previous_tab,
        menu_view_goto_tab,
        menu_view_next_tab,
        menu_view_last_tab,

        menu_view_opacity_up,
        menu_view_opacity_down,
        menu_view_opacity_an,
        menu_view_opacity_change,
        menu_view_lock_window,
        menu_view_gps_map,
        menu_view_slideshow,
        menu_view_movie_pause,
        menu_view_movie_next,
        menu_view_movie_prev,

        menu_view_monitors, // frameless only

        menu_view_end, // nothing beyond this point
    };

    enum SyncMenuActions {
        menu_sync_view,
        menu_sync_pos,
        menu_sync_arrange,
        menu_sync_connect_all,
        menu_sync_all_actions,

        menu_sync_end, // nothing beyond this point
    };

    enum PluginMenuActions {
        menu_plugin_manager,

        menu_plugins_end, // nothing beyond this point
    };

    enum HelpMenuActions {
        menu_help_update,
        menu_help_update_translation,
        menu_help_bug,
        menu_help_documentation,
        menu_help_about,

        menu_help_end, // nothing beyond this point
    };

    enum HiddenActions {
        sc_test_img,
        sc_test_rec,
        sc_test_pong,

        sc_first_file,
        sc_last_file,
        sc_skip_prev,
        sc_skip_next,
        sc_skip_next_sync,
        sc_skip_prev_sync,
        sc_first_file_sync,
        sc_last_file_sync,
        sc_delete_silent,

        sc_star_rating_0,
        sc_star_rating_1,
        sc_star_rating_2,
        sc_star_rating_3,
        sc_star_rating_4,
        sc_star_rating_5,

        sc_pan_up,
        sc_pan_down,
        sc_pan_left,
        sc_pan_right,

        sc_end, // nothing beyond this point
    };

    enum PreviewActions {
        preview_select_all,
        preview_zoom_in,
        preview_zoom_out,
        preview_display_squares,
        preview_show_labels,
        preview_copy,
        preview_paste,
        preview_rename,
        preview_delete,
        preview_filter,
        preview_batch,
        preview_print,

        actions_end
    };

    enum FileIcons {
        icon_file_prev,
        icon_file_next,
        icon_file_dir,
        icon_file_open,
        icon_file_open_large,
        icon_file_dir_large,
        icon_file_save,
        icon_file_print,
        icon_file_filter,
        icon_file_find,

        icon_file_end, // nothing beyond this point
    };

    enum EditIcons {
        icon_edit_image,
        icon_edit_rotate_cw,
        icon_edit_rotate_ccw,
        icon_edit_crop,
        icon_edit_resize,
        icon_edit_copy,
        icon_edit_paste,
        icon_edit_delete,
        icon_edit_undo,
        icon_edit_redo,

        icon_edit_end, // nothing beyond this point
    };

    enum ViewIcons {
        icon_view_fullscreen,
        icon_view_reset,
        icon_view_100,
        icon_view_gps,
        icon_view_movie_play,
        icon_view_movie_prev,
        icon_view_movie_next,
        icon_view_zoom_in,
        icon_view_zoom_out,

        icon_view_end, // nothing beyond this point
    };

    // default nomacs shortcuts
    // keyboard shortcuts
    // general
    static constexpr QKeyCombination shortcut_esc = Qt::Key_Escape;

    // file
    static constexpr QKeyCombination shortcut_open_preview = Qt::Key_T;
    static constexpr QKeyCombination shortcut_open_thumbview = Qt::SHIFT | Qt::Key_T;
    static constexpr QKeyCombination shortcut_open_dir = Qt::CTRL | Qt::SHIFT | Qt::Key_O;
    static constexpr QKeyCombination shortcut_app_manager = Qt::CTRL | Qt::Key_M;
    static constexpr QKeyCombination shortcut_save_as = Qt::CTRL | Qt::SHIFT | Qt::Key_S;
    static constexpr QKeyCombination shortcut_first_file = Qt::Key_Home;
    static constexpr QKeyCombination shortcut_last_file = Qt::Key_End;
    static constexpr QKeyCombination shortcut_skip_prev = Qt::Key_PageUp;
    static constexpr QKeyCombination shortcut_skip_next = Qt::Key_PageDown;
    static constexpr QKeyCombination shortcut_prev_file = Qt::Key_Left;
    static constexpr QKeyCombination shortcut_next_file = Qt::Key_Right;
    static constexpr QKeyCombination shortcut_rename = Qt::Key_F2;
    static constexpr QKeyCombination shortcut_goto = Qt::CTRL | Qt::Key_G;
    static constexpr QKeyCombination shortcut_extract = Qt::CTRL | Qt::Key_E;
    static constexpr QKeyCombination shortcut_reload = Qt::Key_F5;

    static constexpr QKeyCombination shortcut_first_file_sync = Qt::ALT | Qt::Key_Home;
    static constexpr QKeyCombination shortcut_last_file_sync = Qt::ALT | Qt::Key_End;
    static constexpr QKeyCombination shortcut_skip_prev_sync = Qt::ALT | Qt::Key_Left;
    static constexpr QKeyCombination shortcut_skip_next_sync = Qt::ALT | Qt::Key_Right;

    static constexpr QKeyCombination shortcut_star_rating_0 = Qt::Key_0;
    static constexpr QKeyCombination shortcut_star_rating_1 = Qt::Key_1;
    static constexpr QKeyCombination shortcut_star_rating_2 = Qt::Key_2;
    static constexpr QKeyCombination shortcut_star_rating_3 = Qt::Key_3;
    static constexpr QKeyCombination shortcut_star_rating_4 = Qt::Key_4;
    static constexpr QKeyCombination shortcut_star_rating_5 = Qt::Key_5;

    // view
    static constexpr QKeyCombination shortcut_new_tab = Qt::CTRL | Qt::Key_T;
    static constexpr QKeyCombination shortcut_close_tab = Qt::CTRL | Qt::Key_W;
    static constexpr QKeyCombination shortcut_show_toolbar = Qt::CTRL | Qt::Key_B;
    static constexpr QKeyCombination shortcut_show_statusbar = Qt::CTRL | Qt::Key_I;
    static constexpr QKeyCombination shortcut_show_transfer = Qt::CTRL | Qt::Key_U;
#ifdef Q_OS_MAC
    static constexpr QKeyCombination shortcut_next_tab = Qt::META | Qt::Key_Tab;
    static constexpr QKeyCombination shortcut_previous_tab = Qt::META | Qt::SHIFT | Qt::Key_Tab;
    static constexpr QKeyCombination shortcut_full_screen_ff = Qt::CTRL | Qt::Key_F;
    static constexpr QKeyCombination shortcut_frameless = Qt::CTRL | Qt::Key_R;
#else
    static constexpr QKeyCombination shortcut_next_tab = Qt::CTRL | Qt::Key_Tab;
    static constexpr QKeyCombination shortcut_previous_tab = Qt::CTRL | Qt::SHIFT | Qt::Key_Tab;
    static constexpr QKeyCombination shortcut_full_screen_ff = Qt::Key_F11;
    static constexpr QKeyCombination shortcut_frameless = Qt::Key_F10;
#endif
    static constexpr QKeyCombination shortcut_reset_view = Qt::CTRL | Qt::Key_0;
    static constexpr QKeyCombination shortcut_zoom_full = Qt::CTRL | Qt::Key_1;
    static constexpr QKeyCombination shortcut_fit_frame = Qt::CTRL | Qt::Key_2;
    static constexpr QKeyCombination shortcut_show_overview = Qt::Key_O;
    static constexpr QKeyCombination shortcut_show_explorer = Qt::Key_E;
    static constexpr QKeyCombination shortcut_show_metadata_dock = Qt::ALT | Qt::Key_M;
    static constexpr QKeyCombination shortcut_show_history = Qt::CTRL | Qt::SHIFT | Qt::Key_H;
    static constexpr QKeyCombination shortcut_show_log = Qt::CTRL | Qt::ALT | Qt::Key_L;
    static constexpr QKeyCombination shortcut_view_slideshow = Qt::Key_Space;
    static constexpr QKeyCombination shortcut_show_player = Qt::Key_P;
    static constexpr QKeyCombination shortcut_show_exif = Qt::Key_M;
    static constexpr QKeyCombination shortcut_show_info = Qt::Key_I;
    static constexpr QKeyCombination shortcut_show_histogram = Qt::Key_H;
    static constexpr QKeyCombination shortcut_show_comment = Qt::Key_N;
    static constexpr QKeyCombination shortcut_opacity_down = Qt::CTRL | Qt::Key_J;
    static constexpr QKeyCombination shortcut_opacity_up = Qt::CTRL | Qt::SHIFT | Qt::Key_J;
    static constexpr QKeyCombination shortcut_opacity_change = Qt::ALT | Qt::SHIFT | Qt::Key_J;
    static constexpr QKeyCombination shortcut_an_opacity = Qt::ALT | Qt::Key_J;
    static constexpr QKeyCombination shortcut_new_instance = Qt::CTRL | Qt::Key_N;
    static constexpr QKeyCombination shortcut_private_instance = Qt::CTRL | Qt::ALT | Qt::Key_N;
    static constexpr QKeyCombination shortcut_tp_pattern = Qt::Key_B;
    static constexpr QKeyCombination shortcut_anti_aliasing = Qt::Key_A;
    static constexpr QKeyCombination shortcut_lock_window = Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::Key_B;
    static constexpr QKeyCombination shortcut_recent_files = Qt::CTRL | Qt::Key_H;
    static constexpr QKeyCombination shortcut_toggle_panels = Qt::Key_F;

    // hidden viewport static constexpr QKeyCombination shortcuts
    static constexpr QKeyCombination shortcut_pan_left = Qt::CTRL | Qt::Key_Left;
    static constexpr QKeyCombination shortcut_pan_right = Qt::CTRL | Qt::Key_Right;
    static constexpr QKeyCombination shortcut_pan_up = Qt::CTRL | Qt::Key_Up;
    static constexpr QKeyCombination shortcut_pan_down = Qt::CTRL | Qt::Key_Down;

    static constexpr QKeyCombination shortcut_zoom_in = Qt::Key_Plus;
    static constexpr QKeyCombination shortcut_zoom_out = Qt::Key_Minus;
    static constexpr QKeyCombination shortcut_zoom_in_alt = Qt::Key_Up;
    static constexpr QKeyCombination shortcut_zoom_out_alt = Qt::Key_Down;

    // edit
    static constexpr QKeyCombination shortcut_edit_image = Qt::Key_D;
    static constexpr QKeyCombination shortcut_rotate_cw = Qt::Key_R;
    static constexpr QKeyCombination shortcut_rotate_ccw = Qt::SHIFT | Qt::Key_R;
    static constexpr QKeyCombination shortcut_transform = Qt::CTRL | Qt::Key_R;
    static constexpr QKeyCombination shortcut_manipulation = Qt::CTRL | Qt::SHIFT | Qt::Key_M;
    static constexpr QKeyCombination shortcut_paste = Qt::Key_Insert;
    static constexpr QKeyCombination shortcut_delete_silent = Qt::SHIFT | Qt::Key_Delete;
    static constexpr QKeyCombination shortcut_crop = Qt::Key_C;
    static constexpr QKeyCombination shortcut_copy_buffer = Qt::CTRL | Qt::SHIFT | Qt::Key_C;
    static constexpr QKeyCombination shortcut_copy_color = Qt::CTRL | Qt::ALT | Qt::Key_C;

    // tools
    static constexpr QKeyCombination shortcut_batch_processing = Qt::CTRL | Qt::SHIFT | Qt::Key_B;

    // tcp
    static constexpr QKeyCombination shortcut_shortcuts = Qt::CTRL | Qt::Key_K;
    static constexpr QKeyCombination shortcut_settings = Qt::CTRL | Qt::SHIFT | Qt::Key_P;
    static constexpr QKeyCombination shortcut_sync = Qt::CTRL | Qt::Key_D;
    static constexpr QKeyCombination shortcut_tab = Qt::ALT | Qt::Key_O;
    static constexpr QKeyCombination shortcut_arrange = Qt::ALT | Qt::Key_A;
    static constexpr QKeyCombination shortcut_send_img = Qt::ALT | Qt::Key_I;
    static constexpr QKeyCombination shortcut_connect_all = Qt::CTRL | Qt::Key_A;

    // help
    static constexpr QKeyCombination shortcut_show_help = Qt::Key_F1;

    // eggs
    static constexpr QKeyCombination shortcut_test_pong = Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::Key_P;
    static constexpr QKeyCombination shortcut_test_img = Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::Key_L;
    static constexpr QKeyCombination shortcut_test_rec = Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::Key_R;

    QMenu *updateOpenWithMenu();

    QMenu *fileMenu() const;
    QMenu *sortMenu() const;
    QMenu *openWithMenu() const;
    QMenu *viewMenu() const;
    QMenu *editMenu() const;
    QMenu *manipulatorMenu() const;
    QMenu *toolsMenu() const;
    QMenu *panelMenu() const;
    QMenu *helpMenu() const;
    QMenu *contextMenu() const;
    QMenu *syncMenu() const;
    DkTcpMenu *localMenu() const;

    DkManipulatorManager manipulatorManager() const;

    void createActions(QWidget *parent);
    void createMenus(QWidget *parent);

    QAction *action(FileMenuActions action) const;
    QAction *action(SortMenuActions action) const;
    QAction *action(ViewMenuActions action) const;
    QAction *action(EditMenuActions action) const;
    QAction *action(ToolsMenuActions action) const;
    QAction *action(PanelMenuActions action) const;
    QAction *action(SyncMenuActions action) const;
    QAction *action(PluginMenuActions action) const;
    QAction *action(HelpMenuActions action) const;
    QAction *action(HiddenActions action) const;
    QAction *action(PreviewActions action) const;

    QIcon icon(FileIcons icon) const;
    QIcon icon(ViewIcons icon) const;
    QIcon icon(EditIcons icon) const;

    QVector<QAction *> fileActions() const;
    QVector<QAction *> sortActions() const;
    QVector<QAction *> openWithActions() const;
    QVector<QAction *> viewActions() const;
    QVector<QAction *> editActions() const;
    QVector<QAction *> toolsActions() const;
    QVector<QAction *> panelActions() const;
    QVector<QAction *> syncActions() const;
    QVector<QAction *> pluginActions() const;
    QVector<QAction *> helpActions() const;
    QVector<QAction *> previewActions() const;
    QVector<QAction *> manipulatorActions() const;

    QVector<QAction *> allActions() const;

    QVector<QAction *> hiddenActions() const;
    DkAppManager *appManager() const;
    DkPluginActionManager *pluginActionManager() const;

    void assignCustomShortcuts(QVector<QAction *> actions) const;

    /// Enable actions when an image is loaded in the viewport
    void enableImageActions(bool enable = true) const;

    /// Enable actions for animations in the viewport
    void enableMovieActions(bool enable = true) const;

    /// Enable actions for viewport plugins
    void enableViewPortPluginActions(bool enable) const;

protected:
    DkActionManager();

    void init();
    void createIcons();

    QMenu *createFileMenu(QWidget *parent);
    QMenu *createSortMenu(QWidget *parent);
    QMenu *createOpenWithMenu(QWidget *parent);
    QMenu *createViewMenu(QWidget *parent);
    QMenu *createEditMenu(QWidget *parent);
    QMenu *createManipulatorMenu(QWidget *parent);
    QMenu *createToolsMenu(QWidget *parent);
    QMenu *createPanelMenu(QWidget *parent);
    QMenu *createHelpMenu(QWidget *parent);
    QMenu *createContextMenu(QWidget *parent);
    QMenu *createSyncMenu(QWidget *parent);

    // actions
    QVector<QAction *> mFileActions;
    QVector<QAction *> mSortActions;
    QVector<QAction *> mEditActions;
    QVector<QAction *> mToolsActions;
    QVector<QAction *> mPanelActions;
    QVector<QAction *> mViewActions;
    QVector<QAction *> mSyncActions;
    QVector<QAction *> mPluginActions;
    QVector<QAction *> mHelpActions;
    QVector<QAction *> mPreviewActions;

    QVector<QAction *> mHiddenActions;

    DkManipulatorManager mManipulators;

    // dynamic menus
    QMenu *mFileMenu = nullptr;
    QMenu *mOpenWithMenu = nullptr;
    QMenu *mSortMenu = nullptr;
    QMenu *mViewMenu = nullptr;
    QMenu *mEditMenu = nullptr;
    QMenu *mManipulatorMenu = nullptr;
    QMenu *mToolsMenu = nullptr;
    QMenu *mPanelMenu = nullptr;
    QMenu *mHelpMenu = nullptr;
    QMenu *mContextMenu = nullptr;

    // sync
    QMenu *mSyncMenu = nullptr;
    DkTcpMenu *mLocalMenu = nullptr;

    // icons
    QVector<QIcon> mFileIcons;
    QVector<QIcon> mEditIcons;
    QVector<QIcon> mViewIcons;
    QVector<QIcon> mToolsIcons;

    DkAppManager *mAppManager = nullptr;
    DkPluginActionManager *mPluginManager = nullptr;

    QSharedPointer<DkActionManager> inst;
};
}
