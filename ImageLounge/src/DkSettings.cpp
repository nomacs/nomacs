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

bool DkSettings::App::showToolBar = true;
bool DkSettings::App::showMenuBar = true;
bool DkSettings::App::showStatusBar = false;
QBitArray DkSettings::App::showFileInfoLabel = QBitArray(DkSettings::mode_end, false);
QBitArray DkSettings::App::showFilePreview = QBitArray(DkSettings::mode_end, false);
QBitArray DkSettings::App::showScroller = QBitArray(DkSettings::mode_end, false);
QBitArray DkSettings::App::showMetaData = QBitArray(DkSettings::mode_end, false);
QBitArray DkSettings::App::showPlayer = QBitArray(DkSettings::mode_end, false);
QBitArray DkSettings::App::showHistogram = QBitArray(DkSettings::mode_end, false);
QBitArray DkSettings::App::showOverview = QBitArray(DkSettings::mode_end, true);
bool DkSettings::App::closeOnEsc = false;
int DkSettings::App::appMode = 0;
int DkSettings::App::currentAppMode = 0;
bool DkSettings::App::advancedSettings = false;

int DkSettings::Global::skipImgs = 10;
int DkSettings::Global::numFiles = 10;
bool DkSettings::Global::loop = false;
bool DkSettings::Global::scanSubFolders = false;
QString DkSettings::Global::lastDir = QString();
QString DkSettings::Global::lastSaveDir = QString();
QStringList DkSettings::Global::recentFiles = QStringList();
QStringList DkSettings::Global::recentFolders = QStringList();
QStringList DkSettings::Global::searchHistory = QStringList();
bool DkSettings::Global::useTmpPath = false;
QString DkSettings::Global::tmpPath = QString();
QString DkSettings::Global::language = "en";
QString DkSettings::Global::setupPath = "";
QString DkSettings::Global::setupVersion = "";

#ifdef Q_WS_X11
	bool DkSettings::Sync::switchModifier = true;
	Qt::KeyboardModifier DkSettings::Global::altMod = Qt::ControlModifier;
	Qt::KeyboardModifier DkSettings::Global::ctrlMod = Qt::AltModifier;
#else
	bool DkSettings::Sync::switchModifier = false;
	Qt::KeyboardModifier DkSettings::Global::altMod = Qt::AltModifier;
	Qt::KeyboardModifier DkSettings::Global::ctrlMod = Qt::ControlModifier;
#endif

bool DkSettings::MetaData::ignoreExifOrientation = false;
bool DkSettings::MetaData::saveExifOrientation = true;

// open with settings
QString DkSettings::Global::defaultAppPath = QString();
int DkSettings::Global::defaultAppIdx = -1;
bool DkSettings::Global::showDefaultAppDialog = true;
int DkSettings::Global::numUserChoices = 3;
int DkSettings::Global::sortMode = DkSettings::sort_filename;
int DkSettings::Global::sortDir = DkSettings::sort_ascending;
QStringList DkSettings::Global::userAppPaths = QStringList();

bool DkSettings::Display::keepZoom = true;
bool DkSettings::Display::invertZoom = false;
QColor DkSettings::Display::highlightColor = QColor(0, 204, 255);
QColor DkSettings::Display::bgColorWidget = QColor(0,0,0,100);
QColor DkSettings::Display::bgColor = QColor(100, 100, 100, 255);
QColor DkSettings::Display::iconColor = QColor(219, 89, 2, 255);
QColor DkSettings::Display::bgColorFrameless = QColor(0,0,0,180);
int DkSettings::Display::thumbSize = 100; // max seems to be 160 (?!)
bool DkSettings::Display::saveThumb = false;
bool DkSettings::Display::antiAliasing = true;
bool DkSettings::Display::tpPattern = false;
bool DkSettings::Display::smallIcons = true;
#ifdef Q_WS_WIN
bool DkSettings::Display::toolbarGradient = true;
#else
bool DkSettings::Display::toolbarGradient = false;
#endif
bool DkSettings::Display::showBorder = true;
bool DkSettings::Display::useDefaultColor = true;
bool DkSettings::Display::defaultIconColor = true;
int DkSettings::Display::interpolateZoomLevel = 200;

int DkSettings::SlideShow::filter = 0;
float DkSettings::SlideShow::time = 3;
QBitArray DkSettings::SlideShow::display = QBitArray(DkDisplaySettingsWidget::display_end, true);
QColor DkSettings::SlideShow::backgroundColor = QColor(200, 200, 200);
bool DkSettings::SlideShow::silentFullscreen = true;

QBitArray DkSettings::MetaData::metaDataBits = QBitArray(DkMetaDataSettingsWidget::desc_end, false);


//QString DkMetaDataSettingsWidget::scamDataDesc = QString("&Image Size;&Orientation;&Make;M&odel;&Aperture Value;&Shutter Speed Value;&Flash;F&ocalLength;") %
//	QString("&Exposure Mode;Exposure &Time");
//																													
//QString DkMetaDataSettingsWidget::sdescriptionDesc = QString("&Rating;&User Comment;&Date Time;D&ate Time Original;&Image Description;&Creator;C&reator Title;") %
//	QString("&City;C&ountry;&Headline;Ca&ption;Copy&right;Key&words");

//QString DkMetaDataSettingsWidget::scamDataDesc = QString(QT_TRANSLATE_NOOP("nmc::DkMetaData","Image Size;Orientation;Make;Model;Aperture Value;Flash;Focal Length;Exposure Mode;Exposure Time"));

//QString DkMetaDataSettingsWidget::sdescriptionDesc = QString(QT_TRANSLATE_NOOP("nmc::DkMetaData","Rating;User Comment;Date Time;Date Time Original;Image Description;Creator;Creator Title;City;Country;Headline;Caption;Copyright;Keywords;Path;File Size"));

QStringList DkMetaDataSettingsWidget::scamDataDesc = QStringList() << 
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

QStringList DkMetaDataSettingsWidget::sdescriptionDesc = QStringList() <<
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

bool DkSettings::Sync::enableNetworkSync = false;
bool DkSettings::Sync::allowTransformation = true;
bool DkSettings::Sync::allowPosition = true;
bool DkSettings::Sync::allowFile = true;
bool DkSettings::Sync::allowImage = true;
bool DkSettings::Sync::updateDialogShown= false;
QDate DkSettings::Sync::lastUpdateCheck = QDate(1970, 1, 1);	// not my birthday
bool DkSettings::Sync::syncAbsoluteTransform = true;

float DkSettings::Resources::cacheMemory = 0;
bool DkSettings::Resources::fastThumbnailPreview = false;
bool DkSettings::Resources::filterRawImages = true;
bool DkSettings::Resources::filterDuplicats = true;
QString DkSettings::Resources::preferredExtension = "*.jpg";

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

DkSettings::App& DkSettings::app = DkSettings::getAppSettings();


bool DkSettings::loaded = false;

DkSettings::App& DkSettings::getAppSettings() {
	load();
	return app_p;
}

DkSettings::Display& DkSettings::getDisplaySettings() {
	load();
	return display_p;
}

DkSettings::Global& DkSettings::getGlobalSettings() {
	load();
	return global_p;
}

DkSettings::SlideShow& DkSettings::getSlideShowSettings() {
	load();
	return slideShow_p;
}

DkSettings::Sync& DkSettings::getSyncSettings() {
	load();
	return sync_p;
}

DkSettings::MetaData& DkSettings::getMetaDataSettings() {
	load();
	return meta_p;
}

DkSettings::Resources& DkSettings::getResourceSettings() {
	load();
	return resources_p;
}


