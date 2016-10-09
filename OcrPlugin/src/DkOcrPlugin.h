/*******************************************************************************************************
 PLUGIN_CLASS_NAME.h

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
#include "DkOcrToolbar.h"

#include <QDockWidget>
#include <QtWidgets>

namespace nmc {

class DkOcrPlugin : public QObject, nmc::DkPluginInterface {

private:
	Q_OBJECT
	Q_INTERFACES(nmc::DkPluginInterface)
	Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.DkOcrPlugin/0.1" FILE "DkOcrPlugin.json")

public:

	DkOcrPlugin(QObject* parent = 0);
	~DkOcrPlugin();

	QImage image() const override;
	//bool hideHUD() const override;

	QString id() const override;
	QString pluginName() const;// override;
	QString pluginDescription() const;// override;
	QImage pluginDescriptionImage() const;// override;
	QString pluginVersion() const;// override;

	QStringList runID() const;// override;
	QString pluginMenuName(const QString &runID = QString()) const;// override;
	QString pluginStatusTip(const QString &runID = QString()) const;// override;
	QList<QAction*> pluginActions() const override;
	QList<QAction*> createActions(QWidget*) override;

	// DIEM: I think this should solve the sub-menu: 	
	// getMainWindow() call if you need a (or the) parent
	// virtual QList<QAction*> createActions(QWidget*) { return QList<QAction*>();};
	// virtual QList<QAction*> pluginActions()	const { return QList<QAction*>();};
	
	//QImage runPlugin(const QString &runID = QString(), const QImage &image = QImage()) const;
	QSharedPointer<DkImageContainer> runPlugin(const QString &runID = QString(), QSharedPointer<DkImageContainer> imgC = QSharedPointer<DkImageContainer>()) const override;

	enum {
		ACTION_TESTRUN,

		// add actions here

		id_end
	};

protected:

	QList<QAction*> mActions;
	QStringList mRunIDs;
	QStringList mMenuNames;
	QStringList mMenuStatusTips;

	QDockWidget* mDockWidgetSettings;
	QTextEdit* te_resultText;

	QString GetRandomString() const;
};

};
