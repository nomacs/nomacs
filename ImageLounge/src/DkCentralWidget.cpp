/*******************************************************************************************************
 DkCentralWidget.cpp
 Created on:	14.11.2014
 
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

#include "DkCentralWidget.h"
#include "DkViewPort.h"
#include "DkMessageBox.h"
#include "DkThumbsWidgets.h"

namespace nmc {

DkTabInfo::DkTabInfo(const QSharedPointer<DkImageContainerT> imgC, int idx) {

	this->tabMode = tab_single_image;
	this->imgC = imgC;
	this->tabIdx = idx;

}

bool DkTabInfo::operator ==(const DkTabInfo& o) const {

	return this->tabIdx == o.tabIdx;
}

void DkTabInfo::loadSettings(const QSettings& settings) {

	QFileInfo file = settings.value("tabFileInfo", "").toString();
	tabMode = settings.value("tabMode", tab_single_image).toInt();

	if (file.exists())
		imgC = QSharedPointer<DkImageContainerT>(new DkImageContainerT(file));
}

void DkTabInfo::saveSettings(QSettings& settings) const {

	if (imgC)
		settings.setValue("tabFileInfo", imgC->file().absoluteFilePath());
	settings.setValue("tabMode", tabMode);
}

void DkTabInfo::setFileInfo(const QFileInfo& fileInfo) {

	imgC = QSharedPointer<DkImageContainerT>(new DkImageContainerT(fileInfo));
}

QFileInfo DkTabInfo::getFileInfo() const {

	return (imgC) ? imgC->file() : QFileInfo();
}

void DkTabInfo::setTabIdx(int tabIdx) {

	this->tabIdx = tabIdx;
}

int DkTabInfo::getTabIdx() const {

	return tabIdx;
}

void DkTabInfo::setImage(QSharedPointer<DkImageContainerT> imgC) {
	
	this->imgC = imgC;
	tabMode = tab_single_image;
}

QSharedPointer<DkImageContainerT> DkTabInfo::getImage() const {

	return imgC;
}

QIcon DkTabInfo::getIcon() {
	
	QIcon icon;

	if (!imgC)
		return icon;

	if (tabMode == tab_thumb_preview)
		return QIcon(":/nomacs/img/thumbs-view.png");

	QSharedPointer<DkThumbNailT> thumb = imgC->getThumb();

	if (!thumb)
		return icon;

	QImage img = thumb->getImage();

	if (!img.isNull())
		icon = QPixmap::fromImage(img);

	return icon;
}

QString DkTabInfo::getTabText() const {

	QString tabText(QObject::tr("New Tab"));

	if (tabMode == tab_thumb_preview)
		return QObject::tr("Thumbnail Preview");

	if (imgC) {

		tabText = imgC->file().fileName();
		
		if (imgC->isEdited())
			tabText += "*";
	}

	return tabText;
}

int DkTabInfo::getMode() const {

	return tabMode;
}

void DkTabInfo::setMode(int mode) {

	this->tabMode = mode;
}

DkCentralWidget::DkCentralWidget(DkViewPort* viewport, QWidget* parent) : QWidget(parent) {

	this->viewport = viewport;
	setObjectName("DkCentralWidget");
	createLayout();
	loadSettings();

	if (tabInfos.empty())
		addTab();
}

DkCentralWidget::~DkCentralWidget() {
}

void DkCentralWidget::createLayout() {

	thumbScrollWidget = new DkThumbScrollWidget(this);
	thumbScrollWidget->getThumbWidget()->setBackgroundBrush(DkSettings::slideShow.backgroundColor);
	//thumbScrollWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	thumbScrollWidget->hide();

	tabbar = new QTabBar(this);
	tabbar->setShape(QTabBar::RoundedNorth);
	tabbar->setTabsClosable(true);
	tabbar->setMovable(true);
	tabbar->hide();
	//addTab(QFileInfo());

	widgets.resize(widget_end);
	widgets[viewport_widget] = viewport;
	widgets[thumbs_widget] = thumbScrollWidget;

	QWidget* viewWidget = new QWidget(this);
	viewLayout = new QStackedLayout(viewWidget);

	//for each (QWidget* w in widgets)
	for (QWidget* w : widgets)
		viewLayout->addWidget(w);

	QVBoxLayout* vbLayout = new QVBoxLayout(this);
	vbLayout->setContentsMargins(0,0,0,0);
	vbLayout->setSpacing(0);
	vbLayout->addWidget(tabbar);
	vbLayout->addWidget(viewWidget);

	// connections
	connect(this, SIGNAL(loadFileSignal(QFileInfo)), viewport, SLOT(loadFile(QFileInfo)));
	connect(viewport, SIGNAL(addTabSignal(const QFileInfo&)), this, SLOT(addTab(const QFileInfo&)));
	connect(viewport->getImageLoader(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), this, SLOT(imageLoaded(QSharedPointer<DkImageContainerT>)));

	connect(tabbar, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
	connect(tabbar, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
	connect(tabbar, SIGNAL(tabMoved(int, int)), this, SLOT(tabMoved(int, int)));

	// thumbnail preview widget
	//connect(viewport->getImageLoader(), SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT> >)), thumbScrollWidget, SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT> >)));
	connect(thumbScrollWidget->getThumbWidget(), SIGNAL(loadFileSignal(QFileInfo)), viewport, SLOT(loadFile(QFileInfo)));
	connect(thumbScrollWidget, SIGNAL(updateDirSignal(QFileInfo)), viewport, SLOT(loadFile(QFileInfo)));
	connect(thumbScrollWidget->getThumbWidget(), SIGNAL(statusInfoSignal(QString, int)), this, SLOT(showStatusMessage(QString, int)));

}

void DkCentralWidget::saveSettings(bool clearTabs) {

	if (tabInfos.size() <= 1)	// nothing to save here
		return;

	QSettings& settings = Settings::instance().getSettings();

	settings.beginGroup(objectName());
	settings.remove("Tabs");

	if (clearTabs) {

		settings.beginWriteArray("Tabs");

		for (int idx = 0; idx < tabInfos.size(); idx++) {
			settings.setArrayIndex(idx);
			tabInfos.at(idx).saveSettings(settings);
		}
		settings.endArray();
	}
	settings.endGroup();

}

void DkCentralWidget::loadSettings() {

	QSettings& settings = Settings::instance().getSettings();

	settings.beginGroup(objectName());

	int size = settings.beginReadArray("Tabs");
	for (int idx = 0; idx < size; idx++) {
		settings.setArrayIndex(idx);

		DkTabInfo tabInfo;
		tabInfo.loadSettings(settings);
		tabInfo.setTabIdx(idx);
		addTab(tabInfo);
	}

	settings.endArray();
	settings.endGroup();
}

DkViewPort* DkCentralWidget::getViewPort() const {

	return viewport;
}

DkThumbScrollWidget* DkCentralWidget::getThumbScrollWidget() const {

	return thumbScrollWidget;
}

void DkCentralWidget::currentTabChanged(int idx) {

	if (idx < 0 && idx >= tabInfos.size())
		return;

	QSharedPointer<DkImageContainerT> imgC = tabInfos.at(idx).getImage();

	if (imgC && tabInfos.at(idx).getMode() == DkTabInfo::tab_single_image) {
		showViewPort(true);
	}
	else if (tabInfos.at(idx).getMode() == DkTabInfo::tab_thumb_preview) {
		showThumbView(true);
	}
	else {
		viewport->unloadImage();
		viewport->getImageLoader()->clearPath();
		viewport->setImage(QImage());
		viewport->getController()->showRecentFiles(true);
	}

	switchWidget(tabInfos.at(idx).getMode());
}

void DkCentralWidget::tabCloseRequested(int idx) {

	if (idx < 0 && idx >= tabInfos.size())
		return;

	removeTab(idx);
}

void DkCentralWidget::tabMoved(int from, int to) {

	DkTabInfo tabInfo = tabInfos.at(from);
	tabInfos.remove(from);
	tabInfos.insert(to, tabInfo);

	updateTabIdx();
}

void DkCentralWidget::addTab(const QFileInfo& fileInfo, int idx /* = -1 */) {

	QSharedPointer<DkImageContainerT> imgC = QSharedPointer<DkImageContainerT>(new DkImageContainerT(fileInfo));
	addTab(imgC, idx);
}