void DkSettings::load(bool force) {
	
	if (loaded && !force)
		return;

	setToDefaultSettings();

	QSettings settings;
	settings.beginGroup("AppSettings");
	
	App::showMenuBar = settings.value("showMenuBar", app_p.showMenuBar).toBool();
	App::showToolBar = settings.value("showToolBar", app_p.showToolBar).toBool();
	App::showStatusBar = settings.value("showStatusBar", app_p.showStatusBar).toBool();
	
	QBitArray tmpShow = settings.value("showFileInfoLabel", app_p.showFileInfoLabel).toBitArray();
	if (tmpShow.size() == app_p.showFileInfoLabel.size())	app_p.showFileInfoLabel = tmpShow;
	tmpShow = settings.value("showScroller", app_p.showScroller).toBitArray();
	if (tmpShow.size() == app_p.showScroller.size())	app_p.showScroller = tmpShow;
	tmpShow = settings.value("showFilePreview", app_p.showFilePreview).toBitArray();
	if (tmpShow.size() == app_p.showFilePreview.size())	app_p.showFilePreview = tmpShow;
	tmpShow = settings.value("showMetaData", app_p.showMetaData).toBitArray();
	if (tmpShow.size() == app_p.showMetaData.size())	app_p.showMetaData = tmpShow;
	tmpShow = settings.value("showPlayer", app_p.showPlayer).toBitArray();
	if (tmpShow.size() == app_p.showPlayer.size())	app_p.showPlayer = tmpShow;
	tmpShow = settings.value("showHistogram", app_p.showHistogram).toBitArray();
	if (tmpShow.size() == app_p.showHistogram.size())	app_p.showHistogram = tmpShow;
	tmpShow = settings.value("showOverview", app_p.showOverview).toBitArray();
	if (tmpShow.size() == app_p.showOverview.size())	app_p.showOverview = tmpShow;

	app_p.closeOnEsc = settings.value("closeOnEsc", app_p.closeOnEsc).toBool();
	app_p.advancedSettings = settings.value("advancedSettings", app_p.advancedSettings).toBool();

	settings.endGroup();
	// Global Settings --------------------------------------------------------------------
	settings.beginGroup("GlobalSettings");

	global_p.skipImgs = settings.value("skipImgs", global_p.skipImgs).toInt();
	global_p.numFiles = settings.value("numFiles", global_p.numFiles).toInt();

	global_p.loop = settings.value("loop", global_p.loop).toBool();
	global_p.scanSubFolders = settings.value("scanSubFolders", global_p.scanSubFolders).toBool();
	global_p.lastDir = settings.value("lastDir", global_p.lastDir).toString();
	//GlobalSettings::lastSaveDir = settings.value("lastSaveDir", GlobalSettings::lastSaveDir).toString();
	global_p.searchHistory = settings.value("searchHistory", global_p.searchHistory).toStringList();
	global_p.recentFolders = settings.value("recentFolders", global_p.recentFolders).toStringList();
	global_p.recentFiles = settings.value("recentFiles", global_p.recentFiles).toStringList();
	global_p.useTmpPath= settings.value("useTmpPath", global_p.useTmpPath).toBool();
	global_p.tmpPath = settings.value("tmpPath", global_p.tmpPath).toString();
	global_p.language = settings.value("language", global_p.language).toString();

	global_p.defaultAppPath = settings.value("defaultAppPath", global_p.defaultAppPath).toString();
	global_p.defaultAppIdx = settings.value("defaultAppIdx", global_p.defaultAppIdx).toInt();
	global_p.showDefaultAppDialog = settings.value("showDefaultAppDialog", global_p.showDefaultAppDialog).toBool();
	global_p.numUserChoices = settings.value("numUserChoices", global_p.numUserChoices).toInt();
	global_p.sortMode = settings.value("sortMode", global_p.sortMode).toInt();
	global_p.sortDir = settings.value("sortDir", global_p.sortDir).toInt();
	global_p.userAppPaths = settings.value("userAppPaths", global_p.userAppPaths).toStringList();
	global_p.setupPath = settings.value("setupPath", global_p.setupPath).toString();
	global_p.setupVersion = settings.value("setupVersion", global_p.setupVersion).toString();

	settings.endGroup();
	// Display Settings --------------------------------------------------------------------
	settings.beginGroup("DisplaySettings");

	display_p.keepZoom = settings.value("resetMatrix", display_p.keepZoom).toBool();
	display_p.invertZoom = settings.value("invertZoom", display_p.invertZoom).toBool();
	display_p.highlightColor = settings.value("highlightColor", display_p.highlightColor).value<QColor>();
	display_p.bgColorWidget = settings.value("bgColor", display_p.bgColorWidget).value<QColor>();
	display_p.bgColor = settings.value("bgColorNoMacs", display_p.bgColor).value<QColor>();
	display_p.iconColor = settings.value("iconColor", display_p.iconColor).value<QColor>();

	display_p.bgColorFrameless = settings.value("bgColorFrameless", display_p.bgColorFrameless).value<QColor>();
	display_p.thumbSize = settings.value("thumbSize", display_p.thumbSize).toInt();
	display_p.saveThumb = settings.value("saveThumb", display_p.saveThumb).toBool();
	display_p.antiAliasing = settings.value("antiAliasing", display_p.antiAliasing).toBool();
	display_p.tpPattern = settings.value("tpPattern", display_p.tpPattern).toBool();
	display_p.smallIcons = settings.value("smallIcons", display_p.smallIcons).toBool();
	display_p.toolbarGradient = settings.value("toolbarGradient", display_p.toolbarGradient).toBool();
	display_p.showBorder = settings.value("showBorder", display_p.showBorder).toBool();
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

	settings.endGroup();
	// Resource Settings --------------------------------------------------------------------
	settings.beginGroup("ResourceSettings");

	resources_p.cacheMemory = settings.value("cacheMemory", resources_p.cacheMemory).toFloat();
	resources_p.fastThumbnailPreview = settings.value("fastThumbnailPreview", resources_p.fastThumbnailPreview).toBool();
	resources_p.filterRawImages = settings.value("filterRawImages", resources_p.filterRawImages).toBool();	
	resources_p.filterDuplicats = settings.value("filterDuplicates", resources_p.filterDuplicats).toBool();
	resources_p.preferredExtension = settings.value("preferredExtension", resources_p.preferredExtension).toString();	

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

	loaded = true;


}

void DkSettings::save(bool force) {
		
	QSettings settings;
	
	settings.beginGroup("AppSettings");

	if (!force && app_p.showMenuBar != app_d.showMenuBar)
		settings.setValue("showMenuBar", app_p.showMenuBar);
	

	int myAppMode = App::appMode;
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
	if (!force && app_p.appMode != app_d.appMode)
		settings.setValue("appMode", app_p.appMode);
	if (!force && app_p.currentAppMode != app_d.currentAppMode)
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
		settings.setValue("scanSubFolders",global_p.scanSubFolders);
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
	if (!force && global_p.defaultAppIdx != global_d.defaultAppIdx)
		settings.setValue("defaultAppIdx", global_p.defaultAppIdx);
	if (!force && global_p.defaultAppPath != global_d.defaultAppPath)
		settings.setValue("defaultAppPath", global_p.defaultAppPath);
	if (!force && global_p.showDefaultAppDialog != global_d.showDefaultAppDialog)
		settings.setValue("showDefaultAppDialog", global_p.showDefaultAppDialog);
	if (!force && global_p.numUserChoices != global_d.numUserChoices)
		settings.setValue("numUserChoices", global_p.numUserChoices);
	if (!force && global_p.sortMode != global_d.sortMode)
		settings.setValue("sortMode", global_p.sortMode);
	if (!force && global_p.sortDir != global_d.sortDir)
		settings.setValue("sortDir", global_p.sortDir);
	if (!force && global_p.userAppPaths != global_d.userAppPaths)
		settings.setValue("userAppPaths", global_p.userAppPaths);
	if (!force && global_p.setupPath != global_d.setupPath)
		settings.setValue("setupPath", global_p.setupPath);
	if (!force && global_p.setupVersion != global_d.setupVersion)
		settings.setValue("setupVersion", global_p.setupVersion);

	settings.endGroup();
	// Display Settings --------------------------------------------------------------------
	settings.beginGroup("DisplaySettings");

	if (!force && display_p.keepZoom != display_d.keepZoom)
		settings.setValue("resetMatrix",display_p.keepZoom);
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

	settings.endGroup();
	// Resource Settings --------------------------------------------------------------------
	settings.beginGroup("ResourceSettings");

	if (!force && resources_p.cacheMemory != resources_d.cacheMemory)
		settings.setValue("cacheMemory", resources_p.cacheMemory);
	if (!force && resources_p.fastThumbnailPreview != resources_d.fastThumbnailPreview)
		settings.setValue("fastThumbnailPreview", resources_p.fastThumbnailPreview);
	if (!force && resources_p.filterRawImages != resources_d.filterRawImages)
		settings.setValue("filterRawImages", resources_p.filterRawImages);
	if (!force && resources_p.filterDuplicats != resources_d.filterDuplicats)
		settings.setValue("filterDuplicates", resources_p.filterDuplicats);
	if (!force && resources_p.preferredExtension != resources_d.preferredExtension)
		settings.setValue("preferredExtension", resources_p.preferredExtension);

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

	App::showMenuBar = true;
	App::showToolBar = true;
	App::showStatusBar = false;
	App::showFileInfoLabel = QBitArray(mode_end, true);
	App::showFilePreview = QBitArray(mode_end, false);
	App::showScroller = QBitArray(mode_end, false);
	App::showMetaData = QBitArray(mode_end, false);
	App::showPlayer = QBitArray(mode_end, false);
	App::showHistogram = QBitArray(mode_end, false);
	App::showOverview = QBitArray(mode_end, true);
	App::advancedSettings = false;
	App::closeOnEsc = false;

	// now set default show options
	App::showFileInfoLabel.setBit(mode_default, false);
	App::showFileInfoLabel.setBit(mode_contrast, false);


	App::appMode = 0;
	
	Global::skipImgs = 10;
	Global::numFiles = 10;
	Global::loop = false;
	Global::scanSubFolders = true;
	Global::lastDir = QString();
	Global::lastSaveDir = QString();
	Global::recentFiles = QStringList();
	Global::searchHistory = QStringList();
	Global::recentFolders = QStringList();
	Global::useTmpPath = false;
	Global::tmpPath = QString();
	Global::language = QString();
	Global::defaultAppIdx = -1;
	Global::defaultAppPath = QString();
	Global::showDefaultAppDialog = true;
	Global::numUserChoices = 3;
	Global::userAppPaths = QStringList();
	Global::setupPath = "";
	Global::setupVersion = "";
	Global::sortMode = sort_filename;
	Global::sortDir = sort_ascending;

#ifdef Q_WS_X11
	Sync::switchModifier = true;
	Global::altMod = Qt::ControlModifier;
	Global::ctrlMod = Qt::AltModifier;
#else
	Sync::switchModifier = false;
	Global::altMod = Qt::AltModifier;
	Global::ctrlMod = Qt::ControlModifier;
#endif


	Display::keepZoom = true;
	Display::invertZoom = false;
	Display::highlightColor = QColor(0, 204, 255);
	Display::bgColorWidget = QColor(0, 0, 0, 100);
	Display::bgColor = QColor(100, 100, 100, 255);
	Display::bgColor = QColor(219, 89, 2, 255);
	Display::bgColorFrameless = QColor(0, 0, 0, 180);
	Display::thumbSize = 100;
	Display::saveThumb = false;
	Display::antiAliasing = true;
	Display::tpPattern = false;
	Display::smallIcons = true;
#ifdef Q_WS_WIN
	Display::toolbarGradient = true;
#else
	Display::toolbarGradient = false;
#endif
	Display::showBorder = true;
	Display::useDefaultColor = true;
	Display::defaultIconColor = true;
	Display::interpolateZoomLevel = 200;

	SlideShow::filter = 0;
	SlideShow::time = 3.0;
	SlideShow::display = QBitArray(DkDisplaySettingsWidget::display_end, true);
	SlideShow::backgroundColor = QColor(217, 219, 228, 100);
	SlideShow::silentFullscreen = true;


	MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_size] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_orientation] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_make] = true;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_model] = true;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_aperture] = true;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_iso] = true;
	//MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_shutterspeed] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_flash] = true;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_focallength] = true;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_exposuremode] = true;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_exposuretime] = true;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_rating] = true;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_usercomment] = true;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_date] = true;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_datetimeoriginal] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_imagedescription] = true;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_creator] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_creatortitle] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_city] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_country] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_headline] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_caption] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_copyright] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_keywords] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_path] = false;
	MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_filesize] = false;


	Sync::enableNetworkSync = false;
	Sync::allowTransformation = true;
	Sync::allowPosition = true;
	Sync::allowFile = true;
	Sync::allowImage = true;
	Sync::updateDialogShown = false;
	Sync::lastUpdateCheck = QDate(1970 , 1, 1);
	Sync::syncAbsoluteTransform = true;

	Resources::cacheMemory = 0;
	Resources::fastThumbnailPreview = false;
	Resources::filterRawImages = true;
	Resources::filterDuplicats = true;
	Resources::preferredExtension = "*.jpg";

	qDebug() << "ok... default settings are set";
}

