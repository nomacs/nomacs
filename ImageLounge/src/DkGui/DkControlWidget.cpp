/*******************************************************************************************************
 DkControlWidget.cpp
 Created on:	31.08.2015

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

#include "DkControlWidget.h"

#include "DkActionManager.h"
#include "DkMessageBox.h"
#include "DkMetaData.h"
#include "DkMetaDataWidgets.h"
#include "DkPluginManager.h"
#include "DkSettings.h"
#include "DkThumbsWidgets.h"
#include "DkToolbars.h"
#include "DkViewPort.h"
#include "DkWidgets.h"

#include <QGridLayout>
#include <QMouseEvent>
#include <QSharedPointer>
#include <QStackedLayout>

namespace nmc
{
// DkControlWidget --------------------------------------------------------------------
DkControlWidget::DkControlWidget(DkThumbLoader *thumbLoader, DkViewPort *parent, Qt::WindowFlags flags)
    : DkWidget(parent, flags)
{
    mViewport = parent;
    setObjectName("DkControlWidget");

    // cropping
    // TODO: add lazy initialization here
    mCropWidget = new DkCropWidget(QRectF(), this);

    // thumbnails, metadata
    mFilePreview = new DkFilePreview(thumbLoader, this, flags);
    mMetaDataInfo = new DkMetaDataHUD(this);
    mZoomWidget = new DkZoomWidget(this);
    mPlayer = new DkPlayer(this);
    mPlayer->setMaximumHeight(90);

    mFolderScroll = new DkFolderScrollBar(this);

    // brief file info + ratingR
    mFileInfoLabel = new DkFileInfoLabel(this);

    // notes
    mCommentWidget = new DkCommentWidget(this);

    // delayed info
    mDelayedInfo = new DkDelayedMessage(this); // TODO: make a nice constructor

    // info labels
    mBottomLabel = new DkLabelBg("", this);
    mBottomLeftLabel = new DkLabelBg("", this);

    // wheel label
    QPixmap wp = QPixmap(":/nomacs/img/thumbs-move.svg");

    mWheelButton = new QLabel(this);
    mWheelButton->setAttribute(Qt::WA_TransparentForMouseEvents);
    mWheelButton->setPixmap(wp);
    mWheelButton->adjustSize();
    mWheelButton->hide();

    // image histogram
    mHistogram = new DkHistogram(this);

    init();
    connectWidgets();

    // add mousetracking to all widgets which allows the mViewport for status updates
    QObjectList widgets = children();
    for (int idx = 0; idx < widgets.size(); idx++) {
        if (QWidget *w = qobject_cast<QWidget *>(widgets.at(idx))) {
            w->setMouseTracking(true);
        }
    }
}

void DkControlWidget::init()
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    // connect widgets with their settings
    mFilePreview->setDisplaySettings(&DkSettingsManager::param().app().showFilePreview);
    mMetaDataInfo->setDisplaySettings(&DkSettingsManager::param().app().showMetaData);
    mFileInfoLabel->setDisplaySettings(&DkSettingsManager::param().app().showFileInfoLabel);
    mPlayer->setDisplaySettings(&DkSettingsManager::param().app().showPlayer);
    mHistogram->setDisplaySettings(&DkSettingsManager::param().app().showHistogram);
    mCommentWidget->setDisplaySettings(&DkSettingsManager::param().app().showComment);
    mZoomWidget->setDisplaySettings(&DkSettingsManager::param().app().showOverview);
    mFolderScroll->setDisplaySettings(&DkSettingsManager::param().app().showScroller);

    // some adjustments
    mBottomLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mBottomLeftLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mZoomWidget->setContentsMargins(10, 10, 0, 0);
    mCropWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mCommentWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    // register actions
    DkActionManager &am = DkActionManager::instance();
    mFilePreview->registerAction(am.action(DkActionManager::menu_panel_preview));
    mMetaDataInfo->registerAction(am.action(DkActionManager::menu_panel_exif));
    mPlayer->registerAction(am.action(DkActionManager::menu_panel_player));
    mCropWidget->registerAction(am.action(DkActionManager::menu_edit_crop));
    mFileInfoLabel->registerAction(am.action(DkActionManager::menu_panel_info));
    mHistogram->registerAction(am.action(DkActionManager::menu_panel_histogram));
    mCommentWidget->registerAction(am.action(DkActionManager::menu_panel_comment));
    mFolderScroll->registerAction(am.action(DkActionManager::menu_panel_scroller));

    // dummy - needed for three equal columns @markus: do not delete!
    auto *dw = new QWidget(this);
    dw->setMouseTracking(true);
    auto *dLayout = new QBoxLayout(QBoxLayout::LeftToRight, dw);
    dLayout->setContentsMargins(0, 0, 0, 0);
    dLayout->addWidget(mBottomLabel);
    dLayout->addStretch();

    // zoom widget
    auto *bw = new QWidget(this);
    bw->setMouseTracking(true);
    // bw->setMinimumHeight(40);
    // bw->setMaximumHeight(80);
    auto *zLayout = new QVBoxLayout(bw);
    zLayout->setAlignment(Qt::AlignBottom);
    zLayout->setContentsMargins(0, 0, 0, 20);
    zLayout->setSpacing(0);
    zLayout->addWidget(mBottomLabel);
    zLayout->addWidget(mBottomLeftLabel);
    zLayout->addWidget(mCommentWidget);

    // left column widget
    auto *leftWidget = new QWidget(this);
    leftWidget->setMouseTracking(true);
    auto *ulLayout = new QBoxLayout(QBoxLayout::TopToBottom, leftWidget);
    ulLayout->setContentsMargins(0, 0, 0, 0);
    ulLayout->setSpacing(0);
    ulLayout->addWidget(mZoomWidget);
    ulLayout->addStretch();
    ulLayout->addWidget(bw);
    ulLayout->addWidget(dw);

    // center column
    auto *center = new QWidget(this);
    center->setMouseTracking(true);
    auto *cLayout = new QVBoxLayout(center);
    cLayout->setContentsMargins(0, 0, 0, 0);
    cLayout->setAlignment(Qt::AlignBottom);
    cLayout->addWidget(mPlayer);

    // rating widget
    auto *rw = new QWidget(this);
    rw->setMouseTracking(true);
    rw->setMinimumSize(0, 0);
    auto *rLayout = new QBoxLayout(QBoxLayout::RightToLeft, rw);
    rLayout->setContentsMargins(0, 0, 0, 17);
    rLayout->addStretch();

    // file info
    auto *fw = new QWidget(this);
    fw->setContentsMargins(0, 0, 0, 30);
    fw->setMouseTracking(true);
    fw->setMinimumSize(0, 0);
    auto *rwLayout = new QBoxLayout(QBoxLayout::RightToLeft, fw);
    rwLayout->setContentsMargins(0, 0, 0, 0);
    rwLayout->addWidget(mFileInfoLabel);
    rwLayout->addStretch();

    // right column
    auto *hw = new QWidget(this);
    hw->setContentsMargins(0, 10, 10, 0);
    hw->setMouseTracking(true);
    auto *hwLayout = new QBoxLayout(QBoxLayout::RightToLeft, hw);
    hwLayout->setContentsMargins(0, 0, 0, 0);
    hwLayout->addWidget(mHistogram);
    hwLayout->addStretch();

    // right column
    auto *rightWidget = new QWidget(this);
    rightWidget->setMouseTracking(true);
    auto *lrLayout = new QBoxLayout(QBoxLayout::TopToBottom, rightWidget);
    lrLayout->setContentsMargins(0, 0, 0, 0);
    lrLayout->addWidget(hw);
    lrLayout->addStretch();
    lrLayout->addWidget(fw);
    lrLayout->addWidget(rw);

    // init main widgets
    mWidgets.resize(widget_end);
    mWidgets[hud_widget] = new QWidget(this);
    mWidgets[crop_widget] = mCropWidget;

    // global controller layout
    mHudLayout = new QGridLayout(mWidgets[hud_widget]);
    mHudLayout->setContentsMargins(0, 0, 0, 0);
    mHudLayout->setSpacing(0);

    // hudLayout->addWidget(thumbWidget, 0, 0);

    // add elements
    changeThumbNailPosition(mFilePreview->getWindowPosition());
    changeMetaDataPosition(mMetaDataInfo->getWindowPosition());
    // hudLayout->addWidget(filePreview, top_thumbs, left_thumbs, 1, hor_pos_end);
    mHudLayout->addWidget(leftWidget, ver_center, left, 1, 1);
    mHudLayout->addWidget(center, ver_center, hor_center, 1, 1);
    mHudLayout->addWidget(rightWidget, ver_center, right, 1, 1);
    mHudLayout->addWidget(mFolderScroll, top_scroll, left_thumbs, 1, hor_pos_end);

    //// we need to put everything into extra widgets (which are exclusive) in order to handle the mouse events
    /// correctly
    // QHBoxLayout* editLayout = new QHBoxLayout(widgets[crop_widget]);
    // editLayout->setContentsMargins(0,0,0,0);
    // editLayout->addWidget(cropWidget);

    mLayout = new QStackedLayout(this);
    mLayout->setContentsMargins(0, 0, 0, 0);

    for (int idx = 0; idx < mWidgets.size(); idx++)
        mLayout->addWidget(mWidgets[idx]);

    //// debug code...
    // centerLabel->setText("ich bin richtig...", -1);
    // bottomLeftLabel->setText("topLeft label...", -1);
    // spinnerLabel->show();

    show();
}

void DkControlWidget::connectWidgets()
{
    if (!mViewport)
        return;

    // thumbs widget
    connect(mFilePreview,
            &DkFilePreview::loadFileSignal,
            mViewport,
            QOverload<const QString &>::of(&DkViewPort::loadFile));
    connect(mFilePreview, &DkFilePreview::changeFileSignal, mViewport, &DkViewPort::loadFileFast);
    connect(mFilePreview, &DkFilePreview::positionChangeSignal, this, &DkControlWidget::changeThumbNailPosition);

    // metadata widget
    connect(mMetaDataInfo, &DkMetaDataHUD::positionChangeSignal, this, &DkControlWidget::changeMetaDataPosition);

    // overview
    connect(mZoomWidget->getOverview(), &DkOverview::moveViewSignal, mViewport, &DkViewPort::moveView);
    connect(mZoomWidget->getOverview(), &DkOverview::sendTransformSignal, mViewport, [this]() {
        mViewport->tcpSynchronize();
    });

    // zoom widget
    connect(mZoomWidget, &DkZoomWidget::zoomSignal, mViewport, &DkViewPort::zoomTo);
    connect(mViewport, &DkViewPort::zoomSignal, mZoomWidget, &DkZoomWidget::updateZoom);

    // waiting
    connect(mDelayedInfo, &DkDelayedMessage::infoSignal, this, [this](const QString &msg, int time) {
        setInfo(msg, time);
    });

    // rating
    connect(mFileInfoLabel, &DkFileInfoLabel::ratingEdited, this, &DkControlWidget::updateRating);

    // playing
    connect(mPlayer, &DkPlayer::previousSignal, mViewport, &DkViewPort::loadPrevFileFast);
    connect(mPlayer, &DkPlayer::nextSignal, mViewport, &DkViewPort::loadNextFileFast);
    connect(mPlayer, &DkPlayer::loadFileSignal, mViewport, [this](const QString &filePath) {
        mViewport->loadFile(filePath);
    });

    // cropping
    connect(mCropWidget, &DkCropWidget::cropImageSignal, mViewport, &DkViewPort::cropImage);
    connect(mCropWidget, &DkCropWidget::hideSignal, this, [this]() {
        hideCrop();
    });

    // comment widget
    connect(mCommentWidget, &DkCommentWidget::commentSavedSignal, this, &DkControlWidget::setCommentSaved);

    // mViewport
    connect(mViewport, &DkViewPort::infoSignal, this, [this](const QString &msg) {
        setInfo(msg);
    });

    DkActionManager &am = DkActionManager::instance();

    // plugins
    if (am.pluginActionManager()) {
        connect(am.pluginActionManager(),
                QOverload<DkViewPortInterface *, bool>::of(&DkPluginActionManager::runPlugin),
                this,
                &DkControlWidget::setPluginWidget);
        connect(am.pluginActionManager(),
                &DkPluginActionManager::applyPluginChanges,
                this,
                &DkControlWidget::applyPluginChanges);
    }

    // actions
    connect(am.action(DkActionManager::menu_edit_crop), &QAction::triggered, this, &DkControlWidget::showCrop);
    connect(am.action(DkActionManager::menu_panel_overview), &QAction::toggled, this, &DkControlWidget::showOverview);
    connect(am.action(DkActionManager::menu_panel_player), &QAction::toggled, this, &DkControlWidget::showPlayer);
    connect(am.action(DkActionManager::menu_panel_preview), &QAction::toggled, this, &DkControlWidget::showPreview);
    connect(am.action(DkActionManager::menu_panel_scroller), &QAction::toggled, this, &DkControlWidget::showScroller);
    connect(am.action(DkActionManager::menu_panel_exif), &QAction::toggled, this, &DkControlWidget::showMetaData);
    connect(am.action(DkActionManager::menu_panel_info), &QAction::toggled, this, &DkControlWidget::showFileInfo);
    connect(am.action(DkActionManager::menu_panel_histogram), &QAction::toggled, this, &DkControlWidget::showHistogram);
    connect(am.action(DkActionManager::menu_panel_comment),
            &QAction::toggled,
            this,
            &DkControlWidget::showCommentWidget);
    connect(am.action(DkActionManager::menu_panel_toggle), &QAction::toggled, this, &DkControlWidget::toggleHUD);
}

void DkControlWidget::setCommentSaved(const QString &comment)
{
    const auto metaData = mViewport->imageContainer()->getMetaData();
    if (!metaData || comment == metaData->getDescription()) {
        return;
    }

    if (!metaData->setDescription(comment) && !comment.isEmpty()) {
        setInfo(tr("Sorry, I cannot save comments for this image format."));
        return;
    }
    mViewport->imageContainer()->setMetaData(tr("File comment"));
}

void DkControlWidget::update()
{
    mZoomWidget->update();

    QWidget::update();
}

void DkControlWidget::showWidgetsSettings()
{
    if (mViewport->getImage().isNull()) {
        showPreview(false);
        showScroller(false);
        showMetaData(false);
        showFileInfo(false);
        showPlayer(false);
        showOverview(false);
        showHistogram(false);
        showCommentWidget(false);
        return;
    }

    // do not show if hide panels is checked
    if (DkActionManager::instance().action(DkActionManager::menu_panel_toggle)->isChecked())
        return;

    // qDebug() << "current app mode: " << DkSettingsManager::param().app().currentAppMode;

    showOverview(mZoomWidget->getCurrentDisplaySetting());
    showPreview(mFilePreview->getCurrentDisplaySetting());
    showMetaData(mMetaDataInfo->getCurrentDisplaySetting());
    showFileInfo(mFileInfoLabel->getCurrentDisplaySetting());
    showHistogram(mHistogram->getCurrentDisplaySetting());
    showCommentWidget(mCommentWidget->getCurrentDisplaySetting());
    showScroller(mFolderScroll->getCurrentDisplaySetting());

    // don't show player while playing and switching modes
    if (!mPlayer->isPlaying())
        showPlayer(mPlayer->getCurrentDisplaySetting());
}

void DkControlWidget::toggleHUD(bool hide)
{
    if (hide)
        setWidgetsVisible(false, false);
    else
        showWidgetsSettings();
}

void DkControlWidget::setWidgetsVisible(bool visible, bool saveSettings)
{
    mFilePreview->setVisible(visible, saveSettings);
    mFolderScroll->setVisible(visible, saveSettings);
    mMetaDataInfo->setVisible(visible, saveSettings);
    mFileInfoLabel->setVisible(visible, saveSettings);
    mPlayer->setVisible(visible, saveSettings);
    mZoomWidget->setVisible(visible, saveSettings);
    mHistogram->setVisible(visible, saveSettings);
    mCommentWidget->setVisible(visible, saveSettings);
}

void DkControlWidget::showPreview(bool visible)
{
    if (!mFilePreview)
        return;

    if (visible && !mFilePreview->isVisible())
        mFilePreview->show();
    else if (!visible && mFilePreview->isVisible())
        mFilePreview->hide(!mViewport->getImage().isNull()); // do not save settings if we have no image in the viewport
}

void DkControlWidget::showScroller(bool visible)
{
    if (!mFolderScroll)
        return;

    if (visible && !mFolderScroll->isVisible())
        mFolderScroll->show();
    else if (!visible && mFolderScroll->isVisible())
        mFolderScroll->hide(
            !mViewport->getImage().isNull()); // do not save settings if we have no image in the viewport
}

void DkControlWidget::showMetaData(bool visible)
{
    if (!mMetaDataInfo)
        return;

    if (visible && !mMetaDataInfo->isVisible()) {
        mMetaDataInfo->show();
        qDebug() << "showing metadata...";
    } else if (!visible && mMetaDataInfo->isVisible())
        mMetaDataInfo->hide(
            !mViewport->getImage().isNull()); // do not save settings if we have no image in the viewport
}

void DkControlWidget::showFileInfo(bool visible)
{
    if (!mFileInfoLabel)
        return;

    if (visible && !mFileInfoLabel->isVisible())
        mFileInfoLabel->show();
    else if (!visible && mFileInfoLabel->isVisible())
        mFileInfoLabel->hide(
            !mViewport->getImage().isNull()); // do not save settings if we have no image in the viewport
}

void DkControlWidget::showPlayer(bool visible)
{
    if (!mPlayer)
        return;

    if (visible)
        mPlayer->show();
    else
        mPlayer->hide(!mViewport->getImage().isNull()); // do not save settings if we have no image in the viewport
}

void DkControlWidget::startSlideshow(bool start)
{
    mPlayer->play(start);
}

void DkControlWidget::showOverview(bool visible)
{
    if (!mZoomWidget)
        return;

    if (visible && !mZoomWidget->isVisible()) {
        mZoomWidget->show();
    } else if (!visible && mZoomWidget->isVisible()) {
        mZoomWidget->hide(!mViewport->getImage().isNull()); // do not save settings if we have no image in the mViewport
    }
}

void DkControlWidget::hideCrop(bool hide /* = true */)
{
    showCrop(!hide);
}

