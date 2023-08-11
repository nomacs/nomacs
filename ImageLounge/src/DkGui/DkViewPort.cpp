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
#include "DkBasicLoader.h"
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
#include "DkWidgets.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QDrag>
#include <QDragLeaveEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMovie>
#include <QSvgRenderer>
#include <QVBoxLayout>
#include <QtConcurrentRun>

#include <qmath.h>
#pragma warning(pop) // no warnings from includes - end

#include <assert.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace nmc
{
// DkViewPort --------------------------------------------------------------------
DkViewPort::DkViewPort(QWidget *parent)
    : DkBaseViewPort(parent)
{
    mRepeatZoomTimer = new QTimer(this);
    mAnimationTimer = new QTimer(this);

    // try loading a custom file
    mImgBg.load(QFileInfo(QApplication::applicationDirPath(), "bg.png").absoluteFilePath());
    if (mImgBg.isNull() && DkSettingsManager::param().global().showBgImage) {
        QColor col = backgroundBrush().color().darker();
        mImgBg = DkImage::loadIcon(":/nomacs/img/nomacs-bg.svg", col, QSize(100, 100)).toImage();
    }

    mRepeatZoomTimer->setInterval(20);
    connect(mRepeatZoomTimer, SIGNAL(timeout()), this, SLOT(repeatZoom()));

    mAnimationTimer->setInterval(5);
    connect(mAnimationTimer, SIGNAL(timeout()), this, SLOT(animateFade()));

    // no border
    setMouseTracking(true); // receive mouse event everytime

    mPaintLayout = new QVBoxLayout(this);
    mPaintLayout->setContentsMargins(0, 0, 0, 0);

    createShortcuts();

    mController = new DkControlWidget(this);

    mLoader = QSharedPointer<DkImageLoader>(new DkImageLoader());
    connectLoader(mLoader);

    if (DkSettingsManager::param().display().showScrollBars) {
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

    mController->getOverview()->setTransforms(&mWorldMatrix, &mImgMatrix);
    mController->getCropWidget()->setWorldTransform(&mWorldMatrix);
    mController->getCropWidget()->setImageTransform(&mImgMatrix);
    mController->getCropWidget()->setImageRect(&mImgViewRect);

    // this must be initialized after mController to be above it
    mNavigationWidget = new DkHudNavigation(this);
    mPaintLayout->addWidget(mNavigationWidget);
    // TODO: if visible, currently mNavigationWidget eats all mouse events that are supposed for control widget

    // add actions
    DkActionManager &am = DkActionManager::instance();
    addActions(am.fileActions().toList());
    addActions(am.viewActions().toList());
    addActions(am.editActions().toList());
    addActions(am.manipulatorActions().toList());
    addActions(am.sortActions().toList());
    addActions(am.toolsActions().toList());
    addActions(am.panelActions().toList());
    addActions(am.syncActions().toList());
    addActions(am.pluginActions().toList());
    addActions(am.helpActions().toList());
    addActions(am.hiddenActions().toList());

    addActions(am.openWithActions().toList());
#ifdef WITH_PLUGINS
    addActions(am.pluginActionManager()->pluginDummyActions().toList());
#endif

    connect(&mImgStorage, SIGNAL(infoSignal(const QString &)), this, SIGNAL(infoSignal(const QString &)));

    if (am.pluginActionManager())
        connect(am.pluginActionManager(),
                SIGNAL(runPlugin(DkPluginContainer *, const QString &)),
                this,
                SLOT(applyPlugin(DkPluginContainer *, const QString &)));

    // connect
    connect(am.action(DkActionManager::menu_file_reload), SIGNAL(triggered()), this, SLOT(reloadFile()));
    connect(am.action(DkActionManager::menu_file_next), SIGNAL(triggered()), this, SLOT(loadNextFileFast()));
    connect(am.action(DkActionManager::menu_file_prev), SIGNAL(triggered()), this, SLOT(loadPrevFileFast()));
    connect(am.action(DkActionManager::menu_file_save), SIGNAL(triggered()), this, SLOT(saveFile()));
    connect(am.action(DkActionManager::menu_file_save_as), SIGNAL(triggered()), this, SLOT(saveFileAs()));
    connect(am.action(DkActionManager::menu_file_save_web), SIGNAL(triggered()), this, SLOT(saveFileWeb()));
    connect(am.action(DkActionManager::menu_tools_wallpaper), SIGNAL(triggered()), this, SLOT(setAsWallpaper()));

    connect(am.action(DkActionManager::menu_edit_rotate_cw), SIGNAL(triggered()), this, SLOT(rotateCW()));
    connect(am.action(DkActionManager::menu_edit_rotate_ccw), SIGNAL(triggered()), this, SLOT(rotateCCW()));
    connect(am.action(DkActionManager::menu_edit_rotate_180), SIGNAL(triggered()), this, SLOT(rotate180()));
    connect(am.action(DkActionManager::menu_edit_transform), SIGNAL(triggered()), this, SLOT(resizeImage()));
    connect(am.action(DkActionManager::menu_edit_delete), SIGNAL(triggered()), this, SLOT(deleteImage()));
    connect(am.action(DkActionManager::menu_edit_copy), SIGNAL(triggered()), this, SLOT(copyImage()));
    connect(am.action(DkActionManager::menu_edit_copy_buffer), SIGNAL(triggered()), this, SLOT(copyImageBuffer()));
    connect(am.action(DkActionManager::menu_edit_copy_color), SIGNAL(triggered()), this, SLOT(copyPixelColorValue()));

    connect(am.action(DkActionManager::menu_view_reset), SIGNAL(triggered()), this, SLOT(zoomToFit()));
    connect(am.action(DkActionManager::menu_view_100), SIGNAL(triggered()), this, SLOT(fullView()));
    connect(am.action(DkActionManager::menu_view_zoom_in), SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(am.action(DkActionManager::menu_view_zoom_out), SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(am.action(DkActionManager::menu_view_tp_pattern), SIGNAL(toggled(bool)), this, SLOT(togglePattern(bool)));
    connect(am.action(DkActionManager::menu_view_movie_pause), SIGNAL(triggered(bool)), this, SLOT(pauseMovie(bool)));
    connect(am.action(DkActionManager::menu_view_movie_prev), SIGNAL(triggered()), this, SLOT(previousMovieFrame()));
    connect(am.action(DkActionManager::menu_view_movie_next), SIGNAL(triggered()), this, SLOT(nextMovieFrame()));

    connect(am.action(DkActionManager::sc_test_img), SIGNAL(triggered()), this, SLOT(loadLena()));
    connect(am.action(DkActionManager::menu_sync_view), SIGNAL(triggered()), this, SLOT(tcpForceSynchronize()));

    // playing
    connect(mNavigationWidget, SIGNAL(previousSignal()), this, SLOT(loadPrevFileFast()));
    connect(mNavigationWidget, SIGNAL(nextSignal()), this, SLOT(loadNextFileFast()));

    // trivial connects
    connect(this, &DkViewPort::movieLoadedSignal, [this](bool movie) {
        DkActionManager::instance().enableMovieActions(movie);
    });

    // connect sync
    auto cm = DkSyncManager::inst().client();

    connect(this, SIGNAL(sendTransformSignal(QTransform, QTransform, QPointF)), cm, SLOT(sendTransform(QTransform, QTransform, QPointF)));
    connect(this, SIGNAL(sendNewFileSignal(qint16, const QString &)), cm, SLOT(sendNewFile(qint16, const QString &)));
    connect(cm, SIGNAL(receivedNewFile(qint16, const QString &)), this, SLOT(tcpLoadFile(qint16, const QString &)));
    connect(cm, SIGNAL(updateConnectionSignal(const QString &)), mController, SLOT(setInfo(const QString &)));
    connect(cm, SIGNAL(receivedTransformation(QTransform, QTransform, QPointF)), this, SLOT(tcpSetTransforms(QTransform, QTransform, QPointF)));

    for (auto action : am.manipulatorActions())
        connect(action, SIGNAL(triggered()), this, SLOT(applyManipulator()));

    connect(&mManipulatorWatcher, SIGNAL(finished()), this, SLOT(manipulatorApplied()));

    // TODO:
    // one could blur the canvas if a transparent GUI is present
    // what we would need: QGraphicsBlurEffect...
    // render all widgets to the alpha channel (bw)
    // pre-render the mViewport to that image... apply blur
    // and then render the blurred image after the widget is rendered...
    // performance?!
}

DkViewPort::~DkViewPort()
{
    mController->closePlugin(false, true);

    mManipulatorWatcher.cancel();
    mManipulatorWatcher.blockSignals(true);
}

void DkViewPort::createShortcuts()
{
    DkActionManager &am = DkActionManager::instance();
    connect(am.action(DkActionManager::sc_first_file), SIGNAL(triggered()), this, SLOT(loadFirst()));
    connect(am.action(DkActionManager::sc_last_file), SIGNAL(triggered()), this, SLOT(loadLast()));
    connect(am.action(DkActionManager::sc_skip_prev), SIGNAL(triggered()), this, SLOT(loadSkipPrev10()));
    connect(am.action(DkActionManager::sc_skip_next), SIGNAL(triggered()), this, SLOT(loadSkipNext10()));
    connect(am.action(DkActionManager::sc_first_file_sync), SIGNAL(triggered()), this, SLOT(loadFirst()));
    connect(am.action(DkActionManager::sc_last_file_sync), SIGNAL(triggered()), this, SLOT(loadLast()));
    connect(am.action(DkActionManager::sc_skip_next_sync), SIGNAL(triggered()), this, SLOT(loadNextFileFast()));
    connect(am.action(DkActionManager::sc_skip_prev_sync), SIGNAL(triggered()), this, SLOT(loadPrevFileFast()));
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
}

#ifdef WITH_OPENCV
void DkViewPort::setImage(cv::Mat newImg)
{
    QImage imgQt = DkImage::mat2QImage(newImg);
    setImage(imgQt);
}
#endif

void DkViewPort::updateImage(QSharedPointer<DkImageContainerT> image, bool loaded)
{
    // things todo if a file was not loaded...
    if (!loaded) {
        mController->getPlayer()->startTimer();
        return;
    }

    // should not happen -> the mLoader should send this signal
    if (!mLoader)
        return;

    if (mLoader->hasImage()) {
        setImage(mLoader->getPixmap()); // modified image (for view), may differ from lastImage after rotate
    }

    emit imageUpdatedSignal();
}

void DkViewPort::setImageUpdated()
{
    if (!mLoader)
        return;
    mLoader->setImageUpdated();
}

void DkViewPort::loadImage(const QImage &newImg)
{
    // delete current information
    if (mLoader) {
        if (!unloadImage(true))
            return; // user canceled

        mLoader->setImage(newImg, tr("Original Image"));
        setImage(newImg);

        // save to temp folder
        mLoader->saveTempFile(newImg);
    }
}

void DkViewPort::loadImage(QSharedPointer<DkImageContainerT> img)
{
    if (mLoader) {
        if (!unloadImage(true))
            return;

        if (img->hasImage()) {
            mLoader->setCurrentImage(img);
            setImage(img->image());
        }
        mLoader->load(img);
    }
}

void DkViewPort::setImage(QImage newImg)
{
    // calling show here fixes issues with the HUD
    show();

    DkTimer dt;

    emit movieLoadedSignal(false);
    stopMovie(); // just to be sure

    if (mManipulatorWatcher.isRunning())
        mManipulatorWatcher.cancel();

    mController->getOverview()->setImage(QImage()); // clear overview

    mImgStorage.setImage(newImg);

    if (mLoader->hasMovie() && !mLoader->isEdited())
        loadMovie();
    if (mLoader->hasSvg() && !mLoader->isEdited())
        loadSvg();

    mImgRect = QRectF(QPoint(), getImageSize());

    DkActionManager::instance().enableImageActions(!newImg.isNull());
    mController->imageLoaded(!newImg.isNull());

    double oldZoom = mWorldMatrix.m11(); // *mImgMatrix.m11();

    if (!(DkSettingsManager::param().display().keepZoom == DkSettings::zoom_keep_same_size && mOldImgRect == mImgRect))
        mWorldMatrix.reset();

    updateImageMatrix();

    // if image is not inside, we'll align it at the top left border
    if (!mViewportRect.intersects(mWorldMatrix.mapRect(mImgViewRect))) {
        mWorldMatrix.translate(-mWorldMatrix.dx(), -mWorldMatrix.dy());
        centerImage();
    }

    if (DkSettingsManager::param().display().keepZoom == DkSettings::zoom_always_keep) {
        zoomToPoint(oldZoom, mImgViewRect.center().toPoint(), mWorldMatrix);
    }

    mController->getPlayer()->startTimer();
    mController->getOverview()->setImage(newImg); // TODO: maybe we could make use of the image pyramid here

    mOldImgRect = mImgRect;

    // init fading
    if (DkSettingsManager::param().display().animationDuration && DkSettingsManager::param().display().transition != DkSettingsManager::param().trans_appear
        && (mController->getPlayer()->isPlaying() || DkUtils::getMainWindow()->isFullScreen() || DkSettingsManager::param().display().alwaysAnimate)) {
        mAnimationTimer->start();
        mAnimationTime.start();
    } else
        mAnimationValue = 0.0f;

    // set/clear crop rect
    if (mLoader->getCurrentImage())
        mCropRect = mLoader->getCurrentImage()->cropRect();
    else
        mCropRect = DkRotatingRect();

    update();

    // draw a histogram from the image -> does nothing if the histogram is invisible
    if (mController->getHistogram())
        mController->getHistogram()->drawHistogram(newImg);

    emit newImageSignal(&newImg);
    emit zoomSignal(mWorldMatrix.m11() * mImgMatrix.m11() * 100);

    // status info
    if (!newImg.isNull()) {
        DkStatusBarManager::instance().setMessage(QString::number(qRound((float)(mWorldMatrix.m11() * mImgMatrix.m11() * 100))) + "%",
                                                  DkStatusBar::status_zoom_info);
        DkStatusBarManager::instance().setMessage(DkUtils::formatToString(newImg.format()), DkStatusBar::status_format_info);
        DkStatusBarManager::instance().setMessage(QString::number(newImg.width()) + " x " + QString::number(newImg.height()),
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

void DkViewPort::zoom(double factor, const QPointF &center, bool force)
{
    if (mImgStorage.isEmpty() || mBlockZooming)
        return;

    // factor/=5;//-0.1 <-> 0.1
    // factor+=1;//0.9 <-> 1.1

    // limit zoom out ---
    if (mWorldMatrix.m11() * factor < mMinZoom && factor < 1)
        return;

    // reset view & block if we pass the 'image fit to screen' on zoom out
    if (mWorldMatrix.m11() > 1 && mWorldMatrix.m11() * factor < 1 && !force) {
        mBlockZooming = true;
        mZoomTimer->start(500);
        resetView();
        return;
    }

    // reset view if we pass the 'image fit to screen' on zoom in
    if (mWorldMatrix.m11() < 1 && mWorldMatrix.m11() * factor > 1 && !force) {
        resetView();
        return;
    }

    // TODO: the reset in mWorldMatrix introduces wrong pans
    //// reset view & block if we pass the '100%' on zoom out
    // if (mWorldMatrix.m11()*mImgMatrix.m11()-FLT_EPSILON > 1 && mWorldMatrix.m11()*mImgMatrix.m11()*factor < 1) {
    //
    //	mBlockZooming = true;
    //	mZoomTimer->start(500);
    //	mWorldMatrix.reset();
    //	factor = 1.0f / (float)mImgMatrix.m11();
    // }

    //// reset view if we pass the '100%' on zoom in
    // if (mWorldMatrix.m11()*mImgMatrix.m11()+FLT_EPSILON < 1 && mWorldMatrix.m11()*mImgMatrix.m11()*factor > 1) {

    //	mBlockZooming = true;
    //	mZoomTimer->start(500);
    //	mWorldMatrix.reset();
    //	factor = 1.0f / (float)mImgMatrix.m11();
    //}

    // limit zoom in ---
    if (mWorldMatrix.m11() * mImgMatrix.m11() > mMaxZoom && factor > 1)
        return;

    bool blackBorder = false;

    QPointF pos = center;

    // if no center assigned: zoom in at the image center
    if (pos.x() == -1 || pos.y() == -1)
        pos = mImgViewRect.center();
    else {
        // if black border - do not zoom to the mouse coordinate
        if (mImgViewRect.width() * (mWorldMatrix.m11() * factor) < width()) {
            pos.setX(mImgViewRect.center().x());
            blackBorder = true;
        }
        if ((mImgViewRect.height() * mWorldMatrix.m11() * factor) < height()) {
            pos.setY(mImgViewRect.center().y());
            blackBorder = true;
        }
    }

    zoomToPoint(factor, pos, mWorldMatrix);

    controlImagePosition();
    if (blackBorder && factor < 1)
        centerImage(); // TODO: geht auch schöner
    showZoom();
    changeCursor();

    mController->update(); // why do we need to update the mController manually?
    update();

    tcpSynchronize();

    emit zoomSignal(mWorldMatrix.m11() * mImgMatrix.m11() * 100);
    DkStatusBarManager::instance().setMessage(QString::number(qRound(mWorldMatrix.m11() * mImgMatrix.m11() * 100)) + "%", DkStatusBar::status_zoom_info);
}

void DkViewPort::zoomTo(double zoomLevel)
{
    mWorldMatrix.reset();
    zoom(zoomLevel / mImgMatrix.m11());
}

void DkViewPort::zoomToFit()
{
    QSizeF imgSize = getImageSize();
    QSizeF winSize = size();
    double zoomLevel = qMin(winSize.width() / imgSize.width(), winSize.height() / imgSize.height());

    if (zoomLevel > 1)
        zoomTo(zoomLevel);
    else if (zoomLevel < 1)
        resetView();
    else if (zoomLevel == 1 && mLoader && mLoader->hasSvg())
        resetView();
}

void DkViewPort::resetView()
{
    mWorldMatrix.reset();
    showZoom();
    changeCursor();

    update();
    controlImagePosition();

    emit zoomSignal(mWorldMatrix.m11() * mImgMatrix.m11() * 100);
    tcpSynchronize();
}

void DkViewPort::fullView()
{
    QPointF p = mViewportRect.center();
    zoom(1.0 / (mImgMatrix.m11() * mWorldMatrix.m11()), p.toPoint(), true);

    emit zoomSignal(mWorldMatrix.m11() * mImgMatrix.m11() * 100);
    changeCursor();
    update();
}

void DkViewPort::showZoom()
{
    // don't show zoom if we are in fullscreen mode
    if (isFullScreen() || DkSettingsManager::param().app().hideAllPanels)
        return;

    QString zoomStr = QString::asprintf("%.1f%%", mImgMatrix.m11() * mWorldMatrix.m11() * 100);

    if (!mController->getZoomWidget()->isVisible())
        mController->setInfo(zoomStr, 3000, DkControlWidget::bottom_left_label);
}

void DkViewPort::repeatZoom()
{
    qDebug() << "repeating...";
    if ((DkSettingsManager::param().display().invertZoom && QApplication::mouseButtons() == Qt::XButton1)
        || (!DkSettingsManager::param().display().invertZoom && QApplication::mouseButtons() == Qt::XButton2)) {
        zoom(1.1f);
    } else if ((!DkSettingsManager::param().display().invertZoom && QApplication::mouseButtons() == Qt::XButton1)
               || (DkSettingsManager::param().display().invertZoom && QApplication::mouseButtons() == Qt::XButton2)) {
        zoom(0.9f);
    } else {
        mRepeatZoomTimer->stop(); // safety if we don't catch the release
    }
}

void DkViewPort::toggleResetMatrix()
{
    DkSettingsManager::param().display().keepZoom = !DkSettingsManager::param().display().keepZoom;
}

void DkViewPort::updateImageMatrix()
{
    if (mImgStorage.isEmpty())
        return;

    QRectF oldImgRect = mImgViewRect;
    QTransform oldImgMatrix = mImgMatrix;

    mImgMatrix.reset();

    QSize imgSize = getImageSize();

    // if the image is smaller or zoom is active: paint the image as is
    if (!mViewportRect.contains(mImgRect.toRect()))
        mImgMatrix = getScaledImageMatrix();
    else {
        mImgMatrix.translate((float)(getMainGeometry().width() - imgSize.width()) * 0.5f, (float)(getMainGeometry().height() - imgSize.height()) * 0.5f);
        mImgMatrix.scale(1.0f, 1.0f);
    }

    mImgViewRect = mImgMatrix.mapRect(mImgRect);

    // update world matrix?
    // mWorldMatrix.m11() != 1
    if (qAbs(mWorldMatrix.m11() - 1.0) > 1e-4) {
        float scaleFactor = (float)(oldImgMatrix.m11() / mImgMatrix.m11());
        double dx = oldImgRect.x() / scaleFactor - mImgViewRect.x();
        double dy = oldImgRect.y() / scaleFactor - mImgViewRect.y();

        mWorldMatrix.scale(scaleFactor, scaleFactor);
        mWorldMatrix.translate(dx, dy);
    }
    // NOTE: this is not the same as resetView!
    else if (DkSettingsManager::param().display().zoomToFit)
        zoomToFit();
}

void DkViewPort::tcpSetTransforms(QTransform newWorldMatrix, QTransform newImgMatrix, QPointF canvasSize)
{
    // ok relative transform
    if (canvasSize.isNull()) {
        moveView(QPointF(newWorldMatrix.dx(), newWorldMatrix.dy()) / mWorldMatrix.m11());
    } else {
        mWorldMatrix = newWorldMatrix;
        mImgMatrix = newImgMatrix;
        updateImageMatrix();

        QPointF imgPos = QPointF(canvasSize.x() * getImageSize().width(), canvasSize.y() * getImageSize().height());

        // go to screen coordinates
        imgPos = mImgMatrix.map(imgPos);

        // go to world coordinates
        imgPos = mWorldMatrix.map(imgPos);

        // compute difference to current mViewport center - in world coordinates
        imgPos = QPointF(width() * 0.5f, height() * 0.5f) - imgPos;

        // back to screen coordinates
        float s = (float)mWorldMatrix.m11();
        mWorldMatrix.translate(imgPos.x() / s, imgPos.y() / s);
    }

    update();
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
    if ((force || qApp->keyboardModifiers() == mAltMod || DkSettingsManager::param().sync().syncActions) && (hasFocus() || mController->hasFocus())) {
        QPointF size = QPointF(geometry().width() / 2.0f, geometry().height() / 2.0f);
        size = mWorldMatrix.inverted().map(size);
        size = mImgMatrix.inverted().map(size);
        size = QPointF(size.x() / (float)getImageSize().width(), size.y() / (float)getImageSize().height());

        emit sendTransformSignal(mWorldMatrix, mImgMatrix, size);
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

    QSharedPointer<DkImageContainerT> result = DkImageContainerT::fromImageContainer(plugin->plugin()->runPlugin(key, imageContainer()));
    if (result)
        setEditedImage(result);

    plugin->setActive(false);
#else
    Q_UNUSED(plugin);
    Q_UNUSED(key);
#endif
}

QImage DkViewPort::getImage() const
{
    if (imageContainer() && (!mSvg || !mSvg->isValid()) && (!mMovie || !mMovie->isValid()))
        return imageContainer()->image();

    return DkBaseViewPort::getImage();
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
        mResizeDialog->setExifDpi((float)res.x());
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

    DkMessageBox *msgBox =
        new DkMessageBox(QMessageBox::Question, tr("Delete File"), question, (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), this);

    msgBox->setDefaultButton(QMessageBox::Yes);
    msgBox->setObjectName("deleteFileDialog");

    int answer = msgBox->exec();

    if (answer == QMessageBox::Accepted || answer == QMessageBox::Yes) {
        stopMovie(); // movies keep file handles so stop it before we can delete files

        if (!mLoader->deleteFile())
            loadMovie(); // load the movie again, if we could not delete it
    }
}

void DkViewPort::saveFile()
{
    saveFileAs(true);
}

void DkViewPort::saveFileAs(bool silent)
{
    if (mLoader) {
        mController->closePlugin(false);

        QImage img = getImage();

        if (mLoader->hasSvg() && !mLoader->isEdited()) {
            DkSvgSizeDialog *sd = new DkSvgSizeDialog(img.size(), DkUtils::getMainWindow());
            sd->resize(270, 120);

            int answer = sd->exec();

            if (answer == QDialog::Accepted) {
                img = QImage(sd->size(), QImage::Format_ARGB32);
                img.fill(QColor(0, 0, 0, 0));

                QPainter p(&img);
                mSvg->render(&p, QRectF(QPointF(), sd->size()));
            }
        }

        mLoader->saveUserFileAs(img, silent);
    }
}

void DkViewPort::saveFileWeb()
{
    if (mLoader) {
        mController->closePlugin(false);
        mLoader->saveFileWeb(getImage());
    }
}

void DkViewPort::setAsWallpaper()
{
    // based on code from: http://qtwiki.org/Set_windows_background_using_QT
    auto imgC = imageContainer();

    if (!imgC || !imgC->hasImage()) {
        qWarning() << "cannot create wallpaper because there is no image loaded...";
    }

    QImage img = imgC->image();
    QString tmpPath = mLoader->saveTempFile(img, "wallpaper", ".jpg", true, false);

    // is there a more elegant way to see if saveTempFile returned an empty path
    if (tmpPath.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Sorry, I could not create a wallpaper..."));
        return;
    }

#ifdef Q_OS_WIN

    // Read current windows background image path
    QSettings appSettings("HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    appSettings.setValue("Wallpaper", tmpPath);

    QByteArray ba = tmpPath.toLatin1();
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void *)ba.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#endif
    // TODO: add functionality for unix based systems
}

void DkViewPort::applyManipulator()
{
    QAction *action = dynamic_cast<QAction *>(QObject::sender());

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

    // try to cast up
    QSharedPointer<DkBaseManipulatorExt> mplExt = qSharedPointerDynamicCast<DkBaseManipulatorExt>(mpl);

    // mark dirty
    if (mManipulatorWatcher.isRunning() && mplExt && mActiveManipulator == mpl) {
        mplExt->setDirty(true);
        return;
    }

    if (mManipulatorWatcher.isRunning()) {
        mController->setInfo(tr("Busy"));
        return;
    }

    // show the dock (in case it's not shown yet)
    if (mplExt) {
        am.action(DkActionManager::menu_edit_image)->setChecked(true);
    }

    // undo last if it is an extended manipulator
    QImage img;
    if (mplExt && imageContainer()) {
        auto l = imageContainer()->getLoader();
        l->setMinHistorySize(3); // increase the min history size to 3 for correctly popping back
        if (!l->history()->isEmpty() && l->lastEdit().editName() == mplExt->name()) {
            imageContainer()->undo();
        }

        img = imageContainer()->image();
    } else
        img = getImage();

    mManipulatorWatcher.setFuture(QtConcurrent::run(mpl.data(), &nmc::DkBaseManipulator::apply, img));

    mActiveManipulator = mpl;

    emit showProgress(true, 500);
}

void DkViewPort::manipulatorApplied()
{
    DkGlobalProgress::instance().stop();

    if (mManipulatorWatcher.isCanceled() || !mActiveManipulator) {
        qDebug() << "manipulator applied - but it's canceled";
        return;
    }

    // trigger again if it's dirty
    QSharedPointer<DkBaseManipulatorExt> mplExt = qSharedPointerDynamicCast<DkBaseManipulatorExt>(mActiveManipulator);

    // set the edited image
    QImage img = mManipulatorWatcher.result();

    if (!img.isNull())
        setEditedImage(img, mActiveManipulator->name());
    else
        mController->setInfo(mActiveManipulator->errorMessage());

    if (mplExt && mplExt->isDirty()) {
        mplExt->setDirty(false);
        mplExt->action()->trigger();
        qDebug() << "triggering manipulator - it's dirty";
    }

    emit showProgress(false);
}

void DkViewPort::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());

    if (!mImgStorage.isEmpty()) {
        // usually the QGraphicsView should do this - but we have seen issues(e.g. #706)
        painter.setPen(Qt::NoPen);
        painter.setBrush(backgroundBrush());
        painter.drawRect(QRect(QPoint(), size()));

        painter.setWorldTransform(mWorldMatrix);

        // interpolate between 100% and max interpolate level
        if (!mForceFastRendering && // force?
            mImgMatrix.m11() * mWorldMatrix.m11() - DBL_EPSILON > 1.0 && // @100% ?
            mImgMatrix.m11() * mWorldMatrix.m11() <= DkSettingsManager::param().display().interpolateZoomLevel / 100.0) { // > max zoom level
            painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
        }

        if (DkSettingsManager::param().display().transition == DkSettings::trans_swipe && !mAnimationBuffer.isNull()) {
            double dx = mNextSwipe ? width() * (mAnimationValue) : -width() * (mAnimationValue);

            QTransform swipeTransform;
            swipeTransform.translate(dx, 0);
            painter.setTransform(swipeTransform);
        }

        // TODO: if fading is active we interpolate with background instead of the other image
        double opacity = (DkSettingsManager::param().display().transition == DkSettings::trans_fade) ? 1.0 - mAnimationValue : 1.0;
        draw(painter, opacity);

        if (!mAnimationBuffer.isNull() && mAnimationValue > 0) {
            float oldOp = (float)painter.opacity();

            // fade transition
            if (DkSettingsManager::param().display().transition == DkSettings::trans_fade) {
                painter.setOpacity(mAnimationValue);
            } else if (DkSettingsManager::param().display().transition == DkSettings::trans_swipe) {
                double dx = mNextSwipe ? -width() * (1.0 - mAnimationValue) : width() * (1.0 - mAnimationValue);
                QTransform swipeTransform;
                swipeTransform.translate(dx, 0);
                painter.setTransform(swipeTransform);
            }

            painter.drawImage(mFadeImgViewRect, mAnimationBuffer, mAnimationBuffer.rect());
            painter.setOpacity(oldOp);
        }

        // now disable world matrix for overlay display
        painter.setWorldMatrixEnabled(false);
    } else
        drawBackground(painter);

    // draw the cropping rect
    if (!mCropRect.isEmpty() && DkSettingsManager::param().display().showCrop && imageContainer()) {
        // create path
        QPainterPath path;
        path.addRect(getImageViewRect().toRect());

        DkRotatingRect r = mCropRect;
        QPolygonF polyF;
        polyF = r.getClosedPoly();
        polyF = mImgMatrix.map(polyF);
        polyF = mWorldMatrix.map(polyF);
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
    mNavigationWidget->hide();
    DkBaseViewPort::leaveEvent(event);
}

// drawing functions --------------------------------------------------------------------
void DkViewPort::drawBackground(QPainter &painter)
{
    // fit to mViewport
    QSize s = mImgBg.size();
    if (s.width() > (float)(size().width() * 0.5))
        s = s * ((size().width() * 0.5) / s.width());

    if (s.height() > size().height() * 0.6)
        s = s * ((size().height() * 0.6) / s.height());

    QRect bgRect(QPoint(), s);
    bgRect.moveBottomRight(QPoint(width() - 20, height() - 20));

    painter.drawImage(bgRect, mImgBg, QRect(QPoint(), mImgBg.size()));
}

void DkViewPort::loadMovie()
{
    if (!mLoader)
        return;

    if (mMovie)
        mMovie->stop();

    // check if it truely a movie (we need this for we don't know if webp is actually animated)
    QSharedPointer<QMovie> m(new QMovie(mLoader->filePath()));
    if (m->frameCount() == 1)
        return;

    mMovie = m;

    connect(mMovie.data(), SIGNAL(frameChanged(int)), this, SLOT(update()));
    mMovie->start();

    emit movieLoadedSignal(true);
}

void DkViewPort::loadSvg()
{
    if (!mLoader)
        return;

    auto cc = mLoader->getCurrentImage();
    if (cc) {
        mSvg = QSharedPointer<QSvgRenderer>(new QSvgRenderer(*cc->getFileBuffer()));
    } else {
        mSvg = QSharedPointer<QSvgRenderer>(new QSvgRenderer(mLoader->filePath()));
    }

    connect(mSvg.data(), SIGNAL(repaintNeeded()), this, SLOT(update()));
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

    mMovie->jumpToNextFrame();
    update();
}

void DkViewPort::previousMovieFrame()
{
    if (!mMovie)
        return;

    int fn = mMovie->currentFrameNumber() - 1;
    if (fn == -1)
        fn = mMovie->frameCount() - 1;
    // qDebug() << "retrieving frame: " << fn;

    while (mMovie->currentFrameNumber() != fn)
        mMovie->jumpToNextFrame();

    //// the subsequent thing is not working if the movie is paused
    // bool success = movie->jumpToFrame(movie->currentFrameNumber()-1);
    update();
}

void DkViewPort::stopMovie()
{
    if (!mMovie)
        return;

    mMovie->stop();
    mMovie = QSharedPointer<QMovie>();
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
    mViewportRect = QRect(0, 0, width(), height());

    // >DIR: diem - bug if zoom factor is large and window becomes small
    updateImageMatrix();
    centerImage();
    changeCursor();

    mController->getOverview()->setViewPortRect(geometry());
    mController->resize(width(), height());

    return QGraphicsView::resizeEvent(event);
}

// mouse events --------------------------------------------------------------------
bool DkViewPort::event(QEvent *event)
{
    // ok obviously QGraphicsView eats all mouse events -> so we simply redirect these to QWidget in order to get them delivered here
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick || event->type() == QEvent::MouseButtonRelease
        || event->type() == QEvent::MouseMove || event->type() == QEvent::Wheel || event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease
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
        repeatZoom();
        mRepeatZoomTimer->start();
    }

    // ok, start panning
    if (mWorldMatrix.m11() > 1 && !imageInside() && event->buttons() == Qt::LeftButton) {
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

void DkViewPort::mouseMoveEvent(QMouseEvent *event)
{
    if (DkSettingsManager::param().display().showNavigation && event->modifiers() == Qt::NoModifier && event->buttons() == Qt::NoButton) {
        int left = qMin(100, qRound(0.1 * width()));
        int right = qMax(width() - 100, qRound(0.9 * width()));

        if (event->pos().x() < left)
            mNavigationWidget->showPrevious();
        else if (event->pos().x() > right)
            mNavigationWidget->showNext();
        else if (mNavigationWidget->isVisible())
            mNavigationWidget->hide();
    }

    // qDebug() << "mouse move (DkViewPort)";
    // changeCursor();
    mCurrentPixelPos = event->pos();

    if (DkStatusBarManager::instance().statusbar()->isVisible())
        getPixelInfo(event->pos());

    if (mWorldMatrix.m11() > 1 && event->buttons() == Qt::LeftButton) {
        QPointF cPos = event->pos();
        QPointF dxy = (cPos - mPosGrab);
        mPosGrab = cPos;
        moveView(dxy / mWorldMatrix.m11());

        // with shift also a hotkey for fast switching...
        if ((DkSettingsManager::param().sync().syncAbsoluteTransform && event->modifiers() == (mAltMod | Qt::ShiftModifier))
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
    if (event->buttons() == Qt::LeftButton && dist > QApplication::startDragDistance() && imageInside() && !getImage().isNull() && mLoader
        && !QApplication::widgetAt(event->globalPos())) { // is NULL if the mouse leaves the window

        QMimeData *mimeData = createMime();

        QPixmap pm;
        if (!getImage().isNull())
            pm = QPixmap::fromImage(mImgStorage.image().scaledToHeight(73, Qt::SmoothTransformation));
        if (pm.width() > 130)
            pm = pm.scaledToWidth(100, Qt::SmoothTransformation);

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(pm);
        drag->exec(Qt::CopyAction);
    }

    // send to parent
    DkBaseViewPort::mouseMoveEvent(event);
}

void DkViewPort::wheelEvent(QWheelEvent *event)
{
    if ((!DkSettingsManager::param().global().zoomOnWheel && event->modifiers() != mCtrlMod)
        || (DkSettingsManager::param().global().zoomOnWheel
            && (event->modifiers() & mCtrlMod
                || (DkSettingsManager::param().global().horZoomSkips && event->orientation() == Qt::Horizontal && !(event->modifiers() & mAltMod))))) {
        auto delta = event->angleDelta().y();
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
    DkVector vec((float)(start.x() - end.x()), (float)(start.y() - end.y()));

    if (fabs(vec.norm()) < 100)
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
    else if (!horizontal && startPos.y() < height() * 0.5f) {
        // downward gesture is opening
        if (vec.y > 0)
            return open_thumbs;
        else
            return close_thumbs;
    }
    // lower part of the canvas is thumbs
    else if (!horizontal && startPos.y() > height() * 0.5f) {
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
    assert(mController);

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
    assert(mController);
    mController->setFullScreen(fullScreen);
    toggleLena(fullScreen);

    if (fullScreen)
        QWidget::setWindowState(windowState() ^ Qt::WindowFullScreen);
    else
        QWidget::setWindowState(windowState() & ~Qt::WindowFullScreen);

    if (fullScreen)
        mHideCursorTimer->start();
    else
        unsetCursor();
}

QPoint DkViewPort::mapToImage(const QPoint &windowPos) const
{
    QPointF imgPos = mWorldMatrix.inverted().map(QPointF(windowPos));
    imgPos = mImgMatrix.inverted().map(imgPos);

    QPoint xy(qFloor(imgPos.x()), qFloor(imgPos.y()));

    if (xy.x() < 0 || xy.y() < 0 || xy.x() >= getImageSize().width() || xy.y() >= getImageSize().height())
        return QPoint(-1, -1);

    return xy;
}

void DkViewPort::getPixelInfo(const QPoint &pos)
{
    if (mImgStorage.isEmpty())
        return;

    QPoint xy = mapToImage(pos);

    if (xy.x() == -1 || xy.y() == -1)
        return;

    QColor col = getImage().pixel(xy);

    QString msg = "x: " + QString::number(xy.x()) + " y: " + QString::number(xy.y()) + " | r: " + QString::number(col.red())
        + " g: " + QString::number(col.green()) + " b: " + QString::number(col.blue());

    if (mImgStorage.image().hasAlphaChannel())
        msg += " a: " + QString::number(col.alpha());

    msg += " | " + col.name().toUpper();

    DkStatusBarManager::instance().setMessage(msg, DkStatusBar::status_pixel_info);
}

QString DkViewPort::getCurrentPixelHexValue()
{
    if (mImgStorage.isEmpty() || mCurrentPixelPos.isNull())
        return QString();

    QPointF imgPos = mWorldMatrix.inverted().map(QPointF(mCurrentPixelPos));
    imgPos = mImgMatrix.inverted().map(imgPos);

    QPoint xy(qFloor(imgPos.x()), qFloor(imgPos.y()));

    if (xy.x() < 0 || xy.y() < 0 || xy.x() >= getImageSize().width() || xy.y() >= getImageSize().height())
        return QString();

    QColor col = getImage().pixel(xy);

    return col.name().toUpper().remove(0, 1);
}

// Copy & Paste --------------------------------------------------------
void DkViewPort::copyPixelColorValue()
{
    if (getImage().isNull())
        return;

    QMimeData *mimeData = new QMimeData;

    if (!getImage().isNull())
        mimeData->setText(getCurrentPixelHexValue());

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);
}

void DkViewPort::copyImage()
{
    QMimeData *mimeData = createMime();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);
}

QMimeData *DkViewPort::createMime() const
{
    if (getImage().isNull() || !mLoader)
        return 0;

    // NOTE: if we do the file:/// thingy, we will get into problems with mounted drives (e.g. //hermes...)
    QUrl fileUrl = QUrl::fromLocalFile(mLoader->filePath());

    QList<QUrl> urls;
    urls.append(fileUrl);

    QMimeData *mimeData = new QMimeData;

    if (QFileInfo(mLoader->filePath()).exists() && !mLoader->isEdited()) {
        mimeData->setUrls(urls);
        mimeData->setText(fileUrl.toLocalFile());
    } else if (!getImage().isNull())
        mimeData->setImageData(getImage());

    return mimeData;
}

void DkViewPort::copyImageBuffer()
{
    if (getImage().isNull())
        return;

    QMimeData *mimeData = new QMimeData;

    if (!getImage().isNull())
        mimeData->setImageData(getImage());

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);
}

void DkViewPort::animateFade()
{
    mAnimationValue = 1.0f - (float)(mAnimationTime.elapsed() / 1000.0) / DkSettingsManager::param().display().animationDuration;

    // slow in - slow out
    double speed = mAnimationValue > 0.5 ? fabs(1.0 - mAnimationValue) : fabs(mAnimationValue);
    speed *= .05;

    mAnimationValue += (float)speed;

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

    if (mLoader)
        mLoader->rotateImage(90);
}

void DkViewPort::rotateCCW()
{
    if (!mController->applyPluginChanges(true))
        return;

    if (mLoader)
        mLoader->rotateImage(-90);
}

void DkViewPort::rotate180()
{
    if (!mController->applyPluginChanges(true))
        return;

    if (mLoader)
        mLoader->rotateImage(180);
}

// file handling --------------------------------------------------------------------
void DkViewPort::loadLena()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Lenna"), tr("A remarkable woman"), QLineEdit::Normal, 0, &ok);

    // pass phrase
    if (ok && !text.isEmpty() && text == "lenna") {
        mTestLoaded = true;
        toggleLena(DkUtils::getMainWindow()->isFullScreen());
    } else if (!ok) {
        QMessageBox warningDialog(DkUtils::getMainWindow());
        warningDialog.setIcon(QMessageBox::Warning);
        warningDialog.setText(tr("you cannot cancel this"));
        warningDialog.exec();
        loadLena();
    } else {
        QApplication::beep();

        if (text.isEmpty())
            mController->setInfo(tr("did you understand the brainteaser?"));
        else
            mController->setInfo(tr("%1 is wrong...").arg(text));
    }
}

void DkViewPort::toggleLena(bool fullscreen)
{
    if (!mTestLoaded)
        return;

    if (mLoader) {
        if (fullscreen)
            mLoader->downloadFile(QUrl("http://www.lenna.org/lena_std.tif"));
        else
            mLoader->load(":/nomacs/img/we.jpg");
    }
}

void DkViewPort::settingsChanged()
{
    reloadFile();

    mAltMod = DkSettingsManager::param().global().altMod;
    mCtrlMod = DkSettingsManager::param().global().ctrlMod;

    mController->settingsChanged();
}

void DkViewPort::setEditedImage(const QImage &newImg, const QString &editName)
{
    if (!mController->applyPluginChanges(true)) // user wants to first apply the plugin
        return;

    if (newImg.isNull()) {
        emit infoSignal(tr("Attempted to set NULL image")); // not sure if users understand that
        return;
    }

    if (mManipulatorWatcher.isRunning())
        mManipulatorWatcher.cancel();

    QSharedPointer<DkImageContainerT> imgC = mLoader->getCurrentImage();

    if (!imgC)
        imgC = QSharedPointer<DkImageContainerT>(new DkImageContainerT(""));

    if (!imgC)
        imgC = QSharedPointer<DkImageContainerT>();
    imgC->setImage(newImg, editName);
    unloadImage(false);
    mLoader->setImage(imgC);
    qDebug() << "mLoader gets this size: " << newImg.size();
}

void DkViewPort::setEditedImage(QSharedPointer<DkImageContainerT> img)
{
    if (!img) {
        emit infoSignal(tr("Attempted to set NULL image")); // not sure if users understand that
        return;
    }

    unloadImage(false);
    mLoader->setImage(img);
}

bool DkViewPort::unloadImage(bool fileChange)
{
    if (DkSettingsManager::param().display().animationDuration > 0
        && (mController->getPlayer()->isPlaying() || DkUtils::getMainWindow()->isFullScreen() || DkSettingsManager::param().display().alwaysAnimate)) {
        QRect dr = mWorldMatrix.mapRect(mImgViewRect).toRect();
        mAnimationBuffer = mImgStorage.image(dr.size());
        mFadeImgViewRect = mImgViewRect;
        mFadeImgRect = mImgRect;
        mAnimationValue = 1.0f;
    }

    int success = true;

    if (!mController->applyPluginChanges(true)) // user wants to apply changes first
        return false;

    if (fileChange)
        success = mLoader->unloadFile(); // returns false if the user cancels

    // notify controller
    mController->updateImage(imageContainer());

    if (mMovie && success) {
        mMovie->stop();
        mMovie = QSharedPointer<QMovie>();
    }

    if (mSvg && success)
        mSvg = QSharedPointer<QSvgRenderer>();

    return success != 0;
}

void DkViewPort::deactivate()
{
    setImage(QImage());
}

void DkViewPort::loadFile(const QString &filePath)
{
    if (!unloadImage())
        return;

    mTestLoaded = false;

    if (mLoader && !filePath.isEmpty() && QFileInfo(filePath).isDir()) {
        mLoader->setDir(filePath);
    } else if (mLoader)
        mLoader->load(filePath);

    // diem: I removed this line for a) we don't support remote displays anymore and be: https://github.com/nomacs/nomacs/issues/219
    // qDebug() << "sync mode: " << (DkSettingsManager::param().sync().syncMode == DkSettings::sync_mode_remote_display);
    // if ((qApp->keyboardModifiers() == mAltMod || DkSettingsManager::param().sync().syncMode == DkSettings::sync_mode_remote_display) &&
    //	(hasFocus() || mController->hasFocus()) &&
    //	mLoader->hasFile())
    //	tcpLoadFile(0, filePath);
}

void DkViewPort::reloadFile()
{
    if (mLoader) {
        if (unloadImage())
            mLoader->reloadImage();
    }
}

void DkViewPort::loadFile(int skipIdx)
{
    if (!unloadImage())
        return;

    if (mLoader && !mTestLoaded)
        mLoader->changeFile(skipIdx);

    // alt mod
    if ((qApp->keyboardModifiers() == mAltMod || DkSettingsManager::param().sync().syncActions) && (hasFocus() || mController->hasFocus())) {
        emit sendNewFileSignal((qint16)skipIdx);
        qDebug() << "emitting load next";
    }
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

    QApplication::sendPostedEvents();

    int sIdx = skipIdx;
    QSharedPointer<DkImageContainerT> lastImg;

    for (int idx = 0; idx < mLoader->getImages().size(); idx++) {
        QSharedPointer<DkImageContainerT> imgC = mLoader->getSkippedImage(sIdx);

        if (!imgC)
            break;

        mLoader->setCurrentImage(imgC);

        if (imgC && imgC->getLoadState() != DkImageContainer::exists_not) {
            mLoader->load(imgC);
            break;
        } else if (lastImg == imgC) {
            sIdx += skipIdx; // get me out of endless loops (self referencing shortcuts)
        } else {
            qDebug() << "image does not exist - skipping";
        }

        lastImg = imgC;
    }

    if ((qApp->keyboardModifiers() == mAltMod || DkSettingsManager::param().sync().syncActions) && (hasFocus() || mController->hasFocus())) {
        emit sendNewFileSignal((qint16)skipIdx);
        QCoreApplication::sendPostedEvents();
    }
}

void DkViewPort::loadFirst()
{
    if (!unloadImage())
        return;

    if (mLoader && !mTestLoaded)
        mLoader->firstFile();

    if ((qApp->keyboardModifiers() == mAltMod || DkSettingsManager::param().sync().syncActions) && (hasFocus() || mController->hasFocus()))
        emit sendNewFileSignal(SHRT_MIN);
}

void DkViewPort::loadLast()
{
    if (!unloadImage())
        return;

    if (mLoader && !mTestLoaded)
        mLoader->lastFile();

    if ((qApp->keyboardModifiers() == mAltMod || DkSettingsManager::param().sync().syncActions) && (hasFocus() || mController->hasFocus()))
        emit sendNewFileSignal(SHRT_MAX);
}

void DkViewPort::loadSkipPrev10()
{
    loadFileFast(-DkSettingsManager::param().global().skipImgs);
    // unloadImage();

    // if (mLoader && !testLoaded)
    //	mLoader->changeFile(-DkSettingsManager::param().global().skipImgs, (parent->isFullScreen() && DkSettingsManager::param().slideShow().silentFullscreen));

    if (qApp->keyboardModifiers() == mAltMod && (hasFocus() || mController->hasFocus()))
        emit sendNewFileSignal((qint16)-DkSettingsManager::param().global().skipImgs);
}

void DkViewPort::loadSkipNext10()
{
    loadFileFast(DkSettingsManager::param().global().skipImgs);
    // unloadImage();

    // if (mLoader && !testLoaded)
    //	mLoader->changeFile(DkSettingsManager::param().global().skipImgs, (parent->isFullScreen() && DkSettingsManager::param().slideShow().silentFullscreen));

    if (qApp->keyboardModifiers() == mAltMod && (hasFocus() || mController->hasFocus()))
        emit sendNewFileSignal((qint16)DkSettingsManager::param().global().skipImgs);
}

void DkViewPort::tcpLoadFile(qint16 idx, QString filename)
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
            // if (mLoader) mLoader->loadFileAt(idx);
        }
    } else
        loadFile(filename);

    qDebug() << "loading file: " << filename;

    // DkSettingsManager::param().sync().syncMode = oldMode;
}

QSharedPointer<DkImageContainerT> DkViewPort::imageContainer() const
{
    if (!mLoader)
        return QSharedPointer<DkImageContainerT>();

    return mLoader->getCurrentImage();
}

void DkViewPort::setImageLoader(QSharedPointer<DkImageLoader> newLoader)
{
    mLoader = newLoader;
    connectLoader(newLoader);

    if (mLoader)
        mLoader->activate();
}

void DkViewPort::connectLoader(QSharedPointer<DkImageLoader> loader, bool connectSignals)
{
    assert(mController);

    if (!loader)
        return;

    if (connectSignals) {
        connect(loader.data(),
                SIGNAL(imageLoadedSignal(QSharedPointer<DkImageContainerT>, bool)),
                this,
                SLOT(updateImage(QSharedPointer<DkImageContainerT>, bool)),
                Qt::UniqueConnection);
        connect(loader.data(),
                SIGNAL(imageLoadedSignal(QSharedPointer<DkImageContainerT>)),
                mController->getMetaDataWidget(),
                SLOT(updateMetaData(QSharedPointer<DkImageContainerT>)),
                Qt::UniqueConnection);
        connect(loader.data(),
                SIGNAL(imageLoadedSignal(QSharedPointer<DkImageContainerT>)),
                mController,
                SLOT(updateImage(QSharedPointer<DkImageContainerT>)),
                Qt::UniqueConnection);

        connect(loader.data(),
                SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)),
                this,
                SLOT(updateImage(QSharedPointer<DkImageContainerT>)),
                Qt::UniqueConnection); // update image matrix

        connect(loader.data(),
                SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT>>)),
                mController->getFilePreview(),
                SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT>>)),
                Qt::UniqueConnection);
        connect(loader.data(),
                SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)),
                mController->getFilePreview(),
                SLOT(setFileInfo(QSharedPointer<DkImageContainerT>)),
                Qt::UniqueConnection);

        connect(loader.data(), SIGNAL(showInfoSignal(const QString &, int, int)), mController, SLOT(setInfo(const QString &, int, int)), Qt::UniqueConnection);

        connect(loader.data(), SIGNAL(setPlayer(bool)), mController->getPlayer(), SLOT(play(bool)), Qt::UniqueConnection);

        connect(loader.data(),
                SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT>>)),
                mController->getScroller(),
                SLOT(updateDir(QVector<QSharedPointer<DkImageContainerT>>)),
                Qt::UniqueConnection);
        connect(loader.data(), SIGNAL(imageUpdatedSignal(int)), mController->getScroller(), SLOT(updateFile(int)), Qt::UniqueConnection);
        connect(mController->getScroller(), SIGNAL(valueChanged(int)), loader.data(), SLOT(loadFileAt(int)));
    } else {
        disconnect(loader.data(),
                   SIGNAL(imageLoadedSignal(QSharedPointer<DkImageContainerT>, bool)),
                   this,
                   SLOT(updateImage(QSharedPointer<DkImageContainerT>, bool)));
        disconnect(loader.data(),
                   SIGNAL(imageLoadedSignal(QSharedPointer<DkImageContainerT>)),
                   mController->getMetaDataWidget(),
                   SLOT(updateMetaData(QSharedPointer<DkImageContainerT>)));
        disconnect(loader.data(),
                   SIGNAL(imageLoadedSignal(QSharedPointer<DkImageContainerT>)),
                   mController,
                   SLOT(updateImage(QSharedPointer<DkImageContainerT>)));

        disconnect(loader.data(), SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)), this, SLOT(updateImage(QSharedPointer<DkImageContainerT>)));

        disconnect(loader.data(),
                   SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT>>)),
                   mController->getFilePreview(),
                   SLOT(updateThumbs(QVector<QSharedPointer<DkImageContainerT>>)));
        disconnect(loader.data(),
                   SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)),
                   mController->getFilePreview(),
                   SLOT(setFileInfo(QSharedPointer<DkImageContainerT>)));
        disconnect(loader.data(),
                   SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)),
                   mController->getMetaDataWidget(),
                   SLOT(updateMetaData(QSharedPointer<DkImageContainerT>)));
        disconnect(loader.data(),
                   SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)),
                   mController,
                   SLOT(setFileInfo(QSharedPointer<DkImageContainerT>)));

        disconnect(loader.data(), SIGNAL(showInfoSignal(const QString &, int, int)), mController, SLOT(setInfo(const QString &, int, int)));
        disconnect(loader.data(), SIGNAL(updateSpinnerSignalDelayed(bool, int)), mController, SLOT(setSpinnerDelayed(bool, int)));

        disconnect(loader.data(), SIGNAL(setPlayer(bool)), mController->getPlayer(), SLOT(play(bool)));

        disconnect(loader.data(),
                   SIGNAL(updateDirSignal(QVector<QSharedPointer<DkImageContainerT>>)),
                   mController->getScroller(),
                   SLOT(updateDir(QVector<QSharedPointer<DkImageContainerT>>)));
        disconnect(loader.data(),
                   SIGNAL(imageUpdatedSignal(QSharedPointer<DkImageContainerT>)),
                   mController->getScroller(),
                   SLOT(updateFile(QSharedPointer<DkImageContainerT>)));
    }
}

