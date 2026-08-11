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
#include "DkThumbsWidgets.h"
#include "DkUtils.h"
#include "DkViewPort.h"
#include "DkWidgets.h"

#ifdef WITH_PLUGINS
#include "DkPluginManager.h"
#endif

#include <QApplication>
#include <QClipboard>
#include <QColorSpace>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QMimeData>
#include <QProcess>
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
}

DkTabInfo::DkTabInfo(TabMode mode, int idx, QObject *parent)
    : QObject(parent)
{
    mImageLoader = QSharedPointer<DkImageLoader>(new DkImageLoader());
    deactivate();

    mTabMode = mode;
    mTabIdx = idx;
}

DkTabInfo::~DkTabInfo() = default;

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
    auto imgC = mImageLoader->getCurrentImage();
    if (imgC)
        settings.setValue("tabFileInfo", imgC->filePath());
    settings.setValue("tabMode", mTabMode);
}

QString DkTabInfo::getFilePath() const
{
    return mImageLoader->filePath();
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
    mImageLoader->activate(isActive);
}

QSharedPointer<DkImageContainerT> DkTabInfo::getImage() const
{
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

    if (!img || !img->hasImage()) {
        return {};
    }

    // TODO: better scaling that consider aspect ratio
    QImage icon = img->imageScaledToHeight(size.height());
    icon.convertToColorSpace(QColorSpace{QColorSpace::SRgb});
    return QPixmap::fromImage(icon);
}

