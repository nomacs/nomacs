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

#include "DkCropWidgets.h"

#include "DkActionManager.h"
#include "DkBasicLoader.h"
#include "DkBasicWidgets.h"
#include "DkImageContainer.h"
#include "DkMath.h"
#include "DkSettings.h"
#include "DkUtils.h"

#pragma warning(push, 0) // no warnings from includes
#include <QComboBox>
#include <QDebug>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>

#include <QMenu>
#include <QSpinBox>
#include <QWidgetAction>

#include <QHBoxLayout>
#include <QMainWindow>
#include <QSlider>
#pragma warning(pop)

namespace nmc
{

// DkCropViewPort --------------------------------------------------------------------
DkCropViewPort::DkCropViewPort(QWidget *parent /* = 0*/)
    : DkBaseViewPort(parent)
{
    mViewportRect = canvas();
    mPanControl = QPointF(0, 0);
    mMinZoom = 1.0;
    mImgWithin = false;

    mCropArea = std::make_shared<DkCropArea>();

    mCropArea->setWorldMatrix(&mWorldMatrix);
    mCropArea->setImageMatrix(&mImgMatrix);
    mCropArea->setImageRect(&mImgViewRect);

    DkActionManager &am = DkActionManager::instance();

    connect(am.action(DkActionManager::menu_edit_rotate_cw), SIGNAL(triggered()), this, SLOT(rotateCW()));
    connect(am.action(DkActionManager::menu_edit_rotate_ccw), SIGNAL(triggered()), this, SLOT(rotateCCW()));
    connect(am.action(DkActionManager::menu_edit_rotate_180), SIGNAL(triggered()), this, SLOT(rotate180()));

    connect(am.action(DkActionManager::menu_view_reset), SIGNAL(triggered()), this, SLOT(resetView()));
    connect(am.action(DkActionManager::menu_view_100), SIGNAL(triggered()), this, SLOT(fullView()));
    connect(am.action(DkActionManager::menu_view_zoom_in), SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(am.action(DkActionManager::menu_view_zoom_out), SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(am.action(DkActionManager::menu_view_tp_pattern), SIGNAL(toggled(bool)), this, SLOT(togglePattern(bool)));
}

void DkCropViewPort::mouseDoubleClickEvent(QMouseEvent *ev)
{
    crop();
    QWidget::mouseDoubleClickEvent(ev);
}

void DkCropViewPort::mouseMoveEvent(QMouseEvent *ev)
{
    if (ev->modifiers() == Qt::ShiftModifier && mCropArea->aspectRatio() == DkCropArea::Ratio::r_free) {
        mCropArea->setTempRatio(DkCropArea::Ratio::r_original);
    } else if (ev->modifiers() != Qt::ShiftModifier) {
        mCropArea->disableTempRatio();
    }

    if (ev->buttons() & Qt::LeftButton) {
        if (mCropArea->currentHandle() != DkCropArea::h_move)
            mCropArea->update(ev->pos());

        update();

        mLastMousePos = ev->pos();
    }

    // propagate moves
    if (mCropArea->currentHandle() == DkCropArea::h_move) {
        ev->ignore();
        DkBaseViewPort::mouseMoveEvent(ev);
    } else {
        QCursor c = mCropArea->cursor(ev->pos());
        setCursor(c);
    }
}

void DkCropViewPort::mousePressEvent(QMouseEvent *ev)
{
    mLastMousePos = ev->pos();
    mCropArea->updateHandle(ev->pos());

    // create a new crop rect
    if (mCropArea->currentHandle() == DkCropArea::h_new)
        mCropArea->makeRectAt(ev->pos());

    // propagate moves
    if (mCropArea->currentHandle() == DkCropArea::h_move)
        DkBaseViewPort::mousePressEvent(ev);
}

void DkCropViewPort::mouseReleaseEvent(QMouseEvent *ev)
{
    mCropArea->resetHandle();
    mCropArea->commitMouseAction();

    // propagate moves
    if (mCropArea->currentHandle() == DkCropArea::h_move)
        DkBaseViewPort::mouseReleaseEvent(ev);

    recenter();
}

void DkCropViewPort::paintEvent(QPaintEvent *pe)
{
    DkBaseViewPort::paintEvent(pe);

    // create path
    QPainterPath path;
    QRect canvas(geometry().x() - mStyle.pen().width(),
                 geometry().y() - mStyle.pen().width(),
                 geometry().width() + 2 * mStyle.pen().width(),
                 geometry().height() + 2 * mStyle.pen().width());
    path.addRect(canvas);

    QRectF crop = mCropArea->rect();
    path.addRect(crop);

    // init painter
    QPainter painter(viewport());

    painter.setPen(mStyle.pen());
    painter.setBrush(mStyle.bgBrush());
    painter.setRenderHint(QPainter::Antialiasing);

    painter.drawPath(path);

    // draw guides
    auto drawGuides = [&](int N = 3) {
        for (int idx = 0; idx < N; idx++) {
            // vertical lines
            double l = crop.left() + crop.width() / N * idx;
            QLineF line(QPointF(l, crop.top()), QPointF(l, crop.bottom()));
            painter.drawLine(line);

            // horizontal lines
            l = crop.top() + crop.height() / N * idx;
            line = QLineF(QPointF(crop.left(), l), QPointF(crop.right(), l));
            painter.drawLine(line);
        }
    };

    // highlight rectangle's corner
    auto drawCorners = [&](const QRectF &r, double width = 30) {
        painter.setPen(mStyle.cornerPen());

        QPointF p = r.topLeft();
        painter.drawLine(p, QPointF(p.x() + width, p.y()));
        painter.drawLine(p, QPointF(p.x(), p.y() + width));

        p = r.topRight();
        painter.drawLine(p, QPointF(p.x() - width, p.y()));
        painter.drawLine(p, QPointF(p.x(), p.y() + width));

        p = r.bottomRight();
        painter.drawLine(p, QPointF(p.x() - width, p.y()));
        painter.drawLine(p, QPointF(p.x(), p.y() - width));

        p = r.bottomLeft();
        painter.drawLine(p, QPointF(p.x() + width, p.y()));
        painter.drawLine(p, QPointF(p.x(), p.y() - width));
    };

    // draw decorations
    drawGuides(mIsRotating ? 10 : 3);
    drawCorners(crop);

    //// debug vis remove! -------------------------
    // painter.setBrush(Qt::NoBrush);

    // QPen debug;
    // debug.setColor(QColor(201, 60, 140));
    // debug.setWidth(3);
    // debug.setStyle(Qt::DashLine);
    // painter.setPen(debug);
    ////painter.drawRect(mCropArea->imgViewRect());

    ////QTransform t = mCropArea->transformCropToRect(winRect());
    ////QRectF tr = t.mapRect(crop);
    ////painter.drawRect(tr);
    // painter.drawRect(mViewportRect);
    //// debug vis remove! -------------------------
}

void DkCropViewPort::resizeEvent(QResizeEvent *re)
{
    if (re->oldSize() == re->size())
        return;

    updateViewRect(canvas());
    recenter();

    return QGraphicsView::resizeEvent(re);
}

void DkCropViewPort::controlImagePosition(const QRect &r)
{
    QRect cr = controlRect(r);

    QRect imgr = mWorldMatrix.mapRect(mImgViewRect).toRect();

    if (imgr.left() > cr.left())
        mWorldMatrix.translate(((double)cr.left() - imgr.left()) / mWorldMatrix.m11(), 0);

    if (imgr.top() > cr.top())
        mWorldMatrix.translate(0, ((double)cr.top() - imgr.top()) / mWorldMatrix.m11());

    if (imgr.right() < cr.right())
        mWorldMatrix.translate(((double)cr.right() - imgr.right()) / mWorldMatrix.m11(), 0);

    if (imgr.bottom() < cr.bottom())
        mWorldMatrix.translate(0, ((double)cr.bottom() - imgr.bottom()) / mWorldMatrix.m11());

    // update scene size (this is needed to make the scroll area work)
    if (DkSettingsManager::instance().param().display().showScrollBars)
        setSceneRect(getImageViewRect());
}

QRect DkCropViewPort::canvas(int margin) const
{
    return QRect(margin, margin, width() - 2 * margin, height() - 2 * margin);
}

void DkCropViewPort::updateViewRect(const QRect &r)
{
    if (r == mViewportRect)
        return;

    mViewportRect = r;
    updateImageMatrix();
    changeCursor();
}

QTransform DkCropViewPort::getScaledImageMatrix() const
{
    QSize s = size();
    if (!mViewportRect.isNull())
        s = mViewportRect.size();

    return DkBaseViewPort::getScaledImageMatrix(s);
}

void DkCropViewPort::recenter()
{
    mCropArea->recenter(canvas());
    updateViewRect(mCropArea->rect());
    // controlImagePosition(); // TODO: do we need that?

    update();
}

void DkCropViewPort::askBeforeClose()
{
    if (!mIsDirty || !mCropArea->isActive())
        return;

    QMessageBox *msg =
        new QMessageBox(QMessageBox::Question, tr("Crop Image"), tr("Do you want to apply cropping?"), (QMessageBox::Yes | QMessageBox::No), this);
    msg->setButtonText(QMessageBox::Yes, tr("&Crop"));
    msg->setButtonText(QMessageBox::No, tr("&Don't Crop"));

    int answer = msg->exec();
    applyCrop(answer == QMessageBox::Yes);
}

void DkCropViewPort::setImageContainer(const QSharedPointer<DkImageContainerT> &img)
{
    mImage = img;

    if (img) {
        DkBaseViewPort::setImage(mImage->image());
        reset();
    }
}

void DkCropViewPort::applyCrop(bool apply)
{
    if (apply)
        crop();

    // close
    mIsDirty = false;
    emit closeSignal();
}

void DkCropViewPort::crop()
{
    qDebug() << "cropping for you sir...";

    if (!mImage) {
        qWarning() << "cannot crop NULL image...";
        return;
    }

    if (mCropArea->isActive() || mAngle != 0.0) {
        QRect r = mCropArea->mapToImage(mCropArea->rect());

        QTransform t;
        t.translate(-r.left(), -r.top());
        rotateTransform(t, mAngle, r.center());

        mImage->cropImage(r, t);
    }
}

void DkCropViewPort::rotateCW()
{
    rotateWithReset(90);
}

void DkCropViewPort::rotateCCW()
{
    rotateWithReset(-90);
}

void DkCropViewPort::rotate180()
{
    rotateWithReset(180);
}

void DkCropViewPort::rotateWithReset(double angle)
{
    if (!mImage)
        return;

    QImage img = DkImage::rotate(mImage->image(), angle);
    mImage->setImage(img, tr("Rotated"));

    DkBaseViewPort::setImage(mImage->image());
    reset();
}

void DkCropViewPort::reset()
{
    mCropArea->reset();
    recenter();
    resetWorldMatrix();
    emit resetSignal();
    mIsDirty = true;
}

void DkCropViewPort::setWorldTransform(QTransform *worldMatrix)
{
    mCropArea->setWorldMatrix(worldMatrix);
}

void DkCropViewPort::setImageRect(const QRectF *rect)
{
    mCropArea->setImageRect(rect);
}

void DkCropViewPort::setVisible(bool visible)
{
    if (!isVisible() && visible) {
        if (!mCropDock) {
            mCropDock = new QDockWidget(this);
            mCropDock->setContentsMargins(0, 0, 0, 0);
            mCropDock->setObjectName("cropDock");
            mCropDock->setTitleBarWidget(new QWidget());

            DkCropToolBar *ctb = new DkCropToolBar(mCropArea, this);
            connect(ctb, &DkCropToolBar::rotateSignal, this, &DkCropViewPort::rotate);
            connect(ctb, &DkCropToolBar::aspectRatioSignal, this, &DkCropViewPort::setAspectRatio);
            connect(ctb, &DkCropToolBar::flipSignal, this, &DkCropViewPort::flip);
            connect(ctb, &DkCropToolBar::isRotatingSignal, this, [&](bool r) {
                mIsRotating = r;
                update();
            });
            connect(ctb, &DkCropToolBar::update, this, [&]() {
                update();
            });
            connect(ctb, &DkCropToolBar::closeSignal, this, &DkCropViewPort::applyCrop);
            connect(this, &DkCropViewPort::resetSignal, ctb, &DkCropToolBar::reset);

            mCropDock->setWidget(ctb);
        }

        auto w = dynamic_cast<QMainWindow *>(DkUtils::getMainWindow());
        if (w) {
            w->addDockWidget(Qt::BottomDockWidgetArea, mCropDock);
        }
    }

    if (mCropDock)
        mCropDock->setVisible(visible);

    DkBaseViewPort::setVisible(visible);
}

void DkCropViewPort::rotate(double angle)
{
    mAngle = angle;
    update();
}

void DkCropViewPort::setAspectRatio(const DkCropArea::Ratio &ratio)
{
    mCropArea->setAspectRatio(ratio);
    recenter();
}

void DkCropViewPort::flip()
{
    mCropArea->flip();
    recenter();
}

bool DkCropArea::isActive() const
{
    const QRect &r = rect();

    bool aw = qAbs(r.width() / mWorldMatrix->m11() - mImgViewRect->width()) > 1;
    bool ah = qAbs(r.height() / mWorldMatrix->m11() - mImgViewRect->height()) > 1;

    return aw || ah;
}

void DkCropArea::setWorldMatrix(QTransform *matrix)
{
    mWorldMatrix = matrix;
}

void DkCropArea::setImageMatrix(QTransform *matrix)
{
    mImgMatrix = matrix;
}

void DkCropArea::setImageRect(const QRectF *rect)
{
    Q_ASSERT(rect);
    mImgViewRect = rect;
}

void DkCropArea::makeRectAt(const QPoint &pos)
{
    QRect r(pos, QSize(1, 1));
    mCropRect = r;
    mNewRect = true;

    mCurrentHandle = Handle::h_bottom_right;
}

void DkCropArea::commitMouseAction()
{
    if (mTmpRatio)
        mRatio = Ratio::r_free;

    // commit new rect?
    if (mNewRect) {
        const QRect &r = rect();

        // reject?
        if (r.width() < 5 && r.height() < 5) {
            mCropRect = QRect();
        }
        mNewRect = false;
    }
}

void DkCropArea::setRect(const QRect &r)
{
    QRect nr = r;
    clip(nr);

    // if (mRatio != Ratio::r_free)
    //	applyRatio(nr, toRatio(mRatio));

    mCropRect = nr;
}

QRect DkCropArea::rect() const
{
    // init the crop rect
    if (mCropRect.isNull()) {
        Q_ASSERT(mWorldMatrix);
        Q_ASSERT(mImgViewRect != nullptr);
        mCropRect = mWorldMatrix->mapRect(*mImgViewRect).toRect();
    }

    return mCropRect;
}

QRect DkCropArea::mapToImage(const QRect &r) const
{
    QRect mr = r;
    mr = mWorldMatrix->inverted().mapRect(mr);
    mr = mImgMatrix->inverted().mapRect(mr);

    return mr;
}

QRect DkCropArea::mapFromImage(const QRect &r) const
{
    QRect mr = r;
    mr = mImgMatrix->mapRect(mr);
    mr = mWorldMatrix->mapRect(mr);

    return mr;
}

DkCropArea::Handle DkCropArea::getHandle(const QPoint &pos, int proximity) const
{
    if (mCurrentHandle != h_no_handle)
        return mCurrentHandle;

    int pxs = proximity * proximity;
    QRect r = rect();

    // squared euclidean distance
    auto dist = [](const QPoint &p1, const QPoint &p2) {
        int dx = p1.x() - p2.x();
        int dy = p1.y() - p2.y();

        return dx * dx + dy * dy;
    };

    if (dist(r.topLeft(), pos) < pxs)
        return Handle::h_top_left;
    else if (dist(r.bottomRight(), pos) < pxs)
        return Handle::h_bottom_right;
    else if (dist(r.topRight(), pos) < pxs)
        return Handle::h_top_right;
    else if (dist(r.bottomLeft(), pos) < pxs)
        return Handle::h_bottom_left;
    else if (qAbs(r.left() - pos.x()) < proximity)
        return Handle::h_left;
    else if (qAbs(r.right() - pos.x()) < proximity)
        return Handle::h_right;
    else if (qAbs(r.top() - pos.y()) < proximity)
        return Handle::h_top;
    else if (qAbs(r.bottom() - pos.y()) < proximity)
        return Handle::h_bottom;
    else if (r.contains(pos) && isActive())
        return Handle::h_move;
    else if (r.contains(pos))
        return Handle::h_new;

    return Handle::h_no_handle;
}

// QPointF DkCropArea::mapToImage(const QPoint& pos) const {
//	Q_ASSERT(mWorldMatrix);
//	return mWorldMatrix->inverted().map(pos);
// }

void DkCropArea::updateHandle(const QPoint &pos)
{
    mCurrentHandle = getHandle(pos);
}

void DkCropArea::resetHandle()
{
    mCurrentHandle = h_no_handle;
}

QCursor DkCropArea::cursor(const QPoint &pos) const
{
    Handle h = getHandle(pos);

    if (h == h_top_left || h == h_bottom_right) {
        return Qt::SizeFDiagCursor;
    } else if (h == h_top_right || h == h_bottom_left) {
        return Qt::SizeBDiagCursor;
    } else if (h == h_left || h == h_right) {
        return Qt::SizeHorCursor;
    } else if (h == h_top || h == h_bottom) {
        return Qt::SizeVerCursor;
    } else if (h == h_move) {
        return Qt::OpenHandCursor;
    }

    return QCursor();
}

DkCropArea::Handle DkCropArea::currentHandle() const
{
    return mCurrentHandle;
}

void DkCropArea::setAspectRatio(const DkCropArea::Ratio &r)
{
    mRatio = r;

    // don't do anything
    if (r == r_free)
        return;

    QRect nr = rect();
    clip(nr);
    applyRatio(nr, toRatio(r));

    // center it
    mCropRect = moveCenterTo(mCropRect, nr);
}

DkCropArea::Ratio DkCropArea::aspectRatio() const
{
    return mRatio;
}

void DkCropArea::setTempRatio(const DkCropArea::Ratio &r)
{
    if (mRatio == Ratio::r_free) {
        mRatio = r;
        mTmpRatio = true;
    }
}

void DkCropArea::disableTempRatio()
{
    if (mTmpRatio) {
        mRatio = Ratio::r_free;
        mTmpRatio = false;
    }
}

void DkCropArea::applyRatio(QRect &r, double ratio) const
{
    bool landscape = r.width() >= r.height();

    // the new rect should always be smaller...
    if (ratio <= 1) {
        landscape = !landscape;
        ratio = 1.0 / ratio;
    }

    int cl = landscape ? r.width() : r.height();
    int ns = qRound(cl / ratio);

    if (landscape) {
        r.setHeight(ns);
    } else {
        r.setWidth(ns);
    }
}

void DkCropArea::flip()
{
    QRect nr = rect();

    int ow = nr.width();
    nr.setWidth(nr.height());
    nr.setHeight(ow);

    double ratio = (double)nr.width() / nr.height();

    // TODO: there is still an issue with clipping (if you flip from full view 3 times)
    if (clip(nr)) {
        applyRatio(nr, ratio);
    }

    mCropRect = moveCenterTo(mCropRect, nr);
}

double DkCropArea::toRatio(const DkCropArea::Ratio &r)
{
    switch (r) {
    case Ratio::r_square:
        return 1.0;
    case Ratio::r_original:
        return mOriginalRatio;
    case Ratio::r_16_9:
        return 16 / 9.0;
    case Ratio::r_4_3:
        return 4 / 3.0;
    case Ratio::r_3_2:
        return 3 / 2.0;
    }

    qWarning() << "illegal ratio: " << r;

    return 1.0;
}

bool DkCropArea::clip(QRect &r) const
{
    QRect o = mWorldMatrix->mapRect(*mImgViewRect).toRect();
    o = o.intersected(r);

    if (o != r) {
        r = o;
        return true;
    }

    return false;
}

QRect DkCropArea::moveCenterTo(const QRect &from, const QRect &to) const
{
    QRect r = to;
    QPoint dxy = from.center() - to.center();
    r.moveCenter(to.center() + dxy);

    return r;
}

void DkCropArea::move(const QPoint &dxy)
{
    mCropRect.moveCenter(mCropRect.center() - dxy);
}

// void DkCropArea::rotate(double angle) {
//
//	Q_ASSERT(mImgViewRect);
//	Q_ASSERT(mWorldMatrix);
//
//	QPointF c = mCropRect.center();
//
//	//// rotate image around center...
//	//mWorldMatrix->translate(c.x(), c.y());
//	//mWorldMatrix->rotate(angle + getAngle());
//	//mWorldMatrix->translate(-c.x(), -c.y());
// }

void DkCropArea::reset()
{
    mCurrentHandle = Handle::h_no_handle;
    mCropRect = QRect();
    mOriginalRatio = (double)mImgViewRect->width() / mImgViewRect->height();
}

void DkCropArea::recenter(const QRectF &target)
{
    if (target.isNull())
        return;

    QTransform t = transformCropToRect(target);

    mCropRect = t.mapRect(rect());
    *mWorldMatrix = *mWorldMatrix * t;
}

QTransform DkCropArea::transformCropToRect(const QRectF &target) const
{
    QRectF crop = rect();

    if (crop.isNull())
        return QTransform();

    double scale = qMin(target.width() / crop.width(), target.height() / crop.height());

    if (scale == 0.0)
        return QTransform();

    QTransform t;
    t.scale(scale, scale);

    QRectF cs = t.mapRect(crop);
    QPointF dxy(target.center() - cs.center());
    dxy /= scale;

    t.translate(dxy.x(), dxy.y());

    return t;
}

bool DkCropArea::isLandscape() const
{
    const QRect r = rect();
    return r.width() >= r.height();
}

void DkCropArea::update(const QPoint &pos)
{
    if (mCurrentHandle == h_no_handle)
        return;

    // enforce aspect ratios
    auto enforce = [&](const QPoint &p, const QPoint &origin, bool principalDiagonal = true) -> QPoint {
        if (mRatio == Ratio::r_free)
            return p;

        float ar = (float)toRatio(mRatio);

        if (!principalDiagonal)
            ar *= -1.0f;

        // normalized diagonal
        DkVector d = isLandscape() ? DkVector(ar, 1) : DkVector(1, ar);
        d /= d.norm();

        // project p onto the diagonal
        DkVector pv = DkVector(p) - DkVector(origin);
        DkVector lp = d * pv.scalarProduct(d);

        return lp.toQPointF().toPoint() + origin;
    };

    auto clip = [&](QPoint &p) -> bool {
        QPoint src = p;

        // keep position within the image
        QRect ir = mWorldMatrix->mapRect(*mImgViewRect).toRect();
        if (p.x() > ir.right())
            p.setX(ir.right());
        if (p.x() < ir.left())
            p.setX(ir.left());
        if (p.y() > ir.bottom())
            p.setY(ir.bottom());
        if (p.y() < ir.top())
            p.setY(ir.top());

        return src != p;
    };

    QPoint p = pos;

    // fix the other coordinate
    switch (mCurrentHandle) {
    case Handle::h_left:
        p.setY(mCropRect.bottom());
        break;
    case Handle::h_right:
        p.setY(mCropRect.bottom());
        break;
    case Handle::h_top:
        p.setX(mCropRect.right());
        break;
    case Handle::h_bottom:
        p.setX(mCropRect.right());
        break;
    default:
        break;
        // do nothing...
    }

    // update corners
    switch (mCurrentHandle) {
    case Handle::h_top_left: {
        p = enforce(p, mCropRect.bottomRight());
        if (!clip(p) || mRatio == Ratio::r_free)
            mCropRect.setTopLeft(p);
        break;
    }
    case Handle::h_top:
    case Handle::h_top_right: {
        p = enforce(p, mCropRect.bottomLeft(), false);
        if (!clip(p) || mRatio == Ratio::r_free)
            mCropRect.setTopRight(p);
        break;
    }
    case Handle::h_bottom:
    case Handle::h_right:
    case Handle::h_bottom_right: {
        p = enforce(p, mCropRect.topLeft());
        if (!clip(p) || mRatio == Ratio::r_free)
            mCropRect.setBottomRight(p);
        break;
    }
    case Handle::h_left:
    case Handle::h_bottom_left: {
        p = enforce(p, mCropRect.topRight(), false);
        if (!clip(p) || mRatio == Ratio::r_free)
            mCropRect.setBottomLeft(p);
        break;
    }
    }

    if (!mCropRect.isValid()) {
        mCropRect.setWidth(qMax(mCropRect.width(), 1));
        mCropRect.setHeight(qMax(mCropRect.height(), 1));
    }
}

// -------------------------------------------------------------------- DkCropStyle
DkCropStyle::DkCropStyle(const QColor &dark, const QColor &light)
{
    mDarkColor = dark;
    mLightColor = light;
}

QBrush DkCropStyle::bgBrush() const
{
    QColor bb = mDarkColor;
    bb.setAlpha(qRound(mOpacity * 255));

    return bb;
}

QColor DkCropStyle::lightColor() const
{
    return mLightColor;
}

QPen DkCropStyle::pen() const
{
    QPen p(mLightColor, mLineWidth);
    p.setCosmetic(true);

    return p;
}

QPen DkCropStyle::cornerPen() const
{
    QPen p(mLightColor, mLineWidth * 2);
    p.setCosmetic(true);

    return p;
}

// -------------------------------------------------------------------- DkCropEdit
DkCropEdit::DkCropEdit(QWidget *parent)
    : DkWidget(parent)
{
    createLayout();
}

void DkCropEdit::setRect(const QRect &r)
{
    blockSignals(true);
    mRectBoxes[Box::x]->setValue(r.x());
    mRectBoxes[Box::y]->setValue(r.y());
    mRectBoxes[Box::w]->setValue(r.width());
    mRectBoxes[Box::h]->setValue(r.height());
    blockSignals(false);
}

QRect DkCropEdit::rect() const
{
    return QRect(mRectBoxes[Box::x]->value(), mRectBoxes[Box::y]->value(), mRectBoxes[Box::w]->value(), mRectBoxes[Box::h]->value());
}

void DkCropEdit::createLayout()
{
    mRectBoxes.resize(Box::end);

    for (int idx = 0; idx < mRectBoxes.size(); idx++) {
        mRectBoxes[idx] = new QSpinBox(this);
        mRectBoxes[idx]->setMaximum(10000); // FIXME: this is a stupid max val
        connect(mRectBoxes[idx], QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int) {
            emit newRectSignal(rect());
        });
    }

    mRectBoxes[x]->setPrefix(tr("x") + ": ");
    mRectBoxes[y]->setPrefix(tr("y") + ": ");
    mRectBoxes[w]->setPrefix(tr("w") + ": ");
    mRectBoxes[h]->setPrefix(tr("h") + ": ");

    mRectBoxes[x]->setMinimum(0);
    mRectBoxes[y]->setMinimum(0);
    mRectBoxes[w]->setMinimum(1);
    mRectBoxes[h]->setMinimum(1);

    QGridLayout *gl = new QGridLayout(this);
    gl->addWidget(mRectBoxes[x], 1, 1);
    gl->addWidget(mRectBoxes[y], 2, 1);
    gl->addWidget(mRectBoxes[w], 3, 1);
    gl->addWidget(mRectBoxes[h], 4, 1);
}

// -------------------------------------------------------------------- DkCropToolBar
DkCropToolBar::DkCropToolBar(std::shared_ptr<DkCropArea> crop, QWidget *parent)
    : DkWidget(parent)
{
    mCropArea = crop;

    createLayout();
    QMetaObject::connectSlotsByName(this);
}

void DkCropToolBar::createLayout()
{
    setObjectName("darkManipulator");

    QPixmap i = DkImage::loadIcon(":/nomacs/img/crop.svg", QSize(32, 32), QColor(255, 255, 255));
    QPushButton *applyButton = new QPushButton(i, tr("Apply"), this);
    applyButton->setStatusTip(tr("closes the crop view, and applies cropping to the image (ENTER)"));
    applyButton->setShortcut(Qt::Key_Return);
    applyButton->setObjectName("dark");

    i = DkImage::loadIcon(":/nomacs/img/close.svg", QSize(32, 32), QColor(255, 255, 255));
    QPushButton *cancelButton = new QPushButton(i, tr("Cancel"), this);
    cancelButton->setStatusTip(tr("closes the crop view, leaving the image unchanged (ESC)"));
    cancelButton->setShortcut(Qt::Key_Escape);
    cancelButton->setObjectName("dark");

    mAngleSlider = new DkDoubleSlider(tr("Rotate"), this);
    mAngleSlider->setObjectName("darkManipulator");
    mAngleSlider->setTickInterval(1 / 90.0);
    mAngleSlider->setMinimum(-45);
    mAngleSlider->setMaximum(45);
    mAngleSlider->setValue(0.0);
    mAngleSlider->setMaximumWidth(400);

    mRatioBox = new QComboBox(this);
    mRatioBox->setObjectName("ratioBox");

    // dear future me: we can use this with C++20:
    // using enum DkCropArea;
    i = DkImage::loadIcon(":/nomacs/img/aspect-ratio.svg", QSize(32, 32), QColor(255, 255, 255));
    mRatioBox->addItem(i, tr("Aspect Ratio"), DkCropArea::Ratio::r_free);
    mRatioBox->addItem(tr("Free"), DkCropArea::Ratio::r_free);
    mRatioBox->addItem(tr("Original"), DkCropArea::Ratio::r_original);
    mRatioBox->addItem(tr("Square"), DkCropArea::Ratio::r_square);
    mRatioBox->addItem(tr("16:9"), DkCropArea::Ratio::r_16_9);
    mRatioBox->addItem(tr("4:3"), DkCropArea::Ratio::r_4_3);
    mRatioBox->addItem(tr("3:2"), DkCropArea::Ratio::r_3_2);

    i = DkImage::loadIcon(":/nomacs/img/crop-flip.svg", QSize(32, 32), QColor(255, 255, 255));
    QPushButton *flipButton = new QPushButton(i, tr("Flip"), this);
    flipButton->setStatusTip(tr("flips the crop rectangle (F)"));
    flipButton->setShortcut(Qt::Key_F);
    flipButton->setObjectName("dark");

    i = DkImage::loadIcon(":/nomacs/img/bars.svg", QSize(32, 32), QColor(255, 255, 255));
    mEditButton = new QPushButton(i, "", this);
    mEditButton->setStatusTip(tr("edit rectangle coordinates"));
    mEditButton->setObjectName("dark");

    // connections
    auto s = mAngleSlider->getSlider();
    connect(s, &QSlider::sliderPressed, this, [&]() {
        emit isRotatingSignal(true);
    });
    connect(s, &QSlider::sliderReleased, this, [&]() {
        emit isRotatingSignal(false);
    });
    connect(mAngleSlider, &DkDoubleSlider::valueChanged, this, &DkCropToolBar::rotateSignal);
    connect(flipButton, &QPushButton::clicked, this, &DkCropToolBar::flipSignal);
    connect(applyButton, &QPushButton::clicked, this, [&]() {
        emit closeSignal(true);
    });
    connect(cancelButton, &QPushButton::clicked, this, [&]() {
        emit closeSignal(false);
    });
    connect(mEditButton, &QPushButton::clicked, this, [&]() {
        showMenu();
    });

    QHBoxLayout *l = new QHBoxLayout(this);
    l->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    l->setSpacing(20);

    l->addStretch();
    l->addWidget(applyButton);
    l->addWidget(cancelButton);
    l->addSpacing(20);
    l->addWidget(mAngleSlider);
    l->addSpacing(20);
    l->addWidget(mRatioBox);
    l->addWidget(flipButton);
    l->addWidget(mEditButton);
    l->addStretch();
}

void DkCropToolBar::showMenu(const QPoint &pos)
{
    if (!mContextMenu) {
        mContextMenu = new QMenu(this);
        mCropEdit = new DkCropEdit(this);
        connect(mCropEdit, &DkCropEdit::newRectSignal, this, [&](const QRect &r) {
            mCropArea->setRect(mCropArea->mapFromImage(r));
            emit update();
        });

        QWidgetAction *a = new QWidgetAction(this);
        a->setDefaultWidget(mCropEdit);
        mContextMenu->addAction(a);
    }

    // we want the coordinates to be in image coordinates
    mCropEdit->setRect(mCropArea->mapToImage(mCropArea->rect()));
    mContextMenu->exec(!pos.isNull() ? pos : mapToGlobal(mEditButton->geometry().bottomRight()));
}

void DkCropToolBar::on_ratioBox_currentIndexChanged(int idx) const
{
    auto r = static_cast<DkCropArea::Ratio>(mRatioBox->itemData(idx).toInt());
    emit aspectRatioSignal(r);
}

void DkCropToolBar::reset()
{
    mAngleSlider->setValue(0);
    mRatioBox->setCurrentIndex(0);
}

}