DkControlWidget *DkViewPort::getController()
{
    return mController;
}

void DkViewPort::cropImage(const DkRotatingRect &rect, const QColor &bgCol, bool cropToMetaData)
{
    QSharedPointer<DkImageContainerT> imgC = mLoader->getCurrentImage();

    if (!imgC) {
        qWarning() << "cannot crop NULL image...";
        return;
    }

    imgC->cropImage(rect, bgCol, cropToMetaData);
    setEditedImage(imgC);
}

// DkViewPortFrameless --------------------------------------------------------------------
DkViewPortFrameless::DkViewPortFrameless(QWidget *parent)
    : DkViewPort(parent)
{
#ifdef Q_OS_MAC
    parent->setAttribute(Qt::WA_MacNoShadow);
#endif

    setAttribute(Qt::WA_TranslucentBackground, true);
    mImgBg.load(QFileInfo(QApplication::applicationDirPath(), "bgf.png").absoluteFilePath());

    if (mImgBg.isNull())
        mImgBg.load(":/nomacs/img/splash-screen.png");

    DkActionManager &am = DkActionManager::instance();
    mStartActions.append(am.action(DkActionManager::menu_file_open));
    mStartActions.append(am.action(DkActionManager::menu_file_open_dir));

    mStartIcons.append(am.icon(DkActionManager::icon_file_open_large));
    mStartIcons.append(am.icon(DkActionManager::icon_file_dir_large));
}

