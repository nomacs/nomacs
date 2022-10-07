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
#include "DkBaseWidgets.h"

#pragma warning(push, 0) // no warnings from includes
#include <QBrush>
#include <QDockWidget>
#include <QPen>
#pragma warning(pop)

#include <memory>

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
class QSpinBox;
class QMenu;

namespace nmc
{

// nomacs defines
class DkImageContainerT;
class DkDoubleSlider;

class DkCropArea
{
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
        h_new,

        h_end
    };

    enum Ratio : int {
        r_free = 0,
        r_original,
        r_square,
        r_16_9,
        r_4_3,
        r_3_2,

        r_end,
    };

    bool isActive() const;

    void setWorldMatrix(QTransform *matrix);
    void setImageMatrix(QTransform *matrix);
    void setImageRect(const QRectF *rect);

    void makeRectAt(const QPoint &pos);
    void commitMouseAction();

    void setRect(const QRect &r);
    QRect rect() const;
    QRect mapToImage(const QRect &r) const;
    QRect mapFromImage(const QRect &r) const;

    void updateHandle(const QPoint &pos);
    void resetHandle();
    QCursor cursor(const QPoint &pos) const;
    Handle currentHandle() const;

    void setAspectRatio(const DkCropArea::Ratio &r);
    DkCropArea::Ratio aspectRatio() const;
    void setTempRatio(const DkCropArea::Ratio &r);
    void disableTempRatio();
    void flip();

    void update(const QPoint &pos);
    void move(const QPoint &dxy);
    void reset();

    void recenter(const QRectF &target);

private:
    QTransform *mWorldMatrix = nullptr;
    QTransform *mImgMatrix = nullptr;
    const QRectF *mImgViewRect = nullptr;

    // the crop rect is kept in display coordinates
    mutable QRect mCropRect;
    DkCropArea::Ratio mRatio = r_free;
    DkCropArea::Handle mCurrentHandle = DkCropArea::Handle::h_no_handle;

    bool mNewRect = false;
    bool mTmpRatio = false;
    double mOriginalRatio = 1.0;

    Handle getHandle(const QPoint &pos, int proximity = 15) const;
    QTransform transformCropToRect(const QRectF &target) const;
    bool isLandscape() const;
    void applyRatio(QRect &r, double ratio) const;
    double toRatio(const DkCropArea::Ratio &r);
    bool clip(QRect &r) const;

    QRect moveCenterTo(const QRect &from, const QRect &to) const;

    // QPointF mapToImage(const QPoint& pos) const;
};

class DkCropStyle
{
public:
    DkCropStyle(const QColor &dark = QColor(0, 0, 0), const QColor &light = QColor(255, 255, 255));

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

class DkCropViewPort : public DkBaseViewPort
{
    Q_OBJECT

public:
    DkCropViewPort(QWidget *parent = 0);

    void reset();

    void setWorldTransform(QTransform *worldMatrix);
    void setImageRect(const QRectF *rect);

    void recenter();

    void askBeforeClose();

public slots:
    void setVisible(bool visible) override;

    void setAspectRatio(const DkCropArea::Ratio &ratio);
    void flip();

    void setImageContainer(const QSharedPointer<DkImageContainerT> &img);

    void applyCrop(bool apply);
    void crop();

    void rotateCW();
    void rotateCCW();
    void rotate180();
    void rotate(double angle);

signals:
    void cropImageSignal(const QRectF &rect, bool cropToMetaData = false) const;
    void closeSignal() const;
    void resetSignal() const;

protected:
    void mouseDoubleClickEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

    void paintEvent(QPaintEvent *pe) override;
    void resizeEvent(QResizeEvent *re) override;

    void controlImagePosition(const QRect &r = QRect()) override;

    QRect canvas(int margin = 100) const;
    void updateViewRect(const QRect &r);
    QTransform getScaledImageMatrix() const override;
    void rotateWithReset(double angle);

    DkCropStyle mStyle;

    std::shared_ptr<DkCropArea> mCropArea = nullptr;
    QPoint mLastMousePos;

    bool mIsDirty = false;
    bool mIsRotating = false;

    QSharedPointer<DkImageContainerT> mImage;
    QDockWidget *mCropDock = nullptr;
};

class DllCoreExport DkCropEdit : public DkWidget
{
    Q_OBJECT

public:
    DkCropEdit(QWidget *parent = nullptr);

    void setRect(const QRect &r);
    QRect rect() const;

signals:
    void newRectSignal(const QRect &r);

protected:
    void createLayout();

    enum Box {
        x = 0,
        y,
        w,
        h,

        end
    };

    QVector<QSpinBox *> mRectBoxes;
};

// TODO: rename to DkCropToolBar
class DkCropToolBar : public DkWidget
{
    Q_OBJECT

public:
    DkCropToolBar(std::shared_ptr<DkCropArea> crop, QWidget *parent = 0);

public slots:
    void reset();

private slots:
    void on_ratioBox_currentIndexChanged(int idx) const;

signals:
    void isRotatingSignal(bool rotating) const;
    void rotateSignal(double angle) const;
    void aspectRatioSignal(const DkCropArea::Ratio &ratio) const;
    void flipSignal() const;
    void closeSignal(bool apply) const;
    void update() const;

private:
    void createLayout();
    void showMenu(const QPoint &pos = QPoint());

    QMenu *mContextMenu = nullptr;
    DkCropEdit *mCropEdit = nullptr;

    QComboBox *mRatioBox = nullptr;
    DkDoubleSlider *mAngleSlider = nullptr;
    QPushButton *mEditButton = nullptr;

    std::shared_ptr<DkCropArea> mCropArea = nullptr;
};

}