// DkSettingsDialog --------------------------------------------------------------------
DkSettingsDialog::DkSettingsDialog(QWidget* parent) : QDialog(parent) {

	//this->resize(600,420);

	createLayout();
	createSettingsWidgets();
	for (int i = 0; i < widgetList.size(); i++) {
		if (!DkSettings::App::advancedSettings) {
			listView->setRowHidden(i, widgetList[i]->showOnlyInAdvancedMode);
		}
		else
			listView->setRowHidden(i, false);
	}
	init();

	//setMinimumSize(1000,1000);
	this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	connect(listView, SIGNAL(activated(const QModelIndex &)), this, SLOT(listViewSelected(const QModelIndex &)));
	connect(listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(listViewSelected(const QModelIndex &)));
	connect(listView, SIGNAL(entered(const QModelIndex &)), this, SLOT(listViewSelected(const QModelIndex &)));

	connect(buttonOk, SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));
	connect(globalSettingsWidget, SIGNAL(applyDefault()), this, SLOT(setToDefault()));
	connect(cbAdvancedSettings, SIGNAL(stateChanged(int)), this, SLOT(advancedSettingsChanged(int)));
}

DkSettingsDialog::~DkSettingsDialog() {

	QItemSelectionModel *m = listView->selectionModel();
	if (m)
		delete m;

}

void DkSettingsDialog::init() {
	setWindowTitle(tr("Settings"));
	foreach (DkSettingsWidget* curWidget, widgetList) {
		curWidget->hide();
		curWidget->toggleAdvancedOptions(DkSettings::App::advancedSettings);
		centralLayout->addWidget(curWidget);
	}
	widgetList[0]->show(); // display first;
	cbAdvancedSettings->setChecked(DkSettings::App::advancedSettings);
}

void DkSettingsDialog::createLayout() {
	
	QWidget* leftWidget = new QWidget(this);
	QWidget* bottomWidget = new QWidget(this);

	// left Widget
	QVBoxLayout* leftWidgetVBoxLayout = new QVBoxLayout(leftWidget);
	leftLabel = new QLabel;
	leftLabel->setText(tr("Categories"));

	listView = new DkSettingsListView(this); 
	listView->setMaximumWidth(100);
	//listView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	listView->setSelectionMode(QAbstractItemView::SingleSelection);

	QStringList stringList;
	stringList << tr("General") << tr("Display") << tr("File Info") << tr("Synchronize") << tr("Exif") << tr("Resources");
	QItemSelectionModel *m = listView->selectionModel();
	listView->setModel(new QStringListModel(stringList, this));
	delete m;
	
	leftWidgetVBoxLayout->addWidget(leftLabel);
	leftWidgetVBoxLayout->addWidget(listView);

	// bottom widget
	QHBoxLayout* bottomWidgetHBoxLayout = new QHBoxLayout(bottomWidget);
	buttonOk = new QPushButton;
	buttonOk->setText(tr("Ok"));

	buttonCancel = new QPushButton;
	buttonCancel->setText(tr("Cancel"));

	cbAdvancedSettings = new QCheckBox(tr("Advanced"));

	bottomWidgetHBoxLayout->addWidget(cbAdvancedSettings);
	bottomWidgetHBoxLayout->addStretch();
	bottomWidgetHBoxLayout->addWidget(buttonOk);
	bottomWidgetHBoxLayout->addWidget(buttonCancel);


	borderLayout = new BorderLayout(this);
	borderLayout->addWidget(leftWidget, BorderLayout::West);
	borderLayout->addWidget(bottomWidget, BorderLayout::South);
	this->setSizeGripEnabled(false);

	// central widget
	centralWidget = new QWidget(this);
	borderLayout->addWidget(centralWidget, BorderLayout::Center);

	centralLayout = new QHBoxLayout(centralWidget);
}

void DkSettingsDialog::createSettingsWidgets() {
	globalSettingsWidget = new DkGlobalSettingsWidget(centralWidget);
	displaySettingsWidget = new DkDisplaySettingsWidget(centralWidget);
	slideshowSettingsWidget = new DkFileWidget(centralWidget);
	synchronizeSettingsWidget = new DkSynchronizeSettingsWidget(centralWidget);
	exifSettingsWidget = new DkMetaDataSettingsWidget(centralWidget);
	resourceSettingsWidget = new DkResourceSettingsWidgets(centralWidget);

	widgetList.clear();
	widgetList.push_back(globalSettingsWidget);
	widgetList.push_back(displaySettingsWidget);
	widgetList.push_back(slideshowSettingsWidget);
	widgetList.push_back(synchronizeSettingsWidget);
	widgetList.push_back(exifSettingsWidget);
	widgetList.push_back(resourceSettingsWidget);
}

void DkSettingsDialog::listViewSelected(const QModelIndex & qmodel) {
	if (listView->isRowHidden(qmodel.row()))
		return;

	foreach (DkSettingsWidget* curWidget, widgetList) {
		curWidget->hide();
	}
	widgetList[qmodel.row()]->show(); // display selected;
}