DkViewPortFrameless::~DkViewPortFrameless()
{
}

void DkViewPortFrameless::zoom(double factor, const QPointF &center, bool force)
{
    if (mImgStorage.isEmpty() || mBlockZooming)
        return;

    // limit zoom out ---
    if (mWorldMatrix.m11() * factor <= mMinZoom && factor < 1)
        return;

    // reset view & block if we pass the 'image fit to screen' on zoom out
    if (mWorldMatrix.m11() > 1 && mWorldMatrix.m11() * factor < 1 && !force) {
        mBlockZooming = true;
        mZoomTimer->start(500);
    }

    // limit zoom in ---
    if (mWorldMatrix.m11() * mImgMatrix.m11() > mMaxZoom && factor > 1)
        return;

    QRectF viewRect = mWorldMatrix.mapRect(mImgViewRect);
    QPointF pos = center;

    // if no center assigned: zoom in at the image center
    if (pos.x() == -1 || pos.y() == -1)
        pos = viewRect.center();

    if (pos.x() < viewRect.left())
        pos.setX(viewRect.left());
    else if (pos.x() > viewRect.right())
        pos.setX(viewRect.right());
    if (pos.y() < viewRect.top())
        pos.setY(viewRect.top());
    else if (pos.y() > viewRect.bottom())
        pos.setY(viewRect.bottom());

    zoomToPoint(factor, pos, mWorldMatrix);

    controlImagePosition();
    showZoom();
    changeCursor();

    update();

    tcpSynchronize();
    emit zoomSignal(mWorldMatrix.m11() * mImgMatrix.m11() * 100);
}