void DkControlWidget::showCrop(bool visible)
{
    if (visible) {
        mCropWidget->reset();
        switchWidget(mWidgets[crop_widget]);
        connect(mCropWidget->getToolbar(), &DkCropToolBar::colorSignal, mViewport, &DkViewPort::setBackgroundBrush);
    } else
        switchWidget();
}

void DkControlWidget::showHistogram(bool visible)
{
    if (!mHistogram)
        return;

    if (visible && !mHistogram->isVisible()) {
        mHistogram->show();
        if (!mViewport->getImage().isNull())
            mHistogram->drawHistogram(mViewport->getImage());
        else
            mHistogram->clearHistogram();
    } else if (!visible && mHistogram->isVisible()) {
        mHistogram->hide(!mViewport->getImage().isNull()); // do not save settings if we have no image in the mViewport
    }
}

void DkControlWidget::showCommentWidget(bool visible)
{
    if (!mCommentWidget)
        return;

    if (visible && !mCommentWidget->isVisible()) {
        mCommentWidget->show();
    } else if (!visible && mCommentWidget->isVisible()) {
        mCommentWidget->hide(
            !mViewport->getImage().isNull()); // do not save settings if we have no image in the mViewport
    }
}

void DkControlWidget::switchWidget(QWidget *widget)
{
    if (mLayout->currentWidget() == widget || (!widget && mLayout->currentWidget() == mWidgets[hud_widget]))
        return;

    if (widget != mWidgets[hud_widget])
        applyPluginChanges(true);

    if (widget)
        mLayout->setCurrentWidget(widget);
    else
        mLayout->setCurrentWidget(mWidgets[hud_widget]);

    // if (mLayout->currentWidget())
    //	qDebug() << "changed to widget: " << mLayout->currentWidget();
}