void DkCentralWidget::addTab(QSharedPointer<DkImageContainerT> imgC, int idx /* = -1 */) {

	if (idx == -1)
		idx = tabInfos.size();

	DkTabInfo tabInfo(imgC, idx);
	addTab(tabInfo);
}

void DkCentralWidget::addTab(const DkTabInfo& tabInfo) {

	tabInfos.push_back(tabInfo);
	tabbar->addTab(tabInfo.getTabText());
	tabbar->setCurrentIndex(tabInfo.getTabIdx());
	//tabbar->setTabButton(idx, QTabBar::ButtonPosition::RightSide, new DkButton(QPixmap(":/nomacs/img/close.png"), tr("Close")));

	if (tabInfos.size() > 1)
		tabbar->show();

	// TODO: add a plus button
	//// Create button what must be placed in tabs row
	//QToolButton* tb = new QToolButton();
	//tb->setText("+");
	//// Add empty, not enabled tab to tabWidget
	//tabbar->addTab("");
	//tabbar->setTabEnabled(0, false);
	//// Add tab button to current tab. Button will be enabled, but tab -- not
	//
	//tabbar->setTabButton(0, QTabBar::RightSide, tb);
}

void DkCentralWidget::removeTab(int tabIdx) {

	if (tabIdx == -1)
		tabIdx = tabbar->currentIndex();

	for (int idx = 0; idx < tabInfos.size(); idx++) {
		
		if (tabInfos.at(idx).getTabIdx() == tabIdx) {
			tabInfos.remove(idx);
			tabbar->removeTab(tabIdx);
		}
	}

	updateTabIdx();

	if (tabInfos.size() <= 1)
		tabbar->hide();
}