void DkViewPortFrameless::resetView()
{
    // maybe we can delete this function...
    DkViewPort::resetView();
}

void DkViewPortFrameless::paintEvent(QPaintEvent *event)
{
    if (!DkUtils::getMainWindow()->isFullScreen()) {
        QPainter painter(viewport());
        painter.setWorldTransform(mWorldMatrix);
        drawFrame(painter);
        painter.end();
    }

    DkViewPort::paintEvent(event);
}

void DkViewPortFrameless::draw(QPainter &painter, double)
{
    if (DkUtils::getMainWindow()->isFullScreen()) {
        QColor col = QColor(0, 0, 0);
        col.setAlpha(150);
        painter.setWorldMatrixEnabled(false);
        painter.fillRect(QRect(QPoint(), size()), col);
        painter.setWorldMatrixEnabled(true);
    }

    if (mSvg && mSvg->isValid()) {
        mSvg->render(&painter, mImgViewRect);
    } else if (mMovie && mMovie->isValid()) {
        painter.drawPixmap(mImgViewRect, mMovie->currentPixmap(), mMovie->frameRect());
    } else {
        QRect displayRect = mWorldMatrix.mapRect(mImgViewRect).toRect();
        QImage img = mImgStorage.image(displayRect.size());

        // opacity == 1.0f -> do not show pattern if we crossfade two images
        if (DkSettingsManager::param().display().tpPattern && img.hasAlphaChannel())
            drawPattern(painter);

        painter.drawImage(mImgViewRect, img, QRect(QPoint(), img.size()));
    }
}

