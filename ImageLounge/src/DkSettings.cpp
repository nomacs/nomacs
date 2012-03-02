/*******************************************************************************************************
 DkSettings.cpp
 Created on:	07.07.2011
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2012 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2012 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2012 Florian Kleber <florian@nomacs.org>

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
#include "DkWidgets.h"

namespace nmc {

bool DkSettings::AppSettings::showToolBar = true;
bool DkSettings::AppSettings::showMenuBar = true;
bool DkSettings::AppSettings::showStatusBar = false;
int DkSettings::AppSettings::appMode = 0;

int DkSettings::GlobalSettings::skipImgs = 10;
bool DkSettings::GlobalSettings::showOverview = true;
bool DkSettings::GlobalSettings::showInfo = true;
bool DkSettings::GlobalSettings::loop = false;
QString DkSettings::GlobalSettings::lastDir = QString();
QString DkSettings::GlobalSettings::lastSaveDir = QString();
QStringList DkSettings::GlobalSettings::recentFiles = QStringList();
QStringList DkSettings::GlobalSettings::recentFolders = QStringList();
bool DkSettings::GlobalSettings::useTmpPath = false;
QString DkSettings::GlobalSettings::tmpPath = QString();
QString DkSettings::GlobalSettings::language = "en";

#ifdef linux
	bool DkSettings::SynchronizeSettings::switchModifier = true;
	Qt::KeyboardModifier DkSettings::GlobalSettings::altMod = Qt::ControlModifier;
	Qt::KeyboardModifier DkSettings::GlobalSettings::ctrlMod = Qt::AltModifier;
#else
	bool DkSettings::SynchronizeSettings::switchModifier = false;
	Qt::KeyboardModifier DkSettings::GlobalSettings::altMod = Qt::AltModifier;
	Qt::KeyboardModifier DkSettings::GlobalSettings::ctrlMod = Qt::ControlModifier;
#endif

// open with settings
QString DkSettings::GlobalSettings::defaultAppPath = QString();
int DkSettings::GlobalSettings::defaultAppIdx = -1;
bool DkSettings::GlobalSettings::showDefaultAppDialog = true;
int DkSettings::GlobalSettings::numUserChoices = 3;
QStringList DkSettings::GlobalSettings::userAppPaths = QStringList();

bool DkSettings::DisplaySettings::keepZoom = true;
bool DkSettings::DisplaySettings::invertZoom = false;
QColor DkSettings::DisplaySettings::highlightColor = QColor(0, 204, 255);
int DkSettings::DisplaySettings::thumbSize = 100; // max seems to be 160 (?!)
bool DkSettings::DisplaySettings::saveThumb = false;
int DkSettings::DisplaySettings::interpolateZoomLevel = 200;


int DkSettings::SlideShowSettings::filter = 0;
int DkSettings::SlideShowSettings::time = 3;
QBitArray DkSettings::SlideShowSettings::display = QBitArray(DkSlideshowSettingsWidget::display_end, true);
QColor DkSettings::SlideShowSettings::backgroundColor = QColor(200, 200, 200);
bool DkSettings::SlideShowSettings::silentFullscreen = true;

QBitArray DkSettings::MetaDataSettings::metaDataBits = QBitArray(DkMetaDataSettingsWidget::desc_end, false);


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

bool DkSettings::SynchronizeSettings::enableNetworkSync = false;
bool DkSettings::SynchronizeSettings::allowTransformation = true;
bool DkSettings::SynchronizeSettings::allowPosition = true;
bool DkSettings::SynchronizeSettings::allowFile = true;
bool DkSettings::SynchronizeSettings::allowImage = true;
bool DkSettings::SynchronizeSettings::updateDialogShown= false;
QDate DkSettings::SynchronizeSettings::lastUpdateCheck = QDate(1970, 1, 1);	// not my birthday
bool DkSettings::SynchronizeSettings::syncAbsoluteTransform = true;


void DkSettings::load() {
	
	
	setToDefaultSettings();

	QSettings settings;

	AppSettings::showMenuBar = settings.value("AppSettings/showMenuBar", DkSettings::AppSettings::showMenuBar).toBool();
	AppSettings::showToolBar = settings.value("AppSettings/showToolBar", DkSettings::AppSettings::showToolBar).toBool();
	AppSettings::showStatusBar = settings.value("AppSettings/showStatusBar", DkSettings::AppSettings::showStatusBar).toBool();
	
	GlobalSettings::skipImgs = settings.value("GlobalSettings/skipImgs", DkSettings::GlobalSettings::skipImgs).toInt();
	GlobalSettings::showOverview = settings.value("GlobalSettings/hideOverview", DkSettings::GlobalSettings::showOverview = true).toBool();
	GlobalSettings::showInfo = settings.value("GlobalSettings/showInfo", DkSettings::GlobalSettings::showInfo = true).toBool();

	GlobalSettings::loop = settings.value("GlobalSettings/loop", DkSettings::GlobalSettings::loop).toBool();
	GlobalSettings::lastDir = settings.value("GlobalSettings/lastDir", DkSettings::GlobalSettings::lastDir).toString();
	//GlobalSettings::lastSaveDir = settings.value("GlobalSettings/lastSaveDir", DkSettings::GlobalSettings::lastSaveDir).toString();
	GlobalSettings::recentFolders = settings.value("GlobalSettings/recentFolders", DkSettings::GlobalSettings::recentFolders).toStringList();
	GlobalSettings::recentFiles = settings.value("GlobalSettings/recentFiles", DkSettings::GlobalSettings::recentFiles).toStringList();
	GlobalSettings::useTmpPath= settings.value("GlobalSettings/useTmpPath", DkSettings::GlobalSettings::useTmpPath).toBool();
	GlobalSettings::tmpPath = settings.value("GlobalSettings/tmpPath", DkSettings::GlobalSettings::tmpPath).toString();
	GlobalSettings::language = settings.value("GlobalSettings/language", DkSettings::GlobalSettings::language).toString();

	GlobalSettings::defaultAppPath = settings.value("GlobalSettings/defaultAppPath", DkSettings::GlobalSettings::defaultAppPath).toString();
	GlobalSettings::defaultAppIdx = settings.value("GlobalSettings/defaultAppIdx", DkSettings::GlobalSettings::defaultAppIdx).toInt();
	GlobalSettings::showDefaultAppDialog = settings.value("GlobalSettings/showDefaultAppDialog", DkSettings::GlobalSettings::showDefaultAppDialog).toBool();
	GlobalSettings::numUserChoices = settings.value("GlobalSettings/numUserChoices", DkSettings::GlobalSettings::numUserChoices).toInt();
	GlobalSettings::userAppPaths = settings.value("GlobalSettings/userAppPaths", DkSettings::GlobalSettings::userAppPaths).toStringList();

	DisplaySettings::keepZoom = settings.value("DisplaySettings/resetMatrix", DkSettings::DisplaySettings::keepZoom).toBool();
	DisplaySettings::invertZoom = settings.value("DisplaySettings/invertZoom", DkSettings::DisplaySettings::invertZoom).toBool();
	DisplaySettings::highlightColor = settings.value("DisplaySettings/highlightColor", DkSettings::DisplaySettings::highlightColor).value<QColor>();
	DisplaySettings::thumbSize = settings.value("DisplaySettings/thumbSize", DkSettings::DisplaySettings::thumbSize).toInt();
	DisplaySettings::saveThumb = settings.value("DisplaySettings/saveThumb", DkSettings::DisplaySettings::saveThumb).toBool();
	DisplaySettings::interpolateZoomLevel = settings.value("DisplaySettings/interpolateZoomlevel", DkSettings::DisplaySettings::interpolateZoomLevel).toInt();

	QBitArray tmpMetaData = settings.value("MetaDataSettings/metaData", DkSettings::MetaDataSettings::metaDataBits).toBitArray();
	if (tmpMetaData.size() == MetaDataSettings::metaDataBits.size())
		MetaDataSettings::metaDataBits = tmpMetaData;

	SlideShowSettings::filter = settings.value("SlideShowSettings/filter", DkSettings::SlideShowSettings::filter).toInt();
	SlideShowSettings::time = settings.value("SlideShowSettings/time", DkSettings::SlideShowSettings::time).toInt();
	SlideShowSettings::backgroundColor = settings.value("SlideShowSettings/backgroundColor", DkSettings::SlideShowSettings::backgroundColor).value<QColor>();
	SlideShowSettings::silentFullscreen = settings.value("SlideShowSettings/silentFullscreen", DkSettings::SlideShowSettings::silentFullscreen).toBool();
	QBitArray tmpDisplay = settings.value("SlideShowSettings/display", DkSettings::SlideShowSettings::display).toBitArray();

	if (tmpDisplay.size() == SlideShowSettings::display.size())
		SlideShowSettings::display = tmpDisplay;

	SynchronizeSettings::enableNetworkSync= settings.value("SynchronizeSettings/enableNetworkSync", DkSettings::SynchronizeSettings::enableNetworkSync).toBool();
	SynchronizeSettings::allowTransformation = settings.value("SynchronizeSettings/allowTransformation", DkSettings::SynchronizeSettings::allowTransformation).toBool();
	SynchronizeSettings::allowPosition = settings.value("SynchronizeSettings/allowPosition", DkSettings::SynchronizeSettings::allowPosition).toBool();
	SynchronizeSettings::allowFile = settings.value("SynchronizeSettings/allowFile", DkSettings::SynchronizeSettings::allowFile).toBool();
	SynchronizeSettings::allowImage = settings.value("SynchronizeSettings/allowImage", DkSettings::SynchronizeSettings::allowImage).toBool();;
	SynchronizeSettings::updateDialogShown = settings.value("SynchronizeSettings/updateDialogShown", DkSettings::SynchronizeSettings::updateDialogShown).toBool();
	SynchronizeSettings::lastUpdateCheck = settings.value("SynchronizeSettings/lastUpdateCheck", DkSettings::SynchronizeSettings::lastUpdateCheck).toDate();
	SynchronizeSettings::syncAbsoluteTransform = settings.value("SynchronizeSettings/syncAbsoluteTransform", DkSettings::SynchronizeSettings::syncAbsoluteTransform).toBool();
	SynchronizeSettings::switchModifier = settings.value("SynchronizeSettings/switchModifier", DkSettings::SynchronizeSettings::switchModifier).toBool();
	
	if (DkSettings::SynchronizeSettings::switchModifier) {
		DkSettings::GlobalSettings::altMod = Qt::ControlModifier;
		DkSettings::GlobalSettings::ctrlMod = Qt::AltModifier;
	}
	else {
		DkSettings::GlobalSettings::altMod = Qt::AltModifier;
		DkSettings::GlobalSettings::ctrlMod = Qt::ControlModifier;
	}

}

void DkSettings::save() {
	QSettings settings;
	settings.setValue("AppSettings/showMenuBar", DkSettings::AppSettings::showMenuBar);
	
	if (DkSettings::AppSettings::appMode != mode_frameless) {
		qDebug() << "app mode when saving: " << DkSettings::AppSettings::appMode;
		settings.setValue("AppSettings/showToolBar", DkSettings::AppSettings::showToolBar);
		settings.setValue("AppSettings/showStatusBar", DkSettings::AppSettings::showStatusBar);
	}
	settings.setValue("AppSettings/appMode", DkSettings::AppSettings::appMode);

	settings.setValue("GlobalSettings/skipImgs",GlobalSettings::skipImgs);
	settings.setValue("GlobalSettings/hideOverview",GlobalSettings::showOverview);
	settings.setValue("GlobalSettings/showInfo",GlobalSettings::showInfo);
	settings.setValue("GlobalSettings/loop",GlobalSettings::loop);
	settings.setValue("GlobalSettings/lastDir", DkSettings::GlobalSettings::lastDir);
	//settings.setValue("GlobalSettings/lastSaveDir", DkSettings::GlobalSettings::lastSaveDir);
	settings.setValue("GlobalSettings/recentFolders", DkSettings::GlobalSettings::recentFolders);
	settings.setValue("GlobalSettings/recentFiles", DkSettings::GlobalSettings::recentFiles);
	settings.setValue("GlobalSettings/useTmpPath", DkSettings::GlobalSettings::useTmpPath);
	settings.setValue("GlobalSettings/tmpPath", DkSettings::GlobalSettings::tmpPath);
	settings.setValue("GlobalSettings/language", DkSettings::GlobalSettings::language);

	settings.setValue("GlobalSettings/defaultAppIdx", DkSettings::GlobalSettings::defaultAppIdx);
	settings.setValue("GlobalSettings/defaultAppPath", DkSettings::GlobalSettings::defaultAppPath);
	settings.setValue("GlobalSettings/showDefaultAppDialog", DkSettings::GlobalSettings::showDefaultAppDialog);
	settings.setValue("GlobalSettings/numUserChoices", DkSettings::GlobalSettings::numUserChoices);
	settings.setValue("GlobalSettings/userAppPaths", DkSettings::GlobalSettings::userAppPaths);

	settings.setValue("DisplaySettings/resetMatrix",DisplaySettings::keepZoom);
	settings.setValue("DisplaySettings/invertZoom",DisplaySettings::invertZoom);
	settings.setValue("DisplaySettings/highlightColor", DisplaySettings::highlightColor);
	settings.setValue("DisplaySettings/thumbSize", DkSettings::DisplaySettings::thumbSize);
	settings.setValue("DisplaySettings/saveThumb", DkSettings::DisplaySettings::saveThumb);
	settings.setValue("DisplaySettings/interpolateZoomlevel", DkSettings::DisplaySettings::interpolateZoomLevel);


	settings.setValue("MetaDataSettings/metaData", MetaDataSettings::metaDataBits);

	settings.setValue("SlideShowSettings/filter", SlideShowSettings::filter);
	settings.setValue("SlideShowSettings/time", SlideShowSettings::time);
	settings.setValue("SlideShowSettings/display", SlideShowSettings::display);
	settings.setValue("SlideShowSettings/backgroundColor", SlideShowSettings::backgroundColor);
	settings.setValue("SlideShowSettings/silentFullscreen", SlideShowSettings::silentFullscreen);

	settings.setValue("SynchronizeSettings/enableNetworkSync", DkSettings::SynchronizeSettings::enableNetworkSync);
	settings.setValue("SynchronizeSettings/allowTransformation", DkSettings::SynchronizeSettings::allowTransformation);
	settings.setValue("SynchronizeSettings/allowPosition", DkSettings::SynchronizeSettings::allowPosition);
	settings.setValue("SynchronizeSettings/allowFile", DkSettings::SynchronizeSettings::allowFile);
	settings.setValue("SynchronizeSettings/allowImage", DkSettings::SynchronizeSettings::allowImage);
	settings.setValue("SynchronizeSettings/updateDialogShown", DkSettings::SynchronizeSettings::updateDialogShown);
	settings.setValue("SynchronizeSettings/lastUpdateCheck", DkSettings::SynchronizeSettings::lastUpdateCheck);
	settings.setValue("SynchronizeSettings/syncAbsoluteTransform", DkSettings::SynchronizeSettings::syncAbsoluteTransform);
	settings.setValue("SynchronizeSettings/switchModifier", DkSettings::SynchronizeSettings::switchModifier);
	
	qDebug() << "settings saved";
}

void DkSettings::setToDefaultSettings() {

	DkSettings::AppSettings::showMenuBar = true;
	DkSettings::AppSettings::showToolBar = true;
	DkSettings::AppSettings::showStatusBar = false;
	DkSettings::AppSettings::appMode = 0;
	
	DkSettings::GlobalSettings::skipImgs = 10;
	DkSettings::GlobalSettings::showOverview = true;
	DkSettings::GlobalSettings::showInfo = true;
	DkSettings::GlobalSettings::loop = false;
	DkSettings::GlobalSettings::lastDir = QString();
	DkSettings::GlobalSettings::lastSaveDir = QString();
	DkSettings::GlobalSettings::recentFiles = QStringList();
	DkSettings::GlobalSettings::recentFolders = QStringList();
	DkSettings::GlobalSettings::useTmpPath = false;
	DkSettings::GlobalSettings::tmpPath = QString();
	DkSettings::GlobalSettings::language = QString();
	DkSettings::GlobalSettings::defaultAppIdx = -1;
	DkSettings::GlobalSettings::defaultAppPath = QString();
	DkSettings::GlobalSettings::showDefaultAppDialog = true;
	DkSettings::GlobalSettings::numUserChoices = 3;
	DkSettings::GlobalSettings::userAppPaths = QStringList();


#ifdef linux
	DkSettings::SynchronizeSettings::switchModifier = true;
	DkSettings::GlobalSettings::altMod = Qt::ControlModifier;
	DkSettings::GlobalSettings::ctrlMod = Qt::AltModifier;
#else
	DkSettings::SynchronizeSettings::switchModifier = false;
	DkSettings::GlobalSettings::altMod = Qt::AltModifier;
	DkSettings::GlobalSettings::ctrlMod = Qt::ControlModifier;
#endif


	DkSettings::DisplaySettings::keepZoom = true;
	DkSettings::DisplaySettings::invertZoom = false;
	DkSettings::DisplaySettings::highlightColor = QColor(0, 204, 255);
	DkSettings::DisplaySettings::thumbSize = 100;
	DkSettings::DisplaySettings::saveThumb = false;
	DkSettings::DisplaySettings::interpolateZoomLevel = 200;

	DkSettings::SlideShowSettings::filter = 0;
	DkSettings::SlideShowSettings::time = 3;
	DkSettings::SlideShowSettings::display = QBitArray(DkSlideshowSettingsWidget::display_end, true);
	DkSettings::SlideShowSettings::backgroundColor = QColor(217, 219, 228, 100);
	DkSettings::SlideShowSettings::silentFullscreen = true;


	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_size] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_orientation] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_make] = true;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_model] = true;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_aperture] = true;
	//DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_shutterspeed] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_flash] = true;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_focallength] = true;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_exposuremode] = true;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::camData_exposuretime] = true;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_rating] = true;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_usercomment] = true;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_date] = true;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_datetimeoriginal] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_imagedescription] = true;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_creator] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_creatortitle] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_city] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_country] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_headline] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_caption] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_copyright] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_keywords] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_path] = false;
	DkSettings::MetaDataSettings::metaDataBits[DkMetaDataSettingsWidget::desc_filesize] = false;


	DkSettings::SynchronizeSettings::enableNetworkSync = false;
	DkSettings::SynchronizeSettings::allowTransformation = true;
	DkSettings::SynchronizeSettings::allowPosition = true;
	DkSettings::SynchronizeSettings::allowFile = true;
	DkSettings::SynchronizeSettings::allowImage = true;
	DkSettings::SynchronizeSettings::updateDialogShown = false;
	DkSettings::SynchronizeSettings::lastUpdateCheck = QDate(1970 , 1, 1);
	DkSettings::SynchronizeSettings::syncAbsoluteTransform = true;

	qDebug() << "ok... default settings are set";

	emit setToDefaultSettingsSignal();
}

// DkSettingsDialog --------------------------------------------------------------------
DkSettingsDialog::DkSettingsDialog(QWidget* parent) : QDialog(parent) {

	this->resize(600,400);

	s = new DkSettings();

	createLayout();
	init();

	connect(listView, SIGNAL(activated(const QModelIndex &)), this, SLOT(listViewSelected(const QModelIndex &)));
	connect(listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(listViewSelected(const QModelIndex &)));
	connect(listView, SIGNAL(entered(const QModelIndex &)), this, SLOT(listViewSelected(const QModelIndex &)));

	connect(buttonOk, SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelPressed()));
	connect(s, SIGNAL(setToDefaultSettingsSignal()), this, SLOT(initWidgets()));
	connect(globalSettingsWidget, SIGNAL(applyDefault()), this, SLOT(setToDefault()));
	
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
		centralLayout->addWidget(curWidget);
	}
	widgetList[0]->show(); // display first;
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
	stringList << tr("General") << tr("Display") << tr("Slideshow") << tr("Synchronize") << tr("Exif");
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

	globalSettingsWidget = new DkGlobalSettingsWidget(this);
	displaySettingsWidget = new DkDisplaySettingsWidget(this);
	slideshowSettingsWidget = new DkSlideshowSettingsWidget(this);
	synchronizeSettingsWidget = new DkSynchronizeSettingsWidget(this);
	exifSettingsWidget = new DkMetaDataSettingsWidget(this);

	widgetList.push_back(globalSettingsWidget);
	widgetList.push_back(displaySettingsWidget);
	widgetList.push_back(slideshowSettingsWidget);
	widgetList.push_back(synchronizeSettingsWidget);
	widgetList.push_back(exifSettingsWidget);

}

void DkSettingsDialog::listViewSelected(const QModelIndex & qmodel) {
	foreach (DkSettingsWidget* curWidget, widgetList) {
		curWidget->hide();
	}
	widgetList[qmodel.row()]->show(); // display selected;
}

void DkSettingsDialog::saveSettings() {
	
	QString curLanguage = DkSettings::GlobalSettings::language;
	
	foreach (DkSettingsWidget* curWidget, widgetList) {
		curWidget->writeSettings();
	}

	DkSettings* settings = new DkSettings();
	settings->save();
	this->close();
	
	// if the language changed we need to restart nomacs (re-translating while running is pretty hard to accomplish)
	if (curLanguage != DkSettings::GlobalSettings::language)
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

// DkGlobalSettingsWidget --------------------------------------------------------------------

DkGlobalSettingsWidget::DkGlobalSettingsWidget(QWidget* parent) : DkSettingsWidget(parent) {
	createLayout();
	init();

}

void DkGlobalSettingsWidget::init() {

	cbWrapImages->setChecked(DkSettings::GlobalSettings::loop);
	skipImgWidget->setSpinBoxValue(DkSettings::GlobalSettings::skipImgs);
	cbUseTmpPath->setChecked(DkSettings::GlobalSettings::useTmpPath);
	tmpPath = DkSettings::GlobalSettings::tmpPath;
	leTmpPath->setText(tmpPath);
	if (!DkSettings::GlobalSettings::useTmpPath) {
		leTmpPath->setDisabled(true);
		pbTmpPath->setDisabled(true);
	}
	curLanguage = DkSettings::GlobalSettings::language;
	langCombo->setCurrentIndex(languages.indexOf(curLanguage));
	if (langCombo->currentIndex() == -1) // set index to English if language has not been found
		langCombo->setCurrentIndex(0);

	connect(buttonDefaultSettings, SIGNAL(clicked()), this, SLOT(setToDefaultPressed()));
	connect(pbTmpPath, SIGNAL(clicked()), this, SLOT(tmpPathButtonPressed()));
	connect(cbUseTmpPath, SIGNAL(stateChanged(int)), this, SLOT(useTmpPathChanged(int)));
	connect(leTmpPath, SIGNAL(textChanged(QString)), this, SLOT(lineEditChanged(QString)));

	lineEditChanged(tmpPath);
}

void DkGlobalSettingsWidget::createLayout() {
	
	QVBoxLayout* vboxLayout = new QVBoxLayout(this);
	
	QWidget* leftGroupBoxWidget = new QWidget(this);
	QVBoxLayout* gbLeftLayout = new QVBoxLayout(leftGroupBoxWidget);
	gbLeftLayout->setMargin(0);

	QWidget* rightGroupBoxWidget = new QWidget(this);
	QVBoxLayout* gbRightLayout = new QVBoxLayout(rightGroupBoxWidget);
	gbRightLayout->setMargin(0);


	QGroupBox* gbNavigationSettings = new QGroupBox(tr("Global Settings"), this);
	QGridLayout* gbNavigationLayout= new QGridLayout(gbNavigationSettings);
	
	// --- left layout of the global settings widget
	// skip images
	skipImgWidget = new DkSpinBoxWidget(tr("Skip Images:"), tr("on PgUp and PgDown"), 1, 99, this);
	
	// wrap images
	QWidget* checkBoxWidget = new QWidget(this);
	QGridLayout* vbCheckBoxLayout = new QGridLayout(checkBoxWidget);
	//vbCheckBoxLayout->setContentsMargins(11,0,11,0);
	cbWrapImages = new QCheckBox(tr("Wrap Images"));

	vbCheckBoxLayout->addWidget(cbWrapImages, 0, 0);
	//vbCheckBoxLayout->addWidget(pbOpenWith, 0, 1, 1, 1, Qt::AlignRight);
	//vbCheckBoxLayout->addStretch();


	// ---- right layout of the global settings widget
	QWidget* langWidget = new QWidget(this);
	QGridLayout* langLayout = new QGridLayout(langWidget);
	langLayout->setMargin(0);
	QLabel* langLabel = new QLabel("choose language:");
	langCombo = new QComboBox(this);

	QDir qmDir = QDir();
	QStringList fileNames = qmDir.entryList(QStringList("nomacs_*.qm"));

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

	QLabel* translateLabel = new QLabel("<a href=\"http://www.nomacs.org/how-to-translate-nomacs\">translate nomacs</a>", this);
	translateLabel->setToolTip(tr("if you want to help us and translate nomacs"));
	QFont font;
	font.setPointSize(7);
	translateLabel->setFont(font);
	translateLabel->setOpenExternalLinks(true);

	langLayout->addWidget(langLabel,0,0);
	langLayout->addWidget(langCombo,1,0);
	langLayout->addWidget(translateLabel,2,0,Qt::AlignRight);

	QPushButton* pbOpenWith = new QPushButton(tr("&Open With"), this);
	connect(pbOpenWith, SIGNAL(clicked()), this, SLOT(openWithDialog()));

	// ---- drag and drop groupbox


	QGroupBox* gbDragDrop = new QGroupBox(tr("Drag && Drop"), this);
	QVBoxLayout* vboxGbDragDrop = new QVBoxLayout(gbDragDrop);

	// tmp path
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

	vbTmpPathWidget->addWidget(cbUseTmpPath);
	vbTmpPathWidget->addWidget(lineEditWidget);

	vboxGbDragDrop->addWidget(tmpPathWidget);

	// set to default
	QWidget* defaultSettingsWidget = new QWidget(this);
	QHBoxLayout* defaultSettingsLayout = new QHBoxLayout(defaultSettingsWidget);
	defaultSettingsLayout->setContentsMargins(11,0,11,0);
	defaultSettingsLayout->setDirection(QHBoxLayout::RightToLeft);
	buttonDefaultSettings = new QPushButton(tr("Apply default settings"), this);
	defaultSettingsLayout->addWidget(buttonDefaultSettings);
	defaultSettingsLayout->addStretch();

	gbLeftLayout->addWidget(skipImgWidget);
	gbLeftLayout->addWidget(checkBoxWidget);
	gbLeftLayout->addStretch();
	gbRightLayout->addWidget(langWidget);
	gbRightLayout->addWidget(pbOpenWith, 0, Qt::AlignRight);
	gbRightLayout->addStretch();
	gbNavigationLayout->addWidget(leftGroupBoxWidget, 0, 0);
	gbNavigationLayout->setColumnStretch(0, 5);
	//gbRightLayout->addWidget(checkBoxWidget);
	gbRightLayout->addStretch();
	
	gbNavigationLayout->addWidget(rightGroupBoxWidget, 0, 1);
	gbNavigationLayout->setColumnStretch(1, 5);

	
	vboxLayout->addWidget(gbNavigationSettings);
	vboxLayout->addWidget(gbDragDrop);
	vboxLayout->addStretch();
	vboxLayout->addWidget(defaultSettingsWidget);
}

void DkGlobalSettingsWidget::openWithDialog() {

	DkOpenWithDialog* openWithDialog = new DkOpenWithDialog(this);
	openWithDialog->exec();

	delete openWithDialog;
}

void DkGlobalSettingsWidget::writeSettings() {
	
	DkSettings::GlobalSettings::skipImgs = skipImgWidget->getSpinBoxValue();
	DkSettings::GlobalSettings::loop = cbWrapImages->isChecked();
	DkSettings::GlobalSettings::useTmpPath = cbUseTmpPath->isChecked();
	DkSettings::GlobalSettings::tmpPath = existsDirectory(leTmpPath->text()) ? leTmpPath->text() : QString();
	DkSettings::GlobalSettings::language = languages.at(langCombo->currentIndex());
}

void DkGlobalSettingsWidget::tmpPathButtonPressed() {
	tmpPath = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),tmpPath);

	if (tmpPath.isEmpty())
		return;
	
	leTmpPath->setText(tmpPath);
}

void DkGlobalSettingsWidget::useTmpPathChanged(int state) {
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


void DkGlobalSettingsWidget::lineEditChanged(QString path) {
	existsDirectory(path) ? leTmpPath->setStyleSheet("color:black") : leTmpPath->setStyleSheet("color:red");
}

bool DkGlobalSettingsWidget::existsDirectory(QString path) {
	QFileInfo* fi = new QFileInfo(path);
	return fi->exists();
}


// DkDisplaySettingsWidget --------------------------------------------------------------------
DkDisplaySettingsWidget::DkDisplaySettingsWidget(QWidget* parent) : DkSettingsWidget(parent) {
	createLayout();
	init();
}

void DkDisplaySettingsWidget::init() {

	cbInvertZoom->setChecked(DkSettings::DisplaySettings::invertZoom);
	cbKeepZoom->setChecked(DkSettings::DisplaySettings::keepZoom);
	setHighlightColor(DkSettings::DisplaySettings::highlightColor);
	maximalThumbSizeWidget->setSpinBoxValue(DkSettings::DisplaySettings::thumbSize);
	cbSaveThumb->setChecked(DkSettings::DisplaySettings::saveThumb);
	interpolateWidget->setSpinBoxValue(DkSettings::DisplaySettings::interpolateZoomLevel);
	cbShowMenu->setChecked(DkSettings::AppSettings::showMenuBar);
	cbShowStatusbar->setChecked(DkSettings::AppSettings::showStatusBar);
	cbShowToolbar->setChecked(DkSettings::AppSettings::showToolBar);

	connect(highlightColorButton, SIGNAL(clicked()), this, SLOT(highlightButtonClicked()));
	connect(highlightColorResetButton, SIGNAL(clicked()), this, SLOT(resetHighlightColor()));
	connect(colorDialog, SIGNAL(accepted()), this, SLOT(highlightDialogClosed()));

}

void DkDisplaySettingsWidget::createLayout() {
	QVBoxLayout* vboxLayout = new QVBoxLayout(this);

	QWidget* leftGroupBoxWidget = new QWidget(this);
	QVBoxLayout* gbLeftLayout = new QVBoxLayout(leftGroupBoxWidget);
	gbLeftLayout->setMargin(0);

	QWidget* rightGroupBoxWidget = new QWidget(this);
	QVBoxLayout* gbRightLayout = new QVBoxLayout(rightGroupBoxWidget);
	gbRightLayout->setMargin(0);


	QGroupBox* gbDisplaySettings = new QGroupBox(tr("Display Settings"), this);
	QGridLayout* gbLayout= new QGridLayout(gbDisplaySettings);



	colorDialog = new QColorDialog(QColor("gray"), this);
	QWidget* highlightColorWidget = new QWidget(this);
	QVBoxLayout* highlightVLayout = new QVBoxLayout(highlightColorWidget);
	highlightVLayout->setContentsMargins(11,0,11,0);
	highlightColorLabel = new QLabel(tr("Highlight Color:"));
	highlightColorButton = new QPushButton("", this);
	highlightColorButton->setFlat(true);
	highlightColorResetButton = new QPushButton(tr("Reset"), this);
	highlightVLayout->addWidget(highlightColorLabel);

	QWidget* checkBoxWidget = new QWidget(this);
	QVBoxLayout* vbCheckBoxLayout = new QVBoxLayout(checkBoxWidget);
	vbCheckBoxLayout->setContentsMargins(11,0,11,0);
	cbInvertZoom = new QCheckBox(tr("Invert Zoom"), this);
	cbKeepZoom = new QCheckBox(tr("Keep Zoom"), this);
	vbCheckBoxLayout->addWidget(cbInvertZoom);
	vbCheckBoxLayout->addWidget(cbKeepZoom);

	QWidget* highlightColWidget = new QWidget(this);
	QHBoxLayout* highlightHLayout = new QHBoxLayout(highlightColWidget);
	highlightHLayout->setContentsMargins(11,0,11,0);

	highlightHLayout->addWidget(highlightColorButton);
	highlightHLayout->addWidget(highlightColorResetButton);
	highlightHLayout->addStretch();

	highlightVLayout->addWidget(highlightColWidget);


	interpolateWidget = new DkSpinBoxWidget(tr("Stop interpolating at:"), tr("% zoom level"), 0, 7000, this, 10);

	cbShowMenu = new QCheckBox(tr("show Menu"), this);
	cbShowToolbar = new QCheckBox(tr("show Toolbar"), this);
	cbShowStatusbar = new QCheckBox(tr("show Statusbar"), this);

	QGroupBox* gbThumb = new QGroupBox(tr("Thumbnails"));
	QGridLayout* gbHbox = new QGridLayout(gbThumb);

	maximalThumbSizeWidget = new DkSpinBoxWidget(tr("maximal size:"), tr("pixel"), 30, 160, this);
	maximalThumbSizeWidget->setSpinBoxValue(DkSettings::DisplaySettings::thumbSize);

	cbSaveThumb = new QCheckBox(tr("save Thumbnails"), this);
	cbSaveThumb->setToolTip(tr("saves thumbnails to images (EXPERIMENTAL)"));

	gbHbox->addWidget(maximalThumbSizeWidget, 0, 0);
	gbHbox->setColumnStretch(0,5);
	gbHbox->addWidget(cbSaveThumb, 0, 1);
	gbHbox->setColumnStretch(1,5);

	gbLeftLayout->addWidget(highlightColorWidget);
	gbLeftLayout->addStretch();
	gbLeftLayout->addWidget(checkBoxWidget);
	gbLeftLayout->addStretch();

	gbRightLayout->addWidget(interpolateWidget);
	gbRightLayout->addWidget(cbShowMenu);
	gbRightLayout->addWidget(cbShowToolbar);
	gbRightLayout->addWidget(cbShowStatusbar);
	gbRightLayout->addStretch();

	gbLayout->addWidget(leftGroupBoxWidget, 0, 0);
	gbLayout->addWidget(rightGroupBoxWidget, 0, 1);

	vboxLayout->addWidget(gbDisplaySettings);
	vboxLayout->addWidget(gbThumb);
	vboxLayout->addStretch();
}

void DkDisplaySettingsWidget::writeSettings() {

	DkSettings::DisplaySettings::invertZoom = (cbInvertZoom->isChecked()) ? true : false;
	DkSettings::DisplaySettings::keepZoom = (cbKeepZoom->isChecked()) ? true : false;
	DkSettings::DisplaySettings::highlightColor = colorDialog->currentColor();
	DkSettings::DisplaySettings::thumbSize = maximalThumbSizeWidget->getSpinBoxValue();
	DkSettings::DisplaySettings::saveThumb = cbSaveThumb->isChecked();
	DkSettings::DisplaySettings::interpolateZoomLevel = interpolateWidget->getSpinBoxValue();
	DkSettings::AppSettings::showMenuBar = cbShowMenu->isChecked();
	DkSettings::AppSettings::showStatusBar = cbShowStatusbar->isChecked();
	DkSettings::AppSettings::showToolBar = cbShowToolbar->isChecked();

}

void DkDisplaySettingsWidget::setHighlightColor(QColor newColor) {
	colorDialog->setCurrentColor(newColor);
	highlightColorButton->setStyleSheet("QPushButton {background-color: " + newColor.name() + ";border:0px; min-height:24px}");
}

// DkSlideshowSettingsWidget --------------------------------------------------------------------

DkSlideshowSettingsWidget::DkSlideshowSettingsWidget(QWidget* parent) : DkSettingsWidget(parent) {
	createLayout();
	init();
}

void DkSlideshowSettingsWidget::init() {

	//spFilter->setValue(DkSettings::SlideShowSettings::filter);
	timeWidget->setSpinBoxValue(DkSettings::SlideShowSettings::time);
	setBackgroundColor(DkSettings::SlideShowSettings::backgroundColor);

	cbName->setChecked(DkSettings::SlideShowSettings::display.testBit(display_file_name));
	cbCreationDate->setChecked(DkSettings::SlideShowSettings::display.testBit(display_creation_date));
	cbRating->setChecked(DkSettings::SlideShowSettings::display.testBit(display_file_rating));
	cbSilentFullscreen->setChecked(DkSettings::SlideShowSettings::silentFullscreen);

	connect(buttonBackgroundColor, SIGNAL(clicked()), this, SLOT(backgroundButtonClicked()));
	connect(buttonResetBackground, SIGNAL(clicked()), this, SLOT(resetBackground()));
	connect(colorDialog, SIGNAL(accepted()), this, SLOT(backgroundDialogClosed()));
	connect(cbName, SIGNAL(clicked(bool)), this, SLOT(showFileName(bool)));
	connect(cbCreationDate, SIGNAL(clicked(bool)), this, SLOT(showCreationDate(bool)));
	connect(cbRating, SIGNAL(clicked(bool)), this, SLOT(showRating(bool)));
	
}

void DkSlideshowSettingsWidget::createLayout() {
	vBoxLayout = new QVBoxLayout(this);
	//vBoxLayout->setMargin(0);

	// slideshow settings
	QGroupBox* gbSlideShow = new QGroupBox(tr("Slideshow Settings"), this);
	QVBoxLayout* gbSlideShowLayout = new QVBoxLayout(gbSlideShow);

	timeWidget = new DkSpinBoxWidget(tr("Display Time:"), tr("sec"), 1, 99, this );
	
	// fullscreen groupbox
	QGroupBox* gbFullscreen = new QGroupBox(tr("Fullscreen Settings"), this);
	QHBoxLayout* gbFullscreenLayout = new QHBoxLayout(gbFullscreen);

	colorDialog = new QColorDialog(QColor("gray"), this);
	QWidget* backgroundWidget = new QWidget(this);
	QVBoxLayout* backgroundVLayout = new QVBoxLayout(backgroundWidget);
	backgroundVLayout->setContentsMargins(11,0,11,0);
	labelBackgroundText = new QLabel(tr("Background color:"), this);
	buttonBackgroundColor = new QPushButton("");
	buttonBackgroundColor->setFlat(true);
	buttonResetBackground = new QPushButton(tr("Reset"), this);
	backgroundVLayout->addWidget(labelBackgroundText);

	QWidget* backgroundColWidget = new QWidget(this);
	QHBoxLayout* backgroundLayout = new QHBoxLayout(backgroundColWidget);

	backgroundLayout->addWidget(buttonBackgroundColor);
	backgroundLayout->addWidget(buttonResetBackground);
	backgroundLayout->addStretch();
	backgroundVLayout->addWidget(backgroundColWidget);

	cbSilentFullscreen = new QCheckBox(tr("Silent Fullscreen"));


	// display information
	gbInfo = new QGroupBox(tr("Display Information"));
	QVBoxLayout* gbLayout = new QVBoxLayout(gbInfo);

	cbName = new QCheckBox(tr("Image Name"));
	gbLayout->addWidget(cbName);

	cbCreationDate = new QCheckBox(tr("Creation Date"));
	gbLayout->addWidget(cbCreationDate);

	cbRating = new QCheckBox(tr("Rating"));
	gbLayout->addWidget(cbRating);

	gbSlideShowLayout->addWidget(timeWidget);
	gbFullscreenLayout->addWidget(backgroundWidget);
	gbFullscreenLayout->addStretch();
	gbFullscreenLayout->addWidget(cbSilentFullscreen);
	gbFullscreenLayout->addStretch();
	vBoxLayout->addWidget(gbSlideShow);
	vBoxLayout->addWidget(gbFullscreen);
	vBoxLayout->addWidget(gbInfo);
	vBoxLayout->addStretch();
}

void DkSlideshowSettingsWidget::writeSettings() {
	DkSettings::SlideShowSettings::time = timeWidget->getSpinBoxValue();
	DkSettings::SlideShowSettings::backgroundColor = colorDialog->currentColor();

	DkSettings::SlideShowSettings::display.setBit(display_file_name, cbName->isChecked());
	DkSettings::SlideShowSettings::display.setBit(display_creation_date, cbCreationDate->isChecked());
	DkSettings::SlideShowSettings::display.setBit(display_file_rating, cbRating->isChecked());

	DkSettings::SlideShowSettings::silentFullscreen = cbSilentFullscreen->isChecked();
}

void DkSlideshowSettingsWidget::backgroundButtonClicked() {
	colorDialog->show();
}

void DkSlideshowSettingsWidget::backgroundDialogClosed() {
	setBackgroundColor(colorDialog->currentColor());
}

void DkSlideshowSettingsWidget::setBackgroundColor(QColor newColor) {
	colorDialog->setCurrentColor(newColor);
	buttonBackgroundColor->setStyleSheet("QPushButton {background-color: "+ newColor.name()+";border:0px; min-height:24px}");
}

void DkSlideshowSettingsWidget::resetBackground() {
	setBackgroundColor(QColor(200, 200, 200));
}

void DkSlideshowSettingsWidget::showFileName(bool checked) {
	DkSettings::SlideShowSettings::display.setBit(display_file_name, checked);
}

void DkSlideshowSettingsWidget::showCreationDate(bool checked) {
	DkSettings::SlideShowSettings::display.setBit(display_creation_date, checked);
}

void DkSlideshowSettingsWidget::showRating(bool checked) {
	DkSettings::SlideShowSettings::display.setBit(display_file_rating, checked);
}

// DkNetworkSettingsWidget --------------------------------------------------------------------

DkSynchronizeSettingsWidget::DkSynchronizeSettingsWidget(QWidget* parent) : DkSettingsWidget(parent) {
	createLayout();
	init();
}

void DkSynchronizeSettingsWidget::init() {
	connect(cbEnableNetwork, SIGNAL(stateChanged(int)), this, SLOT(enableNetworkCheckBoxChanged(int)));

	cbAllowFile->setChecked(DkSettings::SynchronizeSettings::allowFile);
	cbAllowImage->setChecked(DkSettings::SynchronizeSettings::allowImage);
	cbAllowPosition->setChecked(DkSettings::SynchronizeSettings::allowPosition);
	cbAllowTransformation->setChecked(DkSettings::SynchronizeSettings::allowTransformation);
	cbEnableNetwork->setChecked(DkSettings::SynchronizeSettings::enableNetworkSync);
	DkSettings::SynchronizeSettings::syncAbsoluteTransform ? rbSyncAbsoluteTransform->setChecked(true) : rbSyncRelativeTransform->setChecked(true);
	cbSwitchModifier->setChecked(DkSettings::SynchronizeSettings::switchModifier);

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


	QGroupBox* gbNetworkSettings = new QGroupBox(tr("Network Synchronization"), this);
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


	cbSwitchModifier = new QCheckBox(tr("switch ALT and CTRL key"), this);
	

	gbNetworkSettingsLayout->addWidget(cbEnableNetwork);
	gbNetworkSettingsLayout->addWidget(networkSettings);
	vboxLayout->addWidget(gbSyncSettings);
	vboxLayout->addWidget(gbNetworkSettings);
	vboxLayout->addWidget(cbSwitchModifier);
	vboxLayout->addStretch();
}

void DkSynchronizeSettingsWidget::writeSettings() {
	DkSettings::SynchronizeSettings::enableNetworkSync = cbEnableNetwork->isChecked();
	DkSettings::SynchronizeSettings::allowFile = cbAllowFile->isChecked();
	DkSettings::SynchronizeSettings::allowImage = cbAllowImage->isChecked();
	DkSettings::SynchronizeSettings::allowPosition = cbAllowPosition->isChecked();
	DkSettings::SynchronizeSettings::allowTransformation = cbAllowTransformation->isChecked();
	DkSettings::SynchronizeSettings::syncAbsoluteTransform = rbSyncAbsoluteTransform->isChecked();
	DkSettings::SynchronizeSettings::switchModifier = cbSwitchModifier->isChecked();
	if (DkSettings::SynchronizeSettings::switchModifier) {
		DkSettings::GlobalSettings::altMod = Qt::ControlModifier;
		DkSettings::GlobalSettings::ctrlMod = Qt::AltModifier;
	}
	else {
		DkSettings::GlobalSettings::altMod = Qt::AltModifier;
		DkSettings::GlobalSettings::ctrlMod = Qt::ControlModifier;
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
	createLayout();
	init();
}

void DkMetaDataSettingsWidget::init() {

	for (int i=0; i<desc_end;i++) {
		pCbMetaData[i]->setChecked(DkSettings::MetaDataSettings::metaDataBits[i]);
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
		DkSettings::MetaDataSettings::metaDataBits[i] = pCbMetaData[i]->isChecked();
	}
}

// DkSpinBoxWiget --------------------------------------------------------------------
DkSpinBoxWidget::DkSpinBoxWidget(QWidget* parent) : QWidget(parent) {
	spinBox = new QSpinBox(this);
	lowerLabel = new QLabel(this);
	lowerWidget = new QWidget(this);
	vboxLayout = new QVBoxLayout ;
	hboxLowerLayout = new QHBoxLayout;

	hboxLowerLayout->addWidget(spinBox);
	hboxLowerLayout->addWidget(lowerLabel);
	hboxLowerLayout->addStretch();
	vboxLayout->addWidget(upperLabel);
	vboxLayout->addWidget(lowerWidget);

}

DkSpinBoxWidget::DkSpinBoxWidget(QString upperString, QString lowerString, int spinBoxMin, int spinBoxMax, QWidget* parent/* =0 */, int step/* =1*/) : QWidget(parent) {
	spinBox = new QSpinBox(this);
	spinBox->setMaximum(spinBoxMax);
	spinBox->setMinimum(spinBoxMin);
	spinBox->setSingleStep(step);
	upperLabel = new QLabel(upperString, this);
	lowerLabel = new QLabel(lowerString, this);
	lowerWidget = new QWidget(this);

	vboxLayout = new QVBoxLayout(this) ;
	hboxLowerLayout = new QHBoxLayout(lowerWidget);

	hboxLowerLayout->addWidget(spinBox);
	hboxLowerLayout->addWidget(lowerLabel);
	hboxLowerLayout->addStretch();
	vboxLayout->addWidget(upperLabel);
	vboxLayout->addWidget(lowerWidget);


}

}