void DkControlWidget::pluginClosed(bool askForSaving)
{
    (void)closePlugin(askForSaving);
}

void DkControlWidget::pluginMessage(const QString &msg)
{
    setInfo(msg);
}

void DkControlWidget::pluginLoadFile(const QString &path)
{
    mViewport->loadFile(path);
}

void DkControlWidget::pluginLoadImage(const QImage &img)
{
    mViewport->setImage(img);
}

bool DkControlWidget::closePlugin(bool askForSaving, bool force)
{
#ifdef WITH_PLUGINS

    QSharedPointer<DkPluginContainer> plugin = DkPluginManager::instance().getRunningPlugin();

    if (!plugin)
        return true;

    DkViewPortInterface *vPlugin = plugin->pluginViewPort();

    if (!vPlugin)
        return true;

    // this is that complicated because we do not want plugins to have threaded containers - this could get weird
    QSharedPointer<DkImageContainerT> pluginImage;

    if (!force) {
        if (mViewport->imageContainer()) {
            bool applyChanges = true;

            if (askForSaving) {
                auto *msgBox = new DkMessageBox(QMessageBox::Question,
                                                tr("Closing Plugin"),
                                                tr("Apply plugin changes?"),
                                                QMessageBox::Yes | QMessageBox::No,
                                                this);
                msgBox->setDefaultButton(QMessageBox::Yes);
                msgBox->setObjectName("SavePluginChanges");

                int answer = msgBox->exec();
                applyChanges = (answer == QMessageBox::Accepted || answer == QMessageBox::Yes);
            }

            if (applyChanges)
                pluginImage = DkImageContainerT::fromImageContainer(
                    vPlugin->runPlugin("", mViewport->imageContainer()));
        } else
            qDebug() << "[DkControlWidget] I cannot close a plugin if the image container is NULL";
    }

    setPluginWidget(vPlugin, true);
    plugin->setActive(false); // handles states

    if (!force && pluginImage) {
        mViewport->setEditedImage(pluginImage);
        return false;
    }

    return true;
#else
    Q_UNUSED(askForSaving);
    Q_UNUSED(force);

    return false;
#endif // WITH_PLUGINS
}

