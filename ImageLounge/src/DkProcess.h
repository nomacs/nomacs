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

#pragma once;

#include <QUrl>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentMap>

#include "DkImageContainer.h"

namespace nmc {

class DkAbstractBatch {

public:
	DkAbstractBatch() {};

	virtual void setProperties(...) {};
	virtual bool compute(QSharedPointer<DkImageContainer> container, QStringList& logStrings) const;
	virtual bool compute(QImage& img, QStringList& logStrings) const {return true;};
	virtual QString name() const {return "Abstract Batch";};

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

	virtual void setProperties(float scaleFactor, int iplMethod = DkImage::ipl_area, bool correctGamma = false);
	virtual bool compute(QImage& img, QStringList& logStrings) const;
	virtual QString name() const;

protected:
	float scaleFactor;
	int iplMethod;
	bool correctGamma;

};

class DkBatchProcess {

public:

	DkBatchProcess(const QFileInfo& fileInfoIn = QFileInfo(), const QFileInfo& fileInfoOut = QFileInfo());

	void setProcessChain(const QVector<QSharedPointer<DkAbstractBatch> > processes);
	void setMode(int mode);
	void compute();	// do the work
	QStringList getLog() const;

protected:
	QFileInfo fileInfoIn;
	QFileInfo fileInfoOut;
	int mode;
	int compression;

	QVector<QSharedPointer<DkAbstractBatch> > processFunctions;
	QStringList logStrings;

	bool process();
	bool deleteExisting();
	bool copyFile();
	bool renameFile();
};

class DkBatchConfig {

public:
	DkBatchConfig() { init(); };
	DkBatchConfig(const QList<QUrl>& urls, const QDir& outputDir, const QString& fileNamePattern);

	bool isOk() const;

	void setUrls(const QList<QUrl>& urls) { this->urls = urls; };
	void setOutputDir(const QDir& outputDir) {this->outputDir = outputDir; };
	void setFileNamePattern(const QString& pattern) {this->fileNamePattern = pattern; };
	void setStartIdx(int startIdx) { this->startIdx = startIdx; };
	void setProcessFunctions(const QVector<QSharedPointer<DkAbstractBatch> >& processFunctions) { this->processFunctions = processFunctions; };
	void setCompression(int compression) { this->compression = compression; };
	void setMode(int mode) { this->mode = mode; };

	QList<QUrl> getUrls() const { return urls; };
	QDir getOutputDir() const { return outputDir; };
	QString getFileNamePattern() const { return fileNamePattern; };
	int getStartIdx() const { return startIdx; };
	QVector<QSharedPointer<DkAbstractBatch> > getProcessFunctions() const { return processFunctions; };
	int getCompression() const { return compression; };
	int getMode() const { return mode; };

	enum {
		mode_overwrite,
		mode_skip_existing,

		mode_end
	};

protected:
	void init();

	QList<QUrl> urls;
	QDir outputDir;
	QString fileNamePattern;
	int startIdx;
	int compression;
	int mode;
	QVector<QSharedPointer<DkAbstractBatch> > processFunctions;
};

class DkBatchProcessing : public QObject {
	Q_OBJECT

public:
	DkBatchProcessing(const DkBatchConfig& config = DkBatchConfig(), QWidget* parent = 0);

	void compute();
	static void computeItem(DkBatchProcess& item);
	
	QStringList getLog() const;	// TODO
	
	bool isComputing() const;

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
	
	// threading
	QFutureWatcher<void> batchWatcher;
	
	void init();
};

}