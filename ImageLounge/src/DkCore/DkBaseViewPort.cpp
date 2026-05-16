/*******************************************************************************************************
 DkBaseViewPort.cpp
 Created on:	03.07.2013

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

#include "DkBaseViewPort.h"

#include "DkActionManager.h"
#include "DkSettings.h"
#include "DkShortcuts.h"
#include "DkStatusBar.h"
#include "DkUtils.h"
#include "DkViewPortImageViewModel.h"
#include "DkViewPortTransformViewModel.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMainWindow>
#include <QMovie>
#include <QScrollBar>
#include <QShortcut>
#include <QSvgRenderer>
#include <QSwipeGesture>
#include <QTimer>
#include <QtGlobal>

#include <algorithm>
#include <cfloat>
#include <memory>
#include <qobject.h>
#include <qsvgrenderer.h>
#include <type_traits>
#include <variant>

namespace nmc
{
// DkBaseViewport --------------------------------------------------------------------
DkBaseViewPort::DkBaseViewPort(bool inDialog, QWidget *parent, bool resetWhenZoomPastFit)
    : QGraphicsView(parent)
    , mForceFastRendering{inDialog}
    , mTransformVM{std::make_unique<DkViewPortTransformViewModel>(devicePixelRatioF(), inDialog, resetWhenZoomPastFit)}
    , mImageVM{std::make_unique<DkViewPortImageViewModel>()}
{
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);
    setAttribute(Qt::WA_AcceptTouchEvents);

    mTransformVM->setShowScrollBarSettingProvider([]() {
        return DkSettingsManager::param().display().showScrollBars;
    });
    mTransformVM->setZoomLevelSettingProvider([]() {
        return DkZoomConfig::instance().levels();
    });
    mTransformVM->setWidgetSize(size());

    mAltMod = DkSettingsManager::param().global().altMod;
    mCtrlMod = DkSettingsManager::param().global().ctrlMod;

    connect(mImageVM.get(), &DkViewPortImageViewModel::imageUpdated, this, QOverload<>::of(&DkBaseViewPort::update));

    if (DkSettingsManager::param().display().defaultBackgroundColor)
        setObjectName("DkBaseViewPortDefaultColor");
    else
        setObjectName("DkBaseViewPort");

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setMouseTracking(true);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumSize(10, 10);

    // disable Qt default keys, prevent keyPressEvent() keys from being broken by shortcuts
    DkShortcutEventFilter::reserveKeys(this,
                                       {
                                           DkActionManager::shortcut_zoom_in,
                                           DkActionManager::shortcut_zoom_out,
                                           DkActionManager::shortcut_zoom_in_alt,
                                           DkActionManager::shortcut_zoom_out_alt,
                                       });

    // connect pan actions
    const DkActionManager &am = DkActionManager::instance();
    connect(am.action(DkActionManager::sc_pan_left),
            &QAction::triggered,
            mTransformVM.get(),
            &DkViewPortTransformViewModel::panLeft);
    connect(am.action(DkActionManager::sc_pan_right),
            &QAction::triggered,
            mTransformVM.get(),
            &DkViewPortTransformViewModel::panRight);
    connect(am.action(DkActionManager::sc_pan_up),
            &QAction::triggered,
            mTransformVM.get(),
            &DkViewPortTransformViewModel::panUp);
    connect(am.action(DkActionManager::sc_pan_down),
            &QAction::triggered,
            mTransformVM.get(),
            &DkViewPortTransformViewModel::panDown);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &DkBaseViewPort::scrollVertically);
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &DkBaseViewPort::scrollHorizontally);

    mHideCursorTimer = new QTimer(this);
    mHideCursorTimer->setInterval(1000);
    connect(mHideCursorTimer, &QTimer::timeout, this, &DkBaseViewPort::hideCursor);

    connect(mTransformVM.get(), &DkViewPortTransformViewModel::transformChanged, this, [this]() {
        changeCursor();

        // update scene size (this is needed to make the scroll area work)
        if (DkSettingsManager::instance().param().display().showScrollBars) {
            setSceneRect(getImageViewRect());
        }

        update();
        emit imageUpdated();
    });

    connect(mImageVM.get(), &DkViewPortImageViewModel::contentStateChanged, this, &DkBaseViewPort::updateRenderer);
}

DkBaseViewPort::~DkBaseViewPort() = default;

void DkBaseViewPort::setMinZoomLevelTo1()
{
    mTransformVM->setMinZoomLevelTo1();
}

// zoom - pan --------------------------------------------------------------------

void DkBaseViewPort::fullView()
{
    mTransformVM->fullView();
}

void DkBaseViewPort::togglePattern(bool show)
{
    DkSettingsManager::param().display().tpPattern = show;
    update();
}

void DkBaseViewPort::moveViewInImageCoords(const QPointF &delta)
{
    // Reuse the logic
    moveViewInWidgetCoords(delta * zoomLevel());
}

void DkBaseViewPort::moveViewInWidgetCoords(const QPointF &delta)
{
    mTransformVM->moveViewInWidgetCoords(delta);
}

void DkBaseViewPort::zoom(double factor, const QPointF &center, bool force)
{
    if (mImageVM->isEmpty()) {
        return;
    }

    mTransformVM->zoom(factor, center, force);
}

// set image --------------------------------------------------------------------
void DkBaseViewPort::setImage(const QImage &newImg)
{
    mImageVM->setRasterImage(newImg);
    const bool kz = DkSettingsManager::param().display().keepZoom;
    mTransformVM->setImgSize(getImageSize(), kz ? DkSettings::zoom_keep_same_size : DkSettings::zoom_never_keep);
    update();
}

void DkBaseViewPort::hideCursor()
{
    if (window()->isFullScreen())
        setCursor(Qt::BlankCursor);
}

QImage DkBaseViewPort::getImage() const
{
    const QRectF imgViewRect = mTransformVM->imgViewRect();
    QImage img;
    if (mMovie && mMovie->isValid()) {
        img = mMovie->currentImage();
    } else if (mSvg && mSvg->isValid() && !imgViewRect.isEmpty()) {
        img = QImage(imgViewRect.size().toSize(), QImage::Format_ARGB32);
        img.fill(QColor(0, 0, 0, 0));

        QPainter p(&img);
        if (mSvg && mSvg->isValid()) {
            mSvg->render(&p, imgViewRect);
        }
    } else {
        img = mImageVM->image();
    }

    if (!img.colorSpace().isValid()) {
        QColorSpace srgb{QColorSpace::SRgb};
        srgb.setDescription(tr("sRGB (Unspecified)"));
        img.setColorSpace(srgb);
    }

    return img;
}

QSizeF DkBaseViewPort::getImageSize() const
{
    if (mSvg) {
        // qDebug() << "win: " << size() << "svg:" << mSvg->defaultSize() << "scaled:" <<
        // mSvg->defaultSize().scaled(size(), Qt::KeepAspectRatio);
        return mSvg->defaultSize().scaled(size(), Qt::KeepAspectRatio);
    }

    // HiDPI: Pretend the image is smaller and avoid rewriting scaling/translating logic
    // At 100%, this gives mImgViewRect.size()*dpr == img.rect().size(), so QPainter skips device scaling
    // - this must be reversed only when reaching into the image pixels (extract subimage, eyedropper tool, etc)
    // - we must take care to avoid rounding as this creates a fraction

    return QSizeF(mImageVM->image().size()) / devicePixelRatioF();
}

QRectF DkBaseViewPort::getImageViewRect() const
{
    return mTransformVM->getImageViewRect();
}

QImage DkBaseViewPort::getCurrentImageRegion()
{
    const qreal dpr = devicePixelRatioF();
    const QRectF viewRect = QTransform().scale(dpr, dpr).mapRect(mTransformVM->viewportInImageCoords());

    // Rect is now in image coordinates so just copy it.
    // If there is any oob condition it gets default fill
    return mImageVM->image().copy(viewRect.toRect());
}

// events --------------------------------------------------------------------
void DkBaseViewPort::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());

    if (!mImageVM->isEmpty()) {
        painter.setWorldTransform(mTransformVM->worldMatrix());

        // don't interpolate - we have a sophisticated anti-aliasing methods
        //// don't interpolate if we are forced to, at 100% or we exceed the maximal interpolation level
        const qreal zl = zoomLevel();
        if (!mForceFastRendering && zl - DBL_EPSILON > 1.0
            && zl <= DkSettingsManager::param().display().interpolateZoomLevel / 100.0) { // > max zoom level
            painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
        }

        draw(painter);
    }

    // propagate
    QGraphicsView::paintEvent(event);
}

void DkBaseViewPort::resizeEvent(QResizeEvent *event)
{
    if (event->oldSize() == event->size())
        return;

    mTransformVM->setWidgetSize(event->size());

    QGraphicsView::resizeEvent(event);
}

bool DkBaseViewPort::event(QEvent *event)
{
    // TODO: check if we still need this
    if (event->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent *>(event));

    return QGraphicsView::event(event);
}

bool DkBaseViewPort::gestureEvent(QGestureEvent *event)
{
    if (const QGesture *swipeG = event->gesture(Qt::SwipeGesture)) {
        const auto *swipe = static_cast<const QSwipeGesture *>(swipeG);

        // thanks qt documentation : )
        if (swipe->state() == Qt::GestureFinished) {
            if (swipe->horizontalDirection() == QSwipeGesture::Left || swipe->verticalDirection() == QSwipeGesture::Up)
                qDebug() << "here comes the previous image function...";
            else
                qDebug() << "here comes the next image function...";
        }
        qDebug() << "swiping...";
    } else if (const auto *pinch = static_cast<QPinchGesture *>(event->gesture(Qt::PinchGesture))) {
        double scale = pinch->lastScaleFactor();

        if (fabs(scale - 1.0) > FLT_EPSILON) {
            zoom(scale, mapFromGlobal(pinch->centerPoint().toPoint()));
        }

        qDebug() << "[Qt] pinching...";
    } else if (/*QGesture *pan = */ event->gesture(Qt::PanGesture)) {
        qDebug() << "panning...";
    } else
        return false;

    return true;
}

