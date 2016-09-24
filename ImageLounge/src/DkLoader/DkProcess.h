/*******************************************************************************************************
 DkProcess.h
 Created on:	27.12.2014
 
 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
 Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
 Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
 Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QSharedPointer>
#include <QFutureWatcher>
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QUrl>
#pragma warning(pop)		// no warnings from includes - end

#include "DkBatchInfo.h"

#pragma warning(disable: 4251)	// TODO: remove

#ifndef DllLoaderExport
#ifdef DK_LOADER_DLL_EXPORT
#define DllLoaderExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllLoaderExport Q_DECL_IMPORT
#else
#define DllLoaderExport Q_DECL_IMPORT
#endif
#endif

// Qt defines
class QImage;
class QSettings;

namespace nmc {

// nomacs defines
class DkImageContainer;
class DkPluginContainer;

class DllLoaderExport DkAbstractBatch {

public:
	DkAbstractBatch() {};

	virtual void setProperties(...) {};
	virtual void saveSettings(QSettings&) const {};
	virtual void loadSettings(QSettings&) {};
	virtual bool compute(
		QSharedPointer<DkImageContainer> container, 
		const DkSaveInfo& saveInfo, 
		QStringList& logStrings, 
		QVector<QSharedPointer<DkBatchInfo> >& batchInfos) const;
	virtual bool compute(QSharedPointer<DkImageContainer> container, QStringList& logStrings) const;
	virtual bool compute(QImage&, QStringList&) const { return true; };
	virtual bool isActive() const { return false; };
	virtual void postLoad(const QVector<QSharedPointer<DkBatchInfo> >&) const {};

	virtual QString name() const {return "Abstract Batch";};
	QString settingsName() const;

	static QSharedPointer<DkAbstractBatch> createFromName(const QString& settingsName);

private:
	// ok, this is important:
	// we are using the abstract class to process specialized items
	// if we allow copy operations - we get slicing issues
	// so we just allow pointers to the batch processing functions.
	// but pointers result in threading issues - so we just use
	// QSharedPointers -> problem solved : )
	DkAbstractBatch(const DkAbstractBatch& o);
	DkAbstractBatch& operator=( const DkAbstractBatch& o);

};

class DllLoaderExport DkResizeBatch : public DkAbstractBatch {

public:
	DkResizeBatch();

	// TODO: where shall we put the defines now? e.g. DkImage::ipl_area
	virtual void setProperties(float scaleFactor, int mode = mode_default, int prop = prop_default, int iplMethod = 1/*DkImage::ipl_area*/, bool correctGamma = false);
	virtual void saveSettings(QSettings& settings) const override;
	virtual void loadSettings(QSettings& settings) override;
	virtual bool compute(QImage& img, QStringList& logStrings) const override;
	virtual QString name() const override;
	virtual bool isActive() const override;

	int mode() const;
	int property() const;
	int iplMethod() const;
	float scaleFactor() const;
	bool correctGamma() const;

	enum {
		mode_default,
		mode_long_side,
		mode_short_side,
		mode_width,
		mode_height,

		mode_end
	};

	enum {
		prop_default,
		prop_decrease_only,
		prop_increase_only,

		prop_end
	};

protected:
	bool prepareProperties(const QSize& imgSize, QSize& size, float& scaleFactor, QStringList& logStrings) const;

	int mMode = mode_default;
	int mProperty = prop_default;
	float mScaleFactor = 1.0f;
	int mIplMethod = 0;
	bool mCorrectGamma = false;
};

#ifdef WITH_PLUGINS
class DllLoaderExport DkPluginBatch : public DkAbstractBatch {

public:
	DkPluginBatch();

	virtual void saveSettings(QSettings& settings) const override;
	virtual void loadSettings(QSettings& settings) override;

	virtual void preLoad();
	virtual void postLoad(const QVector<QSharedPointer<DkBatchInfo> >& batchInfo) const override;
	virtual void setProperties(const QStringList& pluginList);
	virtual bool compute(
		QSharedPointer<DkImageContainer> container, 
		const DkSaveInfo& saveInfo, 
		QStringList& logStrings, 
		QVector<QSharedPointer<DkBatchInfo> >& batchInfos) const override;
	virtual QString name() const override;
	virtual bool isActive() const override;
	virtual QStringList pluginList() const;


protected:
	void loadAllPlugins();
	void loadPlugin(const QString& pluginString, QSharedPointer<DkPluginContainer>& plugin, QString& runID) const;

	QVector<QSharedPointer<DkPluginContainer> > mPlugins;
	QStringList mRunIDs;
	QStringList mPluginList;
};
#endif

class DllLoaderExport DkBatchTransform : public DkAbstractBatch {

public:
	DkBatchTransform();

	virtual void saveSettings(QSettings& settings) const override;
	virtual void loadSettings(QSettings& settings) override;

