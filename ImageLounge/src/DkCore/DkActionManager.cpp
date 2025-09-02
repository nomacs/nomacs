/*******************************************************************************************************
 DkActionManager.cpp
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

#include "DkActionManager.h"

#include "DkDialog.h"
#include "DkImageStorage.h"
#include "DkMenu.h"
#include "DkSettings.h"
#include "DkStatusBar.h"
#include "DkToolbars.h"
#include "DkUtils.h"

#ifdef WITH_PLUGINS
#include "DkPluginManager.h"
#endif

#if defined(Q_OS_WIN) && !defined(SOCK_STREAM)
#include <winsock2.h> // needed since libraw 0.16
#endif

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QProgressDialog>
#include <QSvgRenderer>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace nmc
{

// DkAppManager --------------------------------------------------------------------
DkAppManager::DkAppManager(QWidget *parent)
    : QObject(parent)
{
    mDefaultNames.resize(app_idx_end);
    mDefaultNames[app_photohsop] = "PhotoshopAction";
    mDefaultNames[app_picasa] = "PicasaAction";
    mDefaultNames[app_picasa_viewer] = "PicasaViewerAction";
    mDefaultNames[app_irfan_view] = "IrfanViewAction";
    mDefaultNames[app_explorer] = "ExplorerAction";

    loadSettings();
    if (mFirstTime)
        findDefaultSoftware();

    for (int idx = 0; idx < mApps.size(); idx++) {
        assignIcon(mApps.at(idx));
        connect(mApps.at(idx), &QAction::triggered, this, &DkAppManager::openTriggered);
    }
}

DkAppManager::~DkAppManager()
{
    // save settings
    saveSettings();
}

void DkAppManager::saveSettings() const
{
    DefaultSettings settings;
    settings.beginGroup("DkAppManager");
    // clear it first
    settings.remove("Apps");

    settings.beginWriteArray("Apps");

    for (int idx = 0; idx < mApps.size(); idx++) {
        settings.setArrayIndex(idx);
        settings.setValue("appName", mApps.at(idx)->text());
        settings.setValue("appPath", mApps.at(idx)->toolTip());
        settings.setValue("objectName", mApps.at(idx)->objectName());
    }
    settings.endArray();
    settings.endGroup();
}

void DkAppManager::loadSettings()
{
    DefaultSettings settings;
    settings.beginGroup("DkAppManager");

    int size = settings.beginReadArray("Apps");
    if (size > 0)
        mFirstTime = false;

    for (int idx = 0; idx < size; idx++) {
        settings.setArrayIndex(idx);
        auto *action = new QAction(parent());
        action->setText(settings.value("appName", "").toString());
        action->setToolTip(settings.value("appPath", "").toString());
        action->setObjectName(settings.value("objectName", "").toString());

        // default shortcuts
        if (action->objectName() == mDefaultNames[app_explorer])
            action->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_E));

        if (QFileInfo(action->toolTip()).exists() && !action->text().isEmpty())
            mApps.append(action);
        else
            qDebug() << "could not locate: " << action->toolTip();
    }
    settings.endArray();
    settings.endGroup();
}

QVector<QAction *> DkAppManager::getActions() const
{
    return mApps;
}

void DkAppManager::setActions(QVector<QAction *> actions)
{
    mApps = actions;
    saveSettings();
}

QAction *DkAppManager::createAction(const QString &filePath)
{
    QFileInfo file(filePath);
    if (!file.exists())
        return nullptr;

    auto *newApp = new QAction(file.baseName(), parent());
    newApp->setToolTip(QDir::fromNativeSeparators(file.filePath()));
    assignIcon(newApp);
    connect(newApp, &QAction::triggered, this, &DkAppManager::openTriggered);

    return newApp;
}

QAction *DkAppManager::findAction(const QString &appPath) const
{
    for (int idx = 0; idx < mApps.size(); idx++) {
        if (mApps.at(idx)->toolTip() == appPath)
            return mApps.at(idx);
    }

    return nullptr;
}

void DkAppManager::findDefaultSoftware()
{
    QString appPath;

    // Photoshop
    if (!containsApp(mApps, mDefaultNames[app_photohsop])) {
        appPath = searchForSoftware("Adobe", "Photoshop", "ApplicationPath");
        if (!appPath.isEmpty()) {
            auto *a = new QAction(QObject::tr("&Photoshop"), parent());
            a->setToolTip(QDir::fromNativeSeparators(appPath));
            a->setObjectName(mDefaultNames[app_photohsop]);
            mApps.append(a);
        }
    }

    if (!containsApp(mApps, mDefaultNames[app_picasa])) {
        // Picasa
        appPath = searchForSoftware("Google", "Picasa", "Directory");
        if (!appPath.isEmpty()) {
            auto *a = new QAction(QObject::tr("Pic&asa"), parent());
            a->setToolTip(QDir::fromNativeSeparators(appPath));
            a->setObjectName(mDefaultNames[app_picasa]);
            mApps.append(a);
        }
    }

    if (!containsApp(mApps, mDefaultNames[app_picasa_viewer])) {
        // Picasa Photo Viewer
        appPath = searchForSoftware("Google", "Picasa", "Directory", "PicasaPhotoViewer.exe");
        if (!appPath.isEmpty()) {
            auto *a = new QAction(QObject::tr("Picasa Ph&oto Viewer"), parent());
            a->setToolTip(QDir::fromNativeSeparators(appPath));
            a->setObjectName(mDefaultNames[app_picasa_viewer]);
            mApps.append(a);
        }
    }

    if (!containsApp(mApps, mDefaultNames[app_irfan_view])) {
        // IrfanView
        appPath = searchForSoftware("IrfanView", "shell");
        if (!appPath.isEmpty()) {
            auto *a = new QAction(QObject::tr("&IrfanView"), parent());
            a->setToolTip(QDir::fromNativeSeparators(appPath));
            a->setObjectName(mDefaultNames[app_irfan_view]);
            mApps.append(a);
        }
    }

    if (!containsApp(mApps, mDefaultNames[app_explorer])) {
        appPath = "C:/Windows/explorer.exe";
        if (QFileInfo(appPath).exists()) {
            auto *a = new QAction(QObject::tr("&Explorer"), parent());
            a->setToolTip(QDir::fromNativeSeparators(appPath));
            a->setObjectName(mDefaultNames[app_explorer]);
            mApps.append(a);
        }
    }
}

bool DkAppManager::containsApp(QVector<QAction *> apps, const QString &appName) const
{
    for (int idx = 0; idx < apps.size(); idx++)
        if (apps.at(idx)->objectName() == appName)
            return true;

    return false;
}

void DkAppManager::assignIcon(QAction *app) const
{
#ifdef Q_OS_WIN

    if (!app) {
        qDebug() << "SERIOUS problem here, I should assign an icon to a NULL pointer action";
        return;
    }

    QFileInfo file(app->toolTip());

    if (!file.exists())
        return;

    // icon extraction should take between 2ms and 13ms
    QPixmap appIcon;
    QString winPath = QDir::toNativeSeparators(file.absoluteFilePath());

    WCHAR *wDirName = new WCHAR[winPath.length() + 1];

    // CMakeLists.txt:
    // if compile error that toWCharArray is not recognized:
    // in msvc: Project Properties -> C/C++ -> Language -> Treat WChar_t as built-in type: set to No (/Zc:wchar_t-)
    wDirName = (WCHAR *)winPath.utf16();
    wDirName[winPath.length()] = L'\0'; // append null character

    int nIcons = ExtractIconExW(wDirName, 0, NULL, NULL, 0);

    if (!nIcons)
        return;

    HICON largeIcon;
    HICON smallIcon;
    ExtractIconExW(wDirName, 0, &largeIcon, &smallIcon, 1);

    if (nIcons != 0 && smallIcon != NULL) {
        appIcon = QPixmap::fromImage(QImage::fromHICON(smallIcon));
    }

    DestroyIcon(largeIcon);
    DestroyIcon(smallIcon);

    app->setIcon(appIcon);
#else
    Q_UNUSED(app)
#endif
}

QString DkAppManager::searchForSoftware(const QString &organization,
                                        const QString &application,
                                        const QString &pathKey,
                                        const QString &exeName) const
{
    // qDebug() << "searching for: " << organization;

    // locate the settings entry
    QSettings softwareSettings(QSettings::UserScope, organization, application);
    QStringList keys = softwareSettings.allKeys();

    QString appPath;

    for (int idx = 0; idx < keys.length(); idx++) {
        // find the path
        if (keys[idx].contains(pathKey)) {
            appPath = softwareSettings.value(keys[idx]).toString();
            break;
        }
    }

    // if we did not find it -> return
    if (appPath.isEmpty())
        return appPath;

    if (exeName.isEmpty()) {
        // locate the exe
        QDir appFile = appPath.replace("\"", ""); // the string must not have extra quotes
        QFileInfoList apps = appFile.entryInfoList(QStringList() << "*.exe");

        for (int idx = 0; idx < apps.size(); idx++) {
            if (apps[idx].fileName().contains(application)) {
                appPath = apps[idx].absoluteFilePath();
                break;
            }
        }
    } else
        appPath = QFileInfo(appPath, exeName).absoluteFilePath(); // for correct separators

    if (!appPath.isEmpty())
        qInfo() << "I found" << organization << "in:" << appPath;

    return appPath;
}

void DkAppManager::openTriggered() const
{
    auto *a = static_cast<QAction *>(QObject::sender());

    if (a)
        openFileSignal(a);
}

// DkActionManager --------------------------------------------------------------------
DkActionManager::DkActionManager()
{
    init();
}

DkActionManager::~DkActionManager()
{
    qDebug() << "releasing...";
}

DkActionManager &DkActionManager::instance()
{
    static DkActionManager inst;
    return inst;
}

QMenu *DkActionManager::createFileMenu(QWidget *parent /* = 0 */)
{
    mFileMenu = new QMenu(QObject::tr("&File"), parent);

    mFileMenu->addAction(mFileActions[menu_file_open]);
    mFileMenu->addAction(mFileActions[menu_file_open_dir]);
    if (DkSettingsManager::param().global().extendedTabs)
        mFileMenu->addAction(mFileActions[menu_file_open_list]);

    // add open with menu
    mFileMenu->addMenu(openWithMenu());
    mFileMenu->addAction(mFileActions[menu_file_quick_launch]);

    mFileMenu->addSeparator();
    mFileMenu->addAction(mFileActions[menu_file_save]);
    mFileMenu->addAction(mFileActions[menu_file_save_as]);
    mFileMenu->addAction(mFileActions[menu_file_save_copy]);
    if (DkSettingsManager::param().global().extendedTabs)
        mFileMenu->addAction(mFileActions[menu_file_save_list]);
    mFileMenu->addAction(mFileActions[menu_file_save_web]);
    mFileMenu->addAction(mFileActions[menu_file_rename]);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mFileActions[menu_file_show_recent]);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mFileActions[menu_file_print]);
    mFileMenu->addSeparator();

    // add sort menu
    mFileMenu->addMenu(sortMenu());
    mFileMenu->addAction(mFileActions[menu_file_recursive]);
    mFileMenu->addAction(mFileActions[menu_file_goto]);
    mFileMenu->addAction(mFileActions[menu_file_find]);
    mFileMenu->addAction(mFileActions[menu_file_reload]);
    mFileMenu->addAction(mFileActions[menu_file_prev]);
    mFileMenu->addAction(mFileActions[menu_file_next]);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mFileActions[menu_file_new_instance]);
    mFileMenu->addAction(mFileActions[menu_file_private_instance]);
    mFileMenu->addAction(mFileActions[menu_file_exit]);

    return mFileMenu;
}

