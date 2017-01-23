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

#include "DkManipulators.h"
#include "DkManipulatorsIpl.h"

#include "DkImageStorage.h"
#include "DkImageContainer.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QSharedPointer>
#include <QWidget>
#pragma warning(pop)

namespace nmc {

// DkBaseManipulator --------------------------------------------------------------------
DkBaseManipulator::DkBaseManipulator(QAction * action) {
	
	mAction = action;
	
	// add default icon
	if (mAction->icon().isNull())
		mAction->setIcon(DkImage::loadIcon(":/nomacs/img/manipulation.svg"));
}

QString DkBaseManipulator::name() const {
	QString text = mAction->iconText();
	return text.replace("&", "");
}

QAction* DkBaseManipulator::action() const {
	return mAction;
}

QIcon DkBaseManipulator::icon() const {
	return mAction->icon();
}

// DkManipulatorManager --------------------------------------------------------------------
DkManipulatorManager::DkManipulatorManager() {
}

void DkManipulatorManager::createManipulators(QWidget* parent) {

	mManipulators.resize(m_ext_end);

	// grayscale
	QAction* action;
	action = new QAction(QObject::tr("&Grayscale"), parent);
	action->setStatusTip(QObject::tr("Convert to Grayscale"));
	mManipulators[m_grayscale] = QSharedPointer<DkGrayScaleManipulator>::create(action);

	// auto adjust
	action = new QAction(QObject::tr("&Auto Adjust"), parent);
	action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_L);
	action->setStatusTip(QObject::tr("Auto Adjust Image Contrast and Color Balance"));
	mManipulators[m_auto_adjust] = QSharedPointer<DkAutoAdjustManipulator>::create(action);

	// normalize
	action = new QAction(QObject::tr("Nor&malize Image"), parent);
	action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_N);
	action->setStatusTip(QObject::tr("Normalize the Image"));
	mManipulators[m_normalize] = QSharedPointer<DkNormalizeManipulator>::create(action);

	// flip horizontal
	action = new QAction(QObject::tr("Flip &Horizontal"), parent);
	action->setStatusTip(QObject::tr("Flip Image Horizontally"));
	mManipulators[m_flip_h] = QSharedPointer<DkFlipHManipulator>::create(action);

	// flip vertical
	action = new QAction(QObject::tr("Flip &Vertical"), parent);
	action->setStatusTip(QObject::tr("Flip Image Vertically"));
	mManipulators[m_flip_v] = QSharedPointer<DkFlipVManipulator>::create(action);

	// invert image
	action = new QAction(QObject::tr("&Invert Image"), parent);
	action->setStatusTip(QObject::tr("Invert the Image"));
	mManipulators[m_invert] = QSharedPointer<DkInvertManipulator>::create(action);

	// extended --------------------------------------------------------------------
	// tiny planet
	action = new QAction(QObject::tr("&Tiny Planet"), parent);
	action->setStatusTip(QObject::tr("Create a Tiny Planet"));
	mManipulators[m_tiny_planet] = QSharedPointer<DkTinyPlanetManipulator>::create(action);

	// unsharp mask
	action = new QAction(QObject::tr("&Sharpen"), parent);
	action->setStatusTip(QObject::tr("Sharpens the image by applying an unsharp mask"));
	mManipulators[m_unsharp_mask] = QSharedPointer<DkUnsharpMaskManipulator>::create(action);

	// rotate
	action = new QAction(QObject::tr("&Rotate"), parent);
	action->setStatusTip(QObject::tr("Rotate the image"));
	mManipulators[m_rotate] = QSharedPointer<DkRotateManipulator>::create(action);

	// hue/saturation
	action = new QAction(QObject::tr("&Hue/Saturation"), parent);
	action->setStatusTip(QObject::tr("Change Hue and Saturation"));
	mManipulators[m_hue] = QSharedPointer<DkHueManipulator>::create(action);

	// exposure
	action = new QAction(QObject::tr("&Exposure"), parent);
	action->setStatusTip(QObject::tr("Change the Exposure and Gamma"));
	mManipulators[m_exposure] = QSharedPointer<DkExposureManipulator>::create(action);

}

QVector<QAction*> DkManipulatorManager::actions() const {
	
	QVector<QAction*> aVec;

	for (auto m : mManipulators)
		aVec << m->action();
	
	return aVec;
}

QSharedPointer<DkBaseManipulatorExt> DkManipulatorManager::manipulatorExt(const ManipulatorExtId & mId) const {
	return qSharedPointerCast<DkBaseManipulatorExt>(mManipulators[mId]);
}

QSharedPointer<DkBaseManipulator> DkManipulatorManager::manipulator(const ManipulatorId & mId) const {
	return mManipulators[mId];
}

QSharedPointer<DkBaseManipulator> DkManipulatorManager::manipulator(const QAction * action) const {

	for (const QSharedPointer<DkBaseManipulator>& m : mManipulators) {
		if (m->action() == action)
			return m;
	}

	if (action)
		qWarning() << "no manipulator matches" << action;

	return QSharedPointer<DkBaseManipulator>();
}

QString DkBaseManipulator::errorMessage() const {
	return "";
}

// DkBaseMainpulatorExt --------------------------------------------------------------------
DkBaseManipulatorExt::DkBaseManipulatorExt(QAction * action) : DkBaseManipulator(action) {
}

void DkBaseManipulatorExt::setWidget(QWidget * widget) {
	mWidget = widget;
}

QWidget * DkBaseManipulatorExt::widget() const {
	return mWidget;
}

void DkBaseManipulatorExt::setDirty(bool dirty) {
	mDirty = dirty;
}

bool DkBaseManipulatorExt::isDirty() const {
	return mDirty;
}

}