bool DkControlWidget::applyPluginChanges(bool askForSaving)
{
#ifdef WITH_PLUGINS
    QSharedPointer<DkPluginContainer> plugin = DkPluginManager::instance().getRunningPlugin();

    if (!plugin)
        return true;

    // does the plugin want to be closed on image changes?
    if (!plugin->plugin()->closesOnImageChange())
        return true;

    return closePlugin(askForSaving);
#else
    Q_UNUSED(askForSaving);

    return true;
#endif // WITH_PLUGINS
}

void DkControlWidget::setPluginWidget(DkViewPortInterface *pluginWidget, bool removeWidget)
{
    mPluginViewport = pluginWidget->getViewPort();

    if (!mPluginViewport) {
        qDebug() << "cannot set plugin widget since the viewport is NULL" << pluginWidget;
        return;
    }

    // workaround some plugin bugs by disabling actions while the plugin is open
    // TODO: this belongs in central widget as action enablement is tied to tabs
    DkActionManager::instance().enableViewPortPluginActions(removeWidget);

    if (!removeWidget) {
        mPluginViewport->setWorldMatrix(mViewport->getWorldMatrixPtr());
        mPluginViewport->setImgMatrix(mViewport->getImageMatrixPtr());
        mPluginViewport->updateImageContainer(mViewport->imageContainer());

        // NOTE: unique connections can no longer use lambdas, this was unreliable
        // in practice and is now a fatal error in debug builds of nomacs
        connect(mPluginViewport,
                &DkPluginViewPort::closePlugin,
                this,
                &DkControlWidget::pluginClosed,
                Qt::UniqueConnection);
        connect(mPluginViewport,
                &DkPluginViewPort::loadFile,
                this,
                &DkControlWidget::pluginLoadFile,
                Qt::UniqueConnection);

        // TODO: will this copy without using reference?
        connect(mPluginViewport,
                &DkPluginViewPort::loadImage,
                this,
                &DkControlWidget::pluginLoadImage,
                Qt::UniqueConnection);
        connect(mPluginViewport,
                &DkPluginViewPort::showInfo,
                this,
                &DkControlWidget::pluginMessage,
                Qt::UniqueConnection);
    }

    setAttribute(Qt::WA_TransparentForMouseEvents, !removeWidget && pluginWidget->hideHUD());

    if (pluginWidget->hideHUD() && !removeWidget) {
        setWidgetsVisible(false, false);
    } else if (pluginWidget->hideHUD())
        showWidgetsSettings();

    mViewport->setPaintWidget(dynamic_cast<QWidget *>(mPluginViewport), removeWidget);

    if (removeWidget) {
        mPluginViewport = nullptr;
    }
}

