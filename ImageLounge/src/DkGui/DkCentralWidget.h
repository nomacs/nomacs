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

#pragma warning(push, 0) // no warnings from includes - begin
#include <QWidget>
#pragma warning(pop) // no warnings from includes - end

#include "DkBaseWidgets.h"
#include "DkImageContainer.h"

#pragma warning(disable : 4251) // TODO: remove

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QTabBar;
class QMimeData;
class QSettings;
class QStackedLayout;

namespace nmc
{
// nomacs defines
class DkImageLoader;
class DkViewPort;
class DkPreferenceWidget;
class DkProgressBar;
class DkBatchWidget;

class DllCoreExport DkTabInfo : public QObject
{
    Q_OBJECT

public:
    enum TabMode {
        tab_single_image,
        tab_thumb_preview,
        tab_recent_files,
        tab_preferences,
        tab_batch,

        tab_empty,

        tab_end
    };

    DkTabInfo(const QSharedPointer<DkImageContainerT> imgC = QSharedPointer<DkImageContainerT>(), int idx = -1, QObject *parent = 0);
    DkTabInfo(TabMode mode, int idx = -1, QObject *parent = 0);
    ~DkTabInfo();

    bool operator==(const DkTabInfo &o) const;

    QString getFilePath() const;
    bool setDirPath(const QString &dirPath);

    QSharedPointer<DkImageContainerT> getImage() const;
    void setImage(QSharedPointer<DkImageContainerT> imgC);

    QSharedPointer<DkImageLoader> getImageLoader() const;

    void deactivate();
    void activate(bool isActive = true);

    int getTabIdx() const;
    void setTabIdx(int idx);

    void loadSettings(const QSettings &settings);
    void saveSettings(QSettings &settings) const;

    QIcon getIcon(const QSize &size);
    QString getTabText() const;

    TabMode getMode() const;
    void setMode(int mode);

protected:
    QSharedPointer<DkImageLoader> mImageLoader;
    int mTabIdx = 0;
    enum TabMode mTabMode = tab_recent_files;
    QString mFilePath = "";
};

class DkViewPort;
class DkThumbScrollWidget;
class DkRecentFilesWidget;

class DllCoreExport DkCentralWidget : public DkWidget
{
    Q_OBJECT

public:
    DkCentralWidget(QWidget *parent = 0);
    ~DkCentralWidget();

    bool hasViewPort() const;
    DkViewPort *getViewPort() const;
    DkThumbScrollWidget *getThumbScrollWidget() const;
    QString getCurrentDir() const;

    void clearAllTabs();
    void setActiveTab(int idx) const;
    int getActiveTab();
    void updateTab(QSharedPointer<DkTabInfo> tabInfo);
    QVector<QSharedPointer<DkTabInfo>> getTabs() const;
    void loadSettings();
    void saveSettings(bool clearTabs = false) const;
    QSharedPointer<DkImageContainerT> getCurrentImage() const;
    QString getCurrentFilePath() const;
    QSharedPointer<DkImageLoader> getCurrentImageLoader() const;
    bool requestClose() const;

signals:
    void imageUpdatedSignal(QSharedPointer<DkImageContainerT>) const;
    void imageHasGPSSignal(bool) const;
    void thumbViewLoadedSignal(const QString &) const;

public slots:
    void imageLoaded(QSharedPointer<DkImageContainerT> img);
    void currentTabChanged(int idx);
    void tabCloseRequested(int idx);
    void tabMoved(int from, int to);
    void setTabList(QVector<QSharedPointer<DkTabInfo>> tabInfos, int activeIndex = -1);
    void addTab(QSharedPointer<DkImageContainerT> imgC = QSharedPointer<DkImageContainerT>(), int tabIdx = -1, bool background = false);
    void addTab(const QString &filePath, int idx = -1, bool background = false);
    void addTab(const QSharedPointer<DkTabInfo> tabInfo, bool background = false);
    void removeTab(int tabIdx = -1);
    void nextTab() const;
    void previousTab() const;
    void showThumbView(bool show = true);
    void showViewPort(bool show = true);
    void showRecentFiles(bool show = true);
    void showPreferences(bool show = true);
    void showTabs(bool show = true);
    void pasteImage();
    void loadFileToTab(const QString &filePath);
    void loadFile(const QString &filePath, bool newTab = false);
    void loadDirToTab(const QString &dirPath);
    void loadUrl(const QUrl &urls, bool newTab = false);
    void loadUrls(const QList<QUrl> &urls, const int maxUrlsToLoad = 20);
    void openBatch(const QStringList &selectedFiles = QStringList());
    void showBatch(bool show = true);
    void openPreferences();
    void restart() const;
    void showProgress(bool show, int time = -1);
    void startSlideshow(bool start = true) const;
    void setInfo(const QString &msg) const;
    void renameFile();

protected:
    QTabBar *mTabbar = 0;
    DkProgressBar *mProgressBar = 0;
    QVector<QSharedPointer<DkTabInfo>> mTabInfos;

    QVector<QWidget *> mWidgets;
    QStackedLayout *mViewLayout = 0;

    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void paintEvent(QPaintEvent *ev) override;

    void createLayout();
    void updateTabIdx();
    void switchWidget(int widget);
    void switchWidget(QWidget *widget = 0);
    bool loadFromMime(const QMimeData *mimeData);
    bool loadCascadeTrainingFiles(QList<QUrl> urls);
    void updateLoader(QSharedPointer<DkImageLoader> loader) const;

    DkPreferenceWidget *createPreferences();
    DkRecentFilesWidget *createRecentFiles();
    DkThumbScrollWidget *createThumbScrollWidget();
    DkBatchWidget *createBatch();
    void createViewPort();

    enum {
        viewport_widget,
        thumbs_widget,
        recent_files_widget,
        preference_widget,
        batch_widget,

        widget_end
    };

private:
    DkThumbLoader mThumbLoader{};
};

}