QMenu *DkActionManager::createOpenWithMenu(QWidget *parent)
{
    mOpenWithMenu = new QMenu(QObject::tr("&Open With"), parent);
    return updateOpenWithMenu();
}

QMenu *DkActionManager::updateOpenWithMenu()
{
    mOpenWithMenu->clear();

    QVector<QAction *> appActions = mAppManager->getActions();

    assignCustomShortcuts(appActions);
    mOpenWithMenu->addActions(appActions.toList());

    if (!appActions.empty())
        mOpenWithMenu->addSeparator();
    mOpenWithMenu->addAction(mFileActions[menu_file_app_manager]);

    return mOpenWithMenu;
}

QMenu *DkActionManager::createSortMenu(QWidget *parent)
{
    mSortMenu = new QMenu(QObject::tr("S&ort"), parent);
    mSortMenu->addAction(mSortActions[menu_sort_filename]);
    mSortMenu->addAction(mSortActions[menu_sort_file_size]);
    mSortMenu->addAction(mSortActions[menu_sort_date_created]);
    mSortMenu->addAction(mSortActions[menu_sort_date_modified]);
    mSortMenu->addAction(mSortActions[menu_sort_random]);
    mSortMenu->addSeparator();
    mSortMenu->addAction(mSortActions[menu_sort_ascending]);
    mSortMenu->addAction(mSortActions[menu_sort_descending]);

    return mSortMenu;
}

QMenu *DkActionManager::createViewMenu(QWidget *parent /* = 0 */)
{
    mViewMenu = new QMenu(QObject::tr("&View"), parent);

    mViewMenu->addAction(mViewActions[menu_view_frameless]);
    mViewMenu->addAction(mViewActions[menu_view_fullscreen]);
    mViewMenu->addSeparator();

    mViewMenu->addAction(mViewActions[menu_view_new_tab]);
    mViewMenu->addAction(mViewActions[menu_view_close_tab]);
    mViewMenu->addAction(mViewActions[menu_view_close_all_tabs]);

    if (DkSettingsManager::param().global().extendedTabs)
        mViewMenu->addAction(mViewActions[menu_view_first_tab]);
    mViewMenu->addAction(mViewActions[menu_view_previous_tab]);
    if (DkSettingsManager::param().global().extendedTabs)
        mViewMenu->addAction(mViewActions[menu_view_goto_tab]);
    mViewMenu->addAction(mViewActions[menu_view_next_tab]);
    if (DkSettingsManager::param().global().extendedTabs)
        mViewMenu->addAction(mViewActions[menu_view_last_tab]);
    mViewMenu->addSeparator();

    mViewMenu->addAction(mViewActions[menu_view_reset]);
    mViewMenu->addAction(mViewActions[menu_view_100]);
    mViewMenu->addAction(mViewActions[menu_view_fit_frame]);
    mViewMenu->addAction(mViewActions[menu_view_zoom_in]);
    mViewMenu->addAction(mViewActions[menu_view_zoom_out]);
    mViewMenu->addSeparator();

    mViewMenu->addAction(mViewActions[menu_view_tp_pattern]);
    mViewMenu->addAction(mViewActions[menu_view_anti_aliasing]);
    mViewMenu->addSeparator();

    mViewMenu->addAction(mViewActions[menu_view_opacity_change]);
    mViewMenu->addAction(mViewActions[menu_view_opacity_up]);
    mViewMenu->addAction(mViewActions[menu_view_opacity_down]);
    mViewMenu->addAction(mViewActions[menu_view_opacity_an]);
#ifdef Q_OS_WIN
    mViewMenu->addAction(mViewActions[menu_view_lock_window]);
#endif
    mViewMenu->addSeparator();

    mViewMenu->addAction(mViewActions[menu_view_slideshow]);
    mViewMenu->addAction(mViewActions[menu_view_movie_pause]);
    mViewMenu->addAction(mViewActions[menu_view_movie_prev]);
    mViewMenu->addAction(mViewActions[menu_view_movie_next]);
    mViewMenu->addSeparator();

    if (DkSettingsManager::param().app().currentAppMode == DkSettingsManager::param().mode_frameless) {
        mViewMenu->addAction(mViewActions[menu_view_monitors]);
        mViewMenu->addSeparator();
    }

    mViewMenu->addAction(mViewActions[menu_view_gps_map]);

    return mViewMenu;
}

QMenu *DkActionManager::createEditMenu(QWidget *parent /* = 0 */)
{
    mEditMenu = new QMenu(QObject::tr("&Edit"), parent);

    mEditMenu->addAction(mEditActions[menu_edit_copy]);
    mEditMenu->addAction(mEditActions[menu_edit_copy_buffer]);
    mEditMenu->addAction(mEditActions[menu_edit_paste]);
    mEditMenu->addAction(mEditActions[menu_edit_delete]);
    mEditMenu->addSeparator();
    mEditMenu->addAction(mEditActions[menu_edit_rotate_ccw]);
    mEditMenu->addAction(mEditActions[menu_edit_rotate_cw]);
    mEditMenu->addAction(mEditActions[menu_edit_rotate_180]);
    mEditMenu->addSeparator();
    mEditMenu->addAction(mEditActions[menu_edit_undo]);
    mEditMenu->addAction(mEditActions[menu_edit_redo]);
    mEditMenu->addSeparator();
    mEditMenu->addAction(mEditActions[menu_edit_shortcuts]);
    mEditMenu->addAction(mEditActions[menu_edit_preferences]);

    return mEditMenu;
}

QMenu *DkActionManager::createManipulatorMenu(QWidget *parent)
{
    mManipulatorMenu = new QMenu(QObject::tr("&Adjustments"), parent);

    for (auto action : mManipulators.actions())
        mManipulatorMenu->addAction(action);

    mManipulatorMenu->addSeparator();
    mManipulatorMenu->addAction(mEditActions[menu_edit_transform]);
    mManipulatorMenu->addAction(mEditActions[menu_edit_crop]);
    mManipulatorMenu->addSeparator();
    mManipulatorMenu->addAction(mEditActions[menu_edit_image]);

    return mManipulatorMenu;
}

QMenu *DkActionManager::createPanelMenu(QWidget *parent)
{
    mPanelMenu = new QMenu(QObject::tr("&Panels"), parent);

    QMenu *toolsMenu = mPanelMenu->addMenu(QObject::tr("Tool&bars"));
    toolsMenu->addAction(mPanelActions[menu_panel_menu]);
    toolsMenu->addAction(mPanelActions[menu_panel_toolbar]);
    toolsMenu->addAction(mPanelActions[menu_panel_statusbar]);
    toolsMenu->addAction(mPanelActions[menu_panel_transfertoolbar]);

    mPanelMenu->addAction(mPanelActions[menu_panel_explorer]);
    mPanelMenu->addAction(mPanelActions[menu_panel_metadata_dock]);
    mPanelMenu->addAction(mPanelActions[menu_panel_exif]);
    mPanelMenu->addAction(mPanelActions[menu_panel_history]);
    mPanelMenu->addAction(mPanelActions[menu_panel_preview]);
    mPanelMenu->addAction(mPanelActions[menu_panel_thumbview]);
    mPanelMenu->addAction(mPanelActions[menu_panel_scroller]);
    mPanelMenu->addAction(mPanelActions[menu_panel_log]);

    mPanelMenu->addSeparator();

    mPanelMenu->addAction(mPanelActions[menu_panel_overview]);
    mPanelMenu->addAction(mPanelActions[menu_panel_player]);
    mPanelMenu->addAction(mPanelActions[menu_panel_info]);
    mPanelMenu->addAction(mPanelActions[menu_panel_histogram]);
    mPanelMenu->addAction(mPanelActions[menu_panel_comment]);

    mPanelMenu->addSeparator();
    mPanelMenu->addAction(mPanelActions[menu_panel_toggle]);

    return mPanelMenu;
}

QMenu *DkActionManager::createToolsMenu(QWidget *parent /* = 0 */)
{
    mToolsMenu = new QMenu(QObject::tr("&Tools"), parent);

#ifdef WITH_LIBTIFF
    mToolsMenu->addAction(mToolsActions[menu_tools_export_tiff]);
#endif
#ifdef WITH_QUAZIP
    mToolsMenu->addAction(mToolsActions[menu_tools_extract_archive]);
#endif
#ifdef WITH_OPENCV
    mToolsMenu->addAction(mToolsActions[menu_tools_mosaic]);
#endif
#ifdef Q_OS_WIN
    mToolsMenu->addAction(mToolsActions[menu_tools_wallpaper]);
    mToolsMenu->addSeparator();
#endif
    mToolsMenu->addAction(mToolsActions[menu_tools_batch]);
    mToolsMenu->addAction(mToolsActions[menu_tools_thumbs]);
    mToolsMenu->addAction(mToolsActions[menu_tools_train_format]);

    return mToolsMenu;
}

QMenu *DkActionManager::createHelpMenu(QWidget *parent)
{
    mHelpMenu = new QMenu(QObject::tr("&?"), parent);

#ifndef Q_OS_LINUX
    mHelpMenu->addAction(mHelpActions[menu_help_update]);
#endif // !Q_OS_LINUX
    mHelpMenu->addAction(mHelpActions[menu_help_update_translation]);
    mHelpMenu->addSeparator();
    mHelpMenu->addAction(mHelpActions[menu_help_bug]);
    mHelpMenu->addAction(mHelpActions[menu_help_documentation]);
    mHelpMenu->addAction(mHelpActions[menu_help_about]);

    return mHelpMenu;
}

QMenu *DkActionManager::createSyncMenu(QWidget *parent)
{
    mSyncMenu = new QMenu(QObject::tr("&Sync"), parent);

    // local host menu
    mLocalMenu = new DkTcpMenu(QObject::tr("&Synchronize"), mSyncMenu);
    mLocalMenu->showNoClientsFound(true);

    // add connect all action
    mLocalMenu->addTcpAction(mSyncActions[menu_sync_connect_all]);

    mSyncMenu->addMenu(mLocalMenu);
    mSyncMenu->addAction(mSyncActions[menu_sync_view]);
    mSyncMenu->addAction(mSyncActions[menu_sync_pos]);
    mSyncMenu->addAction(mSyncActions[menu_sync_arrange]);
    mSyncMenu->addAction(mSyncActions[menu_sync_all_actions]);

    return mSyncMenu;
}