	virtual void setProperties(int angle, bool horizontalFlip = false, bool verticalFlip = false, bool cropFromMetadata = false);
	virtual bool compute(QSharedPointer<DkImageContainer> container, QStringList& logStrings) const;
	virtual QString name() const;
	virtual bool isActive() const;

	int angle() const;
	bool horizontalFlip() const;
	bool verticalFlip() const;
	bool cropMetatdata() const;

protected:

	int mAngle = 0;
	bool mHorizontalFlip = false;
	bool mVerticalFlip = false;
	bool mCropFromMetadata = false;
};

class DllLoaderExport DkBatchProcess {

public:
	DkBatchProcess(const DkSaveInfo& saveInfo = DkSaveInfo());

	void setProcessChain(const QVector<QSharedPointer<DkAbstractBatch> > processes);
	bool compute();	// do the work
	QStringList getLog() const;
	bool hasFailed() const;
	bool wasProcessed() const;
	QString inputFile() const;
	QString outputFile() const;

	QVector<QSharedPointer<DkBatchInfo> > batchInfo() const;

protected:
	bool process();
	bool prepareDeleteExisting();
	bool deleteOrRestoreExisting();
	bool deleteOriginalFile();
	bool copyFile();
	bool renameFile();

	DkSaveInfo mSaveInfo;
	int mFailure = 0;
	bool mIsProcessed = false;

	QVector<QSharedPointer<DkBatchInfo> > mInfos;
	QVector<QSharedPointer<DkAbstractBatch> > mProcessFunctions;
	QStringList mLogStrings;
};

class DllLoaderExport DkBatchConfig {

public:
	DkBatchConfig() { };
	DkBatchConfig(const QStringList& fileList, const QString& outputDir, const QString& fileNamePattern);

	virtual void saveSettings(QSettings& settings) const;
	virtual void loadSettings(QSettings& settings);

	bool isOk() const;

	void setFileList(const QStringList& fileList) { mFileList = fileList; };
	void setOutputDir(const QString& outputDir) { mOutputDirPath = outputDir; };
	void setFileNamePattern(const QString& pattern) { mFileNamePattern = pattern; };
	void setProcessFunctions(const QVector<QSharedPointer<DkAbstractBatch> >& processFunctions) { mProcessFunctions = processFunctions; };
	void setSaveInfo(const DkSaveInfo& saveInfo) { mSaveInfo = saveInfo; };

	QStringList getFileList() const { return mFileList; };
	QString getOutputDirPath() const { return mOutputDirPath; };
	QString getFileNamePattern() const { return mFileNamePattern; };
	QVector<QSharedPointer<DkAbstractBatch> > getProcessFunctions() const { return mProcessFunctions; };
	DkSaveInfo saveInfo() const { return mSaveInfo; };

protected:
	
	DkSaveInfo mSaveInfo;

	QStringList mFileList;
	QString mOutputDirPath;
	QString mFileNamePattern;
	
	QVector<QSharedPointer<DkAbstractBatch> > mProcessFunctions;
};

class DllLoaderExport DkBatchProcessing : public QObject {
	Q_OBJECT

public:

	enum {
		batch_item_failed,
		batch_item_succeeded,
		batch_item_not_computed,

		batch_item_end
	};

	DkBatchProcessing(const DkBatchConfig& config = DkBatchConfig(), QWidget* parent = 0);

	void compute();
	static bool computeItem(DkBatchProcess& item);
	
	QStringList getLog() const;
	int getNumFailures() const;
	int getNumItems() const;
	int getNumProcessed() const;
	
	bool isComputing() const;
	QList<int> getCurrentResults();
	QStringList getResultList() const;
	QString getBatchSummary(const DkBatchProcess& batch) const;
	void waitForFinished();

	// getter, setter
	void setBatchConfig(const DkBatchConfig& config) { mBatchConfig = config; };
	DkBatchConfig getBatchConfig() const { return mBatchConfig; };

	void postLoad();

public slots:
	// user interaction
	void cancel();

signals:
	void progressValueChanged(int idx);
	void finished();

protected:
	DkBatchConfig mBatchConfig;
	QVector<DkBatchProcess> mBatchItems;
	QList<int> mResList;
	
	// threading
	QFutureWatcher<void> mBatchWatcher;
	
	void init();
};

class DllLoaderExport DkBatchProfile {

public:
	DkBatchProfile(const QString& profileDir = QString());
	
	static DkBatchConfig loadProfile(const QString& profilePath);
	static bool saveProfile(const QString& profilePath, const DkBatchConfig& batchConfig);
	static QString defaultProfilePath();
	static QString profileNameToPath(const QString& profileName);
	static QString makeUserFriendly(const QString& profilePath);
	static QString extension();

	QStringList profileNames();

protected:
	QStringList index(const QString& profileDir) const;

	QString mProfileDir;
	QStringList mProfilePaths;
	static QString ext;
};

}