// key events --------------------------------------------------------------------
void DkBaseViewPort::keyPressEvent(QKeyEvent *event)
{
    QKeyCombination kc = event->keyCombination();
    // we want to change the behaviour on auto-repeat - so we cannot use QShortcuts here...
    if (kc == DkActionManager::shortcut_zoom_in || kc == DkActionManager::shortcut_zoom_in_alt) {
        zoom(event->isAutoRepeat() ? 1.1f : 1.5f);
    }
    if (kc == DkActionManager::shortcut_zoom_out || kc == DkActionManager::shortcut_zoom_out_alt) {
        zoom(event->isAutoRepeat() ? 0.9f : 0.5f);
    }

    QWidget::keyPressEvent(event);
}

void DkBaseViewPort::keyReleaseEvent(QKeyEvent *event)
{
    // NOTE: This bypasses QGraphicsView::keyReleaseEvent,
    // which might accept and stop propagating.
    QWidget::keyReleaseEvent(event);
}

// mouse events --------------------------------------------------------------------
void DkBaseViewPort::mousePressEvent(QMouseEvent *event)
{
    // ok, start panning
    if (mTransformVM->upscaled() && !imageInside() && event->buttons() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
    }

    mPosGrab = event->pos();

    QWidget::mousePressEvent(event);
}