void DkSettingsDialog::saveSettings() {
	
	QString curLanguage = DkSettings::Global::language;
	QColor curBgColWidget = DkSettings::Display::bgColorWidget;
	QColor curBgCol = DkSettings::Display::bgColor;
	QColor curIconCol = DkSettings::Display::iconColor;
	QColor curBgColFrameless = DkSettings::Display::bgColorFrameless;
	bool curIcons = DkSettings::Display::smallIcons;
	bool curGradient = DkSettings::Display::toolbarGradient;
	bool curUseCol = DkSettings::Display::useDefaultColor;
	bool curUseIconCol = DkSettings::Display::defaultIconColor;
	
	foreach (DkSettingsWidget* curWidget, widgetList) {
		curWidget->writeSettings();
	}

	DkSettings* settings = new DkSettings();
	settings->save();
	this->close();
	
	// if the language changed we need to restart nomacs (re-translating while running is pretty hard to accomplish)
	if (curLanguage != DkSettings::Global::language ||
		DkSettings::Display::bgColor != curBgCol ||
		DkSettings::Display::iconColor != curIconCol ||
		DkSettings::Display::bgColorWidget != curBgColWidget ||
		DkSettings::Display::bgColorFrameless != curBgColFrameless ||
		DkSettings::Display::useDefaultColor != curUseCol ||
		DkSettings::Display::defaultIconColor != curUseIconCol ||
		DkSettings::Display::smallIcons != curIcons ||
		DkSettings::Display::toolbarGradient != curGradient)
		emit languageChanged();
	else
		emit settingsChanged();

	if (settings)
		delete settings;
}

void DkSettingsDialog::initWidgets() {
	qDebug() << "initializing widgets...";
	foreach (DkSettingsWidget* curWidget, widgetList) {
		curWidget->init();
	}

}

void DkSettingsDialog::advancedSettingsChanged(int state) {

	DkSettings::App::advancedSettings = cbAdvancedSettings->isChecked();

	QModelIndex selection = listView->currentIndex();

	foreach (DkSettingsWidget* curWidget, widgetList) {
		curWidget->toggleAdvancedOptions(DkSettings::App::advancedSettings);
	}

	bool wasSelected = false;
	for (int i = 0; i < widgetList.size(); i++) {
		if (!DkSettings::App::advancedSettings) {
			listView->setRowHidden(i, widgetList[i]->showOnlyInAdvancedMode);
			if (widgetList[i]->showOnlyInAdvancedMode && selection.row() == i) wasSelected = true;
		}
		else
			listView->setRowHidden(i, false);
	}

	if (wasSelected) {
		listView->setCurrentIndex(selection.model()->index(0,0)); 
		listViewSelected(selection.model()->index(0,0));
	};
	
	
}

// DkGlobalSettingsWidget --------------------------------------------------------------------

DkGlobalSettingsWidget::DkGlobalSettingsWidget(QWidget* parent) : DkSettingsWidget(parent) {
	createLayout();
	init();

}

void DkGlobalSettingsWidget::init() {
	cbShowMenu->setChecked(DkSettings::App::showMenuBar);
	cbShowStatusbar->setChecked(DkSettings::App::showStatusBar);
	cbShowToolbar->setChecked(DkSettings::App::showToolBar);
	cbSmallIcons->setChecked(DkSettings::Display::smallIcons);
	cbToolbarGradient->setChecked(DkSettings::Display::toolbarGradient);
	cbCloseOnEsc->setChecked(DkSettings::App::closeOnEsc);

	curLanguage = DkSettings::Global::language;
	langCombo->setCurrentIndex(languages.indexOf(curLanguage));
	if (langCombo->currentIndex() == -1) // set index to English if language has not been found
		langCombo->setCurrentIndex(0);

	displayTimeSpin->setSpinBoxValue(DkSettings::SlideShow::time);

	connect(buttonDefaultSettings, SIGNAL(clicked()), this, SLOT(setToDefaultPressed()));
	connect(buttonDefaultSettings, SIGNAL(clicked()), highlightColorChooser, SLOT(on_resetButton_clicked()));
	connect(buttonDefaultSettings, SIGNAL(clicked()), fullscreenColChooser, SLOT(on_resetButton_clicked()));
	connect(buttonDefaultSettings, SIGNAL(clicked()), bgColorChooser, SLOT(on_resetButton_clicked()));
	connect(buttonDefaultSettings, SIGNAL(clicked()), iconColorChooser, SLOT(on_resetButton_clicked()));
	connect(buttonDefaultSettings, SIGNAL(clicked()), bgColorWidgetChooser, SLOT(on_resetButton_clicked()));

}

void DkGlobalSettingsWidget::createLayout() {
	
	QHBoxLayout* widgetLayout = new QHBoxLayout(this);
	QVBoxLayout* leftLayout = new QVBoxLayout();
	QVBoxLayout* rightLayout = new QVBoxLayout();
	QWidget* rightWidget = new QWidget(this);
	rightWidget->setLayout(rightLayout);

	highlightColorChooser = new DkColorChooser(QColor(0, 204, 255), tr("Highlight Color"), this);
	highlightColorChooser->setColor(DkSettings::Display::highlightColor);

	iconColorChooser = new DkColorChooser(QColor(219, 89, 2, 255), tr("Icon Color"), this);
	iconColorChooser->setColor(DkSettings::Display::iconColor);
	connect(iconColorChooser, SIGNAL(resetClicked()), this, SLOT(iconColorReset()));

	bgColorChooser = new DkColorChooser(QColor(100, 100, 100, 255), tr("Background Color"), this);
	bgColorChooser->setColor(DkSettings::Display::bgColor);
	connect(bgColorChooser, SIGNAL(resetClicked()), this, SLOT(bgColorReset()));

	bgColorWidgetChooser = new DkColorChooser(QColor(0, 0, 0, 100), tr("Widget Color"), this);
	bgColorWidgetChooser->setColor((DkSettings::App::appMode == DkSettings::mode_frameless) ?
		DkSettings::Display::bgColorFrameless : DkSettings::Display::bgColorWidget);

	fullscreenColChooser = new DkColorChooser(QColor(86,86,90), tr("Fullscreen Color"), this);
	fullscreenColChooser->setColor(DkSettings::SlideShow::backgroundColor);

	displayTimeSpin = new DkDoubleSpinBoxWidget(tr("Display Time:"), tr("sec"), 0.1f, 99, this, 1, 1);

	QWidget* langWidget = new QWidget(rightWidget);
	QGridLayout* langLayout = new QGridLayout(langWidget);
	langLayout->setMargin(0);
	QLabel* langLabel = new QLabel("choose language:", langWidget);
	langCombo = new QComboBox(langWidget);

	QDir qmDir = qApp->applicationDirPath();
	QStringList fileNames = qmDir.entryList(QStringList("nomacs_*.qm"));
	if (fileNames.size() == 0) {
		QDir appDir = QDir(qApp->applicationDirPath());
		qmDir = QDir(appDir.filePath("../share/nomacs/translations/"));
		fileNames = qmDir.entryList(QStringList("nomacs_*.qm"));
	}

	langCombo->addItem("English");
	languages << "en";

	for (int i = 0; i < fileNames.size(); ++i) {
		QString locale = fileNames[i];
		locale.remove(0, locale.indexOf('_') + 1);
		locale.chop(3);

		QTranslator translator;
		if (translator.load(fileNames[i], qmDir.absolutePath()))
			qDebug() << "translation loaded";
		else
			qDebug() << "translation NOT loaded";

		//: this should be the name of the language in which nomacs is translated to
		QString language = translator.translate("nmc::DkGlobalSettingsWidget", "English");
		if (language.isEmpty())
			continue;

		langCombo->addItem(language);
		languages << locale;

		if (locale == curLanguage) {
			langCombo->setCurrentIndex(i+1); // +1 because of english
		}
	}

	QLabel* translateLabel = new QLabel("<a href=\"http://www.nomacs.org/how-to-translate-nomacs/\">translate nomacs</a>", langWidget);
	translateLabel->setToolTip(tr("if you want to help us and translate nomacs"));
	QFont font;
	font.setPointSize(7);
	translateLabel->setFont(font);
	translateLabel->setOpenExternalLinks(true);

	langLayout->addWidget(langLabel,0,0);
	langLayout->addWidget(langCombo,1,0);
	langLayout->addWidget(translateLabel,2,0,Qt::AlignRight);

	QWidget* showBarsWidget = new QWidget(rightWidget);
	QVBoxLayout* showBarsLayout = new QVBoxLayout(showBarsWidget);
	cbShowMenu = new QCheckBox(tr("Show Menu"), showBarsWidget);
	cbShowToolbar = new QCheckBox(tr("Show Toolbar"), showBarsWidget);
	cbShowStatusbar = new QCheckBox(tr("Show Statusbar"), showBarsWidget);
	cbSmallIcons = new QCheckBox(tr("Small Icons"), showBarsWidget);
	cbToolbarGradient = new QCheckBox(tr("Toolbar Gradient"), showBarsWidget);
	cbCloseOnEsc = new QCheckBox(tr("Close on ESC"), showBarsWidget);
	showBarsLayout->addWidget(cbShowMenu);
	showBarsLayout->addWidget(cbShowToolbar);
	showBarsLayout->addWidget(cbShowStatusbar);
	showBarsLayout->addWidget(cbSmallIcons);
	showBarsLayout->addWidget(cbToolbarGradient);
	showBarsLayout->addWidget(cbCloseOnEsc);

	// set to default
	QWidget* defaultSettingsWidget = new QWidget(rightWidget);
	QHBoxLayout* defaultSettingsLayout = new QHBoxLayout(defaultSettingsWidget);
	defaultSettingsLayout->setContentsMargins(0,0,0,0);
	defaultSettingsLayout->setDirection(QHBoxLayout::RightToLeft);
	buttonDefaultSettings = new QPushButton(tr("Apply default settings"), defaultSettingsWidget);
	buttonDefaultSettings->setMinimumSize(buttonDefaultSettings->sizeHint());
	defaultSettingsLayout->addWidget(buttonDefaultSettings);
	defaultSettingsLayout->addStretch();

	defaultSettingsWidget->setMinimumSize(defaultSettingsWidget->sizeHint());

	leftLayout->addWidget(bgColorChooser);
	leftLayout->addWidget(highlightColorChooser);
	leftLayout->addWidget(bgColorWidgetChooser);
	leftLayout->addWidget(fullscreenColChooser);
	leftLayout->addWidget(iconColorChooser);
	leftLayout->addWidget(displayTimeSpin);
	leftLayout->addStretch();
	rightLayout->addWidget(langWidget);
	rightLayout->addWidget(showBarsWidget);
	rightLayout->addStretch();
	rightLayout->addWidget(defaultSettingsWidget);


	widgetLayout->addLayout(leftLayout);
	widgetLayout->addWidget(rightWidget);
}

