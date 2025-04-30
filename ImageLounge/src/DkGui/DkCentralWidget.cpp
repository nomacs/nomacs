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

#include "DkActionManager.h"
#include "DkBasicLoader.h"
#include "DkBatch.h"
#include "DkControlWidget.h"
#include "DkDialog.h"
#include "DkImageContainer.h"
#include "DkImageLoader.h"
#include "DkMessageBox.h"
#include "DkPreferenceWidgets.h"
#include "DkSettings.h"
#include "DkThumbs.h"
#include "DkThumbsWidgets.h"
#include "DkUtils.h"
#include "DkViewPort.h"
#include "DkWidgets.h"

#ifdef WITH_PLUGINS
#include "DkPluginManager.h"
#endif

#pragma warning(push, 0) // no warnings from includes - begin
#include <QApplication>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QMimeData>
#include <QSettings>
#include <QStackedLayout>
#include <QStandardPaths>
#include <QTabBar>

#ifdef Q_OS_WIN
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#include <QNtfsPermissionCheckGuard>
#endif
#include <QStorageInfo>
#endif

#pragma warning(pop) // no warnings from includes - end

namespace nmc
{
DkTabInfo::DkTabInfo(const QSharedPointer<DkImageContainerT> imgC, int idx, QObject *parent)
    : QObject(parent)
{
    mImageLoader = QSharedPointer<DkImageLoader>(new DkImageLoader());

    if (!imgC)
        deactivate();
    mImageLoader->setCurrentImage(imgC);

    if (imgC)
        mTabMode = tab_single_image;
    mTabIdx = idx;
    mFilePath = getFilePath();
}

DkTabInfo::DkTabInfo(TabMode mode, int idx, QObject *parent)
    : QObject(parent)
{
    mImageLoader = QSharedPointer<DkImageLoader>(new DkImageLoader());
    deactivate();

    mTabMode = mode;
    mTabIdx = idx;
}

DkTabInfo::~DkTabInfo()
{
    // qDebug() << "tab at: " << mTabIdx << " released...";
}

bool DkTabInfo::operator==(const DkTabInfo &o) const
{
    return mTabIdx == o.mTabIdx;
}

void DkTabInfo::loadSettings(const QSettings &settings)
{
    QString file = settings.value("tabFileInfo", "").toString();
    int tabSetting = settings.value("tabMode", tab_single_image).toInt();

    if (tabSetting < TabMode::tab_end) {
        mTabMode = static_cast<enum TabMode>(tabSetting);
    } else {
        mTabMode = tab_single_image;
    }

    DkFileInfo info(file);
    if (info.exists())
        mImageLoader->setCurrentImage(QSharedPointer<DkImageContainerT>(new DkImageContainerT(info)));
}

void DkTabInfo::saveSettings(QSettings &settings) const
{
    QSharedPointer<DkImageContainerT> imgC;
    if (mImageLoader->getCurrentImage())
        imgC = mImageLoader->getCurrentImage();
    else
        imgC = mImageLoader->getLastImage();

    if (imgC)
        settings.setValue("tabFileInfo", imgC->filePath());
    settings.setValue("tabMode", mTabMode);
}

bool DkTabInfo::setDirPath(const DkFileInfo &dir)
{
    Q_ASSERT(dir.isDir());
    if (!dir.isDir())
        return false;

    bool dirIsLoaded = mImageLoader->loadDir(dir.path());
    if (dirIsLoaded) {
        setMode(tab_thumb_preview);
        return true;
    }

    return false;
}

QString DkTabInfo::getFilePath() const
{
    return (mImageLoader->getCurrentImage()) ? mImageLoader->getCurrentImage()->filePath() : mFilePath;
}

void DkTabInfo::setTabIdx(int tabIdx)
{
    mTabIdx = tabIdx;
}

int DkTabInfo::getTabIdx() const
{
    return mTabIdx;
}

void DkTabInfo::setImage(QSharedPointer<DkImageContainerT> imgC)
{
    mImageLoader->setCurrentImage(imgC);

    if (imgC)
        mTabMode = tab_single_image;
    mFilePath = getFilePath();
}

QSharedPointer<DkImageLoader> DkTabInfo::getImageLoader() const
{
    return mImageLoader;
}

void DkTabInfo::deactivate()
{
    activate(false);
}

void DkTabInfo::activate(bool isActive)
{
    if (mImageLoader)
        mImageLoader->activate(isActive);
}

QSharedPointer<DkImageContainerT> DkTabInfo::getImage() const
{
    if (!mImageLoader)
        return QSharedPointer<DkImageContainerT>();

    return mImageLoader->getCurrentImage();
}

QIcon DkTabInfo::getIcon(const QSize &size)
{
    if (mTabMode == tab_thumb_preview) {
        return DkImage::loadIcon(":/nomacs/img/rects.svg");
    } else if (mTabMode == tab_recent_files) {
        return DkImage::loadIcon(":/nomacs/img/bars.svg");
    } else if (mTabMode == tab_preferences) {
        return DkImage::loadIcon(":/nomacs/img/settings.svg");
    } else if (mTabMode == tab_batch) {
        return DkImage::loadIcon(":/nomacs/img/batch-processing.svg");
    }

    const QSharedPointer<DkImageContainerT> img = mImageLoader->getCurrentImage();

    if (!img) {
        return {};
    }

    // TODO: better scaling that consider aspect ratio
    return QPixmap::fromImage(img->imageScaledToHeight(size.height()));
}

QString DkTabInfo::getTabText() const
{
    QString tabText(QObject::tr("New Tab"));

    if (mTabMode == tab_thumb_preview)
        return QObject::tr("Thumbnail Preview");
    else if (mTabMode == tab_recent_files)
        return QObject::tr("Recent Files");
    else if (mTabMode == tab_preferences)
        return QObject::tr("Settings");
    else if (mTabMode == tab_batch)
        return QObject::tr("Batch");

    QSharedPointer<DkImageContainerT> imgC = mImageLoader->getCurrentImage();

    if (!imgC)
        imgC = mImageLoader->getLastImage();

    if (imgC) {
        tabText = QFileInfo(imgC->filePath()).fileName();

        if (imgC->isEdited())
            tabText += "*";
    }

    return tabText;
}

enum DkTabInfo::TabMode DkTabInfo::getMode() const
{
    return mTabMode;
}

void DkTabInfo::setMode(int mode)
{
    if (mode < TabMode::tab_end)
        mTabMode = static_cast<enum TabMode>(mode);
}

// DkCenteralWidget --------------------------------------------------------------------
DkCentralWidget::DkCentralWidget(QWidget *parent)
    : DkWidget(parent)
{
    setObjectName("DkCentralWidget");
    createLayout();
    setAcceptDrops(true);

    DkActionManager &am = DkActionManager::instance();
    connect(am.action(DkActionManager::menu_view_new_tab), &QAction::triggered, this, [this]() {
        addTab();
    });
    connect(am.action(DkActionManager::menu_view_close_tab), &QAction::triggered, this, [this]() {
        removeTab();
    });
    connect(am.action(DkActionManager::menu_view_close_all_tabs), &QAction::triggered, this, [this]() {
        clearAllTabs();
    });
    connect(am.action(DkActionManager::menu_view_first_tab), &QAction::triggered, this, [this]() {
        setActiveTab(0);
    });
    connect(am.action(DkActionManager::menu_view_previous_tab), &QAction::triggered, this, &DkCentralWidget::previousTab);
    connect(am.action(DkActionManager::menu_edit_paste), &QAction::triggered, this, &DkCentralWidget::pasteImage);

    connect(am.action(DkActionManager::menu_view_goto_tab), &QAction::triggered, this, [this]() {
        bool ok = false;
        int idx = QInputDialog::getInt(this, tr("Go to Tab"), tr("Go to tab number: "), getActiveTab() + 1, 1, getTabs().count(), 1, &ok);

        if (ok)
            setActiveTab(idx - 1);
    });
    connect(am.action(DkActionManager::menu_view_next_tab), &QAction::triggered, this, &DkCentralWidget::nextTab);
    connect(am.action(DkActionManager::menu_view_last_tab), &QAction::triggered, this, [this]() {
        setActiveTab(getTabs().count() - 1);
    });
    connect(am.action(DkActionManager::menu_tools_batch), &QAction::triggered, this, [this]() {
        openBatch();
    });
    connect(am.action(DkActionManager::menu_panel_thumbview), &QAction::triggered, this, &DkCentralWidget::showThumbView);

#ifdef WITH_PLUGINS
    if (am.pluginActionManager())
        connect(am.pluginActionManager(), &DkPluginActionManager::showViewPort, this, [this]() {
            showViewPort();
        });
#endif

    // runs in the background & will be deleted with this widget...
    DkDialogManager *dm = new DkDialogManager(this);
    dm->setCentralWidget(this);
}

DkCentralWidget::~DkCentralWidget()
{
}

void DkCentralWidget::createLayout()
{
    // add actions
    DkActionManager &am = DkActionManager::instance();

    mTabbar = new QTabBar(this);
    mTabbar->setShape(QTabBar::RoundedNorth);
    mTabbar->setElideMode(Qt::TextElideMode::ElideRight);
    mTabbar->setUsesScrollButtons(true);
    mTabbar->setTabsClosable(true);
    mTabbar->setMovable(true);
    mTabbar->installEventFilter(new TabMiddleMouseCloser([this](int idx) {
        removeTab(idx);
    }));
    mTabbar->hide();

    mProgressBar = new DkProgressBar(this);
    mProgressBar->hide();

    mWidgets.resize(widget_end);
    mWidgets[viewport_widget] = 0;
    mWidgets[recent_files_widget] = 0;
    mWidgets[thumbs_widget] = 0;
    mWidgets[preference_widget] = 0;

    QWidget *viewWidget = new QWidget(this);
    mViewLayout = new QStackedLayout(viewWidget);

    QVBoxLayout *vbLayout = new QVBoxLayout(this);
    vbLayout->setContentsMargins(0, 0, 0, 0);
    vbLayout->setSpacing(0);
    vbLayout->addWidget(mTabbar);
    vbLayout->addWidget(mProgressBar);
    vbLayout->addWidget(viewWidget);

    // connections
    connect(mTabbar, &QTabBar::currentChanged, this, &DkCentralWidget::currentTabChanged);
    connect(mTabbar, &QTabBar::tabCloseRequested, this, &DkCentralWidget::tabCloseRequested);
    connect(mTabbar, &QTabBar::tabMoved, this, &DkCentralWidget::tabMoved);

    connect(this, &DkCentralWidget::imageHasGPSSignal, DkActionManager::instance().action(DkActionManager::menu_view_gps_map), &QAction::setEnabled);

    // preferences
    connect(am.action(DkActionManager::menu_edit_preferences), &QAction::triggered, this, &DkCentralWidget::openPreferences);
}

void DkCentralWidget::saveSettings(bool saveTabs) const
{
    DefaultSettings settings;

    settings.beginGroup(objectName());
    settings.remove("Tabs");

    // user wants to store tabs
    if (saveTabs) {
        settings.beginWriteArray("Tabs");

        for (int idx = 0; idx < mTabInfos.size(); idx++) {
            settings.setArrayIndex(idx);
            mTabInfos.at(idx)->saveSettings(settings);
        }
        settings.endArray();
    }
    settings.endGroup();
}

void DkCentralWidget::loadSettings()
{
    QVector<QSharedPointer<DkTabInfo>> tabInfos;

    DefaultSettings settings;

    settings.beginGroup(objectName());

    int size = settings.beginReadArray("Tabs");
    for (int idx = 0; idx < size; idx++) {
        settings.setArrayIndex(idx);

        QSharedPointer<DkTabInfo> tabInfo = QSharedPointer<DkTabInfo>(new DkTabInfo());
        tabInfo->loadSettings(settings);
        tabInfo->setTabIdx(idx);
        tabInfos.append(tabInfo);
    }

    settings.endArray();
    settings.endGroup();

    setTabList(tabInfos);

    if (tabInfos.empty()) {
        QSharedPointer<DkTabInfo> info = QSharedPointer<DkTabInfo>(new DkTabInfo());
        info->setMode(DkTabInfo::tab_empty);
        info->setTabIdx(0);
        addTab(info);
    }
}

bool DkCentralWidget::hasViewPort() const
{
    return mWidgets[viewport_widget] != 0;
}

DkViewPort *DkCentralWidget::getViewPort() const
{
    if (!mWidgets[viewport_widget])
        qWarning() << "danger zone: viewport is queried before its initialization";

    return dynamic_cast<DkViewPort *>(mWidgets[viewport_widget]);
}

DkThumbScrollWidget *DkCentralWidget::getThumbScrollWidget() const
{
    return dynamic_cast<DkThumbScrollWidget *>(mWidgets[thumbs_widget]);
}

void DkCentralWidget::currentTabChanged(int idx)
{
    if (idx < 0 || idx >= mTabInfos.size())
        return;

    updateLoader(mTabInfos.at(idx)->getImageLoader());

    if (getThumbScrollWidget())
        getThumbScrollWidget()->clear();

    mTabInfos.at(idx)->activate();
    QSharedPointer<DkImageContainerT> imgC = mTabInfos.at(idx)->getImage();

    if (imgC && mTabInfos.at(idx)->getMode() == DkTabInfo::tab_single_image) {
        mTabInfos.at(idx)->getImageLoader()->load(imgC);
        showViewPort();
    } else if (mTabInfos.at(idx)->getMode() == DkTabInfo::tab_thumb_preview) {
        showThumbView();
    } else if (mTabInfos.at(idx)->getMode() == DkTabInfo::tab_recent_files) {
        showRecentFiles();
    } else if (mTabInfos.at(idx)->getMode() == DkTabInfo::tab_preferences) {
        showPreferences();
    } else if (mTabInfos.at(idx)->getMode() == DkTabInfo::tab_batch) {
        showBatch();
    }
}

void DkCentralWidget::updateLoader(QSharedPointer<DkImageLoader> loader) const
{
    for (int tIdx = 0; tIdx < mTabInfos.size(); tIdx++) {
        QSharedPointer<DkImageLoader> l = mTabInfos.at(tIdx)->getImageLoader();

        if (l != loader)
            mTabInfos.at(tIdx)->deactivate();

        disconnect(loader.data(), QOverload<QSharedPointer<DkImageContainerT>>::of(&DkImageLoader::imageUpdatedSignal), this, &DkCentralWidget::imageLoaded);
        disconnect(loader.data(),
                   QOverload<QSharedPointer<DkImageContainerT>>::of(&DkImageLoader::imageUpdatedSignal),
                   this,
                   &DkCentralWidget::imageUpdatedSignal);
        disconnect(loader.data(), &DkImageLoader::imageHasGPSSignal, this, &DkCentralWidget::imageHasGPSSignal);
        disconnect(loader.data(), &DkImageLoader::updateSpinnerSignalDelayed, this, &DkCentralWidget::showProgress);
        disconnect(loader.data(), &DkImageLoader::loadImageToTab, this, &DkCentralWidget::loadFileToTab);
    }

    if (!loader)
        return;

    if (hasViewPort())
        getViewPort()->setImageLoader(loader);

    connect(loader.data(),
            QOverload<QSharedPointer<DkImageContainerT>>::of(&DkImageLoader::imageUpdatedSignal),
            this,
            &DkCentralWidget::imageLoaded,
            Qt::UniqueConnection);
    connect(loader.data(),
            QOverload<QSharedPointer<DkImageContainerT>>::of(&DkImageLoader::imageUpdatedSignal),
            this,
            &DkCentralWidget::imageUpdatedSignal,
            Qt::UniqueConnection);
    connect(loader.data(), &DkImageLoader::imageHasGPSSignal, this, &DkCentralWidget::imageHasGPSSignal, Qt::UniqueConnection);
    connect(loader.data(), &DkImageLoader::updateSpinnerSignalDelayed, this, &DkCentralWidget::showProgress, Qt::UniqueConnection);
    connect(loader.data(), &DkImageLoader::loadImageToTab, this, &DkCentralWidget::loadFileToTab, Qt::UniqueConnection);
}

void DkCentralWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

DkPreferenceWidget *DkCentralWidget::createPreferences()
{
    // add preference widget ------------------------------
    DkPreferenceWidget *pw = new DkPreferenceWidget(this);
    connect(pw, &DkPreferenceWidget::restartSignal, this, &DkCentralWidget::restart, Qt::UniqueConnection);

    QSize s(22, 22);

    // general preferences
    DkPreferenceTabWidget *tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/settings.svg", s), tr("General"), this);
    DkGeneralPreference *gp = new DkGeneralPreference(this);
    connect(gp, &DkGeneralPreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(gp);
    pw->addTabWidget(tab);

    // display preferences
    tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/display.svg", s), tr("Display"), this);
    DkDisplayPreference *dp = new DkDisplayPreference(this);
    connect(dp, &DkDisplayPreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(dp);
    pw->addTabWidget(tab);

    // file preferences
    tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/dir.svg", s), tr("File"), this);
    DkFilePreference *fp = new DkFilePreference(this);
    connect(fp, &DkFilePreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(fp);
    pw->addTabWidget(tab);

    // file association preferences
    tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/nomacs-bg.svg", s), tr("File Associations"), this);
    DkFileAssociationsPreference *fap = new DkFileAssociationsPreference(this);
    connect(fap, &DkFileAssociationsPreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(fap);
    pw->addTabWidget(tab);

    // advanced preferences
    tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/advanced-settings.svg", s), tr("Advanced"), this);
    DkAdvancedPreference *ap = new DkAdvancedPreference(this);
    connect(ap, &DkAdvancedPreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(ap);
    pw->addTabWidget(tab);

    // file association preferences
    tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/sliders.svg", s), tr("Editor"), this);
    DkEditorPreference *ep = new DkEditorPreference(this);
    connect(ep, &DkEditorPreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(ep);
    pw->addTabWidget(tab);

    // add preference widget ------------------------------

    return pw;
}

DkRecentFilesWidget *DkCentralWidget::createRecentFiles()
{
    DkRecentFilesWidget *rw = new DkRecentFilesWidget(&mThumbLoader, this);
    rw->registerAction(DkActionManager::instance().action(DkActionManager::menu_file_show_recent));

    connect(rw, &DkRecentFilesWidget::loadFileSignal, this, &DkCentralWidget::loadFile);
    connect(rw, &DkRecentFilesWidget::loadDirSignal, this, &DkCentralWidget::loadDirToTab);

    return rw;
}

DkThumbScrollWidget *DkCentralWidget::createThumbScrollWidget()
{
    DkThumbScrollWidget *thumbScrollWidget = new DkThumbScrollWidget(&mThumbLoader, this);
    // thumbScrollWidget->getThumbWidget()->setBackgroundBrush(DkSettingsManager::param().slideShow().backgroundColor);
    thumbScrollWidget->registerAction(DkActionManager::instance().action(DkActionManager::menu_panel_thumbview));

    // thumbnail preview widget
    connect(thumbScrollWidget->getThumbWidget(), &DkThumbScene::loadFileSignal, this, &DkCentralWidget::loadFile);
    connect(thumbScrollWidget, &DkThumbScrollWidget::batchProcessFilesSignal, this, &DkCentralWidget::openBatch);

    return thumbScrollWidget;
}

void DkCentralWidget::createViewPort()
{
    if (hasViewPort()) {
        qDebug() << "viewport already created...";
        return;
    }

    DkViewPort *vp = 0;

    if (parent() && parent()->objectName() == "DkNoMacsFrameless")
        vp = new DkViewPortFrameless(&mThumbLoader, this);
    else if (parent() && parent()->objectName() == "DkNoMacsContrast")
        vp = new DkViewPortContrast(&mThumbLoader, this);
    else
        vp = new DkViewPort(&mThumbLoader, this);

    if (mTabbar->currentIndex() != -1)
        vp->setImageLoader(mTabInfos[mTabbar->currentIndex()]->getImageLoader());
    connect(vp, &DkViewPort::addTabSignal, this, [this](const QString &filePath) {
        addTab(filePath);
    });
    connect(vp, &DkViewPort::showProgress, this, &DkCentralWidget::showProgress);

    mWidgets[viewport_widget] = vp;
    mViewLayout->insertWidget(viewport_widget, mWidgets[viewport_widget]);
}

void DkCentralWidget::tabCloseRequested(int idx)
{
    if (idx < 0 && idx >= mTabInfos.size())
        return;

    removeTab(idx);
}

void DkCentralWidget::tabMoved(int from, int to)
{
    QSharedPointer<DkTabInfo> tabInfo = mTabInfos.at(from);
    mTabInfos.remove(from);
    mTabInfos.insert(to, tabInfo);

    updateTabIdx();
}

void DkCentralWidget::setTabList(QVector<QSharedPointer<DkTabInfo>> tabInfos, int activeIndex /* = -1 */)
{
    mTabInfos = tabInfos;

    for (QSharedPointer<DkTabInfo> &tabInfo : tabInfos)
        mTabbar->addTab(tabInfo->getTabText());

    if (activeIndex == -1)
        activeIndex = tabInfos.size() - 1;

    mTabbar->setCurrentIndex(activeIndex);

    if (tabInfos.size() > 1)
        mTabbar->show();
}

void DkCentralWidget::addTab(const QString &filePath, int idx /* = -1 */, bool background)
{
    QSharedPointer<DkImageContainerT> imgC = QSharedPointer<DkImageContainerT>(new DkImageContainerT(DkFileInfo(filePath)));
    addTab(imgC, idx, background);
}

void DkCentralWidget::addTab(QSharedPointer<DkImageContainerT> imgC, int idx /* = -1 */, bool background)
{
    if (idx == -1)
        idx = mTabInfos.size();

    QSharedPointer<DkTabInfo> tabInfo = QSharedPointer<DkTabInfo>(new DkTabInfo(imgC, idx));
    addTab(tabInfo, background);
}

void DkCentralWidget::addTab(QSharedPointer<DkTabInfo> tabInfo, bool background)
{
    mTabInfos.push_back(tabInfo);
    mTabbar->addTab(tabInfo->getTabText());

    if (!background)
        mTabbar->setCurrentIndex(tabInfo->getTabIdx());

    if (mTabInfos.size() > 1)
        mTabbar->show();

    // TODO: add a plus button
    //// Create button what must be placed in tabs row
    // QToolButton* tb = new QToolButton();
    // tb->setText("+");
    //// Add empty, not enabled tab to tabWidget
    // tabbar->addTab("");
    // tabbar->setTabEnabled(0, false);
    //// Add tab button to current tab. Button will be enabled, but tab -- not
    //
    // tabbar->setTabButton(0, QTabBar::RightSide, tb);
}

void DkCentralWidget::removeTab(int tabIdx)
{
    if (tabIdx == -1)
        tabIdx = mTabbar->currentIndex();

    // if user requests close on batch while processing - cancel batch
    if (mTabInfos[tabIdx]->getMode() == DkTabInfo::tab_batch) {
        DkBatchWidget *bw = dynamic_cast<DkBatchWidget *>(mWidgets[batch_widget]);

        if (bw)
            bw->close();
    }

    mTabInfos.remove(tabIdx);
    mTabbar->removeTab(tabIdx);
    updateTabIdx();

    switchWidget(mTabbar->currentIndex());

    if (mTabInfos.size() == 0) { // Make sure we have at least one tab
        addTab();
        imageUpdatedSignal(mTabInfos.at(0)->getImage());
        return;
    }

    if (mTabInfos.size() <= 1)
        mTabbar->hide();
}

void DkCentralWidget::clearAllTabs()
{
    int count = getTabs().count();
    for (int idx = 0; idx < count; idx++)
        removeTab();
}

void DkCentralWidget::updateTab(QSharedPointer<DkTabInfo> tabInfo)
{
    // qDebug() << tabInfo->getTabText() << " set at tab location: " << tabInfo->getTabIdx();
    mTabbar->setTabText(tabInfo->getTabIdx(), tabInfo->getTabText());
    mTabbar->setTabIcon(tabInfo->getTabIdx(), tabInfo->getIcon(mTabbar->iconSize()));
}

void DkCentralWidget::updateTabIdx()
{
    for (int idx = 0; idx < mTabInfos.size(); idx++) {
        mTabInfos[idx]->setTabIdx(idx);
    }
}

void DkCentralWidget::nextTab() const
{
    int idx = mTabbar->currentIndex();
    idx++;
    setActiveTab(idx);
}

void DkCentralWidget::previousTab() const
{
    int idx = mTabbar->currentIndex();
    idx--;
    setActiveTab(idx);
}

void DkCentralWidget::setActiveTab(int idx) const
{
    if (mTabInfos.size() < 2)
        return;

    if (idx < 0)
        idx = mTabInfos.size() - 1;

    idx %= mTabInfos.size();
    mTabbar->setCurrentIndex(idx);
}

int DkCentralWidget::getActiveTab()
{
    return mTabbar->currentIndex();
}

void DkCentralWidget::imageLoaded(QSharedPointer<DkImageContainerT> img)
{
    int idx = mTabbar->currentIndex();

    if (idx == -1) {
        addTab(img, 0);
    } else if (idx > mTabInfos.size())
        addTab(img, idx);
    else {
        QSharedPointer<DkTabInfo> tabInfo = mTabInfos[idx];
        tabInfo->setImage(img);

        updateTab(tabInfo);
        switchWidget(tabInfo->getMode());
    }

    if (img && !img->isEdited()) {
        mThumbLoader.dispatchFullImage(img->filePath(), img->pixmap());
    }
}

QVector<QSharedPointer<DkTabInfo>> DkCentralWidget::getTabs() const
{
    return mTabInfos;
}

void DkCentralWidget::showThumbView(bool show)
{
    if (mTabInfos.empty())
        return;

    QSharedPointer<DkTabInfo> tabInfo = mTabInfos[mTabbar->currentIndex()];

    showViewPort(!show);

    if (show) {
        if (!getThumbScrollWidget()) {
            mWidgets[thumbs_widget] = createThumbScrollWidget();
            mViewLayout->insertWidget(thumbs_widget, mWidgets[thumbs_widget]);
        }

        tabInfo->setMode(DkTabInfo::tab_thumb_preview);
        switchWidget(thumbs_widget);
        tabInfo->activate();

        // should be definitely true
        if (auto tw = getThumbScrollWidget()) {
            tw->updateThumbs(tabInfo->getImageLoader()->getImages());
            tw->getThumbWidget()->setImageLoader(tabInfo->getImageLoader());

            if (tabInfo->getImage())
                tw->getThumbWidget()->ensureVisible(tabInfo->getImage()->filePath());

            connect(tw, &DkThumbScrollWidget::updateDirSignal, tabInfo->getImageLoader().data(), &DkImageLoader::loadDirRecursive, Qt::UniqueConnection);
            connect(tw, &DkThumbScrollWidget::filterChangedSignal, tabInfo->getImageLoader().data(), &DkImageLoader::setFolderFilter, Qt::UniqueConnection);
            emit thumbViewLoadedSignal(tabInfo->getImageLoader().data()->getDirPath());
        }

    } else {
        if (auto tw = getThumbScrollWidget()) {
            disconnect(tw, &DkThumbScrollWidget::updateDirSignal, tabInfo->getImageLoader().data(), &DkImageLoader::loadDirRecursive);
            disconnect(tw, &DkThumbScrollWidget::filterChangedSignal, tabInfo->getImageLoader().data(), &DkImageLoader::setFolderFilter);
        }
    }
}

void DkCentralWidget::showViewPort(bool show /* = true */)
{
    if (show) {
        if (!hasViewPort())
            createViewPort();

        switchWidget(mWidgets[viewport_widget]);
        if (getCurrentImage())
            getViewPort()->setImage(getCurrentImage()->image());
    } else if (hasViewPort())
        getViewPort()->deactivate();
}

void DkCentralWidget::showRecentFiles(bool show)
{
    showViewPort(!show);
    if (show) {
        // create the preferences...
        if (!mWidgets[recent_files_widget]) {
            mWidgets[recent_files_widget] = createRecentFiles();
            mViewLayout->insertWidget(recent_files_widget, mWidgets[recent_files_widget]);
        }

        switchWidget(mWidgets[recent_files_widget]);
    }
}

void DkCentralWidget::openPreferences()
{
    // switch to tab if already opened
    for (QSharedPointer<DkTabInfo> tabInfo : mTabInfos) {
        if (tabInfo->getMode() == DkTabInfo::tab_preferences) {
            mTabbar->setCurrentIndex(tabInfo->getTabIdx());
            return;
        }
    }

    QSharedPointer<DkTabInfo> info(new DkTabInfo(DkTabInfo::tab_preferences, mTabInfos.size()));
    addTab(info);
}

void DkCentralWidget::showPreferences(bool show)
{
    showViewPort(!show);
    if (show) {
        // create the preferences...
        if (!mWidgets[preference_widget]) {
            const auto pref = createPreferences();
            mWidgets[preference_widget] = pref;
            mViewLayout->insertWidget(preference_widget, mWidgets[preference_widget]);
            connect(pref, &DkPreferenceWidget::restartSignal, this, &DkCentralWidget::restart, Qt::UniqueConnection);
        }

        switchWidget(mWidgets[preference_widget]);
    }
}

DkBatchWidget *DkCentralWidget::createBatch()
{
    return new DkBatchWidget(&mThumbLoader, getCurrentDir(), this);
}

void DkCentralWidget::openBatch(const QStringList &selectedFiles)
{
    // switch to tab if already opened
    for (QSharedPointer<DkTabInfo> tabInfo : mTabInfos) {
        if (tabInfo->getMode() == DkTabInfo::tab_batch) {
            mTabbar->setCurrentIndex(tabInfo->getTabIdx());
            return;
        }
    }

    QSharedPointer<DkTabInfo> info(new DkTabInfo(DkTabInfo::tab_batch, mTabInfos.size()));
    addTab(info);

    // create the batch dialog...
    if (!mWidgets[batch_widget]) {
        createBatch();
        mViewLayout->insertWidget(batch_widget, mWidgets[batch_widget]);
    }

    DkBatchWidget *bw = dynamic_cast<DkBatchWidget *>(mWidgets[batch_widget]);

    if (!bw) {
        qWarning() << "batch widget is NULL where it should not be!";
        return;
    }

    bw->setSelectedFiles(selectedFiles);
}

void DkCentralWidget::showBatch(bool show)
{
    showViewPort(!show);
    if (show) {
        if (!mWidgets[batch_widget]) {
            mWidgets[batch_widget] = createBatch();
            mViewLayout->insertWidget(batch_widget, mWidgets[batch_widget]);
        }

        switchWidget(mWidgets[batch_widget]);

        mWidgets[batch_widget]->show();
    }
}

void DkCentralWidget::showTabs(bool show)
{
    if (show && mTabInfos.size() > 1)
        mTabbar->show();
    else
        mTabbar->hide();
}

void DkCentralWidget::switchWidget(int widget)
{
    if (widget == DkTabInfo::tab_single_image)
        switchWidget(mWidgets[viewport_widget]);
    else if (widget == DkTabInfo::tab_thumb_preview)
        switchWidget(mWidgets[thumbs_widget]);
    else if (widget == DkTabInfo::tab_preferences)
        switchWidget(mWidgets[preference_widget]);
    else if (widget == DkTabInfo::tab_recent_files)
        switchWidget(mWidgets[recent_files_widget]);
    else if (widget == DkTabInfo::tab_batch)
        switchWidget(mWidgets[batch_widget]);
    else
        qDebug() << "Sorry, I cannot switch to widget: " << widget;
}

void DkCentralWidget::switchWidget(QWidget *widget)
{
    if (mViewLayout->currentWidget() == widget && mTabInfos[mTabbar->currentIndex()]->getMode() != DkTabInfo::tab_empty)
        return;

    if (widget)
        mViewLayout->setCurrentWidget(widget);
    else
        mViewLayout->setCurrentWidget(mWidgets[viewport_widget]);

    if (!mTabInfos.isEmpty()) {
        int mode = DkTabInfo::tab_single_image;

        if (widget == mWidgets[thumbs_widget])
            mode = DkTabInfo::tab_thumb_preview;
        else if (widget == mWidgets[recent_files_widget])
            mode = DkTabInfo::tab_recent_files;
        else if (widget == mWidgets[preference_widget])
            mode = DkTabInfo::tab_preferences;
        else if (widget == mWidgets[batch_widget])
            mode = DkTabInfo::tab_batch;

        mTabInfos[mTabbar->currentIndex()]->setMode(mode);
        updateTab(mTabInfos[mTabbar->currentIndex()]);
    }
}

void DkCentralWidget::restart() const
{
    // safe settings first - since the intention of a restart is often a global settings change
    DkSettingsManager::param().save();

    QString exe = QApplication::applicationFilePath();
    QStringList args;

    if (getCurrentImage())
        args.append(getCurrentImage()->filePath());

    QProcess p;
    bool started = p.startDetached(exe, args);

    // close me if the new instance started
    if (started)
        QApplication::closeAllWindows();
}

void DkCentralWidget::showProgress(bool show, int time)
{
    mProgressBar->setVisibleTimed(show, time);
}

void DkCentralWidget::startSlideshow(bool start) const
{
    getViewPort()->getController()->startSlideshow(start);
}

void DkCentralWidget::setInfo(const QString &msg) const
{
    if (hasViewPort())
        getViewPort()->getController()->setInfo(msg);

    qInfo() << msg;
}

QSharedPointer<DkImageContainerT> DkCentralWidget::getCurrentImage() const
{
    if (mTabInfos.empty())
        return QSharedPointer<DkImageContainerT>();

    return mTabInfos[mTabbar->currentIndex()]->getImage();
}

QString DkCentralWidget::getCurrentFilePath() const
{
    if (!getCurrentImage())
        return QString();

    return getCurrentImage()->filePath();
}

QSharedPointer<DkImageLoader> DkCentralWidget::getCurrentImageLoader() const
{
    if (mTabInfos.empty())
        return QSharedPointer<DkImageLoader>();

    return mTabInfos[mTabbar->currentIndex()]->getImageLoader();
}

bool DkCentralWidget::requestClose() const
{
    if (hasViewPort())
        return getViewPort()->unloadImage(true);

    return true;
}

QString DkCentralWidget::getCurrentDir() const
{
    QString cDir = mTabInfos[mTabbar->currentIndex()]->getImageLoader()->getDirPath();

    // load the picture folder if there is no recent directory
    if (cDir.isEmpty())
        cDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    return cDir;
}

// DropEvents --------------------------------------------------------------------
void DkCentralWidget::dragEnterEvent(QDragEnterEvent *event)
{
    printf("[DkCentralWidget] drag enter event\n");

    if (event->mimeData()->hasUrls() || event->mimeData()->hasImage()) {
        event->acceptProposedAction();
    }
    QWidget::dragEnterEvent(event);
}

void DkCentralWidget::loadFileToTab(const QString &filePath)
{
    loadFile(filePath, true);
}

void DkCentralWidget::loadFile(const QString &filePath, bool newTab)
{
    if (!newTab) {
        if (!hasViewPort())
            createViewPort();

        getViewPort()->loadFile(filePath);
        return;
    }

    // no tab to reuse -> create a new tab
    addTab(filePath, -1, mTabInfos.size() > 0);
}

/**
 * @brief loadDirToTab loads a directory path into the current tab.
 * @param dirPath the directory to load
 */
void DkCentralWidget::loadDirToTab(const QString &dirPath)
{
    if (mTabInfos.size() > 1
        || (!mTabInfos.empty() && mTabInfos.at(0)->getMode() != DkTabInfo::tab_empty && mTabInfos.at(0)->getMode() != DkTabInfo::tab_recent_files
            && mTabInfos.at(0)->getMode() != DkTabInfo::tab_single_image && mTabInfos.at(0)->getMode() != DkTabInfo::tab_thumb_preview)) {
        addTab();
    }

    QSharedPointer<DkTabInfo> targetTab = mTabInfos[mTabbar->currentIndex()];
    DkFileInfo dir(dirPath);
    Q_ASSERT(dir.isDir());

    if (dir.isDir()) {
        // try to load the dir
        bool dirIsLoaded = targetTab->setDirPath(dir);

        if (dirIsLoaded) {
            // show the directory in overview mode
            targetTab->setMode(DkTabInfo::tab_thumb_preview);
            showThumbView();
            return;
        }
    }

    setInfo(tr("I could not load \"%1\"").arg(dirPath));
}

/** loadUrls() loads a list of valid urls.
 * @param maxUrlsToLoad determines the maximum
 */
void DkCentralWidget::loadUrls(const QList<QUrl> &urls, int maxUrlsToLoad)
{
    if (urls.size() == 0)
        return;

    if (urls.size() > maxUrlsToLoad)
        qWarning() << "Too many urls found, I will only load the first" << maxUrlsToLoad;

    if (urls.size() == 1)
        loadUrl(urls[0], false);
    else {
        for (const QUrl &url : urls)
            loadUrl(url, true);
    }
}

/** loadUrl() loads a single valid url
 *  @param loadInTab: if true, replace the currently active image, so it exists.
 */
void DkCentralWidget::loadUrl(const QUrl &url, bool newTab)
{
    Q_ASSERT(url.isValid());

    QString fp = url.toString();

    // allow drops from VSCode (i.e. images in README files)
    if (fp.startsWith("vscode-resource:/"))
        fp = fp.remove("vscode-resource:/");

    // url.toString fixes windows "C:/" vs "C:\"
    QFileInfo fi(fp);

    if (!fi.exists()) {
        fi = QFileInfo(url.toLocalFile());
    }

    auto display = [&](QString msg) {
        setInfo(msg);
    };

    if (fi.exists()) {
        if (fi.isFile()) {
            // load a local file
            if (DkUtils::isValid(fi)) {
                loadFile(fi.filePath(), newTab);
            } else {
                display(tr("Unable to load file \"%1\"").arg(fi.canonicalPath()));
            }
        } else if (fi.isDir()) {
            // load a directory as thmbnail view
            loadDirToTab(fi.filePath());
        } else {
            display(tr("\"%1\" cannot be loaded").arg(fi.canonicalPath()));
        }
    } else {
        // is this the right way to do it?
        addTab();

        // load a remote url
        QSharedPointer<DkTabInfo> targetTab = mTabInfos[mTabbar->currentIndex()];
        display(tr("downloading \"%1\"").arg(url.toDisplayString()));
        targetTab->getImageLoader()->downloadFile(url);
    }
}

void DkCentralWidget::pasteImage()
{
    qDebug() << "pasting...";

    QClipboard *clipboard = QApplication::clipboard();

    if (!loadFromMime(clipboard->mimeData()))
        setInfo("Clipboard has no image...");
}

void DkCentralWidget::dropEvent(QDropEvent *event)
{
    if (event->source() == this || (hasViewPort() && event->source() == getViewPort())) {
        event->accept();
        return;
    }

    if (!loadFromMime(event->mimeData()))
        setInfo(tr("Sorry, I could not drop the content."));
}

bool DkCentralWidget::loadFromMime(const QMimeData *mimeData)
{
    if (!mimeData)
        return false;

    if (!hasViewPort())
        createViewPort();

    QStringList mimeFmts = mimeData->formats();

    // try to load an image
    QImage dropImg;

    // first see if we have MS mime data
    // since i.e. outlook also add thumbnails, we try to first load the high quality stuff
    for (const QString &fmt : mimeFmts) {
        // qDebugClean() << "mime format:" << fmt << " " << mimeData->data(fmt).size()/1024.0 << "KB";

        if (fmt.contains("Office Drawing Shape Format")) {
            // try to get rid of all decorations
            QSharedPointer<QByteArray> ba(new QByteArray(DkImage::extractImageFromDataStream(mimeData->data(fmt))));

            if (!ba->isEmpty()) {
                DkBasicLoader bl;
                bl.loadGeneral("", ba);

                dropImg = bl.image();

                if (!dropImg.isNull())
                    qDebug() << "image loaded from MS data";
                break;
            }
        }
    }

    // parse mime data. get a non-empty list of urls. url is the first.
    QList<QUrl> urls;

    if (mimeFmts.contains("text/uri-list")) {
        // we got a list of uris
        // mimeData has both urls and text (empty string. at least for dolphin 16.04.3)
        for (QUrl u : mimeData->urls()) {
            QFileInfo f = DkUtils::urlToLocalFile(u);

            if (u.isValid() && DkUtils::isValid(f))
                urls.append(u);
        }
    } else if (mimeData->formats().contains("text/plain")) {
        // we got text data. maybe it is a list of urls
        urls = DkUtils::findUrlsInTextNewline(mimeData->text());
    }
    // load from image buffer
    else if (dropImg.isNull() && mimeData->hasImage()) {
        // we got an image buffer
        dropImg = qvariant_cast<QImage>(mimeData->imageData());
        qInfo() << "Qt image loaded from mime";
    } else {
        qDebug() << "Sorry, I could not handle the clipboard data:" << mimeData->formats();
    }

    if (!dropImg.isNull()) {
        getViewPort()->loadImage(dropImg);
        return true;
    }

    if (urls.size() == 0) {
        return false;
    }

#ifdef _DEBUG
    // At this point we have a non empty list of valid urls we can load
    qDebug() << urls.size() << " files dropped:";
    for (const QUrl &url : urls) {
        QString fname = url.toLocalFile();
        QFileInfo file(fname);
        qDebug() << QString("url [%1]: %2 %3")
                        .arg(url.isLocalFile() ? QString("local") : QString("remote"))
                        .arg(url.toDisplayString())
                        .arg(url.isLocalFile() ? url.toLocalFile() : QString(""));
        Q_ASSERT(url.isValid());
    }
#endif

    // Pass urls to the appropriate loading function
    QFileInfo file(urls[0].toLocalFile());

    if (urls.size() > 1 && file.suffix() == "vec") {
        // assume multiple OpenCV vec files where dropped.
        return loadCascadeTrainingFiles(urls);
    } else {
        // load urls generically
        loadUrls(urls);
        return true;
    }
    return false;
}

/** load a number of Cascade Trainig files */
bool DkCentralWidget::loadCascadeTrainingFiles(QList<QUrl> urls)
{
    QStringList vecFiles;

    if (urls.size() > 1 && urls.at(0).toLocalFile().endsWith("vec")) {
        for (int idx = 0; idx < urls.size(); idx++)
            vecFiles.append(urls.at(idx).toLocalFile());

        // ask user for filename
        QString sPath(QFileDialog::getSaveFileName(this,
                                                   tr("Save File"),
                                                   QFileInfo(vecFiles.first()).absolutePath(),
                                                   "Cascade Training File (*.vec)",
                                                   nullptr,
                                                   DkDialog::fileDialogOptions()));

        DkBasicLoader loader;
        int numFiles = loader.mergeVecFiles(vecFiles, sPath);

        if (numFiles) {
            loadFile(sPath);
            setInfo(tr("%1 vec files merged").arg(numFiles));
            return true;
        }
    }
    return false;
}

void DkCentralWidget::renameFile()
{
    // TODO:ref move! ??
    if (!hasViewPort())
        return;

    if (getViewPort()->isEdited()) {
        setInfo(tr("Sorry, there are unsaved changes"));
        return;
    }

    // temporarily enable slow NTFS permission checks (isWritable() etc)
#if defined(Q_OS_WIN) && QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    QNtfsPermissionCheckGuard permissionGuard;
#endif

    const QString filePath = getCurrentFilePath();
    const QFileInfo fileInfo(filePath);

    if (!fileInfo.absoluteDir().exists()) {
        setInfo(tr("Sorry, the directory: %1 does not exist").arg(fileInfo.absolutePath()));
        return;
    }

    if (fileInfo.exists() && !fileInfo.isWritable()) {
        setInfo(tr("Sorry, I can't write to the file: %1").arg(fileInfo.fileName()));
        return;
    }

    // edit the filename without changing the file suffix
    const QString baseName = fileInfo.completeBaseName();

    bool ok = false;
    QString newFileName = QInputDialog::getText(this, tr("Rename: %1").arg(fileInfo.fileName()), tr("New File Name:"), QLineEdit::Normal, baseName, &ok);

    if (!ok || newFileName.isEmpty() || newFileName == baseName)
        return;

    if (!fileInfo.suffix().isEmpty())
        newFileName.append("." + fileInfo.suffix());

    qDebug() << "renaming: " << fileInfo.fileName() << " -> " << newFileName;

    const QFileInfo renamedInfo(fileInfo.absoluteDir(), newFileName);

    // the new file name could be the same file as the old file name, even if they have a different name,
    // on case-insensitive filesystems (windows).
    bool isOldFile = renamedInfo == fileInfo;

#ifdef Q_OS_WIN
    // There is a Qt bug (QTBUG-132785) with NFS mounts on Windows where a case-insensitive rename will delete
    // the original file. We'll do the same thing as Windows Explorer and refuse to rename the file.
    // If the share is mounted with "-o casesensitive=yes", then isOldFile will always be false
    if (isOldFile && "NFS" == QStorageInfo(fileInfo.absoluteFilePath()).fileSystemType()) {
        setInfo(tr("Sorry, I can't rename: NFS mount is case-insensitive"));
        qInfo() << "Ensure the filesystem is mounted case-sensitive, for example\n"
                << "mount -o anon casesensitive=yes \\192.168.0.2:/mnt/dir N:";
        return;
    }
#endif

    if (renamedInfo.exists() && !isOldFile) {
        QMessageBox infoDialog(this);
        infoDialog.setWindowTitle(tr("Overwrite: %1?").arg(newFileName));
        infoDialog.setText(tr("The file: %1 already exists.\nDo you want to replace it?").arg(newFileName));
        infoDialog.setIcon(QMessageBox::Question);
        infoDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        infoDialog.setDefaultButton(QMessageBox::No);

        if (infoDialog.exec() != QMessageBox::Yes)
            return;

        QFile oldFile(renamedInfo.absoluteFilePath());
        if (!oldFile.remove()) {
            setInfo(tr("Sorry, I can't delete: \"%1\" : %2").arg(renamedInfo.fileName()).arg(oldFile.errorString()));
            return;
        }
    }

    bool unloaded = true;
    if (getViewPort())
        unloaded = getViewPort()->unloadImage();

    if (!unloaded)
        return; // user canceled the unload (probably modified file)

    // NOTE: this permits moving the file to another directory or volume,
    //       if there are otherwise illegal characters in the name the move will fail
    QFile newFile(fileInfo.absoluteFilePath());
    bool renamed = newFile.rename(renamedInfo.absoluteFilePath());

    if (!renamed) {
        setInfo(tr("Sorry, I can't rename: \"%1\" : %2").arg(fileInfo.fileName()).arg(newFile.errorString()));
        loadFile(fileInfo.absoluteFilePath());
        return;
    }

    loadFile(renamedInfo.absoluteFilePath());
}

} // namespace nmc
