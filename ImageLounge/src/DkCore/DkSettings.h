/*******************************************************************************************************
 DkSettings.h
 Created on:	07.07.2011

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

#pragma once

#pragma warning(push, 0) // no warnings from includes - begin
#include <QBitArray>
#include <QColor>
#include <QDate>
#include <QSettings>
#include <QSharedPointer>
#include <QVector>
#pragma warning(pop) // no warnings from includes - end

#pragma warning(disable : 4251) // TODO: remove

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

class QFileInfo;
class QTextStream;
class QTranslator;

namespace nmc
{

class DllCoreExport DefaultSettings : public QSettings
{
public:
    DefaultSettings();
};

class DllCoreExport DkFileFilterHandling
{
public:
    DkFileFilterHandling(){};
    void registerNomacs(bool showDefaultApps = false);
    void registerFileType(const QString &filterString, const QString &attribute, bool add);
    void showDefaultSoftware() const;

    static void registerFileAssociations();
    static void printFormats(QTextStream &out, const QString &outFormat);

protected:
    QString registerProgID(const QString &ext, const QString &friendlyName, bool add);
    void registerExtension(const QString &ext, const QString &progKey, bool add);
    void setAsDefaultApp(const QString &ext, const QString &progKey, bool defaultApp);
    void registerDefaultApp(const QString &ext, const QString &progKey, bool defaultApp);
    QString getIconID(const QString &ext) const;

    QStringList getExtensions(const QString &filter) const;
    QStringList getExtensions(const QString &filter, QString &friendlyName) const;
};

class DllCoreExport DkSettings
{
public:
    DkSettings();

    enum modes {
        mode_fullscreen_offset = 3,
        mode_default = 0,
        mode_frameless = 1,
        mode_contrast = 2,
        mode_default_fullscreen = mode_default + mode_fullscreen_offset,
        mode_frameless_fullscreen = mode_frameless + mode_fullscreen_offset,
        mode_contrast_fullscreen = mode_contrast + mode_fullscreen_offset,
        mode_end
    };

    static bool modeIsValid(int mode)
    {
        return mode >= mode_default && mode < mode_end;
    }

    static bool modeIsFullscreen(int mode)
    {
        return mode >= mode_fullscreen_offset;
    }

    static int normalMode(int mode)
    {
        return modeIsFullscreen(mode) ? mode - mode_fullscreen_offset : mode;
    }

    static int fullscreenMode(int mode)
    {
        return modeIsFullscreen(mode) ? mode : mode + mode_fullscreen_offset;
    }

    enum sortMode {
        sort_filename,
        sort_file_size,
        sort_date_created,
        sort_date_modified,
        sort_random,
        sort_end,
    };

    enum sortDir {
        sort_ascending,
        sort_descending,

        sort_dir_end,
    };

    enum rawThumb {
        raw_thumb_always,
        raw_thumb_if_large,
        raw_thumb_never,

        raw_thumb_end,
    };

    enum keepZoom {
        zoom_always_keep,
        zoom_keep_same_size,
        zoom_never_keep,

        zoom_end,
    };

    enum TransitionMode {
        trans_appear,
        trans_fade,
        trans_swipe,

        trans_end
    };

    enum LoadSaveMode {
        ls_load,
        ls_load_to_tab,
        ls_do_nothing,

        ls,
        end
    };

    struct App {
        bool showToolBar;
        bool showMenuBar;
        bool showStatusBar;
        bool showMovieToolBar;
        QBitArray showFilePreview;
        QBitArray showFileInfoLabel;
        QBitArray showPlayer;
        QBitArray showMetaData;
        QBitArray showHistogram;
        QBitArray showOverview;
        QBitArray showScroller;
        QBitArray showComment;
        QBitArray showExplorer;
        QBitArray showMetaDataDock;
        QBitArray showEditDock;
        QBitArray showHistoryDock;
        QBitArray showLogDock;
        bool showRecentFiles;
        bool useLogFile;
        int appMode;
        int currentAppMode;
        bool privateMode;
        bool advancedSettings;
        bool closeOnEsc;
        bool closeOnMiddleMouse;
        bool hideAllPanels;

        int defaultJpgQuality;

        QStringList browseFilters;
        QStringList registerFilters;

        QStringList fileFilters; // just the filters
        QStringList openFilters; // for open dialog
        QStringList saveFilters; // for save dialog
        QStringList rawFilters;
        QStringList containerFilters;
        QString containerRawFilters;
    };

    struct Display {
        int keepZoom;
        bool zoomToFit;
        bool invertZoom;
        bool tpPattern;
        bool showNavigation;
        QString themeName;
        QString stylePlugin;
        QColor highlightColor;
        QColor hudBgColor;
        QColor bgColor;
        QColor bgColorFrameless;
        QColor hudFgdColor;
        QColor iconColor;
        QColor fgColor;

        // theme colors, cannot be modified by user; only valid after applyTheme()
        QColor themeFgdColor;
        QColor themeBgdColor;
        QColor themeIconColor;

        // if false, the user has customized a color in prefs and we do not use the theme color
        bool defaultForegroundColor;
        bool defaultBackgroundColor;
        bool defaultIconColor;

        int thumbSize;
        int iconSize;
        int thumbPreviewSize;
        // bool saveThumb;
        int interpolateZoomLevel;
        bool showCrop;
        bool antiAliasing;
        bool highQualityAntiAliasing;
        bool showBorder;
        bool displaySquaredThumbs;
        bool showThumbLabel;
        bool showScrollBars;

        TransitionMode transition;
        bool alwaysAnimate;
        float animationDuration;

        int histogramStyle;
        bool animateWidgets; // animate hide/show of widgets/panels/etc
    };

    struct Global {
        int skipImgs;
        int numFiles;
        bool loop;
        bool scanSubFolders;

        QString lastSaveDir;
        QStringList recentFiles;
        QStringList recentFolders;
        QStringList pinnedFiles;
        bool logRecentFiles;
        bool checkOpenDuplicates;
        bool openDirShowFirstImage; // show first image when opening dir, otherwise show thumbs
        bool extendedTabs;
        bool askToSaveDeletedFiles;
        QString tmpPath;
        QString language;
        QStringList searchHistory;

        Qt::KeyboardModifier altMod;
        Qt::KeyboardModifier ctrlMod;
        bool zoomOnWheel;
        bool horZoomSkips;
        bool doubleClickForFullscreen;
        bool showBgImage;

        QString setupPath;
        QString setupVersion;
        int numThreads;

        int sortMode;
        int sortDir;
        quint32 sortSeed;
        QString pluginsDir;
    };

    struct SlideShow {
        int filter;
        float time;
        bool showPlayer;
        bool silentFullscreen;
        QBitArray display;
        QColor backgroundColor;
        float moveSpeed;
    };
    struct Sync {
        bool checkForUpdates;
        bool updateDialogShown;
        bool disableUpdateInteraction;
        QDate lastUpdateCheck;
        bool syncAbsoluteTransform;
        bool switchModifier;
        bool syncActions;
    };
    struct MetaData {
        bool ignoreExifOrientation;
        bool saveExifOrientation;
    };

    struct Resources {
        float cacheMemory;
        float historyMemory;
        bool nativeDialog;
        int maxImagesCached;
        bool waitForLastImg;
        bool filterRawImages;
        bool filterDuplicats;
        int loadRawThumb;
        QString preferredExtension;
        bool gammaCorrection;
        int loadSavedImage;
    };

    enum DisplayItems {
        display_file_name,
        display_creation_date,
        display_file_rating,

        display_end
    };

    QStringList translatedCamData() const;
    QStringList translatedDescriptionData() const;

    void initFileFilters();
    void loadTranslation(const QString &fileName, QTranslator &translator);
    QStringList getTranslationDirs();

    void load();
    void load(QSettings &settings, bool defaults = false);
    void save(bool force = false);
    void save(QSettings &settings, bool force = false);
    void setToDefaultSettings();
    void setNumThreads(int numThreads);

    bool isPortable();
    QString settingsPath() const;

    double dpiScaleFactor(QWidget *w = 0) const;
    int effectiveIconSize(QWidget *w = 0) const;
    int effectiveThumbSize(QWidget *w = 0) const;
    int effectiveThumbPreviewSize(QWidget *w = 0) const;

    App &app();
    Global &global();
    Display &display();
    SlideShow &slideShow();
    Sync &sync();
    MetaData &metaData();
    Resources &resources();

    const Display &defaultDisplay() const
    {
        return display_default;
    }

protected:
    QStringList scamDataDesc;
    QStringList sdescriptionDesc;

    // current settings
    App app_p;
    Global global_p;
    Display display_p;
    SlideShow slideShow_p;
    Sync sync_p;
    MetaData meta_p;
    Resources resources_p;

    // saved settings
    App app_d;
    Global global_d;
    Display display_d;
    SlideShow slideShow_d;
    Sync sync_d;
    MetaData meta_d;
    Resources resources_d;

    // app defaults
    Display display_default;

    void init();

    void applyDefaultsFromFile();
    void copySettings(const QSettings &src, QSettings &dst) const;
    QString getDefaultSettingsFile() const;

    QString mSettingsPath;
};

class DllCoreExport DkSettingsManager
{
public:
    static DkSettingsManager &instance();
    ~DkSettingsManager();

    // singleton
    DkSettingsManager(DkSettingsManager const &) = delete;
    void operator=(DkSettingsManager const &) = delete;

    static DkSettings &param(); // convenience
    // QSettings& qSettings();
    DkSettings &settings(); // rename
    void init();

    static void importSettings(const QString &settingsPath);

private:
    DkSettingsManager();

    // QSettings* mSettings = 0;
    DkSettings *mParams = 0;
};

class DkZoomConfig
{
public:
    static DkZoomConfig &instance();
    ~DkZoomConfig();

    // singleton
    DkZoomConfig(DkZoomConfig const &) = delete;
    void operator=(DkZoomConfig const &) = delete;

    double nextFactor(double currentFactor, double delta) const;
    QVector<double> defaultLevels() const;

    bool useLevels() const;
    void setUseLevels(bool useLevels);

    bool setLevels(const QString &levelStr);
    QString levelsToString() const;

    void setLevelsToDefault();

    static bool checkLevels(const QVector<double> &levels);

private:
    QVector<double> mLevels;
    bool mUseLevels = false;

    DkZoomConfig();
    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings) const;
};

}
