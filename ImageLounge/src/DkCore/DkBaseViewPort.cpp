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

#include <QColorSpace>
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

#include <cfloat>

namespace nmc
{
// DkBaseViewport --------------------------------------------------------------------
DkBaseViewPort::DkBaseViewPort(QWidget *parent)
    : QGraphicsView(parent)
{
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);
    setAttribute(Qt::WA_AcceptTouchEvents);

    mViewportRect = QRect(0, 0, width(), height());

    mPanControl = QPointF(-1.0f, -1.0f);

    mAltMod = DkSettingsManager::param().global().altMod;
    mCtrlMod = DkSettingsManager::param().global().ctrlMod;

    mZoomTimer = new QTimer(this);
    mZoomTimer->setSingleShot(true);
    connect(mZoomTimer, &QTimer::timeout, this, &DkBaseViewPort::stopBlockZooming);
    connect(&mImgStorage, &DkImageStorage::imageUpdated, this, QOverload<>::of(&DkBaseViewPort::update));

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
    connect(am.action(DkActionManager::sc_pan_left), &QAction::triggered, this, &DkBaseViewPort::panLeft);
    connect(am.action(DkActionManager::sc_pan_right), &QAction::triggered, this, &DkBaseViewPort::panRight);
    connect(am.action(DkActionManager::sc_pan_up), &QAction::triggered, this, &DkBaseViewPort::panUp);
    connect(am.action(DkActionManager::sc_pan_down), &QAction::triggered, this, &DkBaseViewPort::panDown);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &DkBaseViewPort::scrollVertically);
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &DkBaseViewPort::scrollHorizontally);

    mHideCursorTimer = new QTimer(this);
    mHideCursorTimer->setInterval(1000);
    connect(mHideCursorTimer, &QTimer::timeout, this, &DkBaseViewPort::hideCursor);
}

DkBaseViewPort::~DkBaseViewPort() = default;

void DkBaseViewPort::zoomConstraints(double minZoom, double maxZoom)
{
    mMinZoom = minZoom;
    mMaxZoom = maxZoom;
}

// zoom - pan --------------------------------------------------------------------
void DkBaseViewPort::resetView()
{
    mWorldMatrix.reset();
    changeCursor();

    update();
}

void DkBaseViewPort::fullView()
{
    mWorldMatrix.reset();
    zoom(1.0 / mImgMatrix.m11());
    changeCursor();

    update();
}

void DkBaseViewPort::togglePattern(bool show)
{
    DkSettingsManager::param().display().tpPattern = show;
    update();
}

void DkBaseViewPort::panLeft()
{
    float delta = -2 * width() / (100.0f * (float)mWorldMatrix.m11());
    moveView(QPointF(delta, 0));
}

void DkBaseViewPort::panRight()
{
    float delta = 2 * width() / (100.0f * (float)mWorldMatrix.m11());
    moveView(QPointF(delta, 0));
}

void DkBaseViewPort::panUp()
{
    float delta = -2 * height() / (100.0f * (float)mWorldMatrix.m11());
    moveView(QPointF(0, delta));
}

void DkBaseViewPort::panDown()
{
    float delta = 2 * height() / (100.0f * (float)mWorldMatrix.m11());
    moveView(QPointF(0, delta));
}

void DkBaseViewPort::moveView(const QPointF &delta)
{
    QPointF lDelta = delta;
    QRectF imgWorldRect = mWorldMatrix.mapRect(mImgViewRect);
    if (imgWorldRect.width() < width())
        lDelta.setX(0);
    if (imgWorldRect.height() < height())
        lDelta.setY(0);

    mWorldMatrix.translate(lDelta.x(), lDelta.y());
    controlImagePosition();
    update();
}

void DkBaseViewPort::zoomIn()
{
    zoomLeveled(1.5);
}

void DkBaseViewPort::zoomOut()
{
    zoomLeveled(0.5);
}

void DkBaseViewPort::zoomLeveled(double factor, const QPointF &center)
{
    factor = DkZoomConfig::instance().nextFactor(mWorldMatrix.m11() * mImgMatrix.m11(), factor);
    zoom(factor, center);
}

void DkBaseViewPort::zoom(double factor, const QPointF &center, bool force)
{
    if (mImgStorage.isEmpty())
        return;

    // limit zoom out ---
    if (mWorldMatrix.m11() * factor < mMinZoom && factor < 1) {
        // clamp to minimum, if we are close do nothing to prevent updates
        if (qFuzzyCompare(mWorldMatrix.m11(), mMinZoom))
            return;

        factor = mMinZoom / mWorldMatrix.m11();
    }

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

    // limit zoom in ---
    if (mWorldMatrix.m11() * mImgMatrix.m11() > mMaxZoom && factor > 1)
        return;

    QPointF pos = center;

    // if no center assigned: zoom in at the image center
    if (pos.x() == -1 || pos.y() == -1)
        pos = mImgViewRect.center();

    zoomToPoint(factor, pos, mWorldMatrix);

    controlImagePosition();
    changeCursor();

    update();
}

