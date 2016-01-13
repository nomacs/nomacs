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
#include <QList>
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
class QStandardItem;
class QStandardItemModel;

namespace nmc {

class DkWorkingDirWidget;

// extend this class if you want to add general preference functions
class DllExport DkPreferenceTabWidget : public DkNamedWidget {
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

class DllExport DkTabEntryWidget : public QPushButton {
	Q_OBJECT

public:
	DkTabEntryWidget(const QIcon& icon, const QString& text, QWidget* parent);

protected:
	void paintEvent(QPaintEvent* event);

};

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

class DkGroupWidget : public QWidget {
	Q_OBJECT

public:
	DkGroupWidget(const QString& title, QWidget* parent);

	void addWidget(QWidget* widget);
	void addSpace();

protected:
	void paintEvent(QPaintEvent* event);
	void createLayout();

	QString mTitle;
	QVBoxLayout* mContentLayout = 0;
};

class DkGeneralPreference : public QWidget {
	Q_OBJECT

public:
	DkGeneralPreference(QWidget* parent = 0);

public slots:
	void on_showRecentFiles_toggled(bool checked) const;
	void on_logRecentFiles_toggled(bool checked) const;
	void on_closeOnEsc_toggled(bool checked) const;
	void on_zoomOnWheel_toggled(bool checked) const;
	void on_checkForUpdates_toggled(bool checked) const;
	void on_switchModifier_toggled(bool checked) const;
	void on_loopImages_toggled(bool checked) const;
	void on_networkSync_toggled(bool checked) const;
	void on_defaultSettings_clicked();
	void on_languageCombo_currentIndexChanged(int index) const;
	void showRestartLabel() const;

	void on_backgroundColor_accepted() const;
	void on_backgroundColor_resetClicked() const;
	void on_iconColor_accepted() const;
	void on_iconColor_resetClicked() const;
	
signals:
	void infoSignal(const QString& msg) const;

protected:
	void createLayout();
	void paintEvent(QPaintEvent* ev);

	QStringList mLanguages;
};

class DkDisplayPreference : public QWidget {
	Q_OBJECT

public:
	DkDisplayPreference(QWidget* parent = 0);

public slots:
	void on_interpolationBox_valueChanged(int value) const;
	void on_iconSizeBox_valueChanged(int value) const;
	void on_fadeImageBox_valueChanged(double value) const;
	void on_displayTimeBox_valueChanged(double value) const;
	void on_keepZoom_buttonClicked(int buttonId) const;
	void on_invertZoom_toggled(bool checked) const;

signals:
	void infoSignal(const QString& msg) const;

protected:
	void createLayout();
	void paintEvent(QPaintEvent* ev);

};

class DkFilePreference : public QWidget {
	Q_OBJECT

public:
	DkFilePreference(QWidget* parent = 0);

public slots:
	void on_dirChooser_directoryChanged(const QString& dirPath) const;
	void on_loadGroup_buttonClicked(int buttonId) const;
	void on_skipBox_valueChanged(int value) const;
	void on_cacheBox_valueChanged(int value) const;

signals:
	void infoSignal(const QString& msg) const;

protected:
	void createLayout();
	void paintEvent(QPaintEvent* ev);

};

class DkFileAssociationsPreference : public QWidget {
	Q_OBJECT

public:
	DkFileAssociationsPreference(QWidget* parent = 0);
	virtual ~DkFileAssociationsPreference();
public slots:
	void on_fileModel_itemChanged(QStandardItem*);
	void on_openDefault_clicked() const;

signals:
   void infoSignal(const QString& msg) const;

protected:
	void createLayout();
	void paintEvent(QPaintEvent* ev);

	bool checkFilter(const QString& cFilter, const QStringList& filters) const;
	QList<QStandardItem*> getItems(const QString& filter, bool browse, bool reg);
	void writeSettings() const;

	bool mSaveSettings = false;
	QStandardItemModel* mModel = 0;
};

class DkAdvancedPreference : public QWidget {
	Q_OBJECT

public:
	DkAdvancedPreference(QWidget* parent = 0);

public slots:
	void on_loadRaw_buttonClicked(int buttonId) const;
	void on_filterRaw_toggled(bool checked) const;
	void on_saveDeleted_toggled(bool checked) const;
	void on_ignoreExif_toggled(bool checked) const;
	void on_saveExif_toggled(bool checked) const;

signals:
	void infoSignal(const QString& msg) const;

protected:
	void createLayout();
	void paintEvent(QPaintEvent* ev);

};

//class DkDummyPreference : public QWidget {
//	Q_OBJECT
//
//public:
//	DkDummyPreference(QWidget* parent = 0);
//
//public slots:
//
//signals:
//   void infoSignal(const QString& msg) const;
//
//protected:
//	void createLayout();
//	void paintEvent(QPaintEvent* ev);
//
//};

};