void DkBaseViewPort::mouseReleaseEvent(QMouseEvent *event)
{
    if (mTransformVM->upscaled() && !imageInside())
        setCursor(Qt::OpenHandCursor);

    QWidget::mouseReleaseEvent(event);
}

void DkBaseViewPort::mouseDoubleClickEvent(QMouseEvent *event)
{
    QCoreApplication::sendEvent(parentWidget(), event);
}

void DkBaseViewPort::mouseMoveEvent(QMouseEvent *event)
{
    if (mTransformVM->upscaled() && event->buttons() == Qt::LeftButton) {
        QPointF cPos = event->pos();
        QPointF dxy = (cPos - mPosGrab);
        mPosGrab = cPos;
        moveViewInWidgetCoords(dxy);
    }
    if (event->buttons() != Qt::LeftButton && event->buttons() != Qt::RightButton) {
        if (event->modifiers() == mCtrlMod && event->modifiers() != mAltMod) {
            setCursor(Qt::CrossCursor);
            // DkStatusBarManager::instance().show(true, false);
        } else if (mTransformVM->upscaled() && !imageInside())
            setCursor(Qt::OpenHandCursor);
        else {
            if (!DkSettingsManager::param().app().showStatusBar)
                DkStatusBarManager::instance().show(false, false);

            if (cursor().shape() != Qt::ArrowCursor)
                unsetCursor();
        }

        if (window()->isFullScreen())
            mHideCursorTimer->start(3000);
    }

    QWidget::mouseMoveEvent(event);
}