QString DkTabInfo::getTabText() const
{
    if (mTabMode == tab_recent_files)
        return QObject::tr("Recent Files");
    else if (mTabMode == tab_preferences)
        return QObject::tr("Settings");
    else if (mTabMode == tab_batch)
        return QObject::tr("Batch");

    QString tabText(QObject::tr("New Tab"));
    if (mTabMode == tab_thumb_preview)
        tabText = QObject::tr("Thumbnail Preview");

    QSharedPointer<DkImageContainerT> imgC = mImageLoader->getCurrentImage();
    if (!imgC)
        return tabText;

    DkFileInfo info = imgC->fileInfo();
    if (mTabMode == tab_thumb_preview)
        tabText = QFileInfo(info.dirPath()).fileName();
    else
        tabText = info.fileName();

    if (imgC->isEdited())
        tabText += "*";

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

bool DkTabInfo::useForNewImageTab() const
{
    auto imgC = getImage();
    if (imgC && imgC->isEdited()) {
        return false;
    }

    switch (mTabMode) {
    case tab_recent_files:
    case tab_preferences:
    case tab_empty:
        return true;
    case tab_single_image:
    case tab_thumb_preview:
    case tab_batch:
    case tab_end:
        return false;
    }
    return false;
}

// DkCenteralWidget --------------------------------------------------------------------
DkCentralWidget::DkCentralWidget(QWidget *parent)
    : DkWidget(parent)
{
    setObjectName("DkCentralWidget");
    createLayout();
    setAcceptDrops(true);

    DkActionManager &am = DkActionManager::instance();
    connect(am.action(DkActionManager::view_new_tab), &QAction::triggered, this, [this]() {
        addTab();
    });
    connect(am.action(DkActionManager::view_close_tab), &QAction::triggered, this, [this]() {
        removeTab();
    });
    connect(am.action(DkActionManager::view_close_all_tabs), &QAction::triggered, this, [this]() {
        clearAllTabs();
    });
    connect(am.action(DkActionManager::view_first_tab), &QAction::triggered, this, [this]() {
        setActiveTab(0);
    });
    connect(am.action(DkActionManager::view_previous_tab), &QAction::triggered, this, &DkCentralWidget::previousTab);
    connect(am.action(DkActionManager::edit_paste), &QAction::triggered, this, &DkCentralWidget::pasteImage);

    connect(am.action(DkActionManager::view_goto_tab), &QAction::triggered, this, [this]() {
        bool ok = false;
        int idx = QInputDialog::getInt(this,
                                       tr("Go to Tab"),
                                       tr("Go to tab number: "),
                                       getActiveTab() + 1,
                                       1,
                                       getTabs().count(),
                                       1,
                                       &ok);

        if (ok)
            setActiveTab(idx - 1);
    });
    connect(am.action(DkActionManager::view_next_tab), &QAction::triggered, this, &DkCentralWidget::nextTab);
    connect(am.action(DkActionManager::view_last_tab), &QAction::triggered, this, [this]() {
        setActiveTab(getTabs().count() - 1);
    });
    connect(am.action(DkActionManager::tools_batch), &QAction::triggered, this, [this]() {
        openBatch();
    });
    connect(am.action(DkActionManager::panel_thumbview), &QAction::triggered, this, &DkCentralWidget::showThumbView);

#ifdef WITH_PLUGINS
    if (am.pluginActionManager())
        connect(am.pluginActionManager(), &DkPluginActionManager::showViewPort, this, [this]() {
            showViewPort();
        });
#endif

    // runs in the background & will be deleted with this widget...
    auto *dm = new DkDialogManager(this);
    dm->setCentralWidget(this);
}

DkCentralWidget::~DkCentralWidget() = default;

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

    auto *viewWidget = new QWidget(this);
    mViewLayout = new QStackedLayout(viewWidget);

    auto *vbLayout = new QVBoxLayout(this);
    vbLayout->setContentsMargins(0, 0, 0, 0);
    vbLayout->setSpacing(0);
    vbLayout->addWidget(mTabbar);
    vbLayout->addWidget(viewWidget);

    // connections
    connect(mTabbar, &QTabBar::currentChanged, this, &DkCentralWidget::currentTabChanged);
    connect(mTabbar, &QTabBar::tabCloseRequested, this, &DkCentralWidget::tabCloseRequested);
    connect(mTabbar, &QTabBar::tabMoved, this, &DkCentralWidget::tabMoved);

    connect(this,
            &DkCentralWidget::imageHasGPSSignal,
            DkActionManager::instance().action(DkActionManager::view_gps_map),
            &QAction::setEnabled);

    // preferences
    connect(am.action(DkActionManager::edit_preferences), &QAction::triggered, this, &DkCentralWidget::openPreferences);
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
    DefaultSettings settings;
    settings.beginGroup(objectName());

    int size = settings.beginReadArray("Tabs");
    for (int idx = 0; idx < size; idx++) {
        settings.setArrayIndex(idx);

        QSharedPointer<DkTabInfo> tabInfo = QSharedPointer<DkTabInfo>(new DkTabInfo());
        tabInfo->loadSettings(settings);
        addTab(tabInfo, true);
    }

    if (mTabInfos.empty()) {
        QSharedPointer<DkTabInfo> info = QSharedPointer<DkTabInfo>(new DkTabInfo());
        info->setMode(DkTabInfo::tab_empty);
        addTab(info, true);
    }

    activateTab(mTabbar->count() - 1);
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

    auto tab = mTabInfos.at(idx);
    updateLoader(tab->getImageLoader());

    if (getThumbScrollWidget())
        getThumbScrollWidget()->clear();

    tab->activate();

    switch (tab->getMode()) {
    case DkTabInfo::tab_single_image:
        showViewPort();
        break;
    case DkTabInfo::tab_thumb_preview:
        showThumbView();
        break;
    case DkTabInfo::tab_recent_files:
        showRecentFiles();
        break;
    case DkTabInfo::tab_preferences:
        showPreferences();
        break;
    case DkTabInfo::tab_batch:
        showBatch();
        break;
    case DkTabInfo::tab_empty:
    case DkTabInfo::tab_end:
        break;
    }

    // the tab is fully configured, we can update the text/icon now
    updateTab(tab);
}

void DkCentralWidget::updateLoader(QSharedPointer<DkImageLoader> loader) const
{
    for (int tIdx = 0; tIdx < mTabInfos.size(); tIdx++) {
        QSharedPointer<DkImageLoader> l = mTabInfos.at(tIdx)->getImageLoader();

        if (l != loader)
            mTabInfos.at(tIdx)->deactivate();

        disconnect(loader.data(),
                   QOverload<QSharedPointer<DkImageContainerT>>::of(&DkImageLoader::imageUpdatedSignal),
                   this,
                   &DkCentralWidget::imageLoaded);
        disconnect(loader.data(),
                   QOverload<QSharedPointer<DkImageContainerT>>::of(&DkImageLoader::imageUpdatedSignal),
                   this,
                   &DkCentralWidget::imageUpdatedSignal);
        disconnect(loader.data(), &DkImageLoader::imageHasGPSSignal, this, &DkCentralWidget::imageHasGPSSignal);
        disconnect(loader.data(), &DkImageLoader::updateSpinnerSignalDelayed, this, &DkCentralWidget::showProgress);
        disconnect(loader.data(), &DkImageLoader::loadImageToTab, this, &DkCentralWidget::loadToTab);
    }

    if (!loader)
        return;

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
    connect(loader.data(),
            &DkImageLoader::imageHasGPSSignal,
            this,
            &DkCentralWidget::imageHasGPSSignal,
            Qt::UniqueConnection);
    connect(loader.data(),
            &DkImageLoader::updateSpinnerSignalDelayed,
            this,
            &DkCentralWidget::showProgress,
            Qt::UniqueConnection);
    connect(loader.data(), &DkImageLoader::loadImageToTab, this, &DkCentralWidget::loadToTab, Qt::UniqueConnection);
}

void DkCentralWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void DkCentralWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    mProgressBar->setGeometry(QRect{0, 0, this->width(), mProgressBar->sizeHint().height()});
}

DkPreferenceWidget *DkCentralWidget::createPreferences()
{
    // add preference widget ------------------------------
    auto *pw = new DkPreferenceWidget(this);
    connect(pw, &DkPreferenceWidget::restartSignal, this, &DkCentralWidget::restart, Qt::UniqueConnection);

    // general preferences
    auto *tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/settings.svg"), tr("General"), this);
    auto *gp = new DkGeneralPreference(this);
    connect(gp, &DkGeneralPreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(gp);
    pw->addTabWidget(tab);

    // display preferences
    tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/display.svg"), tr("Display"), this);
    auto *dp = new DkDisplayPreference(this);
    connect(dp, &DkDisplayPreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(dp);
    pw->addTabWidget(tab);

    // file preferences
    tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/dir.svg"), tr("File"), this);
    auto *fp = new DkFilePreference(this);
    connect(fp, &DkFilePreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(fp);
    pw->addTabWidget(tab);

    // file association preferences
    tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/nomacs-bg.svg"), tr("File Associations"), this);
    auto *fap = new DkFileAssociationsPreference(this);
    connect(fap, &DkFileAssociationsPreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(fap);
    pw->addTabWidget(tab);

    // advanced preferences
    tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/advanced-settings.svg"), tr("Advanced"), this);
    auto *ap = new DkAdvancedPreference(this);
    connect(ap, &DkAdvancedPreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(ap);
    pw->addTabWidget(tab);

    // file association preferences
    tab = new DkPreferenceTabWidget(DkImage::loadIcon(":/nomacs/img/sliders.svg"), tr("Editor"), this);
    auto *ep = new DkEditorPreference(this);
    connect(ep, &DkEditorPreference::infoSignal, tab, &DkPreferenceTabWidget::setInfoMessage);
    tab->setWidget(ep);
    pw->addTabWidget(tab);

    // add preference widget ------------------------------

    return pw;
}

DkRecentFilesWidget *DkCentralWidget::createRecentFiles()
{
    auto *rw = new DkRecentFilesWidget(&mThumbLoader, this);
    rw->registerAction(DkActionManager::instance().action(DkActionManager::file_show_recent));

    connect(rw, &DkRecentFilesWidget::loadFileSignal, this, &DkCentralWidget::load);
    connect(rw, &DkRecentFilesWidget::loadDirSignal, this, &DkCentralWidget::load);

    return rw;
}

DkThumbScrollWidget *DkCentralWidget::createThumbScrollWidget()
{
    auto *thumbScrollWidget = new DkThumbScrollWidget(&mThumbLoader, this);
    // thumbScrollWidget->getThumbWidget()->setBackgroundBrush(DkSettingsManager::param().slideShow().backgroundColor);
    thumbScrollWidget->registerAction(DkActionManager::instance().action(DkActionManager::panel_thumbview));

    // thumbnail preview widget
    connect(thumbScrollWidget->getThumbWidget(), &DkThumbScene::loadFileSignal, this, &DkCentralWidget::load);
    connect(thumbScrollWidget, &DkThumbScrollWidget::batchProcessFilesSignal, this, &DkCentralWidget::openBatch);

    return thumbScrollWidget;
}

void DkCentralWidget::createViewPort()
{
    if (hasViewPort()) {
        qDebug() << "viewport already created...";
        return;
    }

    DkViewPort *vp = nullptr;

    if (parent() && parent()->objectName() == "DkNoMacsFrameless")
        vp = new DkViewPortFrameless(&mThumbLoader, this);
    else if (parent() && parent()->objectName() == "DkNoMacsContrast")
        vp = new DkViewPortContrast(&mThumbLoader, this);
    else
        vp = new DkViewPort(&mThumbLoader, this);

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

void DkCentralWidget::addTab(const DkFileInfo &file, bool background)
{
    // image container cannot be constructed from a directory
    if (!file.isFile()) {
        qWarning() << "invalid addTab() with non-file:" << file.path();
        return;
    }

    QSharedPointer<DkImageContainerT> imgC = QSharedPointer<DkImageContainerT>(new DkImageContainerT(file));
    addTab(imgC, background);
}

void DkCentralWidget::addTab(QSharedPointer<DkImageContainerT> imgC, bool background)
{
    QSharedPointer<DkTabInfo> tabInfo = QSharedPointer<DkTabInfo>(new DkTabInfo(imgC));
    addTab(tabInfo, background);
}

void DkCentralWidget::addTab(QSharedPointer<DkTabInfo> tabInfo, bool background)
{
    {
        // prevent currentTabChanged() until we are ready to change tabs
        // this prevents image loading in background tabs
        QSignalBlocker blocker(mTabbar);

        // if the current tab is empty, replace it
        const int currentIdx = mTabbar->currentIndex();
        if (currentIdx >= 0 && mTabInfos.at(currentIdx)->getMode() == DkTabInfo::tab_empty
            && tabInfo->getMode() != DkTabInfo::tab_empty) {
            // don't use removeTab(), it may create a new empty tab
            mTabbar->removeTab(currentIdx);

            tabInfo->setTabIdx(currentIdx);
            mTabInfos.replace(currentIdx, tabInfo);
            updateTabIdx();
            mTabbar->insertTab(currentIdx, tabInfo->getTabText());
        } else {
            tabInfo->setTabIdx(mTabInfos.size());
            mTabInfos.push_back(tabInfo);
            mTabbar->addTab(tabInfo->getTabText());
        }
    }

    if (!background)
        activateTab(tabInfo->getTabIdx());

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
    if (tabIdx == -1) {
        tabIdx = mTabbar->currentIndex();
    }

    // if user requests close on batch while processing - cancel batch
    if (mTabInfos[tabIdx]->getMode() == DkTabInfo::tab_batch) {
        auto *bw = dynamic_cast<DkBatchWidget *>(mWidgets[batch_widget]);

        if (bw)
            bw->close();
    }

    // TODO: ask to save changes here but make sure to abort closeAllTabs

    // must block currentTabChanged() here since we'll see inconsistent
    // state until updateTabIdx() returns. Side benefit, we don't
    // see a refresh when deleting inactive tabs
    bool currentTabWillChange = tabIdx == mTabbar->currentIndex();
    {
        QSignalBlocker blocker(mTabbar);
        mTabInfos.remove(tabIdx);
        mTabbar->removeTab(tabIdx);
        updateTabIdx();
    }

    if (!mTabInfos.empty()) {
        // we blocked the signal so we now manually trip the signal
        if (currentTabWillChange) {
            activateTab(mTabbar->currentIndex());
        }
    } else {
        // make sure we have at least one tab
        addTab();
    }

    if (mTabInfos.size() <= 1) {
        mTabbar->hide();
    }
}

void DkCentralWidget::clearAllTabs()
{
    {
        // block while removing to prevent expensive currentTabChanged()
        QSignalBlocker blocker(mTabbar);
        int count = getTabs().count();
        for (int idx = 0; idx < count; idx++) {
            removeTab();
        }
    }
    currentTabChanged(mTabbar->currentIndex());
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
    Q_UNUSED(img)

    int idx = mTabbar->currentIndex();
    if (idx < 0) {
        qWarning() << "Image loaded but no tab available";
        return;
    }

    QSharedPointer<DkTabInfo> tabInfo = mTabInfos[idx];
    updateTab(tabInfo);
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
        switchWidget(mWidgets[thumbs_widget]);
        tabInfo->activate();

        auto tw = getThumbScrollWidget();
        Q_ASSERT(tw);

        auto imageLoader = tabInfo->getImageLoader();

        tw->getThumbWidget()->setImageLoader(imageLoader);

        if (imageLoader) {
            tw->updateThumbs(imageLoader->getImages());

            auto image = imageLoader->getCurrentImage();
            if (image)
                tw->getThumbWidget()->ensureVisible(image->filePath());
        }

        connect(tw,
                &DkThumbScrollWidget::updateDirSignal,
                tabInfo->getImageLoader().data(),
                &DkImageLoader::loadDirRecursive,
                Qt::UniqueConnection);
        connect(tw,
                &DkThumbScrollWidget::filterChangedSignal,
                tabInfo->getImageLoader().data(),
                &DkImageLoader::setFolderFilter,
                Qt::UniqueConnection);
        emit thumbViewLoadedSignal(tabInfo->getImageLoader().data()->getDirPath());

    } else {
        if (auto tw = getThumbScrollWidget()) {
            disconnect(tw,
                       &DkThumbScrollWidget::updateDirSignal,
                       tabInfo->getImageLoader().data(),
                       &DkImageLoader::loadDirRecursive);
            disconnect(tw,
                       &DkThumbScrollWidget::filterChangedSignal,
                       tabInfo->getImageLoader().data(),
                       &DkImageLoader::setFolderFilter);
        }
    }
}

void DkCentralWidget::showViewPort(bool show /* = true */)
{
    if (show) {
        if (!hasViewPort())
            createViewPort();

        switchWidget(mWidgets[viewport_widget]);

        auto tab = mTabInfos[mTabbar->currentIndex()];

        getViewPort()->setImageLoader(tab->getImageLoader());
        getViewPort()->show();

        tab->getImageLoader()->load(tab->getImage());

    } else if (hasViewPort()) {
        getViewPort()->getController()->getFilePreview()->cancelLoading();
        getViewPort()->deactivate();
        getViewPort()->hide();
    }
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
    // batch wants current tab's directory, must be created before changing tabs
    if (!mWidgets[batch_widget]) {
        mWidgets[batch_widget] = createBatch();
        mViewLayout->insertWidget(batch_widget, mWidgets[batch_widget]);
    }

    // switch to tab if already opened
    for (QSharedPointer<DkTabInfo> tabInfo : mTabInfos) {
        if (tabInfo->getMode() == DkTabInfo::tab_batch) {
            mTabbar->setCurrentIndex(tabInfo->getTabIdx());
            return;
        }
    }

    QSharedPointer<DkTabInfo> info(new DkTabInfo(DkTabInfo::tab_batch, mTabInfos.size()));
    addTab(info);

    auto *bw = dynamic_cast<DkBatchWidget *>(mWidgets[batch_widget]);
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
            // FIXME: createBatch() wants current dir, but there may be no current dir when restoring tabs
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

void DkCentralWidget::tryRestart(const QStringList &args)
{
    auto conn = connect(qApp, &QApplication::lastWindowClosed, [args] {
        qInfo() << "Restarting with args:" << args;
        QStringList tmp;
        tmp << "--nmc-restart";
        tmp << "--nmc-session" << QString::number(DkSettingsManager::param().global().sessionId);
        tmp << args;
        QProcess::startDetached(QApplication::applicationFilePath(), tmp);
    });

    // if all windows close successfully, signal fires
    QApplication::closeAllWindows();

    // closeAllWindows doesn't return until signal fires, safe to disconnect here
    disconnect(conn);
}

void DkCentralWidget::restart() const
{
    // save settings first - since the intention of a restart is often a global settings change
    DkSettingsManager::param().save();

    QStringList args;

    if (getCurrentImage()) {
        args.append(getCurrentImage()->filePath());
    }

    tryRestart(args);
}

void DkCentralWidget::showProgress(bool show, int time)
{
    mProgressBar->setVisibleTimed(show, time);
    mProgressBar->raise();
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
        return getViewPort()->unloadImage();

    return true;
}

QString DkCentralWidget::getCurrentDir() const
{
    QString cDir;

    auto imgC = getCurrentImage();
    if (imgC) {
        DkFileInfo dirInfo(imgC->fileInfo().dirPath());
        if (dirInfo.exists())
            cDir = dirInfo.path();
    }

    if (cDir.isEmpty()) {
        const QStringList recentFiles = DkSettingsManager::param().global().recentFiles;
        if (!recentFiles.isEmpty()) {
            DkFileInfo dirInfo(DkFileInfo(recentFiles.first()).dirPath());
            if (DkUtils::tryExists(dirInfo))
                cDir = dirInfo.path();
        }
    }

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

void DkCentralWidget::load(const QString &path)
{
    if (mTabbar->count() <= 0) {
        qWarning() << "no tab to load into";
        return;
    }

    QSharedPointer<DkTabInfo> tab = mTabInfos[mTabbar->currentIndex()];
    QSharedPointer<DkImageLoader> loader = tab->getImageLoader();

    // if we have changes to the image, always ask to save them
    if (!loader->promptSaveBeforeUnload()) {
        return;
    }

    Q_ASSERT(!loader->isEdited());
    Q_ASSERT(!loader->signalsBlocked());

    DkFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        loader->setCurrentDir(fileInfo);

        if (DkSettingsManager::param().global().openDirShowFirstImage) {
            showViewPort();
        } else {
            showThumbView();
        }
    } else {
        auto img = QSharedPointer<DkImageContainerT>{new DkImageContainerT{fileInfo}};
        loader->setCurrentImage(img);
        showViewPort();
    }

    // must update tab text/icon here in case loading fails
    updateTab(tab);
}

void DkCentralWidget::loadUnique(const QString &path, bool newTab)
{
    if (DkSettingsManager::param().global().checkOpenDuplicates) {
        const DkFileInfo fileInfo(path);
        for (auto &tab : std::as_const(mTabInfos)) {
            const DkFileInfo tabInfo(tab->getFilePath());
            auto imgC = tab->getImage();
            bool edited = imgC && imgC->isEdited();
            bool sameFile = fileInfo == tabInfo;
            bool sameDir = tab->getMode() == DkTabInfo::tab_thumb_preview && fileInfo.isDir()
                && fileInfo.path() == tabInfo.dirPath();
            if (!edited && (sameFile || sameDir)) {
                qInfo() << "Using existing tab for duplicate" << path;
                mTabbar->setCurrentIndex(tab->getTabIdx());
                return;
            }
        }
    }

    if (newTab) {
        loadToTab(path);
    } else {
        load(path);
    }
}

void DkCentralWidget::loadToTab(const QString &path)
{
    // if current tab is empty or non-image just use that instead
    bool useCurrentTab = false;
    if (mTabInfos.count() > 0) {
        useCurrentTab = mTabInfos[mTabbar->currentIndex()]->useForNewImageTab();
    }

    if (!useCurrentTab) {
        QSharedPointer<DkTabInfo> newTab(new DkTabInfo(DkTabInfo::tab_empty));
        addTab(newTab);
    }

    load(path);
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

/** loadUrl() loads a single valid url (probably from drag-drop)
 *  @param newTab: if true, do not replace the currently active image
 */
void DkCentralWidget::loadUrl(const QUrl &url, bool newTab)
{
    if (!url.isValid()) {
        qWarning() << "invalid url:" << url;
        return;
    }

    DkFileInfo fileInfo;

    if (url.isLocalFile()) {
        fileInfo = DkFileInfo(url.toLocalFile());
    } else if (url.scheme() == "vscode-resource") {
        // allow drops from VSCode (i.e. images in README files)
        fileInfo = DkFileInfo(url.path());
    } else if (QNetworkAccessManager().supportedSchemes().contains(url.scheme())) {
        // load a remote url
        if (newTab) {
            QSharedPointer<DkTabInfo> tab(new DkTabInfo(DkTabInfo::tab_empty));
            addTab(tab, false); // must be current tab
        }
        setInfo(tr("Downloading \"%1\"").arg(url.toDisplayString()));
        getCurrentImageLoader()->downloadFile(url);
        return;
    } else {
        qWarning() << "unsupported url:" << url;
    }

    // do not check if the file is valid input or not; the loader already does this
    if (newTab)
        loadToTab(fileInfo.path());
    else
        load(fileInfo.path());
}

void DkCentralWidget::pasteImage()
{
    auto tabInfo = mTabInfos.value(mTabbar->currentIndex());

    if (tabInfo && !tabInfo->useForNewImageTab()) {
        addTab();
    }
    showViewPort();

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
        urls = mimeData->urls();
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
            load(sPath);
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

    if (DkFileInfo(filePath).isFromZip()) {
        setInfo(tr("Sorry, renaming archived files is unsupported."));
        return;
    }

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
    QString newFileName = QInputDialog::getText(this,
                                                tr("Rename: %1").arg(fileInfo.fileName()),
                                                tr("New File Name:"),
                                                QLineEdit::Normal,
                                                baseName,
                                                &ok);

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
        load(fileInfo.absoluteFilePath());
        return;
    }

    load(renamedInfo.absoluteFilePath());
}

void DkCentralWidget::activateTab(int tabIdx)
{
    // make a tab active and guarantee signal delivery as
    // setCurrentIndex is noop when index doesn't change
    Q_ASSERT(tabIdx >= 0 && tabIdx < mTabbar->count());

    // if signals blocked assume we don't want anything to happen (closeAllTabs etc)
    if (mTabbar->signalsBlocked()) {
        return;
    }

    if (tabIdx == mTabbar->currentIndex()) {
        currentTabChanged(tabIdx);
    } else {
        mTabbar->setCurrentIndex(tabIdx);
    }
}

} // namespace nmc
