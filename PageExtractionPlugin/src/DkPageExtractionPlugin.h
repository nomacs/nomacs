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

namespace nmp {

class DkPageExtractionPlugin : public QObject, nmc::DkBatchPluginInterface {
	Q_OBJECT
	Q_INTERFACES(nmc::DkBatchPluginInterface)
	Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.DkPageExtractionPlugin/3.2" FILE "DkPageExtractionPlugin.json")

public:

	DkPageExtractionPlugin(QObject* parent = 0);
	~DkPageExtractionPlugin();

	QImage image() const;
	QString version() const;
	QString name() const;

	QList<QAction*> createActions(QWidget* parent) override;
	QList<QAction*> pluginActions() const override;
	QSharedPointer<nmc::DkImageContainer> runPlugin(
		const QString &runID, 
		QSharedPointer<nmc::DkImageContainer> image, 
		const nmc::DkSaveInfo& saveInfo,
		QSharedPointer<nmc::DkBatchInfo>& batchInfo) const override;

	virtual void preLoadPlugin() const {};	// is called before batch processing
	virtual void postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo> > &) const {};	// is called after batch processing

	enum {
		id_crop_to_page,
		id_crop_to_metadata,
		id_draw_to_page,
		//id_eval_page,
		// add actions here

		id_end
	};

	enum MethodIndex {
		m_thresholds = 0,
		m_bhaskar,

		m_end
	};

	void loadSettings(QSettings& settings) override;
	void saveSettings(QSettings& settings) const override;

protected:
	QList<QAction*> mActions;
	QStringList mRunIDs;
	QStringList mMenuNames;
	QStringList mMenuStatusTips;
	QString mResultPath;

	MethodIndex mMethod = m_thresholds;

	QPolygonF readGT(const QString& imgPath) const;
	double jaccardIndex(const QSize& imgSize, const QPolygonF& gt, const QPolygonF& computed) const;
	QImage drawPoly(const QSize& imgSize, const QPolygonF& poly) const;
};

};
