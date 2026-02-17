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

#include "DkImageStorage.h"

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

    explicit DkBaseViewPort(QWidget *parent = nullptr);
    ~DkBaseViewPort() override;

    void zoomConstraints(double minZoom = 0.01, double maxZoom = 100.0);
    virtual void zoom(double factor = 0.5, const QPointF &center = QPointF(-1, -1), bool force = false);
    virtual void zoomLeveled(double factor = 0.5, const QPointF &center = QPointF(-1, -1));

    void setForceFastRendering(bool fastRendering = true)
    {
        mForceFastRendering = fastRendering;
    };

    /**
     * Returns the scale factor for 100%.
     * Note this factor is only valid for the current image.
     * @return float
     **/
    float get100Factor()
    {
        updateImageMatrix();
        return 1.0f / (float)mImgMatrix.m11();
    };

    void setPanControl(QPointF panControl)
    {
        mPanControl = panControl;
    };

    // world to viewport/widget transform
    QTransform getWorldMatrix() const
    {
        return mWorldMatrix;
    }

    // image(device-normalized) to world transform
    QTransform getImageMatrix() const
    {
        return mImgMatrix;
    }

    virtual QRect getMainGeometry()
    {
        return geometry();
    };

    // visible region of the image, unscaled
    QImage getCurrentImageRegion();

    virtual DkImageStorage *getImageStorage()
    {
        return &mImgStorage;
    };

    virtual QImage getImage() const;

    // image size in logical pixels (actual size divided by device pixel ratio)
    QSizeF getImageSize() const;

    QRectF getImageViewRect() const;
    bool imageInside() const;

    // map point in this widget's local coordinates to image logical (device-normalized) coordinates
    QPointF mapToImage(const QPointF &p);

    // map point in this widget's local coordinates to image pixel
    QPointF mapToImagePixel(const QPointF &p);

signals:
    void keyReleaseSignal(QKeyEvent *event) const; // make key presses available
    void imageUpdated() const; // triggers on zoom/pan

public slots:
    virtual void togglePattern(bool show);
    virtual void panLeft();
    virtual void panRight();
    virtual void panUp();
    virtual void panDown();
    virtual void moveView(const QPointF &);
    virtual void zoomIn();
    virtual void zoomOut();
    virtual void resetView();
    virtual void fullView();
    void resizeEvent(QResizeEvent *event) override;
    virtual void stopBlockZooming();
    virtual void setBackgroundBrush(const QBrush &brush);
    void scrollVertically(int val);
    void scrollHorizontally(int val);

    virtual void setImage(QImage newImg);
    void hideCursor();

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

    virtual bool gestureEvent(QGestureEvent *event);

    QVector<QShortcut *> mShortcuts; // TODO: add to actionManager

    Qt::KeyboardModifier mAltMod; // it makes sense to switch these modifiers on linux (alt + mouse moves windows there)
    Qt::KeyboardModifier mCtrlMod;

    DkImageStorage mImgStorage;
    QSharedPointer<QMovie> mMovie;
    QSharedPointer<QBuffer> mMovieIo;
    QSharedPointer<QSvgRenderer> mSvg;

    QTransform mImgMatrix;
    QTransform mWorldMatrix;
    QRectF mImgViewRect;
    QRectF mViewportRect;
    QRectF mImgRect;
    QTimer *mHideCursorTimer;

    QPointF mPanControl; // controls how far we can pan outside an image
    QPointF mPosGrab;
    double mMinZoom = 0.01;
    double mMaxZoom = 100;

    // TODO: test if gestures are fully supported in Qt5 then remove this
    float mLastZoom;
    float mStartZoom;
    int mSwipeGesture;

    bool mForceFastRendering = false;
    bool mBlockZooming = false;
    QTimer *mZoomTimer;
    QImage mBackBuffer;

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
    virtual void draw(QPainter &frontPainter, double opacity = 1.0, int flags = draw_default);

    // fill entire viewport with bg color, image draws on top
    virtual void eraseBackground(QPainter &painter) const;

    virtual void updateImageMatrix();
    virtual QTransform getScaledImageMatrix() const;
    virtual QTransform getScaledImageMatrix(const QSize &size) const;
    virtual void controlImagePosition(float lb = -1, float ub = -1);
    virtual void centerImage();
    virtual void changeCursor();
    void zoomToPoint(double factor, const QPointF &pos, QTransform &matrix) const;
};

// scaleKeepAspectRatioAndCenter creates a transformation that
// maps coordinates in r1 (`QRectF(QPointF(), src)`)
// to coordinates in r2 (`QRectF(QPointF(), tgt)`)
// when r1 is scaled to maximum inside r2 while keeping aspect ratio
// and the scaled rectangle is centered in r2.
QTransform scaleKeepAspectRatioAndCenter(const QSizeF &src, const QSizeF &tgt, qreal paddingRatio = 0);
}