void DkViewPortFrameless::drawBackground(QPainter &painter)
{
    painter.setWorldTransform(mImgMatrix);
    painter.setBrush(QColor(127, 144, 144, 200));
    painter.setPen(QColor(100, 100, 100, 255));

    QRectF initialRect = rect();
    QPointF oldCenter = initialRect.center();

    QTransform cT;
    cT.scale(400 / initialRect.width(), 400 / initialRect.width());
    initialRect = cT.mapRect(initialRect);
    initialRect.moveCenter(oldCenter);

    // fit to mViewport
    QSize s = mImgBg.size();

    QRectF bgRect(QPoint(), s);
    bgRect.moveCenter(initialRect.center()); // moveTopLeft(QPointF(size().width(), size().height())*0.5 - initialRect.bottomRight()*0.5);

    // painter.drawRect(initialRect);
    painter.drawImage(bgRect, mImgBg, QRect(QPoint(), mImgBg.size()));

    if (mStartActions.isEmpty())
        return;

    // first time?
    if (mStartActionsRects.isEmpty()) {
        float margin = 40;
        float iconSizeMargin = (float)((initialRect.width() - 3 * margin) / mStartActions.size());
        QSize iconSize = QSize(qRound(iconSizeMargin - margin), qRound(iconSizeMargin - margin));
        QPointF offset = QPointF(bgRect.left() + 50, initialRect.center().y() + iconSizeMargin * 0.25f);

        for (int idx = 0; idx < mStartActions.size(); idx++) {
            QRectF iconRect = QRectF(offset, iconSize);
            QPixmap ci = !mStartIcons[idx].isNull() ? mStartIcons[idx].pixmap(iconSize) : mStartActions[idx]->icon().pixmap(iconSize);
            mStartActionsRects.push_back(iconRect);
            mStartActionsIcons.push_back(ci);

            offset.setX(offset.x() + margin + iconSize.width());
        }
    }

    // draw start actions
    for (int idx = 0; idx < mStartActions.size(); idx++) {
        if (!mStartIcons[idx].isNull())
            painter.drawPixmap(mStartActionsRects[idx], mStartActionsIcons[idx], QRect(QPoint(), mStartActionsIcons[idx].size()));
        else
            painter.drawPixmap(mStartActionsRects[idx], mStartActionsIcons[idx], QRect(QPoint(), mStartActionsIcons[idx].size()));

        QRectF tmpRect = mStartActionsRects[idx];
        QString text = mStartActions[idx]->text().remove("&");
        tmpRect.moveTop(tmpRect.bottom() + 10);
        painter.drawText(tmpRect, text);
    }

    QString infoText = tr("Press F10 to exit Frameless view");
    QRectF tmpRect(bgRect.left() + 50, bgRect.bottom() - 60, bgRect.width() - 100, 20);
    painter.drawText(tmpRect, infoText);
}

