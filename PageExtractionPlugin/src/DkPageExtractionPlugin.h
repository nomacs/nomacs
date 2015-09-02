/*******************************************************************************************************
 DkPageExtractionPlugin.h

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

namespace nmc {

class DkPageExtractionPlugin : public QObject, DkPluginInterface {
	Q_OBJECT
	Q_INTERFACES(nmc::DkPluginInterface)
	Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.DkPageExtractionPlugin/2.0" FILE "DkPageExtractionPlugin.json")

public:

	DkPageExtractionPlugin(QObject* parent = 0);
	~DkPageExtractionPlugin();

	QString pluginID() const;
	QString pluginName() const;
	QString pluginDescription() const;
	QImage pluginDescriptionImage() const;
	QString pluginVersion() const;

	QStringList runID() const;
	QString pluginMenuName(const QString &runID = QString()) const;
	QString pluginStatusTip(const QString &runID = QString()) const;
	QList<QAction*> createActions(QWidget* parent);
	QList<QAction*> pluginActions() const;
	QSharedPointer<DkImageContainer> runPlugin(const QString &runID = QString(), QSharedPointer<DkImageContainer> image = QSharedPointer<DkImageContainer>()) const override;

	enum {
		id_crop_to_page,
		id_crop_to_metadata,
		id_draw_to_page,
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
