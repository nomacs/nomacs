/*******************************************************************************************************
DkSettingsWidgets.cpp
Created on:	17.03.2014
 
nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#include "DkSettingsWidgets.h"
#include "DkDialog.h"
#include "DkUtils.h"
#include "BorderLayout.h"
#include "DkWidgets.h"
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QObject>
#include <QModelIndex>
#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QListView>
#include <QKeyEvent>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <QFileDialog>
#include <QStandardItem>
#include <QTableView>
#include <QStyledItemDelegate>
#include <QStringListModel>
#include <QWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QGraphicsOpacityEffect>
#include <QHeaderView>
#include <QApplication>
#include <QPainter>
#pragma warning(pop)		// no warnings from includes - end

namespace nmc {

// DkSettingsDialog --------------------------------------------------------------------
DkSettingsDialog::DkSettingsDialog(QWidget* parent) : QDialog(parent) {

	//this->resize(600,420);

	createLayout();
	createSettingsWidgets();
	for (int i = 0; i < widgetList.size(); i++) {
		if (!DkSettings::app.advancedSettings) {
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
		curWidget->toggleAdvancedOptions(DkSettings::app.advancedSettings);
		centralLayout->addWidget(curWidget);
	}
	widgetList[0]->show(); // display first;
	cbAdvancedSettings->setChecked(DkSettings::app.advancedSettings);
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
	stringList << tr("General") << tr("Display") << tr("File Info") << tr("Synchronize") << tr("Exif") << tr("File Filters") << tr("Resources") << tr("Whitelist");
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
	fileFilterSettingsWidget = new DkFileFilterSettingWidget(centralWidget);
	remoteControlWidget = new DkRemoteControlWidget(centralWidget);

	widgetList.clear();
	widgetList.push_back(globalSettingsWidget);
	widgetList.push_back(displaySettingsWidget);
	widgetList.push_back(slideshowSettingsWidget);
	widgetList.push_back(synchronizeSettingsWidget);
	widgetList.push_back(exifSettingsWidget);
	widgetList.push_back(fileFilterSettingsWidget);
	widgetList.push_back(resourceSettingsWidget);
	widgetList.push_back(remoteControlWidget);
}

void DkSettingsDialog::setToDefault() {

	DkSettings::setToDefaultSettings();
	initWidgets();

	// for main window
	emit setToDefaultSignal();
	emit settingsChanged();
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

	QString curLanguage = DkSettings::global.language;
	QColor curBgColWidget = DkSettings::display.bgColorWidget;
	QColor curBgCol = DkSettings::display.bgColor;
	QColor curIconCol = DkSettings::display.iconColor;
	QColor curBgColFrameless = DkSettings::display.bgColorFrameless;
	bool curIcons = DkSettings::display.smallIcons;
	bool curGradient = DkSettings::display.toolbarGradient;
	bool curUseCol = DkSettings::display.useDefaultColor;
	bool curUseIconCol = DkSettings::display.defaultIconColor;

	foreach (DkSettingsWidget* curWidget, widgetList) {
		curWidget->writeSettings();
	}

	DkSettings* settings = new DkSettings();
	settings->save();
	this->close();

	if (DkSettings::app.privateMode) {
		QMessageBox::information(this, tr("Private Mode"), tr("Settings are not saved in the private mode"), QMessageBox::Ok, QMessageBox::Ok);
	}

	// if the language changed we need to restart nomacs (re-translating while running is pretty hard to accomplish)
	if (!DkSettings::app.privateMode && (curLanguage != DkSettings::global.language ||
		DkSettings::display.bgColor != curBgCol ||
		DkSettings::display.iconColor != curIconCol ||
		DkSettings::display.bgColorWidget != curBgColWidget ||
		DkSettings::display.bgColorFrameless != curBgColFrameless ||
		DkSettings::display.useDefaultColor != curUseCol ||
		DkSettings::display.defaultIconColor != curUseIconCol ||
		DkSettings::display.smallIcons != curIcons ||
		DkSettings::display.toolbarGradient != curGradient))
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

void DkSettingsDialog::advancedSettingsChanged(int) {

	DkSettings::app.advancedSettings = cbAdvancedSettings->isChecked();

	QModelIndex selection = listView->currentIndex();

	foreach (DkSettingsWidget* curWidget, widgetList) {
		curWidget->toggleAdvancedOptions(DkSettings::app.advancedSettings);
	}

	bool wasSelected = false;
	for (int i = 0; i < widgetList.size(); i++) {
		if (!DkSettings::app.advancedSettings) {
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
	cbShowMenu->setChecked(DkSettings::app.showMenuBar);
	cbShowStatusbar->setChecked(DkSettings::app.showStatusBar);
	cbShowToolbar->setChecked(DkSettings::app.showToolBar);
	cbSmallIcons->setChecked(DkSettings::display.smallIcons);
	cbToolbarGradient->setChecked(DkSettings::display.toolbarGradient);
	cbCloseOnEsc->setChecked(DkSettings::app.closeOnEsc);
	cbShowRecentFiles->setChecked(DkSettings::app.showRecentFiles);
	cbZoomOnWheel->setChecked(DkSettings::global.zoomOnWheel);
	cbCheckForUpdates->setChecked(DkSettings::sync.checkForUpdates);

	curLanguage = DkSettings::global.language;
	langCombo->setCurrentIndex(languages.indexOf(curLanguage));
	if (langCombo->currentIndex() == -1) // set index to English if language has not been found
		langCombo->setCurrentIndex(0);

	displayTimeSpin->setSpinBoxValue(DkSettings::slideShow.time);

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
	highlightColorChooser->setColor(DkSettings::display.highlightColor);

	iconColorChooser = new DkColorChooser(QColor(219, 89, 2, 255), tr("Icon Color"), this);
	iconColorChooser->setColor(DkSettings::display.iconColor);
	connect(iconColorChooser, SIGNAL(resetClicked()), this, SLOT(iconColorReset()));

	bgColorChooser = new DkColorChooser(QColor(100, 100, 100, 255), tr("Background Color"), this);
	bgColorChooser->setColor(DkSettings::display.bgColor);
	connect(bgColorChooser, SIGNAL(resetClicked()), this, SLOT(bgColorReset()));

	bgColorWidgetChooser = new DkColorChooser(QColor(0, 0, 0, 100), tr("Widget Color"), this);
	bgColorWidgetChooser->setColor((DkSettings::app.appMode == DkSettings::mode_frameless) ?
		DkSettings::display.bgColorFrameless : DkSettings::display.bgColorWidget);

	fullscreenColChooser = new DkColorChooser(QColor(86,86,90), tr("Fullscreen Color"), this);
	fullscreenColChooser->setColor(DkSettings::slideShow.backgroundColor);

	displayTimeSpin = new DkDoubleSpinBoxWidget(tr("Display Time:"), tr("sec"), 0.1f, 99, this, 1, 1);

	QWidget* langWidget = new QWidget(rightWidget);
	QGridLayout* langLayout = new QGridLayout(langWidget);
	langLayout->setMargin(0);
	QLabel* langLabel = new QLabel("choose language:", langWidget);
	langCombo = new QComboBox(langWidget);
	DkUtils::addLanguages(langCombo, languages);

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
	cbShowRecentFiles = new QCheckBox(tr("Show Recent Files on Start"), showBarsWidget);
	cbZoomOnWheel = new QCheckBox(tr("Mouse Wheel Zooms"), showBarsWidget);
	cbZoomOnWheel->setToolTip(tr("If unchecked, the mouse wheel switches between images."));
	cbZoomOnWheel->setMinimumSize(cbZoomOnWheel->sizeHint());
	cbCheckForUpdates = new QCheckBox(tr("Check for Updates"), showBarsWidget);
	showBarsLayout->addWidget(cbShowMenu);
	showBarsLayout->addWidget(cbShowToolbar);
	showBarsLayout->addWidget(cbShowStatusbar);
	showBarsLayout->addWidget(cbShowRecentFiles);
	showBarsLayout->addWidget(cbSmallIcons);
	showBarsLayout->addWidget(cbToolbarGradient);
	showBarsLayout->addWidget(cbCloseOnEsc);
	showBarsLayout->addWidget(cbZoomOnWheel);
	showBarsLayout->addWidget(cbCheckForUpdates);

#ifdef Q_WS_X11 // hide checkbox in linux
	cbCheckForUpdates->hide();
#endif

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
	DkSettings::app.showMenuBar = cbShowMenu->isChecked();
	DkSettings::app.showStatusBar = cbShowStatusbar->isChecked();
	DkSettings::app.showToolBar = cbShowToolbar->isChecked();
	DkSettings::app.closeOnEsc = cbCloseOnEsc->isChecked();
	DkSettings::app.showRecentFiles = cbShowRecentFiles->isChecked();
	DkSettings::global.zoomOnWheel = cbZoomOnWheel->isChecked();
	DkSettings::display.smallIcons = cbSmallIcons->isChecked();
	DkSettings::display.toolbarGradient = cbToolbarGradient->isChecked();
	DkSettings::slideShow.time = displayTimeSpin->getSpinBoxValue();
	DkSettings::sync.checkForUpdates = cbCheckForUpdates->isChecked();

	if (DkSettings::app.appMode == DkSettings::mode_frameless)
		DkSettings::display.bgColorFrameless = bgColorWidgetChooser->getColor();
	else
		DkSettings::display.bgColorWidget = bgColorWidgetChooser->getColor();

	if (bgColorChooser->isAccept())
		DkSettings::display.useDefaultColor = false;

	if (iconColorChooser->isAccept())
		DkSettings::display.defaultIconColor = false;

	DkSettings::display.iconColor = iconColorChooser->getColor();
	DkSettings::display.bgColor = bgColorChooser->getColor();
	DkSettings::display.highlightColor = highlightColorChooser->getColor();
	DkSettings::slideShow.backgroundColor = fullscreenColChooser->getColor();

	DkSettings::global.language = languages.at(langCombo->currentIndex());
}

void DkGlobalSettingsWidget::setToDefaultPressed() {
	qDebug() << "apply default pressed...";
	emit applyDefault();
}

void DkGlobalSettingsWidget::bgColorReset() {
	DkSettings::display.useDefaultColor = true;
}

void DkGlobalSettingsWidget::iconColorReset() {
	DkSettings::display.defaultIconColor = true;
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
	cbName->setChecked(DkSettings::slideShow.display.testBit(DkSettings::display_file_name));
	cbCreationDate->setChecked(DkSettings::slideShow.display.testBit(DkSettings::display_creation_date));
	cbRating->setChecked(DkSettings::slideShow.display.testBit(DkSettings::display_file_rating));

	cbInvertZoom->setChecked(DkSettings::display.invertZoom);
	keepZoomButtons[DkSettings::display.keepZoom]->setChecked(true);
	maximalThumbSizeWidget->setSpinBoxValue(DkSettings::display.thumbSize);
	fadeSlideShow->setSpinBoxValue(DkSettings::display.fadeSec);
	//cbSaveThumb->setChecked(DkSettings::display.saveThumb);
	interpolateWidget->setSpinBoxValue(DkSettings::display.interpolateZoomLevel);

	cbShowBorder->setChecked(DkSettings::display.showBorder);
	cbSilentFullscreen->setChecked(DkSettings::slideShow.silentFullscreen);
}

void DkDisplaySettingsWidget::createLayout() {
	
	QGridLayout* gridLayout = new QGridLayout(this);

	QGroupBox* gbZoom = new QGroupBox(tr("Zoom"), this);
	gbZoom->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QVBoxLayout* gbZoomLayout = new QVBoxLayout(gbZoom);
	interpolateWidget = new DkSpinBoxWidget(tr("Stop interpolating at:"), tr("% zoom level"), 0, 7000, this, 10);
	QWidget* zoomCheckBoxes = new QWidget(this);
	zoomCheckBoxes->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QVBoxLayout* vbCheckBoxLayout = new QVBoxLayout(zoomCheckBoxes);
	vbCheckBoxLayout->setContentsMargins(11,0,11,0);
	cbInvertZoom = new QCheckBox(tr("Invert Zoom"), this);


	//QGroupBox* gbRawLoader = new QGroupBox(tr("Keep Zoom Settings"), this);

	keepZoomButtonGroup = new QButtonGroup(this);

	keepZoomButtons.resize(DkSettings::raw_thumb_end);
	keepZoomButtons[DkSettings::zoom_always_keep] = new QRadioButton(tr("Always keep zoom"), this);
	keepZoomButtons[DkSettings::zoom_keep_same_size] = new QRadioButton(tr("Keep zoom if equal size"), this);
	keepZoomButtons[DkSettings::zoom_keep_same_size]->setToolTip(tr("If checked, the zoom level is only kept, if the image loaded has the same level as the previous."));
	keepZoomButtons[DkSettings::zoom_never_keep] = new QRadioButton(tr("Never keep zoom"), this);

	QWidget* keepZoomWidget = new QWidget(this);
	keepZoomWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QVBoxLayout* keepZoomButtonLayout = new QVBoxLayout(keepZoomWidget);

	for (int idx = 0; idx < keepZoomButtons.size(); idx++) {
		keepZoomButtonGroup->addButton(keepZoomButtons[idx]);
		keepZoomButtonLayout->addWidget(keepZoomButtons[idx]);
	}

	//QGridLayout* rawLoaderLayout = new QGridLayout(gbRawLoader);
	//cbFilterRawImages = new QCheckBox(tr("filter raw images"));

	//rawLoaderLayout->addWidget(rawThumbWidget);
	//rawLoaderLayout->addWidget(dupWidget);
	//rawLoaderLayout->addWidget(cbFilterRawImages);


	vbCheckBoxLayout->addWidget(cbInvertZoom);
	vbCheckBoxLayout->addWidget(keepZoomWidget);
	gbZoomLayout->addWidget(interpolateWidget);
	gbZoomLayout->addWidget(zoomCheckBoxes);

	QGroupBox* gbThumbs = new QGroupBox(tr("Thumbnails"), this);
	QVBoxLayout* gbThumbsLayout = new QVBoxLayout(gbThumbs);
	maximalThumbSizeWidget = new DkSpinBoxWidget(tr("maximal size:"), tr("pixel"), 16, 160, gbThumbs);
	maximalThumbSizeWidget->setSpinBoxValue(DkSettings::display.thumbSize);
	//cbSaveThumb = new QCheckBox(tr("save Thumbnails"), this);
	//cbSaveThumb->setToolTip(tr("saves thumbnails to images (EXPERIMENTAL)"));
	gbThumbsLayout->addWidget(maximalThumbSizeWidget);
	//gbThumbsLayout->addWidget(cbSaveThumb);

	QGroupBox* gbFileInfo = new QGroupBox(tr("File Information"), this);
	QVBoxLayout* gbLayout = new QVBoxLayout(gbFileInfo);
	cbName = new QCheckBox(tr("Image Name"));
	gbLayout->addWidget(cbName);
	cbCreationDate = new QCheckBox(tr("Creation Date"));
	gbLayout->addWidget(cbCreationDate);
	cbRating = new QCheckBox(tr("Rating"));
	gbLayout->addWidget(cbRating);

	QGroupBox* gbFrameless = new QGroupBox(tr("Frameless"), this);
	QVBoxLayout* gbFramelessLayout = new QVBoxLayout(gbFrameless);
	cbShowBorder = new QCheckBox(tr("Show Border"));
	gbFramelessLayout->addWidget(cbShowBorder);

	QGroupBox* gbFullscreen = new QGroupBox(tr("Fullscreen"), this);
	QVBoxLayout* gbFullScreenLayout = new QVBoxLayout(gbFullscreen);
	cbSilentFullscreen = new QCheckBox(tr("Silent Fullscreen"));
	gbFullScreenLayout->addWidget(cbSilentFullscreen);

	QGroupBox* gbSlideShow = new QGroupBox(tr("Slide Show"), this);
	QVBoxLayout* gbSlideShowLayout = new QVBoxLayout(gbSlideShow);
	fadeSlideShow = new DkDoubleSpinBoxWidget(tr("Fade Images:"), tr("sec"), 0, 16, this);
	fadeSlideShow->setSpinBoxValue(DkSettings::display.fadeSec);
	gbSlideShowLayout->addWidget(fadeSlideShow);

	gridLayout->addWidget(gbZoom, 0, 0, 3, 1);
	gridLayout->addWidget(gbThumbs, 3, 0);
	gridLayout->addWidget(gbFileInfo, 0, 1);
	gridLayout->addWidget(gbFrameless, 1, 1);
	gridLayout->addWidget(gbFullscreen, 2, 1);
	gridLayout->addWidget(gbSlideShow, 3, 1);

	gridLayout->setRowStretch(4, 10);

	adjustSize();
}

void DkDisplaySettingsWidget::writeSettings() {

	DkSettings::display.invertZoom = (cbInvertZoom->isChecked()) ? true : false;

	for (int idx = 0; idx < keepZoomButtons.size(); idx++) {
		if (keepZoomButtons[idx]->isChecked()) {
			DkSettings::display.keepZoom = idx;
			break;
		}
	}

	DkSettings::slideShow.silentFullscreen = cbSilentFullscreen->isChecked();

	DkSettings::slideShow.display.setBit(DkSettings::display_file_name, cbName->isChecked());
	DkSettings::slideShow.display.setBit(DkSettings::display_creation_date, cbCreationDate->isChecked());
	DkSettings::slideShow.display.setBit(DkSettings::display_file_rating, cbRating->isChecked());

	DkSettings::display.thumbSize = maximalThumbSizeWidget->getSpinBoxValue();
	DkSettings::display.fadeSec = fadeSlideShow->getSpinBoxValue();
	//DkSettings::display.saveThumb = cbSaveThumb->isChecked();
	DkSettings::display.interpolateZoomLevel = interpolateWidget->getSpinBoxValue();
	DkSettings::display.showBorder = cbShowBorder->isChecked();
}

void DkDisplaySettingsWidget::showFileName(bool checked) {
	DkSettings::slideShow.display.setBit(DkSettings::display_file_name, checked);
}

void DkDisplaySettingsWidget::showCreationDate(bool checked) {
	DkSettings::slideShow.display.setBit(DkSettings::display_creation_date, checked);
}

void DkDisplaySettingsWidget::showRating(bool checked) {
	DkSettings::slideShow.display.setBit(DkSettings::display_file_rating, checked);
}


// DkFileWidget --------------------------------------------------------------------

DkFileWidget::DkFileWidget(QWidget* parent) : DkSettingsWidget(parent) {
	showOnlyInAdvancedMode = true;

	createLayout();
	init();
}

void DkFileWidget::init() {

	//spFilter->setValue(DkSettings::SlideShowSettings::filter);

	cbWrapImages->setChecked(DkSettings::global.loop);
	cbAskToSaveDeletedFiles->setChecked(DkSettings::global.askToSaveDeletedFiles);
	cbLogRecentFiles->setChecked(DkSettings::global.logRecentFiles);
	skipImgWidget->setSpinBoxValue(DkSettings::global.skipImgs);
	//numberFiles->setSpinBoxValue(DkSettings::global.numFiles);
	cbUseTmpPath->setChecked(DkSettings::global.useTmpPath);
	tmpPath = DkSettings::global.tmpPath;
	leTmpPath->setText(tmpPath);
	if (!DkSettings::global.useTmpPath) {
		leTmpPath->setDisabled(true);
		pbTmpPath->setDisabled(true);
	}

	connect(pbTmpPath, SIGNAL(clicked()), this, SLOT(tmpPathButtonPressed()));
	connect(cbUseTmpPath, SIGNAL(stateChanged(int)), this, SLOT(useTmpPathChanged(int)));

}

void DkFileWidget::createLayout() {
	
	QVBoxLayout* widgetLayout = new QVBoxLayout(this);
	
	gbDragDrop = new QGroupBox(tr("Drag && Drop"));
	QVBoxLayout* vboxGbDragDrop = new QVBoxLayout(gbDragDrop);
	QWidget* tmpPathWidget = new QWidget(this);
	QVBoxLayout* vbTmpPathWidget = new QVBoxLayout(tmpPathWidget);

	cbUseTmpPath = new QCheckBox(tr("use temporary folder"), this);

	QWidget* lineEditWidget = new QWidget(this);
	QHBoxLayout* hbLineEditWidget = new QHBoxLayout(lineEditWidget);
	leTmpPath = new DkDirectoryEdit(this);
	leTmpPath->setObjectName("DkWarningEdit");
	pbTmpPath = new QPushButton(tr("..."), this);
	pbTmpPath->setMaximumWidth(40);
	hbLineEditWidget->addWidget(leTmpPath);
	hbLineEditWidget->addWidget(pbTmpPath);
	vboxGbDragDrop->addWidget(tmpPathWidget);

	vbTmpPathWidget->addWidget(cbUseTmpPath);
	vbTmpPathWidget->addWidget(lineEditWidget);

	// image loading
	rbSkipImage = new QRadioButton(tr("Skip Images"), this);
	rbSkipImage->setToolTip(tr("Images are skipped until the next button is released."));
	rbSkipImage->setChecked(!DkSettings::resources.waitForLastImg);
	rbWaitForImage = new QRadioButton(tr("Wait for Images to be Loaded"), this);
	rbWaitForImage->setToolTip(tr("A new image is loaded after displaying the last image"));
	rbWaitForImage->setChecked(DkSettings::resources.waitForLastImg);

	QButtonGroup* bgImageLoading = new QButtonGroup(this);
	bgImageLoading->addButton(rbSkipImage);
	bgImageLoading->addButton(rbWaitForImage);

	QGroupBox* gbImageLoading = new QGroupBox(tr("Image Loading Policy"), this);
	QVBoxLayout* imageLoadingLayout = new QVBoxLayout(gbImageLoading);
	imageLoadingLayout->addWidget(rbSkipImage);
	imageLoadingLayout->addWidget(rbWaitForImage);

	skipImgWidget = new DkSpinBoxWidget(tr("Skip Images:"), tr("on PgUp and PgDown"), 1, 99, this);
	//numberFiles = new DkSpinBoxWidget(tr("Number of Recent Files/Folders:"), tr("shown in Menu"), 1, 99, this);
	cbWrapImages = new QCheckBox(tr("Loop Images"));
	cbAskToSaveDeletedFiles = new QCheckBox(tr("Ask to Save Deleted Files"));
	cbAskToSaveDeletedFiles->setToolTip(tr("If checked, nomacs asks if you want to save files that are deleted while displaying."));
	cbLogRecentFiles = new QCheckBox(tr("Log Recent Files"));

	widgetLayout->addWidget(gbDragDrop);
	widgetLayout->addWidget(gbImageLoading);
	
	QGridLayout* leftLayout = new QGridLayout(this);
	leftLayout->addWidget(skipImgWidget, 0, 0);
	leftLayout->addWidget(cbWrapImages, 1, 0);
	leftLayout->addWidget(cbLogRecentFiles, 2, 0);
	leftLayout->addWidget(cbAskToSaveDeletedFiles, 1, 1);
	leftLayout->setRowStretch(3, 10);
	leftLayout->setColumnStretch(3, 10);
	widgetLayout->addLayout(leftLayout);
}

void DkFileWidget::writeSettings() {
	DkSettings::global.skipImgs = skipImgWidget->getSpinBoxValue();
	//DkSettings::global.numFiles = numberFiles->getSpinBoxValue();
	DkSettings::global.loop = cbWrapImages->isChecked();
	DkSettings::global.logRecentFiles = cbLogRecentFiles->isChecked();
	DkSettings::global.useTmpPath = cbUseTmpPath->isChecked();
	DkSettings::global.askToSaveDeletedFiles = cbAskToSaveDeletedFiles->isChecked();
	QFileInfo fi = QFileInfo(leTmpPath->text());
	DkSettings::global.tmpPath = fi.exists() ? leTmpPath->text() : QString();
	DkSettings::resources.waitForLastImg = rbWaitForImage->isChecked();
}


void DkFileWidget::tmpPathButtonPressed() {
	tmpPath = QFileDialog::getExistingDirectory(this, tr("Open an Image Directory"),tmpPath);

	if (tmpPath.isEmpty())
		return;

	leTmpPath->setText(tmpPath);
}

void DkFileWidget::useTmpPathChanged(int) {
	
	if (cbUseTmpPath->isChecked()) {
		//lineEditChanged(tmpPath);
		leTmpPath->setDisabled(false);
		pbTmpPath->setDisabled(false);
	} else {
		leTmpPath->setProperty("error", false);
		leTmpPath->setDisabled(true);
		pbTmpPath->setDisabled(true);
	}

	leTmpPath->style()->unpolish(leTmpPath);
	leTmpPath->style()->polish(leTmpPath);
	leTmpPath->update();
}

// DkNetworkSettingsWidget --------------------------------------------------------------------

DkSynchronizeSettingsWidget::DkSynchronizeSettingsWidget(QWidget* parent) : DkSettingsWidget(parent) {
	showOnlyInAdvancedMode = true;

	createLayout();
	init();
}

void DkSynchronizeSettingsWidget::init() {
	connect(cbEnableNetwork, SIGNAL(stateChanged(int)), this, SLOT(enableNetworkCheckBoxChanged(int)));

	cbAllowFile->setChecked(DkSettings::sync.allowFile);
	cbAllowImage->setChecked(DkSettings::sync.allowImage);
	cbAllowPosition->setChecked(DkSettings::sync.allowPosition);
	cbAllowTransformation->setChecked(DkSettings::sync.allowTransformation);
	cbEnableNetwork->setChecked(DkSettings::sync.enableNetworkSync);
	DkSettings::sync.syncAbsoluteTransform ? rbSyncAbsoluteTransform->setChecked(true) : rbSyncRelativeTransform->setChecked(true);
	cbSwitchModifier->setChecked(DkSettings::sync.switchModifier);

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
	DkSettings::sync.enableNetworkSync = cbEnableNetwork->isChecked();
	DkSettings::sync.allowFile = cbAllowFile->isChecked();
	DkSettings::sync.allowImage = cbAllowImage->isChecked();
	DkSettings::sync.allowPosition = cbAllowPosition->isChecked();
	DkSettings::sync.allowTransformation = cbAllowTransformation->isChecked();
	DkSettings::sync.syncAbsoluteTransform = rbSyncAbsoluteTransform->isChecked();
	DkSettings::sync.switchModifier = cbSwitchModifier->isChecked();
	if (DkSettings::sync.switchModifier) {
		DkSettings::global.altMod = Qt::ControlModifier;
		DkSettings::global.ctrlMod = Qt::AltModifier;
	}
	else {
		DkSettings::global.altMod = Qt::AltModifier;
		DkSettings::global.ctrlMod = Qt::ControlModifier;
	}
}

void DkSynchronizeSettingsWidget::enableNetworkCheckBoxChanged(int) {
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

void DkSettingsListView::keyPressEvent(QKeyEvent *event) {
	if (event->key() == Qt::Key_Up) {
		previousIndex(); 
		return;
	}
	else if (event->key() == Qt::Key_Down) {
		nextIndex(); 
		return;
	}
	QListView::keyPressEvent(event);
}


// DkMetaDataSettings --------------------------------------------------------------------------
DkMetaDataSettingsWidget::DkMetaDataSettingsWidget(QWidget* parent) : DkSettingsWidget(parent) {
	showOnlyInAdvancedMode = true;

	createLayout();
	init();
}

void DkMetaDataSettingsWidget::init() {

	for (int i=0; i<DkSettings::desc_end;i++) {
		pCbMetaData[i]->setChecked(DkSettings::metaData.metaDataBits[i]);
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
	for (int i = 0; i  < DkSettings::scamDataDesc.size(); i++) 
		sDescription << qApp->translate("nmc::DkMetaData", DkSettings::scamDataDesc.at(i).toLatin1());

	for (int i = 0; i  < DkSettings::sdescriptionDesc.size(); i++) 
		sDescription << qApp->translate("nmc::DkMetaData", DkSettings::sdescriptionDesc.at(i).toLatin1());


	//QStringList sDescription = qApp->translate("nmc::DkMetaData",scamDataDesc.toAscii()).split(";") + qApp->translate("nmc::DkMetaData",sdescriptionDesc.toAscii()).split(";");

	for (int i=0; i<DkSettings::desc_end;i++) {
		pCbMetaData.append(new QCheckBox(sDescription.at(i), gbDescription));
	}

	for(int i=0; i<DkSettings::camData_end;i++) {
		camDataLayout->addWidget(pCbMetaData[i]);
	}
	camDataLayout->addStretch();

	for(int i=DkSettings::camData_end; i<DkSettings::desc_end;i++) {
		descriptionLayout->addWidget(pCbMetaData[i]);
	}

	descriptionLayout->addStretch();

	QGroupBox* gbOrientation = new QGroupBox(tr("Exif Orientation"), this);

	cbIgnoreOrientation = new QCheckBox(tr("Ignore Exif Orientation"), gbOrientation);
	cbIgnoreOrientation->setChecked(DkSettings::metaData.ignoreExifOrientation);
	cbIgnoreOrientation->setToolTip(tr("Note: instead of checking this option\n you should fix your images."));

	cbSaveOrientation = new QCheckBox(tr("Save Exif Orientation"), gbOrientation);
	cbSaveOrientation->setChecked(DkSettings::metaData.saveExifOrientation);
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

	for (int i=0; i<DkSettings::desc_end;i++) {
		DkSettings::metaData.metaDataBits[i] = pCbMetaData[i]->isChecked();
	}

	DkSettings::metaData.ignoreExifOrientation = cbIgnoreOrientation->isChecked();
	DkSettings::metaData.saveExifOrientation = cbSaveOrientation->isChecked();
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

	float curCache = (float)(DkSettings::resources.cacheMemory/totalMemory * stepSize * 100);

	connect(sliderMemory,SIGNAL(valueChanged(int)), this, SLOT(memorySliderChanged(int)));

	sliderMemory->setValue(qRound(curCache));
	this->memorySliderChanged(qRound(curCache));
	cbFilterRawImages->setChecked(DkSettings::resources.filterRawImages);
	cbRemoveDuplicates->setChecked(DkSettings::resources.filterDuplicats);

	rawThumbButtons[DkSettings::resources.loadRawThumb]->setChecked(true);
}

void DkResourceSettingsWidgets::createLayout() {
	QVBoxLayout* widgetVBoxLayout = new QVBoxLayout(this);

	QGroupBox* gbCache = new QGroupBox(tr("Cache Settings"));
	QGridLayout* cacheLayout = new QGridLayout(gbCache);
	QLabel* labelPercentage = new QLabel(tr("Percentage of memory which should be used for caching:"), gbCache);
	labelPercentage->setMinimumSize(labelPercentage->sizeHint());
	sliderMemory = new QSlider(Qt::Horizontal, gbCache);
	sliderMemory->setMinimum(0);
	sliderMemory->setMaximum(qRound(10*stepSize));
	sliderMemory->setPageStep(40);
	sliderMemory->setSingleStep(40);
	sliderMemory->setContentsMargins(11,11,11,0);

	// widget starts on hide
	QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);
	opacityEffect->setOpacity(0.7);
	setGraphicsEffect(opacityEffect);

	QWidget* memoryGradient = new QWidget(this);
	memoryGradient->setObjectName("memoryGradient");
	memoryGradient->setMinimumHeight(5);
	memoryGradient->setContentsMargins(0,0,0,0);
	memoryGradient->setWindowOpacity(0.3);
	memoryGradient->setGraphicsEffect(opacityEffect);	

	QWidget* captionWidget = new QWidget(this);
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

	labelMemory = new QLabel(this);
	labelMemory->setContentsMargins(10,-5,0,0);
	labelMemory->setAlignment(Qt::AlignCenter);

	cacheLayout->addWidget(labelPercentage,0,0);
	cacheLayout->addWidget(sliderMemory,1,0);
	cacheLayout->addWidget(labelMemory,1,1);
	cacheLayout->addWidget(memoryGradient,2,0);
	cacheLayout->addWidget(captionWidget,3,0);

	QGroupBox* gbRawLoader = new QGroupBox(tr("Raw Loader Settings"));

	rawThumbButtonGroup = new QButtonGroup(this);

	rawThumbButtons.resize(DkSettings::raw_thumb_end);

	rawThumbButtons[DkSettings::raw_thumb_always] = new QRadioButton(tr("Always load JPG if embedded"), this);
	rawThumbButtons[DkSettings::raw_thumb_if_large] = new QRadioButton(tr("Load JPG if it fits the screen resolution"), this);
	rawThumbButtons[DkSettings::raw_thumb_never] = new QRadioButton(tr("Never load embedded JPG"), this);

	QWidget* rawThumbWidget = new QWidget(this);
	QVBoxLayout* rawThumbButtonLayout = new QVBoxLayout(rawThumbWidget);

	for (int idx = 0; idx < rawThumbButtons.size(); idx++) {
		rawThumbButtonGroup->addButton(rawThumbButtons[idx]);
		rawThumbButtonLayout->addWidget(rawThumbButtons[idx]);
	}

	QWidget* dupWidget = new QWidget(this);
	QHBoxLayout* hLayout = new QHBoxLayout(dupWidget);
	hLayout->setContentsMargins(0,0,0,0);

	cbRemoveDuplicates = new QCheckBox(tr("Hide Duplicates"));
	cbRemoveDuplicates->setChecked(DkSettings::resources.filterRawImages);
	cbRemoveDuplicates->setToolTip(tr("If checked, duplicated images are not shown (e.g. RAW+JPG"));

	QLabel* preferredLabel = new QLabel(tr("Preferred Extension: "));

	QString pExt = DkSettings::resources.preferredExtension;
	if (pExt.isEmpty()) pExt = "*.jpg";	// best default
	cmExtensions = new QComboBox();
	cmExtensions->addItems(DkSettings::app.fileFilters);
	cmExtensions->setCurrentIndex(DkSettings::app.fileFilters.indexOf(pExt));

	qDebug() << "preferred extension: " << pExt;

	hLayout->addWidget(cbRemoveDuplicates);
	hLayout->addWidget(preferredLabel);
	hLayout->addWidget(cmExtensions);
	hLayout->addStretch();

	QGridLayout* rawLoaderLayout = new QGridLayout(gbRawLoader);
	cbFilterRawImages = new QCheckBox(tr("filter raw images"));

	rawLoaderLayout->addWidget(rawThumbWidget);
	rawLoaderLayout->addWidget(dupWidget);
	rawLoaderLayout->addWidget(cbFilterRawImages);

	widgetVBoxLayout->addWidget(gbCache);
	widgetVBoxLayout->addWidget(gbRawLoader);
	widgetVBoxLayout->addStretch();
}

void DkResourceSettingsWidgets::writeSettings() {

	DkSettings::resources.cacheMemory = (float)((sliderMemory->value()/stepSize)/100.0 * totalMemory);
	DkSettings::resources.filterRawImages = cbFilterRawImages->isChecked();
	DkSettings::resources.filterDuplicats = cbRemoveDuplicates->isChecked();
	DkSettings::resources.preferredExtension = DkSettings::app.fileFilters.at(cmExtensions->currentIndex());

	for (int idx = 0; idx < rawThumbButtons.size(); idx++) {
		if (rawThumbButtons[idx]->isChecked()) {
			DkSettings::resources.loadRawThumb = idx;
			break;
		}
	}
}

void DkResourceSettingsWidgets::memorySliderChanged(int newValue) {
	labelMemory->setText(QString::number((double)(newValue/stepSize)/100.0*totalMemory,'f',0) + " MB / "+ QString::number(totalMemory,'f',0) + " MB");
}

// DkRemoteControlWidget --------------------------------------------------------------------
DkRemoteControlWidget::DkRemoteControlWidget(QWidget* parent) : DkSettingsWidget(parent) {
	showOnlyInAdvancedMode = true;

	createLayout();
	init();
}

void DkRemoteControlWidget::init() {
	QStringList clients = DkSettings::sync.recentSyncNames;
	clients << DkSettings::sync.syncWhiteList;
	clients.removeDuplicates();

	whiteListModel = new DkWhiteListViewModel(table);
	DkCheckBoxDelegate* cbDelegate = new DkCheckBoxDelegate();
	table->setItemDelegate(cbDelegate);

	QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);
	proxyModel->setSourceModel(whiteListModel);
	table->setSortingEnabled(true);

	for(int i = 0; i < clients.size();i++) {
		whiteListModel->addWhiteListEntry(DkSettings::sync.syncWhiteList.contains(clients[i]) != 0, clients[i], DkSettings::sync.recentLastSeen.value(clients[i],QDateTime::currentDateTime()).toDateTime());
	}
	table->setModel(proxyModel);
	table->resizeColumnsToContents();
	table->resizeRowsToContents();

	// default sorting by checkbox and name
	table->sortByColumn(1, Qt::AscendingOrder);
	table->sortByColumn(0, Qt::DescendingOrder);
}

void DkRemoteControlWidget::createLayout() {
	QVBoxLayout* vbox = new QVBoxLayout(this);

	table = new QTableView(this);
	table->setEditTriggers(QAbstractItemView::AllEditTriggers);
	table->verticalHeader()->setVisible(false);
	vbox->addWidget(table);
	vbox->addStretch();
}

void DkRemoteControlWidget::writeSettings() {
	DkSettings::sync.syncWhiteList = QStringList();
	QVector<bool> checked = whiteListModel->getCheckedVector();
	QVector<QString> names = whiteListModel->getNamesVector();

	for (int i=0; i < checked.size(); i++) {
		if(checked.at(i))
			DkSettings::sync.syncWhiteList << names.at(i);
	}
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
	spinBox = new QSpinBox(this);
	spinBox->setMaximum(spinBoxMax);
	spinBox->setMinimum(spinBoxMin);
	spinBox->setSingleStep(step);
	upperLabel = new QLabel(upperString);
	lowerLabel = new QLabel(lowerString);
	lowerWidget = new QWidget(this);

	vboxLayout = new QVBoxLayout(this) ;
	vboxLayout->setSpacing(0);
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

QSpinBox* DkSpinBoxWidget::getSpinBox() const { 
	return spinBox;
}

void DkSpinBoxWidget::setSpinBoxValue(int value) {
	spinBox->setValue(value);
}

int DkSpinBoxWidget::getSpinBoxValue() const {
	return spinBox->value();
}



// DkDoubleSpinBoxWiget --------------------------------------------------------------------
DkDoubleSpinBoxWidget::DkDoubleSpinBoxWidget(QWidget* parent) : QWidget(parent) {
	spinBox = new QDoubleSpinBox(this);
	lowerLabel = new QLabel(this);
	lowerWidget = new QWidget(this);
	vboxLayout = new QVBoxLayout(this);
	vboxLayout->setSpacing(0);
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
	vboxLayout->setSpacing(0);
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

QDoubleSpinBox* DkDoubleSpinBoxWidget::getSpinBox() const { 
	return spinBox;
}

void DkDoubleSpinBoxWidget::setSpinBoxValue(float value) {
	spinBox->setValue(value);
}

float DkDoubleSpinBoxWidget::getSpinBoxValue() const {
	return (float)spinBox->value();
}

// DkFileFilterSettings --------------------------------------------------------------------
DkFileFilterSettingWidget::DkFileFilterSettingWidget(QWidget* parent) : DkSettingsWidget(parent) {

	init();
}

void DkFileFilterSettingWidget::init() {

	saveSettings = false;
	createLayout();
}

void DkFileFilterSettingWidget::createLayout() {

	QStringList fileFilters = DkSettings::app.openFilters;

	model = new QStandardItemModel(this);
	for (int rIdx = 1; rIdx < fileFilters.size(); rIdx++)
		model->appendRow(getItems(fileFilters.at(rIdx), checkFilter(fileFilters.at(rIdx), DkSettings::app.browseFilters), checkFilter(fileFilters.at(rIdx), DkSettings::app.registerFilters)));
	
	model->setHeaderData(0, Qt::Horizontal, tr("Filter"));
	model->setHeaderData(1, Qt::Horizontal, tr("Browse"));
	model->setHeaderData(2, Qt::Horizontal, tr("Register"));

	connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(itemChanged(QStandardItem*)));

	filterTableView = new QTableView(this);
	filterTableView->setModel(model);
	filterTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	filterTableView->verticalHeader()->hide();
	//filterTableView->horizontalHeader()->hide();
	filterTableView->setShowGrid(false);
	filterTableView->resizeColumnsToContents();
	filterTableView->resizeRowsToContents();
	filterTableView->setWordWrap(false);
	
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(filterTableView);
	setLayout(layout);
	//show();
}

bool DkFileFilterSettingWidget::checkFilter(const QString& cFilter, const QStringList& filters) const {

	if (filters.empty() && (DkSettings::app.containerFilters.contains(cFilter) || cFilter.contains(".ico")))
		return false;

	if (filters.empty())
		return true;

	for (int idx = 0; idx < filters.size(); idx++)
		if (cFilter.contains(filters[idx]))
			return true;

	return filters.indexOf(cFilter) != -1;
}

QList<QStandardItem*> DkFileFilterSettingWidget::getItems(const QString& filter, bool browse, bool reg) {

	QList<QStandardItem* > items;
	QStandardItem* item = new QStandardItem(filter);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	items.append(item);
	item = new QStandardItem("");
	//item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
	item->setCheckable(true);
	item->setCheckState(browse ? Qt::Checked : Qt::Unchecked);
	items.append(item);
	item = new QStandardItem("");
	//item->setFlags(Qt::Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
	item->setCheckable(true);
	item->setCheckState(reg ? Qt::Checked : Qt::Unchecked);
	items.append(item);

	return items;

}

void DkFileFilterSettingWidget::itemChanged(QStandardItem*) {

	saveSettings = true;
}

void DkFileFilterSettingWidget::writeSettings() {

	if (!saveSettings)
		return;

	DkFileFilterHandling fh;
	DkSettings::app.browseFilters.clear();
	DkSettings::app.registerFilters.clear();
	
	for (int idx = 0; idx < model->rowCount(); idx++) {

		QStandardItem* item = model->item(idx, 0);

		if (!item)
			continue;

		QStandardItem* browseItem = model->item(idx, 1);
		QStandardItem* regItem = model->item(idx, 2);

		if (browseItem && browseItem->checkState() == Qt::Checked) {
			
			QString cFilter = item->text();
			cFilter = cFilter.section(QRegExp("(\\(|\\))"), 1);
			cFilter = cFilter.replace(")", "");

			DkSettings::app.browseFilters += cFilter.split(" ");
		}
		
		fh.registerFileType(item->text(), tr("Image"), regItem->checkState() == Qt::Checked);
		
		if (regItem->checkState() == Qt::Checked) {
			DkSettings::app.registerFilters.append(item->text());
			qDebug() << item->text() << " registered";
		}
		else
			qDebug() << item->text() << " unregistered";
	}

}

// DkWhiteListViewModel --------------------------------------------------------------------
DkWhiteListViewModel::DkWhiteListViewModel(QObject* parent) : QAbstractTableModel(parent) {
}


QVariant DkWhiteListViewModel::headerData(int section, Qt::Orientation orientation, int role /* = Qt::DisplayRole */) const {

	if (orientation == Qt::Vertical ||role != Qt::DisplayRole)
		return QAbstractTableModel::headerData(section, orientation, role);

	if (section == 0)
		return QVariant();
	//return QString(tr("Connect"));
	else if (section == 1)
		return QString(tr("Name"));
	else if (section == 2)
		return QString(tr("Last Connected"));
	else
		return QString("That's too much of information");

}