void DkGlobalSettingsWidget::writeSettings() {
	DkSettings::App::showMenuBar = cbShowMenu->isChecked();
	DkSettings::App::showStatusBar = cbShowStatusbar->isChecked();
	DkSettings::App::showToolBar = cbShowToolbar->isChecked();
	DkSettings::App::closeOnEsc = cbCloseOnEsc->isChecked();
	DkSettings::Display::smallIcons = cbSmallIcons->isChecked();
	DkSettings::Display::toolbarGradient = cbToolbarGradient->isChecked();
	DkSettings::SlideShow::time = displayTimeSpin->getSpinBoxValue();

	if (DkSettings::App::appMode == DkSettings::mode_frameless)
		DkSettings::Display::bgColorFrameless = bgColorWidgetChooser->getColor();
	else
		DkSettings::Display::bgColorWidget = bgColorWidgetChooser->getColor();

	if (bgColorChooser->isAccept())
		DkSettings::Display::useDefaultColor = false;

	if (iconColorChooser->isAccept())
		DkSettings::Display::defaultIconColor = false;

	DkSettings::Display::iconColor = iconColorChooser->getColor();
	DkSettings::Display::bgColor = bgColorChooser->getColor();
	DkSettings::Display::highlightColor = highlightColorChooser->getColor();
	DkSettings::SlideShow::backgroundColor = fullscreenColChooser->getColor();

	DkSettings::Global::language = languages.at(langCombo->currentIndex());
}



// DkDisplaySettingsWidget --------------------------------------------------------------------
DkDisplaySettingsWidget::DkDisplaySettingsWidget(QWidget* parent) : DkSettingsWidget(parent) {
	showOnlyInAdvancedMode = true;

	createLayout();
	init();
	//showOnlyInAdvancedMode = true;

	connect(cbName, SIGNAL(clicked(bool)), this, SLOT(showFileName(bool)));
	connect(cbCreationDate, SIGNAL(clicked(bool)), this, SLOT(showCreationDate(bool)));
	connect(cbRating, SIGNAL(clicked(bool)), this, SLOT(showRating(bool)));
}

void DkDisplaySettingsWidget::init() {
	cbName->setChecked(DkSettings::SlideShow::display.testBit(display_file_name));
	cbCreationDate->setChecked(DkSettings::SlideShow::display.testBit(display_creation_date));
	cbRating->setChecked(DkSettings::SlideShow::display.testBit(display_file_rating));

	cbInvertZoom->setChecked(DkSettings::Display::invertZoom);
	cbKeepZoom->setChecked(DkSettings::Display::keepZoom);
	maximalThumbSizeWidget->setSpinBoxValue(DkSettings::Display::thumbSize);
	cbSaveThumb->setChecked(DkSettings::Display::saveThumb);
	interpolateWidget->setSpinBoxValue(DkSettings::Display::interpolateZoomLevel);

	cbShowBorder->setChecked(DkSettings::Display::showBorder);
	cbSilentFullscreen->setChecked(DkSettings::SlideShow::silentFullscreen);
}

void DkDisplaySettingsWidget::createLayout() {
	QHBoxLayout* widgetLayout = new QHBoxLayout(this);
	QVBoxLayout* leftLayout = new QVBoxLayout;
	QVBoxLayout* rightLayout = new QVBoxLayout;

	QGroupBox* gbZoom = new QGroupBox(tr("Zoom"));
	QVBoxLayout* gbZoomLayout = new QVBoxLayout(gbZoom);
	interpolateWidget = new DkSpinBoxWidget(tr("Stop interpolating at:"), tr("% zoom level"), 0, 7000, this, 10);
	QWidget* zoomCheckBoxes = new QWidget(this);
	QVBoxLayout* vbCheckBoxLayout = new QVBoxLayout(zoomCheckBoxes);
	vbCheckBoxLayout->setContentsMargins(11,0,11,0);
	cbInvertZoom = new QCheckBox(tr("Invert Zoom"), this);
	cbKeepZoom = new QCheckBox(tr("Keep Zoom"), this);
	vbCheckBoxLayout->addWidget(cbInvertZoom);
	vbCheckBoxLayout->addWidget(cbKeepZoom);
	gbZoomLayout->addWidget(interpolateWidget);
	gbZoomLayout->addWidget(zoomCheckBoxes);

	QGroupBox* gbThumbs = new QGroupBox(tr("Thumbnails"));
	QVBoxLayout* gbThumbsLayout = new QVBoxLayout(gbThumbs);
	maximalThumbSizeWidget = new DkSpinBoxWidget(tr("maximal size:"), tr("pixel"), 16, 160, this);
	maximalThumbSizeWidget->setSpinBoxValue(DkSettings::Display::thumbSize);
	cbSaveThumb = new QCheckBox(tr("save Thumbnails"), this);
	cbSaveThumb->setToolTip(tr("saves thumbnails to images (EXPERIMENTAL)"));
	gbThumbsLayout->addWidget(maximalThumbSizeWidget);
	gbThumbsLayout->addWidget(cbSaveThumb);

	QGroupBox* gbFileInfo = new QGroupBox(tr("File Information"));
	QVBoxLayout* gbLayout = new QVBoxLayout(gbFileInfo);
	cbName = new QCheckBox(tr("Image Name"));
	gbLayout->addWidget(cbName);
	cbCreationDate = new QCheckBox(tr("Creation Date"));
	gbLayout->addWidget(cbCreationDate);
	cbRating = new QCheckBox(tr("Rating"));
	gbLayout->addWidget(cbRating);

	QGroupBox* gbFrameless = new QGroupBox(tr("Frameless"));
	QVBoxLayout* gbFramelessLayout = new QVBoxLayout(gbFrameless);
	cbShowBorder = new QCheckBox(tr("Show Border"));
	gbFramelessLayout->addWidget(cbShowBorder);

	QGroupBox* gbFullscreen = new QGroupBox(tr("Fullscreen"));
	QVBoxLayout* gbFullScreenLayout = new QVBoxLayout(gbFullscreen);
	cbSilentFullscreen = new QCheckBox(tr("Silent Fullscreen"));
	gbFullScreenLayout->addWidget(cbSilentFullscreen);


	leftLayout->addWidget(gbZoom);
	leftLayout->addWidget(gbThumbs);
	leftLayout->addStretch();
	rightLayout->addWidget(gbFileInfo);
	rightLayout->addWidget(gbFrameless);
	rightLayout->addWidget(gbFullscreen);
	rightLayout->addStretch();

	widgetLayout->addLayout(leftLayout, 1);
	widgetLayout->addLayout(rightLayout, 1);
}

void DkDisplaySettingsWidget::writeSettings() {

	DkSettings::Display::invertZoom = (cbInvertZoom->isChecked()) ? true : false;
	DkSettings::Display::keepZoom = (cbKeepZoom->isChecked()) ? true : false;
	
	DkSettings::SlideShow::silentFullscreen = cbSilentFullscreen->isChecked();

	DkSettings::SlideShow::display.setBit(display_file_name, cbName->isChecked());
	DkSettings::SlideShow::display.setBit(display_creation_date, cbCreationDate->isChecked());
	DkSettings::SlideShow::display.setBit(display_file_rating, cbRating->isChecked());

	DkSettings::Display::thumbSize = maximalThumbSizeWidget->getSpinBoxValue();
	DkSettings::Display::saveThumb = cbSaveThumb->isChecked();
	DkSettings::Display::interpolateZoomLevel = interpolateWidget->getSpinBoxValue();
	DkSettings::Display::showBorder = cbShowBorder->isChecked();
}

