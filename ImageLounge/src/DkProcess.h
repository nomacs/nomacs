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

// Qt defines
class QImage;

namespace nmc {

// nomacs defines
class DkImageContainer;

class DkAbstractBatch {

public:
	DkAbstractBatch() {};

	virtual void setProperties(...) {};
	virtual bool compute(QSharedPointer<DkImageContainer> container, QStringList& logStrings) const;
	virtual bool compute(QImage&, QStringList&) const { return true; };
	virtual QString name() const {return "Abstract Batch";};
	virtual bool isActive() const { return false; };

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

class DkResizeBatch : public DkAbstractBatch {

public:
	DkResizeBatch();

	// TODO: where shall we put the defines now? e.g. DkImage::ipl_area
	virtual void setProperties(float scaleFactor, int mode = mode_default, int prop = prop_default, int iplMethod = 1/*DkImage::ipl_area*/, bool correctGamma = false);
	virtual bool compute(QImage& img, QStringList& logStrings) const;
	virtual QString name() const;
	virtual bool isActive() const;

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

	int mode;
	int property;
	float scaleFactor;
	int iplMethod;
	bool correctGamma;
};

class DkBatchTransform : public DkAbstractBatch {

public:
	DkBatchTransform();

	virtual void setProperties(int angle, bool horizontalFlip = false, bool verticalFlip = false);
	virtual bool compute(QImage& img, QStringList& logStrings) const;
	virtual QString name() const;
	virtual bool isActive() const;

protected:

	int angle;
	bool horizontalFlip;
	bool verticalFlip;
};

class DkBatchProcess {

public:
	DkBatchProcess(const QFileInfo& fileInfoIn = QFileInfo(), const QFileInfo& fileInfoOut = QFileInfo());

	void setProcessChain(const QVector<QSharedPointer<DkAbstractBatch> > processes);
	void setMode(int mode);
	void setDeleteOriginal(bool deleteOriginal);
	bool compute();	// do the work
	QStringList getLog() const;
	bool hasFailed() const;
	bool wasProcessed() const;
	QFileInfo inputFile() const;
	QFileInfo outputFile() const;

protected:
	QFileInfo fileInfoIn;
	QFileInfo fileInfoOut;
	int mode;
	bool deleteOriginal;
	int compression;
	int failure;
	bool isProcessed;

	QVector<QSharedPointer<DkAbstractBatch> > processFunctions;
	QStringList logStrings;

	bool process();
	bool deleteExisting();
	bool deleteOriginalFile();
	bool copyFile();
	bool renameFile();
};

class DkBatchConfig {

public:
	DkBatchConfig() { init(); };
	DkBatchConfig(const QStringList& fileList, const QString& outputDir, const QString& fileNamePattern);

	bool isOk() const;

	void setFileList(const QStringList& fileList) { this->fileList = fileList; };
	void setOutputDir(const QString& outputDir) {this->outputDirPath = outputDir; };
	void setFileNamePattern(const QString& pattern) {this->fileNamePattern = pattern; };
	void setProcessFunctions(const QVector<QSharedPointer<DkAbstractBatch> >& processFunctions) { this->processFunctions = processFunctions; };
	void setCompression(int compression) { this->compression = compression; };
	void setMode(int mode) { this->mode = mode; };
	void setDeleteOriginal(bool deleteOriginal) { this->deleteOriginal = deleteOriginal; };

	QStringList getFileList() const { return fileList; };
	QString getOutputDirPath() const { return outputDirPath; };
	QString getFileNamePattern() const { return fileNamePattern; };
	QVector<QSharedPointer<DkAbstractBatch> > getProcessFunctions() const { return processFunctions; };
	int getCompression() const { return compression; };
	int getMode() const { return mode; };
	bool getDeleteOriginal() const { return deleteOriginal; };

	enum {
		mode_overwrite,
		mode_skip_existing,

		mode_end
	};

protected:
	void init();

	QStringList fileList;
	QString outputDirPath;
	QString fileNamePattern;
	int compression;
	int mode;
	bool deleteOriginal;
	
	QVector<QSharedPointer<DkAbstractBatch> > processFunctions;
};

class DkBatchProcessing : public QObject {
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

	// getter, setter
	void setBatchConfig(const DkBatchConfig& config) { this->batchConfig = config; };
	DkBatchConfig getBatchConfig() const { return batchConfig; };

public slots:
	// user interaction
	void cancel();

signals:
	void progressValueChanged(int idx);
	void finished();

protected:
	DkBatchConfig batchConfig;
	QVector<DkBatchProcess> batchItems;
	QList<int> resList;
	
	// threading
	QFutureWatcher<void> batchWatcher;
	
	void init();
};

}