/*******************************************************************************************************
 RulerDetectionPlugin.h

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2017 Pascal Plank

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
#include "RulerSettingsDialog.h"
#include "DkImageStorage.h"		// For image operations (and OpenCV headers)


namespace nmc {

class RulerDetectionPlugin : public QObject, DkBatchPluginInterface {
	Q_OBJECT
	Q_INTERFACES(nmc::DkBatchPluginInterface)
	Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.RulerDetectionPlugin/3.2" FILE "RulerDetectionPlugin.json")

public:
	RulerDetectionPlugin(QObject* parent = 0);
	~RulerDetectionPlugin();

	QImage image() const override;
	QString name() const;

	QList<QAction*> createActions(QWidget* parent) override;
	QList<QAction*> pluginActions() const override;
	QSharedPointer<nmc::DkImageContainer> runPlugin(
		const QString &runID,
		QSharedPointer<nmc::DkImageContainer> image,
		const nmc::DkSaveInfo& saveInfo,
		QSharedPointer<nmc::DkBatchInfo>& batchInfo) const override;


	virtual void preLoadPlugin() const {};	// is called before batch processing
	virtual void postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo> > & batchInfo) const {};	// is called after batch processing

	// The 3 action-ids for this plugin
	enum {
		id_detectionSettings,
		id_detectRuler,
		//id_extractionInfo,
		
		id_end
	};

	// For foading and saving settings
	void loadSettings(QSettings& settings) override;
	void saveSettings(QSettings& settings) const override;

protected:
	QList<QAction*> mActions;
	QStringList mRunIDs;
	QStringList mMenuNames;
	QStringList mMenuStatusTips;

	void handleSettingsDialog() const;
	QSharedPointer<nmc::DkImageContainer> handleRulerDetection(QSharedPointer<nmc::DkImageContainer> imgC, QString templatepath) const;
	void handleDetectionInfo() const;

	// Settings variables (mutable to be accessable by const method to save it via the saveSettings method)
	mutable QString curr_template_path;
	mutable QString curr_tickDistance;
	mutable int curr_referencePoints;
	mutable int curr_hessianThreshold;

	// Ruler Detection Methods
	int calculateDPI(cv::Mat img, cv::Mat ruler_template) const;
	cv::RotatedRect locateTemplate(cv::Mat img, cv::Mat templ) const;
	cv::Mat RulerDetectionPlugin::cropRotatedRectangle(cv::Mat src_image, cv::RotatedRect rect) const;
	int RulerDetectionPlugin::getDPIFromCroppedRuler(cv::Mat img) const;
	cv::Mat RulerDetectionPlugin::swapQuadrants(cv::Mat img) const;
	float RulerDetectionPlugin::getPixelPerTick(cv::Mat img) const;
	int RulerDetectionPlugin::applyMetricToPixelticks(float pixelPerTick, QString tickMetric) const;

	cv::Mat RulerDetectionPlugin::convertFloat32toUint8_1channel(cv::Mat image) const;
};

};