QMenu *DkActionManager::createContextMenu(QWidget *parent)
{
    mContextMenu = new QMenu(parent);

    mContextMenu->addAction(mEditActions[menu_edit_copy_buffer]);
    mContextMenu->addAction(mEditActions[menu_edit_copy]);
    mContextMenu->addAction(mEditActions[menu_edit_copy_color]);
    mContextMenu->addAction(mEditActions[menu_edit_paste]);
    mContextMenu->addSeparator();

    mContextMenu->addAction(mViewActions[menu_view_frameless]);
    mContextMenu->addAction(mViewActions[menu_view_fullscreen]);
    mContextMenu->addAction(mPanelActions[menu_panel_menu]);
    mContextMenu->addSeparator();

    if (DkSettingsManager::param().global().extendedTabs) {
        mContextMenu->addAction(mViewActions[menu_view_first_tab]);
        mContextMenu->addAction(mViewActions[menu_view_goto_tab]);
        mContextMenu->addAction(mViewActions[menu_view_last_tab]);
        mContextMenu->addSeparator();
    }

    QMenu *panelMenu = mContextMenu->addMenu(QObject::tr("&Panels"));
    panelMenu->addAction(mPanelActions[menu_panel_toggle]);
    panelMenu->addAction(mPanelActions[menu_panel_explorer]);
    panelMenu->addAction(mPanelActions[menu_panel_metadata_dock]);
    panelMenu->addAction(mPanelActions[menu_panel_preview]);
    panelMenu->addAction(mPanelActions[menu_panel_thumbview]);
    panelMenu->addAction(mPanelActions[menu_panel_scroller]);
    panelMenu->addAction(mPanelActions[menu_panel_exif]);
    panelMenu->addAction(mPanelActions[menu_panel_overview]);
    panelMenu->addAction(mPanelActions[menu_panel_player]);
    panelMenu->addAction(mPanelActions[menu_panel_info]);
    panelMenu->addAction(mPanelActions[menu_panel_histogram]);
    panelMenu->addAction(mPanelActions[menu_panel_comment]);

    mContextMenu->addMenu(sortMenu());

    QMenu *viewContextMenu = mContextMenu->addMenu(QObject::tr("&View"));
    viewContextMenu->addAction(mViewActions[menu_view_fullscreen]);
    viewContextMenu->addAction(mViewActions[menu_view_reset]);
    viewContextMenu->addAction(mViewActions[menu_view_100]);
    viewContextMenu->addAction(mViewActions[menu_view_fit_frame]);

    QMenu *editContextMenu = mContextMenu->addMenu(QObject::tr("&Edit"));
    editContextMenu->addAction(mEditActions[menu_edit_image]);
    editContextMenu->addSeparator();
    editContextMenu->addAction(mEditActions[menu_edit_undo]);
    editContextMenu->addAction(mEditActions[menu_edit_redo]);
    editContextMenu->addSeparator();
    editContextMenu->addAction(mEditActions[menu_edit_rotate_cw]);
    editContextMenu->addAction(mEditActions[menu_edit_rotate_ccw]);
    editContextMenu->addAction(mEditActions[menu_edit_rotate_180]);
    editContextMenu->addSeparator();
    editContextMenu->addAction(mEditActions[menu_edit_transform]);
    editContextMenu->addAction(mEditActions[menu_edit_crop]);
    editContextMenu->addAction(mEditActions[menu_edit_delete]);

    mContextMenu->addMenu(mOpenWithMenu);

    mContextMenu->addSeparator();
    mContextMenu->addAction(mEditActions[menu_edit_preferences]);

    return mContextMenu;
}

// TODO: fix that once and for all
// QMenu* DkActionManager::pluginMenu(QWidget* parent) const {
//
//	if (!mPluginMenu)
//		mPluginMenu = new QMenu(QObject::tr("&Plugins"), parent);
//
//	return mPluginMenu;
//}

QAction *DkActionManager::action(FileMenuActions action) const
{
    return mFileActions[action];
}

QAction *DkActionManager::action(SortMenuActions action) const
{
    return mSortActions[action];
}

QAction *DkActionManager::action(ViewMenuActions action) const
{
    return mViewActions[action];
}

QAction *DkActionManager::action(EditMenuActions action) const
{
    return mEditActions[action];
}

QAction *DkActionManager::action(ToolsMenuActions action) const
{
    return mToolsActions[action];
}

QAction *DkActionManager::action(PanelMenuActions action) const
{
    return mPanelActions[action];
}

QAction *DkActionManager::action(SyncMenuActions action) const
{
    return mSyncActions[action];
}

QAction *DkActionManager::action(HelpMenuActions action) const
{
    return mHelpActions[action];
}

QAction *DkActionManager::action(PluginMenuActions action) const
{
    return mPluginActions[action];
}

QAction *DkActionManager::action(HiddenActions action) const
{
    return mHiddenActions[action];
}

QAction *DkActionManager::action(PreviewActions action) const
{
    return mPreviewActions[action];
}

QIcon DkActionManager::icon(FileIcons icon) const
{
    return mFileIcons[icon];
}

QIcon DkActionManager::icon(ViewIcons icon) const
{
    return mViewIcons[icon];
}

QIcon DkActionManager::icon(EditIcons icon) const
{
    return mEditIcons[icon];
}

QVector<QAction *> DkActionManager::fileActions() const
{
    return mFileActions;
}

QVector<QAction *> DkActionManager::sortActions() const
{
    return mSortActions;
}

QVector<QAction *> DkActionManager::openWithActions() const
{
    if (!openWithMenu())
        return QVector<QAction *>();

    QList<QAction *> openWithActionList = openWithMenu()->actions();
    QVector<QAction *> owas;

    // the separator creates a NULL action?! - remove it here...
    for (auto a : openWithActionList)
        if (!a->text().isNull())
            owas << a;

    return owas;
}

QVector<QAction *> DkActionManager::viewActions() const
{
    return mViewActions;
}

QVector<QAction *> DkActionManager::editActions() const
{
    return mEditActions;
}

QVector<QAction *> DkActionManager::toolsActions() const
{
    return mToolsActions;
}

QVector<QAction *> DkActionManager::panelActions() const
{
    return mPanelActions;
}

QVector<QAction *> DkActionManager::syncActions() const
{
    return mSyncActions;
}

QVector<QAction *> DkActionManager::helpActions() const
{
    return mHelpActions;
}

QVector<QAction *> DkActionManager::pluginActions() const
{
    return mPluginActions;
}

QVector<QAction *> DkActionManager::hiddenActions() const
{
    return mHiddenActions;
}

QVector<QAction *> DkActionManager::previewActions() const
{
    return mPreviewActions;
}

QVector<QAction *> DkActionManager::manipulatorActions() const
{
    return mManipulators.actions();
}

DkAppManager *DkActionManager::appManager() const
{
    return mAppManager;
}

DkPluginActionManager *DkActionManager::pluginActionManager() const
{
    return mPluginManager;
}

QMenu *DkActionManager::fileMenu() const
{
    return mFileMenu;
}

QMenu *DkActionManager::sortMenu() const
{
    return mSortMenu;
}

QMenu *DkActionManager::openWithMenu() const
{
    return mOpenWithMenu;
}

QMenu *DkActionManager::viewMenu() const
{
    return mViewMenu;
}

QMenu *DkActionManager::editMenu() const
{
    return mEditMenu;
}

QMenu *DkActionManager::toolsMenu() const
{
    return mToolsMenu;
}

QMenu *DkActionManager::panelMenu() const
{
    return mPanelMenu;
}

QMenu *DkActionManager::helpMenu() const
{
    return mHelpMenu;
}

QMenu *DkActionManager::contextMenu() const
{
    return mContextMenu;
}

QMenu *DkActionManager::syncMenu() const
{
    return mSyncMenu;
}

QMenu *DkActionManager::manipulatorMenu() const
{
    return mManipulatorMenu;
}

DkTcpMenu *DkActionManager::localMenu() const
{
    return mLocalMenu;
}

DkManipulatorManager DkActionManager::manipulatorManager() const
{
    return mManipulators;
}

void DkActionManager::createMenus(QWidget *parent)
{
    createSortMenu(parent);
    createOpenWithMenu(parent);
    createFileMenu(parent);
    createViewMenu(parent);
    createEditMenu(parent);
    createManipulatorMenu(parent);
    createToolsMenu(parent);
    createPanelMenu(parent);
    createSyncMenu(parent);
    createHelpMenu(parent);
    createContextMenu(parent);
}

void DkActionManager::init()
{
    mAppManager = new DkAppManager(DkUtils::getMainWindow());

#ifdef WITH_PLUGINS
    mPluginManager = new DkPluginActionManager(DkUtils::getMainWindow());
#endif

    createIcons();
}

void DkActionManager::createIcons()
{
    mFileIcons.resize(icon_file_end);
    mFileIcons[icon_file_dir] = DkImage::loadIcon(":/nomacs/img/dir.svg");
    mFileIcons[icon_file_open] = DkImage::loadIcon(":/nomacs/img/open.svg");
    mFileIcons[icon_file_save] = DkImage::loadIcon(":/nomacs/img/save.svg");
    mFileIcons[icon_file_print] = DkImage::loadIcon(":/nomacs/img/print.svg");
    mFileIcons[icon_file_open_large] = QIcon(":/nomacs/img/open.svg");
    mFileIcons[icon_file_dir_large] = QIcon(":/nomacs/img/dir.svg");
    mFileIcons[icon_file_prev] = DkImage::loadIcon(":/nomacs/img/previous.svg");
    mFileIcons[icon_file_next] = DkImage::loadIcon(":/nomacs/img/next.svg");
    mFileIcons[icon_file_filter] = DkImage::loadIcon(":/nomacs/img/filter-disabled.svg");
    mFileIcons[icon_file_filter].addFile(":/nomacs/img/filter.svg", QSize(), QIcon::Normal, QIcon::On);
    mFileIcons[icon_file_find] = DkImage::loadIcon(":/nomacs/img/find.svg");

    mEditIcons.resize(icon_edit_end);
    mEditIcons[icon_edit_image] = DkImage::loadIcon(":/nomacs/img/sliders.svg");
    mEditIcons[icon_edit_rotate_cw] = DkImage::loadIcon(":/nomacs/img/rotate-cw.svg");
    mEditIcons[icon_edit_rotate_ccw] = DkImage::loadIcon(":/nomacs/img/rotate-cc.svg");
    mEditIcons[icon_edit_crop] = DkImage::loadIcon(":/nomacs/img/crop.svg");
    mEditIcons[icon_edit_resize] = DkImage::loadIcon(":/nomacs/img/resize.svg");
    mEditIcons[icon_edit_copy] = DkImage::loadIcon(":/nomacs/img/copy.svg");
    mEditIcons[icon_edit_paste] = DkImage::loadIcon(":/nomacs/img/paste.svg");
    mEditIcons[icon_edit_delete] = DkImage::loadIcon(":/nomacs/img/trash.svg");
    mEditIcons[icon_edit_undo] = DkImage::loadIcon(":/nomacs/img/edit-undo.svg");
    mEditIcons[icon_edit_redo] = DkImage::loadIcon(":/nomacs/img/edit-redo.svg");

    mViewIcons.resize(icon_view_end);
    mViewIcons[icon_view_fullscreen] = DkImage::loadIcon(":/nomacs/img/fullscreen.svg");
    mViewIcons[icon_view_reset] = DkImage::loadIcon(":/nomacs/img/zoom-reset.svg");
    mViewIcons[icon_view_100] = DkImage::loadIcon(":/nomacs/img/zoom-100.svg");
    mViewIcons[icon_view_gps] = DkImage::loadIcon(":/nomacs/img/location.svg");
    mViewIcons[icon_view_zoom_in] = DkImage::loadIcon(":/nomacs/img/zoom-in.svg");
    mViewIcons[icon_view_zoom_out] = DkImage::loadIcon(":/nomacs/img/zoom-out.svg");

    mViewIcons[icon_view_movie_play] = DkImage::loadIcon(":/nomacs/img/pause.svg");
    mViewIcons[icon_view_movie_play].addFile(":/nomacs/img/play.svg", QSize(), QIcon::Normal, QIcon::On);
    mViewIcons[icon_view_movie_prev] = DkImage::loadIcon(":/nomacs/img/previous.svg");
    mViewIcons[icon_view_movie_next] = DkImage::loadIcon(":/nomacs/img/next.svg");
}

