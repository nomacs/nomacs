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

#pragma warning(push, 0) // no warnings from includes - begin
#include <QGraphicsView>
#pragma warning(pop) // no warnings from includes - end

#pragma warning(disable : 4251) // TODO: remove

#include "DkImageStorage.h"
#include "DkSettings.h"

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QGestureEvent;
class QShortcut;
class QSvgRenderer;
class QSettings;

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

    DkBaseViewPort(QWidget *parent = 0);
    virtual ~DkBaseViewPort();

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

    virtual QTransform getWorldMatrix()
    {
        return mWorldMatrix;
    };
    virtual QRect getMainGeometry()
    {
        return geometry();
    };

    QImage getCurrentImageRegion();

    virtual DkImageStorage *getImageStorage()
    {
        return &mImgStorage;
    };

#ifdef WITH_OPENCV
    virtual void setImage(cv::Mat newImg);
#endif

    virtual QImage getImage() const;
    virtual QSize getImageSize() const;
    virtual QRectF getImageViewRect() const;
    virtual bool imageInside() const;

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
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void stopBlockZooming();
    virtual void setBackgroundBrush(const QBrush &brush);
    void scrollVertically(int val);
    void scrollHorizontally(int val);

    virtual bool unloadImage(bool fileChange = true);
    virtual void setImage(QImage newImg);
    void hideCursor();

protected:
    virtual bool event(QEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

    virtual bool gestureEvent(QGestureEvent *event);

    QVector<QShortcut *> mShortcuts; // TODO: add to actionManager

    Qt::KeyboardModifier mAltMod; // it makes sense to switch these modifiers on linux (alt + mouse moves windows there)
    Qt::KeyboardModifier mCtrlMod;

    DkImageStorage mImgStorage;
    QSharedPointer<QMovie> mMovie;
    QSharedPointer<QSvgRenderer> mSvg;
    QBrush mPattern;

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

    // functions
    virtual void draw(QPainter &painter, double opacity = 1.0);
    virtual void drawPattern(QPainter &painter) const;
    virtual void updateImageMatrix();
    virtual QTransform getScaledImageMatrix() const;
    virtual QTransform getScaledImageMatrix(const QSize &size) const;
    virtual void controlImagePosition(float lb = -1, float ub = -1);
    virtual void centerImage();
    virtual void changeCursor();
    void zoomToPoint(double factor, const QPointF &pos, QTransform &matrix) const;
};

}
