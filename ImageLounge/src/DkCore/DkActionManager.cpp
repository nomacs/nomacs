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
    loadSettings();

    QString name, id;

#ifndef Q_OS_WIN
    name = kOpenDirAppName.toString();
    id = actionId(name);
    if (!containsApp(id)) {
#if defined(Q_OS_MACOS)
        QString fileManagerName = tr("&Finder");
#else
        QString fileManagerName = tr("&File Manager");
#endif
        auto *action = new QAction(fileManagerName);
        action->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_E));
        action->setToolTip(name);
        action->setObjectName(id);
        mApps.append(action);
    }
#endif

    name = kOpenFileAppName.toString();
    id = actionId(name);
    if (!containsApp(id)) {
        auto *action = new QAction(tr("&Default Application"));
        action->setToolTip(name);
        action->setObjectName(id);
        mApps.append(action);
    }

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
    }
    settings.endArray();
    settings.endGroup();
}

QString DkAppManager::actionId(const QString &name) const
{
    // return a unique name for binding shortcuts and custom toolbars
    const static QRegularExpression whiteSpace("\\s+");

    return "openwith_" + name.toLower().remove(whiteSpace);
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
        action->setObjectName(actionId(action->text()));

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
    newApp->setObjectName(actionId(newApp->text()));
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

    QString id = actionId("@photoshop");
    if (!containsApp(id)) {
        appPath = searchForSoftware("Adobe", "Photoshop", "ApplicationPath");
        if (!appPath.isEmpty()) {
            auto *a = new QAction(QObject::tr("&Photoshop"), parent());
            a->setToolTip(QDir::fromNativeSeparators(appPath));
            a->setObjectName(id);
            mApps.append(a);
        }
    }

    id = actionId("@picasa");
    if (!containsApp(id)) {
        appPath = searchForSoftware("Google", "Picasa", "Directory");
        if (!appPath.isEmpty()) {
            auto *a = new QAction(QObject::tr("Pic&asa"), parent());
            a->setToolTip(QDir::fromNativeSeparators(appPath));
            a->setObjectName(id);
            mApps.append(a);
        }
    }

    id = actionId("@picasa_viewer");
    if (!containsApp(id)) {
        appPath = searchForSoftware("Google", "Picasa", "Directory", "PicasaPhotoViewer.exe");
        if (!appPath.isEmpty()) {
            auto *a = new QAction(QObject::tr("Picasa Ph&oto Viewer"), parent());
            a->setToolTip(QDir::fromNativeSeparators(appPath));
            a->setObjectName(id);
            mApps.append(a);
        }
    }

    id = actionId("@irfanview");
    if (!containsApp(id)) {
        appPath = searchForSoftware("IrfanView", "shell");
        if (!appPath.isEmpty()) {
            auto *a = new QAction(QObject::tr("&IrfanView"), parent());
            a->setToolTip(QDir::fromNativeSeparators(appPath));
            a->setObjectName(id);
            mApps.append(a);
        }
    }

    id = actionId("@explorer");
    if (!containsApp(id)) {
        appPath = "C:/Windows/explorer.exe";
        if (QFileInfo(appPath).exists()) {
            auto *a = new QAction(QObject::tr("&Explorer"), parent());
            a->setToolTip(QDir::fromNativeSeparators(appPath));
            a->setObjectName(id);
            a->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_E));
            mApps.append(a);
        }
    }
}

bool DkAppManager::containsApp(QStringView appId) const
{
    auto it = std::find_if(mApps.begin(), mApps.end(), [appId](QAction *action) {
        return action->objectName() == appId;
    });
    return it != mApps.end();
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

DkActionBuilder::DkActionBuilder(QVector<QAction *> &actions, size_t numActions, QWidget *parent)
    : mActions{actions}
    , mParent{parent}
{
    mActions.resize(numActions);
}

DkActionBuilder::~DkActionBuilder() = default;

QAction *DkActionBuilder::add(DkActionId id, const QIcon &icon, const QString &text, const QString &statusTip)
{
    auto *a = new QAction{icon, text, mParent};
    a->setStatusTip(statusTip);
    a->setObjectName(id.objectName);
    mActions[id.index] = a;
    return a;
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

    mFileMenu->addAction(mFileActions[file_open]);
    mFileMenu->addAction(mFileActions[file_open_dir]);
    if (DkSettingsManager::param().global().extendedTabs)
        mFileMenu->addAction(mFileActions[file_open_list]);

    // add open with menu
    mFileMenu->addMenu(openWithMenu());
    mFileMenu->addAction(mFileActions[file_quick_launch]);

    mFileMenu->addSeparator();
    mFileMenu->addAction(mFileActions[file_save]);
    mFileMenu->addAction(mFileActions[file_save_as]);
    mFileMenu->addAction(mFileActions[file_save_copy]);
    if (DkSettingsManager::param().global().extendedTabs)
        mFileMenu->addAction(mFileActions[file_save_list]);
    mFileMenu->addAction(mFileActions[file_save_web]);
    mFileMenu->addAction(mFileActions[file_rename]);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mFileActions[file_show_recent]);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mFileActions[file_print]);
    mFileMenu->addSeparator();

    // add sort menu
    mFileMenu->addMenu(sortMenu());
    mFileMenu->addAction(mFileActions[file_recursive]);
    mFileMenu->addAction(mFileActions[nav_goto_file]);
    mFileMenu->addAction(mFileActions[file_find]);
    mFileMenu->addAction(mFileActions[file_reload]);
    mFileMenu->addAction(mFileActions[nav_prev_file]);
    mFileMenu->addAction(mFileActions[nav_next_file]);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mFileActions[file_new_instance]);
    mFileMenu->addAction(mFileActions[file_private_instance]);
    mFileMenu->addAction(mFileActions[file_exit]);

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
    mOpenWithMenu->addAction(mFileActions[file_app_manager]);

    return mOpenWithMenu;
}

QMenu *DkActionManager::createSortMenu(QWidget *parent)
{
    mSortMenu = new QMenu(QObject::tr("S&ort"), parent);
    mSortMenu->addAction(mSortActions[sort_filename]);
    mSortMenu->addAction(mSortActions[sort_file_size]);
    mSortMenu->addAction(mSortActions[sort_date_created]);
    mSortMenu->addAction(mSortActions[sort_date_modified]);
    mSortMenu->addAction(mSortActions[sort_random]);
    mSortMenu->addSeparator();
    mSortMenu->addAction(mSortActions[sort_ascending]);
    mSortMenu->addAction(mSortActions[sort_descending]);

    return mSortMenu;
}

