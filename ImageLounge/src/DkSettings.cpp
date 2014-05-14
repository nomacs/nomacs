/*******************************************************************************************************
 DkSettings.cpp
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

#include "DkSettings.h"
//#include "DkWidgets.h"
#include "DkDialog.h"
#include "DkUtils.h"

namespace nmc {

QStringList DkSettings::scamDataDesc = QStringList() << 
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Image Size") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Orientation") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Make") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Model") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Aperture Value") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","ISO") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Flash") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Focal Length") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Exposure Mode") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Exposure Time");

QStringList DkSettings::sdescriptionDesc = QStringList() <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Rating") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","User Comment") << 
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Date Time") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Date Time Original") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Image Description") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Creator") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Creator Title") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","City") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Country") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Headline") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Caption") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Copyright") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Keywords") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","Path") <<
												QT_TRANSLATE_NOOP("nmc::DkMetaData","File Size");


// settings
DkSettings::App DkSettings::app_p;
DkSettings::Display DkSettings::display_p;
DkSettings::Global DkSettings::global_p;
DkSettings::SlideShow DkSettings::slideShow_p;
DkSettings::Sync DkSettings::sync_p;
DkSettings::MetaData DkSettings::meta_p;
DkSettings::Resources DkSettings::resources_p;

// load settings
DkSettings::App DkSettings::app_d;
DkSettings::Display DkSettings::display_d;
DkSettings::Global DkSettings::global_d;
DkSettings::SlideShow DkSettings::slideShow_d;
DkSettings::Sync DkSettings::sync_d;
DkSettings::MetaData DkSettings::meta_d;
DkSettings::Resources DkSettings::resources_d;

// well this is pretty shitty... but we need the filter without description too
QStringList DkSettings::fileFilters = QStringList();

// formats we can save
QStringList DkSettings::saveFilters = QStringList();

// formats we can load
QStringList DkSettings::openFilters = QStringList();
QStringList DkSettings::browseFilters = QStringList();
QStringList DkSettings::registerFilters = QStringList();

DkSettings::App& DkSettings::app = DkSettings::getAppSettings();
DkSettings::Display& DkSettings::display = DkSettings::getDisplaySettings();
DkSettings::Global& DkSettings::global = DkSettings::getGlobalSettings();
DkSettings::SlideShow& DkSettings::slideShow = DkSettings::getSlideShowSettings();
DkSettings::Sync& DkSettings::sync = DkSettings::getSyncSettings();
DkSettings::MetaData& DkSettings::metaData = DkSettings::getMetaDataSettings();
DkSettings::Resources& DkSettings::resources = DkSettings::getResourceSettings();


DkSettings::App& DkSettings::getAppSettings() {
	load();
	return app_p;
}

DkSettings::Display& DkSettings::getDisplaySettings() {
	return display_p;
}

DkSettings::Global& DkSettings::getGlobalSettings() {
	return global_p;
}

DkSettings::SlideShow& DkSettings::getSlideShowSettings() {
	return slideShow_p;
}

DkSettings::Sync& DkSettings::getSyncSettings() {
	return sync_p;
}

DkSettings::MetaData& DkSettings::getMetaDataSettings() {
	return meta_p;
}

DkSettings::Resources& DkSettings::getResourceSettings() {
	return resources_p;
}

void DkSettings::initFileFilters() {

	if (!openFilters.empty())
		return;

	QList<QByteArray> qtFormats = QImageReader::supportedImageFormats();

	// formats we can save
	if (qtFormats.contains("png"))		saveFilters.append("PNG (*.png)");
	if (qtFormats.contains("jpg"))		saveFilters.append("JPEG (*.jpg *.jpeg)");
	if (qtFormats.contains("j2k"))		saveFilters.append("JPEG 2000 (*.jp2 *.j2k *.jpf *.jpx *.jpm *.jpgx)");
	if (qtFormats.contains("tif"))		saveFilters.append("TIFF (*.tif *.tiff)");
	if (qtFormats.contains("bmp"))		saveFilters.append("Windows Bitmap (*.bmp)");
	if (qtFormats.contains("ppm"))		saveFilters.append("Portable Pixmap (*.ppm)");
	if (qtFormats.contains("xbm"))		saveFilters.append("X11 Bitmap (*.xbm)");
	if (qtFormats.contains("xpm"))		saveFilters.append("X11 Pixmap (*.xpm)");

	// internal filters
#ifdef WITH_WEBP
	saveFilters.append("WebP (*.webp)");
#endif

	// formats we can load
	openFilters += saveFilters;
	if (qtFormats.contains("gif"))		openFilters.append("Graphic Interchange Format (*.gif)");
	if (qtFormats.contains("pbm"))		openFilters.append("Portable Bitmap (*.pbm)");
	if (qtFormats.contains("pgm"))		openFilters.append("Portable Graymap (*.pgm)");
	if (qtFormats.contains("ico"))		openFilters.append("Icon Files (*.ico)");
	if (qtFormats.contains("tga"))		openFilters.append("Targa Image File (*.tga)");
	if (qtFormats.contains("mng"))		openFilters.append("Multi-Image Network Graphics (*.mng)");

#ifdef WITH_OPENCV
	// raw format
	openFilters.append("Nikon Raw (*.nef)");
	openFilters.append("Canon Raw (*.crw *.cr2)");
	openFilters.append("Sony Raw (*.arw)");
	openFilters.append("Digital Negativ (*.dng)");
	openFilters.append("Panasonic Raw (*.rw2)");
	openFilters.append("Minolta Raw (*.mrw)");
#endif

	// stereo formats
	openFilters.append("JPEG Stereo (*.jps)");
	openFilters.append("PNG Stereo (*.pns)");
	openFilters.append("Multi Picture Object (*.mpo)");

	// other formats
	openFilters.append("Adobe Photoshop (*.psd)");
	openFilters.append("Large Document Format (*.psb)");
	openFilters.append("Rohkost (*.roh)");

	// load user filters
	QSettings settings;
	openFilters += settings.value("ResourceSettings/userFilters", QStringList()).toStringList();

	for (int idx = 0; idx < openFilters.size(); idx++) {

		QString cFilter = openFilters[idx];
		cFilter = cFilter.section(QRegExp("(\\(|\\))"), 1);
		cFilter = cFilter.replace(")", "");
		fileFilters += cFilter.split(" ");
	}

	QString allFilters = fileFilters.join(" ");

	// add unknown formats from Qt plugins
	for (int idx = 0; idx < qtFormats.size(); idx++) {

		if (!allFilters.contains(qtFormats.at(idx))) {
			openFilters.append("Image Format (*." + qtFormats.at(idx) + ")");
			fileFilters.append("*." + qtFormats.at(idx));
		}
	}

	openFilters.prepend("Image Files (" + fileFilters.join(" ") + ")");

	qDebug() << "supported: " << qtFormats;

#ifdef Q_OS_WIN
	fileFilters.append("*.lnk");
#endif

}

void DkSettings::load(bool force) {

	setToDefaultSettings();

	QSettings settings;
	settings.beginGroup("AppSettings");
	
	app_p.showMenuBar = settings.value("showMenuBar", app_p.showMenuBar).toBool();
	app_p.showToolBar = settings.value("showToolBar", app_p.showToolBar).toBool();
	app_p.showStatusBar = settings.value("showStatusBar", app_p.showStatusBar).toBool();
	
	QBitArray tmpShow = settings.value("showFileInfoLabel", app_p.showFileInfoLabel).toBitArray();
	if (tmpShow.size() == app_p.showFileInfoLabel.size())	
		app_p.showFileInfoLabel = tmpShow;
	tmpShow = settings.value("showScroller", app_p.showScroller).toBitArray();
	if (tmpShow.size() == app_p.showScroller.size())	
		app_p.showScroller = tmpShow;
	tmpShow = settings.value("showFilePreview", app_p.showFilePreview).toBitArray();
	if (tmpShow.size() == app_p.showFilePreview.size())	
		app_p.showFilePreview = tmpShow;
	tmpShow = settings.value("showMetaData", app_p.showMetaData).toBitArray();
	if (tmpShow.size() == app_p.showMetaData.size())	
		app_p.showMetaData = tmpShow;
	tmpShow = settings.value("showPlayer", app_p.showPlayer).toBitArray();
	if (tmpShow.size() == app_p.showPlayer.size())	
		app_p.showPlayer = tmpShow;
	tmpShow = settings.value("showHistogram", app_p.showHistogram).toBitArray();
	if (tmpShow.size() == app_p.showHistogram.size())	
		app_p.showHistogram = tmpShow;
	tmpShow = settings.value("showOverview", app_p.showOverview).toBitArray();
	if (tmpShow.size() == app_p.showOverview.size())	
		app_p.showOverview = tmpShow;

	app_p.closeOnEsc = settings.value("closeOnEsc", app_p.closeOnEsc).toBool();
	app_p.advancedSettings = settings.value("advancedSettings", app_p.advancedSettings).toBool();

	settings.endGroup();
	// Global Settings --------------------------------------------------------------------
	settings.beginGroup("GlobalSettings");

	global_p.skipImgs = settings.value("skipImgs", global_p.skipImgs).toInt();
	global_p.numFiles = settings.value("numFiles", global_p.numFiles).toInt();

	global_p.loop = settings.value("loop", global_p.loop).toBool();
	global_p.scanSubFolders = settings.value("scanRecursive", global_p.scanSubFolders).toBool();
	global_p.lastDir = settings.value("lastDir", global_p.lastDir).toString();
	global_p.searchHistory = settings.value("searchHistory", global_p.searchHistory).toStringList();
	global_p.recentFolders = settings.value("recentFolders", global_p.recentFolders).toStringList();
	global_p.recentFiles = settings.value("recentFiles", global_p.recentFiles).toStringList();
	global_p.useTmpPath= settings.value("useTmpPath", global_p.useTmpPath).toBool();
	global_p.tmpPath = settings.value("tmpPath", global_p.tmpPath).toString();
	global_p.language = settings.value("language", global_p.language).toString();

	global_p.sortMode = settings.value("sortMode", global_p.sortMode).toInt();
	global_p.sortDir = settings.value("sortDir", global_p.sortDir).toInt();
	global_p.setupPath = settings.value("setupPath", global_p.setupPath).toString();
	global_p.setupVersion = settings.value("setupVersion", global_p.setupVersion).toString();
	global_p.zoomOnWheel = settings.value("zoomOnWheel", global_p.zoomOnWheel).toBool();

	settings.endGroup();
	// Display Settings --------------------------------------------------------------------
	settings.beginGroup("DisplaySettings");

	display_p.keepZoom = settings.value("keepZoom", display_p.keepZoom).toInt();
	display_p.invertZoom = settings.value("invertZoom", display_p.invertZoom).toBool();
	display_p.highlightColor = settings.value("highlightColor", display_p.highlightColor).value<QColor>();
	display_p.bgColorWidget = settings.value("bgColor", display_p.bgColorWidget).value<QColor>();
	display_p.bgColor = settings.value("bgColorNoMacs", display_p.bgColor).value<QColor>();
	display_p.iconColor = settings.value("iconColor", display_p.iconColor).value<QColor>();

	display_p.bgColorFrameless = settings.value("bgColorFrameless", display_p.bgColorFrameless).value<QColor>();
	display_p.thumbSize = settings.value("thumbSize", display_p.thumbSize).toInt();
	display_p.thumbPreviewSize = settings.value("thumbPreviewSize", display_p.thumbPreviewSize).toInt();
	display_p.saveThumb = settings.value("saveThumb", display_p.saveThumb).toBool();
	display_p.antiAliasing = settings.value("antiAliasing", display_p.antiAliasing).toBool();
	display_p.tpPattern = settings.value("tpPattern", display_p.tpPattern).toBool();
	display_p.smallIcons = settings.value("smallIcons", display_p.smallIcons).toBool();
	display_p.toolbarGradient = settings.value("toolbarGradient", display_p.toolbarGradient).toBool();
	display_p.showBorder = settings.value("showBorder", display_p.showBorder).toBool();
	display_p.displaySquaredThumbs = settings.value("displaySquaredThumbs", display_p.displaySquaredThumbs).toBool();
	display_p.fadeSec = settings.value("fadeSec", display_p.fadeSec).toFloat();
	display_p.useDefaultColor = settings.value("useDefaultColor", display_p.useDefaultColor).toBool();
	display_p.defaultIconColor = settings.value("defaultIconColor", display_p.defaultIconColor).toBool();
	display_p.interpolateZoomLevel = settings.value("interpolateZoomlevel", display_p.interpolateZoomLevel).toInt();

	settings.endGroup();
	// MetaData Settings --------------------------------------------------------------------
	settings.beginGroup("MetaDataSettings");

	QBitArray tmpMetaData = settings.value("metaData", meta_p.metaDataBits).toBitArray();
	if (tmpMetaData.size() == meta_p.metaDataBits.size())
		meta_p.metaDataBits = tmpMetaData;
	
	meta_p.ignoreExifOrientation = settings.value("ignoreExifOrientation", meta_p.ignoreExifOrientation).toBool();
	meta_p.saveExifOrientation = settings.value("saveExifOrientation", meta_p.saveExifOrientation).toBool();

	settings.endGroup();
	// SlideShow Settings --------------------------------------------------------------------
	settings.beginGroup("SlideShowSettings");

	slideShow_p.filter = settings.value("filter", slideShow_p.filter).toInt();
	slideShow_p.time = settings.value("time", slideShow_p.time).toFloat();
	slideShow_p.backgroundColor = settings.value("backgroundColor", slideShow_p.backgroundColor).value<QColor>();
	slideShow_p.silentFullscreen = settings.value("silentFullscreen", slideShow_p.silentFullscreen).toBool();
	QBitArray tmpDisplay = settings.value("display", slideShow_p.display).toBitArray();

	if (tmpDisplay.size() == slideShow_p.display.size())
		slideShow_p.display = tmpDisplay;

	settings.endGroup();
	// Synchronize Settings --------------------------------------------------------------------
	settings.beginGroup("SynchronizeSettings");

	sync_p.enableNetworkSync= settings.value("enableNetworkSync", sync_p.enableNetworkSync).toBool();
	sync_p.allowTransformation = settings.value("allowTransformation", sync_p.allowTransformation).toBool();
	sync_p.allowPosition = settings.value("allowPosition", sync_p.allowPosition).toBool();
	sync_p.allowFile = settings.value("allowFile", sync_p.allowFile).toBool();
	sync_p.allowImage = settings.value("allowImage", sync_p.allowImage).toBool();;
	sync_p.updateDialogShown = settings.value("updateDialogShown", sync_p.updateDialogShown).toBool();
	sync_p.lastUpdateCheck = settings.value("lastUpdateCheck", sync_p.lastUpdateCheck).toDate();
	sync_p.syncAbsoluteTransform = settings.value("syncAbsoluteTransform", sync_p.syncAbsoluteTransform).toBool();
	sync_p.switchModifier = settings.value("switchModifier", sync_p.switchModifier).toBool();
	sync_p.syncMode = settings.value("syncMode", sync_p.syncMode).toInt();
	sync_p.recentSyncNames = settings.value("recentSyncNames", sync_p.recentSyncNames).toStringList();
	sync_p.syncWhiteList = settings.value("syncWhiteList", sync_p.syncWhiteList).toStringList();
	sync_p.recentLastSeen = settings.value("recentLastSeen", sync_p.recentLastSeen).toHash();

	settings.endGroup();
	// Resource Settings --------------------------------------------------------------------
	settings.beginGroup("ResourceSettings");

	resources_p.cacheMemory = settings.value("cacheMemory", resources_p.cacheMemory).toFloat();
	resources_p.maxImagesCached = settings.value("maxImagesCached", resources_p.maxImagesCached).toFloat();
	resources_p.fastThumbnailPreview = settings.value("fastThumbnailPreview", resources_p.fastThumbnailPreview).toBool();
	resources_p.waitForLastImg = settings.value("waitForLastImg", resources_p.waitForLastImg).toBool();
	resources_p.filterRawImages = settings.value("filterRawImages", resources_p.filterRawImages).toBool();	
	resources_p.loadRawThumb = settings.value("loadRawThumb", resources_p.loadRawThumb).toInt();	
	resources_p.filterDuplicats = settings.value("filterDuplicates", resources_p.filterDuplicats).toBool();
	resources_p.preferredExtension = settings.value("preferredExtension", resources_p.preferredExtension).toString();	
	resources_p.gammaCorrection = settings.value("gammaCorrection", resources_p.gammaCorrection).toBool();

	if (sync_p.switchModifier) {
		global_p.altMod = Qt::ControlModifier;
		global_p.ctrlMod = Qt::AltModifier;
	}
	else {
		global_p.altMod = Qt::AltModifier;
		global_p.ctrlMod = Qt::ControlModifier;
	}


	// keep loaded settings in mind
	app_d = app_p;
	global_d = global_p;
	display_d = display_p;
	slideShow_d = slideShow_p;
	sync_d = sync_p;
	meta_d = meta_p;
	resources_d = resources_p;

}

void DkSettings::save(bool force) {
		
	QSettings settings;
	
	settings.beginGroup("AppSettings");

	if (!force && app_p.showMenuBar != app_d.showMenuBar)
		settings.setValue("showMenuBar", app_p.showMenuBar);

	int myAppMode = app_p.appMode;
	if (app_p.currentAppMode != mode_frameless && app_p.currentAppMode != mode_frameless_fullscreen) {
		
		if (!force && app_p.showToolBar != app_d.showToolBar)
			settings.setValue("showToolBar", app_p.showToolBar);

		if (!force && app_p.showStatusBar != app_d.showStatusBar)
			settings.setValue("showStatusBar", app_p.showStatusBar);
	}

	if (!force && app_p.showFileInfoLabel != app_d.showFileInfoLabel)
		settings.setValue("showFileInfoLabel", app_p.showFileInfoLabel);
	if (!force && app_p.showFilePreview != app_d.showFilePreview)
		settings.setValue("showFilePreview", app_p.showFilePreview);
	if (!force && app_p.showScroller != app_d.showScroller)
		settings.setValue("showScroller", app_p.showScroller);
	if (!force && app_p.showMetaData != app_d.showMetaData)
		settings.setValue("showMetaData", app_p.showMetaData);
	if (!force && app_p.showPlayer != app_d.showPlayer)
		settings.setValue("showPlayer", app_p.showPlayer);
	if (!force && app_p.showHistogram != app_d.showHistogram)
		settings.setValue("showHistogram", app_p.showHistogram);
	if (!force && app_p.showOverview != app_d.showOverview)
		settings.setValue("showOverview", app_p.showOverview);
	if (!force && app_p.advancedSettings != app_d.advancedSettings)
		settings.setValue("advancedSettings", app_p.advancedSettings);
	//if (!force && app_p.appMode != app_d.appMode)
		settings.setValue("appMode", app_p.appMode);
	//if (!force && app_p.currentAppMode != app_d.currentAppMode)
		settings.setValue("currentAppMode", app_p.currentAppMode);
	if (!force && app_p.closeOnEsc != app_d.closeOnEsc)
		settings.setValue("closeOnEsc", app_p.closeOnEsc);

	settings.endGroup();
	// Global Settings --------------------------------------------------------------------
	settings.beginGroup("GlobalSettings");

	if (!force && global_p.skipImgs != global_d.skipImgs)
		settings.setValue("skipImgs",global_p.skipImgs);
	if (!force && global_p.numFiles != global_d.numFiles)
		settings.setValue("numFiles",global_p.numFiles);
	if (!force && global_p.loop != global_d.loop)
		settings.setValue("loop",global_p.loop);
	if (!force && global_p.scanSubFolders != global_d.scanSubFolders)
		settings.setValue("scanRecursive",global_p.scanSubFolders);
	if (!force && global_p.lastDir != global_d.lastDir)
		settings.setValue("lastDir", global_p.lastDir);
	if (!force && global_p.searchHistory != global_d.searchHistory)
		settings.setValue("searchHistory", global_p.searchHistory);
	if (!force && global_p.recentFolders != global_d.recentFolders)
		settings.setValue("recentFolders", global_p.recentFolders);
	if (!force && global_p.recentFiles != global_d.recentFiles)
		settings.setValue("recentFiles", global_p.recentFiles);
	if (!force && global_p.useTmpPath != global_d.useTmpPath)
		settings.setValue("useTmpPath", global_p.useTmpPath);
	if (!force && global_p.tmpPath != global_d.tmpPath)
		settings.setValue("tmpPath", global_p.tmpPath);
	if (!force && global_p.language != global_d.language)
		settings.setValue("language", global_p.language);
	if (!force && global_p.sortMode != global_d.sortMode)
		settings.setValue("sortMode", global_p.sortMode);
	if (!force && global_p.sortDir != global_d.sortDir)
		settings.setValue("sortDir", global_p.sortDir);
	if (!force && global_p.setupPath != global_d.setupPath)
		settings.setValue("setupPath", global_p.setupPath);
	if (!force && global_p.setupVersion != global_d.setupVersion)
		settings.setValue("setupVersion", global_p.setupVersion);
	if (!force && global_p.zoomOnWheel != global_d.zoomOnWheel)
		settings.setValue("zoomOnWheel", global_p.zoomOnWheel);

	settings.endGroup();
	// Display Settings --------------------------------------------------------------------
	settings.beginGroup("DisplaySettings");

	if (!force && display_p.keepZoom != display_d.keepZoom)
		settings.setValue("keepZoom",display_p.keepZoom);
	if (!force && display_p.invertZoom != display_d.invertZoom)
		settings.setValue("invertZoom",display_p.invertZoom);
	if (!force && display_p.highlightColor != display_d.highlightColor)
		settings.setValue("highlightColor", display_p.highlightColor);
	if (!force && display_p.bgColorWidget != display_d.bgColorWidget)
		settings.setValue("bgColor", display_p.bgColorWidget);
	if (!force && display_p.bgColor != display_d.bgColor)
		settings.setValue("bgColorNoMacs", display_p.bgColor);
	if (!force && display_p.iconColor != display_d.iconColor)
		settings.setValue("iconColor", display_p.iconColor);
	if (!force && display_p.bgColorFrameless != display_d.bgColorFrameless)
		settings.setValue("bgColorFrameless", display_p.bgColorFrameless);
	if (!force && display_p.thumbSize != display_d.thumbSize)
		settings.setValue("thumbSize", display_p.thumbSize);
	if (!force && display_p.thumbPreviewSize != display_d.thumbPreviewSize)
		settings.setValue("thumbPreviewSize", display_p.thumbPreviewSize);
	if (!force && display_p.saveThumb != display_d.saveThumb)
		settings.setValue("saveThumb", display_p.saveThumb);
	if (!force && display_p.antiAliasing != display_d.antiAliasing)
		settings.setValue("antiAliasing", display_p.antiAliasing);
	if (!force && display_p.tpPattern != display_d.tpPattern)
		settings.setValue("tpPattern", display_p.tpPattern);
	if (!force && display_p.smallIcons != display_d.smallIcons)
		settings.setValue("smallIcons", display_p.smallIcons);
	if (!force && display_p.toolbarGradient != display_d.toolbarGradient)
		settings.setValue("toolbarGradient", display_p.toolbarGradient);
	if (!force && display_p.showBorder != display_d.showBorder)
		settings.setValue("showBorder", display_p.showBorder);
	if (!force && display_p.displaySquaredThumbs != display_d.displaySquaredThumbs)
		settings.setValue("displaySquaredThumbs", display_p.displaySquaredThumbs);
	if (!force && display_p.fadeSec != display_d.fadeSec)
		settings.setValue("fadeSec", display_p.fadeSec);
	if (!force && display_p.useDefaultColor != display_d.useDefaultColor)
		settings.setValue("useDefaultColor", display_p.useDefaultColor);
	if (!force && display_p.defaultIconColor != display_d.defaultIconColor)
		settings.setValue("defaultIconColor", display_p.defaultIconColor);
	if (!force && display_p.interpolateZoomLevel != display_d.interpolateZoomLevel)
		settings.setValue("interpolateZoomlevel", display_p.interpolateZoomLevel);

	settings.endGroup();
	// MetaData Settings --------------------------------------------------------------------
	settings.beginGroup("MetaDataSettings");
	
	if (!force && meta_p.metaDataBits != meta_d.metaDataBits)
		settings.setValue("metaData", meta_p.metaDataBits);
	if (!force && meta_p.ignoreExifOrientation != meta_d.ignoreExifOrientation)
		settings.setValue("ignoreExifOrientation", meta_p.ignoreExifOrientation);
	if (!force && meta_p.saveExifOrientation != meta_d.saveExifOrientation)
		settings.setValue("saveExifOrientation", meta_p.saveExifOrientation);

	settings.endGroup();
	// SlideShow Settings --------------------------------------------------------------------
	settings.beginGroup("SlideShowSettings");

	if (!force && slideShow_p.filter != slideShow_d.filter)
		settings.setValue("filter", slideShow_p.filter);
	if (!force && slideShow_p.time != slideShow_d.time)
		settings.setValue("time", slideShow_p.time);
	if (!force && slideShow_p.display != slideShow_d.display)
		settings.setValue("display", slideShow_p.display);
	if (!force && slideShow_p.backgroundColor != slideShow_d.backgroundColor)
		settings.setValue("backgroundColor", slideShow_p.backgroundColor);
	if (!force && slideShow_p.silentFullscreen != slideShow_d.silentFullscreen)
		settings.setValue("silentFullscreen", slideShow_p.silentFullscreen);

	settings.endGroup();
	// Sync Settings --------------------------------------------------------------------
	settings.beginGroup("SynchronizeSettings");

	if (!force && sync_p.enableNetworkSync != sync_d.enableNetworkSync)
		settings.setValue("enableNetworkSync", sync_p.enableNetworkSync);
	if (!force && sync_p.allowTransformation != sync_d.allowTransformation)
		settings.setValue("allowTransformation", sync_p.allowTransformation);
	if (!force && sync_p.allowPosition != sync_d.allowPosition)
		settings.setValue("allowPosition", sync_p.allowPosition);
	if (!force && sync_p.allowFile != sync_d.allowFile)
		settings.setValue("allowFile", sync_p.allowFile);
	if (!force && sync_p.allowImage != sync_d.allowImage)
		settings.setValue("allowImage", sync_p.allowImage);
	if (!force && sync_p.updateDialogShown != sync_d.updateDialogShown)
		settings.setValue("updateDialogShown", sync_p.updateDialogShown);
	if (!force && sync_p.lastUpdateCheck != sync_d.lastUpdateCheck)
		settings.setValue("lastUpdateCheck", sync_p.lastUpdateCheck);
	if (!force && sync_p.syncAbsoluteTransform != sync_d.syncAbsoluteTransform)
		settings.setValue("syncAbsoluteTransform", sync_p.syncAbsoluteTransform);
	if (!force && sync_p.switchModifier != sync_d.switchModifier)
		settings.setValue("switchModifier", sync_p.switchModifier);
	if (!force && sync_p.syncMode != sync_d.syncMode)
		settings.setValue("syncMode", sync_p.syncMode);
	if (!force && sync_p.recentSyncNames != sync_d.recentSyncNames)
		settings.setValue("recentSyncNames", sync_p.recentSyncNames);
	if (!force && sync_p.syncWhiteList != sync_d.syncWhiteList)
		settings.setValue("syncWhiteList", sync_p.syncWhiteList);
	if (!force && sync_p.recentLastSeen != sync_d.recentLastSeen)
		settings.setValue("recentLastSeen", sync_p.recentLastSeen);

	settings.endGroup();
	// Resource Settings --------------------------------------------------------------------
	settings.beginGroup("ResourceSettings");

	if (!force && resources_p.cacheMemory != resources_d.cacheMemory)
		settings.setValue("cacheMemory", resources_p.cacheMemory);
	if (!force && resources_p.maxImagesCached != resources_d.maxImagesCached)
		settings.setValue("maxImagesCached", resources_p.maxImagesCached);
	if (!force && resources_p.fastThumbnailPreview != resources_d.fastThumbnailPreview)
		settings.setValue("fastThumbnailPreview", resources_p.fastThumbnailPreview);
	if (!force && resources_p.waitForLastImg != resources_d.waitForLastImg)
		settings.setValue("waitForLastImg", resources_p.waitForLastImg);
	if (!force && resources_p.filterRawImages != resources_d.filterRawImages)
		settings.setValue("filterRawImages", resources_p.filterRawImages);
	if (!force && resources_p.loadRawThumb != resources_d.loadRawThumb)
		settings.setValue("loadRawThumb", resources_p.loadRawThumb);
	if (!force && resources_p.filterDuplicats != resources_d.filterDuplicats)
		settings.setValue("filterDuplicates", resources_p.filterDuplicats);
	if (!force && resources_p.preferredExtension != resources_d.preferredExtension)
		settings.setValue("preferredExtension", resources_p.preferredExtension);
	if (!force && resources_p.gammaCorrection != resources_d.gammaCorrection)
		settings.setValue("gammaCorrection", resources_p.gammaCorrection);

	// keep loaded settings in mind
	app_d = app_p;
	global_d = global_p;
	display_d = display_p;
	slideShow_d = slideShow_p;
	sync_d = sync_p;
	meta_d = meta_p;
	resources_d = resources_p;

	qDebug() << "settings saved";
}

void DkSettings::setToDefaultSettings() {

	app_p.showToolBar = true;
	app_p.showStatusBar = false;
	app_p.showFileInfoLabel = QBitArray(mode_end, true);
	app_p.showFilePreview = QBitArray(mode_end, false);
	app_p.showScroller = QBitArray(mode_end, false);
	app_p.showMetaData = QBitArray(mode_end, false);
	app_p.showPlayer = QBitArray(mode_end, false);
	app_p.showHistogram = QBitArray(mode_end, false);
	app_p.showOverview = QBitArray(mode_end, true);
	app_p.advancedSettings = false;
	app_p.closeOnEsc = false;
	app_p.showMenuBar = true;

	// now set default show options
	app_p.showFileInfoLabel.setBit(mode_default, false);
	app_p.showFileInfoLabel.setBit(mode_contrast, false);

	app_p.appMode = 0;
	
	global_p.skipImgs = 10;
	global_p.numFiles = 10;
	global_p.loop = false;
	global_p.scanSubFolders = false;
	global_p.lastDir = QString();
	global_p.lastSaveDir = QString();
	global_p.recentFiles = QStringList();
	global_p.searchHistory = QStringList();
	global_p.recentFolders = QStringList();
	global_p.useTmpPath = false;
	global_p.tmpPath = QString();
	global_p.language = QString();
	global_p.setupPath = "";
	global_p.setupVersion = "";
	global_p.sortMode = sort_filename;
	global_p.sortDir = sort_ascending;
	global_p.zoomOnWheel = true;

#ifdef Q_WS_X11
	sync_p.switchModifier = true;
	global_p.altMod = Qt::ControlModifier;
	global_p.ctrlMod = Qt::AltModifier;
#else
	sync_p.switchModifier = false;
	global_p.altMod = Qt::AltModifier;
	global_p.ctrlMod = Qt::ControlModifier;
#endif

	display_p.keepZoom = zoom_keep_same_size;
	display_p.invertZoom = false;
	display_p.highlightColor = QColor(0, 204, 255);
	display_p.bgColorWidget = QColor(0, 0, 0, 100);
	display_p.bgColor = QColor(100, 100, 100, 255);
	display_p.iconColor = QColor(100,100,100,255);
	//display_p.bgColor = QColor(219, 89, 2, 255);
	display_p.bgColorFrameless = QColor(0, 0, 0, 180);
	display_p.thumbSize = 64;
	display_p.thumbPreviewSize = 64;
	display_p.saveThumb = false;
	display_p.antiAliasing = true;
	display_p.tpPattern = false;
	display_p.smallIcons = true;
	display_p.toolbarGradient = false;
	display_p.showBorder = true;
	display_p.displaySquaredThumbs = true;
	display_p.fadeSec = 0.5f;
	display_p.useDefaultColor = true;
	display_p.defaultIconColor = true;
	display_p.interpolateZoomLevel = 200;

	slideShow_p.filter = 0;
	slideShow_p.time = 3.0;
	slideShow_p.display = QBitArray(display_end, true);
	slideShow_p.backgroundColor = QColor(86, 86, 90, 255);
	slideShow_p.silentFullscreen = true;

	meta_p.metaDataBits = QBitArray(desc_end, false);
	meta_p.metaDataBits[camData_size] = false;
	meta_p.metaDataBits[camData_orientation] = false;
	meta_p.metaDataBits[camData_make] = true;
	meta_p.metaDataBits[camData_model] = true;
	meta_p.metaDataBits[camData_aperture] = true;
	meta_p.metaDataBits[camData_iso] = true;
	//MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_shutterspeed] = false;
	meta_p.metaDataBits[camData_flash] = true;
	meta_p.metaDataBits[camData_focallength] = true;
	meta_p.metaDataBits[camData_exposuremode] = false;
	meta_p.metaDataBits[camData_exposuretime] = true;
	meta_p.metaDataBits[desc_rating] = false;
	meta_p.metaDataBits[desc_usercomment] = false;
	meta_p.metaDataBits[desc_date] = false;
	meta_p.metaDataBits[desc_datetimeoriginal] = true;
	meta_p.metaDataBits[desc_imagedescription] = false;
	meta_p.metaDataBits[desc_creator] = false;
	meta_p.metaDataBits[desc_creatortitle] = false;
	meta_p.metaDataBits[desc_city] = false;
	meta_p.metaDataBits[desc_country] = false;
	meta_p.metaDataBits[desc_headline] = false;
	meta_p.metaDataBits[desc_caption] = false;
	meta_p.metaDataBits[desc_copyright] = false;
	meta_p.metaDataBits[desc_keywords] = false;
	meta_p.metaDataBits[desc_path] = true;
	meta_p.metaDataBits[desc_filesize] = true;
	meta_p.saveExifOrientation = true;
	meta_p.ignoreExifOrientation = false;

	sync_p.enableNetworkSync = false;
	sync_p.allowTransformation = true;
	sync_p.allowPosition = true;
	sync_p.allowFile = true;
	sync_p.allowImage = true;
	sync_p.updateDialogShown = false;
	sync_p.lastUpdateCheck = QDate(1970 , 1, 1);
	sync_p.syncAbsoluteTransform = true;

	resources_p.cacheMemory = 0;
	resources_p.maxImagesCached = 5;
	resources_p.fastThumbnailPreview = false;
	resources_p.fastThumbnailPreview = false;
	resources_p.filterRawImages = true;
	resources_p.loadRawThumb = raw_thumb_always;
	resources_p.filterDuplicats = false;
	resources_p.preferredExtension = "*.jpg";
	resources_p.numThumbsLoading = 0;
	resources_p.maxThumbsLoading = 5;
	resources_p.gammaCorrection = true;

	qDebug() << "ok... default settings are set";
}

QString DkFileFilterHandling::registerProgID(const QString& ext, const QString& friendlyName, bool add) {

#ifdef WIN32

	QString nomacsPath = "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\";
	QString nomacsKey = "nomacs" + ext + ".2";

	QSettings settings(nomacsPath, QSettings::NativeFormat);
	
	if (add) {
		settings.beginGroup(nomacsKey);
		settings.setValue("Default", friendlyName);
		//settings.setValue("AppUserModelID", "nomacs.ImageLounge");
		//settings.setValue("EditFlags", 1);
		//settings.setValue("CurVer", nomacsKey);
		settings.beginGroup("DefaultIcon");
		settings.setValue("Default","\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\", 1");
		settings.endGroup();
		settings.beginGroup("shell");
		settings.beginGroup("open");
		settings.beginGroup("command");
		settings.setValue("Default", "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\" \"%1\"");
		settings.endGroup();
		settings.endGroup();
		settings.endGroup();

		qDebug() << nomacsKey << " written";
	}
	else
		settings.remove(nomacsKey);

	return nomacsKey;
#else
	return QString();
#endif
}

void DkFileFilterHandling::registerExtension(const QString& ext, const QString& progKey, bool add) {

#ifdef WIN32

	QSettings settings2("HKEY_CURRENT_USER\\SOFTWARE\\Classes\\", QSettings::NativeFormat);
	settings2.beginGroup(ext);
	//if (add)
	//	settings2.setValue("Default", progKey);	
	settings2.beginGroup("OpenWithProgIds");
	//settings.beginGroup("nomacs.exe");

	if (add)
		settings2.setValue(progKey, "");	// tell system that nomacs can handle this file type
	else
		settings2.remove(progKey);
#endif
}

void DkFileFilterHandling::registerFileType(const QString& filterString, const QString& attribute, bool add) {

#ifdef WIN32

	QStringList tmp = filterString.split("(");

	if (tmp.size() != 2) {
		qDebug() << "WARNING: wrong filter string!";
		return;
	}

	QString friendlyName = tmp.at(0) + attribute;
	QString filters = tmp.at(1);
	filters.replace(")", "");
	filters.replace("*", "");

	QStringList extList = filters.split(";;");

	if (extList.empty()) {
		qDebug() << "nothing to do here, not registering: " << filterString;
		return;
	}

	// register a new progID
	QString progKey = registerProgID(extList.at(0), friendlyName, add);

	// register the extension
	for (int idx = 0; idx < extList.size(); idx++) {

		qDebug() << "registering: " << extList.at(idx);
		registerExtension(extList.at(idx), progKey, add);
		setAsDefaultApp(extList.at(idx), progKey, add);		// this is not working on Win8
	}

#endif
}

void DkFileFilterHandling::setAsDefaultApp(const QString& ext, const QString& progKey, bool add) {

#ifdef WIN32
	
	QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts", QSettings::NativeFormat);
	settings.beginGroup(ext);

	if (add) {
		settings.beginGroup("UserChoice");
		settings.setValue("ProgId", progKey);
		qDebug() << "default app set";
	}
	else
		settings.setValue("Default", "");
#endif

}

}
