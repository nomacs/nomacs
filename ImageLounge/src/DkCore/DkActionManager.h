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

#include <QAction>
#include <QObject>

#include "DkManipulators.h"

class QMenu;
class QMainWindow;
class QWinTaskbarProgress;
class QProgressDialog;

namespace nmc
{
class DkTcpMenu;
class DkPluginActionManager;

class DllCoreExport DkAppManager : public QObject
{
    Q_OBJECT

public:
    static constexpr QStringView kOpenDirAppName = u"@opendir";
    static constexpr QStringView kOpenFileAppName = u"@openfile";

    explicit DkAppManager(QWidget *parent = nullptr);
    ~DkAppManager() override;

    void setActions(QVector<QAction *> actions);
    QVector<QAction *> getActions() const;
    QAction *createAction(const QString &filePath);
    QAction *findAction(const QString &appPath) const;

public slots:
    void openTriggered() const;

signals:
    void openFileSignal(QAction *action) const;

protected:
    void saveSettings() const;
    void loadSettings();
    void assignIcon(QAction *app) const;
    bool containsApp(QStringView appId) const;
    QString actionId(const QString &name) const;

    QString searchForSoftware(const QString &organization,
                              const QString &application,
                              const QString &pathKey = "",
                              const QString &exeName = "") const;
    void findDefaultSoftware();

    QVector<QAction *> mApps;
    bool mFirstTime = true;
};

struct DkActionId {
    int index; // index into the associated actions array
    const char *objectName; // uuid for settings, etc
};

// Shorthand to make enum index into a unique identifier string,
// which then becomes the action objectName
#define ACTION_ID(enumIndex)                                                                                           \
    nmc::DkActionId                                                                                                    \
    {                                                                                                                  \
        enumIndex, #enumIndex                                                                                          \
    }

class DllCoreExport DkActionBuilder
{
private:
    QVector<QAction *> &mActions; // indexed by id.index
    QWidget *mParent{};

    QAction *add(DkActionId id, const QIcon &icon, const QString &text, const QString &statusTip);

public:
    DkActionBuilder(QVector<QAction *> &actions, size_t numActions, QWidget *parent);
    ~DkActionBuilder();

    QAction *add(DkActionId id,
                 const QIcon &icon,
                 const QString &text,
                 const QString &statusTip,
                 const QKeySequence &shortcut)
    {
        auto *a = add(id, icon, text, statusTip);
        a->setShortcut(shortcut);
        return a;
    }

    // This overload is needed, QKeySequence(StandardKey) only takes the first shortcut
    // when there are multiple for the standard key.
    QAction *add(DkActionId id,
                 const QIcon &icon,
                 const QString &text,
                 const QString &statusTip,
                 QKeySequence::StandardKey standardKey)
    {
        auto *a = add(id, icon, text, statusTip);
        a->setShortcuts(standardKey);
        return a;
    }

    QAction *add(DkActionId id,
                 const QIcon &icon,
                 const QString &text,
                 const QString &statusTip,
                 const QList<QKeySequence> &shortcuts)
    {
        auto *a = add(id, icon, text, statusTip);
        a->setShortcuts(shortcuts);
        return a;
    }

    QAction *addCheckable(DkActionId id,
                          const QIcon &icon,
                          const QString &text,
                          const QString &statusTip,
                          const QKeySequence &shortcut,
                          bool checked)
    {
        auto *a = add(id, icon, text, statusTip, shortcut);
        a->setCheckable(true);
        a->setChecked(checked);
        return a;
    }

    QAction *addCheckable(DkActionId id,
                          const QIcon &icon,
                          const QString &text,
                          const QString &statusTip,
                          QKeySequence::StandardKey standardKey,
                          bool checked)
    {
        auto *a = add(id, icon, text, statusTip, standardKey);
        a->setCheckable(true);
        a->setChecked(checked);
        return a;
    }
};

class DllCoreExport DkActionManager
{
public:
    static DkActionManager &instance();
    ~DkActionManager();

    // singleton
    DkActionManager(DkActionManager const &) = delete;
    void operator=(DkActionManager const &) = delete;