QMenu *DkActionManager::createViewMenu(QWidget *parent /* = 0 */)
{
    mViewMenu = new QMenu(QObject::tr("&View"), parent);

    mViewMenu->addAction(mViewActions[view_frameless]);
    mViewMenu->addAction(mViewActions[view_fullscreen]);
    mViewMenu->addSeparator();

    mViewMenu->addAction(mViewActions[view_new_tab]);
    mViewMenu->addAction(mViewActions[view_close_tab]);
    mViewMenu->addAction(mViewActions[view_close_all_tabs]);

    if (DkSettingsManager::param().global().extendedTabs)
        mViewMenu->addAction(mViewActions[view_first_tab]);
    mViewMenu->addAction(mViewActions[view_previous_tab]);
    if (DkSettingsManager::param().global().extendedTabs)
        mViewMenu->addAction(mViewActions[view_goto_tab]);
    mViewMenu->addAction(mViewActions[view_next_tab]);
    if (DkSettingsManager::param().global().extendedTabs)
        mViewMenu->addAction(mViewActions[view_last_tab]);
    mViewMenu->addSeparator();

    mViewMenu->addAction(mViewActions[view_reset]);
    mViewMenu->addAction(mViewActions[view_100]);
    mViewMenu->addAction(mViewActions[view_fit_frame]);
    mViewMenu->addAction(mViewActions[view_zoom_in]);
    mViewMenu->addAction(mViewActions[view_zoom_out]);
    mViewMenu->addSeparator();

    mViewMenu->addAction(mViewActions[view_tp_pattern]);
    mViewMenu->addAction(mViewActions[view_anti_aliasing]);
    mViewMenu->addSeparator();

    mViewMenu->addAction(mViewActions[view_opacity_change]);
    mViewMenu->addAction(mViewActions[view_opacity_up]);
    mViewMenu->addAction(mViewActions[view_opacity_down]);
    mViewMenu->addAction(mViewActions[view_opacity_an]);
#ifdef Q_OS_WIN
    mViewMenu->addAction(mViewActions[view_lock_window]);
#endif
    mViewMenu->addSeparator();

    mViewMenu->addAction(mViewActions[view_slideshow]);
    mViewMenu->addAction(mViewActions[view_movie_pause]);
    mViewMenu->addAction(mViewActions[view_movie_prev]);
    mViewMenu->addAction(mViewActions[view_movie_next]);
    mViewMenu->addSeparator();

    if (DkSettingsManager::param().app().currentAppMode == DkSettingsManager::param().mode_frameless) {
        mViewMenu->addAction(mViewActions[view_monitors]);
        mViewMenu->addSeparator();
    }

    mViewMenu->addAction(mViewActions[view_gps_map]);

    return mViewMenu;
}

QMenu *DkActionManager::createEditMenu(QWidget *parent /* = 0 */)
{
    mEditMenu = new QMenu(QObject::tr("&Edit"), parent);

    mEditMenu->addAction(mEditActions[edit_copy]);
    mEditMenu->addAction(mEditActions[edit_copy_buffer]);
    mEditMenu->addAction(mEditActions[edit_paste]);
    mEditMenu->addAction(mEditActions[edit_delete]);
    mEditMenu->addSeparator();
    mEditMenu->addAction(mEditActions[edit_rotate_ccw]);
    mEditMenu->addAction(mEditActions[edit_rotate_cw]);
    mEditMenu->addAction(mEditActions[edit_rotate_180]);
    mEditMenu->addSeparator();
    mEditMenu->addAction(mEditActions[edit_undo]);
    mEditMenu->addAction(mEditActions[edit_redo]);
    mEditMenu->addSeparator();
    mEditMenu->addAction(mEditActions[edit_shortcuts]);
    mEditMenu->addAction(mEditActions[edit_preferences]);

    return mEditMenu;
}

QMenu *DkActionManager::createManipulatorMenu(QWidget *parent)
{
    mManipulatorMenu = new QMenu(QObject::tr("&Adjustments"), parent);

    for (auto action : mManipulators.actions())
        mManipulatorMenu->addAction(action);

    mManipulatorMenu->addSeparator();
    mManipulatorMenu->addAction(mEditActions[edit_transform]);
    mManipulatorMenu->addAction(mEditActions[edit_crop]);
    mManipulatorMenu->addSeparator();
    mManipulatorMenu->addAction(mEditActions[panel_edit_image]);

    return mManipulatorMenu;
}

QMenu *DkActionManager::createPanelMenu(QWidget *parent)
{
    mPanelMenu = new QMenu(QObject::tr("&Panels"), parent);

    QMenu *toolsMenu = mPanelMenu->addMenu(QObject::tr("Tool&bars"));
    toolsMenu->addAction(mPanelActions[panel_menubar]);
    toolsMenu->addAction(mPanelActions[panel_toolbar]);
    toolsMenu->addAction(mPanelActions[panel_statusbar]);
    toolsMenu->addAction(mPanelActions[panel_transfertoolbar]);

    mPanelMenu->addAction(mPanelActions[panel_explorer]);
    mPanelMenu->addAction(mPanelActions[panel_metadata_dock]);
    mPanelMenu->addAction(mPanelActions[panel_exif]);
    mPanelMenu->addAction(mPanelActions[panel_history]);
    mPanelMenu->addAction(mPanelActions[panel_preview]);
    mPanelMenu->addAction(mPanelActions[panel_thumbview]);
    mPanelMenu->addAction(mPanelActions[panel_scroller]);
    mPanelMenu->addAction(mPanelActions[panel_log]);

    mPanelMenu->addSeparator();

    mPanelMenu->addAction(mPanelActions[panel_overview]);
    mPanelMenu->addAction(mPanelActions[panel_player]);
    mPanelMenu->addAction(mPanelActions[panel_info]);
    mPanelMenu->addAction(mPanelActions[panel_histogram]);
    mPanelMenu->addAction(mPanelActions[panel_comment]);

    mPanelMenu->addSeparator();
    mPanelMenu->addAction(mPanelActions[panel_toggle_all]);

    return mPanelMenu;
}

QMenu *DkActionManager::createToolsMenu(QWidget *parent /* = 0 */)
{
    mToolsMenu = new QMenu(QObject::tr("&Tools"), parent);

#ifdef WITH_LIBTIFF
    mToolsMenu->addAction(mToolsActions[tools_export_tiff]);
#endif
#ifdef WITH_QUAZIP
    mToolsMenu->addAction(mToolsActions[tools_extract_archive]);
#endif
#ifdef WITH_OPENCV
    mToolsMenu->addAction(mToolsActions[tools_mosaic]);
#endif
#ifdef Q_OS_WIN
    mToolsMenu->addAction(mToolsActions[tools_wallpaper]);
    mToolsMenu->addSeparator();
#endif
    mToolsMenu->addAction(mToolsActions[tools_batch]);
    mToolsMenu->addAction(mToolsActions[tools_thumbs]);
    mToolsMenu->addAction(mToolsActions[tools_train_format]);

    return mToolsMenu;
}

QMenu *DkActionManager::createHelpMenu(QWidget *parent)
{
    mHelpMenu = new QMenu(QObject::tr("&Help"), parent);

#ifndef Q_OS_LINUX
    mHelpMenu->addAction(mHelpActions[help_update]);
#endif // !Q_OS_LINUX
    mHelpMenu->addAction(mHelpActions[help_update_translation]);
    mHelpMenu->addSeparator();
    mHelpMenu->addAction(mHelpActions[help_bug]);
    mHelpMenu->addAction(mHelpActions[help_documentation]);
    mHelpMenu->addAction(mHelpActions[help_about]);

    return mHelpMenu;
}

QMenu *DkActionManager::createSyncMenu(QWidget *parent)
{
    mSyncMenu = new QMenu(QObject::tr("&Sync"), parent);

    // local host menu
    mLocalMenu = new DkTcpMenu(QObject::tr("&Synchronize"), mSyncMenu);
    mLocalMenu->showNoClientsFound(true);

    // add connect all action
    mLocalMenu->addTcpAction(mSyncActions[sync_connect_all]);

    mSyncMenu->addMenu(mLocalMenu);
    mSyncMenu->addAction(mSyncActions[sync_view]);
    mSyncMenu->addAction(mSyncActions[sync_pos]);
    mSyncMenu->addAction(mSyncActions[sync_arrange]);
    mSyncMenu->addAction(mSyncActions[sync_all_actions]);

    return mSyncMenu;
}

