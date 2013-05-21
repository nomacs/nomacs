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

// open with settings
QString DkSettings::Global::defaultAppPath = QString();
int DkSettings::Global::defaultAppIdx = -1;
bool DkSettings::Global::showDefaultAppDialog = true;
int DkSettings::Global::numUserChoices = 3;
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
bool DkSettings::Resources::fastThumbnailPreview = true;
bool DkSettings::Resources::filterRawImages = true;
QString DkSettings::Resources::preferredExtension = "";


void DkSettings::load() {
	
	setToDefaultSettings();

	QSettings settings;

	App::showMenuBar = settings.value("AppSettings/showMenuBar", DkSettings::App::showMenuBar).toBool();
	App::showToolBar = settings.value("AppSettings/showToolBar", DkSettings::App::showToolBar).toBool();
	App::showStatusBar = settings.value("AppSettings/showStatusBar", DkSettings::App::showStatusBar).toBool();
	
	QBitArray tmpShow = settings.value("AppSettings/showFileInfoLabel", DkSettings::App::showFileInfoLabel).toBitArray();
	if (tmpShow.size() == App::showFileInfoLabel.size())	App::showFileInfoLabel = tmpShow;
	tmpShow = settings.value("AppSettings/showScroller", DkSettings::App::showScroller).toBitArray();
	if (tmpShow.size() == App::showScroller.size())	App::showScroller = tmpShow;
	tmpShow = settings.value("AppSettings/showFilePreview", DkSettings::App::showFilePreview).toBitArray();
	if (tmpShow.size() == App::showFilePreview.size())	App::showFilePreview = tmpShow;
	tmpShow = settings.value("AppSettings/showMetaData", DkSettings::App::showMetaData).toBitArray();
	if (tmpShow.size() == App::showMetaData.size())	App::showMetaData = tmpShow;
	tmpShow = settings.value("AppSettings/showPlayer", DkSettings::App::showPlayer).toBitArray();
	if (tmpShow.size() == App::showPlayer.size())	App::showPlayer = tmpShow;
	tmpShow = settings.value("AppSettings/showHistogram", DkSettings::App::showHistogram).toBitArray();
	if (tmpShow.size() == App::showHistogram.size())	App::showHistogram = tmpShow;
	tmpShow = settings.value("AppSettings/showOverview", DkSettings::App::showOverview).toBitArray();
	if (tmpShow.size() == App::showOverview.size())	App::showOverview = tmpShow;

	App::advancedSettings = settings.value("AppSettings/advancedSettings", DkSettings::App::advancedSettings).toBool();

	Global::skipImgs = settings.value("GlobalSettings/skipImgs", DkSettings::Global::skipImgs).toInt();
	Global::numFiles = settings.value("GlobalSettings/numFiles", DkSettings::Global::numFiles).toInt();

	Global::loop = settings.value("GlobalSettings/loop", DkSettings::Global::loop).toBool();
	Global::scanSubFolders = settings.value("GlobalSettings/scanSubFolders", DkSettings::Global::scanSubFolders).toBool();
	Global::lastDir = settings.value("GlobalSettings/lastDir", DkSettings::Global::lastDir).toString();
	//GlobalSettings::lastSaveDir = settings.value("GlobalSettings/lastSaveDir", DkSettings::GlobalSettings::lastSaveDir).toString();
	Global::searchHistory = settings.value("GlobalSettings/searchHistory", DkSettings::Global::searchHistory).toStringList();
	Global::recentFolders = settings.value("GlobalSettings/recentFolders", DkSettings::Global::recentFolders).toStringList();
	Global::recentFiles = settings.value("GlobalSettings/recentFiles", DkSettings::Global::recentFiles).toStringList();
	Global::useTmpPath= settings.value("GlobalSettings/useTmpPath", DkSettings::Global::useTmpPath).toBool();
	Global::tmpPath = settings.value("GlobalSettings/tmpPath", DkSettings::Global::tmpPath).toString();
	Global::language = settings.value("GlobalSettings/language", DkSettings::Global::language).toString();

	Global::defaultAppPath = settings.value("GlobalSettings/defaultAppPath", DkSettings::Global::defaultAppPath).toString();
	Global::defaultAppIdx = settings.value("GlobalSettings/defaultAppIdx", DkSettings::Global::defaultAppIdx).toInt();
	Global::showDefaultAppDialog = settings.value("GlobalSettings/showDefaultAppDialog", DkSettings::Global::showDefaultAppDialog).toBool();
	Global::numUserChoices = settings.value("GlobalSettings/numUserChoices", DkSettings::Global::numUserChoices).toInt();
	Global::userAppPaths = settings.value("GlobalSettings/userAppPaths", DkSettings::Global::userAppPaths).toStringList();
	Global::setupPath = settings.value("GlobalSettings/setupPath", DkSettings::Global::setupPath).toString();
	Global::setupVersion = settings.value("GlobalSettings/setupVersion", DkSettings::Global::setupVersion).toString();

	Display::keepZoom = settings.value("DisplaySettings/resetMatrix", DkSettings::Display::keepZoom).toBool();
	Display::invertZoom = settings.value("DisplaySettings/invertZoom", DkSettings::Display::invertZoom).toBool();
	Display::highlightColor = settings.value("DisplaySettings/highlightColor", DkSettings::Display::highlightColor).value<QColor>();
	Display::bgColorWidget = settings.value("DisplaySettings/bgColor", DkSettings::Display::bgColorWidget).value<QColor>();
	Display::bgColor = settings.value("DisplaySettings/bgColorNoMacs", DkSettings::Display::bgColor).value<QColor>();
	Display::iconColor = settings.value("DisplaySettings/iconColor", DkSettings::Display::iconColor).value<QColor>();

	Display::bgColorFrameless = settings.value("DisplaySettings/bgColorFrameless", DkSettings::Display::bgColorFrameless).value<QColor>();
	Display::thumbSize = settings.value("DisplaySettings/thumbSize", DkSettings::Display::thumbSize).toInt();
	Display::saveThumb = settings.value("DisplaySettings/saveThumb", DkSettings::Display::saveThumb).toBool();
	Display::antiAliasing = settings.value("DisplaySettings/antiAliasing", DkSettings::Display::antiAliasing).toBool();
	Display::smallIcons = settings.value("DisplaySettings/smallIcons", DkSettings::Display::smallIcons).toBool();
	Display::toolbarGradient = settings.value("DisplaySettings/toolbarGradient", DkSettings::Display::toolbarGradient).toBool();
	Display::showBorder = settings.value("DisplaySettings/showBorder", DkSettings::Display::showBorder).toBool();
	Display::useDefaultColor = settings.value("DisplaySettings/useDefaultColor", DkSettings::Display::useDefaultColor).toBool();
	Display::defaultIconColor = settings.value("DisplaySettings/defaultIconColor", DkSettings::Display::defaultIconColor).toBool();
	Display::interpolateZoomLevel = settings.value("DisplaySettings/interpolateZoomlevel", DkSettings::Display::interpolateZoomLevel).toInt();

	QBitArray tmpMetaData = settings.value("MetaDataSettings/metaData", DkSettings::MetaData::metaDataBits).toBitArray();
	if (tmpMetaData.size() == MetaData::metaDataBits.size())
		MetaData::metaDataBits = tmpMetaData;

	SlideShow::filter = settings.value("SlideShowSettings/filter", DkSettings::SlideShow::filter).toInt();
	SlideShow::time = settings.value("SlideShowSettings/time", DkSettings::SlideShow::time).toFloat();
	SlideShow::backgroundColor = settings.value("SlideShowSettings/backgroundColor", DkSettings::SlideShow::backgroundColor).value<QColor>();
	SlideShow::silentFullscreen = settings.value("SlideShowSettings/silentFullscreen", DkSettings::SlideShow::silentFullscreen).toBool();
	QBitArray tmpDisplay = settings.value("SlideShowSettings/display", DkSettings::SlideShow::display).toBitArray();

	if (tmpDisplay.size() == SlideShow::display.size())
		SlideShow::display = tmpDisplay;

	Sync::enableNetworkSync= settings.value("SynchronizeSettings/enableNetworkSync", DkSettings::Sync::enableNetworkSync).toBool();
	Sync::allowTransformation = settings.value("SynchronizeSettings/allowTransformation", DkSettings::Sync::allowTransformation).toBool();
	Sync::allowPosition = settings.value("SynchronizeSettings/allowPosition", DkSettings::Sync::allowPosition).toBool();
	Sync::allowFile = settings.value("SynchronizeSettings/allowFile", DkSettings::Sync::allowFile).toBool();
	Sync::allowImage = settings.value("SynchronizeSettings/allowImage", DkSettings::Sync::allowImage).toBool();;
	Sync::updateDialogShown = settings.value("SynchronizeSettings/updateDialogShown", DkSettings::Sync::updateDialogShown).toBool();
	Sync::lastUpdateCheck = settings.value("SynchronizeSettings/lastUpdateCheck", DkSettings::Sync::lastUpdateCheck).toDate();
	Sync::syncAbsoluteTransform = settings.value("SynchronizeSettings/syncAbsoluteTransform", DkSettings::Sync::syncAbsoluteTransform).toBool();
	Sync::switchModifier = settings.value("SynchronizeSettings/switchModifier", DkSettings::Sync::switchModifier).toBool();

	Resources::cacheMemory = settings.value("ResourceSettings/cacheMemory", DkSettings::Resources::cacheMemory).toFloat();
	Resources::fastThumbnailPreview = settings.value("ResourceSettings/fastThumbnailPreview", DkSettings::Resources::fastThumbnailPreview).toBool();
	Resources::filterRawImages = settings.value("ResourceSettings/filterRawImages", DkSettings::Resources::filterRawImages).toBool();	
	Resources::preferredExtension = settings.value("ResourceSettings/preferredExtension", DkSettings::Resources::preferredExtension).toString();	

	if (DkSettings::Sync::switchModifier) {
		DkSettings::Global::altMod = Qt::ControlModifier;
		DkSettings::Global::ctrlMod = Qt::AltModifier;
	}
	else {
		DkSettings::Global::altMod = Qt::AltModifier;
		DkSettings::Global::ctrlMod = Qt::ControlModifier;
	}

}