void DkControlWidget::updateImage(QSharedPointer<DkImageContainerT> imgC)
{
    if (mImgC) {
        disconnect(mImgC.get(),
                   &DkImageContainerT::imageUpdatedSignal,
                   this,
                   &DkControlWidget::onImageContainerInternalUpdated);
    }

    mImgC = imgC;

    if (mPluginViewport)
        mPluginViewport->updateImageContainer(imgC);

    if (!imgC) {
        mMetaDataInfo->setMetaData(nullptr);
        return;
    }

    onImageContainerInternalUpdated();
    connect(imgC.get(),
            &DkImageContainerT::imageUpdatedSignal,
            this,
            &DkControlWidget::onImageContainerInternalUpdated);
}

void DkControlWidget::setInfo(const QString &msg, int time, int location)
{
    if (location == bottom_left_label && mBottomLabel)
        mBottomLabel->setText(msg, time);
    else if (location == top_left_label && mBottomLeftLabel)
        mBottomLeftLabel->setText(msg, time);

    update();
}

void DkControlWidget::changeMetaDataPosition(int pos)
{
    if (pos == DkFadeWidget::pos_west) {
        mHudLayout->addWidget(mMetaDataInfo, top_metadata, left_metadata, bottom_metadata - top_metadata, 1);
    } else if (pos == DkFadeWidget::pos_east) {
        mHudLayout->addWidget(mMetaDataInfo, top_metadata, right_metadata, bottom_metadata - top_metadata, 1);
    } else if (pos == DkFadeWidget::pos_north) {
        mHudLayout->addWidget(mMetaDataInfo, top_metadata, left_metadata, 1, hor_pos_end - 2);
    } else if (pos == DkFadeWidget::pos_south) {
        mHudLayout->addWidget(mMetaDataInfo, bottom_metadata, left_metadata, 1, hor_pos_end - 2);
    }
}