    // define the action groups for shortcuts customizing; menus need not reflect these exactly
    enum FileAction {
        file_open,
        file_open_dir,
        file_open_list,
        file_quick_launch,
        file_app_manager,
        file_save,
        file_save_as,
        file_save_copy,
        file_save_list,
        file_save_web,
        file_rename,
        nav_goto_file,
        file_find,
        file_recursive,
        file_show_recent,
        file_print,
        file_reload,
        nav_next_file,
        nav_prev_file,
        file_new_instance,
        file_private_instance,
        file_exit,

        numFileActions,
    };

    enum SortAction {
        sort_filename,
        sort_file_size,
        sort_date_created,
        sort_date_modified,
        sort_random,
        sort_ascending,
        sort_descending,

        numSortActions,
    };

    enum EditAction {
        panel_edit_image,

        edit_rotate_cw,
        edit_rotate_ccw,
        edit_rotate_180,
        edit_undo,
        edit_redo,
        edit_copy,
        edit_copy_buffer,
        edit_copy_color,
        edit_paste,
        edit_shortcuts,
        edit_preferences,
        edit_transform,
        edit_delete,
        edit_crop,

        numEditActions,
    };

    enum ToolsAction {
        tools_thumbs,
        tools_filter,
        tools_export_tiff,
        tools_extract_archive,
        tools_mosaic,
        tools_batch,
        tools_wallpaper,
        tools_train_format,

        numToolsActions,
    };

    enum PanelAction {
        panel_toggle_all,

        panel_menubar,
        panel_toolbar,
        panel_statusbar,
        panel_transfertoolbar,

        panel_player,
        panel_preview,
        panel_thumbview,
        panel_scroller,
        panel_exif,
        panel_info,
        panel_histogram,
        panel_overview,
        panel_explorer,
        panel_metadata_dock,
        panel_comment,
        panel_history,
        panel_log,

        numPanelActions,
    };

    enum ViewAction {
        view_fullscreen,
        view_reset,
        view_100,
        view_fit_frame,
        view_zoom_in,
        view_zoom_out,
        view_anti_aliasing,
        view_tp_pattern,
        view_frameless,

        view_new_tab,
        view_close_tab,
        view_close_all_tabs,
        view_first_tab,
        view_previous_tab,
        view_goto_tab,
        view_next_tab,
        view_last_tab,

        view_opacity_up,
        view_opacity_down,
        view_opacity_an,
        view_opacity_change,
        view_lock_window,
        view_gps_map,
        view_slideshow,
        view_movie_pause,
        view_movie_next,
        view_movie_prev,

        view_monitors, // frameless only

        numViewActions,
    };

    enum SyncAction {
        sync_view,
        sync_pos,
        sync_arrange,
        sync_connect_all,
        sync_all_actions,

        numSyncActions,
    };

    enum PluginAction {
        plugin_manager,

        numPluginActions,
    };

    enum HelpAction {
        help_update,
        help_update_translation,
        help_bug,
        help_documentation,
        help_about,

        numHelpActions,
    };

    enum MiscAction {
        test_rec,
        test_pong,

        nav_first_file,
        nav_last_file,
        nav_skip_prev,
        nav_skip_next,
        nav_skip_next_sync,
        nav_skip_prev_sync,
        nav_first_file_sync,
        nav_last_file_sync,

        file_delete_silent,

        star_rating_0,
        star_rating_1,
        star_rating_2,
        star_rating_3,
        star_rating_4,
        star_rating_5,

        view_pan_up,
        view_pan_down,
        view_pan_left,
        view_pan_right,

        numMiscActions,
    };

    enum PreviewAction {
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

        numPreviewActions
    };

    // default nomacs shortcuts
    // keyboard shortcuts
    // general
    static constexpr QKeyCombination shortcut_esc = Qt::Key_Escape;