void DkBaseViewPort::wheelEvent(QWheelEvent *event)
{
    double factor = -event->angleDelta().y();
    if (DkSettingsManager::param().display().invertZoom)
        factor *= -1.0;

    factor /= -1200.0;
    factor += 1.0;

    // qDebug() << "zoom factor..." << factor;
    mTransformVM->zoomLeveled(factor, event->position());
}

void DkBaseViewPort::contextMenuEvent(QContextMenuEvent *event)
{
    // send this event to my parent...
    QWidget::contextMenuEvent(event);
}

// protected functions --------------------------------------------------------------------

DkBaseViewPort::RenderParams DkBaseViewPort::getRenderParams(double devicePixelRatio,
                                                             const QTransform &worldMatrix,
                                                             const QRectF &imgViewRect)
{
    QRectF viewRect = worldMatrix.mapRect(imgViewRect);

    auto tx = QTransform::fromScale(devicePixelRatio, devicePixelRatio);

    // Rect in device coordinates, rounded to pixels
    QRect deviceRect = tx.mapRect(viewRect).toRect();

    // Rect to draw image, with world matrix disabled
    viewRect = tx.inverted().mapRect(QRectF{deviceRect});

    // Rect to draw image, with world matrix enabled
    QRectF dstRect = worldMatrix.inverted().mapRect(viewRect);

    return {devicePixelRatio, imgViewRect, worldMatrix, dstRect, viewRect, deviceRect, deviceRect.size()};
}

void DkBaseViewPort::renderImage(QPainter &painter, const QImage &img, const RenderParams &params)
{
    // use bilinear interpolation for upsampling, to a point. QPainter doesn't offer anything else
    qreal scaleFactor = double(params.imageSize.width()) / img.size().width();
    bool bilinear = img.size() != params.imageSize && scaleFactor > 1.0 && scaleFactor < 2.0; // TODO: prefs

    painter.setRenderHint(QPainter::SmoothPixmapTransform, bilinear);
    painter.drawImage(params.dstRect, img);
}

QImage DkBaseViewPort::renderBuffer(QImage::Format format) const
{
    double dpr = devicePixelRatio();
    QImage buffer = QImage(size() * dpr, format);
    buffer.setDevicePixelRatio(dpr);
    return buffer;
}

void DkBaseViewPort::renderComposite(QPainter &painter, const QImage &img, const RenderParams &params, int flags) const
{
    painter.setWorldTransform(params.worldMatrix);
    if (flags & draw_background) {
        eraseBackground(painter);
    }
    if (flags & draw_pattern) {
        renderPattern(painter, params);
    }
    if (flags & draw_image) {
        renderImage(painter, img, params);
    }
}