void DkDisplaySettingsWidget::showFileName(bool checked) {
	DkSettings::SlideShow::display.setBit(display_file_name, checked);
}

void DkDisplaySettingsWidget::showCreationDate(bool checked) {
	DkSettings::SlideShow::display.setBit(display_creation_date, checked);
}

void DkDisplaySettingsWidget::showRating(bool checked) {
	DkSettings::SlideShow::display.setBit(display_file_rating, checked);
}


// DkFileWidget --------------------------------------------------------------------

DkFileWidget::DkFileWidget(QWidget* parent) : DkSettingsWidget(parent) {
	showOnlyInAdvancedMode = true;

	createLayout();
	init();
}

void DkFileWidget::init() {

	//spFilter->setValue(DkSettings::SlideShowSettings::filter);

	cbWrapImages->setChecked(DkSettings::Global::loop);
	skipImgWidget->setSpinBoxValue(DkSettings::Global::skipImgs);
	numberFiles->setSpinBoxValue(DkSettings::Global::numFiles);
	cbUseTmpPath->setChecked(DkSettings::Global::useTmpPath);
	tmpPath = DkSettings::Global::tmpPath;
	leTmpPath->setText(tmpPath);
	if (!DkSettings::Global::useTmpPath) {
		leTmpPath->setDisabled(true);
		pbTmpPath->setDisabled(true);
	}
	
	connect(pbTmpPath, SIGNAL(clicked()), this, SLOT(tmpPathButtonPressed()));
	connect(cbUseTmpPath, SIGNAL(stateChanged(int)), this, SLOT(useTmpPathChanged(int)));
	connect(leTmpPath, SIGNAL(textChanged(QString)), this, SLOT(lineEditChanged(QString)));

	lineEditChanged(tmpPath);
}

void DkFileWidget::createLayout() {
	QVBoxLayout* widgetLayout = new QVBoxLayout(this);
	QHBoxLayout* subWidgetLayout = new QHBoxLayout();
	QVBoxLayout* leftLayout = new QVBoxLayout;
	QVBoxLayout* rightLayout = new QVBoxLayout;

	gbDragDrop = new QGroupBox(tr("Drag && Drop"));
	QVBoxLayout* vboxGbDragDrop = new QVBoxLayout(gbDragDrop);
	QWidget* tmpPathWidget = new QWidget(this);
	QVBoxLayout* vbTmpPathWidget = new QVBoxLayout(tmpPathWidget);

	cbUseTmpPath = new QCheckBox(tr("use temporary folder"), this);

	QWidget* lineEditWidget = new QWidget(this);
	QHBoxLayout* hbLineEditWidget = new QHBoxLayout(lineEditWidget);
	leTmpPath = new QLineEdit(this);
	pbTmpPath = new QPushButton(tr("..."), this);
	pbTmpPath->setMaximumWidth(40);
	hbLineEditWidget->addWidget(leTmpPath);
	hbLineEditWidget->addWidget(pbTmpPath);
	vboxGbDragDrop->addWidget(tmpPathWidget);

	vbTmpPathWidget->addWidget(cbUseTmpPath);
	vbTmpPathWidget->addWidget(lineEditWidget);

	
	skipImgWidget = new DkSpinBoxWidget(tr("Skip Images:"), tr("on PgUp and PgDown"), 1, 99, this);
	numberFiles = new DkSpinBoxWidget(tr("Number of Recent Files/Folders:"), tr("shown in Menu"), 1, 99, this);
	QWidget* checkBoxWidget = new QWidget(this);
	QGridLayout* vbCheckBoxLayout = new QGridLayout(checkBoxWidget);
	cbWrapImages = new QCheckBox(tr("Wrap Images"));


	QPushButton* pbOpenWith = new QPushButton(tr("&Open With"), this);
	connect(pbOpenWith, SIGNAL(clicked()), this, SLOT(openWithDialog()));
	
	widgetLayout->addWidget(gbDragDrop);
	leftLayout->addWidget(skipImgWidget);
	leftLayout->addWidget(numberFiles);
	leftLayout->addWidget(cbWrapImages);
	leftLayout->addStretch();
	rightLayout->addWidget(pbOpenWith);
	rightLayout->addStretch();
	subWidgetLayout->addLayout(leftLayout);
	subWidgetLayout->addLayout(rightLayout);
	widgetLayout->addLayout(subWidgetLayout);

}

void DkFileWidget::writeSettings() {
	DkSettings::Global::skipImgs = skipImgWidget->getSpinBoxValue();
	DkSettings::Global::numFiles = numberFiles->getSpinBoxValue();
	DkSettings::Global::loop = cbWrapImages->isChecked();
	DkSettings::Global::useTmpPath = cbUseTmpPath->isChecked();
	DkSettings::Global::tmpPath = existsDirectory(leTmpPath->text()) ? leTmpPath->text() : QString();

}

void DkFileWidget::lineEditChanged(QString path) {
	existsDirectory(path) ? leTmpPath->setStyleSheet("color:black") : leTmpPath->setStyleSheet("color:red");
}

bool DkFileWidget::existsDirectory(QString path) {
	QFileInfo* fi = new QFileInfo(path);
	return fi->exists();
}

void DkFileWidget::openWithDialog() {

	DkOpenWithDialog* openWithDialog = new DkOpenWithDialog(this);
	openWithDialog->exec();

	delete openWithDialog;
}

void DkFileWidget::tmpPathButtonPressed() {
	tmpPath = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),tmpPath);

	if (tmpPath.isEmpty())
		return;

	leTmpPath->setText(tmpPath);
}

void DkFileWidget::useTmpPathChanged(int state) {
	if (cbUseTmpPath->isChecked()) {
		lineEditChanged(tmpPath);
		leTmpPath->setDisabled(false);
		pbTmpPath->setDisabled(false);
	} else {
		leTmpPath->setStyleSheet("color:black");
		leTmpPath->setDisabled(true);
		pbTmpPath->setDisabled(true);
	}
}

// DkNetworkSettingsWidget --------------------------------------------------------------------

DkSynchronizeSettingsWidget::DkSynchronizeSettingsWidget(QWidget* parent) : DkSettingsWidget(parent) {
	showOnlyInAdvancedMode = true;

	createLayout();
	init();
}

void DkSynchronizeSettingsWidget::init() {
	connect(cbEnableNetwork, SIGNAL(stateChanged(int)), this, SLOT(enableNetworkCheckBoxChanged(int)));

	cbAllowFile->setChecked(DkSettings::Sync::allowFile);
	cbAllowImage->setChecked(DkSettings::Sync::allowImage);
	cbAllowPosition->setChecked(DkSettings::Sync::allowPosition);
	cbAllowTransformation->setChecked(DkSettings::Sync::allowTransformation);
	cbEnableNetwork->setChecked(DkSettings::Sync::enableNetworkSync);
	DkSettings::Sync::syncAbsoluteTransform ? rbSyncAbsoluteTransform->setChecked(true) : rbSyncRelativeTransform->setChecked(true);
	cbSwitchModifier->setChecked(DkSettings::Sync::switchModifier);

	enableNetworkCheckBoxChanged(0);
}

void DkSynchronizeSettingsWidget::createLayout() {
	vboxLayout = new QVBoxLayout(this);

	QGroupBox* gbSyncSettings = new QGroupBox(tr("Synchronization"), this);
	QVBoxLayout* syncSettingsLayout = new QVBoxLayout(gbSyncSettings);

	rbSyncAbsoluteTransform = new QRadioButton(tr("synchronize absolute transformation"));
	rbSyncRelativeTransform = new QRadioButton(tr("synchronize relative transformation"));

	syncSettingsLayout->addWidget(rbSyncAbsoluteTransform);
	syncSettingsLayout->addWidget(rbSyncRelativeTransform);


	gbNetworkSettings = new QGroupBox(tr("Network Synchronization"));
	QVBoxLayout* gbNetworkSettingsLayout = new QVBoxLayout(gbNetworkSettings);

	cbEnableNetwork = new QCheckBox(tr("enable network sync"), this);

	QWidget* networkSettings = new QWidget(this);
	QVBoxLayout* networkSettingsLayout = new QVBoxLayout(networkSettings);
	QLabel* clientsCan = new QLabel(tr("clients can:"), this);
	cbAllowFile = new QCheckBox(tr("switch files"), this);
	cbAllowImage = new QCheckBox(tr("send new images"), this);
	cbAllowPosition = new QCheckBox(tr("control window position"), this);
	cbAllowTransformation = new QCheckBox(tr("synchronize pan and zoom"), this);

	networkSettingsLayout->addWidget(clientsCan);
	networkSettingsLayout->addWidget(cbAllowFile);
	networkSettingsLayout->addWidget(cbAllowImage);
	networkSettingsLayout->addWidget(cbAllowPosition);
	networkSettingsLayout->addWidget(cbAllowTransformation);

	buttonGroup = new QButtonGroup(this);
	buttonGroup->setExclusive(false);
	buttonGroup->addButton(cbAllowFile);
	buttonGroup->addButton(cbAllowImage);
	buttonGroup->addButton(cbAllowPosition);
	buttonGroup->addButton(cbAllowTransformation);


	cbSwitchModifier = new QCheckBox(tr("switch ALT and CTRL key"));	

	gbNetworkSettingsLayout->addWidget(cbEnableNetwork);
	gbNetworkSettingsLayout->addWidget(networkSettings);
	vboxLayout->addWidget(gbSyncSettings);
	vboxLayout->addWidget(gbNetworkSettings);
	vboxLayout->addWidget(cbSwitchModifier);
	vboxLayout->addStretch();
}

