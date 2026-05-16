// SPDX-License-Identifier: GPL-3.0-or-later
//
#include "DkViewPortTransformViewModel.h"

namespace
{
void applyZoomAroundPos(double factor, const QPointF &pos, QTransform &matrix)
{
    // inverse the transform
    double a, b;
    matrix.inverted().map(pos.x(), pos.y(), &a, &b);

    matrix.translate(a - factor * a, b - factor * b);
    matrix.scale(factor, factor);
}

double nextFactor(QVector<double> levels, double currentFactor, double delta)
{
    assert(currentFactor != 0.0);

    if (currentFactor == 0.0)
        return 1.0;

    if (delta > 1) {
        for (double l : levels) {
            if (currentFactor < l) {
                return l / currentFactor;
            }
        }
    } else if (delta < 1) {
        for (int idx = levels.size() - 1; idx >= 0; idx--) {
            if (currentFactor > levels[idx]) {
                return levels[idx] / currentFactor;
            }
        }
    }

    // do nothing
    return 1.0;
}
}

namespace nmc
{
QTransform scaleKeepAspectRatioAndCenter(const QSizeF &src, const QSizeF &tgt)
{
    qreal s = 1;
    if (src.isValid()) {
        s = std::min(tgt.width() / src.width(), tgt.height() / src.height());
    }

    const QPointF mapped = QPointF(src.width(), src.height()) * s;
    const QPointF offset = (QPointF(tgt.width(), tgt.height()) - mapped) / 2;

    // Scale first, then recenter
    return QTransform().translate(offset.x(), offset.y()).scale(s, s);
}

void DkViewPortTransformViewModel::setBlockZooming(bool v)
{
    mBlockZooming = v;
    if (v) {
        mZoomTimer->start(500);
    }
}

void DkViewPortTransformViewModel::stopBlockZooming()
{
    setBlockZooming(false);
}

void DkViewPortTransformViewModel::translateViewInWidgetCoords(qreal x, qreal y)
{
    mWorldMatrix.translate(x / mWorldMatrix.m11(), y / mWorldMatrix.m22());
}

void DkViewPortTransformViewModel::setWidgetSize(const QSize &size)
{
    mViewportRect = QRect({}, size);
    updateImageMatrix();
    if (!mImgRect.isNull()) {
        centerImage();
    }
}

void DkViewPortTransformViewModel::setImgSize(const QSizeF &size, std::optional<DkSettings::keepZoom> keepZoom)
{
    const QSizeF oldSize = mImgRect.size();
    mImgRect = QRectF({}, size);
    updateImageMatrix(keepZoom, oldSize);
}

void DkViewPortTransformViewModel::resetView()
{
    mWorldMatrix.reset();
    emit transformChanged();
}

void DkViewPortTransformViewModel::fullView()
{
    mWorldMatrix.reset();
    zoom(1.0 / mImgMatrix.m11());
}

qreal DkViewPortTransformViewModel::zoomLevel() const
{
    return mWorldMatrix.m11() * mImgMatrix.m11();
}

void DkViewPortTransformViewModel::zoomToPoint(double factor, const QPointF &pos)
{
    applyZoomAroundPos(factor, pos, mWorldMatrix);
}

void DkViewPortTransformViewModel::zoom(double factor, const QPointF &center, bool force)
{
    if (mBlockZooming) {
        return;
    }

    const double minZoom = mMinZoom;
    // limit zoom out ---
    if (mWorldMatrix.m11() * factor < minZoom && factor < 1) {
        // clamp to minimum, if we are close do nothing to prevent updates
        if (qFuzzyCompare(mWorldMatrix.m11(), minZoom)) {
            return;
        }

        factor = minZoom / mWorldMatrix.m11();
    }

    // reset view & block if we pass the 'image fit to screen' on zoom out
    if (mWorldMatrix.m11() > 1 && mWorldMatrix.m11() * factor < 1 && !force) {
        setBlockZooming(true);
        if (mResetWhenZoomPastFit) {
            resetView();
            return;
        }
    }

    // reset view if we pass the 'image fit to screen' on zoom in
    if (mResetWhenZoomPastFit && mWorldMatrix.m11() < 1 && mWorldMatrix.m11() * factor > 1 && !force) {
        resetView();
        return;
    }

    // limit zoom in ---
    if (zoomLevel() > sMaxZoomLevel && factor > 1) {
        return;
    }

    const ZoomPos pos = calcZoomCenter(center, factor);

    zoomToPoint(factor, pos.pos);

    controlImagePosition();
    if (pos.recenter) {
        centerImage();
    }

    emit transformChanged();
}

QRectF DkViewPortTransformViewModel::getImageViewRect() const
{
    return mWorldMatrix.mapRect(mImgViewRect);
}

QSizeF DkViewPortTransformViewModel::imageViewSize() const
{
    return mWorldMatrix.m11() * mImgViewRect.size();
}

bool DkViewPortTransformViewModel::imageInside() const
{
    return mWorldMatrix.m11() <= 1.0f || mViewportRect.contains(getImageViewRect());
}

QPointF DkViewPortTransformViewModel::mapToImagePixel(const QPointF &p) const
{
    return (mWorldMatrix.inverted() * mImgMatrix.inverted() * mDevicePixelRatio).map(p);
}

DkViewPortTransformViewModel::ZoomPos DkViewPortTransformViewModel::calcZoomCenter(const QPointF &center,
                                                                                   double factor) const
{
    switch (mZoomCenterLimit) {
    case ZoomCenterLimit::ToImageEdge:
        return calcZoomCenterLimitToImageEdge(center);
    case ZoomCenterLimit::CenterSmallDimension:
        return calcZoomCenterLimitCenterSmallDimension(center, factor);
    default:
        // Fallback logic.
        // if no center assigned: zoom in at the image center
        if (center.x() == -1 || center.y() == -1) {
            return {mImgViewRect.center()};
        }
        return {center};
    }
}

void DkViewPortTransformViewModel::updateImageMatrix(std::optional<DkSettings::keepZoom> keepZoom,
                                                     const QSizeF &oldSize)
{
    if (mImgRect.size().isEmpty()) {
        // Initial state has no image.
        return;
    }

    const QRectF oldImgRect = mImgViewRect;
    const QTransform oldImgMatrix = mImgMatrix;
    const qreal oldZoom = zoomLevel();

    mImgMatrix.reset();

    // if the image is smaller or zoom is active: paint the image as is
    if (!mViewportRect.contains(mImgRect.toRect())) {
        mImgMatrix = scaleKeepAspectRatioAndCenter(mImgRect.size(), mViewportRect.size());
    } else {
        const QSizeF offset = (mViewportRect.size() - mImgRect.size()) / 2;
        mImgMatrix.translate(offset.width(), offset.height());
    }

    mImgViewRect = mImgMatrix.mapRect(mImgRect);
    emit zoomLevelRangeChanged();

    if (!keepZoom) {
        // Maintain zoom level for calls that are not from setImage()
        if (qAbs(mWorldMatrix.m11() - 1.0) > 1e-4) {
            const qreal scaleFactor = oldImgMatrix.m11() / mImgMatrix.m11();
            const QPointF offset = oldImgRect.topLeft() / scaleFactor - mImgViewRect.topLeft();

            mWorldMatrix.scale(scaleFactor, scaleFactor);
            mWorldMatrix.translate(offset.x(), offset.y());
        }
        emit transformChanged();
        return;
    }

    constexpr qreal sizeTol = 1e-6;
    const bool isSameSize = std::abs(oldSize.width() - mImgRect.width()) < sizeTol
        && std::abs(oldSize.height() - mImgRect.height()) < sizeTol;

    // Update mWorldMatrix according to keepZoom rules.
    switch (keepZoom.value()) {
    case DkSettings::zoom_always_keep:
        if (!isSameSize) {
            mWorldMatrix.reset();
            zoomToPoint(oldZoom / zoomLevel(), mImgViewRect.center().toPoint());
        }
        break;
    case DkSettings::zoom_keep_same_size: {
        if (!isSameSize) {
            mWorldMatrix.reset();
        }
        break;
    }
    case DkSettings::zoom_never_keep:
        mWorldMatrix.reset();
        break;
    case DkSettings::zoom_always_fit:
        zoomToFit();
        break;

    default:
        Q_UNREACHABLE();
    }

    // if image is not inside, we'll align it at the top left border
    if (!mViewportRect.intersects(getImageViewRect())) {
        mWorldMatrix.translate(-mWorldMatrix.dx(), -mWorldMatrix.dy());
        centerImage();
    }
    emit transformChanged();
}

DkViewPortTransformViewModel::ZoomLevelRange DkViewPortTransformViewModel::zoomLevelRange() const
{
    return {mMinZoom * mImgMatrix.m11(), sMaxZoomLevel};
}

void DkViewPortTransformViewModel::zoomTo(double zoomLevel)
{
    mWorldMatrix.reset();
    zoom(zoomLevel / mImgMatrix.m11());
}

void DkViewPortTransformViewModel::controlImagePosition()
{
    if (mDisableControlAndCenter) {
        return;
    }

    qreal lb;
    qreal ub;
    if (mZeroPanControl || (mShowScrollBarSettingProvider && mShowScrollBarSettingProvider())) {
        // we must not pan further if scrollbars are visible
        lb = 0;
        ub = 0;
    } else {
        // default behavior
        lb = mViewportRect.width() / 2;
        ub = mViewportRect.height() / 2;
    }

    const QRectF imgRectWorld = getImageViewRect();
    const qreal w = mViewportRect.width();
    const qreal h = mViewportRect.height();

    qreal tX = 0;
    qreal tY = 0;
    if (imgRectWorld.left() > lb && imgRectWorld.width() > w) {
        tX = lb - imgRectWorld.left();
    }

    if (imgRectWorld.top() > ub && imgRectWorld.height() > h) {
        tY = ub - imgRectWorld.top();
    }

    if (imgRectWorld.right() < w - lb && imgRectWorld.width() > w) {
        tX = w - lb - imgRectWorld.right();
    }

    if (imgRectWorld.bottom() < h - ub && imgRectWorld.height() > h) {
        tY = h - ub - imgRectWorld.bottom();
    }
    translateViewInWidgetCoords(tX, tY);
    emit transformChanged();
}

void DkViewPortTransformViewModel::centerImage()
{
    if (mDisableControlAndCenter) {
        return;
    }

    const QRectF imgWorldRect = getImageViewRect();

    const qreal w = mViewportRect.width();
    const qreal h = mViewportRect.height();

    // This is not exactly centering the image.
    // In each coordinates:
    // - If the size is smaller than the viewport, translate so that the image is center.
    // - Otherwise (the size is larger), and somehow the viewport is not fully filled,
    //   translate to fill the viewport.
    qreal tX = 0;
    qreal tY = 0;
    if (imgWorldRect.width() < w) {
        tX = (w - imgWorldRect.width()) * 0.5f - imgWorldRect.x();
    } else if (imgWorldRect.left() > 0) {
        tX = -imgWorldRect.left();
    } else if (imgWorldRect.right() < w) {
        tX = (w - imgWorldRect.right());
    }

    if (imgWorldRect.height() < h) {
        tY = (h - imgWorldRect.height()) * 0.5f - imgWorldRect.y();
    } else if (imgWorldRect.top() > 0) {
        tY = -imgWorldRect.top();
    } else if (imgWorldRect.bottom() < h) {
        tY = (h - imgWorldRect.bottom());
    }

    translateViewInWidgetCoords(tX, tY);
}

void DkViewPortTransformViewModel::zoomToFit()
{
    const QSizeF imgSize = mImgRect.size();
    const QSizeF winSize = mViewportRect.size();
    const double zoomLevel = qMin(winSize.width() / imgSize.width(), winSize.height() / imgSize.height());

    if (zoomLevel > 1) {
        zoomTo(zoomLevel);
    } else if (zoomLevel < 1) {
        // FIXME: we originally have a `|| (zoomLevel == 1 && mSvg)`
        resetView();
    }
}

void DkViewPortTransformViewModel::moveViewInWidgetCoords(const QPointF &delta)
{
    QPointF lDelta = delta;
    if (mDisablePanForSmallDimension) {
        const qreal w = mViewportRect.width();
        const qreal h = mViewportRect.height();
        const QSizeF imgWorldRect = imageViewSize();
        if (imgWorldRect.width() < w)
            lDelta.setX(0);
        if (imgWorldRect.height() < h)
            lDelta.setY(0);
    }

    translateViewInWidgetCoords(lDelta.x(), lDelta.y());
    controlImagePosition();
}

void DkViewPortTransformViewModel::panLeft()
{
    const qreal delta = -sPanFraction * static_cast<qreal>(mViewportRect.width());
    moveViewInWidgetCoords(QPointF(delta, 0));
}

void DkViewPortTransformViewModel::panRight()
{
    const qreal delta = sPanFraction * static_cast<qreal>(mViewportRect.width());
    moveViewInWidgetCoords(QPointF(delta, 0));
}

void DkViewPortTransformViewModel::panUp()
{
    const qreal delta = -sPanFraction * static_cast<qreal>(mViewportRect.height());
    moveViewInWidgetCoords(QPointF(0, delta));
}

void DkViewPortTransformViewModel::panDown()
{
    const qreal delta = sPanFraction * static_cast<qreal>(mViewportRect.height());
    moveViewInWidgetCoords(QPointF(0, delta));
}

void DkViewPortTransformViewModel::zoomLeveled(double factor, const QPointF &center)
{
    if (mZoomLevelSettingProvider) {
        const auto levels = mZoomLevelSettingProvider();
        if (levels) {
            factor = nextFactor(levels.value(), zoomLevel(), factor);
        }
    }

    zoom(factor, center);
}

void DkViewPortTransformViewModel::zoomIn()
{
    zoomLeveled(1.5);
}

void DkViewPortTransformViewModel::zoomOut()
{
    zoomLeveled(0.5);
}

void DkViewPortTransformViewModel::syncTransform(const QPointF &pos, qreal zoomLevel, bool isRelativeTranslation)
{
    if (mImgRect.size().isEmpty()) {
        return;
    }

    if (isRelativeTranslation) {
        moveViewInWidgetCoords(pos);
        return;
    }

    zoomTo(zoomLevel);

    QPointF viewPortCenter = QPointF(pos.x() * mImgRect.width(), pos.y() * mImgRect.height());
    viewPortCenter = mImgMatrix.map(viewPortCenter);
    viewPortCenter = mWorldMatrix.map(viewPortCenter);

    // compute difference to current mViewport center - in world coordinates
    viewPortCenter = QPointF(mViewportRect.width() * 0.5, mViewportRect.height() * 0.5) - viewPortCenter;

    translateViewInWidgetCoords(viewPortCenter.x(), viewPortCenter.y());
    emit transformChanged();
}

DkViewPortTransformViewModel::ZoomPos DkViewPortTransformViewModel::calcZoomCenterLimitToImageEdge(
    const QPointF &center) const
{
    QRectF viewRect = getImageViewRect();
    QPointF pos = center;

    // if no center assigned: zoom in at the image center
    if (pos.x() == -1 || pos.y() == -1) {
        pos = viewRect.center();
    }

    if (pos.x() < viewRect.left()) {
        pos.setX(viewRect.left());
    } else if (pos.x() > viewRect.right()) {
        pos.setX(viewRect.right());
    }
    if (pos.y() < viewRect.top()) {
        pos.setY(viewRect.top());
    } else if (pos.y() > viewRect.bottom()) {
        pos.setY(viewRect.bottom());
    }

    return {pos};
}

DkViewPortTransformViewModel::ZoomPos DkViewPortTransformViewModel::calcZoomCenterLimitCenterSmallDimension(
    const QPointF &center,
    double factor) const
{
    // if no center assigned: zoom in at the image center
    if (center.x() == -1 || center.y() == -1) {
        return {mImgViewRect.center()};
    }

    QPointF pos = center;
    bool recenter = false;
    const QSizeF scaledSize = imageViewSize() * factor;
    // if the image does not fill the view port - do not zoom to the mouse coordinate
    if (scaledSize.width() < mViewportRect.width()) {
        pos.setX(mImgViewRect.center().x());
        recenter |= factor < 1;
    }
    if (scaledSize.height() < mViewportRect.height()) {
        pos.setY(mImgViewRect.center().y());
        recenter |= factor < 1;
    }

    return {pos, recenter};
}

void DkViewPortTransformViewModel::setMinZoomLevelTo1()
{
    mMinZoom = 1 / mImgMatrix.m11();
    emit zoomLevelRangeChanged();
}

QRectF DkViewPortTransformViewModel::viewportInImageCoords() const
{
    return mImgMatrix.inverted().mapRect(mWorldMatrix.inverted().mapRect(mViewportRect));
}
}
