/*******************************************************************************************************
 DkProcess.cpp
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

#include "DkProcess.h"

namespace nmc {

// DkAbstractBatch --------------------------------------------------------------------

/**
 * Generic compute method.
 * This method allows for a simplified interface if a derived class
 * just needs to process the image itself (not meta data).
 * @param container the image container to be processed
 * @param logStrings log strings
 * @return bool true on success
 **/ 
bool DkAbstractBatch::compute(QSharedPointer<DkImageContainer> container, QStringList& logStrings) const {
	
	QImage img = container->image();
	
	bool isOk = compute(img, logStrings);

	if (isOk)
		container->setImage(img);

	return isOk;
}

// DkResizeBatch --------------------------------------------------------------------
DkResizeBatch::DkResizeBatch() {
	scaleFactor = 1.0f;
	iplMethod = DkImage::ipl_area;
	correctGamma = false;
}

QString DkResizeBatch::name() const {

	return QObject::tr("[Resize Batch]");
}

void DkResizeBatch::setProperties(float scaleFactor, int iplMethod, bool correctGamma) {
	this->scaleFactor = scaleFactor;
	this->iplMethod = iplMethod;
	this->correctGamma = correctGamma;
}

bool DkResizeBatch::compute(QImage& img, QStringList& logStrings) const {

	if (scaleFactor == 1.0f) {
		logStrings.append(QObject::tr("%1 scale factor is 1 -> ignoring").arg(name()));
		return true;
	}

	QImage tmpImg = DkImage::resizeImage(img, QSize(), scaleFactor, iplMethod, false);

	if (tmpImg.isNull()) {
		logStrings.append(QObject::tr("%1 could not resize image.").arg(name()));
		return false;
	}

	logStrings.append(QObject::tr("%1 image resized, scale factor: %2").arg(name()).arg(scaleFactor));
	img = tmpImg;

	return true;
}

// DkBatchProcess --------------------------------------------------------------------
DkBatchProcess::DkBatchProcess(const QFileInfo& fileInfoIn, const QFileInfo& fileInfoOut) {
	this->fileInfoIn = fileInfoIn;
	this->fileInfoOut = fileInfoOut;
	compression = -1;

	mode = DkBatchConfig::mode_skip_existing;
}

void DkBatchProcess::setProcessChain(const QVector<QSharedPointer<DkAbstractBatch> > processes) {

	this->processFunctions = processes;
}

void DkBatchProcess::setMode(int mode) {

	this->mode = mode;
}

void DkBatchProcess::compute() {

	// check errors
	if (fileInfoOut.exists() && mode == DkBatchConfig::mode_skip_existing) {
		logStrings.append(QObject::tr("Skipping because file already exists."));
		return;
	}
	else if (!fileInfoIn.exists()) {
		logStrings.append(QObject::tr("Error: input file does not exist"));
		logStrings.append(QObject::tr("Input: %1").arg(fileInfoIn.absoluteFilePath()));
		return;
	}
	else if (fileInfoIn == fileInfoOut && processFunctions.empty()) {
		logStrings.append(QObject::tr("Skipping: nothing to do here."));
		return;
	}

	// do the work
	if (processFunctions.empty() && fileInfoIn.absolutePath() == fileInfoOut.absolutePath()) {	// rename?
		renameFile();
		return;
	}
	else if (processFunctions.empty() && fileInfoIn.suffix() == fileInfoOut.suffix())	{	// copy?
		copyFile();
		return;
	}

	process();
}

QStringList DkBatchProcess::getLog() const {

	return logStrings;
}

bool DkBatchProcess::process() {

	logStrings.append(QObject::tr("processing %1").arg(fileInfoIn.absoluteFilePath()));

	QSharedPointer<DkImageContainer> imgC(new DkImageContainer(fileInfoIn));

	if (!imgC->loadImage() || imgC->image().isNull()) {
		logStrings.append(QObject::tr("Error while loading..."));
		return false;
	}

	for (QSharedPointer<DkAbstractBatch> batch : processFunctions) {

		if (!batch) {
			logStrings.append(QObject::tr("Error: cannot process a NULL function."));
			continue;
		}

		if (!batch->compute(imgC, logStrings)) {
			logStrings.append(QObject::tr("Warning: %1 failed").arg(batch->name()));
		}
	}

	imgC->saveImage(fileInfoOut, compression);
	logStrings.append(QObject::tr("%1 saved...").arg(fileInfoOut.absoluteFilePath()));

	return true;
}