void DkSettings::save() {
	QSettings settings;
	settings.setValue("AppSettings/showMenuBar", DkSettings::App::showMenuBar);
	

	int myAppMode = DkSettings::App::appMode;
	if (App::currentAppMode != mode_frameless && App::currentAppMode != mode_frameless_fullscren) {
		qDebug() << "app mode when saving: " << DkSettings::App::appMode;
		settings.setValue("AppSettings/showToolBar", DkSettings::App::showToolBar);
		settings.setValue("AppSettings/showStatusBar", DkSettings::App::showStatusBar);
	}

	settings.setValue("AppSettings/showFileInfoLabel", App::showFileInfoLabel);
	settings.setValue("AppSettings/showFilePreview", App::showFilePreview);
	settings.setValue("AppSettings/showScroller", App::showScroller);
	settings.setValue("AppSettings/showMetaData", App::showMetaData);
	settings.setValue("AppSettings/showPlayer", App::showPlayer);
	settings.setValue("AppSettings/showHistogram", App::showHistogram);
	settings.setValue("AppSettings/showOverview", App::showOverview);
	settings.setValue("AppSettings/advancedSettings", App::advancedSettings);

	settings.setValue("AppSettings/appMode", DkSettings::App::appMode);

	settings.setValue("GlobalSettings/skipImgs",Global::skipImgs);
	settings.setValue("GlobalSettings/numFiles",Global::numFiles);
	settings.setValue("GlobalSettings/loop",Global::loop);
	settings.setValue("GlobalSettings/scanSubFolders",Global::scanSubFolders);
	settings.setValue("GlobalSettings/lastDir", DkSettings::Global::lastDir);
	//settings.setValue("GlobalSettings/lastSaveDir", DkSettings::GlobalSettings::lastSaveDir);
	settings.setValue("GlobalSettings/searchHistory", DkSettings::Global::searchHistory);
	settings.setValue("GlobalSettings/recentFolders", DkSettings::Global::recentFolders);
	settings.setValue("GlobalSettings/recentFiles", DkSettings::Global::recentFiles);
	settings.setValue("GlobalSettings/useTmpPath", DkSettings::Global::useTmpPath);
	settings.setValue("GlobalSettings/tmpPath", DkSettings::Global::tmpPath);
	settings.setValue("GlobalSettings/language", DkSettings::Global::language);

	settings.setValue("GlobalSettings/defaultAppIdx", DkSettings::Global::defaultAppIdx);
	settings.setValue("GlobalSettings/defaultAppPath", DkSettings::Global::defaultAppPath);
	settings.setValue("GlobalSettings/showDefaultAppDialog", DkSettings::Global::showDefaultAppDialog);
	settings.setValue("GlobalSettings/numUserChoices", DkSettings::Global::numUserChoices);
	settings.setValue("GlobalSettings/userAppPaths", DkSettings::Global::userAppPaths);
	settings.setValue("GlobalSettings/setupPath", DkSettings::Global::setupPath);
	settings.setValue("GlobalSettings/setupVersion", DkSettings::Global::setupVersion);

	settings.setValue("DisplaySettings/resetMatrix",Display::keepZoom);
	settings.setValue("DisplaySettings/invertZoom",Display::invertZoom);
	settings.setValue("DisplaySettings/highlightColor", Display::highlightColor);
	settings.setValue("DisplaySettings/bgColor", Display::bgColorWidget);
	settings.setValue("DisplaySettings/bgColorNoMacs", Display::bgColor);
	settings.setValue("DisplaySettings/iconColor", Display::iconColor);
	settings.setValue("DisplaySettings/bgColorFrameless", Display::bgColorFrameless);
	settings.setValue("DisplaySettings/thumbSize", DkSettings::Display::thumbSize);
	settings.setValue("DisplaySettings/saveThumb", DkSettings::Display::saveThumb);
	settings.setValue("DisplaySettings/antiAliasing", DkSettings::Display::antiAliasing);
	settings.setValue("DisplaySettings/smallIcons", DkSettings::Display::smallIcons);
	settings.setValue("DisplaySettings/toolbarGradient", DkSettings::Display::toolbarGradient);
	settings.setValue("DisplaySettings/showBorder", DkSettings::Display::showBorder);
	settings.setValue("DisplaySettings/useDefaultColor", DkSettings::Display::useDefaultColor);
	settings.setValue("DisplaySettings/defaultIconColor", DkSettings::Display::defaultIconColor);
	settings.setValue("DisplaySettings/interpolateZoomlevel", DkSettings::Display::interpolateZoomLevel);


	settings.setValue("MetaDataSettings/metaData", MetaData::metaDataBits);

	settings.setValue("SlideShowSettings/filter", SlideShow::filter);
	settings.setValue("SlideShowSettings/time", SlideShow::time);
	settings.setValue("SlideShowSettings/display", SlideShow::display);
	settings.setValue("SlideShowSettings/backgroundColor", SlideShow::backgroundColor);
	settings.setValue("SlideShowSettings/silentFullscreen", SlideShow::silentFullscreen);

	settings.setValue("SynchronizeSettings/enableNetworkSync", DkSettings::Sync::enableNetworkSync);
	settings.setValue("SynchronizeSettings/allowTransformation", DkSettings::Sync::allowTransformation);
	settings.setValue("SynchronizeSettings/allowPosition", DkSettings::Sync::allowPosition);
	settings.setValue("SynchronizeSettings/allowFile", DkSettings::Sync::allowFile);
	settings.setValue("SynchronizeSettings/allowImage", DkSettings::Sync::allowImage);
	settings.setValue("SynchronizeSettings/updateDialogShown", DkSettings::Sync::updateDialogShown);
	settings.setValue("SynchronizeSettings/lastUpdateCheck", DkSettings::Sync::lastUpdateCheck);
	settings.setValue("SynchronizeSettings/syncAbsoluteTransform", DkSettings::Sync::syncAbsoluteTransform);
	settings.setValue("SynchronizeSettings/switchModifier", DkSettings::Sync::switchModifier);
	
	settings.setValue("ResourceSettings/cacheMemory", DkSettings::Resources::cacheMemory);
	settings.setValue("ResourceSettings/fastThumbnailPreview", DkSettings::Resources::fastThumbnailPreview);
	settings.setValue("ResourceSettings/filterRawImages", DkSettings::Resources::filterRawImages);
	settings.setValue("ResourceSettings/preferredExtension", DkSettings::Resources::preferredExtension);

	qDebug() << "settings saved";
}