bool DkWhiteListViewModel::setHeaderData(int, Qt::Orientation, const QVariant&, int) {

	return false;
}

QVariant DkWhiteListViewModel::data(const QModelIndex & index, int role /* = Qt::DisplayRole */) const {
	if (!index.isValid()) {
		qDebug() << "invalid row: " << index.row();
		return QVariant();
	}

	if (role == Qt::DisplayRole) {

		if (index.column() == 0)
			return checked.at(index.row()) ? Qt::Checked : Qt::Unchecked;
		else if (index.column() == 1)
			return names.at(index.row());
		else if (index.column() == 2)
			return lastSeen.at(index.row());
		else
			return QVariant();
	}
	else if (role == Qt::CheckStateRole && index.column() == 0) {
		return checked.at(index.row()) ? Qt::Checked : Qt::Unchecked;
	}

	return QVariant();
}

bool DkWhiteListViewModel::setData(const QModelIndex &index, const QVariant &value, int) {
	qDebug() << __FUNCTION__;
	if (!index.isValid()) {
		qDebug() << "invalid row: " << index.row();
		return false;
	}

	if (index.column() == 0) {
		checked[index.row()] = value.toBool();
	}
	return false;
}

Qt::ItemFlags DkWhiteListViewModel::flags(const QModelIndex & index) const {
	if(!index.isValid())
		return Qt::ItemIsEnabled;

	if (index.column()==0) {
		return Qt::ItemIsEnabled | Qt::ItemIsEditable;
	}
	else
		return QAbstractTableModel::flags(index);
}