QMenu *DkActionManager::createContextMenu(QWidget *parent)
{
    mContextMenu = new QMenu(parent);

    mContextMenu->addAction(mEditActions[edit_copy_buffer]);
    mContextMenu->addAction(mEditActions[edit_copy]);
    mContextMenu->addAction(mEditActions[edit_copy_color]);
    mContextMenu->addAction(mEditActions[edit_paste]);
    mContextMenu->addSeparator();

    mContextMenu->addAction(mViewActions[view_frameless]);
    mContextMenu->addAction(mViewActions[view_fullscreen]);
    mContextMenu->addAction(mPanelActions[panel_menubar]);
    mContextMenu->addSeparator();

    if (DkSettingsManager::param().global().extendedTabs) {
        mContextMenu->addAction(mViewActions[view_first_tab]);
        mContextMenu->addAction(mViewActions[view_goto_tab]);
        mContextMenu->addAction(mViewActions[view_last_tab]);
        mContextMenu->addSeparator();
    }

    QMenu *panelMenu = mContextMenu->addMenu(QObject::tr("&Panels"));
    panelMenu->addAction(mPanelActions[panel_toggle_all]);
    panelMenu->addAction(mPanelActions[panel_explorer]);
    panelMenu->addAction(mPanelActions[panel_metadata_dock]);
    panelMenu->addAction(mPanelActions[panel_preview]);
    panelMenu->addAction(mPanelActions[panel_thumbview]);
    panelMenu->addAction(mPanelActions[panel_scroller]);
    panelMenu->addAction(mPanelActions[panel_exif]);
    panelMenu->addAction(mPanelActions[panel_overview]);
    panelMenu->addAction(mPanelActions[panel_player]);
    panelMenu->addAction(mPanelActions[panel_info]);
    panelMenu->addAction(mPanelActions[panel_histogram]);
    panelMenu->addAction(mPanelActions[panel_comment]);

    mContextMenu->addMenu(sortMenu());

    QMenu *viewContextMenu = mContextMenu->addMenu(QObject::tr("&View"));
    viewContextMenu->addAction(mViewActions[view_fullscreen]);
    viewContextMenu->addAction(mViewActions[view_reset]);
    viewContextMenu->addAction(mViewActions[view_100]);
    viewContextMenu->addAction(mViewActions[view_fit_frame]);

    QMenu *editContextMenu = mContextMenu->addMenu(QObject::tr("&Edit"));
    editContextMenu->addAction(mEditActions[panel_edit_image]);
    editContextMenu->addSeparator();
    editContextMenu->addAction(mEditActions[edit_undo]);
    editContextMenu->addAction(mEditActions[edit_redo]);
    editContextMenu->addSeparator();
    editContextMenu->addAction(mEditActions[edit_rotate_cw]);
    editContextMenu->addAction(mEditActions[edit_rotate_ccw]);
    editContextMenu->addAction(mEditActions[edit_rotate_180]);
    editContextMenu->addSeparator();
    editContextMenu->addAction(mEditActions[edit_transform]);
    editContextMenu->addAction(mEditActions[edit_crop]);
    editContextMenu->addAction(mEditActions[edit_delete]);

    mContextMenu->addMenu(mOpenWithMenu);

    mContextMenu->addSeparator();
    mContextMenu->addAction(mEditActions[edit_preferences]);

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

QAction *DkActionManager::action(FileAction action) const
{
    return mFileActions[action];
}

QAction *DkActionManager::action(SortAction action) const
{
    return mSortActions[action];
}

QAction *DkActionManager::action(ViewAction action) const
{
    return mViewActions[action];
}

QAction *DkActionManager::action(EditAction action) const
{
    return mEditActions[action];
}

QAction *DkActionManager::action(ToolsAction action) const
{
    return mToolsActions[action];
}

QAction *DkActionManager::action(PanelAction action) const
{
    return mPanelActions[action];
}

QAction *DkActionManager::action(SyncAction action) const
{
    return mSyncActions[action];
}

QAction *DkActionManager::action(HelpAction action) const
{
    return mHelpActions[action];
}

QAction *DkActionManager::action(PluginAction action) const
{
    return mPluginActions[action];
}

QAction *DkActionManager::action(MiscAction action) const
{
    return mMiscActions[action];
}

QAction *DkActionManager::action(PreviewAction action) const
{
    return mPreviewActions[action];
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

QVector<QAction *> DkActionManager::miscActions() const
{
    return mMiscActions;
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
    // FIXME: find a way not to hold pointers to things we do not own
    mAppManager = new DkAppManager(DkUtils::getMainWindow());

#ifdef WITH_PLUGINS
    mPluginManager = new DkPluginActionManager(DkUtils::getMainWindow());
    QObject::connect(mPluginManager, &QObject::destroyed, [this] {
        mPluginManager = nullptr;
    });
#endif
}
void DkActionManager::createActions(QWidget *parent)
{
    static const QIcon no_icon{};
    static const QKeySequence no_shortcut{};
    static const QString no_tooltip{};
    const bool unchecked{false};
    const bool checked{true};

    // file actions
    DkActionBuilder fileActions(mFileActions, numFileActions, parent);

    fileActions.add(ACTION_ID(file_open),
                    DkImage::loadIcon(":/nomacs/img/open.svg"),
                    QObject::tr("&Open"),
                    QObject::tr("Open an image"),
                    QKeySequence::Open);

    fileActions.add(ACTION_ID(file_open_dir),
                    DkImage::loadIcon(":/nomacs/img/open.svg"),
                    QObject::tr("Open &Directory"),
                    QObject::tr("Open a directory"),
                    shortcut_open_dir);

    fileActions.add(ACTION_ID(file_open_list),
                    no_icon,
                    QObject::tr("&Open Tabs"),
                    QObject::tr("Open a text file containing a list of filepaths, and open tabs for them"),
                    no_shortcut);

    fileActions.add(ACTION_ID(file_quick_launch),
                    no_icon,
                    QObject::tr("&Quick Launch"),
                    no_tooltip,
                    shortcut_quick_launch);

    fileActions.add(ACTION_ID(file_app_manager),
                    no_icon,
                    QObject::tr("&Manage Applications"),
                    QObject::tr("Manage Applications which are Automatically Opened"),
                    shortcut_app_manager);

    fileActions.add(ACTION_ID(file_rename),
                    no_icon,
                    QObject::tr("Re&name"),
                    QObject::tr("Rename an image"),
                    shortcut_rename);

    fileActions.add(ACTION_ID(nav_goto_file),
                    no_icon,
                    QObject::tr("&Go To"),
                    QObject::tr("Go To an image"),
                    shortcut_goto);

    fileActions.add(ACTION_ID(file_save),
                    DkImage::loadIcon(":/nomacs/img/save.svg"),
                    QObject::tr("&Save"),
                    QObject::tr("Save an image"),
                    QKeySequence::Save);

    fileActions.add(ACTION_ID(file_save_as),
                    no_icon,
                    QObject::tr("S&ave As"),
                    QObject::tr("Save an image as"),
                    shortcut_save_as);

    fileActions.add(ACTION_ID(file_save_copy),
                    no_icon,
                    QObject::tr("Sa&ve a Copy"),
                    QObject::tr("Copy the Image"),
                    no_shortcut);

    fileActions.add(ACTION_ID(file_save_list),
                    no_icon,
                    QObject::tr("&Save Tabs"),
                    QObject::tr("Save a newline separated list of the filenames of the open tabs"),
                    no_shortcut);

    fileActions.add(ACTION_ID(file_save_web),
                    no_icon,
                    QObject::tr("&Save for Web"),
                    QObject::tr("Save an Image for Web Applications"),
                    no_shortcut);

    fileActions.add(ACTION_ID(file_print),
                    DkImage::loadIcon(":/nomacs/img/print.svg"),
                    QObject::tr("&Print"),
                    QObject::tr("Print an image"),
                    QKeySequence::Print);

    fileActions.addCheckable(ACTION_ID(file_show_recent),
                             no_icon,
                             QObject::tr("&Recent Files"),
                             QObject::tr("Show Recent Files"),
                             shortcut_recent_files,
                             unchecked);

    fileActions.add(ACTION_ID(file_reload),
                    no_icon,
                    QObject::tr("&Reload File"),
                    QObject::tr("Reload File"),
                    shortcut_reload);

    fileActions.add(ACTION_ID(nav_next_file),
                    DkImage::loadIcon(":/nomacs/img/next.svg"),
                    QObject::tr("Ne&xt File"),
                    QObject::tr("Load next file"),
                    shortcut_next_file);

    fileActions.add(ACTION_ID(nav_prev_file),
                    DkImage::loadIcon(":/nomacs/img/previous.svg"),
                    QObject::tr("Pre&vious File"),
                    QObject::tr("Load previous file"),
                    shortcut_prev_file);

    fileActions.add(ACTION_ID(file_new_instance),
                    no_icon,
                    QObject::tr("St&art New Instance"),
                    QObject::tr("Open file in new instance"),
                    shortcut_new_instance);

    fileActions.add(ACTION_ID(file_private_instance),
                    no_icon,
                    QObject::tr("St&art Private Instance"),
                    QObject::tr("Open private instance"),
                    shortcut_private_instance);

    fileActions.add(ACTION_ID(file_find),
                    DkImage::loadIcon(":/nomacs/img/find.svg"),
                    QObject::tr("&Find && Filter"),
                    QObject::tr("Find an image"),
                    QKeySequence::Find);

    fileActions.addCheckable(ACTION_ID(file_recursive),
                             no_icon,
                             QObject::tr("Scan Folder Re&cursive"),
                             QObject::tr("Step through Folder and Sub Folders"),
                             no_shortcut,
                             DkSettingsManager::param().global().scanSubFolders);

    fileActions.add(ACTION_ID(file_exit), //
                    no_icon,
                    QObject::tr("&Exit"),
                    QObject::tr("Exit"),
                    no_shortcut);

    const auto sortMode = DkSettingsManager::param().global().sortMode;
    const auto sortDir = DkSettingsManager::param().global().sortDir;

    DkActionBuilder sortActions(mSortActions, numSortActions, parent);

    sortActions.addCheckable(ACTION_ID(sort_filename),
                             no_icon,
                             QObject::tr("by &Filename"),
                             QObject::tr("Sort by Filename"),
                             no_shortcut,
                             sortMode == DkSettings::sort_filename);

    sortActions.addCheckable(ACTION_ID(sort_file_size),
                             no_icon,
                             QObject::tr("by File &Size"),
                             QObject::tr("Sort by File Size"),
                             no_shortcut,
                             sortMode == DkSettings::sort_file_size);

    sortActions.addCheckable(ACTION_ID(sort_date_created),
                             no_icon,
                             QObject::tr("by Date &Created"),
                             QObject::tr("Sort by Date Created"),
                             no_shortcut,
                             sortMode == DkSettings::sort_date_created);

    sortActions.addCheckable(ACTION_ID(sort_date_modified),
                             no_icon,
                             QObject::tr("by Date Modified"),
                             QObject::tr("Sort by Date Last Modified"),
                             no_shortcut,
                             sortMode == DkSettings::sort_date_modified);

    sortActions.addCheckable(ACTION_ID(sort_random),
                             no_icon,
                             QObject::tr("Random"),
                             QObject::tr("Sort in Random Order"),
                             no_shortcut,
                             sortMode == DkSettings::sort_random);

    sortActions.addCheckable(ACTION_ID(sort_ascending),
                             no_icon,
                             QObject::tr("&Ascending"),
                             QObject::tr("Sort in Ascending Order"),
                             no_shortcut,
                             sortDir == Qt::AscendingOrder);

    sortActions.addCheckable(ACTION_ID(sort_descending),
                             no_icon,
                             QObject::tr("&Descending"),
                             QObject::tr("Sort in Descending Order"),
                             no_shortcut,
                             sortDir == Qt::DescendingOrder);

    DkActionBuilder editActions(mEditActions, numEditActions, parent);

    editActions.add(ACTION_ID(edit_rotate_cw),
                    DkImage::loadIcon(":/nomacs/img/rotate-cw.svg"),
                    QObject::tr("9&0%1 Clockwise").arg(dk_degree_str),
                    QObject::tr("rotate the image 90%1 clockwise").arg(dk_degree_str),
                    shortcut_rotate_cw);

    editActions.add(ACTION_ID(edit_rotate_ccw),
                    DkImage::loadIcon(":/nomacs/img/rotate-cc.svg"),
                    QObject::tr("&90%1 Counter Clockwise").arg(dk_degree_str),
                    QObject::tr("rotate the image 90%1 counter clockwise").arg(dk_degree_str),
                    shortcut_rotate_ccw);

    editActions.add(ACTION_ID(edit_rotate_180),
                    no_icon,
                    QObject::tr("180%1").arg(dk_degree_str),
                    QObject::tr("rotate the image by 180%1").arg(dk_degree_str),
                    no_shortcut);

    editActions.add(ACTION_ID(edit_undo),
                    DkImage::loadIcon(":/nomacs/img/edit-undo.svg"),
                    QObject::tr("&Undo"),
                    QObject::tr("Undo Last Action"),
                    QKeySequence::Undo);

    editActions.add(ACTION_ID(edit_redo),
                    DkImage::loadIcon(":/nomacs/img/edit-redo.svg"),
                    QObject::tr("&Redo"),
                    QObject::tr("Redo Last Action"),
                    QKeySequence::Redo);

    editActions.add(ACTION_ID(edit_copy),
                    DkImage::loadIcon(":/nomacs/img/copy.svg"),
                    QObject::tr("&Copy"),
                    QObject::tr("copy file path"),
                    QKeySequence::Copy);

    editActions.add(ACTION_ID(edit_copy_buffer),
                    no_icon,
                    QObject::tr("Copy &Buffer"),
                    QObject::tr("copy image pixels"),
                    shortcut_copy_buffer);

    editActions.add(ACTION_ID(edit_copy_color),
                    no_icon,
                    QObject::tr("Copy Co&lor"),
                    QObject::tr("copy pixel color value as HEX"),
                    shortcut_copy_color);

    editActions.add(ACTION_ID(edit_paste),
                    DkImage::loadIcon(":/nomacs/img/paste.svg"),
                    QObject::tr("&Paste"),
                    QObject::tr("paste image"),
                    QList<QKeySequence>{QKeySequence::Paste, shortcut_paste});

    editActions.add(ACTION_ID(edit_delete),
                    DkImage::loadIcon(":/nomacs/img/trash.svg"),
                    QObject::tr("&Delete"),
                    QObject::tr("delete current fileInfo"),
                    QKeySequence::Delete);

    editActions.add(ACTION_ID(edit_shortcuts),
                    no_icon,
                    QObject::tr("&Keyboard Shortcuts"),
                    QObject::tr("lets you customize your keyboard shortcuts"),
                    shortcut_shortcuts);

    editActions.add(ACTION_ID(edit_preferences),
                    no_icon,
                    QObject::tr("&Settings"),
                    QObject::tr("settings"),
                    shortcut_settings);

    editActions.addCheckable(ACTION_ID(panel_edit_image),
                             DkImage::loadIcon(":/nomacs/img/sliders.svg"),
                             QObject::tr("Image &Adjustments"),
                             QObject::tr("open image manipulation toolbox"),
                             shortcut_edit_image,
                             unchecked);

    editActions.add(ACTION_ID(edit_transform),
                    DkImage::loadIcon(":/nomacs/img/resize.svg"),
                    QObject::tr("R&esize Image"),
                    QObject::tr("resize the current image"),
                    shortcut_transform);

    editActions.addCheckable(ACTION_ID(edit_crop),
                             DkImage::loadIcon(":/nomacs/img/crop.svg"),
                             QObject::tr("Cr&op Image"),
                             QObject::tr("Crop the current image"),
                             shortcut_crop,
                             unchecked);

    // panel actions
    DkActionBuilder panelActions(mPanelActions, numPanelActions, parent);

    panelActions.addCheckable(ACTION_ID(panel_menubar),
                              no_icon,
                              QObject::tr("Show &Menu"),
                              QObject::tr("Hides the Menu and Shows it Again on ALT"),
                              no_shortcut,
                              checked);

    panelActions.addCheckable(ACTION_ID(panel_toolbar),
                              no_icon,
                              QObject::tr("Tool&bar"),
                              QObject::tr("Show Toolbar"),
                              shortcut_show_toolbar,
                              checked);

    panelActions.addCheckable(ACTION_ID(panel_statusbar),
                              no_icon,
                              QObject::tr("&Statusbar"),
                              QObject::tr("Show Statusbar"),
                              shortcut_show_statusbar,
                              checked);

    panelActions.addCheckable(ACTION_ID(panel_transfertoolbar),
                              no_icon,
                              QObject::tr("&Pseudocolor Function"),
                              QObject::tr("Show Pseudocolor Function"),
                              shortcut_show_transfer,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_overview),
                              no_icon,
                              QObject::tr("O&verview"),
                              QObject::tr("Shows the Zoom Overview"),
                              shortcut_show_overview,
                              DkSettingsManager::param().app().showOverview.testBit(
                                  DkSettingsManager::param().app().currentAppMode));

    panelActions.addCheckable(ACTION_ID(panel_player),
                              no_icon,
                              QObject::tr("Pla&yer"),
                              QObject::tr("Shows the Slide Show Player"),
                              shortcut_show_player,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_toggle_all),
                              no_icon,
                              QObject::tr("&Hide All Panels"),
                              QObject::tr("Hide all panels"),
                              shortcut_toggle_panels,
                              DkSettingsManager::param().app().hideAllPanels);

    panelActions.addCheckable(ACTION_ID(panel_explorer),
                              no_icon,
                              QObject::tr("File &Explorer"),
                              QObject::tr("Show File Explorer"),
                              shortcut_show_explorer,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_metadata_dock),
                              no_icon,
                              QObject::tr("Metadata &Info"),
                              QObject::tr("Show Metadata Info"),
                              shortcut_show_metadata_dock,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_preview),
                              no_icon,
                              QObject::tr("&Thumbnails"),
                              QObject::tr("Show Thumbnails"),
                              shortcut_open_preview,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_thumbview),
                              no_icon,
                              QObject::tr("&Thumbnail Preview"),
                              QObject::tr("Show Thumbnails Preview"),
                              shortcut_open_thumbview,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_scroller),
                              no_icon,
                              QObject::tr("&Folder Scrollbar"),
                              QObject::tr("Show Folder Scrollbar"),
                              no_shortcut,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_exif),
                              no_icon,
                              QObject::tr("&Metadata Ribbon"),
                              QObject::tr("Shows the Metadata Panel"),
                              shortcut_show_exif,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_info),
                              no_icon,
                              QObject::tr("File &Info"),
                              QObject::tr("Shows the Info Panel"),
                              shortcut_show_info,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_histogram),
                              no_icon,
                              QObject::tr("&Histogram"),
                              QObject::tr("Shows the Histogram Panel"),
                              shortcut_show_histogram,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_comment),
                              no_icon,
                              QObject::tr("Image &Notes"),
                              QObject::tr("Shows Image Notes"),
                              shortcut_show_comment,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_history),
                              no_icon,
                              QObject::tr("Edit &History"),
                              QObject::tr("Shows the edit history"),
                              shortcut_show_history,
                              unchecked);

    panelActions.addCheckable(ACTION_ID(panel_log),
                              no_icon,
                              QObject::tr("Show &Log"),
                              QObject::tr("Shows the log window"),
                              shortcut_show_log,
                              unchecked);

    // view actions
    DkActionBuilder viewActions(mViewActions, numViewActions, parent);

    viewActions.add(ACTION_ID(view_new_tab),
                    no_icon,
                    QObject::tr("New &Tab"),
                    QObject::tr("Open a new tab"),
                    shortcut_new_tab);

    viewActions.add(ACTION_ID(view_close_tab),
                    no_icon,
                    QObject::tr("&Close Tab"),
                    QObject::tr("Close current tab"),
                    shortcut_close_tab);

    viewActions.add(ACTION_ID(view_close_all_tabs),
                    no_icon,
                    QObject::tr("&Close All Tabs"),
                    QObject::tr("Close all open tabs"),
                    no_shortcut);

    viewActions.add(ACTION_ID(view_fit_frame),
                    no_icon,
                    QObject::tr("&Fit Window to Image"),
                    QObject::tr("Fit window to the image"),
                    shortcut_fit_frame);

    viewActions.add(ACTION_ID(view_first_tab),
                    no_icon,
                    QObject::tr("F&irst Tab"),
                    QObject::tr("Switch to first tab"),
                    no_shortcut);

    viewActions.add(ACTION_ID(view_previous_tab),
                    no_icon,
                    QObject::tr("&Previous Tab"),
                    QObject::tr("Switch to previous tab"),
                    shortcut_previous_tab);

    viewActions.add(ACTION_ID(view_goto_tab),
                    no_icon,
                    QObject::tr("&Go to Tab"),
                    QObject::tr("Go to tab by index"),
                    no_shortcut);

    viewActions.add(ACTION_ID(view_next_tab),
                    no_icon,
                    QObject::tr("&Next Tab"),
                    QObject::tr("Switch to next tab"),
                    shortcut_next_tab);

    viewActions.add(ACTION_ID(view_last_tab),
                    no_icon,
                    QObject::tr("La&st Tab"),
                    QObject::tr("Switch to last tab"),
                    no_shortcut);

    viewActions.add(ACTION_ID(view_fullscreen),
                    DkImage::loadIcon(":/nomacs/img/fullscreen.svg"),
                    QObject::tr("Fu&ll Screen"),
                    QObject::tr("Full Screen"),
                    shortcut_full_screen_ff);

    viewActions.add(ACTION_ID(view_reset),
                    DkImage::loadIcon(":/nomacs/img/zoom-reset.svg"),
                    QObject::tr("&Fit Image to Window"),
                    QObject::tr("Zoom image to fit window"),
                    shortcut_reset_view);

    viewActions.add(ACTION_ID(view_100),
                    DkImage::loadIcon(":/nomacs/img/zoom-100.svg"),
                    QObject::tr("Show &100%"),
                    QObject::tr("Shows the image at 100%"),
                    shortcut_zoom_full);

    viewActions.add(ACTION_ID(view_zoom_in),
                    DkImage::loadIcon(":/nomacs/img/zoom-in.svg"),
                    QObject::tr("Zoom &In"),
                    QObject::tr("zoom in"),
                    QKeySequence::ZoomIn);

    viewActions.add(ACTION_ID(view_zoom_out),
                    DkImage::loadIcon(":/nomacs/img/zoom-out.svg"),
                    QObject::tr("&Zoom Out"),
                    QObject::tr("zoom out"),
                    QKeySequence::ZoomOut);

    viewActions.addCheckable(ACTION_ID(view_anti_aliasing),
                             no_icon,
                             QObject::tr("&Anti Aliasing"),
                             QObject::tr("if checked images are smoother"),
                             shortcut_anti_aliasing,
                             DkSettingsManager::param().display().antiAliasing);

    viewActions.addCheckable(ACTION_ID(view_tp_pattern),
                             no_icon,
                             QObject::tr("&Transparency Pattern"),
                             QObject::tr("if checked, a pattern will be displayed for transparent objects"),
                             shortcut_tp_pattern,
                             DkSettingsManager::param().display().tpPattern);

    viewActions.addCheckable(ACTION_ID(view_frameless),
                             no_icon,
                             QObject::tr("&Frameless"),
                             QObject::tr("shows a frameless window"),
                             shortcut_frameless,
                             unchecked);

    viewActions.add(ACTION_ID(view_opacity_change),
                    no_icon,
                    QObject::tr("&Change Opacity"),
                    QObject::tr("change the window opacity"),
                    shortcut_opacity_change);

    viewActions.add(ACTION_ID(view_opacity_up),
                    no_icon,
                    QObject::tr("Opacity &Up"),
                    QObject::tr("changes the window opacity"),
                    shortcut_opacity_up);

    viewActions.add(ACTION_ID(view_opacity_down),
                    no_icon,
                    QObject::tr("Opacity &Down"),
                    QObject::tr("changes the window opacity"),
                    shortcut_opacity_down);

    viewActions.add(ACTION_ID(view_opacity_an),
                    no_icon,
                    QObject::tr("To&ggle Opacity"),
                    QObject::tr("toggle the window opacity"),
                    shortcut_an_opacity);

    viewActions.addCheckable(ACTION_ID(view_lock_window),
                             no_icon,
                             QObject::tr("Lock &Window"),
                             QObject::tr("lock the window"),
                             shortcut_lock_window,
                             unchecked);

    viewActions.add(ACTION_ID(view_slideshow),
                    no_icon,
                    QObject::tr("&Toggle Slideshow"),
                    QObject::tr("Start/Pause the slideshow"),
                    shortcut_view_slideshow);

    QIcon pausePlayIcon = DkImage::loadIcon(":/nomacs/img/pause.svg");
    pausePlayIcon.addFile(":/nomacs/img/play.svg", QSize(), QIcon::Normal, QIcon::On);
    viewActions.addCheckable(ACTION_ID(view_movie_pause),
                             pausePlayIcon,
                             QObject::tr("&Pause Movie"),
                             QObject::tr("pause the current movie"),
                             no_shortcut,
                             unchecked);

    viewActions.add(ACTION_ID(view_movie_prev),
                    DkImage::loadIcon(":/nomacs/img/previous.svg"),
                    QObject::tr("P&revious Frame"),
                    QObject::tr("show previous frame"),
                    no_shortcut);

    viewActions.add(ACTION_ID(view_movie_next),
                    DkImage::loadIcon(":/nomacs/img/next.svg"),
                    QObject::tr("&Next Frame"),
                    QObject::tr("show next frame"),
                    no_shortcut);

    viewActions.add(ACTION_ID(view_monitors),
                    no_icon,
                    QObject::tr("Choose &Monitor"),
                    QObject::tr("Choose the Monitor to run nomacs"),
                    no_shortcut);

    viewActions
        .add(ACTION_ID(view_gps_map),
             DkImage::loadIcon(":/nomacs/img/location.svg"),
             QObject::tr("Show Image Location"),
             QObject::tr("shows where the image was taken in Google maps"),
             no_shortcut)
        ->setEnabled(false);

    // tools actions
    DkActionBuilder toolsActions(mToolsActions, numToolsActions, parent);

    toolsActions
        .add(ACTION_ID(tools_thumbs),
             no_icon,
             QObject::tr("Compute &Thumbnails"),
             QObject::tr("compute all thumbnails of the current folder"),
             no_shortcut)
        ->setEnabled(false);

    QIcon filterIcon = DkImage::loadIcon(":/nomacs/img/filter-disabled.svg");
    filterIcon.addFile(":/nomacs/img/filter.svg", QSize(), QIcon::Normal, QIcon::On);

    toolsActions.addCheckable(ACTION_ID(tools_filter),
                              filterIcon,
                              QObject::tr("&Filter"),
                              QObject::tr("Find an image"),
                              no_shortcut,
                              unchecked);

    toolsActions.add(ACTION_ID(tools_export_tiff),
                     no_icon,
                     QObject::tr("Export Multipage &TIFF"),
                     QObject::tr("Export TIFF pages to multiple tiff files"),
                     no_shortcut);

    toolsActions.add(ACTION_ID(tools_extract_archive),
                     no_icon,
                     QObject::tr("Extract From Archive"),
                     QObject::tr("Extract images from an archive (%1)")
                         .arg(DkSettingsManager::param().app().containerRawFilters),
                     shortcut_extract);

    toolsActions.add(ACTION_ID(tools_mosaic),
                     no_icon,
                     QObject::tr("&Mosaic Image"),
                     QObject::tr("Create a Mosaic Image"),
                     no_shortcut);

    toolsActions.add(ACTION_ID(tools_wallpaper),
                     no_icon,
                     QObject::tr("Set Desktop &Wallpaper"),
                     QObject::tr("set the current image as wallpaper"),
                     no_shortcut);

    toolsActions.add(ACTION_ID(tools_train_format),
                     no_icon,
                     QObject::tr("Add Image Format"),
                     QObject::tr("Add a new image format to nomacs"),
                     no_shortcut);

    toolsActions.add(ACTION_ID(tools_batch),
                     no_icon,
                     QObject::tr("Batch Processing"),
                     QObject::tr("Apply actions to multiple images"),
                     shortcut_batch_processing);

    // help actions
    DkActionBuilder helpActions(mHelpActions, numHelpActions, parent);

    helpActions.add(ACTION_ID(help_about),
                    no_icon,
                    QObject::tr("&About Nomacs"),
                    QObject::tr("about"),
                    shortcut_show_help);

    helpActions.add(ACTION_ID(help_documentation),
                    no_icon,
                    QObject::tr("&Documentation"),
                    QObject::tr("Online Documentation"),
                    no_shortcut);

    helpActions.add(ACTION_ID(help_bug),
                    no_icon,
                    QObject::tr("&Report a Bug"),
                    QObject::tr("Report a Bug"),
                    no_shortcut);

    helpActions
        .add(ACTION_ID(help_update),
             no_icon,
             QObject::tr("&Check for Updates"),
             QObject::tr("check for updates"),
             no_shortcut)
        ->setDisabled(DkSettingsManager::param().sync().disableUpdateInteraction);

    helpActions
        .add(ACTION_ID(help_update_translation),
             no_icon,
             QObject::tr("&Update Translation"),
             QObject::tr("Checks for a new version of the translations of the current language"),
             no_shortcut)
        ->setDisabled(DkSettingsManager::param().sync().disableUpdateInteraction);

    // sync actions
    DkActionBuilder syncActions(mSyncActions, numSyncActions, parent);

    syncActions
        .add(ACTION_ID(sync_view),
             no_icon,
             QObject::tr("Synchronize &View"),
             QObject::tr("synchronize the current view"),
             shortcut_sync)
        ->setEnabled(false);

    syncActions
        .add(ACTION_ID(sync_pos),
             no_icon,
             QObject::tr("&Window Overlay"),
             QObject::tr("toggle the window opacity"),
             shortcut_tab)
        ->setEnabled(false);

    syncActions
        .add(ACTION_ID(sync_arrange),
             no_icon,
             QObject::tr("Arrange Instances"),
             QObject::tr("arrange connected instances"),
             shortcut_arrange)
        ->setEnabled(false);

    syncActions.add(ACTION_ID(sync_connect_all),
                    no_icon,
                    QObject::tr("Connect &All"),
                    QObject::tr("connect all instances"),
                    shortcut_connect_all);

    syncActions.addCheckable(ACTION_ID(sync_all_actions),
                             no_icon,
                             QObject::tr("&Sync All Actions"),
                             QObject::tr("Transmit All Signals Automatically."),
                             no_shortcut,
                             DkSettingsManager::param().sync().syncActions);

    // plugin actions
    DkActionBuilder pluginActions(mPluginActions, numPluginActions, parent);

    pluginActions.add(ACTION_ID(plugin_manager),
                      no_icon,
                      QObject::tr("&Plugin Manager"),
                      QObject::tr("manage installed plugins and download new ones"),
                      no_shortcut);

    // preview actions
    DkActionBuilder previewActions(mPreviewActions, numPreviewActions, parent);

    previewActions.addCheckable(ACTION_ID(preview_select_all),
                                no_icon,
                                QObject::tr("Select &All"),
                                no_tooltip,
                                QKeySequence::SelectAll,
                                checked);

    previewActions.add(ACTION_ID(preview_zoom_in),
                       DkImage::loadIcon(":/nomacs/img/zoom-in.svg"),
                       QObject::tr("Zoom &In"),
                       no_tooltip,
                       QKeySequence::ZoomIn);

    previewActions.add(ACTION_ID(preview_zoom_out),
                       DkImage::loadIcon(":/nomacs/img/zoom-out.svg"),
                       QObject::tr("Zoom &Out"),
                       no_tooltip,
                       QKeySequence::ZoomOut);

    previewActions.addCheckable(ACTION_ID(preview_display_squares),
                                DkImage::loadIcon(":/nomacs/img/rects.svg"),
                                QObject::tr("Display &Squares"),
                                no_tooltip,
                                no_shortcut,
                                DkSettingsManager::param().display().displaySquaredThumbs);

    previewActions.addCheckable(ACTION_ID(preview_show_labels),
                                DkImage::loadIcon(":/nomacs/img/show-filename.svg"),
                                QObject::tr("Show &Filename"),
                                no_tooltip,
                                no_shortcut,
                                DkSettingsManager::param().display().showThumbLabel);

    previewActions.add(ACTION_ID(preview_filter), //
                       no_icon,
                       QObject::tr("&Filter"),
                       no_tooltip,
                       QKeySequence::Find);

    previewActions.add(ACTION_ID(preview_delete), //
                       DkImage::loadIcon(":/nomacs/img/trash.svg"),
                       QObject::tr("&Delete"),
                       no_tooltip,
                       QKeySequence::Delete);

    previewActions.add(ACTION_ID(preview_copy), //
                       DkImage::loadIcon(":/nomacs/img/copy.svg"),
                       QObject::tr("&Copy"),
                       no_tooltip,
                       QKeySequence::Copy);

    previewActions.add(ACTION_ID(preview_paste), //
                       DkImage::loadIcon(":/nomacs/img/paste.svg"),
                       QObject::tr("&Paste"),
                       no_tooltip,
                       QKeySequence::Paste);

    previewActions.add(ACTION_ID(preview_rename), //
                       DkImage::loadIcon(":/nomacs/img/rename.svg"),
                       QObject::tr("&Rename"),
                       no_tooltip,
                       Qt::Key_F2);

    previewActions.add(ACTION_ID(preview_batch), //
                       DkImage::loadIcon(":/nomacs/img/batch-processing.svg"),
                       QObject::tr("&Batch Process"),
                       QObject::tr("Adds selected files to batch processing."),
                       Qt::Key_B);

    previewActions.add(ACTION_ID(preview_print), //
                       DkImage::loadIcon(":/nomacs/img/print.svg"),
                       QObject::tr("&Batch Print"), // FIXME: accelerator conflict
                       QObject::tr("Prints selected files."),
                       QKeySequence::Print);

    // miscellaneous actions
    DkActionBuilder miscActions(mMiscActions, numMiscActions, parent);

    miscActions.add(ACTION_ID(test_rec),
                    no_icon,
                    QObject::tr("All Images"),
                    QObject::tr("Generates all images in the world"),
                    shortcut_test_rec);

    miscActions.add(ACTION_ID(test_pong), no_icon, QObject::tr("Pong"), QObject::tr("Start pong"), shortcut_test_pong);

    miscActions.add(ACTION_ID(nav_first_file),
                    no_icon,
                    QObject::tr("First File"),
                    QObject::tr("Jump to first file"),
                    shortcut_first_file);

    miscActions.add(ACTION_ID(nav_last_file),
                    no_icon,
                    QObject::tr("Last File"),
                    QObject::tr("Jump to the end of the current folder"),
                    shortcut_last_file);

    miscActions.add(ACTION_ID(nav_skip_prev),
                    no_icon,
                    QObject::tr("Skip Previous Images"),
                    QObject::tr("Jumps 10 images before the current image"),
                    shortcut_skip_prev);

    miscActions.add(ACTION_ID(nav_skip_next),
                    no_icon,
                    QObject::tr("Skip Next Images"),
                    QObject::tr("Jumps 10 images after the current image"),
                    shortcut_skip_next);

    miscActions.add(ACTION_ID(nav_first_file_sync),
                    no_icon,
                    QObject::tr("First File Sync"),
                    QObject::tr("Jump to first file"),
                    shortcut_first_file_sync);

    miscActions.add(ACTION_ID(nav_last_file_sync),
                    no_icon,
                    QObject::tr("Last File Sync"),
                    QObject::tr("Jump to the end of the current folder"),
                    shortcut_last_file_sync);

    miscActions.add(ACTION_ID(nav_skip_prev_sync),
                    no_icon,
                    QObject::tr("Skip Previous Images Sync"),
                    QObject::tr("Jumps 10 images before the current image"),
                    shortcut_skip_prev_sync);

    miscActions.add(ACTION_ID(nav_skip_next_sync),
                    no_icon,
                    QObject::tr("Skip Next Images Sync"),
                    QObject::tr("Jumps 10 images after the current image"),
                    shortcut_skip_next_sync);

    miscActions.add(ACTION_ID(file_delete_silent),
                    no_icon,
                    QObject::tr("Delete File Silent"),
                    QObject::tr("Deletes a file without warning"),
                    shortcut_delete_silent);

    miscActions.add(ACTION_ID(star_rating_0),
                    no_icon,
                    QObject::tr("Star Rating 0"),
                    QObject::tr("Star rating which is saved to an image's metadata"),
                    shortcut_star_rating_0);

    miscActions.add(ACTION_ID(star_rating_1),
                    no_icon,
                    QObject::tr("Star Rating 1"),
                    QObject::tr("Star rating which is saved to an image's metadata"),
                    shortcut_star_rating_1);

    miscActions.add(ACTION_ID(star_rating_2),
                    no_icon,
                    QObject::tr("Star Rating 2"),
                    QObject::tr("Star rating which is saved to an image's metadata"),
                    shortcut_star_rating_2);

    miscActions.add(ACTION_ID(star_rating_3),
                    no_icon,
                    QObject::tr("Star Rating 3"),
                    QObject::tr("Star rating which is saved to an image's metadata"),
                    shortcut_star_rating_3);

    miscActions.add(ACTION_ID(star_rating_4),
                    no_icon,
                    QObject::tr("Star Rating 4"),
                    QObject::tr("Star rating which is saved to an image's metadata"),
                    shortcut_star_rating_4);

    miscActions.add(ACTION_ID(star_rating_5),
                    no_icon,
                    QObject::tr("Star Rating 5"),
                    QObject::tr("Star rating which is saved to an image's metadata"),
                    shortcut_star_rating_5);

    miscActions.add(ACTION_ID(view_pan_up),
                    no_icon,
                    QObject::tr("Pan Image Up"),
                    QObject::tr("Pans the image up if zoomed."),
                    shortcut_pan_up);
    miscActions.add(ACTION_ID(view_pan_down),
                    no_icon,
                    QObject::tr("Pan Image Down"),
                    QObject::tr("Pans the image down if zoomed."),
                    shortcut_pan_down);
    miscActions.add(ACTION_ID(view_pan_left),
                    no_icon,
                    QObject::tr("Pan Image Left"),
                    QObject::tr("Pans the image left if zoomed."),
                    shortcut_pan_left);
    miscActions.add(ACTION_ID(view_pan_right),
                    no_icon,
                    QObject::tr("Pan Image Right"),
                    QObject::tr("Pans the image right if zoomed."),
                    shortcut_pan_right);

    mManipulators.createManipulators(parent);

    assignCustomShortcuts(allActions());

    // automatically add status tip as tool tip
    for (QAction *a : allActions())
        a->setToolTip(a->statusTip());

    // trivial connects
    QObject::connect(action(panel_statusbar), &QAction::triggered, [](bool show) {
        DkStatusBarManager::instance().show(show);
    });

    QObject::connect(action(panel_toolbar), &QAction::triggered, [](bool show) {
        DkSettingsManager::param().app().showToolBar = show;
        DkToolBarManager::inst().showDefaultToolBar(show);
    });

    QObject::connect(action(sync_all_actions), &QAction::triggered, [](bool sync) {
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

    all += miscActions();

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

        QString actionId = a->objectName();
        QString val = settings.value(actionId, "no-shortcut").toString();

        if (val != "no-shortcut")
            a->setShortcut(val);
    }

    settings.endGroup();
}

