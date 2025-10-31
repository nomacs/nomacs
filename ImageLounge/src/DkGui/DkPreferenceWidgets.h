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

#include <QIcon>
#include <QList>

#include "DkBaseWidgets.h"

class QStackedLayout;
class QVBoxLayout;
class QStandardItem;
class QStandardItemModel;
class QLineEdit;
class QPushButton;

namespace nmc
{
class DkWorkingDirWidget;
class DkSettingsWidget;
class DkTabEntryWidget;

// extend this class if you want to add general preference functions
class DllCoreExport DkPreferenceTabWidget : public DkNamedWidget
{
    Q_OBJECT

public:
    DkPreferenceTabWidget(const QIcon &icon, const QString &name, QWidget *parent);

    void setWidget(QWidget *widget);
    QWidget *widget() const;

    QIcon icon() const;

public slots:
    void setInfoMessage(const QString &msg);

signals:
    void restartSignal() const;

protected:
    void createLayout();

    DkResizableScrollArea *mCentralScroller = nullptr;
    QPushButton *mInfoButton;

    QIcon mIcon;
};

class DllCoreExport DkPreferenceWidget : public DkWidget
{
    Q_OBJECT

public:
    explicit DkPreferenceWidget(QWidget *parent);

    void addTabWidget(DkPreferenceTabWidget *tabWidget);

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
    QVector<DkTabEntryWidget *> mTabEntries;
    QVector<DkPreferenceTabWidget *> mWidgets;

    QStackedLayout *mCentralLayout = nullptr;
    QVBoxLayout *mTabLayout = nullptr;
};

class DkGroupWidget : public DkWidget
{
    Q_OBJECT

public:
    DkGroupWidget(const QString &title, QWidget *parent);

    void addWidget(QWidget *widget);
    void addSpace();

protected:
    void paintEvent(QPaintEvent *event) override;
    void createLayout();

    QString mTitle;
    QVBoxLayout *mContentLayout = nullptr;
};

class DkGeneralPreference : public DkWidget
{
    Q_OBJECT

public:
    explicit DkGeneralPreference(QWidget *parent = nullptr);

public slots:
    void onShowRecentFilesToggled(bool checked) const;
    void onLogRecentFilesToggled(bool checked) const;
    void onCheckOpenDuplicatesToggled(bool checked) const;
    void onExtendedTabsToggled(bool checked) const;
    void onCloseOnEscToggled(bool checked) const;
    void onCloseOnMiddleMouseToggled(bool checked) const;
    void onZoomOnWheelToggled(bool checked) const;
    void onHorZoomSkipsToggled(bool checked) const;
    void onDoubleClickForFullscreenToggled(bool checked) const;
    void onShowBgImageToggled(bool checked) const;
    void onCheckForUpdatesToggled(bool checked) const;
    void onSwitchModifierToggled(bool checked) const;
    void onLoopImagesToggled(bool checked) const;
    void onDefaultSettingsClicked();
    void onImportSettingsClicked();
    void onExportSettingsClicked();
    void onLanguageComboCurrentIndexChanged(int index) const;
    void showRestartLabel() const;

signals:
    void infoSignal(const QString &msg) const;

protected:
    void createLayout();
    void paintEvent(QPaintEvent *ev) override;

    QStringList mLanguages;
};

class DkDisplayPreference : public DkWidget
{
    Q_OBJECT

public:
    explicit DkDisplayPreference(QWidget *parent = nullptr);

public slots:
    void onInterpolationBoxValueChanged(int value) const;
    void onIconSizeBoxValueChanged(int value) const;
    void onFadeImageBoxValueChanged(double value) const;
    void onDisplayTimeBoxValueChanged(double value) const;
    void onShowPlayerToggled(bool checked) const;
    void onKeepZoomButtonClicked(int buttonId) const;
    void onInvertZoomToggled(bool checked) const;
    void onHQAntiAliasingToggled(bool checked) const;
    void onTransitionCurrentIndexChanged(int index) const;
    void onAlwaysAnimateToggled(bool checked) const;
    void onShowCropToggled(bool checked) const;
    void onShowScrollBarsToggled(bool checked) const;
    void onUseZoomLevelsToggled(bool checked) const;
    void onShowNavigationToggled(bool checked) const;
    void onZoomLevelsEditingFinished() const;
    void onZoomLevelsDefaultClicked() const;

signals:
    void infoSignal(const QString &msg) const;

protected:
    void createLayout();
    void paintEvent(QPaintEvent *ev) override;

    QWidget *mZoomLevels = nullptr;
    QLineEdit *mZoomLevelsEdit = nullptr;
};

class DkFilePreference : public DkWidget
{
    Q_OBJECT

public:
    explicit DkFilePreference(QWidget *parent = nullptr);

public slots:
    void onDirChooserDirectoryChanged(const QString &dirPath) const;
    void onLoadGroupButtonClicked(int buttonId) const;
    void onSkipBoxValueChanged(int value) const;
    void onCacheBoxValueChanged(int value) const;
    void onHistoryBoxValueChanged(int value) const;
    void onSaveGroupButtonClicked(int buttonId) const;

signals:
    void infoSignal(const QString &msg) const;

protected:
    void createLayout();
    void paintEvent(QPaintEvent *ev) override;
};

class DkFileAssociationsPreference : public DkWidget
{
    Q_OBJECT

public:
    explicit DkFileAssociationsPreference(QWidget *parent = nullptr);
    ~DkFileAssociationsPreference() override;
public slots:
    void onFileModelItemChanged(QStandardItem *);
    void onOpenDefaultClicked() const;
    void onAssociateFilesClicked();

signals:
    void infoSignal(const QString &msg) const;

protected:
    void createLayout();
    void paintEvent(QPaintEvent *ev) override;

    bool checkFilter(const QString &cFilter, const QStringList &filters) const;
    QList<QStandardItem *> getItems(const QString &filter, bool browse, bool reg);
    void writeSettings() const;

    bool mSaveSettings = false;
    QStandardItemModel *mModel = nullptr;
};

class DkAdvancedPreference : public DkWidget
{
    Q_OBJECT

public:
    explicit DkAdvancedPreference(QWidget *parent = nullptr);

public slots:
    void onLoadRawButtonClicked(int buttonId) const;
    void onFilterRawToggled(bool checked) const;
    void onSaveDeletedToggled(bool checked) const;
    void onIgnoreExifToggled(bool checked) const;
    void onSaveExifToggled(bool checked) const;
    void onUseLogToggled(bool checked) const;
    void onUseNativeToggled(bool checked) const;
    void onLogFolderClicked() const;
    void onNumThreadsValueChanged(int val) const;

signals:
    void infoSignal(const QString &msg) const;

protected:
    void createLayout();
    void paintEvent(QPaintEvent *ev) override;
};

class DkEditorPreference : public DkWidget
{
    Q_OBJECT

public:
    explicit DkEditorPreference(QWidget *parent = nullptr);

signals:
    void infoSignal(const QString &msg) const;

public slots:
    void changeSetting(const QString &key, const QVariant &value, const QStringList &groups) const;
    void removeSetting(const QString &key, const QStringList &groups) const;

protected:
    void createLayout();

    DkSettingsWidget *mSettingsWidget;
    void paintEvent(QPaintEvent *ev) override;
};

}
