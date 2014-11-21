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

#pragma once

#include "QWidget"

#include "DkSettings.h"
#include "DkImageContainer.h"

namespace nmc {

class DkTabInfo {
	
public:
	DkTabInfo(const QSharedPointer<DkImageContainerT> imgC = QSharedPointer<DkImageContainerT>(), int idx = -1);

	bool operator==(const DkTabInfo& o) const;

	QFileInfo getFileInfo() const;
	void setFileInfo(const QFileInfo& fileInfo);

	QSharedPointer<DkImageContainerT> getImage() const;
	void setImage(QSharedPointer<DkImageContainerT> imgC);

	int getTabIdx() const;
	void setTabIdx(int idx);
	
	void saveSettings(const QSettings& settings) const;
	void loadSettings(const QSettings& settings);

	QIcon getIcon();
	QString getTabText() const;

protected:
	QSharedPointer<DkImageContainerT> imgC;
	int tabIdx;
	int mode;
};

class DkViewPort;

class DllExport DkCentralWidget : public QWidget {
	Q_OBJECT

public:
	DkCentralWidget(DkViewPort* viewport, QWidget* parent = 0);

	DkViewPort* getViewPort() const;

	void clearAllTabs();
	void updateTabs();
	void updateTab(DkTabInfo& tabInfo);

signals:
	void loadFileSignal(QFileInfo);

public slots:
	void imageLoaded(QSharedPointer<DkImageContainerT> img);
	void currentTabChanged(int idx);
	void tabCloseRequested(int idx);
	void tabMoved(int from, int to);
	void addTab(QSharedPointer<DkImageContainerT> imgC = QSharedPointer<DkImageContainerT>(), int tabIdx = -1);
	void addTab(const QFileInfo& fileInfo, int idx = -1);
	void removeTab(int tabIdx = -1);
	void nextTab() const;
	void previousTab() const;

protected:
	DkViewPort* viewport;
	QTabBar* tabbar;
	QVector<DkTabInfo> tabInfos;

	void createLayout();
	void loadSettings();
	void saveSettings() const;
	void updateTabIdx();
};


}