void DkBaseViewPort::draw(QPainter &frontPainter, double opacity, int flags)
{
    const QRectF imgViewRect = mTransformVM->imgViewRect();
    const qreal dpr = devicePixelRatioF();
    RenderParams params = getRenderParams(dpr, frontPainter.worldTransform(), imgViewRect);

    // this may return the size we want or give the full size image and rescale in the background
    const QImage img = mImageVM->downsampled(params.imageSize,
                                             DkImage::targetColorSpace(this),
                                             DkImage::targetFormat());

    // draw into an offscreen buffer for display colorspace conversion
    const QColorSpace targetColorSpace = DkImage::targetColorSpace(this);

    QColorSpace srcColorSpace;
    if (mSvg && mSvg->isValid()) {
        ; // unsupported, rarely used
    } else if (mMovie && mMovie->isValid()) {
        srcColorSpace = mMovie->currentImage().colorSpace();
    } else {
        srcColorSpace = img.colorSpace();
    }

    if (!srcColorSpace.isValid()) {
        static QColorSpace srgb{QColorSpace::SRgb};
        srcColorSpace = srgb;
    }

    // this has a slight performance hit, skip if we don't need it
    std::unique_ptr<QPainter> backPainter;
    if (targetColorSpace.isValid() && srcColorSpace != targetColorSpace) {
        QSize backingSize = this->size() * dpr;
        auto format = DkImage::renderFormat(img.format());
        if (mBackBuffer.size() != backingSize || mBackBuffer.format() != format) {
            mBackBuffer = QImage(backingSize, format);
        }
        mBackBuffer.fill(Qt::transparent); // eraseBackground() does not touch all pixels
        mBackBuffer.setDevicePixelRatio(dpr);
        mBackBuffer.setColorSpace(srcColorSpace);

        backPainter = std::make_unique<QPainter>(&mBackBuffer);
        backPainter->setWorldTransform(frontPainter.worldTransform());
    }

    if (flags & draw_background) {
        eraseBackground(frontPainter); // never with opacity (transparent when frameless)
    }

    double oldOpacity = frontPainter.opacity();
    frontPainter.setOpacity(opacity);

    if ((flags & draw_pattern) && DkSettingsManager::param().display().tpPattern && mImageVM->alphaChannelUsed()) {
        renderPattern(frontPainter, params);
    }

    if (!(flags & draw_image)) {
        frontPainter.setOpacity(oldOpacity);
        return;
    }

    QPainter &imgPainter = backPainter ? *(backPainter.get()) : frontPainter;

    if (mSvg && mSvg->isValid()) {
        mSvg->render(&imgPainter, imgViewRect);
    } else if (mMovie && mMovie->isValid()) {
        imgPainter.drawPixmap(imgViewRect, mMovie->currentPixmap(), mMovie->frameRect());
    } else {
        renderImage(imgPainter, img, params);
    }

    if (backPainter) {
        backPainter->end();
        // might be in-place or not
        QImage converted = DkImage::convertToColorSpaceInPlace(this, mBackBuffer);
        frontPainter.setWorldMatrixEnabled(false);
        frontPainter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        frontPainter.drawImage(params.viewRect, converted, params.deviceRect);
        frontPainter.setWorldMatrixEnabled(true);
    }

    frontPainter.setOpacity(oldOpacity);
}

void DkBaseViewPort::renderPattern(QPainter &painter, const RenderParams &params)
{
    static const QPixmap pattern(":/nomacs/img/tp-pattern.png");

    QBrush brush(pattern);

    // if there is a transform, apply it (animations, etc)
    QRectF viewRect = painter.worldTransform().mapRect(params.dstRect);

    painter.save();

    // don't scale the pattern relative to screen
    // device scaling will still be applied for HiDPI
    painter.setWorldMatrixEnabled(false);

    // anchor the pattern/rect so it doesn't "walk" when panning/zooming
    QPointF topLeft(viewRect.center() - pattern.rect().center());
    auto brushTransform = QTransform::fromTranslate(topLeft.x(), topLeft.y());
    brush.setTransform(brushTransform);

    painter.setBrush(brush);
    painter.setPen(Qt::NoPen); // no border
    painter.drawRect(viewRect);
    painter.restore();
}