int DkWhiteListViewModel::rowCount(const QModelIndex& parent /* = QModelIndex */) const {
	if (parent.isValid())
		return 0;
	return names.size();
}


void DkWhiteListViewModel::addWhiteListEntry(bool checked, QString name, QDateTime lastSeen) {
	this->checked.push_back(checked);
	this->names.push_back(name);
	this->lastSeen.push_back(lastSeen);

	dataChanged(createIndex(this->checked.size()-1, 0, &this->checked[this->checked.size()-1]), createIndex(this->checked.size()-1, 2, &this->checked[this->checked.size()-1]));
}

// DkCheckBoxDelegate --------------------------------------------------------------------

QWidget *DkCheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem&, const QModelIndex&) const {
	QCheckBox* editor = new QCheckBox(parent);
	connect(editor, SIGNAL(stateChanged(int)), this, SLOT(cbChanged(int)));
	return editor;

}

void DkCheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
	bool value = index.data().toBool();

	QCheckBox* cb = static_cast<QCheckBox*>(editor);
	cb->setChecked(!value); 
}

void DkCheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
	QCheckBox* cb = static_cast<QCheckBox*>(editor);
	model->setData(index, cb->isChecked());
}

void DkCheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const {
	qDebug() << __FUNCTION__;
	QRect rect = option.rect;
	rect.setLeft(option.rect.left()+10);
	editor->setGeometry(rect);
}

void DkCheckBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	if (index.column()==0) {
		painter->save();

		QStyle* style = QApplication::style();
		QStyleOptionButton cbOpt;
		cbOpt.rect = option.rect;
		cbOpt.rect.setLeft(cbOpt.rect.left()+10);
		cbOpt.state = QStyle::State_Enabled;
		cbOpt.state |= index.data().toBool() ? QStyle::State_On : QStyle::State_Off;
		style->drawControl(QStyle::CE_CheckBox, &cbOpt, painter);
		painter->restore();
	}
	else
		QStyledItemDelegate::paint(painter, option, index);
}

void DkCheckBoxDelegate::cbChanged(int) {
	QCheckBox* cb = qobject_cast<QCheckBox*>(sender());
	emit commitData(cb);
	emit closeEditor(cb);
}

}