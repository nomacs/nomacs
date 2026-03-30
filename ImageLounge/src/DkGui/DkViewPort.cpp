/*******************************************************************************************************
 DkViewPort.cpp
 Created on:	05.05.2011

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

#include "DkViewPort.h"

#include "DkActionManager.h"
#include "DkControlWidget.h"
#include "DkDialog.h"
#include "DkImageLoader.h"
#include "DkMessageBox.h"
#include "DkMetaData.h"
#include "DkMetaDataWidgets.h"
#include "DkNetwork.h"
#include "DkPluginManager.h"
#include "DkSettings.h"
#include "DkStatusBar.h"
#include "DkThumbsWidgets.h" // needed in the connects -> shall we move them to mController?
#include "DkToolbars.h"
#include "DkUtils.h"
#include "DkViewPortFSViewModel.h"
#include "DkViewPortImageViewModel.h"
#include "DkViewPortTransformViewModel.h"
#include "DkWidgets.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColorSpace>
#include <QDrag>
#include <QDragLeaveEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QMovie>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QVBoxLayout>
#include <QtConcurrentRun>
#include <cmath>
#include <memory>
#include <qassert.h>
#include <qobject.h>
#include <utility>

#ifdef WITH_OPENCV
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#endif

namespace nmc
{
// DkViewPort --------------------------------------------------------------------
DkViewPort::DkViewPort(DkThumbLoader *thumbLoader, QWidget *parent, bool resetWhenZoomPastFit)
    : DkBaseViewPort(false, parent, resetWhenZoomPastFit)
    , mFSVM{std::make_unique<DkViewPortFSViewModel>()}
{
    mRepeatZoomTimer = new QTimer(this);
    mAnimationTimer = new QTimer(this);

    // try loading a custom file
    mImgBg.load(QFileInfo(QApplication::applicationDirPath(), "bg.png").absoluteFilePath());
    if (mImgBg.isNull() && DkSettingsManager::param().global().showLogoImage) {
        QColor col = Qt::black;
        col.setAlpha(90);
        mImgBg = DkImage::loadIcon(":/nomacs/img/nomacs-bg.svg", col).pixmap(80).toImage();
        mImgBg.setDevicePixelRatio(1.0); // handle device scaling ourselves
    }

    mRepeatZoomTimer->setInterval(20);
    connect(mRepeatZoomTimer, &QTimer::timeout, this, &DkViewPort::repeatZoom);

    mAnimationTimer->setInterval(5);
    connect(mAnimationTimer, &QTimer::timeout, this, &DkViewPort::animateFade);

    // no border
    setMouseTracking(true); // receive mouse event everytime

    mPaintLayout = new QVBoxLayout(this);
    mPaintLayout->setContentsMargins(0, 0, 0, 0);

    createShortcuts();

    mController = new DkControlWidget(thumbLoader, this);

    connectLoader();

    if (DkSettingsManager::param().display().showScrollBars) {
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

    mController->getOverview()->setViewPort(this);
    connect(transformVM(), &DkViewPortTransformViewModel::transformChanged, this, [this]() {
        const auto vm = transformVM();
        auto cw = mController->getCropWidget();
        cw->setWorldTransform(vm->worldMatrix());
        cw->setImageTransform(vm->imgMatrix());
        cw->setImageRect(vm->imgViewRect());
    });

    // nav buttons initialized after mController to place them above all other hud widgets
    QSize s(64, 64);
    QColor c(0, 0, 0);
    c.setAlpha(0);

    mPrevButton = new DkFadeButton(DkImage::loadIcon(":/nomacs/img/previous-hud.svg", s, c), "", this);
    mPrevButton->setObjectName("hudNavigationButton");
    mPrevButton->setToolTip(tr("Show previous image"));
    mPrevButton->setFlat(true);
    mPrevButton->setIconSize(s);

    mNextButton = new DkFadeButton(DkImage::loadIcon(":/nomacs/img/next-hud.svg", s, c), "", this);
    mNextButton->setObjectName("hudNavigationButton");
    mNextButton->setToolTip(tr("Show next image"));
    mNextButton->setFlat(true);
    mNextButton->setIconSize(s);

    auto *navLayout = new QHBoxLayout(this);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->addWidget(mPrevButton);
    navLayout->addStretch();
    navLayout->addWidget(mNextButton);
    mPaintLayout->addLayout(navLayout);

    // add actions that cannot be found in the main menu
    DkActionManager &am = DkActionManager::instance();
    addActions(am.hiddenActions().toList());
    addActions(am.openWithActions().toList());

    connect(imageVM(), &DkViewPortImageViewModel::antiAliasingChanged, this, [this](bool antiAliasing) {
        emit infoSignal(antiAliasing ? tr("Anti Aliasing Enabled") : tr("Anti Aliasing Disabled"));
    });

    if (am.pluginActionManager())
        connect(am.pluginActionManager(),
                QOverload<DkPluginContainer *, const QString &>::of(&DkPluginActionManager::runPlugin),
                this,
                &DkViewPort::applyPlugin);

    // connect
    connect(am.action(DkActionManager::menu_file_reload), &QAction::triggered, this, &DkViewPort::reloadFile);
    connect(am.action(DkActionManager::menu_file_next), &QAction::triggered, this, &DkViewPort::loadNextFileFast);
    connect(am.action(DkActionManager::menu_file_prev), &QAction::triggered, this, &DkViewPort::loadPrevFileFast);
    connect(am.action(DkActionManager::menu_file_save), &QAction::triggered, this, &DkViewPort::saveFile);
    connect(am.action(DkActionManager::menu_file_save_as), &QAction::triggered, this, &DkViewPort::saveFileAs);
    connect(am.action(DkActionManager::menu_file_save_web), &QAction::triggered, this, &DkViewPort::saveFileWeb);
    connect(am.action(DkActionManager::menu_tools_wallpaper), &QAction::triggered, this, &DkViewPort::setAsWallpaper);

    connect(am.action(DkActionManager::menu_edit_rotate_cw), &QAction::triggered, this, &DkViewPort::rotateCW);
    connect(am.action(DkActionManager::menu_edit_rotate_ccw), &QAction::triggered, this, &DkViewPort::rotateCCW);
    connect(am.action(DkActionManager::menu_edit_rotate_180), &QAction::triggered, this, &DkViewPort::rotate180);
    connect(am.action(DkActionManager::menu_edit_transform), &QAction::triggered, this, &DkViewPort::resizeImage);
    connect(am.action(DkActionManager::menu_edit_delete), &QAction::triggered, this, &DkViewPort::deleteImage);
    connect(am.action(DkActionManager::menu_edit_copy), &QAction::triggered, this, &DkViewPort::copyImagePath);
    connect(am.action(DkActionManager::menu_edit_copy_buffer), &QAction::triggered, this, &DkViewPort::copyImageBuffer);
    connect(am.action(DkActionManager::menu_edit_copy_color),
            &QAction::triggered,
            this,
            &DkViewPort::copyPixelColorValue);

    connect(am.action(DkActionManager::menu_view_reset),
            &QAction::triggered,
            transformVM(),
            &DkViewPortTransformViewModel::zoomToFit);
    connect(am.action(DkActionManager::menu_view_100), &QAction::triggered, this, &DkViewPort::fullView);
    connect(am.action(DkActionManager::menu_view_zoom_in),
            &QAction::triggered,
            transformVM(),
            &DkViewPortTransformViewModel::zoomIn);
    connect(am.action(DkActionManager::menu_view_zoom_out),
            &QAction::triggered,
            transformVM(),
            &DkViewPortTransformViewModel::zoomOut);
    connect(am.action(DkActionManager::menu_view_tp_pattern), &QAction::toggled, this, &DkViewPort::togglePattern);
    connect(am.action(DkActionManager::menu_view_movie_pause), &QAction::triggered, this, &DkViewPort::pauseMovie);
    connect(am.action(DkActionManager::menu_view_movie_prev),
            &QAction::triggered,
            this,
            &DkViewPort::previousMovieFrame);
    connect(am.action(DkActionManager::menu_view_movie_next), &QAction::triggered, this, &DkViewPort::nextMovieFrame);

    connect(am.action(DkActionManager::menu_sync_view), &QAction::triggered, this, &DkViewPort::tcpForceSynchronize);

    // playing
    connect(mPrevButton, &QPushButton::pressed, this, &DkViewPort::loadPrevFileFast);
    connect(mNextButton, &QPushButton::pressed, this, &DkViewPort::loadNextFileFast);

    // trivial connects
    connect(this, &DkViewPort::movieLoadedSignal, [](bool movie) {
        DkActionManager::instance().enableMovieActions(movie);
    });

    // connect sync
    auto cm = DkSyncManager::inst().client();

    connect(this, &DkViewPort::sendTransformSignal, cm, &DkClientManager::sendTransform);
    connect(this, &DkViewPort::sendNewFileSignal, cm, &DkClientManager::sendNewFile);
    connect(cm, &DkClientManager::receivedNewFile, this, &DkViewPort::tcpLoadFile);
    connect(cm, &DkClientManager::updateConnectionSignal, mController, [this](const QString &msg) {
        mController->setInfo(msg);
    });
    connect(cm, &DkClientManager::receivedTransformation, transformVM(), &DkViewPortTransformViewModel::syncTransform);

    for (auto action : am.manipulatorActions())
        connect(action, &QAction::triggered, this, &DkViewPort::applyManipulator);

    // TODO:
    // one could blur the canvas if a transparent GUI is present
    // what we would need: QGraphicsBlurEffect...
    // render all widgets to the alpha channel (bw)
    // pre-render the mViewport to that image... apply blur
    // and then render the blurred image after the widget is rendered...
    // performance?!
    //
    transformVM()->setZoomCenterLimit(DkViewPortTransformViewModel::ZoomCenterLimit::CenterSmallDimension);
    connect(transformVM(), &DkViewPortTransformViewModel::transformChanged, this, [this]() {
        const qreal newZoomLevel = zoomLevel();
        const bool zoomChanged = std::abs(mZoomLevel - newZoomLevel) > 1e-6;
        mZoomLevel = newZoomLevel;
        if (zoomChanged) {
            showZoom();
        }
        mController->update(); // why do we need to update the mController manually?
        tcpSynchronize();

        if (zoomChanged) {
            emitZoomSignal();
        }
    });
}

DkViewPort::~DkViewPort()
{
    mController->closePlugin(false, true);
}

void DkViewPort::createShortcuts()
{
    DkActionManager &am = DkActionManager::instance();
    connect(am.action(DkActionManager::sc_first_file), &QAction::triggered, this, &DkViewPort::loadFirst);
    connect(am.action(DkActionManager::sc_last_file), &QAction::triggered, this, &DkViewPort::loadLast);
    connect(am.action(DkActionManager::sc_skip_prev), &QAction::triggered, this, &DkViewPort::loadSkipPrev10);
    connect(am.action(DkActionManager::sc_skip_next), &QAction::triggered, this, &DkViewPort::loadSkipNext10);
    connect(am.action(DkActionManager::sc_first_file_sync), &QAction::triggered, this, &DkViewPort::loadFirst);
    connect(am.action(DkActionManager::sc_last_file_sync), &QAction::triggered, this, &DkViewPort::loadLast);
    connect(am.action(DkActionManager::sc_skip_next_sync), &QAction::triggered, this, &DkViewPort::loadNextFileFast);
    connect(am.action(DkActionManager::sc_skip_prev_sync), &QAction::triggered, this, &DkViewPort::loadPrevFileFast);
}

void DkViewPort::setPaintWidget(QWidget *widget, bool removeWidget)
{
    if (!removeWidget) {
        mPaintLayout->addWidget(widget);
        // pluginImageWasApplied = false;
    } else {
        mPaintLayout->removeWidget(widget);
        // widget->deleteLater();
    }
    mController->raise();
    mNextButton->raise();
    mPrevButton->raise();
}
void DkViewPort::updateLoadedImage()
{
    if (mFSVM->loader()->hasImage()) {
        // modified image (for view), may differ from lastImage after rotate
        setImage(mFSVM->loader()->getPixmap());
    }
}

void DkViewPort::onImageLoaded(QSharedPointer<DkImageContainerT> image, bool loaded)
{
    // things todo if a file was not loaded...
    if (!loaded) {
        mController->getPlayer()->startTimer();
        mController->updateImage(nullptr);
        return;
    }

    // retain the previous image for animation, release when animation ends
    // we don't do this on unloadImage() because we might be on the last image in the slideshow
    const auto &dpy = DkSettingsManager::param().display();
    if (!imageVM()->isEmpty() //
        && dpy.transition != DkSettings::trans_appear //
        && dpy.animationDuration > 0.0 && //
        (mController->getPlayer()->isPlaying() //
         || window()->isFullScreen() //
         || DkSettingsManager::param().display().alwaysAnimate)) {
        mAnimationParams = getRenderParams(devicePixelRatio(), getWorldMatrix(), transformVM()->imgViewRect());
        mAnimationBuffer = imageVM()->downsampled(mAnimationParams.imageSize,
                                                  DkImage::targetColorSpace(this),
                                                  DkImage::targetFormat(),
                                                  DkImageStorage::process_sync | DkImageStorage::process_fallback);
        mAnimationBufferHasAlpha = imageVM()->alphaChannelUsed();
        mAnimationBuffer = DkImage::convertToColorSpaceInPlace(this, mAnimationBuffer);
        mAnimationValue = 1.0;

        if (dpy.transition == DkSettings::trans_fade && //
            mAnimationBufferHasAlpha //
            && DkSettingsManager::param().display().tpPattern) {
            // To draw nicely we must fade a composite with image and pattern
            QImage composite = renderBuffer(DkImage::targetFormat());
            QPainter offscreen(&composite);
            renderComposite(offscreen, mAnimationBuffer, mAnimationParams, draw_image | draw_pattern);
            mAnimationBuffer = composite.copy(mAnimationParams.deviceRect);
            mAnimationBufferHasAlpha = false;
        }
    }

    mController->updateImage(image);
}

void DkViewPort::loadImage(const QImage &newImg)
{
    // delete current information
    if (!unloadImage())
        return; // user canceled

    mFSVM->loader()->setImage(newImg, tr("Original Image"));
    setImage(newImg);

    // save to temp folder
    mFSVM->loader()->saveTempFile(newImg);
}

void DkViewPort::setImage(const QImage &newImg)
{
    mDisabledBackground = false;

    // calling show here fixes issues with the HUD
    // FIXME: would update() be better or is this still needed?
    show();

    DkTimer dt;

    emit movieLoadedSignal(false);
    stopMovie(); // just to be sure
    mSvg = {};

    mFSVM->cancelManipulator();

    bool isNewFile = mFSVM->prevFilePath() != mFSVM->currentFilePath();
    mFSVM->setPrevFilePath(mFSVM->currentFilePath());

    bool wasImageLoaded = !imageVM()->isEmpty();
    bool isImageLoaded = !newImg.isNull();

    imageVM()->setRasterImage(newImg);
    if (mFSVM->loader()->hasMovie() && !mFSVM->isCurrentFileEdited()) {
        loadMovie();
    }
    if (mFSVM->loader()->hasSvg() && !mFSVM->isCurrentFileEdited()) {
        imageVM()->setSVG(*mFSVM->currentImage()->getFileBuffer());
    }

    transformVM()->setImgSize(getImageSize(),
                              static_cast<DkSettings::keepZoom>(DkSettingsManager::param().display().keepZoom));

    DkActionManager::instance().enableImageActions(!newImg.isNull());

    if (wasImageLoaded ^ isImageLoaded)
        mController->imagePresenceChanged(isImageLoaded);

    mController->getPlayer()->startTimer();
    emit viewImageChanged();

    // init fading
    if (isNewFile && wasImageLoaded && DkSettingsManager::param().display().animationDuration != 0
        && DkSettingsManager::param().display().transition != DkSettingsManager::param().trans_appear
        && (mController->getPlayer()->isPlaying() || window()->isFullScreen()
            || DkSettingsManager::param().display().alwaysAnimate)) {
        mAnimationTimer->start();
        mAnimationTime.start();
    } else
        mAnimationValue = 0.0f;

    // set/clear crop rect
    const QSharedPointer<DkImageContainerT> currImg = mFSVM->currentImage();
    if (currImg) {
        mCropRect = currImg->cropRect();
    } else {
        mCropRect = DkRotatingRect();
    }

    update();

    // draw a histogram from the image -> does nothing if the histogram is invisible
    if (mController->getHistogram())
        mController->getHistogram()->drawHistogram(newImg);

    emitZoomSignal();

    // status info
    if (!newImg.isNull()) {
        QString colorSpaceDesc = tr("Unspecified"); // No color profile in the image
        QColorSpace colorSpace = newImg.colorSpace();
        if (colorSpace.isValid()) {
            colorSpaceDesc = colorSpace.description();
            if (colorSpaceDesc.isEmpty())
                colorSpaceDesc = tr("Custom"); // Unrecognized but valid color profile
        }

        DkStatusBarManager::instance().setMessage(DkUtils::formatToString(newImg.format()) + u'|' + colorSpaceDesc,
                                                  DkStatusBar::status_format_info);
        DkStatusBarManager::instance().setMessage(QString::number(newImg.width()) + " x "
                                                      + QString::number(newImg.height()),
                                                  DkStatusBar::status_dimension_info);

        if (imageContainer())
            DkStatusBarManager::instance().setMessage(imageContainer()->fileName(), DkStatusBar::status_file_info);
    } else {
        DkStatusBarManager::instance().setMessage("", DkStatusBar::status_zoom_info);
        DkStatusBarManager::instance().setMessage("", DkStatusBar::status_format_info);
        DkStatusBarManager::instance().setMessage("", DkStatusBar::status_dimension_info);
        DkStatusBarManager::instance().setMessage("", DkStatusBar::status_file_info);
    }
}

void DkViewPort::fullView()
{
    QPointF p = transformVM()->viewPortRect().center();
    zoom(1.0 / zoomLevel(), p.toPoint(), true);
}

void DkViewPort::showZoom()
{
    // don't show zoom if we are in fullscreen mode
    if (window()->isFullScreen() || DkSettingsManager::param().app().hideAllPanels)
        return;

    QString zoomStr = QString::asprintf("%.1f%%", zoomLevel() * 100);

    if (!mController->getZoomWidget()->isVisible())
        mController->setInfo(zoomStr, 3000, DkControlWidget::bottom_left_label);
}

void DkViewPort::repeatZoom()
{
    const auto btn = QApplication::mouseButtons();

    if (btn != Qt::XButton1 && btn != Qt::XButton2) {
        mRepeatZoomTimer->stop(); // safety if we did not catch the button release
        return;
    }

    if (DkSettingsManager::param().display().invertZoom ^ (btn == Qt::XButton1)) {
        zoom(0.9, mRepeatZoomCenter);
    } else {
        zoom(1.1, mRepeatZoomCenter);
    }
}

void DkViewPort::tcpSetWindowRect(QRect rect)
{
    this->setGeometry(rect);
}

void DkViewPort::tcpForceSynchronize()
{
    tcpSynchronize(QTransform(), true);
}

void DkViewPort::tcpSynchronize(QTransform relativeMatrix, bool force)
{
    if (!relativeMatrix.isIdentity()) {
        emit sendTransformSignal(relativeMatrix, QTransform(), QPointF());
        return;
    }

    // check if we need a synchronization
    if ((force || qApp->keyboardModifiers() == mAltMod || DkSettingsManager::param().sync().syncActions)
        && (hasFocus() || mController->hasFocus())) {
        QPointF size = QPointF(width(), height()) / 2;
        size = getWorldMatrix().inverted().map(size);
        size = getImageMatrix().inverted().map(size);
        size = QPointF(size.x() / getImageSize().width(), size.y() / getImageSize().height());

        emit sendTransformSignal(getWorldMatrix(), getImageMatrix(), size);
    }
}

void DkViewPort::applyPlugin(DkPluginContainer *plugin, const QString &key)
{
#ifdef WITH_PLUGINS
    if (!plugin)
        return;

    DkBatchPluginInterface *bPlugin = plugin->batchPlugin();
    if (bPlugin)
        bPlugin->loadSettings();

    QSharedPointer<DkImageContainerT> result = DkImageContainerT::fromImageContainer(
        plugin->plugin()->runPlugin(key, imageContainer()));
    if (result)
        setEditedImage(result);

    plugin->setActive(false);
#else
    Q_UNUSED(plugin);
    Q_UNUSED(key);
#endif
}

bool DkViewPort::isEdited() const
{
    return mFSVM->isCurrentFileEdited();
}

QImage DkViewPort::getImage() const
{
    if (imageContainer() && (!mSvg || !mSvg->isValid()) && (!mMovie || !mMovie->isValid()))
        return imageContainer()->image();

    return DkBaseViewPort::getImage();
}

QImage DkViewPort::getDrawImage() const
{
    return imageVM()->image();
}

void DkViewPort::resizeImage()
{
    if (!mResizeDialog)
        mResizeDialog = new DkResizeDialog(this);

    QSharedPointer<DkImageContainerT> imgC = imageContainer();
    QSharedPointer<DkMetaDataT> metaData;

    if (imgC) {
        metaData = imgC->getMetaData();
        QVector2D res = metaData->getResolution();
        mResizeDialog->setExifDpi(static_cast<float>(res.x()));
    }

    if (!imgC) {
        qWarning() << "cannot resize empty image...";
        return;
    }

    mResizeDialog->setImage(imgC->image());

    if (!mResizeDialog->exec())
        return;

    if (mResizeDialog->resample()) {
        QImage rImg = mResizeDialog->getResizedImage();

        if (!rImg.isNull()) {
            // this reloads the image -> that's not what we want!
            if (metaData)
                metaData->setResolution(QVector2D(mResizeDialog->getExifDpi(), mResizeDialog->getExifDpi()));

            imgC->setImage(rImg, tr("Resize"));
            setEditedImage(imgC);
        }
    } else if (metaData) {
        // ok, user just wants to change the resolution
        metaData->setResolution(QVector2D(mResizeDialog->getExifDpi(), mResizeDialog->getExifDpi()));
        qDebug() << "setting resolution to: " << mResizeDialog->getExifDpi();
    }
}

void DkViewPort::deleteImage()
{
    auto imgC = imageContainer();

    if (!imgC || !imgC->hasImage())
        return;

    getController()->applyPluginChanges(true);

    QFileInfo fileInfo(imgC->filePath());
    QString question;

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    question = tr("Shall I move %1 to trash?").arg(fileInfo.fileName());
#else
    question = tr("Do you want to permanently delete %1?").arg(fileInfo.fileName());
#endif

    auto *msgBox = new DkMessageBox(QMessageBox::Question,
                                    tr("Delete File"),
                                    question,
                                    (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel),
                                    this);

    msgBox->setDefaultButton(QMessageBox::Yes);
    msgBox->setObjectName("deleteFileDialog");

    int answer = msgBox->exec();

    if (answer == QMessageBox::Accepted || answer == QMessageBox::Yes) {
        mFSVM->deleteCurrentFile();
    }
}

void DkViewPort::saveFile()
{
    saveFileAs(true);
}

void DkViewPort::saveFileAs(bool silent)
{
    mController->closePlugin(false);

    QImage img = getImage();

    if (mFSVM->loader()->hasSvg() && !mFSVM->isCurrentFileEdited()) {
        auto *sd = new DkSvgSizeDialog(img.size(), DkUtils::getMainWindow());
        sd->resize(270, 120);

        int answer = sd->exec();

        if (answer == QDialog::Accepted) {
            img = QImage(sd->size(), QImage::Format_ARGB32);
            img.fill(QColor(0, 0, 0, 0));

            QPainter p(&img);
            mSvg->render(&p, QRectF(QPointF(), sd->size()));
        }
    }

    mFSVM->saveUserFile(img, silent);
}

void DkViewPort::saveFileWeb()
{
    mController->closePlugin(false);
    // TODO: shouldn't this use the SVG size dialog?
    mFSVM->saveFileWeb(getImage());
}

void DkViewPort::setAsWallpaper()
{
    const bool ok = mFSVM->setCurrentFileAsWallpaper();
    if (!ok) {
        QMessageBox::critical(this, tr("Error"), tr("Sorry, I could not create a wallpaper..."));
        return;
    }
}

void DkViewPort::applyManipulator()
{
    const auto *action = dynamic_cast<QAction *>(QObject::sender());
    if (!action) {
        qWarning() << "applyManipulator is not called from its action!";
        return;
    }

    DkActionManager &am = DkActionManager::instance();
    QSharedPointer<DkBaseManipulator> mpl = am.manipulatorManager().manipulator(action);

    if (!mpl) {
        qWarning() << "could not find manipulator for:" << action;
        return;
    }

    mFSVM->applyManipulator(mpl, [this]() {
        return getImage();
    });
}

void DkViewPort::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());

    if (!imageVM()->isEmpty()) {
        // usually the QGraphicsView should do this - but we have seen issues(e.g. #706)
        painter.setPen(Qt::NoPen);
        painter.setBrush(backgroundBrush());
        painter.drawRect(QRect(QPoint(), size()));

        painter.setWorldTransform(getWorldMatrix());

        const qreal zl = zoomLevel();
        // interpolate between 100% and max interpolate level
        if (!mForceFastRendering && zl - DBL_EPSILON > 1.0
            && zl <= DkSettingsManager::param().display().interpolateZoomLevel / 100.0) { // > max zoom level
            painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
        }

        if (mAnimationBuffer.isNull()) {
            draw(painter, 1.0);
        } else {
            switch (DkSettingsManager::param().display().transition) {
            case DkSettings::trans_fade: {
                // To properly cross-fade images we must consider the overlapped part of images
                // separately from non-overlapped part blended with background.
                // TODO: blend images in linear colorspace for nicer result

                const RenderParams newParams = getRenderParams(devicePixelRatio(),
                                                               getWorldMatrix(),
                                                               transformVM()->imgViewRect());
                bool newHasAlpha = imageVM()->alphaChannelUsed();
                bool oldHasAlpha = mAnimationBufferHasAlpha;

                // Fade-in new image
                if ((newHasAlpha | oldHasAlpha) && !DkSettingsManager::param().display().tpPattern) {
                    // Either image already blended with background, old way seems to be best
                    draw(painter, 1.0 - mAnimationValue, draw_default);
                } else {
                    // Get the intersection of the two images to separate
                    // cross-dissolved region from background-blended region
                    QRectF newRect = newParams.viewRect;
                    QRectF oldRect = mAnimationParams.viewRect;

                    newRect = newRect.intersected(this->rect()); // viewRects may extend past viewPort
                    oldRect = oldRect.intersected(this->rect());
                    QRectF clipRect = newRect.intersected(oldRect);

                    // Draw background-blended region
                    if (clipRect.toRect() == newRect.toRect()) {
                        // Images overlap within one pixel, no blend with background needed
                        // An edge might show from rounding so clip this as well
                        draw(painter, 1.0, draw_background);
                    } else {
                        // Mixed overlap, blend bottom image w/background
                        draw(painter, 1.0 - mAnimationValue, draw_default);
                    }

                    // Draw the cross-dissolved region
                    clipRect = newParams.worldMatrix.inverted().mapRect(clipRect);
                    painter.setClipRect(clipRect);
                    draw(painter, 1.0, draw_image | draw_pattern);
                    painter.setClipping(false);
                }

                // Fade-out old image
                double oldOpacity = painter.opacity();
                painter.setOpacity(mAnimationValue);
                painter.setTransform(mAnimationParams.worldMatrix);
                renderImage(painter, mAnimationBuffer, mAnimationParams);
                painter.setOpacity(oldOpacity);
                break;
            }
            case DkSettings::trans_swipe: {
                RenderParams params = getRenderParams(devicePixelRatio(),
                                                      getWorldMatrix(),
                                                      transformVM()->imgViewRect());
                QRectF viewRect = params.viewRect;
                double total = mNextSwipe ? width() - viewRect.x() //
                                          : -(viewRect.x() + viewRect.width());
                double dx = total * mAnimationValue;
                painter.setTransform(params.worldMatrix * QTransform::fromTranslate(dx, 0));
                draw(painter, 1.0);

                viewRect = mAnimationParams.viewRect;
                total = mNextSwipe ? -(viewRect.x() + viewRect.width()) //
                                   : width() - viewRect.x();
                dx = total * (1.0 - mAnimationValue);

                painter.setTransform(mAnimationParams.worldMatrix * QTransform::fromTranslate(dx, 0));
                if (DkSettingsManager::param().display().tpPattern && mAnimationBufferHasAlpha) {
                    renderPattern(painter, mAnimationParams);
                }
                renderImage(painter, mAnimationBuffer, mAnimationParams);
                break;
            }
            case DkSettings::trans_appear:
            case DkSettings::trans_end:
                break;
            }
        }

        // now disable world matrix for overlay display
        painter.setWorldMatrixEnabled(false);
    } else {
        if (!mDisabledBackground)
            eraseBackground(painter);
    }

    // draw the cropping rect
    if (!mCropRect.isEmpty() && DkSettingsManager::param().display().showCrop && imageContainer()) {
        // create path
        QPainterPath path;
        path.addRect(getImageViewRect().toRect());

        DkRotatingRect r = mCropRect;
        QPolygonF polyF;
        polyF = r.getClosedPoly();
        polyF = getImageMatrix().map(polyF);
        polyF = getWorldMatrix().map(polyF);
        path.addPolygon(polyF.toPolygon());

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 100));
        painter.drawPath(path);
    }

    painter.end();

    // propagate
    QGraphicsView::paintEvent(event);
}

void DkViewPort::leaveEvent(QEvent *event)
{
    // hide navigation buttons if the mouse leaves the viewport
    mNextButton->hide();
    mPrevButton->hide();
    DkBaseViewPort::leaveEvent(event);
}

// drawing functions --------------------------------------------------------------------
void DkViewPort::eraseBackground(QPainter &painter) const
{
    DkBaseViewPort::eraseBackground(painter);

    // draw logo/bg.png in the bottom-right corner, 1:1 pixels unless too big
    painter.save();
    painter.setWorldMatrixEnabled(false);
    painter.setTransform(QTransform());

    qreal dpr = devicePixelRatioF();
    QSizeF s = mImgBg.size() / dpr;

    if (s.width() > size().width() * 0.5)
        s = s * ((size().width() * 0.5) / s.width());

    if (s.height() > size().height() * 0.6)
        s = s * ((size().height() * 0.6) / s.height());

    QRectF bgRect(QPointF(), s);
    bgRect.moveBottomRight(geometry().bottomRight() - QPoint(20, 20));

    painter.translate(bgRect.topLeft());
    painter.scale(1.0 / dpr, 1.0 / dpr);
    painter.drawImage(QRectF(QPointF(), bgRect.size() * dpr), mImgBg);
    painter.restore();
}

void DkViewPort::loadMovie()
{
    if (mMovie)
        mMovie->stop();

    DkFileInfo fileInfo = mFSVM->currentImage()->fileInfo();
    if (fileInfo.isSymLink() && !fileInfo.resolveSymLink())
        return;

    std::unique_ptr<QIODevice> io = fileInfo.getIODevice();
    if (!io)
        return;

    QByteArray format = fileInfo.suffix().toLower().toLatin1();

    imageVM()->setMovie(io->readAll(), format, fileInfo.fileName());
}

void DkViewPort::pauseMovie(bool pause)
{
    if (!mMovie)
        return;

    mMovie->setPaused(pause);
}

void DkViewPort::nextMovieFrame()
{
    if (!mMovie)
        return;

    if (mMovie->state() == QMovie::Running) {
        DkActionManager::instance().action(DkActionManager::menu_view_movie_pause)->trigger();
        return;
    }

    mMovie->jumpToNextFrame();
    update();
}

void DkViewPort::previousMovieFrame()
{
    if (!mMovie)
        return;

    if (mMovie->state() == QMovie::Running) {
        DkActionManager::instance().action(DkActionManager::menu_view_movie_pause)->trigger();
        return;
    }

    int fn = mMovie->currentFrameNumber() - 1;
    if (fn == -1)
        fn = mMovie->frameCount() - 1;

    // NOTE: we'd prefer QMovie::jumpToFrame but it does not always work
    // and might cause freezes after a few clicks (GIF images)
    while (mMovie->currentFrameNumber() != fn)
        mMovie->jumpToNextFrame();

    update();
}

void DkViewPort::stopMovie()
{
    if (!mMovie)
        return;

    mMovie = {};
    mMovieIo = {};
}

void DkViewPort::drawPolygon(QPainter &painter, const QPolygon &polygon)
{
    QPoint lastPoint;

    for (const QPoint &p : polygon) {
        if (!lastPoint.isNull())
            painter.drawLine(p, lastPoint);

        lastPoint = p;
    }
}

// event listeners --------------------------------------------------------------------
void DkViewPort::resizeEvent(QResizeEvent *event)
{
    DkBaseViewPort::resizeEvent(event);
    const QSize s = event->size();
    mController->resize(s.width(), s.height());
}

// mouse events --------------------------------------------------------------------
bool DkViewPort::event(QEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    if (event->type() == QEvent::DevicePixelRatioChange) {
        // image matrix includes dpr adjustment
        transformVM()->setImgSize(getImageSize());
    }
#endif

    // ok obviously QGraphicsView eats all mouse events -> so we simply redirect these to QWidget in order to get them
    // delivered here
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick
        || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove
        || event->type() == QEvent::Wheel || event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease
        || event->type() == QEvent::DragEnter ||

        event->type() == QEvent::Drop) {
        // qDebug() << "redirecting event...";
        //  mouse events that double are now fixed, since the mViewport is now overlayed by the mController
        return QWidget::event(event);
    } else {
        // qDebug() << "not redirecting - type: " << event->type();
        return DkBaseViewPort::event(event);
    }
}

void DkViewPort::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void DkViewPort::mousePressEvent(QMouseEvent *event)
{
    // if zoom on wheel, the additional keys should be used for switching files
    if (DkSettingsManager::param().global().zoomOnWheel) {
        if (event->buttons() == Qt::XButton1)
            loadPrevFileFast();
        else if (event->buttons() == Qt::XButton2)
            loadNextFileFast();
    } else if (event->buttons() == Qt::XButton1 || event->buttons() == Qt::XButton2) {
        mRepeatZoomCenter = event->pos();
        repeatZoom();
        mRepeatZoomTimer->start();
    }

    if (DkSettingsManager::param().app().closeOnMiddleMouse && event->buttons() == Qt::MiddleButton)
        DkUtils::getMainWindow()->close();

    // ok, start panning
    if (transformVM()->upscaled() && !imageInside() && event->buttons() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
        mPosGrab = event->pos();
    }

    // keep in mind if the gesture was started in the mViewport
    // this fixes issues if some HUD widgets or child widgets
    // do not implement mouse events correctly
    if (event->buttons() == Qt::LeftButton)
        mGestureStarted = true;
    else
        mGestureStarted = false;

    // should be sent to QWidget?!
    DkBaseViewPort::mousePressEvent(event);
}

void DkViewPort::mouseReleaseEvent(QMouseEvent *event)
{
    mRepeatZoomTimer->stop();

    int sa = swipeRecognition(event->pos(), mPosGrab.toPoint());
    QPoint pos = mapToImage(event->pos());

    if (imageInside() && mGestureStarted) {
        swipeAction(sa);
    }

    // needed for scientific projects...
    if (pos.x() != -1 && pos.y() != -1)
        emit mouseClickSignal(event, pos);

    mGestureStarted = false;

    DkBaseViewPort::mouseReleaseEvent(event);
}

void DkViewPort::mouseDoubleClickEvent(QMouseEvent *event)
{
    // if zoom on wheel, the additional keys should be used for switching files
    if (DkSettingsManager::param().global().zoomOnWheel) {
        // double click event happens on second mouse press, so offset by 1
        if (event->buttons() == Qt::XButton1) {
            loadPrevFileFast();
            return;
        }

        if (event->buttons() == Qt::XButton2) {
            loadNextFileFast();
            return;
        }
    }

    DkBaseViewPort::mouseDoubleClickEvent(event);
}

void DkViewPort::mouseMoveEvent(QMouseEvent *event)
{
    if (DkSettingsManager::param().display().showNavigation && event->modifiers() == Qt::NoModifier
        && event->buttons() == Qt::NoButton) {
        int left = qMin(100, qRound(0.1 * width()));
        int right = qMax(width() - 100, qRound(0.9 * width()));

        if (event->pos().x() < left) {
            mNextButton->hide();
            mPrevButton->show();
        } else if (event->pos().x() > right) {
            mNextButton->show();
            mPrevButton->hide();
        } else {
            mNextButton->hide();
            mPrevButton->hide();
        }
    }

    // qDebug() << "mouse move (DkViewPort)";
    // changeCursor();
    mCurrentPixelPos = event->pos();

    if (DkStatusBarManager::instance().statusbar()->isVisible())
        getPixelInfo(event->pos());

    if (transformVM()->upscaled() && event->buttons() == Qt::LeftButton) {
        QPointF cPos = event->pos();
        QPointF dxy = (cPos - mPosGrab);
        mPosGrab = cPos;
        moveViewInWidgetCoords(dxy);

        // with shift also a hotkey for fast switching...
        if ((DkSettingsManager::param().sync().syncAbsoluteTransform
             && event->modifiers() == (mAltMod | Qt::ShiftModifier))
            || (!DkSettingsManager::param().sync().syncAbsoluteTransform && event->modifiers() == (mAltMod))) {
            if (dxy.x() != 0 || dxy.y() != 0) {
                QTransform relTransform;
                relTransform.translate(dxy.x(), dxy.y());
                tcpSynchronize(relTransform);
            }
        }
        // absolute transformation
        else
            tcpSynchronize();
    }

    int dist = QPoint(event->pos() - mPosGrab.toPoint()).manhattanLength();

    // drag & drop action
    if (event->buttons() == Qt::LeftButton && dist > QApplication::startDragDistance() && imageInside()
        && !getImage().isNull()
        && !QApplication::widgetAt(event->globalPosition().toPoint())) { // is NULL if the mouse leaves the window

        QMimeData *mimeData = createMimeForDrag();

        QPixmap pm;
        if (!getImage().isNull())
            pm = QPixmap::fromImage(imageVM()->image().scaledToHeight(73, Qt::SmoothTransformation));
        if (pm.width() > 130)
            pm = pm.scaledToWidth(100, Qt::SmoothTransformation);

        auto *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(pm);
        drag->exec(Qt::CopyAction);
    }

    // send to parent
    DkBaseViewPort::mouseMoveEvent(event);
}

void DkViewPort::wheelEvent(QWheelEvent *event)
{
    auto ctrlMod = DkSettingsManager::param().global().ctrlMod;
    if ((!DkSettingsManager::param().global().zoomOnWheel && !(event->modifiers() & ctrlMod))
        || (DkSettingsManager::param().global().zoomOnWheel && (event->modifiers() & ctrlMod))) {
        auto delta = 0;

        if (DkSettingsManager::param().global().horZoomSkips || (event->modifiers() == Qt::AltModifier)) {
            delta = event->angleDelta().x();
        } else {
            delta = event->angleDelta().y();
        }
        if (delta < 0)
            loadNextFileFast();
        if (delta > 0)
            loadPrevFileFast();
    } else
        DkBaseViewPort::wheelEvent(event);

    tcpSynchronize();
}

int DkViewPort::swipeRecognition(QPoint start, QPoint end)
{
    DkVector vec(static_cast<float>(start.x() - end.x()), static_cast<float>(start.y() - end.y()));

    if (std::fabs(vec.norm()) < 100)
        return no_swipe;

    double angle = DkMath::normAngleRad(vec.angle(DkVector(0, 1)), 0.0, CV_PI);
    bool horizontal = false;

    if (angle > CV_PI * 0.3 && angle < CV_PI * 0.6)
        horizontal = true;
    else if (angle < 0.2 * CV_PI || angle > 0.8 * CV_PI)
        horizontal = false;
    else
        return no_swipe; // angles ~45� are not accepted

    QPoint startPos = QWidget::mapFromGlobal(end);
    qDebug() << "vec: " << vec.x << ", " << vec.y;

    if (horizontal) {
        if (vec.x < 0)
            return next_image;
        else
            return prev_image;

    }
    // upper part of the canvas is thumbs
    else if (!horizontal && startPos.y() < height() * 0.5) {
        // downward gesture is opening
        if (vec.y > 0)
            return open_thumbs;
        else
            return close_thumbs;
    }
    // lower part of the canvas is thumbs
    else if (!horizontal && startPos.y() > height() * 0.5) {
        // upward gesture is opening
        if (vec.y < 0)
            return open_metadata;
        else
            return close_metadata;
    }

    return no_swipe;
}

void DkViewPort::swipeAction(int swipeGesture)
{
    Q_ASSERT(mController);

    switch (swipeGesture) {
    case next_image:
        loadNextFileFast();
        break;
    case prev_image:
        loadPrevFileFast();
        break;
    case open_thumbs:
        mController->showPreview(true);
        break;
    case close_thumbs:
        mController->showPreview(false);
        break;
    case open_metadata:
        mController->showMetaData(true);
        break;
    case close_metadata:
        mController->showMetaData(false);
        break;
    default:
        break;
    }
}

void DkViewPort::setFullScreen(bool fullScreen)
{
    Q_ASSERT(mController);
    mController->setFullScreen(fullScreen);

    if (fullScreen)
        mHideCursorTimer->start();
    else
        unsetCursor();
}

QPoint DkViewPort::mapToImage(const QPoint &windowPos) const
{
    QPointF imgPos = getWorldMatrix().inverted().map(QPointF(windowPos));
    imgPos = (getImageMatrix().inverted() * devicePixelRatioF()).map(imgPos);

    QPoint p(qFloor(imgPos.x()), qFloor(imgPos.y()));
    QSize sz = imageVM()->image().size();

    if (p.x() < 0 || p.y() < 0 || p.x() >= sz.width() || p.y() >= sz.height()) {
        return {-1, -1};
    }

    return p;
}

void DkViewPort::getPixelInfo(const QPoint &pos)
{
    if (imageVM()->isEmpty())
        return;

    QPoint xy = mapToImage(pos);
    if (xy.x() < 0)
        return;

    // TODO: This is rgb now, but we could display native pixel values too, even CMYK in Qt 6.8.0
    const QImage img = getImage();
    const QColor color = img.pixelColor(xy);

    const QRgb rgba = color.rgba(); // converts higher depths to ARGB-8888
    QString msg;
    msg.reserve(64);
    msg = msg + "x: " + QString::number(xy.x()) + " y: " + QString::number(xy.y()) + " | r: "
        + QString::number(qRed(rgba)) + " g: " + QString::number(qGreen(rgba)) + " b: " + QString::number(qBlue(rgba));

    if (img.hasAlphaChannel())
        msg = msg + " a: " + QString::number(qAlpha(rgba));

    // TODO: convert color to sRgb colorspace
    msg = msg + " | " + DkUtils::colorToCssHex(color, img.hasAlphaChannel());

    DkStatusBarManager::instance().setMessage(msg, DkStatusBar::status_pixel_info);
}

QString DkViewPort::getCurrentPixelHexValue()
{
    if (imageVM()->isEmpty() || mCurrentPixelPos.isNull())
        return {};

    QPoint xy = mapToImage(mCurrentPixelPos);
    if (xy.x() < 0)
        return {};

    const QImage img = getImage();
    return DkUtils::colorToCssHex(img.pixelColor(xy), img.hasAlphaChannel()).remove(0, 1);
}

// Copy & Paste --------------------------------------------------------
void DkViewPort::copyPixelColorValue()
{
    if (getImage().isNull())
        return;

    auto *mimeData = new QMimeData;
    mimeData->setText(getCurrentPixelHexValue());
    QApplication::clipboard()->setMimeData(mimeData);
}

void DkViewPort::copyImagePath()
{
    auto *mimeData = new QMimeData;

    // NOTE: if we simply prepend "file://", we will get into problems with mounted drives (e.g. //hermes...)
    QUrl fileUrl = QUrl::fromLocalFile(mFSVM->currentFilePath());
    mimeData->setUrls({fileUrl});
    mimeData->setText(fileUrl.toLocalFile());
    QApplication::clipboard()->setMimeData(mimeData);
}

QMimeData *DkViewPort::createMimeForDrag() const
{
    auto *mimeData = new QMimeData;

    QImage img = getImage();
    QString filePath = mFSVM->currentFilePath();
    DkFileInfo fileInfo(filePath);

    // Provide image buffer if file edited or cannot be opened by receiver
    if (!img.isNull() && (mFSVM->isCurrentFileEdited() || !fileInfo.exists() || fileInfo.isFromZip())) {
        mimeData->setImageData(img);
    } else {
        QUrl fileUrl = QUrl::fromLocalFile(filePath);
        mimeData->setUrls({fileUrl});
        mimeData->setText(fileUrl.toLocalFile());
    }

    return mimeData;
}

void DkViewPort::copyImageBuffer()
{
    QImage img = getImage();
    if (img.isNull())
        return;

    auto *mimeData = new QMimeData;
    mimeData->setImageData(img);
    QApplication::clipboard()->setMimeData(mimeData);
}

void DkViewPort::animateFade()
{
    double duration = DkSettingsManager::param().display().animationDuration;
    if (duration <= 0.0) {
        mAnimationBuffer = QImage();
        mAnimationTimer->stop();
        mAnimationValue = 0;
        return;
    }

    double elapsed = mAnimationTime.elapsed() / 1000.0;
    double t = elapsed / duration;
    t = std::clamp(t, 0.0, 1.0);

    // Smootherstep easing curve (Ken Perlin variation)
    t = t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    mAnimationValue = 1.0 - t;

    // slow in - slow out (previous)
    // t = 1.0 - t;
    // double speed = t > 0.5 ? 1.0 - t : t;
    // speed *= .05;
    // mAnimationValue = t;

    if (mAnimationValue <= 0) {
        mAnimationBuffer = QImage();
        mAnimationTimer->stop();
        mAnimationValue = 0;
    }

    update();
}

void DkViewPort::togglePattern(bool show)
{
    emit infoSignal((show) ? tr("Transparency Pattern Enabled") : tr("Transparency Pattern Disabled"));
    DkBaseViewPort::togglePattern(show);
}

// edit image --------------------------------------------------------------------
void DkViewPort::rotateCW()
{
    if (!mController->applyPluginChanges(true))
        return;

    mFSVM->rotateImage(90);
}

void DkViewPort::rotateCCW()
{
    if (!mController->applyPluginChanges(true))
        return;

    mFSVM->rotateImage(-90);
}

void DkViewPort::rotate180()
{
    if (!mController->applyPluginChanges(true))
        return;

    mFSVM->rotateImage(180);
}

// file handling --------------------------------------------------------------------

void DkViewPort::setEditedImage(QSharedPointer<DkImageContainerT> img)
{
    if (!img) {
        emit infoSignal(tr("Attempted to set NULL image")); // not sure if users understand that
        return;
    }

    if (!mController->applyPluginChanges(true)) {
        return;
    }

    mFSVM->cancelManipulator();

    mFSVM->loader()->setImage(img);
}

bool DkViewPort::unloadImage()
{
    if (!mController->applyPluginChanges(true)) // user wants to apply changes first
        return false;

    bool success = mFSVM->loader()->promptSaveBeforeUnload(); // returns false if the user cancels
    if (!success) {
        return false;
    }

    // notify controller
    mController->updateImage({}, false);

    stopMovie();

    if (mSvg) {
        mSvg = {};
    }

    return true;
}

void DkViewPort::deactivate()
{
    setImage(QImage());
    mDisabledBackground = true;
}

void DkViewPort::loadFile(const QString &filePath)
{
    if (!unloadImage())
        return;

    mFSVM->loadFile(filePath);

    // diem: I removed this line for a) we don't support remote displays anymore and be:
    // https://github.com/nomacs/nomacs/issues/219 qDebug() << "sync mode: " <<
    // (DkSettingsManager::param().sync().syncMode == DkSettings::sync_mode_remote_display); if
    // ((qApp->keyboardModifiers() == mAltMod || DkSettingsManager::param().sync().syncMode ==
    // DkSettings::sync_mode_remote_display) &&
    //	(hasFocus() || mController->hasFocus()) &&
    //	mFSVM->loader()->hasFile())
    //	tcpLoadFile(0, filePath);
}

void DkViewPort::reloadFile()
{
    if (!unloadImage()) {
        return;
    }
    mFSVM->reloadFile();
}

void DkViewPort::loadPrevFileFast()
{
    loadFileFast(-1);
}

void DkViewPort::loadNextFileFast()
{
    loadFileFast(1);
}

void DkViewPort::loadFileFast(int skipIdx)
{
    if (!unloadImage())
        return;

    mNextSwipe = skipIdx > 0;

    // TODO: remove this hack
    QApplication::sendPostedEvents();

    mFSVM->loadOffsetFromCurrentFile(skipIdx);

    if ((qApp->keyboardModifiers() == mAltMod || DkSettingsManager::param().sync().syncActions)
        && (hasFocus() || mController->hasFocus())) {
        emit sendNewFileSignal((qint16)skipIdx);
        QCoreApplication::sendPostedEvents();
    }
}

void DkViewPort::loadFirst()
{
    if (!unloadImage())
        return;

    mFSVM->loadFirst();

    if ((qApp->keyboardModifiers() == mAltMod || DkSettingsManager::param().sync().syncActions)
        && (hasFocus() || mController->hasFocus()))
        emit sendNewFileSignal(SHRT_MIN);
}

void DkViewPort::loadLast()
{
    if (!unloadImage())
        return;

    mFSVM->loadLast();

    if ((qApp->keyboardModifiers() == mAltMod || DkSettingsManager::param().sync().syncActions)
        && (hasFocus() || mController->hasFocus()))
        emit sendNewFileSignal(SHRT_MAX);
}

void DkViewPort::loadSkipPrev10()
{
    loadFileFast(-DkSettingsManager::param().global().skipImgs);
    // unloadImage();

    // if (mFSVM->loader() && !testLoaded)
    //	mFSVM->loader()->changeFile(-DkSettingsManager::param().global().skipImgs, (parent->isFullScreen() &&
    // DkSettingsManager::param().slideShow().silentFullscreen));

    if (qApp->keyboardModifiers() == mAltMod && (hasFocus() || mController->hasFocus()))
        emit sendNewFileSignal((qint16)-DkSettingsManager::param().global().skipImgs);
}

void DkViewPort::loadSkipNext10()
{
    loadFileFast(DkSettingsManager::param().global().skipImgs);
    // unloadImage();

    // if (mFSVM->loader() && !testLoaded)
    //	mFSVM->loader()->changeFile(DkSettingsManager::param().global().skipImgs, (parent->isFullScreen() &&
    // DkSettingsManager::param().slideShow().silentFullscreen));

    if (qApp->keyboardModifiers() == mAltMod && (hasFocus() || mController->hasFocus()))
        emit sendNewFileSignal((qint16)DkSettingsManager::param().global().skipImgs);
}

void DkViewPort::tcpLoadFile(qint16 idx, const QString &filename)
{
    qDebug() << "I got a file request??";

    // some hack: set the mode to default in order to prevent loops (if both are auto connected)
    // should be mostly harmless
    // int oldMode = DkSettingsManager::param().sync().syncMode;
    // DkSettingsManager::param().sync().syncMode = DkSettings::sync_mode_receiveing_command;

    if (filename.isEmpty()) {
        // change the file idx according to my brother
        switch (idx) {
        case SHRT_MIN:
            loadFirst();
            break;
        case SHRT_MAX:
            loadLast();
            break;
        // case 1:
        //	loadNextFileFast();
        //	break;
        // case -1:
        //	loadPrevFileFast();
        //	break;
        default:
            loadFileFast(idx);
            // if (mFSVM->loader()) mFSVM->loader()->loadFileAt(idx);
        }
    } else
        loadFile(filename);

    qDebug() << "loading file: " << filename;

    // DkSettingsManager::param().sync().syncMode = oldMode;
}

QSharedPointer<DkImageContainerT> DkViewPort::imageContainer() const
{
    return mFSVM->currentImage();
}

void DkViewPort::setImageLoader(QSharedPointer<DkImageLoader> newLoader)
{
    mFSVM->setLoader(std::move(newLoader));

    // The image loader can have a previous directory,
    // so need to get the states from it.
    mController->getFilePreview()->updateThumbs(mFSVM->loader()->getImages());
    mFSVM->loader()->activate();
}

void DkViewPort::connectLoader()
{
    auto *vm = mFSVM.get();
    connect(vm, &DkViewPortFSViewModel::currentImageLoaded, this, &DkViewPort::onImageLoaded);
    connect(vm, &DkViewPortFSViewModel::currentImageUpdated, this, &DkViewPort::updateLoadedImage);
    connect(vm, &DkViewPortFSViewModel::directoryChanged, mController->getFilePreview(), &DkFilePreview::updateThumbs);
    connect(vm,
            &DkViewPortFSViewModel::currentImageUpdated,
            mController->getFilePreview(),
            &DkFilePreview::setFileInfo);
    connect(vm, &DkViewPortFSViewModel::showInfoRequested, mController, &DkControlWidget::setInfo);
    connect(vm, &DkViewPortFSViewModel::playStateChanged, mController->getPlayer(), &DkPlayer::play);
    connect(vm, &DkViewPortFSViewModel::directoryChanged, mController->getScroller(), &DkFolderScrollBar::updateDir);
    connect(vm, &DkViewPortFSViewModel::imageIndexChanged, mController->getScroller(), &DkFolderScrollBar::updateFile);
    connect(mController->getScroller(), &DkFolderScrollBar::valueChanged, vm, &DkViewPortFSViewModel::loadFileAt);

    connect(vm, &DkViewPortFSViewModel::manipulatorStarted, this, [this](bool isExtended) {
        // show the dock (in case it's not shown yet)
        if (isExtended) {
            DkActionManager::instance().action(DkActionManager::menu_edit_image)->setChecked(true);
        }
        emit showProgress(true, 500);
    });
    connect(vm, &DkViewPortFSViewModel::manipulatorBusyAborted, this, [this]() {
        mController->setInfo(tr("Busy"));
    });
    connect(vm, &DkViewPortFSViewModel::manipulatorSucceeded, this, [this](QSharedPointer<DkImageContainerT> img) {
        if (img) {
            setEditedImage(img);
        }
        emit showProgress(false);
    });
    connect(vm, &DkViewPortFSViewModel::manipulatorErrored, this, [this](const QString &msg) {
        mController->setInfo(msg);
        emit showProgress(false);
    });
}

DkControlWidget *DkViewPort::getController()
{
    return mController;
}

void DkViewPort::cropImage(const DkRotatingRect &rect, const QColor &bgCol, bool cropToMetaData)
{
    QSharedPointer<DkImageContainerT> imgC = mFSVM->currentImage();

    if (!imgC) {
        qWarning() << "cannot crop NULL image...";
        return;
    }

    imgC->cropImage(rect, bgCol, cropToMetaData);
    setEditedImage(imgC);
}

void DkViewPort::emitZoomSignal()
{
    qreal zoomPercentage = zoomLevel() * 100;
    emit zoomSignal(zoomPercentage);
    DkStatusBarManager::instance().setMessage(QString::number(qRound(zoomPercentage)) + "%",
                                              DkStatusBar::status_zoom_info);
}

// DkViewPortFrameless --------------------------------------------------------------------
DkViewPortFrameless::DkViewPortFrameless(DkThumbLoader *thumbLoader, QWidget *parent)
    : DkViewPort(thumbLoader, parent, false)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    mImgBg.load(QFileInfo(QApplication::applicationDirPath(), "bgf.png").absoluteFilePath());

    if (mImgBg.isNull())
        mImgBg.load(":/nomacs/img/splash-screen.png");

    DkActionManager &am = DkActionManager::instance();
    mStartActions.append(am.action(DkActionManager::menu_file_open));
    mStartActions.append(am.action(DkActionManager::menu_file_open_dir));

    mStartIcons.append(am.icon(DkActionManager::icon_file_open_large));
    mStartIcons.append(am.icon(DkActionManager::icon_file_dir_large));
    transformVM()->setZoomCenterLimit(DkViewPortTransformViewModel::ZoomCenterLimit::ToImageEdge);
    transformVM()->setDisablePanForSmallDimension(false);
    transformVM()->setDisableControlAndCenter(true);
}

void DkViewPortFrameless::paintEvent(QPaintEvent *event)
{
    if (!window()->isFullScreen()) {
        QPainter painter(viewport());
        painter.setWorldTransform(getWorldMatrix());
        drawFrame(painter);
        painter.end();
    }

    DkViewPort::paintEvent(event);
}

void DkViewPortFrameless::resizeEvent(QResizeEvent *event)
{
    DkViewPort::resizeEvent(event);

    mStartActionsRects.clear();
    mStartActionsIcons.clear();

    QRectF initialRect = rect();
    QPointF oldCenter = initialRect.center();
    QTransform cT;
    cT.scale(400 / initialRect.width(), 400 / initialRect.width());
    initialRect = cT.mapRect(initialRect);
    initialRect.moveCenter(oldCenter);

    QSize s = mImgBg.size();
    QRectF bgRect(QPoint(), s);
    bgRect.moveCenter(initialRect.center());
    mStartBgRect = bgRect;

    constexpr qreal margin = 40;
    qreal iconSizeMargin = (initialRect.width() - 3 * margin) / static_cast<qreal>(mStartActions.size());
    QSize iconSize = QSize(qRound(iconSizeMargin - margin), qRound(iconSizeMargin - margin));
    QPointF offset = QPointF(bgRect.left() + 49, bgRect.top() + 246 + 15);

    for (int idx = 0; idx < mStartActions.size(); idx++) {
        QRectF iconRect = QRectF(offset, iconSize);
        QPixmap ci = !mStartIcons[idx].isNull() ? mStartIcons[idx].pixmap(iconSize)
                                                : mStartActions[idx]->icon().pixmap(iconSize);
        mStartActionsRects.push_back(iconRect);
        mStartActionsIcons.push_back(ci);

        offset.setX(offset.x() + margin + iconSize.width());
    }
}

void DkViewPortFrameless::eraseBackground(QPainter &painter) const
{
    if (window()->isFullScreen()) {
        QColor col = QColor(0, 0, 0);
        col.setAlpha(150);
        painter.setWorldMatrixEnabled(false);
        painter.fillRect(QRect(QPoint(), size()), col);
        painter.setWorldMatrixEnabled(true);
    }

    if (!imageVM()->isEmpty())
        return;

    painter.setWorldTransform(getImageMatrix());
    painter.setBrush(QColor(127, 144, 144, 200));
    painter.setPen(QColor(100, 100, 100, 255));

    painter.drawImage(mStartBgRect, mImgBg, QRect(QPoint(), mImgBg.size()));

    // draw start actions
    for (int idx = 0; idx < mStartActions.size(); idx++) {
        // TODO: what if the start icon is null?
        painter.drawPixmap(mStartActionsRects[idx],
                           mStartActionsIcons[idx],
                           QRect(QPoint(), mStartActionsIcons[idx].size()));

        QRectF tmpRect = mStartActionsRects[idx];
        QString text = mStartActions[idx]->text().remove("&");

        tmpRect.moveTop(tmpRect.bottom() + 10);
        tmpRect.setWidth(tmpRect.width() + 20.0); // more space for text; half of margin
        tmpRect.setBottom(mStartBgRect.bottom() - 40.0); // do not overlap info text
        painter.drawText(tmpRect, text);
    }

    QString infoText = tr("Press F10 to exit Frameless view");
    QRectF tmpRect = QRectF{mStartBgRect.left() + 50.0,
                            mStartBgRect.bottom() - 40.0,
                            mStartBgRect.width() - 100.0,
                            20.0};
    painter.drawText(tmpRect, infoText);
}

void DkViewPortFrameless::drawFrame(QPainter &painter)
{
    // TODO: replace hasAlphaChannel with has alphaBorder
    if ((!imageVM()->isEmpty() && imageVM()->image().hasAlphaChannel())
        || !DkSettingsManager::param().display().showBorder) // braces
        return;

    painter.setBrush(QColor(255, 255, 255, 200));
    painter.setPen(QColor(100, 100, 100, 255));

    QRectF frameRect;

    const QRectF imgViewRect = transformVM()->imgViewRect();
    qreal fs = qMin(imgViewRect.width(), imgViewRect.height()) * 0.1;

    // looks pretty bad if the frame is too small
    if (fs < 4)
        return;

    frameRect = imgViewRect;
    frameRect.setSize(frameRect.size() + QSize(qRound(fs), qRound(fs)));
    frameRect.moveCenter(imgViewRect.center());

    painter.drawRect(frameRect);
}

void DkViewPortFrameless::mousePressEvent(QMouseEvent *event)
{
    // move the window - todo: NOT full screen, window inside...
    setCursor(Qt::ClosedHandCursor);
    mPosGrab = event->pos();

    DkViewPort::mousePressEvent(event);
}

void DkViewPortFrameless::mouseReleaseEvent(QMouseEvent *event)
{
    if (imageVM()->isEmpty()) {
        QPointF pos = getImageMatrix().inverted().map(event->pos());

        for (int idx = 0; idx < mStartActionsRects.size(); idx++) {
            if (mStartActionsRects[idx].contains(pos)) {
                event->accept();
                unsetCursor();
                // setCursor(Qt::OpenHandCursor);
                mStartActions[idx]->trigger();
                return;
            }
        }
    }

    unsetCursor();
    DkViewPort::mouseReleaseEvent(event);
}

void DkViewPortFrameless::mouseMoveEvent(QMouseEvent *event)
{
    if (imageVM()->isEmpty()) {
        QPointF pos = getImageMatrix().inverted().map(event->pos());

        int idx;
        for (idx = 0; idx < mStartActionsRects.size(); idx++) {
            if (mStartActionsRects[idx].contains(pos)) {
                setCursor(Qt::PointingHandCursor);
                break;
            }
        }

        //// TODO: change if closed hand cursor is present...
        // if (idx == startActionsRects.size())
        //	setCursor(Qt::OpenHandCursor);
    }

    if (DkStatusBarManager::instance().statusbar()->isVisible())
        getPixelInfo(event->pos());

    if (event->buttons() == Qt::LeftButton) {
        QPointF cPos = event->pos();
        QPointF dxy = (cPos - mPosGrab);
        mPosGrab = cPos;
        moveViewInWidgetCoords(dxy);
    }

    QGraphicsView::mouseMoveEvent(event);
}

// DkViewPortContrast --------------------------------------------------------------------
DkViewPortContrast::DkViewPortContrast(DkThumbLoader *thumbLoader, QWidget *parent)
    : DkViewPort(thumbLoader, parent)
{
    mColorTable = QVector<QRgb>(256);
    for (int i = 0; i < mColorTable.size(); i++)
        mColorTable[i] = qRgb(i, i, i);

    // connect
    auto ttb = DkToolBarManager::inst().transferToolBar();
    connect(ttb, &DkTransferToolBar::colorTableChanged, this, &DkViewPortContrast::changeColorTable);
    connect(ttb, &DkTransferToolBar::channelChanged, this, &DkViewPortContrast::changeChannel);
    connect(ttb, &DkTransferToolBar::pickColorRequest, this, &DkViewPortContrast::pickColor);
    connect(ttb, &DkTransferToolBar::tFEnabled, this, &DkViewPortContrast::updateImage);
    connect(this, &DkViewPortContrast::cancelPickColor, ttb, &DkTransferToolBar::pickColorCancelled);
    connect(this, &DkViewPortContrast::tFSliderAdded, ttb, &DkTransferToolBar::insertSlider);
    connect(this, &DkViewPortContrast::imageModeSet, ttb, &DkTransferToolBar::setImageMode);
}

DkViewPortContrast::~DkViewPortContrast() = default;

void DkViewPortContrast::changeChannel(int channel)
{
    if (channel < 0 || channel >= mImgs.size())
        return;

    mActiveChannel = channel;
    if (mDrawFalseColorImg)
        updateImage(true);
}

void DkViewPortContrast::changeColorTable(const QGradientStops &stops)
{
    qreal fac;

    qreal actPos, leftStop, rightStop;
    QColor tmp;

    int rLeft, gLeft, bLeft, rRight, gRight, bRight;
    int rAct, gAct, bAct;

    // At least one stop has to be set:
    tmp = stops.at(0).second;
    tmp.getRgb(&rLeft, &gLeft, &bLeft);
    leftStop = stops.at(0).first;

    // If just one stop is set, we can speed things up:
    if (stops.size() == 1) {
        for (QRgb &i : mColorTable)
            i = qRgb(rLeft, gLeft, bLeft);
    }
    // Otherwise interpolate:
    else {
        int rightStopIdx = 1;
        tmp = stops.at(rightStopIdx).second;
        tmp.getRgb(&rRight, &gRight, &bRight);
        rightStop = stops.at(rightStopIdx).first;

        for (int i = 0; i < mColorTable.size(); i++) {
            actPos = static_cast<qreal>(i) / static_cast<qreal>(mColorTable.size());

            if (actPos > rightStop) {
                leftStop = rightStop;

                rLeft = rRight;
                gLeft = gRight;
                bLeft = bRight;

                if (stops.size() > rightStopIdx + 1) {
                    rightStopIdx++;
                    rightStop = stops.at(rightStopIdx).first;
                    tmp = stops.at(rightStopIdx).second;
                    tmp.getRgb(&rRight, &gRight, &bRight);
                }
            }

            if (actPos <= leftStop)
                mColorTable[i] = qRgb(rLeft, gLeft, bLeft);
            else if (actPos >= rightStop)
                mColorTable[i] = qRgb(rRight, gRight, bRight);
            else {
                fac = (actPos - leftStop) / (rightStop - leftStop);
                rAct = qRound(rLeft + (rRight - rLeft) * fac);
                gAct = qRound(gLeft + (gRight - gLeft) * fac);
                bAct = qRound(bLeft + (bRight - bLeft) * fac);
                mColorTable[i] = qRgb(rAct, gAct, bAct);
            }
        }
    }

    if (mDrawFalseColorImg)
        updateImage(true);
}

void DkViewPortContrast::setImage(const QImage &newImg)
{
    DkViewPort::setImage(newImg);

    if (newImg.isNull())
        return;

    QImage img = imageVM()->image();

    // for mono,gray8,gray16, create indexed image w/empty color table; it will be added later
    if (img.pixelFormat().colorModel() == QPixelFormat::Grayscale) {
        img = img.convertToFormat(QImage::Format_Grayscale8);
        img.reinterpretAsFormat(QImage::Format_Indexed8);
    }

    // picked colors are in screen space and so too should be the image to prevent conversion
    // note we do not apply any colorspace conversion to the source so we are mapping the raw image values
    // which may correspond to real-world non-image things like sensor data, spectrograms etc
    QColorSpace screenColorSpace = DkImage::targetColorSpace(this);
    img.setColorSpace(screenColorSpace);

    if (img.format() == QImage::Format_Indexed8) {
        mImgs = QVector<QImage>(1);
        mImgs[0] = img;
        mActiveChannel = 0;
    }
#ifdef WITH_OPENCV

    else {

        mImgs = QVector<QImage>(4);
        std::vector<cv::Mat> planes;

        cv::Mat imgUC3 = DkImage::qImage2Mat(img);
        // int format = imgQt.format();
        // if (format == QImage::Format_RGB888)
        //	imgUC3 = Mat(imgQt.height(), imgQt.width(), CV_8UC3, (uchar*)imgQt.bits(), imgQt.bytesPerLine());
        // else
        //	imgUC3 = Mat(imgQt.height(), imgQt.width(), CV_8UC4, (uchar*)imgQt.bits(), imgQt.bytesPerLine());
        cv::split(imgUC3, planes);
        // Store the 3 channels in a QImage Vector.
        // Be aware that OpenCV 'swaps' the rgb triplet, hence process it in a descending way:
        int idx = 1;
        for (int i = 2; i >= 0; i--) {
            // dirty hack
            if (i >= (int)planes.size())
                i = 0;
            auto planeImg = QImage((const unsigned char *)planes[i].data,
                                   (int)planes[i].cols,
                                   (int)planes[i].rows,
                                   (int)planes[i].step,
                                   QImage::Format_Indexed8)
                                .copy();
            planeImg.setColorSpace(screenColorSpace);
            mImgs[idx] = planeImg;
            idx++;
        }
        // The first element in the vector contains the gray scale 'average' of the 3 channels:
        cv::Mat grayMat;
        cv::cvtColor(imgUC3, grayMat, CV_BGR2GRAY);
        auto grayImg = QImage((const unsigned char *)grayMat.data,
                              (int)grayMat.cols,
                              (int)grayMat.rows,
                              (int)grayMat.step,
                              QImage::Format_Indexed8)
                           .copy();
        grayImg.setColorSpace(screenColorSpace);
        mImgs[0] = grayImg;
        planes.clear();
    }
#else

    else {
        mDrawFalseColorImg = false;
        emit imageModeSet(mode_invalid_format);
        return;
    }

#endif

    // images with valid color table return img.isGrayScale() false...
    if (mSvg || mMovie)
        emit imageModeSet(mode_invalid_format);
    else if (mImgs.size() == 1)
        emit imageModeSet(mode_gray);
    else
        emit imageModeSet(mode_rgb);

    if (mDrawFalseColorImg) // we can skip update if disabled (already handled by parent)
        updateImage(true);
}

void DkViewPortContrast::pickColor(bool enable)
{
    mIsColorPickerActive = enable;
    if (enable) {
        setCursor(Qt::CrossCursor);
    } else {
        unsetCursor();
        emit cancelPickColor();
    }
}

void DkViewPortContrast::updateImage(bool enable)
{
    mDrawFalseColorImg = enable;

    if (enable) {
        QImage falseColorImg = mImgs[mActiveChannel];
        falseColorImg.setColorTable(mColorTable);
        imageVM()->setImageOnly(falseColorImg);
    } else if (imageContainer()) {
        imageVM()->setImageOnly(imageContainer()->image());
        pickColor(false);
    }

    emit viewImageChanged();

    // the histogram normally redraws from imageContainer, we want it to use the grayscale image
    if (mController->getHistogram() && mController->getHistogram()->isVisible()) {
        mController->getHistogram()->drawHistogram(getImage());
    }

    update();
}

void DkViewPortContrast::mousePressEvent(QMouseEvent *event)
{
    if (!mIsColorPickerActive)
        DkViewPort::mousePressEvent(event); // just propagate events, if the color picker is not active
}

void DkViewPortContrast::mouseMoveEvent(QMouseEvent *event)
{
    if (!mIsColorPickerActive)
        DkViewPort::mouseMoveEvent(event); // just propagate events, if the color picker is not active
    else if (DkStatusBarManager::instance().statusbar()->isVisible())
        getPixelInfo(event->pos());
}

void DkViewPortContrast::mouseReleaseEvent(QMouseEvent *event)
{
    if (!mIsColorPickerActive) {
        DkViewPort::mouseReleaseEvent(event);
        return;
    }

    QPoint p = mapToImage(event->position().toPoint());
    if (p.x() < 0 || p.y() < 0)
        return;

    int colorIdx = mImgs[mActiveChannel].pixelIndex(p);
    auto normedPos = (qreal)colorIdx / 255.0;
    emit tFSliderAdded(normedPos);
}

void DkViewPortContrast::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!mIsColorPickerActive)
        DkViewPort::mouseDoubleClickEvent(event);
    else
        event->accept();
}

void DkViewPortContrast::keyPressEvent(QKeyEvent *event)
{
    if (mIsColorPickerActive && (event->key() == Qt::Key_Escape))
        pickColor(false);
    else
        DkViewPort::keyPressEvent(event);
}

QImage DkViewPortContrast::getImage() const
{
    if (mDrawFalseColorImg)
        return imageVM()->image();
    else
        return DkViewPort::getImage();
}
}