void DkActionManager::createActions(QWidget *parent)
{
    // file actions
    mFileActions.resize(menu_file_end);

    mFileActions[menu_file_open] = new QAction(mFileIcons[icon_file_open], QObject::tr("&Open"), parent);
    mFileActions[menu_file_open]->setShortcuts(QKeySequence::Open);
    mFileActions[menu_file_open]->setStatusTip(QObject::tr("Open an image"));

    mFileActions[menu_file_open_dir] = new QAction(mFileIcons[icon_file_dir], QObject::tr("Open &Directory"), parent);
    mFileActions[menu_file_open_dir]->setShortcut(QKeySequence(shortcut_open_dir));
    mFileActions[menu_file_open_dir]->setStatusTip(QObject::tr("Open a directory"));

    mFileActions[menu_file_open_list] = new QAction(QObject::tr("&Open Tabs"), parent);
    mFileActions[menu_file_open_list]->setStatusTip(
        QObject::tr("Open a text file containing a list of filepaths, and open tabs for them"));

    mFileActions[menu_file_quick_launch] = new QAction(QObject::tr("&Quick Launch"), parent);
#ifdef Q_OS_WIN
    mFileActions[menu_file_quick_launch]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
#else
    mFileActions[menu_file_quick_launch]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
#endif
    mFileActions[menu_file_app_manager] = new QAction(QObject::tr("&Manage Applications"), parent);
    mFileActions[menu_file_app_manager]->setStatusTip(
        QObject::tr("Manage Applications which are Automatically Opened"));
    mFileActions[menu_file_app_manager]->setShortcut(QKeySequence(shortcut_app_manager));

    mFileActions[menu_file_rename] = new QAction(QObject::tr("Re&name"), parent);
    mFileActions[menu_file_rename]->setShortcut(QKeySequence(shortcut_rename));
    mFileActions[menu_file_rename]->setStatusTip(QObject::tr("Rename an image"));

    mFileActions[menu_file_goto] = new QAction(QObject::tr("&Go To"), parent);
    mFileActions[menu_file_goto]->setShortcut(QKeySequence(shortcut_goto));
    mFileActions[menu_file_goto]->setStatusTip(QObject::tr("Go To an image"));

    mFileActions[menu_file_save] = new QAction(mFileIcons[icon_file_save], QObject::tr("&Save"), parent);
    mFileActions[menu_file_save]->setShortcuts(QKeySequence::Save);
    mFileActions[menu_file_save]->setStatusTip(QObject::tr("Save an image"));

    mFileActions[menu_file_save_as] = new QAction(QObject::tr("S&ave As"), parent);
    mFileActions[menu_file_save_as]->setShortcut(QKeySequence(shortcut_save_as));
    mFileActions[menu_file_save_as]->setStatusTip(QObject::tr("Save an image as"));

    mFileActions[menu_file_save_copy] = new QAction(QObject::tr("Sa&ve a Copy"), parent);
    mFileActions[menu_file_save_copy]->setStatusTip(QObject::tr("Copy the Image"));

    mFileActions[menu_file_save_list] = new QAction(QObject::tr("&Save Tabs"), parent);
    mFileActions[menu_file_save_list]->setStatusTip(
        QObject::tr("Save a newline separated list of the filenames of the open tabs"));

    mFileActions[menu_file_save_web] = new QAction(QObject::tr("&Save for Web"), parent);
    mFileActions[menu_file_save_web]->setStatusTip(QObject::tr("Save an Image for Web Applications"));

    mFileActions[menu_file_print] = new QAction(mFileIcons[icon_file_print], QObject::tr("&Print"), parent);
    mFileActions[menu_file_print]->setShortcuts(QKeySequence::Print);
    mFileActions[menu_file_print]->setStatusTip(QObject::tr("Print an image"));

    mFileActions[menu_file_show_recent] = new QAction(QObject::tr("&Recent Files"), parent);
    mFileActions[menu_file_show_recent]->setShortcut(QKeySequence(shortcut_recent_files));
    mFileActions[menu_file_show_recent]->setCheckable(true);
    mFileActions[menu_file_show_recent]->setChecked(false);
    mFileActions[menu_file_show_recent]->setStatusTip(QObject::tr("Show Recent Files"));

    mFileActions[menu_file_reload] = new QAction(QObject::tr("&Reload File"), parent);
    mFileActions[menu_file_reload]->setShortcut(QKeySequence(shortcut_reload));
    mFileActions[menu_file_reload]->setStatusTip(QObject::tr("Reload File"));

    mFileActions[menu_file_next] = new QAction(mFileIcons[icon_file_next], QObject::tr("Ne&xt File"), parent);
    mFileActions[menu_file_next]->setShortcut(QKeySequence(shortcut_next_file));
    mFileActions[menu_file_next]->setStatusTip(QObject::tr("Load next file"));

    mFileActions[menu_file_prev] = new QAction(mFileIcons[icon_file_prev], QObject::tr("Pre&vious File"), parent);
    mFileActions[menu_file_prev]->setShortcut(QKeySequence(shortcut_prev_file));
    mFileActions[menu_file_prev]->setStatusTip(QObject::tr("Load previous file"));

    mFileActions[menu_file_new_instance] = new QAction(QObject::tr("St&art New Instance"), parent);
    mFileActions[menu_file_new_instance]->setShortcut(QKeySequence(shortcut_new_instance));
    mFileActions[menu_file_new_instance]->setStatusTip(QObject::tr("Open file in new instance"));

    mFileActions[menu_file_private_instance] = new QAction(QObject::tr("St&art Private Instance"), parent);
    mFileActions[menu_file_private_instance]->setShortcut(QKeySequence(shortcut_private_instance));
    mFileActions[menu_file_private_instance]->setStatusTip(QObject::tr("Open private instance"));

    mFileActions[menu_file_find] = new QAction(mFileIcons[icon_file_find], QObject::tr("&Find && Filter"), parent);
    mFileActions[menu_file_find]->setShortcut(QKeySequence::Find);
    mFileActions[menu_file_find]->setStatusTip(QObject::tr("Find an image"));

    mFileActions[menu_file_recursive] = new QAction(QObject::tr("Scan Folder Re&cursive"), parent);
    mFileActions[menu_file_recursive]->setStatusTip(QObject::tr("Step through Folder and Sub Folders"));
    mFileActions[menu_file_recursive]->setCheckable(true);
    mFileActions[menu_file_recursive]->setChecked(DkSettingsManager::param().global().scanSubFolders);

    mFileActions[menu_file_exit] = new QAction(QObject::tr("&Exit"), parent);
    mFileActions[menu_file_exit]->setStatusTip(QObject::tr("Exit"));

    // sort actions
    mSortActions.resize(menu_sort_end);

    mSortActions[menu_sort_filename] = new QAction(QObject::tr("by &Filename"), parent);
    mSortActions[menu_sort_filename]->setObjectName("menu_sort_filename");
    mSortActions[menu_sort_filename]->setStatusTip(QObject::tr("Sort by Filename"));
    mSortActions[menu_sort_filename]->setCheckable(true);
    mSortActions[menu_sort_filename]->setChecked(DkSettingsManager::param().global().sortMode
                                                 == DkSettings::sort_filename);

    mSortActions[menu_sort_file_size] = new QAction(QObject::tr("by File &Size"), parent);
    mSortActions[menu_sort_file_size]->setObjectName("menu_sort_file_size");
    mSortActions[menu_sort_file_size]->setStatusTip(QObject::tr("Sort by File Size"));
    mSortActions[menu_sort_file_size]->setCheckable(true);
    mSortActions[menu_sort_file_size]->setChecked(DkSettingsManager::param().global().sortMode
                                                  == DkSettings::sort_file_size);

    mSortActions[menu_sort_date_created] = new QAction(QObject::tr("by Date &Created"), parent);
    mSortActions[menu_sort_date_created]->setObjectName("menu_sort_date_created");
    mSortActions[menu_sort_date_created]->setStatusTip(QObject::tr("Sort by Date Created"));
    mSortActions[menu_sort_date_created]->setCheckable(true);
    mSortActions[menu_sort_date_created]->setChecked(DkSettingsManager::param().global().sortMode
                                                     == DkSettings::sort_date_created);

    mSortActions[menu_sort_date_modified] = new QAction(QObject::tr("by Date Modified"), parent);
    mSortActions[menu_sort_date_modified]->setObjectName("menu_sort_date_modified");
    mSortActions[menu_sort_date_modified]->setStatusTip(QObject::tr("Sort by Date Last Modified"));
    mSortActions[menu_sort_date_modified]->setCheckable(true);
    mSortActions[menu_sort_date_modified]->setChecked(DkSettingsManager::param().global().sortMode
                                                      == DkSettings::sort_date_modified);

    mSortActions[menu_sort_random] = new QAction(QObject::tr("Random"), parent);
    mSortActions[menu_sort_random]->setObjectName("menu_sort_random");
    mSortActions[menu_sort_random]->setStatusTip(QObject::tr("Sort in Random Order"));
    mSortActions[menu_sort_random]->setCheckable(true);
    mSortActions[menu_sort_random]->setChecked(DkSettingsManager::param().global().sortMode == DkSettings::sort_random);

    mSortActions[menu_sort_ascending] = new QAction(QObject::tr("&Ascending"), parent);
    mSortActions[menu_sort_ascending]->setObjectName("menu_sort_ascending");
    mSortActions[menu_sort_ascending]->setStatusTip(QObject::tr("Sort in Ascending Order"));
    mSortActions[menu_sort_ascending]->setCheckable(true);
    mSortActions[menu_sort_ascending]->setChecked(DkSettingsManager::param().global().sortDir == Qt::AscendingOrder);

    mSortActions[menu_sort_descending] = new QAction(QObject::tr("&Descending"), parent);
    mSortActions[menu_sort_descending]->setObjectName("menu_sort_descending");
    mSortActions[menu_sort_descending]->setStatusTip(QObject::tr("Sort in Descending Order"));
    mSortActions[menu_sort_descending]->setCheckable(true);
    mSortActions[menu_sort_descending]->setChecked(DkSettingsManager::param().global().sortDir == Qt::DescendingOrder);

    // edit actions
    mEditActions.resize(menu_edit_end);

    mEditActions[menu_edit_rotate_cw] = new QAction(mEditIcons[icon_edit_rotate_cw],
                                                    QObject::tr("9&0%1 Clockwise").arg(dk_degree_str),
                                                    parent);
    mEditActions[menu_edit_rotate_cw]->setShortcut(QKeySequence(shortcut_rotate_cw));
    mEditActions[menu_edit_rotate_cw]->setStatusTip(QObject::tr("rotate the image 90%1 clockwise").arg(dk_degree_str));

    mEditActions[menu_edit_rotate_ccw] = new QAction(mEditIcons[icon_edit_rotate_ccw],
                                                     QObject::tr("&90%1 Counter Clockwise").arg(dk_degree_str),
                                                     parent);
    mEditActions[menu_edit_rotate_ccw]->setShortcut(QKeySequence(shortcut_rotate_ccw));
    mEditActions[menu_edit_rotate_ccw]->setStatusTip(
        QObject::tr("rotate the image 90%1 counter clockwise").arg(dk_degree_str));

    mEditActions[menu_edit_rotate_180] = new QAction(QObject::tr("180%1").arg(dk_degree_str), parent);
    mEditActions[menu_edit_rotate_180]->setStatusTip(QObject::tr("rotate the image by 180%1").arg(dk_degree_str));

    mEditActions[menu_edit_undo] = new QAction(mEditIcons[icon_edit_undo], QObject::tr("&Undo"), parent);
    mEditActions[menu_edit_undo]->setShortcut(QKeySequence::Undo);
    mEditActions[menu_edit_undo]->setStatusTip(QObject::tr("Undo Last Action"));

    mEditActions[menu_edit_redo] = new QAction(mEditIcons[icon_edit_redo], QObject::tr("&Redo"), parent);
    mEditActions[menu_edit_redo]->setShortcut(QKeySequence::Redo);
    mEditActions[menu_edit_redo]->setStatusTip(QObject::tr("Redo Last Action"));

    mEditActions[menu_edit_copy] = new QAction(mEditIcons[icon_edit_copy], QObject::tr("&Copy"), parent);
    mEditActions[menu_edit_copy]->setShortcut(QKeySequence::Copy);
    mEditActions[menu_edit_copy]->setStatusTip(QObject::tr("copy image"));

    mEditActions[menu_edit_copy_buffer] = new QAction(QObject::tr("Copy &Buffer"), parent);
    mEditActions[menu_edit_copy_buffer]->setShortcut(shortcut_copy_buffer);
    mEditActions[menu_edit_copy_buffer]->setStatusTip(QObject::tr("copy image"));

    mEditActions[menu_edit_copy_color] = new QAction(QObject::tr("Copy Co&lor"), parent);
    mEditActions[menu_edit_copy_color]->setShortcut(shortcut_copy_color);
    mEditActions[menu_edit_copy_color]->setStatusTip(QObject::tr("copy pixel color value as HEX"));

    QList<QKeySequence> pastScs;
    pastScs.append(QKeySequence::Paste);
    pastScs.append(shortcut_paste);
    mEditActions[menu_edit_paste] = new QAction(mEditIcons[icon_edit_paste], QObject::tr("&Paste"), parent);
    mEditActions[menu_edit_paste]->setShortcuts(pastScs);
    mEditActions[menu_edit_paste]->setStatusTip(QObject::tr("paste image"));

    mEditActions[menu_edit_delete] = new QAction(mEditIcons[icon_edit_delete], QObject::tr("&Delete"), parent);
    mEditActions[menu_edit_delete]->setShortcut(QKeySequence::Delete);
    mEditActions[menu_edit_delete]->setStatusTip(QObject::tr("delete current fileInfo"));

    mEditActions[menu_edit_shortcuts] = new QAction(QObject::tr("&Keyboard Shortcuts"), parent);
    mEditActions[menu_edit_shortcuts]->setShortcut(QKeySequence(shortcut_shortcuts));
    mEditActions[menu_edit_shortcuts]->setStatusTip(QObject::tr("lets you customize your keyboard shortcuts"));

    mEditActions[menu_edit_preferences] = new QAction(QObject::tr("&Settings"), parent);
    mEditActions[menu_edit_preferences]->setShortcut(QKeySequence(shortcut_settings));
    mEditActions[menu_edit_preferences]->setStatusTip(QObject::tr("settings"));

    // image adjustments menu
    mEditActions[menu_edit_image] = new QAction(mEditIcons[icon_edit_image], QObject::tr("Image &Adjustments"), parent);
    mEditActions[menu_edit_image]->setShortcut(QKeySequence(shortcut_edit_image));
    mEditActions[menu_edit_image]->setStatusTip(QObject::tr("open image manipulation toolbox"));
    mEditActions[menu_edit_image]->setCheckable(true);

    mEditActions[menu_edit_transform] = new QAction(mEditIcons[icon_edit_resize], QObject::tr("R&esize Image"), parent);
    mEditActions[menu_edit_transform]->setShortcut(shortcut_transform);
    mEditActions[menu_edit_transform]->setStatusTip(QObject::tr("resize the current image"));

    mEditActions[menu_edit_crop] = new QAction(mEditIcons[icon_edit_crop], QObject::tr("Cr&op Image"), parent);
    mEditActions[menu_edit_crop]->setShortcut(shortcut_crop);
    mEditActions[menu_edit_crop]->setStatusTip(QObject::tr("cut the current image"));
    mEditActions[menu_edit_crop]->setCheckable(true);
    mEditActions[menu_edit_crop]->setChecked(false);

    // panel actions
    mPanelActions.resize(menu_panel_end);
    mPanelActions[menu_panel_menu] = new QAction(QObject::tr("Show &Menu"), parent);
    mPanelActions[menu_panel_menu]->setStatusTip(QObject::tr("Hides the Menu and Shows it Again on ALT"));
    mPanelActions[menu_panel_menu]->setCheckable(true);

    mPanelActions[menu_panel_toolbar] = new QAction(QObject::tr("Tool&bar"), parent);
    mPanelActions[menu_panel_toolbar]->setShortcut(QKeySequence(shortcut_show_toolbar));
    mPanelActions[menu_panel_toolbar]->setStatusTip(QObject::tr("Show Toolbar"));
    mPanelActions[menu_panel_toolbar]->setCheckable(true);

    mPanelActions[menu_panel_statusbar] = new QAction(QObject::tr("&Statusbar"), parent);
    mPanelActions[menu_panel_statusbar]->setShortcut(QKeySequence(shortcut_show_statusbar));
    mPanelActions[menu_panel_statusbar]->setStatusTip(QObject::tr("Show Statusbar"));
    mPanelActions[menu_panel_statusbar]->setCheckable(true);

    mPanelActions[menu_panel_transfertoolbar] = new QAction(QObject::tr("&Pseudocolor Function"), parent);
    mPanelActions[menu_panel_transfertoolbar]->setShortcut(QKeySequence(shortcut_show_transfer));
    mPanelActions[menu_panel_transfertoolbar]->setStatusTip(QObject::tr("Show Pseudocolor Function"));
    mPanelActions[menu_panel_transfertoolbar]->setCheckable(true);
    mPanelActions[menu_panel_transfertoolbar]->setChecked(false);

    mPanelActions[menu_panel_overview] = new QAction(QObject::tr("O&verview"), parent);
    mPanelActions[menu_panel_overview]->setShortcut(QKeySequence(shortcut_show_overview));
    mPanelActions[menu_panel_overview]->setStatusTip(QObject::tr("Shows the Zoom Overview"));
    mPanelActions[menu_panel_overview]->setCheckable(true);
    mPanelActions[menu_panel_overview]->setChecked(
        DkSettingsManager::param().app().showOverview.testBit(DkSettingsManager::param().app().currentAppMode));

    mPanelActions[menu_panel_player] = new QAction(QObject::tr("Pla&yer"), parent);
    mPanelActions[menu_panel_player]->setShortcut(QKeySequence(shortcut_show_player));
    mPanelActions[menu_panel_player]->setStatusTip(QObject::tr("Shows the Slide Show Player"));
    mPanelActions[menu_panel_player]->setCheckable(true);

    mPanelActions[menu_panel_toggle] = new QAction(QObject::tr("&Hide All Panels"), parent);
    mPanelActions[menu_panel_toggle]->setShortcut(QKeySequence(shortcut_toggle_panels));
    mPanelActions[menu_panel_toggle]->setStatusTip(QObject::tr("Hide all panels"));
    mPanelActions[menu_panel_toggle]->setCheckable(true);
    mPanelActions[menu_panel_toggle]->setChecked(DkSettingsManager::param().app().hideAllPanels);

    mPanelActions[menu_panel_explorer] = new QAction(QObject::tr("File &Explorer"), parent);
    mPanelActions[menu_panel_explorer]->setShortcut(QKeySequence(shortcut_show_explorer));
    mPanelActions[menu_panel_explorer]->setStatusTip(QObject::tr("Show File Explorer"));
    mPanelActions[menu_panel_explorer]->setCheckable(true);

    mPanelActions[menu_panel_metadata_dock] = new QAction(QObject::tr("Metadata &Info"), parent);
    mPanelActions[menu_panel_metadata_dock]->setShortcut(QKeySequence(shortcut_show_metadata_dock));
    mPanelActions[menu_panel_metadata_dock]->setStatusTip(QObject::tr("Show Metadata Info"));
    mPanelActions[menu_panel_metadata_dock]->setCheckable(true);

    mPanelActions[menu_panel_preview] = new QAction(QObject::tr("&Thumbnails"), parent);
    mPanelActions[menu_panel_preview]->setShortcut(QKeySequence(shortcut_open_preview));
    mPanelActions[menu_panel_preview]->setStatusTip(QObject::tr("Show Thumbnails"));
    mPanelActions[menu_panel_preview]->setCheckable(true);

    mPanelActions[menu_panel_thumbview] = new QAction(QObject::tr("&Thumbnail Preview"), parent);
    mPanelActions[menu_panel_thumbview]->setShortcut(QKeySequence(shortcut_open_thumbview));
    mPanelActions[menu_panel_thumbview]->setStatusTip(QObject::tr("Show Thumbnails Preview"));
    mPanelActions[menu_panel_thumbview]->setCheckable(true);

    mPanelActions[menu_panel_scroller] = new QAction(QObject::tr("&Folder Scrollbar"), parent);
    mPanelActions[menu_panel_scroller]->setStatusTip(QObject::tr("Show Folder Scrollbar"));
    mPanelActions[menu_panel_scroller]->setCheckable(true);

    mPanelActions[menu_panel_exif] = new QAction(QObject::tr("&Metadata Ribbon"), parent);
    mPanelActions[menu_panel_exif]->setShortcut(QKeySequence(shortcut_show_exif));
    mPanelActions[menu_panel_exif]->setStatusTip(QObject::tr("Shows the Metadata Panel"));
    mPanelActions[menu_panel_exif]->setCheckable(true);

    mPanelActions[menu_panel_info] = new QAction(QObject::tr("File &Info"), parent);
    mPanelActions[menu_panel_info]->setShortcut(QKeySequence(shortcut_show_info));
    mPanelActions[menu_panel_info]->setStatusTip(QObject::tr("Shows the Info Panel"));
    mPanelActions[menu_panel_info]->setCheckable(true);

    mPanelActions[menu_panel_histogram] = new QAction(QObject::tr("&Histogram"), parent);
    mPanelActions[menu_panel_histogram]->setShortcut(QKeySequence(shortcut_show_histogram));
    mPanelActions[menu_panel_histogram]->setStatusTip(QObject::tr("Shows the Histogram Panel"));
    mPanelActions[menu_panel_histogram]->setCheckable(true);

    mPanelActions[menu_panel_comment] = new QAction(QObject::tr("Image &Notes"), parent);
    mPanelActions[menu_panel_comment]->setShortcut(QKeySequence(shortcut_show_comment));
    mPanelActions[menu_panel_comment]->setStatusTip(QObject::tr("Shows Image Notes"));
    mPanelActions[menu_panel_comment]->setCheckable(true);

    mPanelActions[menu_panel_history] = new QAction(QObject::tr("Edit &History"), parent);
    mPanelActions[menu_panel_history]->setStatusTip(QObject::tr("Shows the edit history"));
    mPanelActions[menu_panel_history]->setShortcut(QKeySequence(shortcut_show_history));
    mPanelActions[menu_panel_history]->setCheckable(true);

    mPanelActions[menu_panel_log] = new QAction(QObject::tr("Show &Log"), parent);
    mPanelActions[menu_panel_log]->setStatusTip(QObject::tr("Shows the log window"));
    mPanelActions[menu_panel_log]->setShortcut(QKeySequence(shortcut_show_log));
    mPanelActions[menu_panel_log]->setCheckable(true);

    // view actions
    mViewActions.resize(menu_view_end);

    mViewActions[menu_view_new_tab] = new QAction(QObject::tr("New &Tab"), parent);
    mViewActions[menu_view_new_tab]->setShortcut(QKeySequence(shortcut_new_tab));
    mViewActions[menu_view_new_tab]->setStatusTip(QObject::tr("Open a new tab"));

    mViewActions[menu_view_close_tab] = new QAction(QObject::tr("&Close Tab"), parent);
    mViewActions[menu_view_close_tab]->setShortcut(QKeySequence(shortcut_close_tab));
    mViewActions[menu_view_close_tab]->setStatusTip(QObject::tr("Close current tab"));

    mViewActions[menu_view_close_all_tabs] = new QAction(QObject::tr("&Close All Tabs"), parent);
    mViewActions[menu_view_close_all_tabs]->setStatusTip(QObject::tr("Close all open tabs"));

    mViewActions[menu_view_fit_frame] = new QAction(QObject::tr("&Fit Window to Image"), parent);
    mViewActions[menu_view_fit_frame]->setShortcut(QKeySequence(shortcut_fit_frame));
    mViewActions[menu_view_fit_frame]->setStatusTip(QObject::tr("Fit window to the image"));

    mViewActions[menu_view_first_tab] = new QAction(QObject::tr("F&irst Tab"), parent);
    mViewActions[menu_view_first_tab]->setStatusTip(QObject::tr("Switch to first tab"));

    mViewActions[menu_view_previous_tab] = new QAction(QObject::tr("&Previous Tab"), parent);
    mViewActions[menu_view_previous_tab]->setShortcut(QKeySequence(shortcut_previous_tab));
    mViewActions[menu_view_previous_tab]->setStatusTip(QObject::tr("Switch to previous tab"));

    mViewActions[menu_view_goto_tab] = new QAction(QObject::tr("&Go to Tab"), parent);
    mViewActions[menu_view_goto_tab]->setStatusTip(QObject::tr("Go to tab by index"));

    mViewActions[menu_view_next_tab] = new QAction(QObject::tr("&Next Tab"), parent);
    mViewActions[menu_view_next_tab]->setShortcut(QKeySequence(shortcut_next_tab));
    mViewActions[menu_view_next_tab]->setStatusTip(QObject::tr("Switch to next tab"));

    mViewActions[menu_view_last_tab] = new QAction(QObject::tr("La&st Tab"), parent);
    mViewActions[menu_view_last_tab]->setStatusTip(QObject::tr("Switch to last tab"));

    mViewActions[menu_view_fullscreen] = new QAction(mViewIcons[icon_view_fullscreen],
                                                     QObject::tr("Fu&ll Screen"),
                                                     parent);
    mViewActions[menu_view_fullscreen]->setShortcut(QKeySequence(shortcut_full_screen_ff));
    mViewActions[menu_view_fullscreen]->setStatusTip(QObject::tr("Full Screen"));

    mViewActions[menu_view_reset] = new QAction(mViewIcons[icon_view_reset],
                                                QObject::tr("&Fit Image to Window"),
                                                parent);
    mViewActions[menu_view_reset]->setShortcut(QKeySequence(shortcut_reset_view));
    mViewActions[menu_view_reset]->setStatusTip(QObject::tr("Zoom image to fit window"));

    mViewActions[menu_view_100] = new QAction(mViewIcons[icon_view_100], QObject::tr("Show &100%"), parent);
    mViewActions[menu_view_100]->setShortcut(QKeySequence(shortcut_zoom_full));
    mViewActions[menu_view_100]->setStatusTip(QObject::tr("Shows the image at 100%"));

    mViewActions[menu_view_zoom_in] = new QAction(mViewIcons[icon_view_zoom_in], QObject::tr("Zoom &In"), parent);
    mViewActions[menu_view_zoom_in]->setShortcut(QKeySequence::ZoomIn);
    mViewActions[menu_view_zoom_in]->setStatusTip(QObject::tr("zoom in"));

    mViewActions[menu_view_zoom_out] = new QAction(mViewIcons[icon_view_zoom_out], QObject::tr("&Zoom Out"), parent);
    mViewActions[menu_view_zoom_out]->setShortcut(QKeySequence::ZoomOut);
    mViewActions[menu_view_zoom_out]->setStatusTip(QObject::tr("zoom out"));

    mViewActions[menu_view_anti_aliasing] = new QAction(QObject::tr("&Anti Aliasing"), parent);
    mViewActions[menu_view_anti_aliasing]->setShortcut(QKeySequence(shortcut_anti_aliasing));
    mViewActions[menu_view_anti_aliasing]->setStatusTip(QObject::tr("if checked images are smoother"));
    mViewActions[menu_view_anti_aliasing]->setCheckable(true);
    mViewActions[menu_view_anti_aliasing]->setChecked(DkSettingsManager::param().display().antiAliasing);

    mViewActions[menu_view_tp_pattern] = new QAction(QObject::tr("&Transparency Pattern"), parent);
    mViewActions[menu_view_tp_pattern]->setShortcut(QKeySequence(shortcut_tp_pattern));
    mViewActions[menu_view_tp_pattern]->setStatusTip(
        QObject::tr("if checked, a pattern will be displayed for transparent objects"));
    mViewActions[menu_view_tp_pattern]->setCheckable(true);
    mViewActions[menu_view_tp_pattern]->setChecked(DkSettingsManager::param().display().tpPattern);

    mViewActions[menu_view_frameless] = new QAction(QObject::tr("&Frameless"), parent);
    mViewActions[menu_view_frameless]->setShortcut(QKeySequence(shortcut_frameless));
    mViewActions[menu_view_frameless]->setStatusTip(QObject::tr("shows a frameless window"));
    mViewActions[menu_view_frameless]->setCheckable(true);
    mViewActions[menu_view_frameless]->setChecked(false);

    mViewActions[menu_view_opacity_change] = new QAction(QObject::tr("&Change Opacity"), parent);
    mViewActions[menu_view_opacity_change]->setShortcut(QKeySequence(shortcut_opacity_change));
    mViewActions[menu_view_opacity_change]->setStatusTip(QObject::tr("change the window opacity"));

    mViewActions[menu_view_opacity_up] = new QAction(QObject::tr("Opacity &Up"), parent);
    mViewActions[menu_view_opacity_up]->setShortcut(QKeySequence(shortcut_opacity_up));
    mViewActions[menu_view_opacity_up]->setStatusTip(QObject::tr("changes the window opacity"));

    mViewActions[menu_view_opacity_down] = new QAction(QObject::tr("Opacity &Down"), parent);
    mViewActions[menu_view_opacity_down]->setShortcut(QKeySequence(shortcut_opacity_down));
    mViewActions[menu_view_opacity_down]->setStatusTip(QObject::tr("changes the window opacity"));

    mViewActions[menu_view_opacity_an] = new QAction(QObject::tr("To&ggle Opacity"), parent);
    mViewActions[menu_view_opacity_an]->setShortcut(QKeySequence(shortcut_an_opacity));
    mViewActions[menu_view_opacity_an]->setStatusTip(QObject::tr("toggle the window opacity"));

    mViewActions[menu_view_lock_window] = new QAction(QObject::tr("Lock &Window"), parent);
    mViewActions[menu_view_lock_window]->setShortcut(QKeySequence(shortcut_lock_window));
    mViewActions[menu_view_lock_window]->setStatusTip(QObject::tr("lock the window"));
    mViewActions[menu_view_lock_window]->setCheckable(true);
    mViewActions[menu_view_lock_window]->setChecked(false);

    mViewActions[menu_view_slideshow] = new QAction(QObject::tr("&Toggle Slideshow"), parent);
    mViewActions[menu_view_slideshow]->setStatusTip(QObject::tr("Start/Pause the slideshow"));
    mViewActions[menu_view_slideshow]->setShortcut(QKeySequence(shortcut_view_slideshow));

    mViewActions[menu_view_movie_pause] = new QAction(mViewIcons[icon_view_movie_play],
                                                      QObject::tr("&Pause Movie"),
                                                      parent);
    mViewActions[menu_view_movie_pause]->setStatusTip(QObject::tr("pause the current movie"));
    mViewActions[menu_view_movie_pause]->setCheckable(true);
    mViewActions[menu_view_movie_pause]->setChecked(false);

    mViewActions[menu_view_movie_prev] = new QAction(mViewIcons[icon_view_movie_prev],
                                                     QObject::tr("P&revious Frame"),
                                                     parent);
    mViewActions[menu_view_movie_prev]->setStatusTip(QObject::tr("show previous frame"));

    mViewActions[menu_view_movie_next] = new QAction(mViewIcons[icon_view_movie_next],
                                                     QObject::tr("&Next Frame"),
                                                     parent);
    mViewActions[menu_view_movie_next]->setStatusTip(QObject::tr("show next frame"));

    mViewActions[menu_view_monitors] = new QAction(QObject::tr("Choose &Monitor"), parent);
    mViewActions[menu_view_monitors]->setStatusTip(QObject::tr("Choose the Monitor to run nomacs"));

    mViewActions[menu_view_gps_map] = new QAction(mViewIcons[icon_view_gps],
                                                  QObject::tr("Show Image Location"),
                                                  parent);
    mViewActions[menu_view_gps_map]->setStatusTip(QObject::tr("shows where the image was taken in Google maps"));
    mViewActions[menu_view_gps_map]->setEnabled(false);

    // tools actions
    mToolsActions.resize(menu_tools_end);

    mToolsActions[menu_tools_thumbs] = new QAction(QObject::tr("Compute &Thumbnails"), parent);
    mToolsActions[menu_tools_thumbs]->setStatusTip(QObject::tr("compute all thumbnails of the current folder"));
    mToolsActions[menu_tools_thumbs]->setEnabled(false);

    mToolsActions[menu_tools_filter] = new QAction(mFileIcons[icon_file_filter], QObject::tr("&Filter"), parent);
    mToolsActions[menu_tools_filter]->setStatusTip(QObject::tr("Find an image"));
    mToolsActions[menu_tools_filter]->setCheckable(true);
    mToolsActions[menu_tools_filter]->setChecked(false);

    mToolsActions[menu_tools_export_tiff] = new QAction(QObject::tr("Export Multipage &TIFF"), parent);
    mToolsActions[menu_tools_export_tiff]->setStatusTip(QObject::tr("Export TIFF pages to multiple tiff files"));

    mToolsActions[menu_tools_extract_archive] = new QAction(QObject::tr("Extract From Archive"), parent);
    mToolsActions[menu_tools_extract_archive]->setStatusTip(
        QObject::tr("Extract images from an archive (%1)").arg(DkSettingsManager::param().app().containerRawFilters));
    mToolsActions[menu_tools_extract_archive]->setShortcut(QKeySequence(shortcut_extract));

    mToolsActions[menu_tools_mosaic] = new QAction(QObject::tr("&Mosaic Image"), parent);
    mToolsActions[menu_tools_mosaic]->setStatusTip(QObject::tr("Create a Mosaic Image"));

    mToolsActions[menu_tools_batch] = new QAction(QObject::tr("Batch Processing"), parent);
    mToolsActions[menu_tools_batch]->setStatusTip(QObject::tr("Apply actions to multiple images"));
    mToolsActions[menu_tools_batch]->setShortcut(QKeySequence(shortcut_batch_processing));

    mToolsActions[menu_tools_wallpaper] = new QAction(QObject::tr("&Wallpaper"), parent);
    mToolsActions[menu_tools_wallpaper]->setStatusTip(QObject::tr("set the current image as wallpaper"));

    mToolsActions[menu_tools_train_format] = new QAction(QObject::tr("Add Image Format"), parent);
    mToolsActions[menu_tools_train_format]->setStatusTip(QObject::tr("Add a new image format to nomacs"));

    // help menu
    mHelpActions.resize(menu_help_end);
    mHelpActions[menu_help_about] = new QAction(QObject::tr("&About Nomacs"), parent);
    mHelpActions[menu_help_about]->setShortcut(QKeySequence(shortcut_show_help));
    mHelpActions[menu_help_about]->setStatusTip(QObject::tr("about"));

    mHelpActions[menu_help_documentation] = new QAction(QObject::tr("&Documentation"), parent);
    mHelpActions[menu_help_documentation]->setStatusTip(QObject::tr("Online Documentation"));

    mHelpActions[menu_help_bug] = new QAction(QObject::tr("&Report a Bug"), parent);
    mHelpActions[menu_help_bug]->setStatusTip(QObject::tr("Report a Bug"));

    mHelpActions[menu_help_update] = new QAction(QObject::tr("&Check for Updates"), parent);
    mHelpActions[menu_help_update]->setStatusTip(QObject::tr("check for updates"));
    mHelpActions[menu_help_update]->setDisabled(DkSettingsManager::param().sync().disableUpdateInteraction);

    mHelpActions[menu_help_update_translation] = new QAction(QObject::tr("&Update Translation"), parent);
    mHelpActions[menu_help_update_translation]->setStatusTip(
        QObject::tr("Checks for a new version of the translations of the current language"));
    mHelpActions[menu_help_update_translation]->setDisabled(DkSettingsManager::param().sync().disableUpdateInteraction);

    // sync actions
    mSyncActions.resize(menu_sync_end);
    mSyncActions[menu_sync_view] = new QAction(QObject::tr("Synchronize &View"), parent);
    mSyncActions[menu_sync_view]->setShortcut(QKeySequence(shortcut_sync));
    mSyncActions[menu_sync_view]->setStatusTip(QObject::tr("synchronize the current view"));
    mSyncActions[menu_sync_view]->setEnabled(false);

    mSyncActions[menu_sync_pos] = new QAction(QObject::tr("&Window Overlay"), parent);
    mSyncActions[menu_sync_pos]->setShortcut(QKeySequence(shortcut_tab));
    mSyncActions[menu_sync_pos]->setStatusTip(QObject::tr("toggle the window opacity"));
    mSyncActions[menu_sync_pos]->setEnabled(false);

    mSyncActions[menu_sync_arrange] = new QAction(QObject::tr("Arrange Instances"), parent);
    mSyncActions[menu_sync_arrange]->setShortcut(QKeySequence(shortcut_arrange));
    mSyncActions[menu_sync_arrange]->setStatusTip(QObject::tr("arrange connected instances"));
    mSyncActions[menu_sync_arrange]->setEnabled(false);

    mSyncActions[menu_sync_connect_all] = new QAction(QObject::tr("Connect &All"), parent);
    mSyncActions[menu_sync_connect_all]->setShortcut(QKeySequence(shortcut_connect_all));
    mSyncActions[menu_sync_connect_all]->setStatusTip(QObject::tr("connect all instances"));

    mSyncActions[menu_sync_all_actions] = new QAction(QObject::tr("&Sync All Actions"), parent);
    mSyncActions[menu_sync_all_actions]->setStatusTip(QObject::tr("Transmit All Signals Automatically."));
    mSyncActions[menu_sync_all_actions]->setCheckable(true);
    mSyncActions[menu_sync_all_actions]->setChecked(DkSettingsManager::param().sync().syncActions);

    // plugin actions
    mPluginActions.resize(menu_plugins_end);
    mPluginActions[menu_plugin_manager] = new QAction(QObject::tr("&Plugin Manager"), parent);
    mPluginActions[menu_plugin_manager]->setStatusTip(QObject::tr("manage installed plugins and download new ones"));

    // preview actions
    mPreviewActions.resize(actions_end);

    mPreviewActions[preview_select_all] = new QAction(QObject::tr("Select &All"), parent);
    mPreviewActions[preview_select_all]->setShortcut(QKeySequence::SelectAll);
    mPreviewActions[preview_select_all]->setCheckable(true);

    mPreviewActions[preview_zoom_in] = new QAction(DkImage::loadIcon(":/nomacs/img/zoom-in.svg"),
                                                   QObject::tr("Zoom &In"),
                                                   parent);
    mPreviewActions[preview_zoom_in]->setShortcut(QKeySequence::ZoomIn);

    mPreviewActions[preview_zoom_out] = new QAction(DkImage::loadIcon(":/nomacs/img/zoom-out.svg"),
                                                    QObject::tr("Zoom &Out"),
                                                    parent);
    mPreviewActions[preview_zoom_out]->setShortcut(QKeySequence::ZoomOut);

    mPreviewActions[preview_display_squares] = new QAction(DkImage::loadIcon(":/nomacs/img/rects.svg"),
                                                           QObject::tr("Display &Squares"),
                                                           parent);
    mPreviewActions[preview_display_squares]->setCheckable(true);
    mPreviewActions[preview_display_squares]->setChecked(DkSettingsManager::param().display().displaySquaredThumbs);

    mPreviewActions[preview_show_labels] = new QAction(DkImage::loadIcon(":/nomacs/img/show-filename.svg"),
                                                       QObject::tr("Show &Filename"),
                                                       parent);
    mPreviewActions[preview_show_labels]->setCheckable(true);
    mPreviewActions[preview_show_labels]->setChecked(DkSettingsManager::param().display().showThumbLabel);

    mPreviewActions[preview_filter] = new QAction(QObject::tr("&Filter"), parent);
    mPreviewActions[preview_filter]->setShortcut(QKeySequence::Find);

    mPreviewActions[preview_delete] = new QAction(DkImage::loadIcon(":/nomacs/img/trash.svg"),
                                                  QObject::tr("&Delete"),
                                                  parent);
    mPreviewActions[preview_delete]->setShortcut(QKeySequence::Delete);

    mPreviewActions[preview_copy] = new QAction(DkImage::loadIcon(":/nomacs/img/copy.svg"),
                                                QObject::tr("&Copy"),
                                                parent);
    mPreviewActions[preview_copy]->setShortcut(QKeySequence::Copy);

    mPreviewActions[preview_paste] = new QAction(DkImage::loadIcon(":/nomacs/img/paste.svg"),
                                                 QObject::tr("&Paste"),
                                                 parent);
    mPreviewActions[preview_paste]->setShortcut(QKeySequence::Paste);

    mPreviewActions[preview_rename] = new QAction(DkImage::loadIcon(":/nomacs/img/rename.svg"),
                                                  QObject::tr("&Rename"),
                                                  parent);
    mPreviewActions[preview_rename]->setShortcut(QKeySequence(Qt::Key_F2));

    mPreviewActions[preview_batch] = new QAction(DkImage::loadIcon(":/nomacs/img/batch-processing.svg"),
                                                 QObject::tr("&Batch Process"),
                                                 parent);
    mPreviewActions[preview_batch]->setToolTip(QObject::tr("Adds selected files to batch processing."));
    mPreviewActions[preview_batch]->setShortcut(QKeySequence(Qt::Key_B));

    mPreviewActions[preview_print] = new QAction(DkImage::loadIcon(":/nomacs/img/print.svg"),
                                                 QObject::tr("&Batch Print"),
                                                 parent);
    mPreviewActions[preview_print]->setToolTip(QObject::tr("Prints selected files."));
    mPreviewActions[preview_print]->setShortcut(QKeySequence::Print);

    // hidden actions
    mHiddenActions.resize(sc_end);

    mHiddenActions[sc_test_img] = new QAction(QObject::QObject::tr("Lena"), parent);
    mHiddenActions[sc_test_img]->setStatusTip(QObject::QObject::tr("Show test image"));
    mHiddenActions[sc_test_img]->setShortcut(QKeySequence(shortcut_test_img));

    mHiddenActions[sc_test_rec] = new QAction(QObject::QObject::tr("All Images"), parent);
    mHiddenActions[sc_test_rec]->setStatusTip(QObject::QObject::tr("Generates all images in the world"));
    mHiddenActions[sc_test_rec]->setShortcut(QKeySequence(shortcut_test_rec));

    mHiddenActions[sc_test_pong] = new QAction(QObject::QObject::tr("Pong"), parent);
    mHiddenActions[sc_test_pong]->setStatusTip(QObject::tr("Start pong"));
    mHiddenActions[sc_test_pong]->setShortcut(QKeySequence(shortcut_test_pong));

    mHiddenActions[sc_first_file] = new QAction(QObject::tr("First File"), parent);
    mHiddenActions[sc_first_file]->setStatusTip(QObject::tr("Jump to first file"));
    mHiddenActions[sc_first_file]->setShortcut(QKeySequence(shortcut_first_file));

    mHiddenActions[sc_last_file] = new QAction(QObject::tr("Last File"), parent);
    mHiddenActions[sc_last_file]->setStatusTip(QObject::tr("Jump to the end of the current folder"));
    mHiddenActions[sc_last_file]->setShortcut(QKeySequence(shortcut_last_file));

    mHiddenActions[sc_skip_prev] = new QAction(QObject::tr("Skip Previous Images"), parent);
    mHiddenActions[sc_skip_prev]->setStatusTip(QObject::tr("Jumps 10 images before the current image"));
    mHiddenActions[sc_skip_prev]->setShortcut(QKeySequence(shortcut_skip_prev));

    mHiddenActions[sc_skip_next] = new QAction(QObject::tr("Skip Next Images"), parent);
    mHiddenActions[sc_skip_next]->setStatusTip(QObject::tr("Jumps 10 images after the current image"));
    mHiddenActions[sc_skip_next]->setShortcut(QKeySequence(shortcut_skip_next));

    mHiddenActions[sc_first_file_sync] = new QAction(QObject::tr("First File Sync"), parent);
    mHiddenActions[sc_first_file_sync]->setStatusTip(QObject::tr("Jump to first file"));
    mHiddenActions[sc_first_file_sync]->setShortcut(QKeySequence(shortcut_first_file_sync));

    mHiddenActions[sc_last_file_sync] = new QAction(QObject::tr("Last File Sync"), parent);
    mHiddenActions[sc_last_file_sync]->setStatusTip(QObject::tr("Jump to the end of the current folder"));
    mHiddenActions[sc_last_file_sync]->setShortcut(QKeySequence(shortcut_last_file_sync));

    mHiddenActions[sc_skip_prev_sync] = new QAction(QObject::tr("Skip Previous Images Sync"), parent);
    mHiddenActions[sc_skip_prev_sync]->setStatusTip(QObject::tr("Jumps 10 images before the current image"));
    mHiddenActions[sc_skip_prev_sync]->setShortcut(QKeySequence(shortcut_skip_prev_sync));

    mHiddenActions[sc_skip_next_sync] = new QAction(QObject::tr("Skip Next Images Sync"), parent);
    mHiddenActions[sc_skip_next_sync]->setStatusTip(QObject::tr("Jumps 10 images after the current image"));
    mHiddenActions[sc_skip_next_sync]->setShortcut(QKeySequence(shortcut_skip_next_sync));

    mHiddenActions[sc_delete_silent] = new QAction(QObject::tr("Delete File Silent"), parent);
    mHiddenActions[sc_delete_silent]->setStatusTip(QObject::tr("Deletes a file without warning"));
    mHiddenActions[sc_delete_silent]->setShortcut(QKeySequence(shortcut_delete_silent));

    mHiddenActions[sc_star_rating_0] = new QAction(QObject::tr("Star Rating 0"), parent);
    mHiddenActions[sc_star_rating_0]->setStatusTip(QObject::tr("Star rating which is saved to an image's metadata"));
    mHiddenActions[sc_star_rating_0]->setShortcut(QKeySequence(shortcut_star_rating_0));

    mHiddenActions[sc_star_rating_1] = new QAction(QObject::tr("Star Rating 1"), parent);
    mHiddenActions[sc_star_rating_1]->setStatusTip(QObject::tr("Star rating which is saved to an image's metadata"));
    mHiddenActions[sc_star_rating_1]->setShortcut(QKeySequence(shortcut_star_rating_1));

    mHiddenActions[sc_star_rating_2] = new QAction(QObject::tr("Star Rating 2"), parent);
    mHiddenActions[sc_star_rating_2]->setStatusTip(QObject::tr("Star rating which is saved to an image's metadata"));
    mHiddenActions[sc_star_rating_2]->setShortcut(QKeySequence(shortcut_star_rating_2));

    mHiddenActions[sc_star_rating_3] = new QAction(QObject::tr("Star Rating 3"), parent);
    mHiddenActions[sc_star_rating_3]->setStatusTip(QObject::tr("Star rating which is saved to an image's metadata"));
    mHiddenActions[sc_star_rating_3]->setShortcut(QKeySequence(shortcut_star_rating_3));

    mHiddenActions[sc_star_rating_4] = new QAction(QObject::tr("Star Rating 4"), parent);
    mHiddenActions[sc_star_rating_4]->setStatusTip(QObject::tr("Star rating which is saved to an image's metadata"));
    mHiddenActions[sc_star_rating_4]->setShortcut(QKeySequence(shortcut_star_rating_4));

    mHiddenActions[sc_star_rating_5] = new QAction(QObject::tr("Star Rating 5"), parent);
    mHiddenActions[sc_star_rating_5]->setStatusTip(QObject::tr("Star rating which is saved to an image's metadata"));
    mHiddenActions[sc_star_rating_5]->setShortcut(QKeySequence(shortcut_star_rating_5));

    mHiddenActions[sc_pan_up] = new QAction(QObject::tr("Pan Image Up"), parent);
    mHiddenActions[sc_pan_up]->setStatusTip(QObject::tr("Pans the image up if zoomed."));
    mHiddenActions[sc_pan_up]->setShortcut(QKeySequence(shortcut_pan_up));

    mHiddenActions[sc_pan_down] = new QAction(QObject::tr("Pan Image Down"), parent);
    mHiddenActions[sc_pan_down]->setStatusTip(QObject::tr("Pans the image down if zoomed."));
    mHiddenActions[sc_pan_down]->setShortcut(QKeySequence(shortcut_pan_down));

    mHiddenActions[sc_pan_left] = new QAction(QObject::tr("Pan Image Left"), parent);
    mHiddenActions[sc_pan_left]->setStatusTip(QObject::tr("Pans the image left if zoomed."));
    mHiddenActions[sc_pan_left]->setShortcut(QKeySequence(shortcut_pan_left));

    mHiddenActions[sc_pan_right] = new QAction(QObject::tr("Pan Image Right"), parent);
    mHiddenActions[sc_pan_right]->setStatusTip(QObject::tr("Pans the image right if zoomed."));
    mHiddenActions[sc_pan_right]->setShortcut(QKeySequence(shortcut_pan_right));

    mManipulators.createManipulators(parent);

    assignCustomShortcuts(allActions());

    // automatically add status tip as tool tip
    for (QAction *a : allActions())
        a->setToolTip(a->statusTip());

    // trivial connects
    QObject::connect(action(menu_panel_toggle), &QAction::triggered, [](bool hide) {
        DkSettingsManager::param().app().hideAllPanels = hide;
    });

    QObject::connect(action(menu_panel_statusbar), &QAction::triggered, [](bool show) {
        DkStatusBarManager::instance().show(show);
    });

    QObject::connect(action(menu_panel_toolbar), &QAction::triggered, [](bool show) {
        DkToolBarManager::inst().showDefaultToolBar(show);
    });

    QObject::connect(action(menu_sync_all_actions), &QAction::triggered, [](bool sync) {
        DkSettingsManager::param().sync().syncActions = sync;
    });
}