void DkSettings::setToDefaultSettings() {

	DkSettings::App::showMenuBar = true;
	DkSettings::App::showToolBar = true;
	DkSettings::App::showStatusBar = false;
	DkSettings::App::showFileInfoLabel = QBitArray(DkSettings::mode_end, true);
	DkSettings::App::showFilePreview = QBitArray(DkSettings::mode_end, false);
	DkSettings::App::showScroller = QBitArray(DkSettings::mode_end, false);
	DkSettings::App::showMetaData = QBitArray(DkSettings::mode_end, false);
	DkSettings::App::showPlayer = QBitArray(DkSettings::mode_end, false);
	DkSettings::App::showHistogram = QBitArray(DkSettings::mode_end, false);
	DkSettings::App::showOverview = QBitArray(DkSettings::mode_end, true);
	DkSettings::App::advancedSettings = false;

	// now set default show options
	DkSettings::App::showFileInfoLabel.setBit(DkSettings::mode_default, false);
	DkSettings::App::showFileInfoLabel.setBit(DkSettings::mode_contrast, false);


	DkSettings::App::appMode = 0;
	
	DkSettings::Global::skipImgs = 10;
	DkSettings::Global::numFiles = 10;
	DkSettings::Global::loop = false;
	DkSettings::Global::scanSubFolders = true;
	DkSettings::Global::lastDir = QString();
	DkSettings::Global::lastSaveDir = QString();
	DkSettings::Global::recentFiles = QStringList();
	DkSettings::Global::searchHistory = QStringList();
	DkSettings::Global::recentFolders = QStringList();
	DkSettings::Global::useTmpPath = false;
	DkSettings::Global::tmpPath = QString();
	DkSettings::Global::language = QString();
	DkSettings::Global::defaultAppIdx = -1;
	DkSettings::Global::defaultAppPath = QString();
	DkSettings::Global::showDefaultAppDialog = true;
	DkSettings::Global::numUserChoices = 3;
	DkSettings::Global::userAppPaths = QStringList();
	DkSettings::Global::setupPath = "";
	DkSettings::Global::setupVersion = "";

#ifdef Q_WS_X11
	DkSettings::Sync::switchModifier = true;
	DkSettings::Global::altMod = Qt::ControlModifier;
	DkSettings::Global::ctrlMod = Qt::AltModifier;
#else
	DkSettings::Sync::switchModifier = false;
	DkSettings::Global::altMod = Qt::AltModifier;
	DkSettings::Global::ctrlMod = Qt::ControlModifier;
#endif


	DkSettings::Display::keepZoom = true;
	DkSettings::Display::invertZoom = false;
	DkSettings::Display::highlightColor = QColor(0, 204, 255);
	DkSettings::Display::bgColorWidget = QColor(0, 0, 0, 100);
	DkSettings::Display::bgColor = QColor(100, 100, 100, 255);
	DkSettings::Display::bgColor = QColor(219, 89, 2, 255);
	DkSettings::Display::bgColorFrameless = QColor(0, 0, 0, 180);
	DkSettings::Display::thumbSize = 100;
	DkSettings::Display::saveThumb = false;
	DkSettings::Display::antiAliasing = true;
	DkSettings::Display::smallIcons = true;
#ifdef Q_WS_WIN
	DkSettings::Display::toolbarGradient = true;
#else
	DkSettings::Display::toolbarGradient = false;
#endif
	DkSettings::Display::showBorder = true;
	DkSettings::Display::useDefaultColor = true;
	DkSettings::Display::defaultIconColor = true;
	DkSettings::Display::interpolateZoomLevel = 200;

	DkSettings::SlideShow::filter = 0;
	DkSettings::SlideShow::time = 3.0;
	DkSettings::SlideShow::display = QBitArray(DkDisplaySettingsWidget::display_end, true);
	DkSettings::SlideShow::backgroundColor = QColor(217, 219, 228, 100);
	DkSettings::SlideShow::silentFullscreen = true;


	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_size] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_orientation] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_make] = true;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_model] = true;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_aperture] = true;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_iso] = true;
	//DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_shutterspeed] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_flash] = true;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_focallength] = true;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_exposuremode] = true;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::camData_exposuretime] = true;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_rating] = true;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_usercomment] = true;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_date] = true;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_datetimeoriginal] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_imagedescription] = true;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_creator] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_creatortitle] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_city] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_country] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_headline] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_caption] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_copyright] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_keywords] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_path] = false;
	DkSettings::MetaData::metaDataBits[DkMetaDataSettingsWidget::desc_filesize] = false;


	DkSettings::Sync::enableNetworkSync = false;
	DkSettings::Sync::allowTransformation = true;
	DkSettings::Sync::allowPosition = true;
	DkSettings::Sync::allowFile = true;
	DkSettings::Sync::allowImage = true;
	DkSettings::Sync::updateDialogShown = false;
	DkSettings::Sync::lastUpdateCheck = QDate(1970 , 1, 1);
	DkSettings::Sync::syncAbsoluteTransform = true;

	DkSettings::Resources::cacheMemory = 0;
	DkSettings::Resources::fastThumbnailPreview = true;
	DkSettings::Resources::filterRawImages = true;
	DkSettings::Resources::preferredExtension = "";

	qDebug() << "ok... default settings are set";

	emit setToDefaultSettingsSignal();
}