void DkSynchronizeSettingsWidget::writeSettings() {
	DkSettings::Sync::enableNetworkSync = cbEnableNetwork->isChecked();
	DkSettings::Sync::allowFile = cbAllowFile->isChecked();
	DkSettings::Sync::allowImage = cbAllowImage->isChecked();
	DkSettings::Sync::allowPosition = cbAllowPosition->isChecked();
	DkSettings::Sync::allowTransformation = cbAllowTransformation->isChecked();
	DkSettings::Sync::syncAbsoluteTransform = rbSyncAbsoluteTransform->isChecked();
	DkSettings::Sync::switchModifier = cbSwitchModifier->isChecked();
	if (DkSettings::Sync::switchModifier) {
		DkSettings::Global::altMod = Qt::ControlModifier;
		DkSettings::Global::ctrlMod = Qt::AltModifier;
	}
	else {
		DkSettings::Global::altMod = Qt::AltModifier;
		DkSettings::Global::ctrlMod = Qt::ControlModifier;
	}
}

void DkSynchronizeSettingsWidget::enableNetworkCheckBoxChanged(int state) {
	if (cbEnableNetwork->isChecked()) {
		foreach(QAbstractButton* button, buttonGroup->buttons())
			button->setEnabled(true);
	}
	else {
		foreach(QAbstractButton* button, buttonGroup->buttons())
		button->setEnabled(false);
	}

}

// DkSettingsListView --------------------------------------------------------------------

void DkSettingsListView::previousIndex() {
	QModelIndex curIndex = currentIndex();
	
		
	if (this->model()->hasIndex(curIndex.row()-1, 0)) {
		QModelIndex newIndex = this->model()->index(curIndex.row()-1, 0);
		this->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::SelectCurrent);
		emit activated(newIndex);
	}
		
}

void DkSettingsListView::nextIndex() {
	QModelIndex curIndex = currentIndex();
	
	
	if (this->model()->hasIndex(curIndex.row()+1, 0)) {
		QModelIndex newIndex = this->model()->index(curIndex.row()+1, 0);
		this->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::SelectCurrent);
		emit activated(newIndex);
	}

}

// DkMetaDataSettings --------------------------------------------------------------------------

DkMetaDataSettingsWidget::DkMetaDataSettingsWidget(QWidget* parent) : DkSettingsWidget(parent) {
	showOnlyInAdvancedMode = true;

	createLayout();
	init();
}

void DkMetaDataSettingsWidget::init() {

	for (int i=0; i<desc_end;i++) {
		pCbMetaData[i]->setChecked(DkSettings::MetaData::metaDataBits[i]);
	}
}

void DkMetaDataSettingsWidget::createLayout() {

	//QString DkMetaDataSettingsWidget::sExifDesc = QString("Image Width;Image Length;Orientation;Make;Model;Rating;Aperture Value;Shutter Speed Value;Flash;FocalLength;") %
	//	QString("Exposure Mode;Exposure Time;User Comment;Date Time;Date Time Original;Image Description");

	//QString DkMetaDataSettingsWidget::sIptcDesc = QString("Creator CreatorTitle City Country Headline Caption Copyright Keywords");

	//QHBoxLayout* gbHbox = new QHBoxLayout(gbThumb);

	QHBoxLayout* hboxLayout = new QHBoxLayout(this);

	QGroupBox* gbCamData = new QGroupBox(tr("Camera Data"), this);
	QGroupBox* gbDescription = new QGroupBox(tr("Description"), this);

	QVBoxLayout* camDataLayout = new QVBoxLayout(gbCamData);
	QVBoxLayout* descriptionLayout = new QVBoxLayout(gbDescription);

	//QWidget* leftCol = new QWidget();
	//leftCol->setLayout(vboxLayoutLeft);
	//QWidget* rightCol = new QWidget();/
	//rightCol->setLayout(vboxLayoutRight);

	//QLabel* topLabel = new QLabel;
	QStringList sDescription;
	for (int i = 0; i  < scamDataDesc.size(); i++) 
		sDescription << qApp->translate("nmc::DkMetaData", scamDataDesc.at(i).toAscii());
	
	for (int i = 0; i  < sdescriptionDesc.size(); i++) 
		sDescription << qApp->translate("nmc::DkMetaData", sdescriptionDesc.at(i).toAscii());
	

	//QStringList sDescription = qApp->translate("nmc::DkMetaData",scamDataDesc.toAscii()).split(";") + qApp->translate("nmc::DkMetaData",sdescriptionDesc.toAscii()).split(";");

	for (int i=0; i<desc_end;i++) {
		pCbMetaData.append(new QCheckBox(sDescription.at(i), gbDescription));
	}

	for(int i=0; i<camData_end;i++) {
		camDataLayout->addWidget(pCbMetaData[i]);
	}
	camDataLayout->addStretch();

	for(int i=camData_end; i<desc_end;i++) {
		descriptionLayout->addWidget(pCbMetaData[i]);
	}

	descriptionLayout->addStretch();
	
	QGroupBox* gbOrientation = new QGroupBox(tr("Exif Orientation"), this);

	cbIgnoreOrientation = new QCheckBox(tr("Ignore Exif Orientation"), gbOrientation);
	cbIgnoreOrientation->setChecked(DkSettings::MetaData::ignoreExifOrientation);
	cbIgnoreOrientation->setToolTip(tr("Note: instead of checking this option\n you should fix your images."));

	cbSaveOrientation = new QCheckBox(tr("Save Exif Orientation"), gbOrientation);
	cbSaveOrientation->setChecked(DkSettings::MetaData::saveExifOrientation);
	cbSaveOrientation->setToolTip(tr("Note: unchecking this option decreases the speed of rotating images."));

	QVBoxLayout* orientationLayout = new QVBoxLayout(gbOrientation);
	orientationLayout->addWidget(cbIgnoreOrientation);
	orientationLayout->addWidget(cbSaveOrientation);

	QWidget* rightWidget = new QWidget(this);
	QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
	rightLayout->addWidget(gbCamData);
	rightLayout->addWidget(gbOrientation);
	rightLayout->addStretch();
	//vboxLayoutDescription->addStretch();

	hboxLayout->addWidget(gbDescription);
	hboxLayout->addWidget(rightWidget);

}

void DkMetaDataSettingsWidget::writeSettings() {

	for (int i=0; i<desc_end;i++) {
		DkSettings::MetaData::metaDataBits[i] = pCbMetaData[i]->isChecked();
	}

	DkSettings::MetaData::ignoreExifOrientation = cbIgnoreOrientation->isChecked();
	DkSettings::MetaData::saveExifOrientation = cbSaveOrientation->isChecked();
}


// DkResourceSettings --------------------------------------------------------------------
DkResourceSettingsWidgets::DkResourceSettingsWidgets(QWidget* parent) : DkSettingsWidget(parent) {
	showOnlyInAdvancedMode = true;

	stepSize = 1000;
	createLayout();
	init();
}

void DkResourceSettingsWidgets::init() {
	
	totalMemory = DkMemory::getTotalMemory();
	if (totalMemory <= 0)
		totalMemory = 2048;	// assume at least 2048 MB RAM
	
	float curCache = DkSettings::Resources::cacheMemory/totalMemory * stepSize * 100;

	connect(sliderMemory,SIGNAL(valueChanged(int)), this, SLOT(memorySliderChanged(int)));
	
	sliderMemory->setValue(curCache);
	this->memorySliderChanged(curCache);
	cbFastThumbnailPreview->setChecked(DkSettings::Resources::fastThumbnailPreview);
	cbFilterRawImages->setChecked(DkSettings::Resources::filterRawImages);
	cbRemoveDuplicates->setChecked(DkSettings::Resources::filterDuplicats);
}