void DkViewPortFrameless::drawFrame(QPainter &painter)
{
    // TODO: replace hasAlphaChannel with has alphaBorder
    if ((!mImgStorage.isEmpty() && mImgStorage.image().hasAlphaChannel()) || !DkSettingsManager::param().display().showBorder) // braces
        return;

    painter.setBrush(QColor(255, 255, 255, 200));
    painter.setPen(QColor(100, 100, 100, 255));

    QRectF frameRect;

    float fs = qMin((float)mImgViewRect.width(), (float)mImgViewRect.height()) * 0.1f;

    // looks pretty bad if the frame is too small
    if (fs < 4)
        return;

    frameRect = mImgViewRect;
    frameRect.setSize(frameRect.size() + QSize(qRound(fs), qRound(fs)));
    frameRect.moveCenter(mImgViewRect.center());

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
    if (mImgStorage.isEmpty()) {
        QPointF pos = mImgMatrix.inverted().map(event->pos());

        for (int idx = 0; idx < mStartActionsRects.size(); idx++) {
            if (mStartActionsRects[idx].contains(pos)) {
                mStartActions[idx]->trigger();
                break;
            }
        }
    }

    unsetCursor();
    // setCursor(Qt::OpenHandCursor);
    DkViewPort::mouseReleaseEvent(event);
}

