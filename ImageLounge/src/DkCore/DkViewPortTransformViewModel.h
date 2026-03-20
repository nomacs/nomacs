// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QRectF>
#include <QTimer>
#include <QTransform>
#include <cstdint>
#include <functional>

#include "DkSettings.h"
#include "nmc_config.h"

namespace nmc
{
class DllCoreExport DkViewPortTransformViewModel : public QObject
{
    Q_OBJECT

public:
    explicit DkViewPortTransformViewModel(double devicePixelRatio, bool zeroPanControl, bool resetWhenZoomPastFit)
        : mZoomTimer{std::make_unique<QTimer>()}
        , mDevicePixelRatio{devicePixelRatio}
        , mZeroPanControl{zeroPanControl}
        , mResetWhenZoomPastFit{resetWhenZoomPastFit}
    {
        mZoomTimer->setSingleShot(true);
        connect(mZoomTimer.get(), &QTimer::timeout, this, &DkViewPortTransformViewModel::stopBlockZooming);
    }

    void setMinZoomLevelTo1();

    [[nodiscard]] QTransform imgMatrix() const
    {
        return mImgMatrix;
    }

    [[nodiscard]] QTransform worldMatrix() const
    {
        return mWorldMatrix;
    }

    void syncTransform(const QTransform &world, const QTransform &img, const QPointF &canvasSize);

    [[nodiscard]] QRectF imgViewRect() const
    {
        return mImgViewRect;
    }

    [[nodiscard]] QRectF viewPortRect() const
    {
        return mViewportRect;
    }

    void setDevicePixelRatio(double v)
    {
        mDevicePixelRatio = v;
    }

    void setWidgetSize(const QSize &size);
    void setImgSize(const QSizeF &size, std::optional<DkSettings::keepZoom> keepZoom = std::nullopt);
    void fullView();
    [[nodiscard]] qreal zoomLevel() const;
    void zoom(double factor, const QPointF &center = QPointF(-1, -1), bool force = false);

    // getImageViewRect returns the rectangle that contains the image in the
    // coordinates of this widget.
    [[nodiscard]] QRectF getImageViewRect() const;

    [[nodiscard]] bool imageInside() const;

    [[nodiscard]] QPointF mapToImagePixel(const QPointF &p) const;

    struct ZoomPos {
        QPointF pos;
        bool recenter = false;
    };

    enum class ZoomCenterLimit : std::uint8_t {
        None,
        CenterSmallDimension,
        ToImageEdge,
    };

    void setZoomCenterLimit(ZoomCenterLimit v)
    {
        mZoomCenterLimit = v;
    }

    [[nodiscard]] bool upscaled() const
    {
        return mWorldMatrix.m11() > 1;
    }

    struct ZoomLevelRange {
        qreal mMin;
        qreal mMax;
    };
    [[nodiscard]] ZoomLevelRange zoomLevelRange() const;

    void zoomTo(double zoomLevel);

    void zoomToFit();

    void moveViewInWidgetCoords(const QPointF &delta);

    void setDisablePanForSmallDimension(bool v)
    {
        mDisablePanForSmallDimension = v;
    }

    using ShowScrollBarSettingProvider = std::function<bool()>;
    void setShowScrollBarSettingProvider(const ShowScrollBarSettingProvider &v)
    {
        mShowScrollBarSettingProvider = v;
    }

    using ZoomLevelSettingProvider = std::function<std::optional<QVector<double>>()>;
    void setZoomLevelSettingProvider(const ZoomLevelSettingProvider &v)
    {
        mZoomLevelSettingProvider = v;
    }

    void panLeft();
    void panRight();
    void panUp();
    void panDown();

    void zoomLeveled(double factor, const QPointF &center = QPointF(-1, -1));
    void zoomIn();
    void zoomOut();

    void setDisableControlAndCenter(bool v)
    {
        mDisableControlAndCenter = v;
    }

signals:
    void transformChanged();
    void zoomLevelRangeChanged();

private:
    std::unique_ptr<QTimer> mZoomTimer = nullptr;
    QTransform mImgMatrix;
    QTransform mWorldMatrix;
    QRectF mImgViewRect;
    QRectF mViewportRect;
    QRectF mImgRect;
    ShowScrollBarSettingProvider mShowScrollBarSettingProvider;
    ZoomLevelSettingProvider mZoomLevelSettingProvider;

    // mMinZoom is the constraint on zoomLevel relative to the default state
    // (when fit to view for image larger than the viewport or 100% for image smaller)
    double mMinZoom = 0.01;
    double mDevicePixelRatio = 1;
    static constexpr double sMaxZoomLevel = 100;

    ZoomCenterLimit mZoomCenterLimit = ZoomCenterLimit::None;

    bool mBlockZooming = false;
    // controls whether we cannot pan outside an image
    bool mZeroPanControl = false;

    bool mResetWhenZoomPastFit = true;

    bool mDisablePanForSmallDimension = true;

    bool mDisableControlAndCenter = false;

    static constexpr qreal sPanFraction = 0.02;

    void stopBlockZooming();

    [[nodiscard]] ZoomPos calcZoomCenter(const QPointF &center, double factor) const;
    [[nodiscard]] ZoomPos calcZoomCenterLimitCenterSmallDimension(const QPointF &center, double factor) const;
    [[nodiscard]] ZoomPos calcZoomCenterLimitToImageEdge(const QPointF &center) const;

    void updateImageMatrix(std::optional<DkSettings::keepZoom> keepZoom = std::nullopt, const QSizeF &oldSize = {});
    void setBlockZooming(bool v);
    void zoomToPoint(double factor, const QPointF &pos);
    void translateViewInWidgetCoords(qreal x, qreal y);
    void resetView();
    void controlImagePosition();
    void centerImage();

    // imageViewSize returns the size of the rectangle that contains the image in the
    // coordinates of this widget.
    [[nodiscard]] QSizeF imageViewSize() const;
};

// scaleKeepAspectRatioAndCenter creates a transformation that
// maps coordinates in r1 (`QRectF(QPointF(), src)`)
// to coordinates in r2 (`QRectF(QPointF(), tgt)`)
// when r1 is scaled to maximum inside r2 while keeping aspect ratio
// and the scaled rectangle is centered in r2.
[[nodiscard]] QTransform scaleKeepAspectRatioAndCenter(const QSizeF &src, const QSizeF &tgt);
}