void DkResourceSettingsWidgets::createLayout() {
	QVBoxLayout* widgetVBoxLayout = new QVBoxLayout(this);
	
	QGroupBox* gbCache = new QGroupBox(tr("Cache Settings"));
	QGridLayout* cacheLayout = new QGridLayout(gbCache);
	QLabel* labelPercentage = new QLabel(tr("Percentage of memory which should be used for caching:"), gbCache);
	labelPercentage->setMinimumSize(labelPercentage->sizeHint());
	sliderMemory = new QSlider(Qt::Horizontal, gbCache);
	sliderMemory->setMinimum(0);
	sliderMemory->setMaximum(10*stepSize);
	sliderMemory->setPageStep(40);
	sliderMemory->setSingleStep(40);
	sliderMemory->setContentsMargins(11,11,11,0);

	// widget starts on hide
	QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0.7);
	setGraphicsEffect(opacityEffect);

	QWidget* memoryGradient = new QWidget;
	memoryGradient->setStyleSheet("background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #185a2b, stop: 1 #66131c);");
	memoryGradient->setMinimumHeight(5);
	memoryGradient->setContentsMargins(0,0,0,0);
	memoryGradient->setWindowOpacity(0.3);
	memoryGradient->setGraphicsEffect(opacityEffect);	

	QWidget* captionWidget = new QWidget;
	captionWidget->setContentsMargins(0,0,0,0);
	
	QHBoxLayout* captionLayout = new QHBoxLayout(captionWidget);
	captionLayout->setContentsMargins(0,0,0,0);
	
	QLabel* labelMinPercent = new QLabel(QString::number(sliderMemory->minimum()/stepSize)+"%");
	labelMinPercent->setContentsMargins(0,0,0,0);
	
	QLabel* labelMaxPercent = new QLabel(QString::number(sliderMemory->maximum()/stepSize)+"%");
	labelMaxPercent->setContentsMargins(0,0,0,0);
	labelMaxPercent->setAlignment(Qt::AlignRight);
	captionLayout->addWidget(labelMinPercent);
	captionLayout->addWidget(labelMaxPercent);

	labelMemory = new QLabel;
	labelMemory->setContentsMargins(10,-5,0,0);
	labelMemory->setAlignment(Qt::AlignCenter);

	cacheLayout->addWidget(labelPercentage,0,0);
	cacheLayout->addWidget(sliderMemory,1,0);
	cacheLayout->addWidget(labelMemory,1,1);
	cacheLayout->addWidget(memoryGradient,2,0);
	cacheLayout->addWidget(captionWidget,3,0);

	QGroupBox* gbFastPreview = new QGroupBox(tr("Fast Preview Settings"));
	QGridLayout* fastPreviewLayuot = new QGridLayout(gbFastPreview);
	cbFastThumbnailPreview = new QCheckBox(tr("enable fast thumbnail preview"));
	fastPreviewLayuot->addWidget(cbFastThumbnailPreview);

	QGroupBox* gbRawLoader = new QGroupBox(tr("Raw Loader Settings"));
	
	QWidget* dupWidget = new QWidget();
	QHBoxLayout* hLayout = new QHBoxLayout(dupWidget);
	hLayout->setContentsMargins(0,0,0,0);
	
	cbRemoveDuplicates = new QCheckBox(tr("Hide Duplicates"));
	cbRemoveDuplicates->setChecked(DkSettings::Resources::filterRawImages);
	cbRemoveDuplicates->setToolTip(tr("If checked, duplicated images are not shown (e.g. RAW+JPG"));

	QLabel* preferredLabel = new QLabel(tr("Preferred Extension: "));

	QString pExt = DkSettings::Resources::preferredExtension;
	if (pExt.isEmpty()) pExt = "*.jpg";	// best default
	cmExtensions = new QComboBox();
	cmExtensions->addItems(DkImageLoader::fileFilters);
	cmExtensions->setCurrentIndex(DkImageLoader::fileFilters.indexOf(pExt));
	
	qDebug() << "preferred extension: " << pExt;

	hLayout->addWidget(cbRemoveDuplicates);
	hLayout->addWidget(preferredLabel);
	hLayout->addWidget(cmExtensions);
	hLayout->addStretch();

	QGridLayout* rawLoaderLayuot = new QGridLayout(gbRawLoader);
	cbFilterRawImages = new QCheckBox(tr("filter raw images"));
	rawLoaderLayuot->addWidget(dupWidget);
	rawLoaderLayuot->addWidget(cbFilterRawImages);

	widgetVBoxLayout->addWidget(gbCache);
	widgetVBoxLayout->addWidget(gbFastPreview);
	widgetVBoxLayout->addWidget(gbRawLoader);
	widgetVBoxLayout->addStretch();
}

void DkResourceSettingsWidgets::writeSettings() {
	
	DkSettings::Resources::cacheMemory = (sliderMemory->value()/stepSize)/100.0 * totalMemory;
	DkSettings::Resources::fastThumbnailPreview = cbFastThumbnailPreview->isChecked();
	DkSettings::Resources::filterRawImages = cbFilterRawImages->isChecked();
	DkSettings::Resources::filterDuplicats = cbRemoveDuplicates->isChecked();
	DkSettings::Resources::preferredExtension = DkImageLoader::fileFilters.at(cmExtensions->currentIndex());
}

void DkResourceSettingsWidgets::memorySliderChanged(int newValue) {
	labelMemory->setText(QString::number((double)(newValue/stepSize)/100.0*totalMemory,'f',0) + " MB / "+ QString::number(totalMemory,'f',0) + " MB");
}

// DkSpinBoxWiget --------------------------------------------------------------------
DkSpinBoxWidget::DkSpinBoxWidget(QWidget* parent) : QWidget(parent) {
	spinBox = new QSpinBox(this);
	lowerLabel = new QLabel(this);
	lowerWidget = new QWidget(this);
	vboxLayout = new QVBoxLayout(this);
	hboxLowerLayout = new QHBoxLayout(lowerWidget);

	hboxLowerLayout->addWidget(spinBox);
	hboxLowerLayout->addWidget(lowerLabel);
	hboxLowerLayout->addStretch();
	vboxLayout->addWidget(upperLabel);
	vboxLayout->addWidget(lowerWidget);

}

DkSpinBoxWidget::DkSpinBoxWidget(QString upperString, QString lowerString, int spinBoxMin, int spinBoxMax, QWidget* parent/* =0 */, int step/* =1*/) : QWidget(parent) {
	spinBox = new QSpinBox();
	spinBox->setMaximum(spinBoxMax);
	spinBox->setMinimum(spinBoxMin);
	spinBox->setSingleStep(step);
	upperLabel = new QLabel(upperString);
	lowerLabel = new QLabel(lowerString);
	lowerWidget = new QWidget();

	vboxLayout = new QVBoxLayout(this) ;
	hboxLowerLayout = new QHBoxLayout(lowerWidget);

	hboxLowerLayout->addWidget(spinBox);
	hboxLowerLayout->addWidget(lowerLabel);
	hboxLowerLayout->addStretch();
	vboxLayout->addWidget(upperLabel);
	vboxLayout->addWidget(lowerWidget);
	setMinimumSize(sizeHint());
	//adjustSize();
	//optimalSize = size();

}


// DkDoubleSpinBoxWiget --------------------------------------------------------------------
DkDoubleSpinBoxWidget::DkDoubleSpinBoxWidget(QWidget* parent) : QWidget(parent) {
	spinBox = new QDoubleSpinBox(this);
	lowerLabel = new QLabel(this);
	lowerWidget = new QWidget(this);
	vboxLayout = new QVBoxLayout(this);
	hboxLowerLayout = new QHBoxLayout(lowerWidget);

	hboxLowerLayout->addWidget(spinBox);
	hboxLowerLayout->addWidget(lowerLabel);
	hboxLowerLayout->addStretch();
	vboxLayout->addWidget(upperLabel);
	vboxLayout->addWidget(lowerWidget);

}

DkDoubleSpinBoxWidget::DkDoubleSpinBoxWidget(QString upperString, QString lowerString, float spinBoxMin, float spinBoxMax, QWidget* parent/* =0 */, int step/* =1*/, int decimals/* =2*/) : QWidget(parent) {
	spinBox = new QDoubleSpinBox(this);
	spinBox->setMaximum(spinBoxMax);
	spinBox->setMinimum(spinBoxMin);
	spinBox->setSingleStep(step);
	spinBox->setDecimals(decimals);
	upperLabel = new QLabel(upperString);
	lowerLabel = new QLabel(lowerString);
	lowerWidget = new QWidget(this);

	vboxLayout = new QVBoxLayout(this);
	hboxLowerLayout = new QHBoxLayout(lowerWidget);

	hboxLowerLayout->addWidget(spinBox);
	hboxLowerLayout->addWidget(lowerLabel);
	hboxLowerLayout->addStretch();
	vboxLayout->addWidget(upperLabel);
	vboxLayout->addWidget(lowerWidget);
	vboxLayout->addStretch();
	//adjustSize();
	//optimalSize = size();

	//setLayout(vboxLayout);
	setMinimumSize(sizeHint());
}

}