void DkViewPortFrameless::mouseMoveEvent(QMouseEvent *event)
{
    if (mImgStorage.isEmpty()) {
        QPointF pos = mImgMatrix.inverted().map(event->pos());

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
        moveView(dxy / mWorldMatrix.m11());
    }

    QGraphicsView::mouseMoveEvent(event);
}

void DkViewPortFrameless::moveView(QPointF delta)
{
    // if no zoom is present -> the translation is like a move window
    if (mWorldMatrix.m11() == 1.0f) {
        float s = (float)mImgMatrix.m11();
        mImgMatrix.translate(delta.x() / s, delta.y() / s);
        mImgViewRect = mImgMatrix.mapRect(mImgRect);
    } else
        mWorldMatrix.translate(delta.x(), delta.y());

    controlImagePosition();
    update();
}

void DkViewPortFrameless::controlImagePosition(float, float)
{
    // dummy method
}

void DkViewPortFrameless::centerImage()
{
}

void DkViewPortFrameless::updateImageMatrix()
{
    if (mImgStorage.isEmpty())
        return;

    QRectF oldImgRect = mImgViewRect;
    QTransform oldImgMatrix = mImgMatrix;

    mImgMatrix.reset();

    QSize imgSize = getImageSize();

    // if the image is smaller or zoom is active: paint the image as is
    if (!mViewportRect.contains(mImgRect.toRect())) {
        mImgMatrix = getScaledImageMatrix(size() * 0.9f);
        QSize shift = size() * 0.1f;
        mImgMatrix.translate(shift.width(), shift.height());
    } else {
        mImgMatrix.translate((float)(getMainGeometry().width() - imgSize.width()) * 0.5f, (float)(getMainGeometry().height() - imgSize.height()) * 0.5f);
        mImgMatrix.scale(1.0f, 1.0f);
    }

    mImgViewRect = mImgMatrix.mapRect(mImgRect);

    // update world matrix
    if (mWorldMatrix.m11() != 1) {
        float scaleFactor = (float)(oldImgMatrix.m11() / mImgMatrix.m11());
        double dx = oldImgRect.x() / scaleFactor - mImgViewRect.x();
        double dy = oldImgRect.y() / scaleFactor - mImgViewRect.y();

        mWorldMatrix.scale(scaleFactor, scaleFactor);
        mWorldMatrix.translate(dx, dy);
    }
}

