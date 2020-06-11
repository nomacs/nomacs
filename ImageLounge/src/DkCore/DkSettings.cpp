/*******************************************************************************************************
 DkSettings.cpp
 Created on:	07.07.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2016 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2016 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2016 Florian Kleber <florian@nomacs.org>

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
#include "DkUtils.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <iostream>
#include <cassert>

#include <QImageReader>
#include <QScreen>
#include <QDesktopServices>
#include <QTranslator>
#include <QFileInfo>
#include <QDebug>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QDir>
#include <QApplication>
#include <QThreadPool>

#ifdef Q_OS_WIN
#include "Shobjidl.h"
#endif
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

DkSettings::DkSettings() {

	// cache settings path
	mSettingsPath = settingsPath();
	init();
}

void DkSettings::init() {
	scamDataDesc = QStringList() << 
		QT_TRANSLATE_NOOP("nmc::DkMetaData","Image Size") <<
		QT_TRANSLATE_NOOP("nmc::DkMetaData","Orientation") <<
		QT_TRANSLATE_NOOP("nmc::DkMetaData","Make") <<
		QT_TRANSLATE_NOOP("nmc::DkMetaData","Model") <<
		QT_TRANSLATE_NOOP("nmc::DkMetaData","Aperture Value") <<
		QT_TRANSLATE_NOOP("nmc::DkMetaData","ISO") <<
		QT_TRANSLATE_NOOP("nmc::DkMetaData","Flash") <<
		QT_TRANSLATE_NOOP("nmc::DkMetaData","Focal Length") <<
		QT_TRANSLATE_NOOP("nmc::DkMetaData","Exposure Mode") <<
		QT_TRANSLATE_NOOP("nmc::DkMetaData","Exposure Time") <<
		QT_TRANSLATE_NOOP("nmc::DkMetaData", "Compression");

	sdescriptionDesc = QStringList() <<
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
}

double DkSettings::dpiScaleFactor(QWidget *widget) const {
    
	double dpi = 96.0;
    if (widget) {
        dpi = (double) widget->logicalDpiX();
    } else {
        QList<QScreen*> screens = QApplication::screens();
        for(const QScreen* s : screens) {
            if (s->logicalDotsPerInch() > dpi) 
				dpi = s->logicalDotsPerInch();
        }
    }
    if (dpi < 96.0) dpi = 96.0;
    return dpi / 96.0;
}

int DkSettings::effectiveIconSize(QWidget *widget) const {
    return qRound(display_p.iconSize * dpiScaleFactor(widget));
}

int DkSettings::effectiveThumbSize(QWidget *widget) const {
    return qRound(display_p.thumbSize * dpiScaleFactor(widget));
}

int DkSettings::effectiveThumbPreviewSize(QWidget *widget) const {
    return qRound(display_p.thumbPreviewSize * dpiScaleFactor(widget));
}

QStringList DkSettings::translatedCamData() const {
	return scamDataDesc;
}

QStringList DkSettings::translatedDescriptionData() const {
	return sdescriptionDesc;
}

void DkSettings::initFileFilters() {

	if (!app_p.openFilters.empty())
		return;

	QList<QByteArray> qtFormats = QImageReader::supportedImageFormats();

	// formats we can save
	if (qtFormats.contains("png"))		app_p.saveFilters.append("PNG (*.png)");
	if (qtFormats.contains("jpg"))		app_p.saveFilters.append("JPEG (*.jpg *.jpeg *.jpe)");
	if (qtFormats.contains("jp2"))		app_p.saveFilters.append("JPEG 2000 (*.jp2 *.j2k *.jpf *.jpx *.jpm *.jpgx)");
	if (qtFormats.contains("tif"))		app_p.saveFilters.append("TIFF (*.tif *.tiff)");
	if (qtFormats.contains("bmp"))		app_p.saveFilters.append("Windows Bitmap (*.bmp)");
	if (qtFormats.contains("ppm"))		app_p.saveFilters.append("Portable Pixmap (*.ppm)");
	if (qtFormats.contains("xbm"))		app_p.saveFilters.append("X11 Bitmap (*.xbm)");
	if (qtFormats.contains("xpm"))		app_p.saveFilters.append("X11 Pixmap (*.xpm)");
	if (qtFormats.contains("dds"))		app_p.saveFilters.append("Direct Draw Surface (*.dds)");
	if (qtFormats.contains("wbmp"))		app_p.saveFilters.append("Wireless Bitmap (*.wbmp)");
	//if (qtFormats.contains("icns"))		app_p.saveFilters.append("Apple Icon Image (*.icns)");

	if (qtFormats.contains("webp"))		app_p.saveFilters.append("WebP (*.webp)");
	if (qtFormats.contains("avif"))		app_p.saveFilters.append("AV1 Image File Format (*.avif)");

#ifdef Q_OS_WIN
	if (qtFormats.contains("ico"))		app_p.saveFilters.append("Icon Files (*.ico)");
#endif

	// formats we can load
	app_p.openFilters += app_p.saveFilters;
	if (qtFormats.contains("gif"))		app_p.openFilters.append("Graphic Interchange Format (*.gif)");
	if (qtFormats.contains("pbm"))		app_p.openFilters.append("Portable Bitmap (*.pbm)");
	if (qtFormats.contains("pgm"))		app_p.openFilters.append("Portable Graymap (*.pgm)");
	if (qtFormats.contains("tga"))		app_p.openFilters.append("Truvision Graphics Adapter (*.tga)");
	if (qtFormats.contains("mng"))		app_p.openFilters.append("Multi-Image Network Graphics (*.mng)");
	if (qtFormats.contains("cur"))		app_p.openFilters.append("Windows Cursor Files (*.cur)");
	if (qtFormats.contains("icns"))		app_p.openFilters.append("Apple Icon Image (*.icns)");
	if (qtFormats.contains("svgz"))		app_p.openFilters.append("Scalable Vector Graphics (*.svg *.svgz)");
	if (qtFormats.contains("avifs"))	app_p.openFilters.append("AVIF image sequence (*.avifs)");

#ifndef Q_OS_WIN
	if (qtFormats.contains("ico"))		app_p.openFilters.append("Icon Files (*.ico)");
#endif

#ifdef WITH_LIBRAW
	// raw format
	app_p.rawFilters.append("Nikon Raw (*.nef *.nrw)");
	app_p.rawFilters.append("Canon Raw (*.crw *.cr2 *.cr3)");
	app_p.rawFilters.append("Sony Raw (*.arw)");
	app_p.rawFilters.append("Digital Negativ (*.dng)");
	app_p.rawFilters.append("Panasonic Raw (*.raw *.rw2)");
	app_p.rawFilters.append("Minolta Raw (*.mrw)");
	app_p.rawFilters.append("Samsung Raw (*.srw)");
	app_p.rawFilters.append("Olympus Raw (*.orf)");
	app_p.rawFilters.append("Hasselblad Raw (*.3fr)");
	app_p.rawFilters.append("Sigma Raw (*.x3f)");
	app_p.rawFilters.append("Leaf Raw (*.mos)");
	app_p.rawFilters.append("Pentax Raw (*.pef)");
	app_p.rawFilters.append("Phase One (*.iiq)");
	app_p.rawFilters.append("Fujifilm Raw (*.raf)");

	app_p.openFilters += app_p.rawFilters;
#endif

	// stereo formats
	app_p.openFilters.append("JPEG Stereo (*.jps)");
	app_p.openFilters.append("PNG Stereo (*.pns)");
	app_p.openFilters.append("Multi Picture Object (*.mpo)");

	// other formats
	app_p.openFilters.append("Adobe Photoshop (*.psd)");
	app_p.openFilters.append("Large Document Format (*.psb)");

	// archive formats
	app_p.containerFilters.append("ZIP Archive (*.zip)");
	app_p.containerFilters.append("Microsoft Word Document (*.docx)");
	app_p.containerFilters.append("Microsoft PowerPoint Document (*.pptx)");
	app_p.containerFilters.append("Microsoft Excel Document (*.xlsx)");
	app_p.containerFilters.append("Comic Book Archive (*.cbz)");
	app_p.containerFilters.append("Krita (*.kra)");
	
	app_p.openFilters += app_p.containerFilters;

	app_p.containerRawFilters = "*.docx *.pptx *.xlsx *.zip";

	// finally: fabians filter & OpenCV vecs
	app_p.openFilters.append("OpenCV Haar Training (*.vec)");
	app_p.openFilters.append("Rohkost (*.roh)");

    // drif images
    app_p.openFilters.append("Developers Raw Image Format (*.drif)");

	// load user filters
	DefaultSettings settings;
	app_p.openFilters += settings.value("ResourceSettings/userFilters", QStringList()).toStringList();

	for (int idx = 0; idx < app_p.openFilters.size(); idx++) {

		QString cFilter = app_p.openFilters[idx];
		cFilter = cFilter.section(QRegExp("(\\(|\\))"), 1);
		cFilter = cFilter.replace(")", "");
		app_p.fileFilters += cFilter.split(" ");
	}

	QString allFilters = app_p.fileFilters.join(" ");

	// add unknown formats from Qt plugins
	for (int idx = 0; idx < qtFormats.size(); idx++) {

		if (!allFilters.contains(qtFormats.at(idx))) {
			app_p.openFilters.append("Image Format (*." + qtFormats.at(idx) + ")");
			app_p.fileFilters.append("*." + qtFormats.at(idx));
		}
	}

	app_p.openFilters.prepend("Image Files (" + app_p.fileFilters.join(" ") + ")");

//#ifdef Q_OS_WIN
//	app_p.fileFilters.append("*.lnk");
//#endif

}

void DkSettings::loadTranslation(const QString& fileName, QTranslator& translator) {

	QStringList translationDirs = getTranslationDirs();

	for (int idx = 0; idx < translationDirs.size(); idx++) {

		if (translator.load(fileName, translationDirs[idx])) {
			qDebugClean() << "translation loaded from: " << translationDirs[idx] << "/" << fileName;
			break;
		}
	}
}

QStringList DkSettings::getTranslationDirs() {
	
	QStringList trDirs;
	trDirs << DkUtils::getTranslationPath();
	trDirs << qApp->applicationDirPath();
	
	QStringList rDirs;
	rDirs << qApp->applicationDirPath();
	rDirs << QStandardPaths::standardLocations(QStandardPaths::DataLocation);

	for (const QString& d : rDirs)
		trDirs << d + QDir::separator() + "translations";

	trDirs.removeDuplicates();

	return trDirs;
}

void DkSettings::load() {

	DefaultSettings s;
	load(s);
}

void DkSettings::load(QSettings& settings, bool defaults) {

	applyDefaultsFromFile();	// copies entries from "default.ini" to the user settings file
	setToDefaultSettings();
	qInfoClean() << "loading settings from: " << settings.fileName();

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
	tmpShow = settings.value("showComment", app_p.showComment).toBitArray();
	if (tmpShow.size() == app_p.showComment.size())	
		app_p.showComment = tmpShow;
	tmpShow = settings.value("showExplorer", app_p.showExplorer).toBitArray();
	if (tmpShow.size() == app_p.showExplorer.size())	
		app_p.showExplorer = tmpShow;
	tmpShow = settings.value("showMetaDataDock", app_p.showMetaDataDock).toBitArray();
	if (tmpShow.size() == app_p.showMetaDataDock.size())	
		app_p.showMetaDataDock = tmpShow;
	tmpShow = settings.value("showEditDock", app_p.showEditDock).toBitArray();
	if (tmpShow.size() == app_p.showEditDock.size())	
		app_p.showEditDock = tmpShow;
	tmpShow = settings.value("showHistoryDock", app_p.showHistoryDock).toBitArray();
	if (tmpShow.size() == app_p.showHistoryDock.size())	
		app_p.showHistoryDock = tmpShow;
	tmpShow = settings.value("showLogDock", app_p.showLogDock).toBitArray();
	if (tmpShow.size() == app_p.showLogDock.size())
		app_p.showLogDock = tmpShow;

	app_p.hideAllPanels = settings.value("hideAllPanels", app_p.hideAllPanels).toBool();
	app_p.closeOnEsc = settings.value("closeOnEsc", app_p.closeOnEsc).toBool();
	app_p.showRecentFiles = settings.value("showRecentFiles", app_p.showRecentFiles).toBool();
	app_p.useLogFile = settings.value("useLogFile", app_p.useLogFile).toBool();
	app_p.defaultJpgQuality = settings.value("defaultJpgQuality", app_p.defaultJpgQuality).toInt();

	QStringList tmpFileFilters = app_p.fileFilters;
	QStringList tmpContainerFilters = app_p.containerRawFilters.split(" ");
	for (int idx = 0; idx < tmpContainerFilters.size(); idx++) {
		tmpFileFilters.removeAll(tmpContainerFilters.at(idx));
	}
	app_p.browseFilters = settings.value("browseFilters", tmpFileFilters).toStringList();

	// double-check (if user removes all filters he can't browse anymore - so override this case)
	if (app_p.browseFilters.empty())
		app_p.browseFilters = tmpFileFilters;

	app_p.registerFilters = settings.value("registerFilters", app_p.registerFilters).toStringList();
	app_p.advancedSettings = settings.value("advancedSettings", app_p.advancedSettings).toBool();

	settings.endGroup();
	// Global Settings --------------------------------------------------------------------
	settings.beginGroup("GlobalSettings");

	global_p.skipImgs = settings.value("skipImgs", global_p.skipImgs).toInt();
	global_p.checkOpenDuplicates = settings.value("checkOpenDuplicates", global_p.checkOpenDuplicates).toBool();
	global_p.extendedTabs = settings.value("extendedTabs", global_p.extendedTabs).toBool();

	global_p.loop = settings.value("loop", global_p.loop).toBool();
	global_p.scanSubFolders = settings.value("scanRecursive", global_p.scanSubFolders).toBool();
	global_p.lastDir = settings.value("lastDir", global_p.lastDir).toString();
	global_p.searchHistory = settings.value("searchHistory", global_p.searchHistory).toStringList();
	global_p.recentFolders = settings.value("recentFolders", global_p.recentFolders).toStringList();
	global_p.pinnedFiles = settings.value("pinnedFiles", global_p.pinnedFiles).toStringList();
	global_p.recentFiles = settings.value("recentFiles", global_p.recentFiles).toStringList();
	global_p.logRecentFiles = settings.value("logRecentFiles", global_p.logRecentFiles).toBool();
	global_p.askToSaveDeletedFiles = settings.value("askToSaveDeletedFiles", global_p.askToSaveDeletedFiles).toBool();
	global_p.tmpPath = settings.value("tmpPath", global_p.tmpPath).toString();
	global_p.language = settings.value("language", global_p.language).toString();
	global_p.numThreads = settings.value("numThreads", global_p.numThreads).toInt();

	global_p.sortMode = settings.value("sortMode", global_p.sortMode).toInt();
	global_p.sortDir = settings.value("sortDir", global_p.sortDir).toInt();
	global_p.setupPath = settings.value("setupPath", global_p.setupPath).toString();
	global_p.setupVersion = settings.value("setupVersion", global_p.setupVersion).toString();
	global_p.zoomOnWheel = settings.value("zoomOnWheel", global_p.zoomOnWheel).toBool();
	global_p.horZoomSkips = settings.value("horZoomSkips", global_p.horZoomSkips).toBool();
	global_p.doubleClickForFullscreen = settings.value("doubleClickForFullscreen", global_p.doubleClickForFullscreen).toBool();
	global_p.showBgImage = settings.value("showBgImage", global_p.showBgImage).toBool();

	settings.endGroup();
	// Display Settings --------------------------------------------------------------------
	settings.beginGroup("DisplaySettings");

	display_p.keepZoom = settings.value("keepZoom", display_p.keepZoom).toInt();
	display_p.invertZoom = settings.value("invertZoom", display_p.invertZoom).toBool();
	display_p.zoomToFit = settings.value("zoomToFit", display_p.zoomToFit).toBool();
	display_p.highlightColor = QColor::fromRgba(settings.value("highlightColorRGBA", display_p.highlightColor.rgba()).toInt());
	display_p.hudBgColor = QColor::fromRgba(settings.value("bgColorWidgetRGBA", display_p.hudBgColor.rgba()).toInt());
	display_p.hudFgdColor = QColor::fromRgba(settings.value("fontColorRGBA", display_p.hudFgdColor.rgba()).toInt());
	display_p.bgColor = QColor::fromRgba(settings.value("bgColorNoMacsRGBA", display_p.bgColor.rgba()).toInt());
	display_p.iconColor = QColor::fromRgba(settings.value("iconColorRGBA", display_p.iconColor.rgba()).toInt());
	display_p.bgColorFrameless = QColor::fromRgba(settings.value("bgColorFramelessRGBA", display_p.bgColorFrameless.rgba()).toInt());
	display_p.thumbSize = settings.value("thumbSize", display_p.thumbSize).toInt();
	display_p.iconSize = settings.value("iconSize", display_p.iconSize).toInt();
	display_p.thumbPreviewSize = settings.value("thumbPreviewSize", display_p.thumbPreviewSize).toInt();
	//display_p.saveThumb = settings.value("saveThumb", display_p.saveThumb).toBool();
	display_p.antiAliasing = settings.value("antiAliasing", display_p.antiAliasing).toBool();
	display_p.highQualityAntiAliasing = settings.value("highQualityAntiAliasing", display_p.highQualityAntiAliasing).toBool();
	display_p.showCrop = settings.value("showCrop", display_p.showCrop).toBool();
	display_p.histogramStyle = settings.value("histogramStyle", display_p.histogramStyle).toInt();
	display_p.tpPattern = settings.value("tpPattern", display_p.tpPattern).toBool();
	display_p.showNavigation = settings.value("showNavigation", display_p.showNavigation).toBool();
	display_p.themeName = settings.value("themeName312", display_p.themeName).toString();
	display_p.showBorder = settings.value("showBorder", display_p.showBorder).toBool();
	display_p.displaySquaredThumbs = settings.value("displaySquaredThumbs", display_p.displaySquaredThumbs).toBool();
	display_p.showThumbLabel = settings.value("showThumbLabel", display_p.showThumbLabel).toBool();
	display_p.showScrollBars = settings.value("showScrollBars", display_p.showScrollBars).toBool();
	display_p.animationDuration = settings.value("fadeSec", display_p.animationDuration).toFloat();
	display_p.alwaysAnimate = settings.value("alwaysAnimate", display_p.alwaysAnimate).toBool();
	display_p.transition = (TransitionMode)settings.value("transition", display_p.transition).toInt();
	display_p.defaultBackgroundColor = settings.value("useDefaultColor", display_p.defaultBackgroundColor).toBool();
	display_p.defaultIconColor = settings.value("defaultIconColor", display_p.defaultIconColor).toBool();
	display_p.interpolateZoomLevel = settings.value("interpolateZoomlevel", display_p.interpolateZoomLevel).toInt();

	settings.endGroup();
	// MetaData Settings --------------------------------------------------------------------
	settings.beginGroup("MetaDataSettings");

	meta_p.ignoreExifOrientation = settings.value("ignoreExifOrientation", meta_p.ignoreExifOrientation).toBool();
	meta_p.saveExifOrientation = settings.value("saveExifOrientation", meta_p.saveExifOrientation).toBool();

	settings.endGroup();
	// SlideShow Settings --------------------------------------------------------------------
	settings.beginGroup("SlideShowSettings");

	slideShow_p.filter = settings.value("filter", slideShow_p.filter).toInt();
	slideShow_p.time = settings.value("time", slideShow_p.time).toFloat();
	slideShow_p.showPlayer = settings.value("showPlayer", slideShow_p.showPlayer).toBool();
	slideShow_p.moveSpeed = settings.value("moveSpeed", slideShow_p.moveSpeed).toFloat();
	slideShow_p.backgroundColor = QColor::fromRgba(settings.value("backgroundColorRGBA", slideShow_p.backgroundColor.rgba()).toInt());
	slideShow_p.silentFullscreen = settings.value("silentFullscreen", slideShow_p.silentFullscreen).toBool();
	QBitArray tmpDisplay = settings.value("display", slideShow_p.display).toBitArray();

	if (tmpDisplay.size() == slideShow_p.display.size())
		slideShow_p.display = tmpDisplay;

	settings.endGroup();
	// Synchronize Settings --------------------------------------------------------------------
	settings.beginGroup("SynchronizeSettings");

	sync_p.checkForUpdates = settings.value("checkForUpdates", sync_p.checkForUpdates).toBool();
	sync_p.disableUpdateInteraction = settings.value("disableUpdateInteraction", sync_p.disableUpdateInteraction).toBool();
	sync_p.updateDialogShown = settings.value("updateDialogShown", sync_p.updateDialogShown).toBool();
	sync_p.lastUpdateCheck = settings.value("lastUpdateCheck", sync_p.lastUpdateCheck).toDate();
	sync_p.syncAbsoluteTransform = settings.value("syncAbsoluteTransform", sync_p.syncAbsoluteTransform).toBool();
	sync_p.switchModifier = settings.value("switchModifier", sync_p.switchModifier).toBool();
	sync_p.syncActions = settings.value("syncActions", sync_p.syncActions).toBool();

	settings.endGroup();
	// Resource Settings --------------------------------------------------------------------
	settings.beginGroup("ResourceSettings");

	resources_p.cacheMemory = settings.value("cacheMemory", resources_p.cacheMemory).toFloat();
	resources_p.historyMemory = settings.value("historyMemory", resources_p.historyMemory).toFloat();
	resources_p.maxImagesCached = settings.value("maxImagesCached", resources_p.maxImagesCached).toInt();
	resources_p.waitForLastImg = settings.value("waitForLastImg", resources_p.waitForLastImg).toBool();
	resources_p.filterRawImages = settings.value("filterRawImages", resources_p.filterRawImages).toBool();	
	resources_p.loadRawThumb = settings.value("loadRawThumb", resources_p.loadRawThumb).toInt();	
	resources_p.filterDuplicats = settings.value("filterDuplicates", resources_p.filterDuplicats).toBool();
	resources_p.preferredExtension = settings.value("preferredExtension", resources_p.preferredExtension).toString();	
	resources_p.gammaCorrection = settings.value("gammaCorrection", resources_p.gammaCorrection).toBool();
	resources_p.loadSavedImage = settings.value("loadSavedImage", resources_p.loadSavedImage).toInt();

	if (sync_p.switchModifier) {
		global_p.altMod = Qt::ControlModifier;
		global_p.ctrlMod = Qt::AltModifier;
	}
	else {
		global_p.altMod = Qt::AltModifier;
		global_p.ctrlMod = Qt::ControlModifier;
	}

	settings.endGroup();

	if (global_p.numThreads != -1)
		QThreadPool::globalInstance()->setMaxThreadCount(global_p.numThreads);
	else
		global_p.numThreads = QThreadPool::globalInstance()->maxThreadCount();

	// keep loaded settings in mind
	if (defaults) {
		app_d = app_p;
		global_d = global_p;
		display_d = display_p;
		slideShow_d = slideShow_p;
		sync_d = sync_p;
		meta_d = meta_p;
		resources_d = resources_p;
	}
}

void DkSettings::save(bool force) {
	DefaultSettings s;
	save(s, force);
}

void DkSettings::save(QSettings& settings, bool force) {
		
	if (DkSettingsManager::param().app().privateMode)
		return;

	settings.beginGroup("AppSettings");
	if (force ||app_p.showMenuBar != app_d.showMenuBar)
		settings.setValue("showMenuBar", app_p.showMenuBar);

	if (app_p.currentAppMode != mode_frameless && app_p.currentAppMode != mode_frameless_fullscreen) {
		
		if (force ||app_p.showToolBar != app_d.showToolBar)
			settings.setValue("showToolBar", app_p.showToolBar);

		if (force ||app_p.showStatusBar != app_d.showStatusBar)
			settings.setValue("showStatusBar", app_p.showStatusBar);
	}

	if (force ||app_p.showFileInfoLabel != app_d.showFileInfoLabel)
		settings.setValue("showFileInfoLabel", app_p.showFileInfoLabel);
	if (force ||app_p.showFilePreview != app_d.showFilePreview)
		settings.setValue("showFilePreview", app_p.showFilePreview);
	if (force ||app_p.showScroller != app_d.showScroller)
		settings.setValue("showScroller", app_p.showScroller);
	if (force ||app_p.showMetaData != app_d.showMetaData)
		settings.setValue("showMetaData", app_p.showMetaData);
	if (force ||app_p.showPlayer != app_d.showPlayer)
		settings.setValue("showPlayer", app_p.showPlayer);
	if (force ||app_p.showHistogram != app_d.showHistogram)
		settings.setValue("showHistogram", app_p.showHistogram);
	if (force ||app_p.showComment != app_d.showComment)
		settings.setValue("showComment", app_p.showComment);
	if (force ||app_p.showOverview != app_d.showOverview)
		settings.setValue("showOverview", app_p.showOverview);
	if (force ||app_p.showExplorer != app_d.showExplorer)
		settings.setValue("showExplorer", app_p.showExplorer);
	if (force ||app_p.showMetaDataDock != app_d.showMetaDataDock)
		settings.setValue("showMetaDataDock", app_p.showMetaDataDock);
	if (force ||app_p.showEditDock != app_d.showEditDock)
		settings.setValue("showEditDock", app_p.showEditDock);
	if (force ||app_p.showHistoryDock != app_d.showHistoryDock)
		settings.setValue("showHistoryDock", app_p.showHistoryDock);
	if (force || app_p.showLogDock != app_d.showLogDock)
		settings.setValue("showLogDock", app_p.showLogDock);
	if (force || app_p.hideAllPanels != app_d.hideAllPanels)
		settings.setValue("hideAllPanels", app_p.hideAllPanels);
	if (force ||app_p.advancedSettings != app_d.advancedSettings)
		settings.setValue("advancedSettings", app_p.advancedSettings);
	if (force ||app_p.closeOnEsc != app_d.closeOnEsc)
		settings.setValue("closeOnEsc", app_p.closeOnEsc);
	if (force ||app_p.showRecentFiles != app_d.showRecentFiles)
		settings.setValue("showRecentFiles", app_p.showRecentFiles);
	if (force ||app_p.useLogFile != app_d.useLogFile)
		settings.setValue("useLogFile", app_p.useLogFile);
	if (force ||app_p.browseFilters != app_d.browseFilters)
		settings.setValue("browseFilters", app_p.browseFilters);
	if (force ||app_p.registerFilters != app_d.registerFilters)
		settings.setValue("registerFilters", app_p.registerFilters);

	// always save (user setting)
	settings.setValue("defaultJpgQuality", app_p.defaultJpgQuality);
	settings.setValue("appMode", app_p.appMode);
	settings.setValue("currentAppMode", app_p.currentAppMode);

	settings.endGroup();
	// Global Settings --------------------------------------------------------------------
	settings.beginGroup("GlobalSettings");

	if (force ||global_p.skipImgs != global_d.skipImgs)
		settings.setValue("skipImgs",global_p.skipImgs);
	if (force ||global_p.checkOpenDuplicates != global_d.checkOpenDuplicates)
		settings.setValue("checkOpenDuplicates",global_p.checkOpenDuplicates);
	if (force ||global_p.extendedTabs != global_d.extendedTabs)
		settings.setValue("extendedTabs", global_p.extendedTabs);
	if (force ||global_p.loop != global_d.loop)
		settings.setValue("loop",global_p.loop);
	if (force ||global_p.scanSubFolders != global_d.scanSubFolders)
		settings.setValue("scanRecursive",global_p.scanSubFolders);
	if (force ||global_p.lastDir != global_d.lastDir)
		settings.setValue("lastDir", global_p.lastDir);
	if (force ||global_p.searchHistory != global_d.searchHistory)
		settings.setValue("searchHistory", global_p.searchHistory);
	if (force ||global_p.recentFolders != global_d.recentFolders)
		settings.setValue("recentFolders", global_p.recentFolders);
	if (force ||global_p.recentFiles != global_d.recentFiles)
		settings.setValue("recentFiles", global_p.recentFiles);
	if (force || global_p.pinnedFiles != global_d.pinnedFiles)
		settings.setValue("pinnedFiles", global_p.pinnedFiles);
	if (force ||global_p.logRecentFiles != global_d.logRecentFiles)
		settings.setValue("logRecentFiles", global_p.logRecentFiles);
	if (force ||global_p.askToSaveDeletedFiles != global_d.askToSaveDeletedFiles)
		settings.setValue("askToSaveDeletedFiles", global_p.askToSaveDeletedFiles);
	if (force ||global_p.tmpPath != global_d.tmpPath)
		settings.setValue("tmpPath", global_p.tmpPath);
	if (force ||global_p.language != global_d.language)
		settings.setValue("language", global_p.language);
	if (force ||global_p.numThreads != global_d.numThreads)
		settings.setValue("numThreads", global_p.numThreads);
	if (force ||global_p.sortMode != global_d.sortMode)
		settings.setValue("sortMode", global_p.sortMode);
	if (force ||global_p.sortDir != global_d.sortDir)
		settings.setValue("sortDir", global_p.sortDir);
	if (force ||global_p.setupPath != global_d.setupPath)
		settings.setValue("setupPath", global_p.setupPath);
	if (force ||global_p.setupVersion != global_d.setupVersion)
		settings.setValue("setupVersion", global_p.setupVersion);
	if (force ||global_p.zoomOnWheel != global_d.zoomOnWheel)
		settings.setValue("zoomOnWheel", global_p.zoomOnWheel);
	if (force ||global_p.horZoomSkips != global_d.horZoomSkips)
		settings.setValue("horZoomSkips", global_p.horZoomSkips);
	if (force ||global_p.doubleClickForFullscreen != global_d.doubleClickForFullscreen)
		settings.setValue("doubleClickForFullscreen", global_p.doubleClickForFullscreen);
	if (force ||global_p.showBgImage != global_d.showBgImage)
		settings.setValue("showBgImage", global_p.showBgImage);

	settings.endGroup();
	// Display Settings --------------------------------------------------------------------
	settings.beginGroup("DisplaySettings");

	if (force ||display_p.keepZoom != display_d.keepZoom)
		settings.setValue("keepZoom",display_p.keepZoom);
	if (force ||display_p.invertZoom != display_d.invertZoom)
		settings.setValue("invertZoom",display_p.invertZoom);
	if (force ||display_p.zoomToFit != display_d.zoomToFit)
		settings.setValue("zoomToFit",display_p.zoomToFit);
	if (force ||display_p.highlightColor != display_d.highlightColor)
		settings.setValue("highlightColorRGBA", display_p.highlightColor.rgba());
	if (force ||display_p.hudBgColor != display_d.hudBgColor)
		settings.setValue("bgColorWidgetRGBA", display_p.hudBgColor.rgba());
	if (force ||display_p.hudFgdColor != display_d.hudFgdColor)
		settings.setValue("fontColorRGBA", display_p.hudFgdColor.rgba());
	if (force || display_p.bgColor != display_d.bgColor)
		settings.setValue("bgColorNoMacsRGBA", display_p.bgColor.rgba());
	if (force ||display_p.iconColor != display_d.iconColor)
		settings.setValue("iconColorRGBA", display_p.iconColor.rgba());
	if (force ||display_p.bgColorFrameless != display_d.bgColorFrameless)
		settings.setValue("bgColorFramelessRGBA", display_p.bgColorFrameless.rgba());
	if (force ||display_p.thumbSize != display_d.thumbSize)
		settings.setValue("thumbSize", display_p.thumbSize);
	if (force ||display_p.iconSize != display_d.iconSize)
		settings.setValue("iconSize", display_p.iconSize);
	if (force ||display_p.thumbPreviewSize != display_d.thumbPreviewSize)
		settings.setValue("thumbPreviewSize", display_p.thumbPreviewSize);
	//if (force ||display_p.saveThumb != display_d.saveThumb)
	//	settings.setValue("saveThumb", display_p.saveThumb);
	if (force ||display_p.antiAliasing != display_d.antiAliasing)
		settings.setValue("antiAliasing", display_p.antiAliasing);
	if (force || display_p.highQualityAntiAliasing != display_d.highQualityAntiAliasing)
		settings.setValue("highQualityAntiAliasing", display_p.highQualityAntiAliasing);
	if (force ||display_p.showCrop != display_d.showCrop)
		settings.setValue("showCrop", display_p.showCrop);
	if (force ||display_p.histogramStyle != display_d.histogramStyle)
		settings.setValue("histogramStyle", display_p.histogramStyle);
	if (force ||display_p.tpPattern != display_d.tpPattern)
		settings.setValue("tpPattern", display_p.tpPattern);
	if (force || display_p.showNavigation != display_d.showNavigation)
		settings.setValue("showNavigation", display_p.showNavigation);
	if (force || display_p.themeName != display_d.themeName)
		settings.setValue("themeName312", display_p.themeName);
	if (force ||display_p.showBorder != display_d.showBorder)
		settings.setValue("showBorder", display_p.showBorder);
	if (force ||display_p.displaySquaredThumbs != display_d.displaySquaredThumbs)
		settings.setValue("displaySquaredThumbs", display_p.displaySquaredThumbs);
	if (force ||display_p.showThumbLabel != display_d.showThumbLabel)
		settings.setValue("showThumbLabel", display_p.showThumbLabel);
	if (force || display_p.showScrollBars != display_d.showScrollBars)
		settings.setValue("showScrollBars", display_p.showScrollBars);
	if (force ||display_p.alwaysAnimate != display_d.alwaysAnimate)
		settings.setValue("alwaysAnimate", display_p.alwaysAnimate);
	if (force ||display_p.animationDuration != display_d.animationDuration)
		settings.setValue("fadeSec", display_p.animationDuration);
	if (force ||display_p.transition != display_d.transition)
		settings.setValue("transition", display_p.transition);
	if (force ||display_p.defaultBackgroundColor != display_d.defaultBackgroundColor)
		settings.setValue("useDefaultColor", display_p.defaultBackgroundColor);
	if (force ||display_p.defaultIconColor != display_d.defaultIconColor)
		settings.setValue("defaultIconColor", display_p.defaultIconColor);
	if (force ||display_p.interpolateZoomLevel != display_d.interpolateZoomLevel)
		settings.setValue("interpolateZoomlevel", display_p.interpolateZoomLevel);

	settings.endGroup();
	// MetaData Settings --------------------------------------------------------------------
	settings.beginGroup("MetaDataSettings");
	
	if (force ||meta_p.ignoreExifOrientation != meta_d.ignoreExifOrientation)
		settings.setValue("ignoreExifOrientation", meta_p.ignoreExifOrientation);
	if (force ||meta_p.saveExifOrientation != meta_d.saveExifOrientation)
		settings.setValue("saveExifOrientation", meta_p.saveExifOrientation);

	settings.endGroup();
	// SlideShow Settings --------------------------------------------------------------------
	settings.beginGroup("SlideShowSettings");

	if (force ||slideShow_p.filter != slideShow_d.filter)
		settings.setValue("filter", slideShow_p.filter);
	if (force ||slideShow_p.time != slideShow_d.time)
		settings.setValue("time", slideShow_p.time);
	if (force || slideShow_p.showPlayer != slideShow_d.showPlayer)
		settings.setValue("showPlayer", slideShow_p.showPlayer);
	if (force ||slideShow_p.moveSpeed != slideShow_d.moveSpeed)
		settings.setValue("moveSpeed", slideShow_p.moveSpeed);
	if (force ||slideShow_p.display != slideShow_d.display)
		settings.setValue("display", slideShow_p.display);
	if (force ||slideShow_p.backgroundColor != slideShow_d.backgroundColor)
		settings.setValue("backgroundColorRGBA", slideShow_p.backgroundColor.rgba());
	if (force ||slideShow_p.silentFullscreen != slideShow_d.silentFullscreen)
		settings.setValue("silentFullscreen", slideShow_p.silentFullscreen);

	settings.endGroup();
	// Sync Settings --------------------------------------------------------------------
	settings.beginGroup("SynchronizeSettings");

	//if (force ||sync_p.checkForUpdates != sync_d.checkForUpdates)
		settings.setValue("checkForUpdates", sync_p.checkForUpdates);
	//if (force || sync_p.disableUpdateInteraction != sync_d.disableUpdateInteraction)
		settings.setValue("disableUpdateInteraction", sync_p.disableUpdateInteraction);
	if (force ||sync_p.updateDialogShown != sync_d.updateDialogShown)
		settings.setValue("updateDialogShown", sync_p.updateDialogShown);
	if (force ||sync_p.lastUpdateCheck != sync_d.lastUpdateCheck)
		settings.setValue("lastUpdateCheck", sync_p.lastUpdateCheck);
	if (force ||sync_p.syncAbsoluteTransform != sync_d.syncAbsoluteTransform)
		settings.setValue("syncAbsoluteTransform", sync_p.syncAbsoluteTransform);
	if (force ||sync_p.switchModifier != sync_d.switchModifier)
		settings.setValue("switchModifier", sync_p.switchModifier);
	if (force ||sync_p.syncActions != sync_d.syncActions)
		settings.setValue("syncActions", sync_p.syncActions);

	settings.endGroup();
	// Resource Settings --------------------------------------------------------------------
	settings.beginGroup("ResourceSettings");

	if (force ||resources_p.cacheMemory != resources_d.cacheMemory)
		settings.setValue("cacheMemory", resources_p.cacheMemory);
	if (force ||resources_p.historyMemory != resources_d.historyMemory)
		settings.setValue("historyMemory", resources_p.historyMemory);
	if (force ||resources_p.maxImagesCached != resources_d.maxImagesCached)
		settings.setValue("maxImagesCached", resources_p.maxImagesCached);
	if (force ||resources_p.waitForLastImg != resources_d.waitForLastImg)
		settings.setValue("waitForLastImg", resources_p.waitForLastImg);
	if (force ||resources_p.filterRawImages != resources_d.filterRawImages)
		settings.setValue("filterRawImages", resources_p.filterRawImages);
	if (force ||resources_p.loadRawThumb != resources_d.loadRawThumb)
		settings.setValue("loadRawThumb", resources_p.loadRawThumb);
	if (force ||resources_p.filterDuplicats != resources_d.filterDuplicats)
		settings.setValue("filterDuplicates", resources_p.filterDuplicats);
	if (force ||resources_p.preferredExtension != resources_d.preferredExtension)
		settings.setValue("preferredExtension", resources_p.preferredExtension);
	if (force ||resources_p.gammaCorrection != resources_d.gammaCorrection)
		settings.setValue("gammaCorrection", resources_p.gammaCorrection);
	if (force || resources_p.loadSavedImage != resources_d.loadSavedImage)
		settings.setValue("loadSavedImage", resources_p.loadSavedImage);

	settings.endGroup();

	// keep loaded settings in mind
	app_d = app_p;
	global_d = global_p;
	display_d = display_p;
	slideShow_d = slideShow_p;
	sync_d = sync_p;
	meta_d = meta_p;
	resources_d = resources_p;

	qDebug() << "settings saved to" << settingsPath();
}

void DkSettings::setToDefaultSettings() {

	app_p.showToolBar = true;
	app_p.showStatusBar = false;
	app_p.showMovieToolBar = false;
	app_p.showFileInfoLabel = QBitArray(mode_end, false);
	app_p.showFilePreview = QBitArray(mode_end, false);
	app_p.showScroller = QBitArray(mode_end, false);
	app_p.showMetaData = QBitArray(mode_end, false);
	app_p.showPlayer = QBitArray(mode_end, false);
	app_p.showHistogram = QBitArray(mode_end, false);
	app_p.showComment = QBitArray(mode_end, false);
	app_p.showOverview = QBitArray(mode_end, false);
	app_p.showExplorer = QBitArray(mode_end, false);
	app_p.showMetaDataDock = QBitArray(mode_end, false);
	app_p.showEditDock = QBitArray(mode_end, false);
	app_p.showHistoryDock = QBitArray(mode_end, false);
	app_p.showLogDock = QBitArray(mode_end, false);
	app_p.advancedSettings = false;
	app_p.closeOnEsc = false;
	app_p.hideAllPanels = false;
	app_p.showRecentFiles = true;
	app_p.browseFilters = QStringList();
	app_p.showMenuBar = true;
	app_p.useLogFile = false;

	// now set default show options
	app_p.showFileInfoLabel.setBit(mode_default, false);
	app_p.showFileInfoLabel.setBit(mode_contrast, false);

	app_p.appMode = 0;
	app_p.privateMode = false;
	app_p.defaultJpgQuality = 97;

	global_p.skipImgs = 10;
	global_p.numFiles = 50;
	global_p.checkOpenDuplicates = true;
	global_p.extendedTabs = false;
	global_p.loop = true;
	global_p.scanSubFolders = false;
	global_p.lastDir = QString();
	global_p.lastSaveDir = QString();
	global_p.recentFiles = QStringList();
	global_p.pinnedFiles = QStringList();
	global_p.searchHistory = QStringList();
	global_p.recentFolders = QStringList();
	global_p.logRecentFiles = true;
	global_p.askToSaveDeletedFiles = false;
	global_p.tmpPath = "";
	global_p.language = "en";
	global_p.setupPath = "";
	global_p.setupVersion = "";
	global_p.numThreads = -1;
	global_p.sortMode = sort_filename;
	global_p.sortDir = sort_ascending;
	global_p.zoomOnWheel = true;
	global_p.horZoomSkips = true;
	global_p.doubleClickForFullscreen = true;
	global_p.showBgImage = true;

#ifdef Q_OS_LINUX
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
	display_p.zoomToFit = false;
	display_p.highlightColor = QColor(0, 204, 255);
	display_p.hudBgColor = QColor(0, 0, 0, 100);
	display_p.hudFgdColor = QColor(255, 255, 255);
	display_p.bgColor = QColor(100, 100, 100, 255);
	display_p.iconColor = QColor(100,100,100,255);
	display_p.bgColorFrameless = QColor(0, 0, 0, 180);
	display_p.thumbSize = 64;
	display_p.iconSize = 16;
	display_p.thumbPreviewSize = 64;
	display_p.antiAliasing = true;
	display_p.highQualityAntiAliasing = false;
	display_p.showCrop = false;
	display_p.histogramStyle = 0; // DkHistogram::DisplayMode::histogram_mode_simple
	display_p.tpPattern = false;
	display_p.showNavigation = true;
	display_p.themeName = "Light-Theme.css";
	display_p.showBorder = false;
	display_p.displaySquaredThumbs = true;
	display_p.showThumbLabel = false;
	display_p.showScrollBars = false;
	display_p.animationDuration = 0.5f;
	display_p.alwaysAnimate = false;
	display_p.transition = trans_swipe;
	display_p.defaultBackgroundColor = true;
	display_p.defaultIconColor = true;
	display_p.interpolateZoomLevel = 200;

	slideShow_p.filter = 0;
	slideShow_p.time = 3.0;
	slideShow_p.showPlayer = true;
	slideShow_p.moveSpeed = 0;		// TODO: set to 1 for finishing slideshow
	slideShow_p.display = QBitArray(display_end, true);
	slideShow_p.backgroundColor = QColor(51, 51, 51, 255);
	slideShow_p.silentFullscreen = true;

	meta_p.saveExifOrientation = true;
	meta_p.ignoreExifOrientation = false;

	sync_p.checkForUpdates = !isPortable();	// installed version should only check for updates by default
	sync_p.disableUpdateInteraction = isPortable();	// installed version should only check for updates by default
	sync_p.updateDialogShown = false;
	sync_p.lastUpdateCheck = QDate(2018, 7, 14);	// ; )
	sync_p.syncAbsoluteTransform = true;
	sync_p.syncActions = false;

	resources_p.cacheMemory = 0;
	resources_p.historyMemory = 128;
	resources_p.maxImagesCached = 5;
	resources_p.filterRawImages = true;
	resources_p.loadRawThumb = raw_thumb_always;
	resources_p.filterDuplicats = false;
	resources_p.preferredExtension = "*.jpg";
	resources_p.gammaCorrection = true;
	resources_p.loadSavedImage = ls_load_to_tab;
	resources_p.waitForLastImg = true;

	qDebug() << "ok... default settings are set";
}

void DkSettings::applyDefaultsFromFile() {

	DefaultSettings ds;

	if (ds.value("firstTime", true).toBool()) {

		QString dsf = getDefaultSettingsFile();
		if (!QFileInfo(dsf).exists()) {
			qInfo() << "I could not find the default settings file: " << dsf;
		}

		QSettings defaultSettings(dsf, QSettings::IniFormat);
		copySettings(defaultSettings, ds);

		ds.setValue("firstTime", false);

		qInfo() << "defaults loaded from" << dsf;
	}
}

void DkSettings::copySettings(const QSettings & src, QSettings & dst) const {

	// copy default settings
	if (src.allKeys().count() > 0) {
		QStringList keys = src.allKeys();

		for (const QString& key : keys) {
			dst.setValue(key, src.value(key));
		}
	}
}

QString DkSettings::getDefaultSettingsFile() const {

	return QFileInfo(QCoreApplication::applicationDirPath(), "default.ini").absoluteFilePath();
}

void DkSettings::setNumThreads(int numThreads) {

	if (numThreads != global_p.numThreads) {
		global_p.numThreads = numThreads;
		QThreadPool::globalInstance()->setMaxThreadCount(numThreads);
	}

}

bool DkSettings::isPortable() {
	
	QFileInfo fi(QCoreApplication::applicationDirPath(), "settings.ini");
	return fi.absoluteFilePath() == settingsPath();
}

QString DkSettings::settingsPath() const {

	if (!mSettingsPath.isEmpty())
		return mSettingsPath;

	QFileInfo fi(QCoreApplication::applicationDirPath(), "settings.ini");

	if (fi.exists())
		return fi.absoluteFilePath();

	fi = QFileInfo(DkUtils::getAppDataPath(), "settings.ini");

	return fi.absoluteFilePath();
}

DkSettings::App & DkSettings::app() {
	return app_p;
}

DkSettings::Global & DkSettings::global() {
	return global_p;
}

DkSettings::Display & DkSettings::display() {
	return display_p;
}

DkSettings::SlideShow & DkSettings::slideShow() {
	return slideShow_p;
}

DkSettings::Sync & DkSettings::sync() {
	return sync_p;
}

DkSettings::MetaData & DkSettings::metaData() {
	return meta_p;
}

DkSettings::Resources & DkSettings::resources() {
	return resources_p;
}

DkSettingsManager::DkSettingsManager() {
	
	mParams = new DkSettings();
}

DkSettingsManager& DkSettingsManager::instance() { 
	static DkSettingsManager inst;
	return inst; 
}

DkSettingsManager::~DkSettingsManager() {

	if (mParams) {
		delete mParams;
		mParams = 0;
	}
}

DkSettings& DkSettingsManager::param() {
	return instance().settings();
}

DkSettings& DkSettingsManager::settings() {
	return *mParams;
}

void DkSettingsManager::init() {
	
	// init settings
	param().initFileFilters();
	DefaultSettings settings;

	param().load(settings, true);	// load defaults

	int mode = settings.value("AppSettings/appMode", param().app().appMode).toInt();
	param().app().currentAppMode = mode;

	// init debug
	DkUtils::initializeDebug();

	if (nmc::DkSettingsManager::param().app().useLogFile)
		std::cout << "log is saved to: " << nmc::DkUtils::getLogFilePath().toStdString() << std::endl;

	qInfo() << "Hi there";
	qInfoClean() << "my name is " << QApplication::organizationName() << " | " << QApplication::applicationName() 
		<< " v" << QApplication::applicationVersion() << (nmc::DkSettingsManager::param().isPortable() ? " (portable)" : " (installed)");
}

void DkSettingsManager::importSettings(const QString & settingsPath) {

	QSettings settings(settingsPath, QSettings::IniFormat);
	param().load(settings);
	param().save(true);

	qInfo() << "settings imported...";
}

// -------------------------------------------------------------------- DkFileFilterHandling 
void DkFileFilterHandling::registerFileAssociations() {

	DkFileFilterHandling fh;

	// register file associations
	QStringList rFilters = DkSettingsManager::param().app().openFilters;

	// do not register containers such as *.pptx *.zip
	for (const QString& filter : DkSettingsManager::param().app().containerFilters)
		rFilters.removeAll(filter);

	for (const QString& filter : rFilters) {

		// remove the icon file -> otherwise icons might be destroyed (e.g. acrobat)
		if (!filter.contains("ico")) {
			fh.registerFileType(filter, QObject::tr("Image"), true);
			qInfo() << "registering" << filter;
		}
	}

	qInfo() << "files registered...";

}

void DkFileFilterHandling::registerNomacs(bool showDefaultApps) {

#ifdef Q_OS_WIN
	
	// do not register if nomacs is portable
	if (DkSettingsManager::param().isPortable() && !showDefaultApps)
		return;

	// TODO: this is still not working for me on win8??
	QString capName = "Capabilities";
	QString capPath = "Software\\" + QApplication::organizationName() + "\\" + QApplication::applicationName() + "\\" + capName;
	QSettings settings("HKEY_CURRENT_USER\\" + capPath, QSettings::NativeFormat);

	settings.setValue("ApplicationDescription", QObject::tr("nomacs - Image Lounge is a lightweight image viewer."));
	settings.setValue("ApplicationName", QApplication::organizationName() + " " + QApplication::applicationName());
	settings.setValue("ApplicationIcon", QApplication::applicationFilePath() + ",0");

	settings.beginGroup("FileAssociations");
	QStringList rFilters = DkSettingsManager::param().app().openFilters;

	for (int idx = 0; idx < DkSettingsManager::param().app().containerFilters.size(); idx++)
		rFilters.removeAll(DkSettingsManager::param().app().containerFilters.at(idx));

	for (int idx = 0; idx < rFilters.size(); idx++) {

		// remove the icon file -> otherwise icons might be destroyed (e.g. acrobat)
		if (!rFilters.at(idx).contains("ico")) {	
			QStringList extList = getExtensions(rFilters.at(idx));
			
			for (QString cExt : extList)
				settings.setValue(cExt, "nomacs" + cExt + ".3");
		}
	}
	settings.endGroup();
	
	QString softwarePath = "HKEY_CURRENT_USER\\Software\\";
	QSettings wsettings(softwarePath, QSettings::NativeFormat);

	wsettings.beginGroup("RegisteredApplications");
	wsettings.setValue("nomacs.ImageLounge." + QApplication::applicationVersion(), capPath);
	wsettings.endGroup();

	qDebug() << "nomacs registered ============================";

	if (showDefaultApps) {
		showDefaultSoftware();
	}
#endif

}

void DkFileFilterHandling::showDefaultSoftware() const {

#ifdef Q_OS_WIN
	IApplicationActivationManager* manager = 0;
	CoCreateInstance(CLSID_ApplicationActivationManager,
		0,
		CLSCTX_LOCAL_SERVER,
		IID_IApplicationActivationManager,
		(LPVOID*)&manager);

	if (manager) {
		DWORD pid = GetCurrentProcessId();
		manager->ActivateApplication(
			L"windows.immersivecontrolpanel_cw5n1h2txyewy"
			L"!microsoft.windows.immersivecontrolpanel",
			L"page=SettingsPageAppsDefaults", AO_NONE, &pid);
		qDebug() << "launching application registration...";
	}
#endif
}

QString DkFileFilterHandling::registerProgID(const QString& ext, const QString& friendlyName, bool add) {

#ifdef Q_OS_WIN

	QString nomacsPath = "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\";
	QString nomacsKey = "nomacs" + ext + ".3";

	QSettings settings(nomacsPath, QSettings::NativeFormat);
	
	if (add) {
	
		QString iconID = getIconID(ext);

		settings.beginGroup(nomacsKey);
		settings.setValue("Default", friendlyName);
		//settings.setValue("AppUserModelID", "nomacs.ImageLounge");
		//settings.setValue("EditFlags", 1);
		//settings.setValue("CurVer", nomacsKey);
		settings.beginGroup("DefaultIcon");
		settings.setValue("Default",QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "," + iconID);
		settings.endGroup();
		settings.beginGroup("shell");
		settings.beginGroup("open");
		settings.beginGroup("command");
		settings.setValue("Default", "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\" \"%1\"");
		settings.endGroup();
		settings.endGroup();
		settings.endGroup();

		//qDebug() << nomacsKey << " written";
	}
	else
		settings.remove(nomacsKey);

	return nomacsKey;
#else
	return QString();
#endif
}

QString DkFileFilterHandling::getIconID(const QString& ext) const {

	//qDebug() << "ID: " << ext;
	if (ext.contains(".jpg") || ext.contains(".jpeg")) {
		return "1";
	}
	else if (ext.contains(".gif") || ext.contains(".mng")) {
		return "2";
	}
	else if (ext.contains(".png")) {
		return "3";
	}
	else if (ext.contains(".tif") || ext.contains(".tiff") || ext.contains(".bmp") || ext.contains(".pgm") || ext.contains(".webp")) {
		return "4";
	}
	else if (!DkSettingsManager::param().app().rawFilters.filter(ext).empty()) {
		return "5";
	}
	else
		return "0";
}

void DkFileFilterHandling::registerExtension(const QString& ext, const QString& progKey, bool add) {

#ifdef Q_OS_WIN

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

#ifdef Q_OS_WIN

	if (DkSettingsManager::param().app().privateMode)
		return;
	
	QString friendlyName;
	QStringList extList = getExtensions(filterString, friendlyName);
	friendlyName += attribute;

	// register a new progID
	QString progKey = registerProgID(extList.at(0), friendlyName, add);

	// register the extension
	for (int idx = 0; idx < extList.size(); idx++) {

		//qDebug() << "registering: " << extList.at(idx);

		registerExtension(extList.at(idx), progKey, add);
		registerDefaultApp(extList.at(idx), progKey, add);
		setAsDefaultApp(extList.at(idx), progKey, add);		// this is not working on Win8
	}

#endif
}

QStringList DkFileFilterHandling::getExtensions(const QString& filter) const {

	QString dummy;
	return getExtensions(filter, dummy);
}

QStringList DkFileFilterHandling::getExtensions(const QString& filter, QString& friendlyName) const {

	QStringList tmp = filter.split("(");

	if (tmp.size() != 2) {
		qDebug() << "WARNING: wrong filter string!";
		return QStringList();
	}

	friendlyName = tmp.at(0);
	QString filters = tmp.at(1);
	filters.replace(")", "");
	filters.replace("*", "");

	QStringList extList = filters.split(" ");

	if (extList.empty()) {
		qDebug() << "nothing to do here, not registering: " << filter;
		return QStringList();
	}

	return extList;
}

void DkFileFilterHandling::registerDefaultApp(const QString& ext, const QString&, bool add) {

#ifdef Q_OS_WIN

	QSettings settings("HKEY_CURRENT_USER\\Software\\Classes\\Applications\\nomacs.exe", QSettings::NativeFormat);
	
	if (add) {
		settings.beginGroup("SupportedTypes");
		settings.setValue(ext, "");
		//qDebug() << ext << "registered...";
	}
	else
		settings.remove(ext);
#endif

}

void DkFileFilterHandling::setAsDefaultApp(const QString& ext, const QString& progKey, bool add) {

#ifdef Q_OS_WIN
	
	QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts", QSettings::NativeFormat);
	settings.beginGroup(ext);

	if (add) {
		// windows 7 only
		settings.beginGroup("UserChoice");
		settings.setValue("ProgId", progKey);
		settings.endGroup();

		settings.beginGroup("OpenWithProgids");
		settings.setValue("nomacs" + ext + ".3","");
		//qDebug() << "default app set";
	}
	else
		settings.setValue("Default", "");
#endif

}

// -------------------------------------------------------------------- DefaultSettings
#ifdef Q_OS_WIN
// read the settings from a file on Windows
DefaultSettings::DefaultSettings() : QSettings(DkSettingsManager::instance().param().settingsPath(), QSettings::IniFormat) {}
#else
DefaultSettings::DefaultSettings() : QSettings() {}
#endif

// -------------------------------------------------------------------- DkThemeManager 
QStringList DkThemeManager::getAvailableThemes() const {
	
	QDir td(themeDir());
	td.setNameFilters(QStringList() << "*.css");

	QStringList themes = td.entryList(QDir::Files, QDir::Name);

	return themes;
}

QString DkThemeManager::getCurrentThemeName() const {
	return DkSettingsManager::param().display().themeName;
}

QString DkThemeManager::themeDir() const {

	QStringList paths;
	paths << QCoreApplication::applicationDirPath();
	paths << QStandardPaths::standardLocations(QStandardPaths::DataLocation);

	QDir themeDir;

	for (const QString& p : paths) {
		themeDir = QDir(p + QDir::separator() + "themes");

		if (themeDir.exists())
			break;
	}

	return themeDir.absolutePath();
}

void DkThemeManager::setCurrentTheme(const QString & themeName) const {
	DkSettingsManager::param().display().themeName = themeName;
}

QString DkThemeManager::loadTheme(const QString & themeName) const {

	QString cssString;
	QFileInfo tfi(themeDir(), themeName);
	QFile theme(tfi.absoluteFilePath());

	if (theme.open(QFile::ReadOnly)) {

		QString ts = theme.readAll();
		cssString = parseColors(ts);
		cssString = cssString.trimmed();

		qInfo() << "theme loaded from" << tfi.absoluteFilePath();
	}
	else
		qInfo() << "could not load theme from" << tfi.absoluteFilePath();

	return cssString;
}

QString DkThemeManager::loadStylesheet() const {

	QString cssString;

	QFileInfo cssInfo(":/nomacs/stylesheet.css");
	QFile file(cssInfo.absoluteFilePath());

	if (file.open(QFile::ReadOnly)) {

		cssString = file.readAll();
		cssString = replaceColors(cssString);

		qInfo() << "CSS loaded from: " << cssInfo.absoluteFilePath();
	}
	
	file.close();

	return cssString;
}

void DkThemeManager::applyTheme() const {
	
	// add theme
	QString cssString = loadTheme(getCurrentThemeName());

	DkSettings::Display& dp = DkSettingsManager::param().display();

	// NOTE: it is important, that default.css does not contain
	// any line of code except for the color definitions
	// otherwise, we change the default palette here...
	if (!cssString.isEmpty()) {

		cssString = replaceColors(cssString);

		QPalette p = qApp->palette();

		if (dp.themeBgdColor != QPalette().color(QPalette::Window)) {
			p.setColor(QPalette::Window, dp.themeBgdColor);
			p.setColor(QPalette::Base, dp.themeBgdColor);
		}

		p.setColor(QPalette::WindowText, dp.themeFgdColor);
		p.setColor(QPalette::ButtonText, dp.themeFgdColor);

		//p.setColor(QPalette::Button, QColor(0, 0, 0));

		qApp->setPalette(p);
	}

	QString cs = loadStylesheet();
	cs += cssString;

	qApp->setStyleSheet(cs);
}

QString DkThemeManager::cleanThemeName(const QString & theme) const {
	
	QString t = theme;
	t = t.replace(".css", "");
	t = t.replace("-", " ");
	
	return t;
}

QStringList DkThemeManager::cleanThemeNames(const QStringList & themes) const {

	QStringList ctn;

	for (const QString& t : themes)
		ctn << cleanThemeName(t);

	return ctn;
}

QString DkThemeManager::parseColors(const QString & styleSheet) const {

	QStringList cs = styleSheet.split("--nomacs-color-def");

	if (cs.size() == 3) {

		// we expect something like this:
		/* overload color settings of nomacs */
		//	--nomacs-color-def
		//		HIGHLIGHT_COLOR:	#ff0;
		//		WINDOW_COLOR:		#333;
		//	--nomacs-color-def
		QStringList cols = cs[1].split(";");

		for (auto str : cols) {

			str = str.simplified();

			// igonre blanks
			if (str.isEmpty())
				continue;

			QStringList kv = str.split(":");

			if (kv.size() != 2) {
				qWarning() << "could not parse color from" << str;
				qWarning() << "I expected a line like this: HUD_BACKGROUND_COLOR: #f00;";
				continue;
			}

			QString cc = kv[1].simplified();

			if (kv[0] == "HIGHLIGHT_COLOR" && cc != "default")
				DkSettingsManager::param().display().highlightColor.setNamedColor(cc);
			else if (kv[0] == "HUD_BACKGROUND_COLOR" && cc != "default")
				DkSettingsManager::param().display().hudBgColor.setNamedColor(cc);
			else if (kv[0] == "HUD_FOREGROUND_COLOR" && cc != "default")
				DkSettingsManager::param().display().hudFgdColor.setNamedColor(cc);
			else if (kv[0] == "BACKGROUND_COLOR") {

				QColor c;
				c.setNamedColor(cc);

				if (cc == "default")
					c = QPalette().color(QPalette::Window);
					
				if (DkSettingsManager::param().display().defaultBackgroundColor)
						DkSettingsManager::param().display().bgColor = c;
				DkSettingsManager::param().display().themeBgdColor = c;
			}
			else if (kv[0] == "FOREGROUND_COLOR" && cc != "default")
				DkSettingsManager::param().display().themeFgdColor.setNamedColor(cc);
			else if (kv[0] == "ICON_COLOR" && cc != "default") {
				if (DkSettingsManager::param().display().defaultIconColor)
					DkSettingsManager::param().display().iconColor.setNamedColor(cc);
			}
			else if (cc != "default")
				qWarning() << "could not parse color:" << str;
		}

		return cs[0] + cs[2];
	}

	return styleSheet;
}