void DkControlWidget::changeThumbNailPosition(int pos)
{
    if (pos == DkFadeWidget::pos_west) {
        mHudLayout->addWidget(mFilePreview, top_thumbs, left_thumbs, ver_pos_end, 1);
    } else if (pos == DkFadeWidget::pos_east) {
        mHudLayout->addWidget(mFilePreview, top_thumbs, right_thumbs, ver_pos_end, 1);
    } else if (pos == DkFadeWidget::pos_north) {
        mHudLayout->addWidget(mFilePreview, top_thumbs, left_thumbs, 1, hor_pos_end);
    } else if (pos == DkFadeWidget::pos_south) {
        mHudLayout->addWidget(mFilePreview, bottom_thumbs, left_thumbs, 1, hor_pos_end);
    } else
        mFilePreview->hide();
}

void DkControlWidget::settingsChanged()
{
    if (mFileInfoLabel && mFileInfoLabel->isVisible()) {
        showFileInfo(false); // just a hack but all states are preserved this way
        showFileInfo(true);
    }
}

void DkControlWidget::updateRating(int rating)
{
    if (!mImgC) {
        return;
    }

    mImgC->setRating(rating);
}

void DkControlWidget::imagePresenceChanged(bool imagePresent)
{
    (void)imagePresent;

    // disable animations while building initial view or image is lost
    DkFadeHelper::enableAnimations(false);

    showWidgetsSettings();

    DkFadeHelper::enableAnimations(true);
}

