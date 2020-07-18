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

#pragma warning(push, 0)	// no warnings from includes
// Qt Includes
#pragma warning(pop)

namespace nmc {

// DkCropWidget --------------------------------------------------------------------
DkCropWidget::DkCropWidget(QWidget* parent /* = 0*/, Qt::WindowFlags f /* = 0*/) : DkFadeWidget(parent, f) {

}

void DkCropWidget::mouseDoubleClickEvent(QMouseEvent* ev) {

    crop();
    QWidget::mouseDoubleClickEvent(ev);
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

    QWidget::setVisible(visible);
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

}