QString DkThemeManager::replaceColors(const QString & cssString) const {
	
	QString cs = cssString;

	QColor hc = DkSettingsManager::param().display().highlightColor;
	hc.setAlpha(150);

	// replace color placeholders
	cs.replace("HIGHLIGHT_COLOR", DkUtils::colorToString(DkSettingsManager::param().display().highlightColor));
	cs.replace("HIGHLIGHT_LIGHT", DkUtils::colorToString(hc));
	cs.replace("HUD_BACKGROUND_COLOR", DkUtils::colorToString(DkSettingsManager::param().display().hudBgColor));
	cs.replace("HUD_FOREGROUND_COLOR", DkUtils::colorToString(DkSettingsManager::param().display().hudFgdColor));
	cs.replace("BACKGROUND_COLOR", DkUtils::colorToString(DkSettingsManager::param().display().bgColor));
	cs.replace("FOREGROUND_COLOR", DkUtils::colorToString(DkSettingsManager::param().display().themeFgdColor));
	cs.replace("WINDOW_COLOR", DkUtils::colorToString(QPalette().color(QPalette::Window)));
	
	return cs;
}

// -------------------------------------------------------------------- DkZoomConfig 
DkZoomConfig::DkZoomConfig() {

	mLevels = defaultLevels();

	DefaultSettings ds;
	loadSettings(ds);
}

