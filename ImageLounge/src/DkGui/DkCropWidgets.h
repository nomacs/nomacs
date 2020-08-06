/*******************************************************************************************************
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2016 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2016 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2016 Florian Kleber <florian@nomacs.org>

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

 related links:
 [1] http://www.nomacs.org/
 [2] https://github.com/nomacs/
 [3] http://download.nomacs.org
 *******************************************************************************************************/

#pragma once

#include "DkBaseViewPort.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QPen>
#include <QBrush>
#include <QDockWidget>
#pragma warning(pop)

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QComboBox;

namespace nmc {

// nomacs defines
class DkImageContainerT;
    
class DkCropArea {

public:
    enum Handle : int {

        h_no_handle = 0,
        h_top_left,
        h_top_right,
        h_bottom_right,
        h_bottom_left,
        h_top,
        h_bottom,
        h_left,
        h_right,
        h_move,

        h_end
    };

    enum Ratio : int {
        r_free = 0,
        r_original,
        r_square,
        r_16_9,
        r_4_3,
        r_3_2,
        r_flip,

        r_end,
    };

    void setWorldMatrix(QTransform* matrix);
    void setImageRect(const QRectF* rect);

    QRect rect() const;

    void updateHandle(const QPoint& pos);
    void resetHandle();
    QCursor cursor(const QPoint& pos) const;
    Handle currentHandle() const;

    void setAspectRatio(const DkCropArea::Ratio& r);
    void setOriginalRatio(double ratio);

    void update(const QPoint& pos);
    void move(const QPoint& dxy);
    void reset();

    void recenter(const QRectF& target);

    QPoint mDebugPoint;

private:
    QTransform* mWorldMatrix = nullptr;
    const QRectF* mImgViewRect = nullptr;

    // the crop rect is kept in display coordinates
    mutable QRect mCropRect;
    DkCropArea::Ratio mRatio = r_free;
    DkCropArea::Handle mCurrentHandle = DkCropArea::Handle::h_no_handle;
    
    double mOriginalRatio = 1.0;

    Handle getHandle(const QPoint& pos, int proximity = 15) const;
    QTransform transformCropToRect(const QRectF& target) const;
    bool isLandscape() const;
    void applyRatio(const DkCropArea::Ratio& r);
    double toRatio(const DkCropArea::Ratio& r);

    //QPointF mapToImage(const QPoint& pos) const;
};

class DkCropStyle {

public:

    DkCropStyle(
        const QColor& dark = QColor(0, 0, 0), 
        const QColor& light = QColor(255, 255, 255));

    QBrush bgBrush() const;
    QPen pen() const;
    QPen cornerPen() const;

    QColor lightColor() const;
    
private:
    QColor mDarkColor;
    QColor mLightColor;
    int mLineWidth = 2;
    double mOpacity = 0.8;

};

class DkCropWidget : public DkBaseViewPort {
    Q_OBJECT

public:
    DkCropWidget(QWidget* parent = 0);
    
    void reset();

    void setWorldTransform(QTransform* worldMatrix);
    void setImageRect(const QRectF* rect);

    QRect* cropRect() const;
    void recenter();

public slots:
    void crop(bool cropToMetadata = false);
    void setVisible(bool visible) override;

    void rotate(double angle);
    void setAspectRatio(const DkCropArea::Ratio& ratio);

    void setImageContainer(const QSharedPointer<DkImageContainerT>& img);

signals:
    void cropImageSignal(const QRectF& rect, bool cropToMetaData = false) const;
    void hideSignal() const;

protected:
    void mouseDoubleClickEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;

    void paintEvent(QPaintEvent* pe) override;
    void resizeEvent(QResizeEvent* re) override;

    QRect canvas(int margin = 100) const;
    void updateViewRect(const QRect& r);

    DkCropStyle mStyle;

    DkCropArea mCropArea;
    QRectF mRect; // TODO: remove?
    QPoint mLastMousePos;

    QSharedPointer<DkImageContainerT> mImage;
    QDockWidget* mCropDock = nullptr;

};

// TODO: rename to DkCropToolBar
class DkCropToolBarNew : public QWidget {
    Q_OBJECT

public:
    DkCropToolBarNew(QWidget* parent = 0);

private slots:
    void on_ratioBox_currentIndexChanged(int idx) const;

signals:
    void rotateSignal(double angle) const;
    void aspectRatioSignal(const DkCropArea::Ratio& ratio) const;

private:
    void createLayout();

    QComboBox* mRatioBox = nullptr;

};

}