void DkBaseViewPort::zoomToPoint(double factor, const QPointF &pos, QTransform &matrix) const
{
    // inverse the transform
    double a, b;
    matrix.inverted().map(pos.x(), pos.y(), &a, &b);

    matrix.translate(a - factor * a, b - factor * b);
    matrix.scale(factor, factor);
}

void DkBaseViewPort::stopBlockZooming()
{
    mBlockZooming = false;
}

// set image --------------------------------------------------------------------
void DkBaseViewPort::setImage(QImage newImg)
{
    mImgStorage.setImage(newImg);
    QRectF oldImgRect = mImgRect;
    mImgRect = QRectF(QPointF(), getImageSize());

    if (!DkSettingsManager::param().display().keepZoom || mImgRect != oldImgRect)
        mWorldMatrix.reset();

    updateImageMatrix();
    update();
}

void DkBaseViewPort::hideCursor()
{
    if (isFullScreen())
        setCursor(Qt::BlankCursor);
}

QImage DkBaseViewPort::getImage() const
{
    QImage img;
    if (mMovie && mMovie->isValid()) {
        img = mMovie->currentImage();
    } else if (mSvg && mSvg->isValid() && !mImgViewRect.isEmpty()) {
        img = QImage(mImgViewRect.size().toSize(), QImage::Format_ARGB32);
        img.fill(QColor(0, 0, 0, 0));

        QPainter p(&img);
        if (mSvg && mSvg->isValid()) {
            mSvg->render(&p, mImgViewRect);
        }
    } else {
        img = mImgStorage.image();
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

    return QSizeF(mImgStorage.size()) / devicePixelRatioF();
}

QRectF DkBaseViewPort::getImageViewRect() const
{
    return mWorldMatrix.mapRect(mImgViewRect);
}

QImage DkBaseViewPort::getCurrentImageRegion()
{
    QRectF viewRect = QRectF(QPointF(), size());

    viewRect = mWorldMatrix.inverted().mapRect(viewRect);
    viewRect = (mImgMatrix.inverted() * devicePixelRatioF()).mapRect(viewRect);

    // Rect is now in image coordinates so just copy it.
    // If there is any oob condition it gets default fill
    return mImgStorage.image().copy(viewRect.toRect());
}

// events --------------------------------------------------------------------
void DkBaseViewPort::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());

    if (!mImgStorage.isEmpty()) {
        painter.setWorldTransform(mWorldMatrix);

        // don't interpolate - we have a sophisticated anti-aliasing methods
        //// don't interpolate if we are forced to, at 100% or we exceed the maximal interpolation level
        if (!mForceFastRendering && // force?
            mImgMatrix.m11() * mWorldMatrix.m11() - DBL_EPSILON > 1.0 && // @100% ?
            mImgMatrix.m11() * mWorldMatrix.m11()
                <= DkSettingsManager::param().display().interpolateZoomLevel / 100.0) { // > max zoom level
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

    mViewportRect = QRect(0, 0, event->size().width(), event->size().height());

    updateImageMatrix();
    centerImage();
    changeCursor();

    return QGraphicsView::resizeEvent(event);
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
#ifdef DK_CORE_DLL_EXPORT
    if (!event->isAutoRepeat())
        emit keyReleaseSignal(event); // make key presses available
#endif

    QWidget::keyReleaseEvent(event);
}

// mouse events --------------------------------------------------------------------
void DkBaseViewPort::mousePressEvent(QMouseEvent *event)
{
    // ok, start panning
    if (mWorldMatrix.m11() > 1 && !imageInside() && event->buttons() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
    }

    mPosGrab = event->pos();

    QWidget::mousePressEvent(event);
}

void DkBaseViewPort::mouseReleaseEvent(QMouseEvent *event)
{
    if (mWorldMatrix.m11() > 1 && !imageInside())
        setCursor(Qt::OpenHandCursor);

    QWidget::mouseReleaseEvent(event);
}

void DkBaseViewPort::mouseDoubleClickEvent(QMouseEvent *event)
{
    QCoreApplication::sendEvent(parentWidget(), event);
}

void DkBaseViewPort::mouseMoveEvent(QMouseEvent *event)
{
    if (mWorldMatrix.m11() > 1 && event->buttons() == Qt::LeftButton) {
        QPointF cPos = event->pos();
        QPointF dxy = (cPos - mPosGrab);
        mPosGrab = cPos;
        moveView(dxy / mWorldMatrix.m11());
    }
    if (event->buttons() != Qt::LeftButton && event->buttons() != Qt::RightButton) {
        if (event->modifiers() == mCtrlMod && event->modifiers() != mAltMod) {
            setCursor(Qt::CrossCursor);
            // DkStatusBarManager::instance().show(true, false);
        } else if (mWorldMatrix.m11() > 1 && !imageInside())
            setCursor(Qt::OpenHandCursor);
        else {
            if (!DkSettingsManager::param().app().showStatusBar)
                DkStatusBarManager::instance().show(false, false);

            if (cursor().shape() != Qt::ArrowCursor)
                unsetCursor();
        }

        if (isFullScreen())
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
    zoomLeveled(factor, event->position());
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
    viewRect = tx.inverted().mapRect(deviceRect.toRectF());

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
    const qreal dpr = devicePixelRatioF();
    RenderParams params = getRenderParams(dpr, frontPainter.worldTransform(), mImgViewRect);

    // this may return the size we want or give the full size image and rescale in the background
    const QImage img = mImgStorage.downsampled(params.imageSize, this);

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

    if ((flags & draw_pattern) && DkSettingsManager::param().display().tpPattern && mImgStorage.alphaChannelUsed()) {
        renderPattern(frontPainter, params);
    }

    if (!(flags & draw_image)) {
        frontPainter.setOpacity(oldOpacity);
        return;
    }

    QPainter &imgPainter = backPainter ? *(backPainter.get()) : frontPainter;

    if (mSvg && mSvg->isValid()) {
        mSvg->render(&imgPainter, mImgViewRect);
    } else if (mMovie && mMovie->isValid()) {
        imgPainter.drawPixmap(mImgViewRect, mMovie->currentPixmap(), mMovie->frameRect());
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

    if (DkUtils::getMainWindow()->isFullScreen())
        bgBrush = DkSettingsManager::param().slideShow().backgroundColor;

    if (bgBrush != Qt::NoBrush) {
        painter.setWorldMatrixEnabled(false);
        painter.fillRect(QRect(QPoint(), size()), bgBrush);
        painter.setWorldMatrixEnabled(true);
    }
}

bool DkBaseViewPort::imageInside() const
{
    QRect viewRect = mWorldMatrix.mapRect(mImgViewRect).toRect();

    return mWorldMatrix.m11() <= 1.0f || mViewportRect.contains(viewRect);
}

QPointF DkBaseViewPort::mapToImage(const QPointF &p)
{
    return (mWorldMatrix.inverted() * mImgMatrix.inverted()).map(p);
}

QPointF DkBaseViewPort::mapToImagePixel(const QPointF &p)
{
    return (mWorldMatrix.inverted() * mImgMatrix.inverted() * devicePixelRatioF()).map(p);
}

void DkBaseViewPort::updateImageMatrix()
{
    if (mImgStorage.isEmpty())
        return;

    QRectF oldImgRect = mImgViewRect;
    QTransform oldImgMatrix = mImgMatrix;

    mImgMatrix.reset();

    QSizeF imgSize = getImageSize();

    // if the image is smaller or zoom is active: paint the image as is
    if (!mViewportRect.contains(mImgRect))
        mImgMatrix = getScaledImageMatrix();
    else {
        mImgMatrix.translate((float)(width() - imgSize.width()) * 0.5f, (float)(height() - imgSize.height()) * 0.5f);
        mImgMatrix.scale(1.0f, 1.0f);
    }

    mImgViewRect = mImgMatrix.mapRect(mImgRect);

    // update world matrix
    if (mWorldMatrix.m11() != 1) {
        double scaleFactor = oldImgMatrix.m11() / mImgMatrix.m11();
        double dx = oldImgRect.x() / scaleFactor - mImgViewRect.x();
        double dy = oldImgRect.y() / scaleFactor - mImgViewRect.y();

        mWorldMatrix.scale(scaleFactor, scaleFactor);
        mWorldMatrix.translate(dx, dy);
    }
}

QTransform DkBaseViewPort::getScaledImageMatrix() const
{
    return getScaledImageMatrix(size());
}

QTransform scaleKeepAspectRatioAndCenter(const QSizeF &src, const QSizeF &tgt, qreal paddingRatio)
{
    // the image resizes as we zoom
    float ratioImg = (float)src.width() / (float)src.height();
    float ratioWin = (float)tgt.width() / (float)tgt.height();

    QTransform imgMatrix;
    float s;
    if (src.width() == 0 || src.height() == 0)
        s = 1.0f;
    else
        s = (ratioImg > ratioWin) ? (float)tgt.width() / (float)src.width() : (float)tgt.height() / (float)src.height();

    imgMatrix.scale(s, s);

    QPointF mapped = imgMatrix.map(QPointF(src.width(), src.height()));
    imgMatrix.translate((tgt.width() - mapped.x()) * 0.5f / s, (tgt.height() - mapped.y()) * 0.5f / s);

    return imgMatrix;
}

QTransform DkBaseViewPort::getScaledImageMatrix(const QSize &size) const
{
    return scaleKeepAspectRatioAndCenter(mImgRect.size(), size);
}

void DkBaseViewPort::controlImagePosition(float lb, float ub)
{
    QRectF imgRectWorld = mWorldMatrix.mapRect(mImgViewRect);

    if (lb == -1 && ub == -1 && mPanControl.x() != -1 && mPanControl.y() != -1) {
        lb = (float)mPanControl.x();
        ub = (float)mPanControl.y();
    }
    // we must not pan further if scrollbars are visible
    else if (lb == -1 && ub == -1 && DkSettingsManager::instance().param().display().showScrollBars) {
        lb = 0.0f;
        ub = 0.0f;
    } else {
        // default behavior
        if (lb == -1)
            lb = (float)mViewportRect.width() / 2.0f;
        if (ub == -1)
            ub = (float)mViewportRect.height() / 2.0f;
    }

    if (imgRectWorld.left() > lb && imgRectWorld.width() > width())
        mWorldMatrix.translate((lb - imgRectWorld.left()) / mWorldMatrix.m11(), 0);

    if (imgRectWorld.top() > ub && imgRectWorld.height() > height())
        mWorldMatrix.translate(0, (ub - imgRectWorld.top()) / mWorldMatrix.m11());

    if (imgRectWorld.right() < width() - lb && imgRectWorld.width() > width())
        mWorldMatrix.translate(((width() - lb) - imgRectWorld.right()) / mWorldMatrix.m11(), 0);

    if (imgRectWorld.bottom() < height() - ub && imgRectWorld.height() > height())
        mWorldMatrix.translate(0, ((height() - ub) - imgRectWorld.bottom()) / mWorldMatrix.m11());

    // update scene size (this is needed to make the scroll area work)
    if (DkSettingsManager::instance().param().display().showScrollBars)
        setSceneRect(getImageViewRect());

    emit imageUpdated();
}

void DkBaseViewPort::centerImage()
{
    QRectF imgWorldRect = mWorldMatrix.mapRect(mImgViewRect);
    float dx, dy;

    // if black border - center the image
    if (imgWorldRect.width() < (float)width()) {
        dx = (float)((width() - imgWorldRect.width()) * 0.5f - mImgViewRect.x() * mWorldMatrix.m11());
        dx = (dx - (float)mWorldMatrix.dx()) / (float)mWorldMatrix.m11();
        mWorldMatrix.translate(dx, 0);
    } else if (imgWorldRect.left() > 0)
        mWorldMatrix.translate(-imgWorldRect.left() / mWorldMatrix.m11(), 0);
    else if (imgWorldRect.right() < width())
        mWorldMatrix.translate((width() - imgWorldRect.right()) / mWorldMatrix.m11(), 0);

    if (imgWorldRect.height() < height()) {
        dy = (float)((height() - imgWorldRect.height()) * 0.5f - mImgViewRect.y() * mWorldMatrix.m22());
        dy = (float)((dy - mWorldMatrix.dy()) / mWorldMatrix.m22());
        mWorldMatrix.translate(0, dy);
    } else if (imgWorldRect.top() > 0) {
        mWorldMatrix.translate(0, -imgWorldRect.top() / mWorldMatrix.m22());
    } else if (imgWorldRect.bottom() < height()) {
        mWorldMatrix.translate(0, (height() - imgWorldRect.bottom()) / mWorldMatrix.m22());
    }
}

void DkBaseViewPort::changeCursor()
{
    if (mWorldMatrix.m11() > 1 && !imageInside())
        setCursor(Qt::OpenHandCursor);
    else
        unsetCursor();
}

void DkBaseViewPort::setBackgroundBrush(const QBrush &brush)
{
    QGraphicsView::setBackgroundBrush(brush);
}

void DkBaseViewPort::scrollHorizontally(int val)
{
    moveView(QPointF(-val / mWorldMatrix.m11(), 0.0f));
}

void DkBaseViewPort::scrollVertically(int val)
{
    moveView(QPointF(0.0f, -val / mWorldMatrix.m11()));
}

// Anna's first text:
// gcfbxxxxxxxxxxxbxbbbcx
// tggbeeeeeeeeeeecddddeebljlljl�
//
//
// rr45�[, mgd     7re � 8n484welkmsclsdmvcdsm wr., bpodsa0][gb b c

}