DkZoomConfig& DkZoomConfig::instance() {
	static DkZoomConfig inst;
	return inst;
}

DkZoomConfig::~DkZoomConfig() {

	DefaultSettings ds;
	saveSettings(ds);
}

double DkZoomConfig::nextFactor(double currentFactor, double delta) const {

	// do nothing?
	if (!mUseLevels)
		return delta;

	assert(currentFactor != 0.0);

	if (currentFactor == 0.0)
		return 1.0;

	if (delta > 1) {
		for (double l : mLevels) {

			if (currentFactor < l) {
				return l / currentFactor;
			}
		}
	}
	else if (delta < 1) {

		for (int idx = mLevels.size() - 1; idx >= 0; idx--) {

			if (currentFactor > mLevels[idx]) {
				return mLevels[idx] / currentFactor;
			}
		}
	}

	// do nothing
	return 1.0;
}

QVector<double> DkZoomConfig::defaultLevels() const {

	QVector<double> levels;

	levels << 0.0001;
	levels << 0.001;
	levels << 0.01;
	levels << 0.05;
	levels << 0.1;
	levels << 0.125;
	levels << 0.166;
	levels << 0.25;
	levels << 0.333;
	levels << 0.5;
	levels << 0.66;
	levels << 1;
	levels << 1.5;
	levels << 2;
	levels << 3;
	levels << 4;
	levels << 5;
	levels << 6;
	levels << 7;
	levels << 8;
	levels << 12;
	levels << 16;
	levels << 32;
	levels << 64;
	levels << 128;

	return levels;
}

