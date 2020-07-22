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

#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QMouseEvent>
#pragma warning(pop)

namespace nmc {

// DkCropWidget --------------------------------------------------------------------
DkCropWidget::DkCropWidget(QWidget* parent /* = 0*/, Qt::WindowFlags f /* = 0*/) : DkFadeWidget(parent, f) {

}

void DkCropWidget::mouseDoubleClickEvent(QMouseEvent* ev) {

    crop();
    QWidget::mouseDoubleClickEvent(ev);
}

void DkCropWidget::mouseMoveEvent(QMouseEvent* ev) {

	QCursor c = mCropArea.cursor(ev->pos());
	setCursor(c);

	if (ev->buttons() & Qt::LeftButton) {

		if (mCropArea.currentHandle() == DkCropArea::h_move)
			mCropArea.move(mLastMousePos - ev->pos());
		else
			mCropArea.update(ev->pos());

		update();

		mLastMousePos = ev->pos();
	}

	// do not propagate
	//DkFadeWidget::mouseMoveEvent(ev);
}

void DkCropWidget::mousePressEvent(QMouseEvent* ev) {

	mLastMousePos = ev->pos();
	mCropArea.updateHandle(ev->pos());
	
	if (mCropArea.currentHandle() == DkCropArea::h_move)
		setCursor(Qt::ClosedHandCursor);

	// do not propagate
	//DkFadeWidget::mousePressEvent(ev);
}

void DkCropWidget::mouseReleaseEvent(QMouseEvent*) {

	mCropArea.resetHandle();

	// do not propagate
	//DkFadeWidget::mouseReleaseEvent(ev);
}

void DkCropWidget::paintEvent(QPaintEvent* pe) {

	// create path
	QPainterPath path;
	QRect canvas(
		geometry().x() - mStyle.pen().width(),
		geometry().y() - mStyle.pen().width(),
		geometry().width() + 2*mStyle.pen().width(),
		geometry().height() + 2*mStyle.pen().width()
		);
	path.addRect(canvas);

	QRectF crop = mCropArea.cropViewRect();
	path.addRect(crop);

	// init painter
	QPainter painter(this);

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
	auto drawCorners = [&](const QRectF& r, double width = 30) {

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
	drawGuides();
	drawCorners(crop);
	
	// this changes the painter -> do it at the end
	if (!mRect.isEmpty()) {

		//for (int idx = 0; idx < mCtrlPoints.size(); idx++) {

		//	QPointF cp;

		//	if (idx < 4) {
		//		QPointF c = p[idx];
		//		cp = c - mCtrlPoints[idx]->getCenter();
		//	}
		//	// paint control points in the middle of the edge
		//	else if (idx >= 4) {
		//		QPointF s = mCtrlPoints[idx]->getCenter();

		//		QPointF lp = p[idx % 4];
		//		QPointF rp = p[(idx + 1) % 4];

		//		QVector2D lv = QVector2D(lp - s);
		//		QVector2D rv = QVector2D(rp - s);

		//		cp = (lv + 0.5 * (rv - lv)).toPointF();
		//	}

		//	mCtrlPoints[idx]->move(qRound(cp.x()), qRound(cp.y()));
		//	mCtrlPoints[idx]->draw(&painter);
		//}
	}

	painter.end();

	QWidget::paintEvent(pe);
}

void DkCropWidget::crop(bool cropToMetadata) {

    if (!mRect.isEmpty())
        emit cropImageSignal(mRect, cropToMetadata);

    setVisible(false);
    setWindowOpacity(0);
    emit hideSignal();
}

void DkCropWidget::reset() {

    // TODO: implement reset here...
}

void DkCropWidget::setTransforms(const QTransform* worldMatrix, const QTransform* imgMatrix) {
    mCropArea.setWorldMatrix(worldMatrix);
    mCropArea.setImageMatrix(imgMatrix);
}

void DkCropWidget::setImageRect(const QRectF* rect) {
    mCropArea.setImageRect(rect);
}

void DkCropWidget::setVisible(bool visible) {

    DkFadeWidget::setVisible(visible);
}

void DkCropArea::setWorldMatrix(const QTransform* matrix) {
    mWorldMatrix = matrix;
}

void DkCropArea::setImageMatrix(const QTransform* matrix) {
    mImgMatrix = matrix;
}

void DkCropArea::setImageRect(const QRectF* rect) {
    mImgViewRect = rect;
}

QRectF DkCropArea::cropViewRect() const {
	
	// init the crop rect
	if (mCropRect.isNull()) {

		Q_ASSERT(mImgViewRect != nullptr);
		mCropRect = *mImgViewRect;
	}

	Q_ASSERT(mWorldMatrix);
	return mWorldMatrix->mapRect(mCropRect);
}

DkCropArea::Handle DkCropArea::getHandle(const QPoint& pos, int proximity) const {
	
	if (mCurrentHandle != h_no_handle)
		return mCurrentHandle;

	int pxs = proximity * proximity;
	QRect r = cropViewRect().toRect();

	// squared euclidean distance
	auto dist = [](const QPoint& p1, const QPoint& p2) {

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
	else if (r.contains(pos))
		return Handle::h_move;

	return Handle::h_no_handle;
}

QPointF DkCropArea::mapToImage(const QPoint& pos) const {
	Q_ASSERT(mWorldMatrix);
	return mWorldMatrix->inverted().map(pos);
}

void DkCropArea::updateHandle(const QPoint& pos) {

	mCurrentHandle = getHandle(pos);
}

void DkCropArea::resetHandle() {
	mCurrentHandle = h_no_handle;
}

QCursor DkCropArea::cursor(const QPoint& pos) const {
	
	Handle h = getHandle(pos);

	if (h == h_top_left ||
		h == h_bottom_right) {
		return Qt::SizeFDiagCursor;
	}
	else if (h == h_top_right ||
		h == h_bottom_left) {
		return Qt::SizeBDiagCursor;
	}
	else if (h == h_left ||
		h == h_right) {
		return Qt::SizeHorCursor;
	}
	else if (h == h_top ||
		h == h_bottom) {
		return Qt::SizeVerCursor;
	}
	else if (h == h_move) {
		return Qt::OpenHandCursor;
	}

	return QCursor();
}

DkCropArea::Handle DkCropArea::currentHandle() const {
	return mCurrentHandle;
}

void DkCropArea::move(const QPoint& dxy) {
	mCropRect.moveCenter(mCropRect.center() - dxy);
}

void DkCropArea::update(const QPoint& pos) {

	if (mCurrentHandle == h_no_handle)
		return;

	QPointF ipos = mapToImage(pos);

	switch (mCurrentHandle) {

	case Handle::h_top_left:
		mCropRect.setTopLeft(ipos); break;
	case Handle::h_top_right:
		mCropRect.setTopRight(ipos); break;
	case Handle::h_bottom_right:
		mCropRect.setBottomRight(ipos); break;
	case Handle::h_bottom_left:
		mCropRect.setBottomLeft(ipos); break;
	case Handle::h_left:
		mCropRect.setLeft(ipos.x()); break;
	case Handle::h_right:
		mCropRect.setRight(ipos.x()); break;
	case Handle::h_top:
		mCropRect.setTop(ipos.y()); break;
	case Handle::h_bottom:
		mCropRect.setBottom(ipos.y()); break;
	}
}

// -------------------------------------------------------------------- DkCropStyle 
DkCropStyle::DkCropStyle(const QColor& dark, const QColor& light) {

	mDarkColor = dark;
	mLightColor = light;
}

QBrush DkCropStyle::bgBrush() const {

	QColor bb = mDarkColor;
	bb.setAlpha(qRound(mOpacity * 255));

	return bb;
}

QColor DkCropStyle::lightColor() const {
	return mLightColor;
}

QPen DkCropStyle::pen() const {

	QPen p(mLightColor, mLineWidth);
	p.setCosmetic(true);

	return p;
}

QPen DkCropStyle::cornerPen() const {

	QPen p(mLightColor, mLineWidth*2);
	p.setCosmetic(true);

	return p;
}

}