void DkControlWidget::setFullScreen(bool fullscreen)
{
    showWidgetsSettings();

    if (DkSettingsManager::param().slideShow().showPlayer && fullscreen && !mPlayer->getCurrentDisplaySetting()
        && !mPlayer->isPlaying())
        mPlayer->showTemporarily();
}

DkOverview *DkControlWidget::getOverview() const
{
    return mZoomWidget->getOverview();
}

DkZoomWidget *DkControlWidget::getZoomWidget() const
{
    return mZoomWidget;
}

DkPlayer *DkControlWidget::getPlayer() const
{
    return mPlayer;
}

DkFileInfoLabel *DkControlWidget::getFileInfoLabel() const
{
    return mFileInfoLabel;
}

DkHistogram *DkControlWidget::getHistogram() const
{
    return mHistogram;
}

DkCropWidget *DkControlWidget::getCropWidget() const
{
    return mCropWidget;
}

DkFilePreview *DkControlWidget::getFilePreview() const
{
    return mFilePreview;
}

DkFolderScrollBar *DkControlWidget::getScroller() const
{
    return mFolderScroll;
}

// DkControlWidget - Events --------------------------------------------------------------------
void DkControlWidget::mousePressEvent(QMouseEvent *event)
{
    mEnterPos = event->pos();

    if (mFilePreview && mFilePreview->isVisible() && event->buttons() == Qt::MiddleButton) {
        QTimer *mImgTimer = mFilePreview->getMoveImageTimer();
        mImgTimer->start(1);

        // show icon
        mWheelButton->move(event->pos().x() - 16, event->pos().y() - 16);
        mWheelButton->show();
    }

    if (mPluginViewport)
        QCoreApplication::sendEvent(mPluginViewport, event);
    else
        QWidget::mousePressEvent(event);
}

void DkControlWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (mFilePreview && mFilePreview->isVisible()) {
        mFilePreview->setCurrentDx(0);
        QTimer *mImgTimer = mFilePreview->getMoveImageTimer();
        mImgTimer->stop();
        mWheelButton->hide();
    }

    if (mPluginViewport)
        QCoreApplication::sendEvent(mPluginViewport, event);
    else
        QWidget::mouseReleaseEvent(event);
}

void DkControlWidget::mouseMoveEvent(QMouseEvent *event)
{
    // scroll thumbs preview
    if (mFilePreview && mFilePreview->isVisible() && event->buttons() == Qt::MiddleButton) {
        float dx = (float)std::fabs(mEnterPos.x() - event->pos().x()) * 0.015f;
        dx = std::exp(dx);
        if (mEnterPos.x() - event->pos().x() < 0)
            dx = -dx;

        mFilePreview->setCurrentDx(dx); // update dx
    }

    if (mPluginViewport)
        QCoreApplication::sendEvent(mPluginViewport, event);
    else
        QWidget::mouseMoveEvent(event);
}

void DkControlWidget::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void DkControlWidget::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);
}

void DkControlWidget::onImageContainerInternalUpdated()
{
    // This is called on a signal from a valid DkImageContainerT,
    // meaning mImgC should always be not null.
    Q_ASSERT(mImgC);

    const auto metaData = mImgC->getMetaData();
    mMetaDataInfo->setMetaData(metaData);

    if (!metaData) {
        return;
    }

    mCommentWidget->setText(metaData->getDescription());

    QString dateString = metaData->getExifValue("DateTimeOriginal");
    mFileInfoLabel->updateInfo(mImgC->filePath(), dateString, metaData->getRating(), mImgC->isEdited());
    mCommentWidget->setText(metaData->getDescription()); // reset
}
}
