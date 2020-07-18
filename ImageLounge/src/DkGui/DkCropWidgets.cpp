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
#pragma warning(pop)

namespace nmc {

// DkCropWidget --------------------------------------------------------------------
DkCropWidget::DkCropWidget(QWidget* parent /* = 0*/, Qt::WindowFlags f /* = 0*/) : DkFadeWidget(parent, f) {

	mPen = Qt::NoPen;
	mBrush = QColor(0, 0, 0, 200);
}

void DkCropWidget::mouseDoubleClickEvent(QMouseEvent* ev) {

    crop();
    QWidget::mouseDoubleClickEvent(ev);
}

void DkCropWidget::paintEvent(QPaintEvent* pe) {

	// create path
	QPainterPath path;
	QRect canvas = geometry();
	path.addRect(canvas);

	QRectF crop = mCropArea.cropViewRect();
	path.addRect(crop);

	// now draw
	QPainter painter(this);

	painter.setPen(mPen);
	painter.setBrush(mBrush);
	painter.drawPath(path);

	// draw corners
	int r = 4;
	painter.setBrush(QColor(255, 255, 255));
	painter.drawEllipse(crop.topLeft(), r, r);
	painter.drawEllipse(crop.topRight(), r, r);
	painter.drawEllipse(crop.bottomLeft(), r, r);
	painter.drawEllipse(crop.bottomRight(), r, r);

	//drawGuide(&painter, p, mPaintMode);

	//// debug
	//painter.drawPoint(mRect.getCenter());

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
	
	Q_ASSERT(mImgViewRect != nullptr);
	Q_ASSERT(mImgMatrix != nullptr);
	Q_ASSERT(mWorldMatrix != nullptr);

	// TODO: use updated rect here...
	QRectF rect = *mImgViewRect;
	
	//rect = mImgMatrix->mapRect(rect);
	rect = mWorldMatrix->mapRect(rect);
	
	return rect;
}

}