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

namespace nmc {

DkTabInfo::DkTabInfo(const QSharedPointer<DkImageContainerT> imgC, int idx) {

	this->imgC = imgC;
	this->tabIdx = idx;

}

bool DkTabInfo::operator ==(const DkTabInfo& o) const {

	return this->tabIdx == o.tabIdx;
}

void DkTabInfo::loadSettings(const QSettings& settings) {

}

void DkTabInfo::saveSettings(const QSettings& settings) const {

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
}

QSharedPointer<DkImageContainerT> DkTabInfo::getImage() const {

	return imgC;
}

QIcon DkTabInfo::getIcon() {

	QIcon icon(":/nomacs/img/nomacs32.png");

	if (!imgC)
		return icon;

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

	if (imgC) {

		tabText = imgC->file().fileName();
		
		if (imgC->isEdited())
			tabText += "*";
	}

	return tabText;
}

DkCentralWidget::DkCentralWidget(DkViewPort* viewport, QWidget* parent) : QWidget(parent) {

	this->viewport = viewport;
	setObjectName("DkCentralWidget");
	createLayout();
}

void DkCentralWidget::createLayout() {

	tabbar = new QTabBar(this);
	tabbar->setShape(QTabBar::RoundedNorth);
	tabbar->setTabsClosable(true);
	tabbar->setMovable(true);
	tabbar->hide();
	//addTab(QFileInfo());

	QVBoxLayout* vbLayout = new QVBoxLayout(this);
	vbLayout->setContentsMargins(0,0,0,0);
	vbLayout->setSpacing(0);
	vbLayout->addWidget(tabbar);
	vbLayout->addWidget(viewport);

	// connections
	connect(this, SIGNAL(loadFileSignal(QFileInfo)), viewport, SLOT(loadFile(QFileInfo)));
	connect(viewport->getImageLoader(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), this, SLOT(imageLoaded(QSharedPointer<DkImageContainerT>)));

	connect(tabbar, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
	connect(tabbar, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
	connect(tabbar, SIGNAL(tabMoved(int, int)), this, SLOT(tabMoved(int, int)));
}

void DkCentralWidget::saveSettings() const {

}

void DkCentralWidget::loadSettings() {

}

DkViewPort* DkCentralWidget::getViewPort() const {

	return viewport;
}

void DkCentralWidget::currentTabChanged(int idx) {

	if (idx < 0 && idx >= tabInfos.size())
		return;

	QSharedPointer<DkImageContainerT> imgC = tabInfos.at(idx).getImage();
	if (imgC) {
		viewport->loadImage(imgC);
		DkTabInfo tabInfo = tabInfos.at(idx);
		updateTab(tabInfo);
		qDebug() << "triggering: " << imgC->file().absoluteFilePath();
	}
	else {// TODO: add option for file preview
		viewport->unloadImage();
		viewport->getImageLoader()->clearPath();
		viewport->setImage(QImage());
		viewport->getController()->showRecentFiles(true);
	}
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

	for (int idx = to; idx < tabInfos.size(); idx++) {
		tabInfos[idx].setTabIdx(idx);
	}

}

void DkCentralWidget::addTab(const QFileInfo& fileInfo, int idx /* = -1 */) {

	QSharedPointer<DkImageContainerT> imgC = QSharedPointer<DkImageContainerT>(new DkImageContainerT(fileInfo));
	addTab(imgC, idx);
}

void DkCentralWidget::addTab(QSharedPointer<DkImageContainerT> imgC, int idx /* = -1 */) {

	if (idx == -1)
		idx = tabInfos.size();

	if (imgC)
		connect(imgC.data(), SIGNAL(thumbLoadedSignal(bool)), this, SLOT(updateTabIcon(bool)));

	DkTabInfo tabInfo(imgC, idx);
	tabInfos.push_back(tabInfo);
	tabbar->addTab(tabInfo.getTabText());
	tabbar->setCurrentIndex(idx);
	//tabbar->setTabButton(idx, QTabBar::ButtonPosition::RightSide, new DkButton(QPixmap(":/nomacs/img/close.png"), tr("Close")));

	if (tabInfos.size() > 1)
		tabbar->show();
}

void DkCentralWidget::removeTab(int tabIdx) {

	if (tabIdx == -1)
		tabIdx = tabbar->currentIndex();

	for (int idx = 0; idx < tabInfos.size(); idx++) {
		
		if (tabInfos.at(idx).getTabIdx() == tabIdx) {
			tabbar->removeTab(tabInfos.at(idx).getTabIdx());
			tabInfos.remove(idx);
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
	}
}

}