bool DkZoomConfig::useLevels() const {
	return mUseLevels;
}

void DkZoomConfig::setUseLevels(bool useLevels) {
	mUseLevels = useLevels;
}

bool DkZoomConfig::setLevels(const QString & levelStr) {

	QVector<double> levels;
	QStringList levelList = levelStr.split(",");

	bool ok = false;
	for (const QString& s : levelList) {
		levels << s.toDouble(&ok);
		if (!ok)
			break;
	}

	if (ok && checkLevels(levels)) {
		mLevels = levels;
		return true;
	}
	
	return false;
}

QString DkZoomConfig::levelsToString() const {
	
	QStringList levelStr;
	for (double l : mLevels)
		levelStr << QString::number(l);
	
	return levelStr.join(",");
}

void DkZoomConfig::setLevelsToDefault() {

	mLevels = defaultLevels();
}

bool DkZoomConfig::checkLevels(const QVector<double>& levels) {

	if (levels.isEmpty())
		return false;

	double last = 0;

	// levels must be monotonically increasing
	for (double l : levels) {
		if (l < last)
			return false;
	}

	return true;
}

void DkZoomConfig::loadSettings(QSettings & settings) {

	settings.beginGroup("zooming");
	QString levelStr = settings.value("zoomLevels").toString();
	mUseLevels = settings.value("useLevels").toBool();
	settings.endGroup();

	if (!setLevels(levelStr))
		qWarning() << "illegal zoom levels when loading from settings:" << levelStr;
}

void DkZoomConfig::saveSettings(QSettings & settings) const {

	settings.beginGroup("zooming");
	settings.setValue("zoomLevels", levelsToString());
	settings.setValue("useLevels", mUseLevels);
	settings.endGroup();
}

}