void DkActionManager::enableImageActions(bool enable) const
{
    action(DkActionManager::file_save)->setEnabled(enable);
    action(DkActionManager::file_save_as)->setEnabled(enable);
    action(DkActionManager::file_save_copy)->setEnabled(enable);
    action(DkActionManager::file_save_list)->setEnabled(enable);
    action(DkActionManager::file_save_web)->setEnabled(enable);
    action(DkActionManager::file_rename)->setEnabled(enable);
    action(DkActionManager::file_print)->setEnabled(enable);
    action(DkActionManager::file_reload)->setEnabled(enable);
    action(DkActionManager::nav_prev_file)->setEnabled(enable);
    action(DkActionManager::nav_next_file)->setEnabled(enable);
    action(DkActionManager::nav_goto_file)->setEnabled(enable);
    action(DkActionManager::file_find)->setEnabled(enable);

    action(DkActionManager::edit_rotate_cw)->setEnabled(enable);
    action(DkActionManager::edit_rotate_ccw)->setEnabled(enable);
    action(DkActionManager::edit_rotate_180)->setEnabled(enable);
    action(DkActionManager::edit_delete)->setEnabled(enable);
    action(DkActionManager::edit_transform)->setEnabled(enable);
    action(DkActionManager::edit_crop)->setEnabled(enable);
    action(DkActionManager::edit_copy)->setEnabled(enable);
    action(DkActionManager::edit_copy_buffer)->setEnabled(enable);
    action(DkActionManager::edit_copy_color)->setEnabled(enable);
    action(DkActionManager::edit_undo)->setEnabled(enable);
    action(DkActionManager::edit_redo)->setEnabled(enable);

    action(DkActionManager::panel_info)->setEnabled(enable);
#ifdef WITH_OPENCV
    action(DkActionManager::panel_histogram)->setEnabled(enable);
#else
    action(DkActionManager::panel_histogram)->setEnabled(false);
#endif
    action(DkActionManager::panel_scroller)->setEnabled(enable);
    action(DkActionManager::panel_comment)->setEnabled(enable);
    action(DkActionManager::panel_preview)->setEnabled(enable);
    action(DkActionManager::panel_exif)->setEnabled(enable);
    action(DkActionManager::panel_overview)->setEnabled(enable);
    action(DkActionManager::panel_player)->setEnabled(enable);
    // action(DkActionManager::panel_thumbview)->setEnabled(enable);	// if we don't disable the thumbnail
    // preview, we can toggle it with SHIFT+T

    action(DkActionManager::view_slideshow)->setEnabled(enable);
    action(DkActionManager::view_fullscreen)->setEnabled(enable);
    action(DkActionManager::view_reset)->setEnabled(enable);
    action(DkActionManager::view_100)->setEnabled(enable);
    action(DkActionManager::view_fit_frame)->setEnabled(enable);
    action(DkActionManager::view_zoom_in)->setEnabled(enable);
    action(DkActionManager::view_zoom_out)->setEnabled(enable);
    action(DkActionManager::view_tp_pattern)->setEnabled(enable);
    action(DkActionManager::view_anti_aliasing)->setEnabled(enable);

    action(DkActionManager::tools_wallpaper)->setEnabled(enable);
    action(DkActionManager::tools_thumbs)->setEnabled(enable);

    action(DkActionManager::nav_skip_prev)->setEnabled(enable);
    action(DkActionManager::nav_skip_prev_sync)->setEnabled(enable);
    action(DkActionManager::nav_skip_next)->setEnabled(enable);
    action(DkActionManager::nav_skip_next_sync)->setEnabled(enable);
    action(DkActionManager::nav_first_file)->setEnabled(enable);
    action(DkActionManager::nav_first_file_sync)->setEnabled(enable);
    action(DkActionManager::nav_last_file)->setEnabled(enable);
    action(DkActionManager::nav_last_file_sync)->setEnabled(enable);

    // disable open with actions
    for (QAction *a : DkActionManager::instance().appManager()->getActions())
        a->setEnabled(enable);
}