void DkBaseViewPort::eraseBackground(QPainter &painter) const
{
    QBrush bgBrush = backgroundBrush();

    if (window()->isFullScreen())
        bgBrush = DkSettingsManager::param().slideShow().backgroundColor;

    if (bgBrush != Qt::NoBrush) {
        painter.setWorldMatrixEnabled(false);
        painter.fillRect(QRect(QPoint(), size()), bgBrush);
        painter.setWorldMatrixEnabled(true);
    }
}

bool DkBaseViewPort::imageInside() const
{
    return mTransformVM->imageInside();
}

QPointF DkBaseViewPort::mapToImagePixel(const QPointF &p)
{
    return mTransformVM->mapToImagePixel(p);
}

void DkBaseViewPort::changeCursor()
{
    if (mTransformVM->upscaled() && !imageInside())
        setCursor(Qt::OpenHandCursor);
    else
        unsetCursor();
}

void DkBaseViewPort::scrollHorizontally(int val)
{
    moveViewInWidgetCoords(QPointF(-val, 0.0f));
}

void DkBaseViewPort::scrollVertically(int val)
{
    moveViewInWidgetCoords(QPointF(0.0f, -val));
}

qreal DkBaseViewPort::zoomLevel() const
{
    return mTransformVM->zoomLevel();
}

void DkBaseViewPort::zoomTo(double zoomLevel)
{
    mTransformVM->zoomTo(zoomLevel);
}

DkViewPortTransformViewModel::ZoomLevelRange DkBaseViewPort::zoomLevelRange() const
{
    return mTransformVM->zoomLevelRange();
}

void DkBaseViewPort::updateRenderer()
{
    if (mMovie) {
        mMovie->stop();
        mMovie.reset();
    }
    mSvg.reset();
    mMovieIo.reset();
    DkActionManager::instance().enableMovieActions(false);

    std::visit(
        [this](const auto &arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, DkViewPortImageViewModel::RasterImage>) {
            } else if constexpr (std::is_same_v<T, DkViewPortImageViewModel::SVG>) {
                mSvg = QSharedPointer<QSvgRenderer>(new QSvgRenderer(arg.data));
                connect(mSvg.data(), &QSvgRenderer::repaintNeeded, this, QOverload<>::of(&DkBaseViewPort::update));
            } else if constexpr (std::is_same_v<T, DkViewPortImageViewModel::Movie>) {
                // read file to buffer, uses more memory, but:
                // - devices that can't seek also can't loop (zip, network)
                // - QMovie has a bug, fails to loop when constructed with a QFile
                // - we don't keep the file handle open (on windows can be a problem with delete, rename etc)
                // - animation won't hitch at the start
                mMovieIo.reset(new QBuffer);
                mMovieIo->setData(arg.data);

                // QIODevice pointer is not owned by QMovie
                QSharedPointer<QMovie> m(new QMovie(mMovieIo.get(), arg.format));

                // check if it truely a movie (we need this for we don't know if webp is actually animated)
                if (!m->isValid() || m->frameCount() == 1) {
                    qWarning() << "[movie]" << arg.fileName << "invalid format or not an animation";
                    return;
                }

                mMovie = m;
                qInfo() << "[movie] loaded animation:" << arg.fileName;

                connect(mMovie.data(), &QMovie::frameChanged, this, QOverload<>::of(&DkBaseViewPort::update));
                mMovie->start();
                DkActionManager::instance().enableMovieActions(true);
            } else {
                static_assert(sizeof(T) == 0, "non-exhaustive");
            }
        },
        mImageVM->contentState());
}
}
