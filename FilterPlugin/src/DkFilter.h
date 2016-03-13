/*******************************************************************************************************
 DkFilter.h

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>

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

#include "DkPluginInterface.h"
#include "DkImageStorage.h"
// OpenCV
#ifdef WITH_OPENCV

#ifdef Q_WS_WIN
	#pragma warning(disable: 4996)
#endif

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <algorithm> // for Min in Sepia Filter

#endif

namespace nmc {

class DkFilter : public QObject, DkPluginInterface {
	Q_OBJECT
	Q_INTERFACES(nmc::DkPluginInterface)
	Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.DkFilter/3.0" FILE "DkFilter.json")

public:

	DkFilter(QObject* parent = 0);
	~DkFilter();

	QString id() const override;
	QImage image() const override;
	QString version() const override;

	QList<QAction*> createActions(QWidget* parent) override;
	QList<QAction*> pluginActions() const override;
	QSharedPointer<nmc::DkImageContainer> runPlugin(const QString &runID = QString(), QSharedPointer<nmc::DkImageContainer> imgC = QSharedPointer<nmc::DkImageContainer>()) const override;
	static QImage applyGothamFilter(QImage inImg);
	static QImage applyLomoFilter(QImage inImg);
	static QImage applyKelvinFilter(QImage inImg);
	static QImage applySepiaFilter(QImage inImg);
	enum {
		ID_ACTION1,
		ID_ACTION2,
		ID_ACTION3,
		ID_ACTION4,
		// add actions here

		id_end
	};

protected:
	QList<QAction*> mActions;
	QStringList mRunIDs;
	QStringList mMenuNames;
	QStringList mMenuStatusTips;
};

};