// DkSettingsDialog --------------------------------------------------------------------
DkSettingsDialog::DkSettingsDialog(QWidget* parent) : QDialog(parent) {

	//this->resize(600,420);

	s = new DkSettings();

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

	connect(listView, SIGNAL(activated(const QModelIndex &)), this, SLOT(listViewSelected(const QModelIndex &)));
	connect(listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(listViewSelected(const QModelIndex &)));
	connect(listView, SIGNAL(entered(const QModelIndex &)), this, SLOT(listViewSelected(const QModelIndex &)));

	connect(buttonOk, SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));
	connect(s, SIGNAL(setToDefaultSettingsSignal()), this, SLOT(initWidgets()));
	connect(globalSettingsWidget, SIGNAL(applyDefault()), this, SLOT(setToDefault()));
	connect(cbAdvancedSettings, SIGNAL(stateChanged(int)), this, SLOT(advancedSettingsChanged(int)));
}

DkSettingsDialog::~DkSettingsDialog() {
	if (s) 
		delete s; 
	s=0;

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

	cbAdvancedSettings = new QCheckBox("Advanced");

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
	globalSettingsWidget = new DkGlobalSettingsWidget(this);
	displaySettingsWidget = new DkDisplaySettingsWidget(this);
	slideshowSettingsWidget = new DkFileWidget(this);
	synchronizeSettingsWidget = new DkSynchronizeSettingsWidget(this);
	exifSettingsWidget = new DkMetaDataSettingsWidget(this);
	resourceSettingsWidget = new DkResourceSettingsWidgets(this);

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

	highlightColorChooser = new DkColorChooser(QColor(0, 204, 255), tr("Highlight Color"));
	highlightColorChooser->setColor(DkSettings::Display::highlightColor);

	iconColorChooser = new DkColorChooser(QColor(219, 89, 2, 255), tr("Icon Color"));
	iconColorChooser->setColor(DkSettings::Display::iconColor);
	connect(iconColorChooser, SIGNAL(resetClicked()), this, SLOT(iconColorReset()));

	bgColorChooser = new DkColorChooser(QColor(100, 100, 100, 255), tr("Background Color"));
	bgColorChooser->setColor(DkSettings::Display::bgColor);
	connect(bgColorChooser, SIGNAL(resetClicked()), this, SLOT(bgColorReset()));

	bgColorWidgetChooser = new DkColorChooser(QColor(0, 0, 0, 100), tr("Widget Color"));
	bgColorWidgetChooser->setColor((DkSettings::App::appMode == DkSettings::mode_frameless) ?
		DkSettings::Display::bgColorFrameless : DkSettings::Display::bgColorWidget);

	fullscreenColChooser = new DkColorChooser(QColor(86,86,90), "Fullscreen Color", this);
	fullscreenColChooser->setColor(DkSettings::SlideShow::backgroundColor);

	displayTimeSpin = new DkDoubleSpinBoxWidget(tr("Display Time:"), tr("sec"), 0.1f, 99, this, 1, 1);

	QWidget* langWidget = new QWidget(this);
	QGridLayout* langLayout = new QGridLayout(langWidget);
	langLayout->setMargin(0);
	QLabel* langLabel = new QLabel("choose language:");
	langCombo = new QComboBox(this);

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

	QLabel* translateLabel = new QLabel("<a href=\"http://www.nomacs.org/how-to-translate-nomacs/\">translate nomacs</a>", this);
	translateLabel->setToolTip(tr("if you want to help us and translate nomacs"));
	QFont font;
	font.setPointSize(7);
	translateLabel->setFont(font);
	translateLabel->setOpenExternalLinks(true);

	langLayout->addWidget(langLabel,0,0);
	langLayout->addWidget(langCombo,1,0);
	langLayout->addWidget(translateLabel,2,0,Qt::AlignRight);

	QWidget* showBarsWidget = new QWidget;
	QVBoxLayout* showBarsLayout = new QVBoxLayout(showBarsWidget);
	cbShowMenu = new QCheckBox(tr("show Menu"), this);
	cbShowToolbar = new QCheckBox(tr("show Toolbar"), this);
	cbShowStatusbar = new QCheckBox(tr("show Statusbar"), this);
	cbSmallIcons = new QCheckBox(tr("small icons"), this);
	cbToolbarGradient = new QCheckBox(tr("Toolbar Gradient"), this);
	showBarsLayout->addWidget(cbShowMenu);
	showBarsLayout->addWidget(cbShowToolbar);
	showBarsLayout->addWidget(cbShowStatusbar);
	showBarsLayout->addWidget(cbSmallIcons);
	showBarsLayout->addWidget(cbToolbarGradient);

	// set to default
	QWidget* defaultSettingsWidget = new QWidget(this);
	QHBoxLayout* defaultSettingsLayout = new QHBoxLayout(defaultSettingsWidget);
	defaultSettingsLayout->setContentsMargins(11,0,11,0);
	defaultSettingsLayout->setDirection(QHBoxLayout::RightToLeft);
	buttonDefaultSettings = new QPushButton(tr("Apply default settings"), this);
	defaultSettingsLayout->addWidget(buttonDefaultSettings);
	defaultSettingsLayout->addStretch();

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
	widgetLayout->addLayout(rightLayout);
}

