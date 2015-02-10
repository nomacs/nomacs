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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QSettings>
#include <QBitArray>
#include <QColor>
#include <QDate>
#pragma warning(pop)	// no warnings from includes - end

#pragma warning(disable: 4251)	// TODO: remove

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

class QFileInfo;
class QTranslator;

namespace nmc {

class DkWhiteListViewModel;

class DkFileFilterHandling {

public:
	DkFileFilterHandling() {};
	void registerFileType(const QString& filterString, const QString& attribute, bool add);

protected:
	QString registerProgID(const QString& ext, const QString& friendlyName, bool add);
	void registerExtension(const QString& ext, const QString& progKey, bool add);
	void setAsDefaultApp(const QString& ext, const QString& progKey, bool defaultApp);
};

class DllExport Settings {

public:
	static Settings& instance();
	QSettings& getSettings();
	
private:
	Settings();

	QSharedPointer<QSettings> m_settings;
};

class DllExport DkSettings {

public:

	enum modes {
		mode_default = 0,
		mode_frameless,
		mode_contrast,
		mode_default_fullscreen,
		mode_frameless_fullscreen,
		mode_contrast_fullscreen,
		mode_end,
	};

	enum sortMode {
		sort_filename,
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

	enum syncModes {
		sync_mode_default = 0,
		sync_mode_remote_display,
		sync_mode_remote_control,
		sync_mode_receiveing_command,

		sync_mode_end,
	};

	struct App {
		bool showToolBar;
		bool showMenuBar;
		bool showStatusBar;
		QBitArray showFilePreview;
		QBitArray showFileInfoLabel;
		QBitArray showPlayer;
		QBitArray showMetaData;
		QBitArray showHistogram;
		QBitArray showOverview;
		QBitArray showScroller;
		QBitArray showComment;
		bool showRecentFiles;
		int appMode;
		int currentAppMode;
		bool privateMode;
		bool advancedSettings;
		bool closeOnEsc;
		QStringList browseFilters;
		QStringList registerFilters;

		QStringList fileFilters;	// just the filters
		QStringList openFilters;	// for open dialog
		QStringList saveFilters;	// for save dialog
		QStringList containerFilters;
		QString containerRawFilters;
	};

	struct Display {
		int keepZoom;
		bool invertZoom;
		bool tpPattern;
		QColor highlightColor;
		QColor bgColorWidget;
		QColor bgColor;
		QColor bgColorFrameless;
		QColor fontColor;
		QColor iconColor;
		bool useDefaultColor;
		bool defaultIconColor;
		int thumbSize;
		int thumbPreviewSize;
		//bool saveThumb;
		int interpolateZoomLevel;
		bool antiAliasing;
		bool smallIcons;
		bool toolbarGradient;
		bool showBorder;
		bool displaySquaredThumbs;
		bool showThumbLabel;
		float fadeSec;
	};

	struct Global {
		int skipImgs;
		int numFiles;
		bool loop;
		bool scanSubFolders;

		QString lastDir;
		QString lastSaveDir;
		QStringList recentFiles;
		QStringList recentFolders;
		bool logRecentFiles;
		bool useTmpPath;
		bool askToSaveDeletedFiles;
		QString tmpPath;
		QString language;
		QStringList searchHistory;
		
		Qt::KeyboardModifier altMod;
		Qt::KeyboardModifier ctrlMod;
		bool zoomOnWheel;

		QString setupPath;
		QString setupVersion;

		int sortMode;
		int sortDir;
		QString pluginsDir;
	};

	struct SlideShow {
		int filter;
		float time;
		bool silentFullscreen;
		QBitArray display;
		QColor backgroundColor;
		float moveSpeed;
	};
	struct Sync {
		bool enableNetworkSync;
		bool allowTransformation;
		bool allowPosition;
		bool allowFile;
		bool allowImage;
		bool checkForUpdates;
		bool updateDialogShown;
		QDate lastUpdateCheck;
		bool syncAbsoluteTransform;
		bool switchModifier;
		QStringList recentSyncNames;
		QStringList syncWhiteList;
		QHash<QString, QVariant> recentLastSeen;
		int syncMode;
		bool syncActions;
	};
	struct MetaData {
		QBitArray metaDataBits;
		bool ignoreExifOrientation;
		bool saveExifOrientation;
	};
		
	struct Resources {
		float cacheMemory;
		int maxImagesCached;
		bool waitForLastImg;
		bool filterRawImages;
		bool filterDuplicats;
		int loadRawThumb;
		QString preferredExtension;
		int numThumbsLoading;
		int maxThumbsLoading;
		bool gammaCorrection;
	};

	//enums for checkboxes - divide in camera data and description
	enum cameraData {
		camData_size,
		camData_orientation,
		camData_make,
		camData_model,
		camData_aperture,
		//camData_shutterspeed,
		camData_iso,
		camData_flash,
		camData_focallength,
		camData_exposuremode,
		camData_exposuretime,

		camData_end
	};

	enum descriptionT {
		desc_rating = camData_end,
		desc_usercomment,
		desc_date,
		desc_datetimeoriginal,
		desc_imagedescription,
		desc_creator,
		desc_creatortitle,
		desc_city,
		desc_country,
		desc_headline,
		desc_caption,
		desc_copyright,
		desc_keywords,
		desc_path,
		desc_filesize,

		desc_end
	};

	enum DisplayItems{
		display_file_name,
		display_creation_date,
		display_file_rating,

		display_end
	};

	static QStringList scamDataDesc;
	static QStringList sdescriptionDesc;

	static App& getAppSettings();
	static Display& getDisplaySettings();
	static Global& getGlobalSettings();
	static SlideShow& getSlideShowSettings();
	static Sync& getSyncSettings();
	static MetaData& getMetaDataSettings();
	static Resources& getResourceSettings();
	static void initFileFilters();
	static void loadTranslation(const QString& fileName, QTranslator& translator);
	static QStringList getTranslationDirs();

	static void load();
	static void save(bool force = false);
	static void setToDefaultSettings();

	static bool isPortable();
	static QFileInfo getSettingsFile();

	static App& app;
	static Global& global;
	static Display& display;
	static SlideShow& slideShow;
	static Sync& sync;
	static MetaData& metaData;
	static Resources& resources;

protected:
	static App app_p;
	static Global global_p;
	static Display display_p;
	static SlideShow slideShow_p;
	static Sync sync_p;
	static MetaData meta_p;
	static Resources resources_p;

	static App app_d;
	static Global global_d;
	static Display display_d;
	static SlideShow slideShow_d;
	static Sync sync_d;
	static MetaData meta_d;
	static Resources resources_d;
};

};
