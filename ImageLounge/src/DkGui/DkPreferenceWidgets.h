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

#pragma warning(disable: 4251)	// TODO: remove

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

class QStackedLayout;
class QVBoxLayout;
class QGridLayout;
class QStandardItem;
class QStandardItemModel;
class QLineEdit;

namespace nmc {

class DkWorkingDirWidget;
class DkSettingsWidget;
class DkTabEntryWidget;

// extend this class if you want to add general preference functions
class DllCoreExport DkPreferenceTabWidget : public DkNamedWidget {
	Q_OBJECT

public:
	DkPreferenceTabWidget(const QIcon& icon, const QString& name, QWidget* parent);

	void setWidget(QWidget* widget);
	QWidget* widget() const;

	QIcon icon() const;

public slots:
	void setInfoMessage(const QString& msg);

signals:
	void restartSignal() const;
	
protected:
	void createLayout();

	DkResizableScrollArea* mCentralScroller = 0;
	QPushButton* mInfoButton;

	QIcon mIcon;
};

class DllCoreExport DkPreferenceWidget : public DkFadeWidget {
	Q_OBJECT

public:
	DkPreferenceWidget(QWidget* parent);

	void addTabWidget(DkPreferenceTabWidget* tabWidget);

public slots:
	void changeTab();
	void setCurrentIndex(int index);
	void previousTab();
	void nextTab();

signals:
	void restartSignal() const;

protected:
	void createLayout();

	int mCurrentIndex = 0;
	QVector<DkTabEntryWidget*> mTabEntries;
	QVector<DkPreferenceTabWidget*> mWidgets;

	QStackedLayout* mCentralLayout = 0;
	QVBoxLayout* mTabLayout = 0;
};

class DkGroupWidget : public DkWidget {
	Q_OBJECT

public:
	DkGroupWidget(const QString& title, QWidget* parent);

	void addWidget(QWidget* widget);
	void addSpace();

protected:
	void paintEvent(QPaintEvent* event) override;
	void createLayout();

	QString mTitle;
	QVBoxLayout* mContentLayout = 0;
};

class DkGeneralPreference : public DkWidget {
	Q_OBJECT

public:
	DkGeneralPreference(QWidget* parent = 0);

public slots:
	void on_themeBox_currentIndexChanged(const QString& text) const;
	void on_showRecentFiles_toggled(bool checked) const;
	void on_logRecentFiles_toggled(bool checked) const;
	void on_checkOpenDuplicates_toggled(bool checked) const;
	void on_extendedTabs_toggled(bool checked) const;
	void on_closeOnEsc_toggled(bool checked) const;
	void on_zoomOnWheel_toggled(bool checked) const;
	void on_horZoomSkips_toggled(bool checked) const;
	void on_doubleClickForFullscreen_toggled(bool checked) const;
	void on_showBgImage_toggled(bool checked) const;
	void on_checkForUpdates_toggled(bool checked) const;
	void on_switchModifier_toggled(bool checked) const;
	void on_loopImages_toggled(bool checked) const;
	void on_defaultSettings_clicked();
	void on_importSettings_clicked();
	void on_exportSettings_clicked();
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
	void paintEvent(QPaintEvent* ev) override;

	QStringList mLanguages;
};

class DkDisplayPreference : public DkWidget {
	Q_OBJECT

public:
	DkDisplayPreference(QWidget* parent = 0);

public slots:
	void on_interpolationBox_valueChanged(int value) const;
	void on_iconSizeBox_valueChanged(int value) const;
	void on_fadeImageBox_valueChanged(double value) const;
	void on_displayTimeBox_valueChanged(double value) const;
	void on_showPlayer_toggled(bool checked) const;
	void on_keepZoom_buttonClicked(int buttonId) const;
	void on_invertZoom_toggled(bool checked) const;
	void on_hQAntiAliasing_toggled(bool checked) const;
	void on_zoomToFit_toggled(bool checked) const;
	void on_transition_currentIndexChanged(int index) const;
	void on_alwaysAnimate_toggled(bool checked) const;
	void on_showCrop_toggled(bool checked) const;
	void on_showScrollBars_toggled(bool checked) const;
	void on_useZoomLevels_toggled(bool checked) const;
	void on_showNavigation_toggled(bool checked) const;
	void on_zoomLevels_editingFinished() const;	
	void on_zoomLevelsDefault_clicked() const;

signals:
	void infoSignal(const QString& msg) const;

protected:
	void createLayout();
	void paintEvent(QPaintEvent* ev) override;

	QWidget* mZoomLevels = 0;
	QLineEdit* mZoomLevelsEdit = 0;

};

class DkFilePreference : public DkWidget {
	Q_OBJECT

public:
	DkFilePreference(QWidget* parent = 0);

public slots:
	void on_dirChooser_directoryChanged(const QString& dirPath) const;
	void on_loadGroup_buttonClicked(int buttonId) const;
	void on_skipBox_valueChanged(int value) const;
	void on_cacheBox_valueChanged(int value) const;
	void on_historyBox_valueChanged(int value) const;
	void on_saveGroup_buttonClicked(int buttonId) const;

signals:
	void infoSignal(const QString& msg) const;

protected:
	void createLayout();
	void paintEvent(QPaintEvent* ev) override;

};

class DkFileAssociationsPreference : public DkWidget {
	Q_OBJECT

public:
	DkFileAssociationsPreference(QWidget* parent = 0);
	virtual ~DkFileAssociationsPreference();
public slots:
	void on_fileModel_itemChanged(QStandardItem*);
	void on_openDefault_clicked() const;
	void on_associateFiles_clicked();

signals:
   void infoSignal(const QString& msg) const;

protected:
	void createLayout();
	void paintEvent(QPaintEvent* ev) override;

	bool checkFilter(const QString& cFilter, const QStringList& filters) const;
	QList<QStandardItem*> getItems(const QString& filter, bool browse, bool reg);
	void writeSettings() const;

	bool mSaveSettings = false;
	QStandardItemModel* mModel = 0;
};

class DkAdvancedPreference : public DkWidget {
	Q_OBJECT

public:
	DkAdvancedPreference(QWidget* parent = 0);

public slots:
	void on_loadRaw_buttonClicked(int buttonId) const;
	void on_filterRaw_toggled(bool checked) const;
	void on_saveDeleted_toggled(bool checked) const;
	void on_ignoreExif_toggled(bool checked) const;
	void on_saveExif_toggled(bool checked) const;
	void on_useLog_toggled(bool checked) const;
	void on_useNative_toggled(bool checked) const;
	void on_logFolder_clicked() const;
	void on_numThreads_valueChanged(int val) const;

signals:
	void infoSignal(const QString& msg) const;

protected:
	void createLayout();
	void paintEvent(QPaintEvent* ev) override;
};

class DkEditorPreference : public DkWidget {
	Q_OBJECT

public:
	DkEditorPreference(QWidget* parent = 0);

signals:
   void infoSignal(const QString& msg) const;

public slots:
	void changeSetting(const QString& key, const QVariant& value, const QStringList& groups) const;
	void removeSetting(const QString& key, const QStringList& groups) const;

protected:
	void createLayout();

	DkSettingsWidget* mSettingsWidget;
	void paintEvent(QPaintEvent* ev) override;
};

}