void DkGlobalSettingsWidget::writeSettings() {
	DkSettings::App::showMenuBar = cbShowMenu->isChecked();
	DkSettings::App::showStatusBar = cbShowStatusbar->isChecked();
	DkSettings::App::showToolBar = cbShowToolbar->isChecked();
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

	cbSilentFullscreen->setChecked(DkSettings::SlideShow::silentFullscreen);
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

	QVBoxLayout* vboxLayoutLeft = new QVBoxLayout(gbCamData);
	QVBoxLayout* vboxLayoutRight = new QVBoxLayout(gbDescription);

	//QWidget* leftCol = new QWidget();
	//leftCol->setLayout(vboxLayoutLeft);
	//QWidget* rightCol = new QWidget();/
	//rightCol->setLayout(vboxLayoutRight);

	hboxLayout->addWidget(gbDescription);
	hboxLayout->addWidget(gbCamData);

	//QLabel* topLabel = new QLabel;
	QStringList sDescription;
	for (int i = 0; i  < scamDataDesc.size(); i++) 
		sDescription << qApp->translate("nmc::DkMetaData", scamDataDesc.at(i).toAscii());
	
	for (int i = 0; i  < sdescriptionDesc.size(); i++) 
		sDescription << qApp->translate("nmc::DkMetaData", sdescriptionDesc.at(i).toAscii());
	

	//QStringList sDescription = qApp->translate("nmc::DkMetaData",scamDataDesc.toAscii()).split(";") + qApp->translate("nmc::DkMetaData",sdescriptionDesc.toAscii()).split(";");

	for (int i=0; i<desc_end;i++) {
		pCbMetaData.append(new QCheckBox(sDescription.at(i), this));
	}

	for(int i=0; i<camData_end;i++) {
		vboxLayoutLeft->addWidget(pCbMetaData[i]);
	}
	vboxLayoutLeft->addStretch();

	for(int i=camData_end; i<desc_end;i++) {
		vboxLayoutRight->addWidget(pCbMetaData[i]);
	}
	vboxLayoutRight->addStretch();
}