bool DkBatchProcess::renameFile() {

	if (fileInfoOut.exists()) {
		logStrings.append(QObject::tr("Error: could not rename file, the target file exists already."));
		return false;
	}

	QFile file(fileInfoIn.absoluteFilePath());

	if (!file.rename(fileInfoOut.absoluteFilePath())) {
		logStrings.append(QObject::tr("Error: could not rename file"));
		logStrings.append(file.errorString());
		return false;
	}
	else
		logStrings.append(QObject::tr("Renaming: %1 -> %2").arg(fileInfoIn.absoluteFilePath()).arg(fileInfoOut.absoluteFilePath()));

	return true;
}

bool DkBatchProcess::copyFile() {

	QFile file(fileInfoIn.absoluteFilePath());

	if (fileInfoOut.exists() && mode == DkBatchConfig::mode_overwrite) {
		if (!deleteExisting())
			return false;	// early break
	}

	if (!file.copy(fileInfoOut.absoluteFilePath())) {
		logStrings.append(QObject::tr("Error: could not copy file"));
		logStrings.append(QObject::tr("Input: %1").arg(fileInfoIn.absoluteFilePath()));
		logStrings.append(QObject::tr("Output: %1").arg(fileInfoOut.absoluteFilePath()));
		logStrings.append(file.errorString());
		return false;
	}
	else
		logStrings.append(QObject::tr("Copying: %1 -> %2").arg(fileInfoIn.absoluteFilePath()).arg(fileInfoOut.absoluteFilePath()));

	return true;
}

bool DkBatchProcess::deleteExisting() {

	if (fileInfoOut.exists() && mode == DkBatchConfig::mode_overwrite) {
		QFile file(fileInfoIn.absoluteFilePath());

		if (!file.remove()) {
			logStrings.append(QObject::tr("Error: could not delete existing file"));
			logStrings.append(file.errorString());
			return false;
		}
	}

	return true;
}

// DkBatchConfig --------------------------------------------------------------------
DkBatchConfig::DkBatchConfig(const QList<QUrl>& urls, const QDir& outputDir, const QString& fileNamePattern) {

	this->urls = urls;
	this->outputDir = outputDir;
	this->fileNamePattern = fileNamePattern;
};

bool DkBatchConfig::isOk() const {

	if (!outputDir.exists()) {
		if (!outputDir.mkpath("."))
			return false;	// output dir does not exist & i cannot create it
	}

	if (outputDir == QDir())
		return false; // do not allow to write into my (exe) directory

	if (urls.empty())
		return false;

	if (fileNamePattern.isEmpty())
		return false;

	return true;

}

// DkBatchProcessing --------------------------------------------------------------------
DkBatchProcessing::DkBatchProcessing(const DkBatchConfig& config, QWidget* parent /*= 0*/) : QObject(parent) {

	this->batchConfig = config;

	connect(&batchWatcher, SIGNAL(progressValueChanged(int)), this, SIGNAL(progressValueChanged(int)));
	connect(&batchWatcher, SIGNAL(finished()), this, SIGNAL(finished()));
}

void DkBatchProcessing::init() {

	batchItems.clear();
	
	QList<QUrl>& urls = batchConfig.getUrls();

	for (int idx = 0; idx < urls.size(); idx++) {

		QFileInfo cFileInfo(urls.at(idx).toLocalFile());

		DkFileNameConverter converter(cFileInfo.fileName(), batchConfig.getFileNamePattern(), idx+batchConfig.getStartIdx());
		QFileInfo newFileInfo(batchConfig.getOutputDir(), converter.getConvertedFileName());

		DkBatchProcess cProcess(cFileInfo, newFileInfo);
		cProcess.setMode(batchConfig.getMode());
		cProcess.setProcessChain(batchConfig.getProcessFunctions());

		batchItems.push_back(cProcess);
	}
}

void DkBatchProcessing::compute() {

	init();

	qDebug() << "computing...";

	QFuture<void> future = QtConcurrent::map(batchItems, &nmc::DkBatchProcessing::computeItem);
	batchWatcher.setFuture(future);
}

void DkBatchProcessing::computeItem(DkBatchProcess& item) {

	item.compute();
	qDebug() << "" << item.getLog();
}

QStringList DkBatchProcessing::getLog() const {

	QStringList log;

	for (DkBatchProcess batch : batchItems) {

		log << batch.getLog();
		log << "";	// add empty line between images
	}

	return log;
}

bool DkBatchProcessing::isComputing() const {

	return batchWatcher.isRunning();
}

void DkBatchProcessing::cancel() {

	batchWatcher.cancel();
}

}