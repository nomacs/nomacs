/*******************************************************************************************************
 DkBaseViewPort.h
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

#pragma once

#include <QBuffer>
#include <QGraphicsView>
#include <memory>

#include "DkImageStorage.h"
#include "DkSettings.h"
#include "DkViewPortImageViewModel.h"
#include "DkViewPortTransformViewModel.h"

#include "nmc_config.h"

class QGestureEvent;
class QShortcut;
class QSvgRenderer;

namespace nmc
{
class DllCoreExport DkBaseViewPort : public QGraphicsView
{
    Q_OBJECT

public:
    enum swipes {
        no_swipe = 0, // dummy for now
        next_image,
        prev_image,
        open_thumbs,
        close_thumbs,
        open_metadata,
        close_metadata,

        swipes_end
    };

    explicit DkBaseViewPort(bool inDialog, QWidget *parent = nullptr, bool resetWhenZoomPastFit = true);
    ~DkBaseViewPort() override;

    void setMinZoomLevelTo1();

    void setForceFastRendering(bool fastRendering = true)
    {
        mForceFastRendering = fastRendering;
    };

    // world to viewport/widget transform
    QTransform getWorldMatrix() const
    {
        return mTransformVM->worldMatrix();
    }

    // image(device-normalized) to world transform
    QTransform getImageMatrix() const
    {
        return mTransformVM->imgMatrix();
    }

    // visible region of the image, unscaled
    QImage getCurrentImageRegion();

    virtual QImage getImage() const;

    // image size in logical pixels (actual size divided by device pixel ratio)
    QSizeF getImageSize() const;

    // getImageViewRect returns the rectangle that contains the image in the
    // coordinates of this widget.
    QRectF getImageViewRect() const;

    // map point in this widget's local coordinates to image pixel
    QPointF mapToImagePixel(const QPointF &p);

    void zoomTo(double zoomLevel);

    [[nodiscard]] DkViewPortTransformViewModel::ZoomLevelRange zoomLevelRange() const;

signals:
    void imageUpdated() const; // triggers on zoom/pan
    void zoomLevelRangeChanged() const;

public slots:
    void moveViewInImageCoords(const QPointF &delta);
    void moveViewInWidgetCoords(const QPointF &delta);
    virtual void fullView();

    virtual void setImage(const QImage &newImg);

protected:
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void zoom(double factor, const QPointF &center = QPointF(-1, -1), bool force = false);
    bool imageInside() const;

    // slots
    virtual void togglePattern(bool show);

    Qt::KeyboardModifier mAltMod; // it makes sense to switch these modifiers on linux (alt + mouse moves windows there)

    QSharedPointer<QMovie> mMovie;
    QSharedPointer<QBuffer> mMovieIo;
    QSharedPointer<QSvgRenderer> mSvg;

    QTimer *mHideCursorTimer;

    QPointF mPosGrab;

    bool mForceFastRendering = false;

    // flags to draw() call for multi-pass rendering
    enum RenderFlag {
        draw_image = 1, // draw image
        draw_background = 2, // fill viewport background
        draw_pattern = 4, // draw transparency pattern behind image
        draw_default = draw_background | draw_pattern | draw_image
    };

    /**
     * @brief context needed for drawing images or image overlays/backgrounds
     *
     * @note All members should be considered const; use getRenderParams() to modify
     */
    struct RenderParams {
        // inputs to getRenderParams()
        double devicePixelRatio; // scale factor from viewPort to screen/device
        QRectF imgViewRect; // image rect scaled and centered to viewPort, logical coordinates
        QTransform worldMatrix; // transform applied to imgViewRect (zoom & pan) => viewPort coordinates
        // outputs
        QRectF dstRect; // target rect for drawImage() with world matrix
        QRectF viewRect; // target rect for drawImage() without world matrix
        QRect deviceRect; // source rect in screen/target coordinates, for read-back from render target
        QSize imageSize; // size of image needed for AA (rounded to device pixels)
    };

    /**
     * @brief get parameters needed to draw image
     * @param devicePixelRatio device scaling of viewport/render target
     * @param worldMatrix world matrix of painter like mWorldMatrix
     * @param imgViewRect image rect fit to viewport like mImgViewRect
     *
     * @note This is intentionally static for use outside of normal draw() flow
     */
    static RenderParams getRenderParams(double devicePixelRatio,
                                        const QTransform &worldMatrix,
                                        const QRectF &imgViewRect);

    /**
     * @brief draw image from precalculated parameters
     * @param painter target
     * @param img full-size or scaled image
     * @param params from getRenderParams()
     *
     * @note painter must have world matrix set correctly (normally params.worldMatrix)
     */
    static void renderImage(QPainter &painter, const QImage &img, const RenderParams &params);

    /**
     * @brief draw transparency pattern behind where the image will draw
     * @param painter target
     * @param params from getRenderParams()
     *
     * @note painter must have world matrix set correctly (normally params.worldMatrix)
     */
    static void renderPattern(QPainter &painter, const RenderParams &params);

    /**
     * @brief get offscreen surface suitable for draw() or renderComposite()
     * @param imageFormat format of image to draw to the buffer
     * @return image of the appropriate size and devicePixelRatio
     */
    QImage renderBuffer(QImage::Format format) const;

    /**
     * @brief offscreen rendering of viewport
     * @param painter target (attached to renderBuffer)
     * @param img source image (already scaled or color converted)
     * @param params
     * @param flags
     */
    void renderComposite(QPainter &painter,
                         const QImage &img,
                         const RenderParams &params,
                         int flags = draw_default) const;

    // draw the entire viewport
    void draw(QPainter &frontPainter, double opacity = 1.0, int flags = draw_default);

    // fill entire viewport with bg color, image draws on top
    virtual void eraseBackground(QPainter &painter) const;

    [[nodiscard]] qreal zoomLevel() const;

    // The returned view model is owned by DkBaseViewPort
    DkViewPortTransformViewModel *transformVM() const
    {
        return mTransformVM.get();
    }

    [[nodiscard]] DkViewPortImageViewModel *imageVM() const
    {
        return mImageVM.get();
    }

private:
    bool gestureEvent(QGestureEvent *event);

    void changeCursor();

    // Slots
    void scrollVertically(int val);
    void scrollHorizontally(int val);
    void hideCursor();

    void updateRenderer();

    Qt::KeyboardModifier mCtrlMod;
    QBrush mPattern;
    QImage mBackBuffer;

    std::unique_ptr<DkViewPortTransformViewModel> mTransformVM = nullptr;
    std::unique_ptr<DkViewPortImageViewModel> mImageVM = nullptr;
};

}
