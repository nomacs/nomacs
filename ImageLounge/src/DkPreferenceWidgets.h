/*******************************************************************************************************
 DkPreferenceWidgets.h
 Created on:	14.12.2015
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2015 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2015 Florian Kleber <florian@nomacs.org>

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

// Qt defines
#include "DkBaseWidgets.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QIcon>
#include <QPushButton>
#pragma warning(pop)

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

class QStackedLayout;
class QVBoxLayout;
class QGridLayout;

namespace nmc {

class DkWorkingDirWidget;
class DkPreferenceTabWidget;
class DkTabEntryWidget;

class DllExport DkPreferenceWidget : public DkWidget {
	Q_OBJECT

public:
	DkPreferenceWidget(QWidget* parent);

	void addTabWidget(DkPreferenceTabWidget* tabWidget);

	public slots:
	void changeTab();
	void setCurrentIndex(int index);
	void previousTab();
	void nextTab();

protected:
	void createLayout();

	int mCurrentIndex = 0;
	QVector<DkTabEntryWidget*> mTabEntries;
	QVector<DkPreferenceTabWidget*> mWidgets;

	QStackedLayout* mCentralLayout = 0;
	QVBoxLayout* mTabLayout = 0;
};

// extend this class if you want to add general preference functions
class DkPreferenceTabWidget : public DkNamedWidget {
	Q_OBJECT

public:
	DkPreferenceTabWidget(const QIcon& icon, const QString& name, QWidget* parent);

	void setWidget(QWidget* widget);
	QWidget* widget() const;

	QIcon icon() const;

	public slots:
	void setInfoMessage(const QString& msg);

protected:
	void createLayout();

	QGridLayout* mLayout = 0;
	QWidget* mCentralWidget = 0;
	QLabel* mInfoLabel;

	QIcon mIcon;
};

class DkTabEntryWidget : public QPushButton {
	Q_OBJECT

public:
	DkTabEntryWidget(const QIcon& icon, const QString& text, QWidget* parent);

protected:
	void paintEvent(QPaintEvent* event);

};

class DkGeneralPreference : public QWidget {
	Q_OBJECT

public:
	DkGeneralPreference(QWidget* parent = 0);

public slots:

signals:
	void infoSignal(const QString& msg) const;

protected:
	void createLayout();
	void paintEvent(QPaintEvent* ev);

	DkWorkingDirWidget* mWorkingDir = 0;
};

class DkAdvancedPreference : public QWidget {
	Q_OBJECT

public:
	DkAdvancedPreference(QWidget* parent = 0);

public slots:

signals:
	void infoSignal(const QString& msg) const;

protected:
	void createLayout();
	void paintEvent(QPaintEvent* ev);

};

};