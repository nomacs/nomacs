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
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QStyleOption>
#include <QPainter>
#include <QAction>
#include <QFileInfo>
#include <QCheckBox>
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