void DkMetaDataSettingsWidget::writeSettings() {

	for (int i=0; i<desc_end;i++) {
		DkSettings::MetaData::metaDataBits[i] = pCbMetaData[i]->isChecked();
	}
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
		totalMemory = 512;	// assume at least 512 MB RAM
	
	float curCache = DkSettings::Resources::cacheMemory/totalMemory * stepSize * 100;

	connect(sliderMemory,SIGNAL(valueChanged(int)), this, SLOT(memorySliderChanged(int)));
	
	sliderMemory->setValue(curCache);
	this->memorySliderChanged(curCache);
	cbFastThumbnailPreview->setChecked(DkSettings::Resources::fastThumbnailPreview);
	cbFilterRawImages->setChecked(DkSettings::Resources::filterRawImages);
}

void DkResourceSettingsWidgets::createLayout() {
	QVBoxLayout* widgetVBoxLayout = new QVBoxLayout(this);
	
	QGroupBox* gbCache = new QGroupBox(tr("Cache Settings"));
	QGridLayout* cacheLayout = new QGridLayout(gbCache);
	QLabel* labelPercentage = new QLabel(tr("Percentage of memory which should be used for caching:"));
	sliderMemory = new QSlider(Qt::Horizontal);
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
	
	cbRemoveDuplicates = new QCheckBox(tr("Hide Duplicates"));
	cbRemoveDuplicates->setChecked(!DkSettings::Resources::preferredExtension.isEmpty());
	cbRemoveDuplicates->setToolTip(tr("If checked, duplicated images are not shown (e.g. RAW+JPG"));

	QLabel* preferredLabel = new QLabel(tr("Preferred Extension: "));

	cmExtensions = new QComboBox();
	cmExtensions->addItems(DkImageLoader::fileFilters);
	
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
	adjustSize();
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
	spinBox = new QDoubleSpinBox();
	spinBox->setMaximum(spinBoxMax);
	spinBox->setMinimum(spinBoxMin);
	spinBox->setSingleStep(step);
	spinBox->setDecimals(decimals);
	upperLabel = new QLabel(upperString);
	lowerLabel = new QLabel(lowerString);
	lowerWidget = new QWidget();

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

}

}