    // file
    static constexpr QKeyCombination shortcut_open_preview = Qt::Key_T;
    static constexpr QKeyCombination shortcut_open_thumbview = Qt::SHIFT | Qt::Key_T;
    static constexpr QKeyCombination shortcut_open_dir = Qt::CTRL | Qt::SHIFT | Qt::Key_O;
#ifdef Q_OS_WIN
    static constexpr QKeyCombination shortcut_quick_launch = Qt::CTRL | Qt::Key_Q;
#else
    static constexpr QKeyCombination shortcut_quick_launch = Qt::CTRL | Qt::Key_L;
#endif
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
    static constexpr QKeyCombination shortcut_test_rec = Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::Key_R;

    QMenu *updateOpenWithMenu();

    QMenu *fileMenu() const
    {
        return mFileMenu;
    }
    QMenu *sortMenu() const
    {
        return mSortMenu;
    }
    QMenu *openWithMenu() const
    {
        return mOpenWithMenu;
    }
    QMenu *viewMenu() const
    {
        return mViewMenu;
    }
    QMenu *editMenu() const
    {
        return mEditMenu;
    }
    QMenu *manipulatorMenu() const
    {
        return mManipulatorMenu;
    }
    QMenu *toolsMenu() const
    {
        return mToolsMenu;
    }
    QMenu *panelMenu() const
    {
        return mPanelMenu;
    }
    QMenu *helpMenu() const
    {
        return mHelpMenu;
    }
    QMenu *contextMenu() const
    {
        return mContextMenu;
    }
    QMenu *syncMenu() const
    {
        return mSyncMenu;
    }
    DkTcpMenu *localMenu() const
    {
        return mLocalMenu;
    }

    DkManipulatorManager manipulatorManager() const
    {
        return mManipulators;
    }

    void createActions(QWidget *parent);
    void createMenus(QWidget *parent);

    QAction *action(FileAction action) const
    {
        return mFileActions[action];
    }
    QAction *action(SortAction action) const
    {
        return mSortActions[action];
    }
    QAction *action(ViewAction action) const
    {
        return mViewActions[action];
    }
    QAction *action(EditAction action) const
    {
        return mEditActions[action];
    }
    QAction *action(ToolsAction action) const
    {
        return mToolsActions[action];
    }
    QAction *action(PanelAction action) const
    {
        return mPanelActions[action];
    }
    QAction *action(SyncAction action) const
    {
        return mSyncActions[action];
    }
    QAction *action(PluginAction action) const
    {
        return mPluginActions[action];
    }
    QAction *action(HelpAction action) const
    {
        return mHelpActions[action];
    }
    QAction *action(MiscAction action) const
    {
        return mMiscActions[action];
    }
    QAction *action(PreviewAction action) const
    {
        return mPreviewActions[action];
    }

    QVector<QAction *> fileActions() const
    {
        return mFileActions;
    }
    QVector<QAction *> sortActions() const
    {
        return mSortActions;
    }

    QVector<QAction *> openWithActions() const;

    QVector<QAction *> viewActions() const
    {
        return mViewActions;
    }
    QVector<QAction *> editActions() const
    {
        return mEditActions;
    }
    QVector<QAction *> toolsActions() const
    {
        return mToolsActions;
    }
    QVector<QAction *> panelActions() const
    {
        return mPanelActions;
    }
    QVector<QAction *> syncActions() const
    {
        return mSyncActions;
    }
    QVector<QAction *> pluginActions() const
    {
        return mPluginActions;
    }
    QVector<QAction *> helpActions() const
    {
        return mHelpActions;
    }
    QVector<QAction *> previewActions() const
    {
        return mPreviewActions;
    }
    QVector<QAction *> manipulatorActions() const
    {
        return mManipulators.actions();
    }

    QVector<QAction *> allActions() const;

    QVector<QAction *> miscActions() const
    {
        return mMiscActions;
    }
    DkAppManager *appManager() const
    {
        return mAppManager;
    }
    DkPluginActionManager *pluginActionManager() const
    {
        return mPluginManager;
    }

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

    // actions indexed by their associated enum
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
    QVector<QAction *> mMiscActions;

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

    DkAppManager *mAppManager = nullptr;
    DkPluginActionManager *mPluginManager = nullptr;

    QSharedPointer<DkActionManager> inst;
};
}