void DkActionManager::enableMovieActions(bool enable) const
{
    DkSettingsManager::param().app().showMovieToolBar = enable;

    action(DkActionManager::view_movie_pause)->setEnabled(enable);
    action(DkActionManager::view_movie_prev)->setEnabled(enable);
    action(DkActionManager::view_movie_next)->setEnabled(enable);

    action(DkActionManager::view_movie_pause)->setChecked(false);

    DkToolBarManager::inst().showMovieToolBar(enable);
}

void DkActionManager::enableViewPortPluginActions(bool enable) const
{
    // actions to disable when a viewport plugin is activated,
    // and disable when de-activated

#ifdef WITH_PLUGINS
    // fix use-after-free since mPluginManger is owned by main window
    if (!mPluginManager) {
        return;
    }

    // opening another plugin is currently broken
    // fixme: for some reason these don't disable in the menu ?!
    for (auto *a : mPluginManager->pluginActions())
        a->setEnabled(enable);

    for (auto *a : mPluginManager->pluginDummyActions())
        a->setEnabled(enable);

    // this is a partial workaround; quick launch will continue to launch plugins
    mPluginManager->menu()->setEnabled(enable);
#endif

    constexpr EditAction disabledEditActions[] = {
        edit_undo, // plugins have their own undo/redo, do not interface directly with history/imageloader
        edit_redo,
        edit_copy, // plugins have their own copy/paste, should not copy from underlying imageloader
        edit_paste,
        edit_copy_buffer,
        edit_copy_color,
        edit_preferences, // tabs do not work with plugins
    };
    for (auto i : disabledEditActions)
        action(i)->setEnabled(enable);

    constexpr ViewAction disabledViewActions[] = {
        view_new_tab, // tabs
        view_close_tab,
        view_close_all_tabs,
        view_first_tab,
        view_previous_tab,
        view_goto_tab,
        view_next_tab,
        view_last_tab,
        view_frameless, // restarts nomacs
        view_slideshow,
    };
    for (auto i : disabledViewActions)
        action(i)->setEnabled(enable);

    action(tools_batch)->setEnabled(enable); // tabs
}
}
