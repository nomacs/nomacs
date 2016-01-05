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

#pragma warning(push, 0)	// no warnings from includes
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QStyleOption>
#include <QPainter>
#include <QAction>
#include <QFileInfo>
#include <QCheckBox>
#include <QComboBox>

#include <QDebug>
#pragma warning(pop)

namespace nmc {

DkPreferenceWidget::DkPreferenceWidget(QWidget* parent) : DkWidget(parent) {

	createLayout();

	QAction* nextAction = new QAction(tr("next"), this);
	nextAction->setShortcut(Qt::Key_Down);
	connect(nextAction, SIGNAL(triggered()), this, SLOT(nextTab()));
	addAction(nextAction);

	QAction* previousAction = new QAction(tr("previous"), this);
	previousAction->setShortcut(Qt::Key_Up);
	connect(previousAction, SIGNAL(triggered()), this, SLOT(previousTab()));
	addAction(previousAction);
}

void DkPreferenceWidget::createLayout() {

	// create tab entries
	QWidget* tabs = new QWidget(this);
	tabs->setObjectName("DkPreferenceTabs");

	mTabLayout = new QVBoxLayout(tabs);
	mTabLayout->setContentsMargins(0, 60, 0, 0);
	mTabLayout->setSpacing(0);
	mTabLayout->setAlignment(Qt::AlignTop);

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
	mTabLayout->addWidget(tabEntry);
	connect(tabEntry, SIGNAL(clicked()), this, SLOT(changeTab()));
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
}

void DkPreferenceTabWidget::createLayout() {

	QLabel* titleLabel = new QLabel(name(), this);
	titleLabel->setObjectName("DkPreferenceTitle");

	mInfoLabel = new QLabel(tr(""), this);
	mInfoLabel->setObjectName("DkInfoLabel");

	mLayout = new QGridLayout(this);
	mLayout->setContentsMargins(0, 0, 0, 0);
	mLayout->setAlignment(Qt::AlignTop);
	mLayout->addWidget(titleLabel, 0, 0);
	mLayout->addWidget(mInfoLabel, 2, 0);
}

void DkPreferenceTabWidget::setWidget(QWidget* w) {

	mCentralWidget = w;
	mCentralWidget->setObjectName("DkPreferenceWidget");
	mLayout->addWidget(mCentralWidget, 1, 0);

	connect(w, SIGNAL(infoSignal(const QString&)), this, SLOT(setInfoMessage(const QString&)));
}

void DkPreferenceTabWidget::setInfoMessage(const QString& msg) {
	mInfoLabel->setText(msg);
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

	QPixmap pm = DkImage::colorizePixmap(icon.pixmap(QSize(32, 32)), QColor(255, 255, 255));
	setIcon(pm);
	setIconSize(QSize(24, 24));

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

// DkGeneralPreference --------------------------------------------------------------------
DkGeneralPreference::DkGeneralPreference(QWidget* parent) : QWidget(parent) {

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkGeneralPreference::createLayout() {

	// color settings
	QLabel* colorLabel = new QLabel(tr("Color Settings"), this);
	colorLabel->setObjectName("subTitle");

	DkColorChooser* highlightColorChooser = new DkColorChooser(QColor(0, 204, 255), tr("Highlight Color"), this);
	highlightColorChooser->setObjectName("highlightColor");
	highlightColorChooser->setColor(&DkSettings::display.highlightColor);
	connect(highlightColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* iconColorChooser = new DkColorChooser(QColor(219, 89, 2, 255), tr("Icon Color"), this);
	iconColorChooser->setObjectName("iconColor");
	iconColorChooser->setColor(&DkSettings::display.iconColor);
	connect(iconColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* bgColorChooser = new DkColorChooser(QColor(100, 100, 100, 255), tr("Background Color"), this);
	bgColorChooser->setObjectName("backgroundColor");
	bgColorChooser->setColor(&DkSettings::display.bgColor);
	connect(bgColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* fullscreenColorChooser = new DkColorChooser(QColor(86,86,90), tr("Fullscreen Color"), this);
	fullscreenColorChooser->setObjectName("fullscreenColor");
	fullscreenColorChooser->setColor(&DkSettings::slideShow.backgroundColor);
	connect(fullscreenColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* fgdHUDColorChooser = new DkColorChooser(QColor(255, 255, 255, 255), tr("HUD Foreground Color"), this);
	fgdHUDColorChooser->setObjectName("fgdHUDColor");
	fgdHUDColorChooser->setColor(&DkSettings::display.hudFgdColor);
	connect(fgdHUDColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* bgHUDColorChooser = new DkColorChooser(QColor(0, 0, 0, 100), tr("HUD Background Color"), this);
	bgHUDColorChooser->setObjectName("bgHUDColor");
	bgHUDColorChooser->setColor((DkSettings::app.appMode == DkSettings::mode_frameless) ?
		&DkSettings::display.bgColorFrameless : &DkSettings::display.hudBgColor);
	connect(bgHUDColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	// the left column (holding all color settings)
	QWidget* colorWidget = new QWidget(this);
	QVBoxLayout* colorLayout = new QVBoxLayout(colorWidget);
	colorLayout->setAlignment(Qt::AlignTop);
	colorLayout->addWidget(colorLabel);
	colorLayout->addWidget(highlightColorChooser);
	colorLayout->addWidget(iconColorChooser);
	colorLayout->addWidget(bgColorChooser);
	colorLayout->addWidget(fullscreenColorChooser);
	colorLayout->addWidget(fgdHUDColorChooser);
	colorLayout->addWidget(bgHUDColorChooser);

	// checkboxes
	QLabel* generalLabel = new QLabel(tr("General"), this);
	generalLabel->setObjectName("subTitle");

	QCheckBox* cbRecentFiles = new QCheckBox(tr("Show Recent Files on Start-Up"), this);
	cbRecentFiles->setObjectName("showRecentFiles");
	cbRecentFiles->setToolTip(tr("Show the History Panel on Start-Up"));
	cbRecentFiles->setChecked(DkSettings::app.showRecentFiles);

	QCheckBox* cbLoopImages = new QCheckBox(tr("Loop Images"), this);
	cbLoopImages->setObjectName("loopImages");
	cbLoopImages->setToolTip(tr("Start with the first image in a folder after showing the last."));
	cbLoopImages->setChecked(DkSettings::global.loop);

	QCheckBox* cbZoomOnWheel = new QCheckBox(tr("Mouse Wheel Zooms"), this);
	cbZoomOnWheel->setObjectName("zoomOnWheel");
	cbZoomOnWheel->setToolTip(tr("If checked, the mouse wheel zooms - otherwise it is used to switch between images."));
	cbZoomOnWheel->setChecked(DkSettings::global.zoomOnWheel);

	QCheckBox* cbSwitchModifier = new QCheckBox(tr("Switch CTRL with ALT"), this);
	cbSwitchModifier->setObjectName("switchModifier");
	cbSwitchModifier->setToolTip(tr("If checked, CTRL + Mouse is switched with ALT + Mouse."));
	cbSwitchModifier->setChecked(DkSettings::sync.switchModifier);

	QCheckBox* cbEnableNetworkSync = new QCheckBox(tr("Enable LAN Sync"), this);
	cbEnableNetworkSync->setObjectName("networkSync");
	cbEnableNetworkSync->setToolTip(tr("If checked, syncing in your LAN is enabled."));
	cbEnableNetworkSync->setChecked(DkSettings::sync.enableNetworkSync);

	QCheckBox* cbCloseOnEsc = new QCheckBox(tr("Close on ESC"), this);
	cbCloseOnEsc->setObjectName("closeOnEsc");
	cbCloseOnEsc->setToolTip(tr("Close nomacs if ESC is pressed."));
	cbCloseOnEsc->setChecked(DkSettings::app.closeOnEsc);

	QCheckBox* cbCheckForUpdates = new QCheckBox(tr("Check For Updates"), this);
	cbCheckForUpdates->setObjectName("checkForUpdates");
	cbCheckForUpdates->setToolTip(tr("Check for updates on start-up."));
	cbCheckForUpdates->setChecked(DkSettings::sync.checkForUpdates);

	// language
	QLabel* languageLabel = new QLabel(tr("Language"), this);
	languageLabel->setObjectName("subTitle");

	QStringList languages;
	QComboBox* languageCombo = new QComboBox(this);
	languageCombo->setObjectName("languageCombo");
	languageCombo->setToolTip(tr("Choose your preferred language."));
	DkUtils::addLanguages(languageCombo, languages);

	QLabel* translateLabel = new QLabel("<a href=\"http://www.nomacs.org/how-to-translate-nomacs/\">How-to translate nomacs</a>", this);
	translateLabel->setToolTip(tr("Info on how to translate nomacs."));
	translateLabel->setOpenExternalLinks(true);

	// the right column (holding all checkboxes)
	QWidget* cbWidget = new QWidget(this);
	QVBoxLayout* cbLayout = new QVBoxLayout(cbWidget);
	cbLayout->setAlignment(Qt::AlignTop);
	cbLayout->addWidget(generalLabel);
	cbLayout->addWidget(cbRecentFiles);
	cbLayout->addWidget(cbLoopImages);
	cbLayout->addWidget(cbZoomOnWheel);
	cbLayout->addWidget(cbSwitchModifier);
	cbLayout->addWidget(cbEnableNetworkSync);
	cbLayout->addWidget(cbCloseOnEsc);
	cbLayout->addWidget(cbCheckForUpdates);

	// add language
	cbLayout->addWidget(languageLabel);
	cbLayout->addWidget(languageCombo);
	cbLayout->addWidget(translateLabel);

	// the column widget
	QWidget* contentWidget = new QWidget(this);
	contentWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	QHBoxLayout* contentLayout = new QHBoxLayout(contentWidget);
	contentLayout->addWidget(colorWidget);
	contentLayout->addWidget(cbWidget);

	// finally my layout
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(contentWidget);

}

void DkGeneralPreference::showRestartLabel() const {
	emit infoSignal(tr("Please Restart nomacs to apply changes"));
}

void DkGeneralPreference::on_backgroundColor_accepted() const {
	DkSettings::display.defaultBackgroundColor = false;
}

void DkGeneralPreference::on_backgroundColor_resetClicked() const {
	DkSettings::display.defaultBackgroundColor = true;
}

void DkGeneralPreference::on_iconColor_accepted() const {
	DkSettings::display.defaultIconColor = false;
}

void DkGeneralPreference::on_iconColor_resetClicked() const {
	DkSettings::display.defaultIconColor = true;
}

void DkGeneralPreference::on_showRecentFiles_toggled(bool checked) const {

	if (DkSettings::app.showRecentFiles != checked)
		DkSettings::app.showRecentFiles = checked;
}

void DkGeneralPreference::on_closeOnEsc_toggled(bool checked) const {

	if (DkSettings::app.closeOnEsc != checked)
		DkSettings::app.closeOnEsc = checked;
}

void DkGeneralPreference::on_zoomOnWheel_toggled(bool checked) const {

	if (DkSettings::global.zoomOnWheel != checked) {
		DkSettings::global.zoomOnWheel = checked;
	}
}

void DkGeneralPreference::on_checkForUpdates_toggled(bool checked) const {

	if (DkSettings::sync.checkForUpdates != checked)
		DkSettings::sync.checkForUpdates = checked;
}

void DkGeneralPreference::on_switchModifier_toggled(bool checked) const {

	if (DkSettings::sync.switchModifier != checked) {

		DkSettings::sync.switchModifier = checked;

		if (DkSettings::sync.switchModifier) {
			DkSettings::global.altMod = Qt::ControlModifier;
			DkSettings::global.ctrlMod = Qt::AltModifier;
		}
		else {
			DkSettings::global.altMod = Qt::AltModifier;
			DkSettings::global.ctrlMod = Qt::ControlModifier;
		}
	}
}

void DkGeneralPreference::on_loopImages_toggled(bool checked) const {

	if (DkSettings::global.loop != checked)
		DkSettings::global.loop = checked;
}

void DkGeneralPreference::on_networkSync_toggled(bool checked) const {

	if (DkSettings::sync.enableNetworkSync != checked)
		DkSettings::sync.enableNetworkSync = checked;
}

void DkGeneralPreference::on_languageCombo_currentIndexChanged(const QString& text) const {

	if (DkSettings::global.language != text) {
		DkSettings::global.language = text;
		emit infoSignal(tr("Please Restart nomacs to apply changes"));
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

// DkAdvancedSettings --------------------------------------------------------------------
DkAdvancedPreference::DkAdvancedPreference(QWidget* parent) : QWidget(parent) {

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkAdvancedPreference::createLayout() {

	QVBoxLayout* layout = new QVBoxLayout(this);
}
void DkAdvancedPreference::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

}