// DkViewPortContrast --------------------------------------------------------------------
DkViewPortContrast::DkViewPortContrast(QWidget *parent)
    : DkViewPort(parent)
{
    mColorTable = QVector<QRgb>(256);
    for (int i = 0; i < mColorTable.size(); i++)
        mColorTable[i] = qRgb(i, i, i);

    // connect
    auto ttb = DkToolBarManager::inst().transferToolBar();
    connect(ttb, SIGNAL(colorTableChanged(QGradientStops)), this, SLOT(changeColorTable(QGradientStops)));
    connect(ttb, SIGNAL(channelChanged(int)), this, SLOT(changeChannel(int)));
    connect(ttb, SIGNAL(pickColorRequest(bool)), this, SLOT(pickColor(bool)));
    connect(ttb, SIGNAL(tFEnabled(bool)), this, SLOT(enableTF(bool)));
    connect(this, SIGNAL(tFSliderAdded(qreal)), ttb, SLOT(insertSlider(qreal)));
    connect(this, SIGNAL(imageModeSet(int)), ttb, SLOT(setImageMode(int)));
}

DkViewPortContrast::~DkViewPortContrast()
{
}

void DkViewPortContrast::changeChannel(int channel)
{
    if (channel < 0 || channel >= mImgs.size())
        return;

    if (!mImgStorage.isEmpty()) {
        mFalseColorImg = mImgs[channel];
        mFalseColorImg.setColorTable(mColorTable);
        mDrawFalseColorImg = true;

        update();

        drawImageHistogram();
    }
}

void DkViewPortContrast::changeColorTable(QGradientStops stops)
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
        for (int i = 0; i < mColorTable.size(); i++)
            mColorTable[i] = qRgb(rLeft, gLeft, bLeft);
    }
    // Otherwise interpolate:
    else {
        int rightStopIdx = 1;
        tmp = stops.at(rightStopIdx).second;
        tmp.getRgb(&rRight, &gRight, &bRight);
        rightStop = stops.at(rightStopIdx).first;

        for (int i = 0; i < mColorTable.size(); i++) {
            actPos = (qreal)i / mColorTable.size();

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

    mFalseColorImg.setColorTable(mColorTable);

    update();
}

void DkViewPortContrast::draw(QPainter &painter, double opacity)
{
    if (!mDrawFalseColorImg || mSvg || mMovie) {
        DkBaseViewPort::draw(painter, opacity);
        return;
    }

    if (DkUtils::getMainWindow()->isFullScreen())
        painter.setBackground(DkSettingsManager::param().slideShow().backgroundColor);

    QRect dr = mWorldMatrix.mapRect(mImgViewRect).toRect();
    QImage img = mImgStorage.image(dr.size());

    // opacity == 1.0f -> do not show pattern if we crossfade two images
    if (DkSettingsManager::param().display().tpPattern && img.hasAlphaChannel() && opacity == 1.0)
        drawPattern(painter);

    if (mDrawFalseColorImg)
        painter.drawImage(mImgViewRect, mFalseColorImg, mImgRect);
}

void DkViewPortContrast::setImage(QImage newImg)
{
    DkViewPort::setImage(newImg);

    if (newImg.isNull())
        return;

    if (mImgStorage.image().format() == QImage::Format_Indexed8) {
        mImgs = QVector<QImage>(1);
        mImgs[0] = mImgStorage.image();
        mActiveChannel = 0;
    }
#ifdef WITH_OPENCV

    else {

        mImgs = QVector<QImage>(4);
        std::vector<cv::Mat> planes;

        cv::Mat imgUC3 = DkImage::qImage2Mat(mImgStorage.image());
        // int format = imgQt.format();
        // if (format == QImage::Format_RGB888)
        //	imgUC3 = Mat(imgQt.height(), imgQt.width(), CV_8UC3, (uchar*)imgQt.bits(), imgQt.bytesPerLine());
        // else
        //	imgUC3 = Mat(imgQt.height(), imgQt.width(), CV_8UC4, (uchar*)imgQt.bits(), imgQt.bytesPerLine());
        split(imgUC3, planes);
        // Store the 3 channels in a QImage Vector.
        // Be aware that OpenCV 'swaps' the rgb triplet, hence process it in a descending way:
        int idx = 1;
        for (int i = 2; i >= 0; i--) {
            // dirty hack
            if (i >= (int)planes.size())
                i = 0;
            mImgs[idx] = QImage((const unsigned char *)planes[i].data, (int)planes[i].cols, (int)planes[i].rows, (int)planes[i].step, QImage::Format_Indexed8);
            mImgs[idx] = mImgs[idx].copy();
            idx++;
        }
        // The first element in the vector contains the gray scale 'average' of the 3 channels:
        cv::Mat grayMat;
        cv::cvtColor(imgUC3, grayMat, CV_BGR2GRAY);
        mImgs[0] = QImage((const unsigned char *)grayMat.data, (int)grayMat.cols, (int)grayMat.rows, (int)grayMat.step, QImage::Format_Indexed8);
        mImgs[0] = mImgs[0].copy();
        planes.clear();
    }
#else

    else {
        mDrawFalseColorImg = false;
        emit imageModeSet(mode_invalid_format);
        return;
    }

#endif

    mFalseColorImg = mImgs[mActiveChannel];
    mFalseColorImg.setColorTable(mColorTable);

    // images with valid color table return img.isGrayScale() false...
    if (mSvg || mMovie)
        emit imageModeSet(mode_invalid_format);
    else if (mImgs.size() == 1)
        emit imageModeSet(mode_gray);
    else
        emit imageModeSet(mode_rgb);

    update();
}

void DkViewPortContrast::pickColor(bool enable)
{
    mIsColorPickerActive = enable;
    this->setCursor(Qt::CrossCursor);
}

void DkViewPortContrast::enableTF(bool enable)
{
    mDrawFalseColorImg = enable;
    update();

    drawImageHistogram();
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
    if (mIsColorPickerActive) {
        QPointF imgPos = mWorldMatrix.inverted().map(event->pos());
        imgPos = mImgMatrix.inverted().map(imgPos);

        QPoint xy = imgPos.toPoint();

        bool isPointValid = true;

        if (xy.x() < 0 || xy.y() < 0 || xy.x() >= getImageSize().width() || xy.y() >= getImageSize().height())
            isPointValid = false;

        if (isPointValid) {
            int colorIdx = mImgs[mActiveChannel].pixelIndex(xy);
            qreal normedPos = (qreal)colorIdx / 255;
            emit tFSliderAdded(normedPos);
        }

        // unsetCursor();
        // isColorPickerActive = false;
    } else
        DkViewPort::mouseReleaseEvent(event);
}

void DkViewPortContrast::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Escape) && mIsColorPickerActive) {
        unsetCursor();
        mIsColorPickerActive = false;
        update();
        return;
    } else
        DkViewPort::keyPressEvent(event);
}

QImage DkViewPortContrast::getImage() const
{
    if (mDrawFalseColorImg)
        return mFalseColorImg;
    else
        return imageContainer() ? imageContainer()->image() : QImage();
}

// in contrast mode: if the histogram widget is visible redraw the histogram from the selected image channel data
void DkViewPortContrast::drawImageHistogram()
{
    if (mController->getHistogram() && mController->getHistogram()->isVisible()) {
        if (mDrawFalseColorImg)
            mController->getHistogram()->drawHistogram(mFalseColorImg);
        else
            mController->getHistogram()->drawHistogram(getImage());
    }
}

}
