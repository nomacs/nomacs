/*******************************************************************************************************
DkPreferenceWidgets.cpp
Created on:	14.12.2015
 
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

#include "DkPreferenceWidgets.h"

#include "DkImageStorage.h"
#include "DkWidgets.h"
#include "DkSettings.h"
#include "DkUtils.h"
#include "DkBasicWidgets.h"
#include "DkSettingsWidget.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QStyleOption>
#include <QPainter>
#include <QAction>
#include <QFileInfo>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTableView>
#include <QHeaderView>
#include <QStandardPaths>
#include <QProcess>
#include <QFileDialog>
#include <QStandardPaths>

#include <QButtonGroup>
#include <QRadioButton>

#include <QDebug>
#pragma warning(pop)

namespace nmc {

DkPreferenceWidget::DkPreferenceWidget(QWidget* parent) : DkWidget(parent) {

	createLayout();

	QAction* nextAction = new QAction(tr("next"), this);
	nextAction->setShortcut(Qt::Key_PageDown);
	connect(nextAction, SIGNAL(triggered()), this, SLOT(nextTab()));
	addAction(nextAction);

	QAction* previousAction = new QAction(tr("previous"), this);
	previousAction->setShortcut(Qt::Key_PageUp);
	previousAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(previousAction, SIGNAL(triggered()), this, SLOT(previousTab()));
	addAction(previousAction);
}

void DkPreferenceWidget::createLayout() {

	// create tab entries
	QWidget* tabs = new QWidget(this);
	tabs->setObjectName("DkPreferenceTabs");

	QPixmap pm = DkImage::colorizePixmap(QIcon(":/nomacs/img/power.svg").pixmap(QSize(32, 32)), QColor(255, 255, 255));
	QPushButton* restartButton = new QPushButton(pm, "", this);
	restartButton->setObjectName("DkPlayerButton");
	restartButton->setFlat(true);
	restartButton->setIconSize(QSize(32, 32));
	restartButton->setObjectName("DkRestartButton");
	restartButton->setStatusTip(tr("Restart nomacs"));
	connect(restartButton, SIGNAL(clicked()), this, SIGNAL(restartSignal()));

	mTabLayout = new QVBoxLayout(tabs);
	mTabLayout->setContentsMargins(0, 60, 0, 0);
	mTabLayout->setSpacing(0);
	mTabLayout->setAlignment(Qt::AlignTop);
	mTabLayout->addStretch();
	mTabLayout->addWidget(restartButton);

	// create central widget
	QWidget* centralWidget = new QWidget(this);
	mCentralLayout = new QStackedLayout(centralWidget);
	mCentralLayout->setContentsMargins(0, 0, 0, 0);

	QWidget* dummy = new QWidget(this);
	QHBoxLayout* layout = new QHBoxLayout(dummy);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->setAlignment(Qt::AlignLeft);
	layout->addWidget(tabs);
	layout->addWidget(centralWidget);

	// add a scroll area
	DkResizableScrollArea* scrollArea = new DkResizableScrollArea(this);
	scrollArea->setObjectName("DkScrollAreaPlots");
	scrollArea->setWidgetResizable(true);
	scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	scrollArea->setWidget(dummy);

	QVBoxLayout* sL = new QVBoxLayout(this);
	sL->setContentsMargins(1, 1, 1, 1);	// 1 to get the border
	sL->addWidget(scrollArea);
}

void DkPreferenceWidget::addTabWidget(DkPreferenceTabWidget* tabWidget) {

	mWidgets.append(tabWidget);
	mCentralLayout->addWidget(tabWidget);

	DkTabEntryWidget* tabEntry = new DkTabEntryWidget(tabWidget->icon(), tabWidget->name(), this);
	mTabLayout->insertWidget(mTabLayout->count()-2, tabEntry);	// -2 -> insert before stretch
	connect(tabEntry, SIGNAL(clicked()), this, SLOT(changeTab()));
	connect(tabWidget, SIGNAL(restartSignal()), this, SIGNAL(restartSignal()));
	mTabEntries.append(tabEntry);

	// tick the first
	if (mTabEntries.size() == 1)
		tabEntry->click();
}

void DkPreferenceWidget::previousTab() {

	// modulo is sign aware in cpp...
	int idx = (mCurrentIndex == 0) ? mWidgets.size()-1 : mCurrentIndex-1;
	setCurrentIndex(idx);
}

void DkPreferenceWidget::nextTab() {
	setCurrentIndex((mCurrentIndex+1) % mWidgets.size());
}

void DkPreferenceWidget::changeTab() {

	DkTabEntryWidget* te = qobject_cast<DkTabEntryWidget*>(sender());

	for (int idx = 0; idx < mTabEntries.size(); idx++) {

		if (te == mTabEntries[idx]) {
			setCurrentIndex(idx);
		}
	}
}

void DkPreferenceWidget::setCurrentIndex(int index) {

	// something todo here?
	if (index == mCurrentIndex)
		return;

	mCurrentIndex = index;
	mCentralLayout->setCurrentIndex(index);

	// now check the correct one
	for (int idx = 0; idx < mTabEntries.size(); idx++)
		mTabEntries[idx]->setChecked(idx == index);

}

// DkPreferenceTabWidget --------------------------------------------------------------------
DkPreferenceTabWidget::DkPreferenceTabWidget(const QIcon& icon, const QString& name, QWidget* parent) : DkNamedWidget(name, parent) {

	setObjectName("DkPreferenceTab");
	mIcon = icon;

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkPreferenceTabWidget::createLayout() {

	QLabel* titleLabel = new QLabel(name(), this);
	titleLabel->setObjectName("DkPreferenceTitle");

	mInfoButton = new QPushButton(tr(""), this);
	mInfoButton->setObjectName("infoButton");
	mInfoButton->setFlat(true);
	connect(mInfoButton, SIGNAL(clicked()), this, SIGNAL(restartSignal()));

	mLayout = new QGridLayout(this);
	mLayout->setContentsMargins(0, 0, 0, 0);
	mLayout->setAlignment(Qt::AlignTop);
	mLayout->addWidget(titleLabel, 0, 0);
	
	mLayout->addWidget(mInfoButton, 2, 0, Qt::AlignBottom);
}

void DkPreferenceTabWidget::setWidget(QWidget* w) {

	mCentralWidget = w;
	mCentralWidget->setObjectName("DkPreferenceWidget");
	mLayout->addWidget(mCentralWidget, 1, 0);

	connect(w, SIGNAL(infoSignal(const QString&)), this, SLOT(setInfoMessage(const QString&)));
}

void DkPreferenceTabWidget::setInfoMessage(const QString& msg) {
	mInfoButton->setText(msg);
}

QWidget* DkPreferenceTabWidget::widget() const {
	return mCentralWidget;
}

QIcon DkPreferenceTabWidget::icon() const {
	return mIcon;
}

// DkTabEntryWidget --------------------------------------------------------------------
DkTabEntryWidget::DkTabEntryWidget(const QIcon& icon, const QString& text, QWidget* parent) : QPushButton(text, parent) {

	setObjectName("DkTabEntryWidget");

	QSize s(DkSettingsManager::param().effectiveIconSize(parent), DkSettingsManager::param().effectiveIconSize(parent));
	QPixmap pm = DkImage::colorizePixmap(icon.pixmap(s), QColor(255, 255, 255));
	setIcon(pm);
	setIconSize(s);

	setFlat(true);
	setCheckable(true);
}

void DkTabEntryWidget::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QPushButton::paintEvent(event);
}

// DkGroupWidget --------------------------------------------------------------------
DkGroupWidget::DkGroupWidget(const QString& title, QWidget* parent) : QWidget(parent) {

	setObjectName("DkGroupWidget");
	mTitle = title;

	createLayout();
}

void DkGroupWidget::createLayout() {

	QLabel* titleLabel = new QLabel(mTitle, this);
	titleLabel->setObjectName("subTitle");

	// we create a content widget to have control over the margins
	QWidget* contentWidget = new QWidget(this);
	mContentLayout = new QVBoxLayout(contentWidget);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(titleLabel);
	layout->addWidget(contentWidget);
}

void DkGroupWidget::addWidget(QWidget* widget) {
	
	mContentLayout->addWidget(widget);
}

void DkGroupWidget::addSpace() {

	mContentLayout->addSpacing(10);
}

void DkGroupWidget::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

// DkGeneralPreference --------------------------------------------------------------------
DkGeneralPreference::DkGeneralPreference(QWidget* parent) : QWidget(parent) {

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkGeneralPreference::createLayout() {

	// color settings
	DkColorChooser* highlightColorChooser = new DkColorChooser(QColor(0, 204, 255), tr("Highlight Color"), this);
	highlightColorChooser->setObjectName("highlightColor");
	highlightColorChooser->setColor(&DkSettingsManager::param().display().highlightColor);
	connect(highlightColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* iconColorChooser = new DkColorChooser(QColor(219, 89, 2, 255), tr("Icon Color"), this);
	iconColorChooser->setObjectName("iconColor");
	iconColorChooser->setColor(&DkSettingsManager::param().display().iconColor);
	connect(iconColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* bgColorChooser = new DkColorChooser(QColor(100, 100, 100, 255), tr("Background Color"), this);
	bgColorChooser->setObjectName("backgroundColor");
	bgColorChooser->setColor(&DkSettingsManager::param().display().bgColor);
	connect(bgColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* fullscreenColorChooser = new DkColorChooser(QColor(86,86,90), tr("Fullscreen Color"), this);
	fullscreenColorChooser->setObjectName("fullscreenColor");
	fullscreenColorChooser->setColor(&DkSettingsManager::param().slideShow().backgroundColor);
	connect(fullscreenColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* fgdHUDColorChooser = new DkColorChooser(QColor(255, 255, 255, 255), tr("HUD Foreground Color"), this);
	fgdHUDColorChooser->setObjectName("fgdHUDColor");
	fgdHUDColorChooser->setColor(&DkSettingsManager::param().display().hudFgdColor);
	connect(fgdHUDColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* bgHUDColorChooser = new DkColorChooser(QColor(0, 0, 0, 100), tr("HUD Background Color"), this);
	bgHUDColorChooser->setObjectName("bgHUDColor");
	bgHUDColorChooser->setColor((DkSettingsManager::param().app().appMode == DkSettings::mode_frameless) ?
		&DkSettingsManager::param().display().bgColorFrameless : &DkSettingsManager::param().display().hudBgColor);
	connect(bgHUDColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkGroupWidget* colorGroup = new DkGroupWidget(tr("Color Settings"), this);
	colorGroup->addWidget(highlightColorChooser);
	colorGroup->addWidget(iconColorChooser);
	colorGroup->addWidget(bgColorChooser);
	colorGroup->addWidget(fullscreenColorChooser);
	colorGroup->addWidget(fgdHUDColorChooser);
	colorGroup->addWidget(bgHUDColorChooser);

	// default pushbutton
	QPushButton* defaultSettings = new QPushButton(tr("Reset All Settings"));
	defaultSettings->setObjectName("defaultSettings");
	defaultSettings->setMaximumWidth(300);

	QPushButton* importSettings = new QPushButton(tr("&Import Settings"));
	importSettings->setObjectName("importSettings");
	importSettings->setMaximumWidth(300);

	QPushButton* exportSettings = new QPushButton(tr("&Export Settings"));
	exportSettings->setObjectName("exportSettings");
	exportSettings->setMaximumWidth(300);

	DkGroupWidget* defaultGroup = new DkGroupWidget(tr("Default Settings"), this);
	defaultGroup->addWidget(defaultSettings);
	defaultGroup->addWidget(importSettings);
	defaultGroup->addWidget(exportSettings);

	// the left column (holding all color settings)
	QWidget* leftColumn = new QWidget(this);
	leftColumn->setMinimumWidth(400);

	QVBoxLayout* leftColumnLayout = new QVBoxLayout(leftColumn);
	leftColumnLayout->setAlignment(Qt::AlignTop);
	leftColumnLayout->addWidget(colorGroup);
	leftColumnLayout->addWidget(defaultGroup);

	// checkboxes
	QCheckBox* cbRecentFiles = new QCheckBox(tr("Show Recent Files on Start-Up"), this);
	cbRecentFiles->setObjectName("showRecentFiles");
	cbRecentFiles->setToolTip(tr("Show the History Panel on Start-Up"));
	cbRecentFiles->setChecked(DkSettingsManager::param().app().showRecentFiles);

	QCheckBox* cbLogRecentFiles = new QCheckBox(tr("Log Recent Files"), this);
	cbLogRecentFiles->setObjectName("logRecentFiles");
	cbLogRecentFiles->setToolTip(tr("If checked, recent files will be saved."));
	cbLogRecentFiles->setChecked(DkSettingsManager::param().global().logRecentFiles);

	QCheckBox* cbLoopImages = new QCheckBox(tr("Loop Images"), this);
	cbLoopImages->setObjectName("loopImages");
	cbLoopImages->setToolTip(tr("Start with the first image in a folder after showing the last."));
	cbLoopImages->setChecked(DkSettingsManager::param().global().loop);

	QCheckBox* cbZoomOnWheel = new QCheckBox(tr("Mouse Wheel Zooms"), this);
	cbZoomOnWheel->setObjectName("zoomOnWheel");
	cbZoomOnWheel->setToolTip(tr("If checked, the mouse wheel zooms - otherwise it is used to switch between images."));
	cbZoomOnWheel->setChecked(DkSettingsManager::param().global().zoomOnWheel);

	QCheckBox* cbHorZoomSkips = new QCheckBox(tr("Next Image on Horizontal Zoom"), this);
	cbHorZoomSkips->setObjectName("horZoomSkips");
	cbHorZoomSkips->setToolTip(tr("If checked, horizontal wheel events load the next/previous images."));
	cbHorZoomSkips->setChecked(DkSettingsManager::param().global().horZoomSkips);

	QCheckBox* cbDoubleClickForFullscreen = new QCheckBox(tr("Double Click Opens Fullscreen"), this);
	cbDoubleClickForFullscreen->setObjectName("doubleClickForFullscreen");
	cbDoubleClickForFullscreen->setToolTip(tr("If checked, a double click on the canvas opens the fullscreen mode."));
	cbDoubleClickForFullscreen->setChecked(DkSettingsManager::param().global().doubleClickForFullscreen);

	QCheckBox* cbShowBgImage = new QCheckBox(tr("Show Background Image"), this);
	cbShowBgImage->setObjectName("showBgImage");
	cbShowBgImage->setToolTip(tr("If checked, the nomacs logo is shown in the bottom right corner."));
	cbShowBgImage->setChecked(DkSettingsManager::param().global().showBgImage);

	QCheckBox* cbSwitchModifier = new QCheckBox(tr("Switch CTRL with ALT"), this);
	cbSwitchModifier->setObjectName("switchModifier");
	cbSwitchModifier->setToolTip(tr("If checked, CTRL + Mouse is switched with ALT + Mouse."));
	cbSwitchModifier->setChecked(DkSettingsManager::param().sync().switchModifier);

	QCheckBox* cbEnableNetworkSync = new QCheckBox(tr("Enable LAN Sync"), this);
	cbEnableNetworkSync->setObjectName("networkSync");
	cbEnableNetworkSync->setToolTip(tr("If checked, syncing in your LAN is enabled."));
	cbEnableNetworkSync->setChecked(DkSettingsManager::param().sync().enableNetworkSync);

	QCheckBox* cbCloseOnEsc = new QCheckBox(tr("Close on ESC"), this);
	cbCloseOnEsc->setObjectName("closeOnEsc");
	cbCloseOnEsc->setToolTip(tr("Close nomacs if ESC is pressed."));
	cbCloseOnEsc->setChecked(DkSettingsManager::param().app().closeOnEsc);

	QCheckBox* cbCheckForUpdates = new QCheckBox(tr("Check For Updates"), this);
	cbCheckForUpdates->setObjectName("checkForUpdates");
	cbCheckForUpdates->setToolTip(tr("Check for updates on start-up."));
	cbCheckForUpdates->setChecked(DkSettingsManager::param().sync().checkForUpdates);

	DkGroupWidget* generalGroup = new DkGroupWidget(tr("General"), this);
	generalGroup->addWidget(cbRecentFiles);
	generalGroup->addWidget(cbLogRecentFiles);
	generalGroup->addWidget(cbLoopImages);
	generalGroup->addWidget(cbZoomOnWheel);
	generalGroup->addWidget(cbHorZoomSkips);
	generalGroup->addWidget(cbDoubleClickForFullscreen);
	generalGroup->addWidget(cbSwitchModifier);
	generalGroup->addWidget(cbEnableNetworkSync);
	generalGroup->addWidget(cbCloseOnEsc);
	generalGroup->addWidget(cbCheckForUpdates);
	generalGroup->addWidget(cbShowBgImage);

	// language
	QComboBox* languageCombo = new QComboBox(this);
	languageCombo->setObjectName("languageCombo");
	languageCombo->setToolTip(tr("Choose your preferred language."));
	DkUtils::addLanguages(languageCombo, mLanguages);
	languageCombo->setCurrentIndex(mLanguages.indexOf(DkSettingsManager::param().global().language));

	QLabel* translateLabel = new QLabel("<a href=\"http://www.nomacs.org/how-to-translate-nomacs/\">How-to translate nomacs</a>", this);
	translateLabel->setToolTip(tr("Info on how to translate nomacs."));
	translateLabel->setOpenExternalLinks(true);

	DkGroupWidget* languageGroup = new DkGroupWidget(tr("Language"), this);
	languageGroup->addWidget(languageCombo);
	languageGroup->addWidget(translateLabel);

	// the right column (holding all checkboxes)
	QWidget* cbWidget = new QWidget(this);
	QVBoxLayout* cbLayout = new QVBoxLayout(cbWidget);
	cbLayout->setAlignment(Qt::AlignTop);
	cbLayout->addWidget(generalGroup);

	// add language
	cbLayout->addWidget(languageGroup);

	QHBoxLayout* contentLayout = new QHBoxLayout(this);
	contentLayout->setAlignment(Qt::AlignLeft);
	contentLayout->addWidget(leftColumn);
	contentLayout->addWidget(cbWidget);

}

void DkGeneralPreference::showRestartLabel() const {
	emit infoSignal(tr("Please Restart nomacs to apply changes"));
}

void DkGeneralPreference::on_backgroundColor_accepted() const {
	DkSettingsManager::param().display().defaultBackgroundColor = false;
}

void DkGeneralPreference::on_backgroundColor_resetClicked() const {
	DkSettingsManager::param().display().defaultBackgroundColor = true;
}

void DkGeneralPreference::on_iconColor_accepted() const {
	DkSettingsManager::param().display().defaultIconColor = false;
}

void DkGeneralPreference::on_iconColor_resetClicked() const {
	DkSettingsManager::param().display().defaultIconColor = true;
}

void DkGeneralPreference::on_showRecentFiles_toggled(bool checked) const {

	if (DkSettingsManager::param().app().showRecentFiles != checked)
		DkSettingsManager::param().app().showRecentFiles = checked;
}

void DkGeneralPreference::on_logRecentFiles_toggled(bool checked) const {

	if (DkSettingsManager::param().global().logRecentFiles != checked)
		DkSettingsManager::param().global().logRecentFiles = checked;
}

void DkGeneralPreference::on_closeOnEsc_toggled(bool checked) const {

	if (DkSettingsManager::param().app().closeOnEsc != checked)
		DkSettingsManager::param().app().closeOnEsc = checked;
}

void DkGeneralPreference::on_zoomOnWheel_toggled(bool checked) const {

	if (DkSettingsManager::param().global().zoomOnWheel != checked) {
		DkSettingsManager::param().global().zoomOnWheel = checked;
	}
}

void DkGeneralPreference::on_horZoomSkips_toggled(bool checked) const {

	if (DkSettingsManager::param().global().horZoomSkips != checked) {
		DkSettingsManager::param().global().horZoomSkips = checked;
	}
}

void DkGeneralPreference::on_doubleClickForFullscreen_toggled(bool checked) const {

	if (DkSettingsManager::param().global().doubleClickForFullscreen != checked)
		DkSettingsManager::param().global().doubleClickForFullscreen = checked;

}

void DkGeneralPreference::on_showBgImage_toggled(bool checked) const {

	if (DkSettingsManager::param().global().showBgImage != checked) {
		DkSettingsManager::param().global().showBgImage = checked;
		showRestartLabel();
	}

}

void DkGeneralPreference::on_checkForUpdates_toggled(bool checked) const {

	if (DkSettingsManager::param().sync().checkForUpdates != checked)
		DkSettingsManager::param().sync().checkForUpdates = checked;
}

void DkGeneralPreference::on_switchModifier_toggled(bool checked) const {

	if (DkSettingsManager::param().sync().switchModifier != checked) {

		DkSettingsManager::param().sync().switchModifier = checked;

		if (DkSettingsManager::param().sync().switchModifier) {
			DkSettingsManager::param().global().altMod = Qt::ControlModifier;
			DkSettingsManager::param().global().ctrlMod = Qt::AltModifier;
		}
		else {
			DkSettingsManager::param().global().altMod = Qt::AltModifier;
			DkSettingsManager::param().global().ctrlMod = Qt::ControlModifier;
		}
	}
}

void DkGeneralPreference::on_loopImages_toggled(bool checked) const {

	if (DkSettingsManager::param().global().loop != checked)
		DkSettingsManager::param().global().loop = checked;
}

void DkGeneralPreference::on_networkSync_toggled(bool checked) const {

	if (DkSettingsManager::param().sync().enableNetworkSync != checked)
		DkSettingsManager::param().sync().enableNetworkSync = checked;
}

void DkGeneralPreference::on_defaultSettings_clicked() {

	int answer = QMessageBox::warning(this, tr("Reset All Settings"), tr("This will reset all personal settings!"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	
	if (answer == QMessageBox::Yes) {
		DkSettingsManager::param().setToDefaultSettings();
		emit infoSignal(tr("Please Restart nomacs to apply changes"));
		qDebug() << "answer is: " << answer << "flushing all settings...";
	}

}

void DkGeneralPreference::on_importSettings_clicked() {

	QString filePath = QFileDialog::getOpenFileName(
		DkUtils::getMainWindow(), 
		tr("Import Settings"),
		QDir::homePath(), 
		"Nomacs Settings (*.nfo)");

	// user canceled?
	if (filePath.isEmpty())
		return;

	DkSettingsManager::importSettings(filePath);

	emit infoSignal(tr("Please Restart nomacs to apply changes"));
}

void DkGeneralPreference::on_exportSettings_clicked() {

	QString filePath = QFileDialog::getSaveFileName(
		DkUtils::getMainWindow(), 
		tr("Export Settings"),
		QDir::homePath(), 
		"Nomacs Settings (*.nfo)");

	// user canceled?
	if (filePath.isEmpty())
		return;

	QSettings settings(filePath, QSettings::IniFormat);
	DkSettingsManager::instance().settings().save(settings, true);

	emit infoSignal(tr("Settings exported"));
}

void DkGeneralPreference::on_languageCombo_currentIndexChanged(int index) const {

	if (index >= 0 && index < mLanguages.size()) {
		QString language = mLanguages[index];

		if (DkSettingsManager::param().global().language != language) {
			DkSettingsManager::param().global().language = language;
			emit infoSignal(tr("Please Restart nomacs to apply changes"));
		}
	}
}

void DkGeneralPreference::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

// DkDisplaySettings --------------------------------------------------------------------
DkDisplayPreference::DkDisplayPreference(QWidget* parent) : QWidget(parent) {

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkDisplayPreference::createLayout() {

	// zoom settings
	QCheckBox* invertZoom = new QCheckBox(tr("Invert mouse wheel behaviour for zooming"), this);
	invertZoom->setObjectName("invertZoom");
	invertZoom->setToolTip(tr("If checked, the mouse wheel behaviour is inverted while zooming."));
	invertZoom->setChecked(DkSettingsManager::param().display().invertZoom);

	QLabel* interpolationLabel = new QLabel(tr("Show pixels if zoom level is above"), this);

	QSpinBox* sbInterpolation = new QSpinBox(this);
	sbInterpolation->setObjectName("interpolationBox");
	sbInterpolation->setToolTip(tr("nomacs will not interpolate images if the zoom level is larger."));
	sbInterpolation->setSuffix("%");
	sbInterpolation->setMinimum(0);
	sbInterpolation->setMaximum(10000);
	sbInterpolation->setValue(DkSettingsManager::param().display().interpolateZoomLevel);

	DkGroupWidget* zoomGroup = new DkGroupWidget(tr("Zoom"), this);
	zoomGroup->addWidget(invertZoom);
	zoomGroup->addWidget(interpolationLabel);
	zoomGroup->addWidget(sbInterpolation);

	// keep zoom radio buttons
	QVector<QRadioButton*> keepZoomButtons;
	keepZoomButtons.resize(DkSettings::zoom_end);
	keepZoomButtons[DkSettings::zoom_always_keep] = new QRadioButton(tr("Always keep zoom"), this);
	keepZoomButtons[DkSettings::zoom_keep_same_size] = new QRadioButton(tr("Keep zoom if the size is the same"), this);
	keepZoomButtons[DkSettings::zoom_keep_same_size]->setToolTip(tr("If checked, the zoom level is only kept, if the image loaded has the same level as the previous."));
	keepZoomButtons[DkSettings::zoom_never_keep] = new QRadioButton(tr("Never keep zoom"), this);

	QCheckBox* cbZoomToFit = new QCheckBox(tr("Always zoom to fit"), this);
	cbZoomToFit->setObjectName("zoomToFit");
	cbZoomToFit->setChecked(DkSettingsManager::param().display().zoomToFit);

	// check wrt the current settings
	keepZoomButtons[DkSettingsManager::param().display().keepZoom]->setChecked(true);

	QButtonGroup* keepZoomButtonGroup = new QButtonGroup(this);
	keepZoomButtonGroup->setObjectName("keepZoom");
	keepZoomButtonGroup->addButton(keepZoomButtons[DkSettings::zoom_always_keep], DkSettings::zoom_always_keep);
	keepZoomButtonGroup->addButton(keepZoomButtons[DkSettings::zoom_keep_same_size], DkSettings::zoom_keep_same_size);
	keepZoomButtonGroup->addButton(keepZoomButtons[DkSettings::zoom_never_keep], DkSettings::zoom_never_keep);

	DkGroupWidget* keepZoomGroup = new DkGroupWidget(tr("When Displaying New Images"), this);
	keepZoomGroup->addWidget(keepZoomButtons[DkSettings::zoom_always_keep]);
	keepZoomGroup->addWidget(keepZoomButtons[DkSettings::zoom_keep_same_size]);
	keepZoomGroup->addWidget(keepZoomButtons[DkSettings::zoom_never_keep]);
	keepZoomGroup->addWidget(cbZoomToFit);
	
	// icon size
	QSpinBox* sbIconSize = new QSpinBox(this);
	sbIconSize->setObjectName("iconSizeBox");
	sbIconSize->setToolTip(tr("Define the icon size in pixel."));
	sbIconSize->setSuffix(" px");
	sbIconSize->setMinimum(16);
	sbIconSize->setMaximum(1024);
	sbIconSize->setValue(DkSettingsManager::param().effectiveIconSize(sbIconSize));

	DkGroupWidget* iconGroup = new DkGroupWidget(tr("Icon Size"), this);
	iconGroup->addWidget(sbIconSize);

	// slideshow
	QLabel* fadeImageLabel = new QLabel(tr("Image Transition"), this);

	QComboBox* cbTransition = new QComboBox(this);
	cbTransition->setObjectName("transition");
	cbTransition->setToolTip(tr("Choose a transition when loading a new image"));

	for (int idx = 0; idx < DkSettings::trans_end; idx++) {

		QString str = tr("Unknown Transition");

		switch (idx) {
		case DkSettings::trans_appear:	str = tr("Appear"); break;
		case DkSettings::trans_swipe:	str = tr("Swipe");	break;
		case DkSettings::trans_fade:	str = tr("Fade");	break;
		}

		cbTransition->addItem(str);
	}
	cbTransition->setCurrentIndex(DkSettingsManager::param().display().transition);

	QDoubleSpinBox* fadeImageBox = new QDoubleSpinBox(this);
	fadeImageBox->setObjectName("fadeImageBox");
	fadeImageBox->setToolTip(tr("Define the image transition speed."));
	fadeImageBox->setSuffix(" sec");
	fadeImageBox->setMinimum(0.0);
	fadeImageBox->setMaximum(3);
	fadeImageBox->setSingleStep(.2);
	fadeImageBox->setValue(DkSettingsManager::param().display().animationDuration);

	QCheckBox* cbAlwaysAnimate = new QCheckBox(tr("Always Animate Image Loading"), this);
	cbAlwaysAnimate->setObjectName("alwaysAnimate");
	cbAlwaysAnimate->setToolTip(tr("If unchecked, loading is only animated if nomacs is fullscreen"));
	cbAlwaysAnimate->setChecked(DkSettingsManager::param().display().alwaysAnimate);

	QLabel* displayTimeLabel = new QLabel(tr("Display Time"), this);
	
	QDoubleSpinBox* displayTimeBox = new QDoubleSpinBox(this);
	displayTimeBox->setObjectName("displayTimeBox");
	displayTimeBox->setToolTip(tr("Define the time an image is displayed."));
	displayTimeBox->setSuffix(" sec");
	displayTimeBox->setMinimum(0.0);
	displayTimeBox->setMaximum(30);
	displayTimeBox->setSingleStep(.2);
	displayTimeBox->setValue(DkSettingsManager::param().slideShow().time);

	DkGroupWidget* slideshowGroup = new DkGroupWidget(tr("Slideshow"), this);
	slideshowGroup->addWidget(fadeImageLabel);
	slideshowGroup->addWidget(cbTransition);
	slideshowGroup->addWidget(fadeImageBox);
	slideshowGroup->addWidget(cbAlwaysAnimate);
	slideshowGroup->addWidget(displayTimeLabel);
	slideshowGroup->addWidget(displayTimeBox);

	// show crop from metadata
	QCheckBox* showCrop = new QCheckBox(tr("Show crop rectangle"), this);
	showCrop->setObjectName("showCrop");
	showCrop->setChecked(DkSettingsManager::param().display().showCrop);

	DkGroupWidget* showCropGroup = new DkGroupWidget(tr("Show Metadata Cropping"), this);
	showCropGroup->addWidget(showCrop);

	// left column
	QWidget* leftWidget = new QWidget(this);
	QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
	leftLayout->setAlignment(Qt::AlignTop);
	leftLayout->addWidget(zoomGroup);
	leftLayout->addWidget(keepZoomGroup);
	leftLayout->addWidget(iconGroup);
	leftLayout->addWidget(slideshowGroup);
	leftLayout->addWidget(showCropGroup);

	// right column
	QWidget* rightWidget = new QWidget(this);
	QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
	rightLayout->setAlignment(Qt::AlignTop);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setAlignment(Qt::AlignLeft);

	layout->addWidget(leftWidget);
	layout->addWidget(rightWidget);
}

void DkDisplayPreference::on_interpolationBox_valueChanged(int value) const {

	if (DkSettingsManager::param().display().interpolateZoomLevel != value)
		DkSettingsManager::param().display().interpolateZoomLevel = value;

}

void DkDisplayPreference::on_fadeImageBox_valueChanged(double value) const {

	if (DkSettingsManager::param().display().animationDuration != value)
		DkSettingsManager::param().display().animationDuration = (float)value;

}

void DkDisplayPreference::on_displayTimeBox_valueChanged(double value) const {

	if (DkSettingsManager::param().slideShow().time != value)
		DkSettingsManager::param().slideShow().time = (float)value;

}

void DkDisplayPreference::on_iconSizeBox_valueChanged(int value) const {

	if (DkSettingsManager::param().display().iconSize != value) {
		DkSettingsManager::param().display().iconSize = value;
		emit infoSignal(tr("Please Restart nomacs to apply changes"));
	}

}

void DkDisplayPreference::on_keepZoom_buttonClicked(int buttonId) const {
	
	if (DkSettingsManager::param().display().keepZoom != buttonId)
		DkSettingsManager::param().display().keepZoom = buttonId;
}

void DkDisplayPreference::on_invertZoom_toggled(bool checked) const {

	if (DkSettingsManager::param().display().invertZoom != checked)
		DkSettingsManager::param().display().invertZoom = checked;
}

void DkDisplayPreference::on_zoomToFit_toggled(bool checked) const {

	if (DkSettingsManager::param().display().zoomToFit != checked)
		DkSettingsManager::param().display().zoomToFit = checked;

}

void DkDisplayPreference::on_transition_currentIndexChanged(int index) const {

	if (DkSettingsManager::param().display().transition != index)
		DkSettingsManager::param().display().transition = (DkSettings::TransitionMode)index;

}

void DkDisplayPreference::on_alwaysAnimate_toggled(bool checked) const {

	if (DkSettingsManager::param().display().alwaysAnimate != checked)
		DkSettingsManager::param().display().alwaysAnimate = checked;

}

void DkDisplayPreference::on_showCrop_toggled(bool checked) const {

	if (DkSettingsManager::param().display().showCrop != checked)
		DkSettingsManager::param().display().showCrop = checked;

}


void DkDisplayPreference::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

// DkDummySettings --------------------------------------------------------------------
DkFilePreference::DkFilePreference(QWidget* parent) : QWidget(parent) {

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkFilePreference::createLayout() {

	// temp folder
	DkDirectoryChooser* dirChooser = new DkDirectoryChooser(DkSettingsManager::param().global().tmpPath, this);
	dirChooser->setObjectName("dirChooser");

	QLabel* tLabel = new QLabel(tr("Screenshots are automatically saved to this folder"), this);
	
	DkGroupWidget* tempFolderGroup = new DkGroupWidget(tr("Use Temporary Folder"), this);
	tempFolderGroup->addWidget(dirChooser);
	tempFolderGroup->addWidget(tLabel);

	// cache size
	int maxCache = qMax(qRound(DkMemory::getTotalMemory()*0.5), 1024);
	qDebug() << "Cache size: " << maxCache;
	QSpinBox* cacheBox = new QSpinBox(this);
	cacheBox->setObjectName("cacheBox");
	cacheBox->setMinimum(0);
	cacheBox->setMaximum(maxCache);
	cacheBox->setSuffix(" MB");
	cacheBox->setMaximumWidth(200);
	cacheBox->setValue(qRound(DkSettingsManager::param().resources().cacheMemory));

	QLabel* cLabel = new QLabel(tr("We recommend to set a moderate cache value around 100 MB. [%1-%2 MB]")
		.arg(cacheBox->minimum()).arg(cacheBox->maximum()), this);
	
	DkGroupWidget* cacheGroup = new DkGroupWidget(tr("Maximal Cache Size"), this);
	cacheGroup->addWidget(cacheBox);
	cacheGroup->addWidget(cLabel);

	// history size
	// cache size
	QSpinBox* historyBox = new QSpinBox(this);
	historyBox->setObjectName("historyBox");
	historyBox->setMinimum(0);
	historyBox->setMaximum(1024);
	historyBox->setSuffix(" MB");
	historyBox->setMaximumWidth(200);
	historyBox->setValue(qRound(DkSettingsManager::param().resources().historyMemory));

	QLabel* hLabel = new QLabel(tr("We recommend to set a moderate edit history value around 100 MB. [%1-%2 MB]")
		.arg(historyBox->minimum()).arg(historyBox->maximum()), this);

	DkGroupWidget* historyGroup = new DkGroupWidget(tr("History Size"), this);
	historyGroup->addWidget(historyBox);
	historyGroup->addWidget(hLabel);


	// loading policy
	QVector<QRadioButton*> loadButtons;
	loadButtons.append(new QRadioButton(tr("Skip Images"), this));
	loadButtons[0]->setToolTip(tr("Images are skipped until the Next key is released"));
	loadButtons.append(new QRadioButton(tr("Wait for Images to be Loaded"), this));
	loadButtons[1]->setToolTip(tr("The next image is loaded after the current image is shown."));

	// check wrt the current settings
	loadButtons[0]->setChecked(!DkSettingsManager::param().resources().waitForLastImg);
	loadButtons[1]->setChecked(DkSettingsManager::param().resources().waitForLastImg);

	QButtonGroup* loadButtonGroup = new QButtonGroup(this);
	loadButtonGroup->setObjectName("loadGroup");
	loadButtonGroup->addButton(loadButtons[0], 0);
	loadButtonGroup->addButton(loadButtons[1], 1);

	DkGroupWidget* loadGroup = new DkGroupWidget(tr("Image Loading Policy"), this);
	loadGroup->addWidget(loadButtons[0]);
	loadGroup->addWidget(loadButtons[1]);

	// skip images
	QSpinBox* skipBox = new QSpinBox(this);
	skipBox->setObjectName("skipBox");
	skipBox->setMinimum(2);
	skipBox->setMaximum(1000);
	skipBox->setValue(DkSettingsManager::param().global().skipImgs);
	skipBox->setMaximumWidth(200);

	DkGroupWidget* skipGroup = new DkGroupWidget(tr("Number of Skipped Images on PgUp/PgDown"), this);
	skipGroup->addWidget(skipBox);

	// left column
	QWidget* leftWidget = new QWidget(this);
	QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
	leftLayout->addWidget(tempFolderGroup);
	leftLayout->addWidget(cacheGroup);
	leftLayout->addWidget(historyGroup);
	leftLayout->addWidget(loadGroup);
	leftLayout->addWidget(skipGroup);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setAlignment(Qt::AlignLeft);
	layout->addWidget(leftWidget);
}

void DkFilePreference::on_dirChooser_directoryChanged(const QString& dirPath) const {

	bool dirExists = QDir(dirPath).exists();
	DkSettingsManager::param().global().useTmpPath = dirExists;

	if (DkSettingsManager::param().global().tmpPath != dirPath && dirExists) {
		DkSettingsManager::param().global().tmpPath = dirPath;
	}

}

void DkFilePreference::on_loadGroup_buttonClicked(int buttonId) const {

	if (DkSettingsManager::param().resources().waitForLastImg != (buttonId == 1))
		DkSettingsManager::param().resources().waitForLastImg = (buttonId == 1);

}

void DkFilePreference::on_skipBox_valueChanged(int value) const {

	if (DkSettingsManager::param().global().skipImgs != value) {
		DkSettingsManager::param().global().skipImgs = value;
	}

}

void DkFilePreference::on_cacheBox_valueChanged(int value) const {

	if (DkSettingsManager::param().resources().cacheMemory != value) {
		DkSettingsManager::param().resources().cacheMemory = (float)value;
	}

}

void DkFilePreference::on_historyBox_valueChanged(int value) const {

	if (DkSettingsManager::param().resources().historyMemory != value) {
		DkSettingsManager::param().resources().historyMemory = (float)value;
	}
}

void DkFilePreference::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

// DkFileAssocationsSettings --------------------------------------------------------------------
DkFileAssociationsPreference::DkFileAssociationsPreference(QWidget* parent) : QWidget(parent) {

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

DkFileAssociationsPreference::~DkFileAssociationsPreference() {

	if (mSaveSettings) {
		writeSettings();
		mSaveSettings = false;
		DkSettingsManager::param().save(DkSettingsManager::instance().qSettings());
	}
}

void DkFileAssociationsPreference::createLayout() {
	
	QStringList fileFilters = DkSettingsManager::param().app().openFilters;

	mModel = new QStandardItemModel(this);
	mModel->setObjectName("fileModel");
	for (int rIdx = 1; rIdx < fileFilters.size(); rIdx++)
		mModel->appendRow(getItems(fileFilters.at(rIdx), checkFilter(fileFilters.at(rIdx), DkSettingsManager::param().app().browseFilters), checkFilter(fileFilters.at(rIdx), DkSettingsManager::param().app().registerFilters)));

	mModel->setHeaderData(0, Qt::Horizontal, tr("Filter"));
	mModel->setHeaderData(1, Qt::Horizontal, tr("Browse"));
	mModel->setHeaderData(2, Qt::Horizontal, tr("Register"));

	QTableView* filterTableView = new QTableView(this);
	filterTableView->setModel(mModel);
	filterTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	filterTableView->verticalHeader()->hide();
	//filterTableView->horizontalHeader()->hide();
	filterTableView->setShowGrid(false);
	filterTableView->resizeColumnsToContents();
	filterTableView->resizeRowsToContents();
	filterTableView->setWordWrap(false);

	QPushButton* openDefault = new QPushButton(tr("Set as Default Viewer"), this);
	openDefault->setObjectName("openDefault");

	// now the final widgets
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(filterTableView);

#ifdef Q_OS_WIN
	layout->addWidget(openDefault);
#else
	openDefault->hide();
#endif

}

void DkFileAssociationsPreference::on_fileModel_itemChanged(QStandardItem*) {

	mSaveSettings = true;
	emit infoSignal(tr("Please Restart nomacs to apply changes"));
}

void DkFileAssociationsPreference::on_openDefault_clicked() const {
	
	DkFileFilterHandling fh;
	fh.showDefaultSoftware();
}

bool DkFileAssociationsPreference::checkFilter(const QString& cFilter, const QStringList& filters) const {

	if (filters.empty() && (DkSettingsManager::param().app().containerFilters.contains(cFilter) || cFilter.contains("ico")))
		return false;

	if (filters.empty())
		return true;

	for (int idx = 0; idx < filters.size(); idx++)
		if (cFilter.contains(filters[idx]))
			return true;

	return filters.indexOf(cFilter) != -1;
}

QList<QStandardItem*> DkFileAssociationsPreference::getItems(const QString& filter, bool browse, bool reg) {

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
#ifndef Q_OS_WIN	// registering is windows only
	item->setEnabled(false);
#endif
	items.append(item);

	return items;

}

void DkFileAssociationsPreference::writeSettings() const {

	DkFileFilterHandling fh;
	DkSettingsManager::param().app().browseFilters.clear();
	DkSettingsManager::param().app().registerFilters.clear();

	for (int idx = 0; idx < mModel->rowCount(); idx++) {

		QStandardItem* item = mModel->item(idx, 0);

		if (!item)
			continue;

		QStandardItem* browseItem = mModel->item(idx, 1);
		QStandardItem* regItem = mModel->item(idx, 2);

		if (browseItem && browseItem->checkState() == Qt::Checked) {

			QString cFilter = item->text();
			cFilter = cFilter.section(QRegExp("(\\(|\\))"), 1);
			cFilter = cFilter.replace(")", "");

			DkSettingsManager::param().app().browseFilters += cFilter.split(" ");
		}

		fh.registerFileType(item->text(), tr("Image"), regItem->checkState() == Qt::Checked);

		if (regItem->checkState() == Qt::Checked) {
			DkSettingsManager::param().app().registerFilters.append(item->text());
			qDebug() << item->text() << " registered";
		}
		else
			qDebug() << item->text() << " unregistered";
	}

	fh.registerNomacs();	// register nomacs again - to be save
}

void DkFileAssociationsPreference::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

// DkAdvancedSettings --------------------------------------------------------------------
DkAdvancedPreference::DkAdvancedPreference(QWidget* parent) : QWidget(parent) {

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkAdvancedPreference::createLayout() {

	// load RAW radio buttons
	QVector<QRadioButton*> loadRawButtons;
	loadRawButtons.resize(DkSettings::raw_thumb_end);
	loadRawButtons[DkSettings::raw_thumb_always] = new QRadioButton(tr("Always Load JPG if Embedded"), this);
	loadRawButtons[DkSettings::raw_thumb_if_large] = new QRadioButton(tr("Load JPG if it Fits the Screen Resolution"), this);
	loadRawButtons[DkSettings::raw_thumb_never] = new QRadioButton(tr("Always Load RAW Data"), this);

	// check wrt the current settings
	loadRawButtons[DkSettingsManager::param().resources().loadRawThumb]->setChecked(true);

	QButtonGroup* loadRawButtonGroup = new QButtonGroup(this);
	loadRawButtonGroup->setObjectName("loadRaw");
	loadRawButtonGroup->addButton(loadRawButtons[DkSettings::raw_thumb_always], DkSettings::raw_thumb_always);
	loadRawButtonGroup->addButton(loadRawButtons[DkSettings::raw_thumb_if_large], DkSettings::raw_thumb_if_large);
	loadRawButtonGroup->addButton(loadRawButtons[DkSettings::raw_thumb_never], DkSettings::raw_thumb_never);

	QCheckBox* cbFilterRaw = new QCheckBox(tr("Apply Noise Filtering to RAW Images"), this);
	cbFilterRaw->setObjectName("filterRaw");
	cbFilterRaw->setToolTip(tr("If checked, a noise filter is applied which reduced color noise"));
	cbFilterRaw->setChecked(DkSettingsManager::param().resources().filterRawImages);

	DkGroupWidget* loadRawGroup = new DkGroupWidget(tr("RAW Loader Settings"), this);
	loadRawGroup->addWidget(loadRawButtons[DkSettings::raw_thumb_always]);
	loadRawGroup->addWidget(loadRawButtons[DkSettings::raw_thumb_if_large]);
	loadRawGroup->addWidget(loadRawButtons[DkSettings::raw_thumb_never]);
	loadRawGroup->addSpace();
	loadRawGroup->addWidget(cbFilterRaw);

	// file loading
	QCheckBox* cbSaveDeleted = new QCheckBox(tr("Ask to Save Deleted Files"), this);
	cbSaveDeleted->setObjectName("saveDeleted");
	cbSaveDeleted->setToolTip(tr("If checked, nomacs asked to save files which were deleted by other applications"));
	cbSaveDeleted->setChecked(DkSettingsManager::param().global().askToSaveDeletedFiles);

	QCheckBox* cbIgnoreExif = new QCheckBox(tr("Ignore Exif Orientation when Loading"), this);
	cbIgnoreExif->setObjectName("ignoreExif");
	cbIgnoreExif->setToolTip(tr("If checked, images are NOT rotated with respect to their Exif orientation"));
	cbIgnoreExif->setChecked(DkSettingsManager::param().metaData().ignoreExifOrientation);

	QCheckBox* cbSaveExif = new QCheckBox(tr("Save Exif Orientation"), this);
	cbSaveExif->setObjectName("saveExif");
	cbSaveExif->setToolTip(tr("If checked, orientation is written to the Exif rather than rotating the image Matrix\n") +
		tr("NOTE: this allows for rotating JPGs without losing information."));
	cbSaveExif->setChecked(DkSettingsManager::param().metaData().saveExifOrientation);

	DkGroupWidget* loadFileGroup = new DkGroupWidget(tr("File Loading/Saving"), this);
	loadFileGroup->addWidget(cbSaveDeleted);
	loadFileGroup->addWidget(cbIgnoreExif);
	loadFileGroup->addWidget(cbSaveExif);

	// batch processing
	QSpinBox* sbNumThreads = new QSpinBox(this);
	sbNumThreads->setObjectName("numThreads");
	sbNumThreads->setToolTip(tr("Choose the number of Threads in the thread pool"));
	sbNumThreads->setMinimum(1);
	sbNumThreads->setMaximum(100);
	sbNumThreads->setValue(DkSettingsManager::param().global().numThreads);

	DkGroupWidget* threadsGroup = new DkGroupWidget(tr("Number of Threads"), this);
	threadsGroup->addWidget(sbNumThreads);

	// log
	QCheckBox* cbUseLog = new QCheckBox(tr("Use Log File"), this);
	cbUseLog->setObjectName("useLog");
	cbUseLog->setToolTip(tr("If checked, a log file will be created."));
	cbUseLog->setChecked(DkSettingsManager::param().app().useLogFile);

	QPushButton* pbLog = new QPushButton(tr("Open Log"), this);
	pbLog->setObjectName("logFolder");
#ifdef Q_OS_WIN
	pbLog->setVisible(DkSettingsManager::param().app().useLogFile);
#else
	pbLog->setVisible(false);
#endif

	DkGroupWidget* useLogGroup = new DkGroupWidget(tr("Logging"), this);
	useLogGroup->addWidget(cbUseLog);
	useLogGroup->addWidget(pbLog);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(loadRawGroup);
	layout->addWidget(loadFileGroup);
	layout->addWidget(threadsGroup);
	layout->addWidget(useLogGroup);

}

void DkAdvancedPreference::on_loadRaw_buttonClicked(int buttonId) const {

	if (DkSettingsManager::param().resources().loadRawThumb != buttonId)
		DkSettingsManager::param().resources().loadRawThumb = buttonId;
}

void DkAdvancedPreference::on_filterRaw_toggled(bool checked) const {

	if (DkSettingsManager::param().resources().filterRawImages != checked)
		DkSettingsManager::param().resources().filterRawImages = checked;
}

void DkAdvancedPreference::on_saveDeleted_toggled(bool checked) const {

	if (DkSettingsManager::param().global().askToSaveDeletedFiles != checked)
		DkSettingsManager::param().global().askToSaveDeletedFiles = checked;
}

void DkAdvancedPreference::on_ignoreExif_toggled(bool checked) const {

	if (DkSettingsManager::param().metaData().ignoreExifOrientation != checked)
		DkSettingsManager::param().metaData().ignoreExifOrientation = checked;
}

void DkAdvancedPreference::on_saveExif_toggled(bool checked) const {

	if (DkSettingsManager::param().metaData().saveExifOrientation != checked)
		DkSettingsManager::param().metaData().saveExifOrientation = checked;
}

void DkAdvancedPreference::on_useLog_toggled(bool checked) const {

	if (DkSettingsManager::param().app().useLogFile != checked) {
		DkSettingsManager::param().app().useLogFile = checked;
		emit infoSignal(tr("Please Restart nomacs to apply changes"));
	}
}

void DkAdvancedPreference::on_logFolder_clicked() const {

	// TODO: add linux/mac os
	QString logPath = QDir::toNativeSeparators(DkUtils::getLogFilePath());
	QProcess::startDetached(QString("explorer /select, \"%1\"").arg(logPath));
}

void DkAdvancedPreference::on_numThreads_valueChanged(int val) const {

	if (DkSettingsManager::param().global().numThreads != val)
		DkSettingsManager::param().setNumThreads(val);

}

void DkAdvancedPreference::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

// DkDummySettings --------------------------------------------------------------------
DkEditorPreference::DkEditorPreference(QWidget* parent) : QWidget(parent) {

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkEditorPreference::createLayout() {

	mSettingsWidget = new DkSettingsWidget(this);
	mSettingsWidget->setSettings(DkSettingsManager::instance().qSettings());

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setAlignment(Qt::AlignLeft);

	layout->addWidget(mSettingsWidget);

}
void DkEditorPreference::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

//// DkDummySettings --------------------------------------------------------------------
//DkDummyPreference::DkDummyPreference(QWidget* parent) : QWidget(parent) {
//
//	createLayout();
//	QMetaObject::connectSlotsByName(this);
//}
//
//void DkDummyPreference::createLayout() {

	//QHBoxLayout* layout = new QHBoxLayout(this);
	//layout->setAlignment(Qt::AlignLeft);

	//layout->addWidget(leftWidget);

//}
//void DkDummyPreference::paintEvent(QPaintEvent *event) {
//
//	// fixes stylesheets which are not applied to custom widgets
//	QStyleOption opt;
//	opt.init(this);
//	QPainter p(this);
//	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
//
//	QWidget::paintEvent(event);
//}

}
