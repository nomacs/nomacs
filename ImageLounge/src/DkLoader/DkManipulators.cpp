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

	mManipulators.resize(m_end);

	// grayscale
	QAction* action;
	action = new QAction(QObject::tr("&Grayscale"), parent);
	action->setStatusTip(QObject::tr("Convert to Grayscale"));
	mManipulators[m_grayscale] = QSharedPointer<DkGrayScaleManipulator>::create(action);
}

QVector<QAction*> DkManipulatorManager::actions() const {
	
	QVector<QAction*> aVec;

	for (auto m : mManipulators)
		aVec << m->action();
	
	return aVec;
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

}