void DkCentralWidget::clearAllTabs() {
	
	for (int idx = 0; idx < tabInfos.size(); idx++)
		tabbar->removeTab(tabInfos.at(idx).getTabIdx());
	
	tabInfos.clear();

	tabbar->hide();
}

void DkCentralWidget::updateTab(DkTabInfo& tabInfo) {

	tabbar->setTabText(tabInfo.getTabIdx(), tabInfo.getTabText());
	tabbar->setTabIcon(tabInfo.getTabIdx(), tabInfo.getIcon());
}

void DkCentralWidget::updateTabIdx() {

	for (int idx = 0; idx < tabInfos.size(); idx++) {
		tabInfos[idx].setTabIdx(idx);
	}
}

void DkCentralWidget::nextTab() const {

	if (tabInfos.size() < 2)
		return;

	int idx = tabbar->currentIndex();
	idx++;
	idx %= tabInfos.size();
	tabbar->setCurrentIndex(idx);
}

void DkCentralWidget::previousTab() const {

	if (tabInfos.size() < 2)
		return;

	int idx = tabbar->currentIndex();
	idx--;
	if (idx < 0)
		idx = tabInfos.size()-1;
	tabbar->setCurrentIndex(idx);
}

void DkCentralWidget::imageLoaded(QSharedPointer<DkImageContainerT> img) {

	int idx = tabbar->currentIndex();

	if (idx == -1) {
		addTab(img, 0);
	}
	else if (idx > tabInfos.size())
		addTab(img, idx);
	else {
		DkTabInfo tabInfo = tabInfos.at(idx);
		tabInfo.setImage(img);
		tabInfos.replace(idx, tabInfo);

		updateTab(tabInfo);
		switchWidget(tabInfo.getMode());
	}
}

QVector<DkTabInfo> DkCentralWidget::getTabs() const {

	return tabInfos;
}

void DkCentralWidget::showThumbView(bool show) {

	//if (show == thumbScrollWidget->isVisible())
	//	return;

	if (show) {

		
		DkTabInfo& tabInfo = tabInfos[tabbar->currentIndex()];
		tabInfo.setMode(DkTabInfo::tab_thumb_preview);

		// clear viewport
		viewport->unloadImage();
		viewport->getImageLoader()->clearPath();
		viewport->setImage(QImage());

		switchWidget(thumbs_widget);
		DkImageLoader* loader = viewport->getImageLoader();
		loader->setCurrentImage(tabInfo.getImage());
		thumbScrollWidget->updateThumbs(loader->getImages());
		//thumbScrollWidget->getThumbWidget()->updateLayout();
	}
	else
		showViewPort(true);	// TODO: this triggers switchWidget - but switchWidget might also trigger showThumbView(false)
}

void DkCentralWidget::showViewPort(bool show /* = true */) {

	if (show) {
		QSharedPointer<DkImageContainerT> imgC = tabInfos[tabbar->currentIndex()].getImage();
		if (imgC && imgC != viewport->getImageLoader()->getCurrentImage())
			viewport->loadImage(imgC);
		else
			viewport->getImageLoader()->firstFile();

		switchWidget(widgets[viewport_widget]);
	}
}

void DkCentralWidget::switchWidget(int widget) {

	if (widget == DkTabInfo::tab_single_image)
		switchWidget(widgets[viewport_widget]);
	else if (widget == DkTabInfo::tab_thumb_preview)
		switchWidget(widgets[thumbs_widget]);
	else
		qDebug() << "Sorry, I cannot switch to widget: " << widget;

}

void DkCentralWidget::switchWidget(QWidget* widget) {

	if (viewLayout->currentWidget() == widget)
		return;

	if (widget)
		viewLayout->setCurrentWidget(widget);
	else
		viewLayout->setCurrentWidget(widgets[viewport_widget]);

	if (!tabInfos.isEmpty()) {
		
		int mode = widget == widgets[viewport_widget] ? DkTabInfo::tab_single_image : DkTabInfo::tab_thumb_preview;
		tabInfos[tabbar->currentIndex()].setMode(mode);
		updateTab(tabInfos[tabbar->currentIndex()]);
	}
	
}

int DkCentralWidget::currentViewMode() const {

	return tabInfos[tabbar->currentIndex()].getMode();
}

}