QVector<QAction *> DkActionManager::allActions() const
{
    QVector<QAction *> all;
    all += fileActions();
    all += sortActions();
    all += openWithActions();
    all += viewActions();
    all += editActions();
    all += manipulatorActions();
    all += toolsActions();
    all += panelActions();
    all += syncActions();
    all += pluginActions();
    all += helpActions();
    all += previewActions();

    all += hiddenActions();

#ifdef WITH_PLUGINS
    all += pluginActionManager()->pluginDummyActions();
#endif

    return all;
}

void DkActionManager::assignCustomShortcuts(QVector<QAction *> actions) const
{
    DefaultSettings settings;
    settings.beginGroup("CustomShortcuts");

    for (QAction *a : actions) {
        if (!a) {
            qWarning() << "NULL action detected!";
            continue;
        }

        QString at = a->text().remove("&");
        QString val = settings.value(at, "no-shortcut").toString();

        if (val != "no-shortcut")
            a->setShortcut(val);
    }

    settings.endGroup();
}

void DkActionManager::enableImageActions(bool enable) const
{
    action(DkActionManager::menu_file_save)->setEnabled(enable);
    action(DkActionManager::menu_file_save_as)->setEnabled(enable);
    action(DkActionManager::menu_file_save_copy)->setEnabled(enable);
    action(DkActionManager::menu_file_save_list)->setEnabled(enable);
    action(DkActionManager::menu_file_save_web)->setEnabled(enable);
    action(DkActionManager::menu_file_rename)->setEnabled(enable);
    action(DkActionManager::menu_file_print)->setEnabled(enable);
    action(DkActionManager::menu_file_reload)->setEnabled(enable);
    action(DkActionManager::menu_file_prev)->setEnabled(enable);
    action(DkActionManager::menu_file_next)->setEnabled(enable);
    action(DkActionManager::menu_file_goto)->setEnabled(enable);
    action(DkActionManager::menu_file_find)->setEnabled(enable);

    action(DkActionManager::menu_edit_rotate_cw)->setEnabled(enable);
    action(DkActionManager::menu_edit_rotate_ccw)->setEnabled(enable);
    action(DkActionManager::menu_edit_rotate_180)->setEnabled(enable);
    action(DkActionManager::menu_edit_delete)->setEnabled(enable);
    action(DkActionManager::menu_edit_transform)->setEnabled(enable);
    action(DkActionManager::menu_edit_crop)->setEnabled(enable);
    action(DkActionManager::menu_edit_copy)->setEnabled(enable);
    action(DkActionManager::menu_edit_copy_buffer)->setEnabled(enable);
    action(DkActionManager::menu_edit_copy_color)->setEnabled(enable);
    action(DkActionManager::menu_edit_undo)->setEnabled(enable);
    action(DkActionManager::menu_edit_redo)->setEnabled(enable);

    action(DkActionManager::menu_panel_info)->setEnabled(enable);
#ifdef WITH_OPENCV
    action(DkActionManager::menu_panel_histogram)->setEnabled(enable);
#else
    action(DkActionManager::menu_panel_histogram)->setEnabled(false);
#endif
    action(DkActionManager::menu_panel_scroller)->setEnabled(enable);
    action(DkActionManager::menu_panel_comment)->setEnabled(enable);
    action(DkActionManager::menu_panel_preview)->setEnabled(enable);
    action(DkActionManager::menu_panel_exif)->setEnabled(enable);
    action(DkActionManager::menu_panel_overview)->setEnabled(enable);
    action(DkActionManager::menu_panel_player)->setEnabled(enable);
    // action(DkActionManager::menu_panel_thumbview)->setEnabled(enable);	// if we don't disable the thumbnail
    // preview, we can toggle it with SHIFT+T

    action(DkActionManager::menu_view_slideshow)->setEnabled(enable);
    action(DkActionManager::menu_view_fullscreen)->setEnabled(enable);
    action(DkActionManager::menu_view_reset)->setEnabled(enable);
    action(DkActionManager::menu_view_100)->setEnabled(enable);
    action(DkActionManager::menu_view_fit_frame)->setEnabled(enable);
    action(DkActionManager::menu_view_zoom_in)->setEnabled(enable);
    action(DkActionManager::menu_view_zoom_out)->setEnabled(enable);
    action(DkActionManager::menu_view_tp_pattern)->setEnabled(enable);
    action(DkActionManager::menu_view_anti_aliasing)->setEnabled(enable);

    action(DkActionManager::menu_tools_wallpaper)->setEnabled(enable);
    action(DkActionManager::menu_tools_thumbs)->setEnabled(enable);

    // hidden actions
    action(DkActionManager::sc_skip_prev)->setEnabled(enable);
    action(DkActionManager::sc_skip_prev_sync)->setEnabled(enable);
    action(DkActionManager::sc_skip_next)->setEnabled(enable);
    action(DkActionManager::sc_skip_next_sync)->setEnabled(enable);
    action(DkActionManager::sc_first_file)->setEnabled(enable);
    action(DkActionManager::sc_first_file_sync)->setEnabled(enable);
    action(DkActionManager::sc_last_file)->setEnabled(enable);
    action(DkActionManager::sc_last_file_sync)->setEnabled(enable);

    // disable open with actions
    for (QAction *a : DkActionManager::instance().appManager()->getActions())
        a->setEnabled(enable);
}

void DkActionManager::enableMovieActions(bool enable) const
{
    DkSettingsManager::param().app().showMovieToolBar = enable;

    action(DkActionManager::menu_view_movie_pause)->setEnabled(enable);
    action(DkActionManager::menu_view_movie_prev)->setEnabled(enable);
    action(DkActionManager::menu_view_movie_next)->setEnabled(enable);

    action(DkActionManager::menu_view_movie_pause)->setChecked(false);

    DkToolBarManager::inst().showMovieToolBar(enable);
}

void DkActionManager::enableViewPortPluginActions(bool enable) const
{
    // actions to disable when a viewport plugin is activated,
    // and disable when de-activated

#ifdef WITH_PLUGINS
    // opening another plugin is currently broken
    // fixme: for some reason these don't disable in the menu ?!
    for (auto *a : mPluginManager->pluginActions())
        a->setEnabled(enable);

    for (auto *a : mPluginManager->pluginDummyActions())
        a->setEnabled(enable);

    // this is a partial workaround; quick launch will continue to launch plugins
    mPluginManager->menu()->setEnabled(enable);
#endif

    constexpr EditMenuActions disabledEditActions[] = {
        menu_edit_undo, // plugins have their own undo/redo, do not interface directly with history/imageloader
        menu_edit_redo,
        menu_edit_copy, // plugins have their own copy/paste, should not copy from underlying imageloader
        menu_edit_paste,
        menu_edit_copy_buffer,
        menu_edit_copy_color,
        menu_edit_preferences, // tabs do not work with plugins
    };
    for (auto i : disabledEditActions)
        action(i)->setEnabled(enable);

    constexpr ViewMenuActions disabledViewActions[] = {
        menu_view_new_tab, // tabs
        menu_view_close_tab,
        menu_view_close_all_tabs,
        menu_view_first_tab,
        menu_view_previous_tab,
        menu_view_goto_tab,
        menu_view_next_tab,
        menu_view_last_tab,
        menu_view_frameless, // restarts nomacs
        menu_view_slideshow,
    };
    for (auto i : disabledViewActions)
        action(i)->setEnabled(enable);

    action(menu_tools_batch)->setEnabled(enable); // tabs
}
}
