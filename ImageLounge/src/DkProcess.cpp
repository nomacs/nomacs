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
#include "DkUtils.h"
#include "DkImageContainer.h"
#include "DkImageStorage.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentMap>
#include <QWidget>
#pragma warning(pop)		// no warnings from includes - end

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

	mode = mode_default;
	property = prop_default;
}

QString DkResizeBatch::name() const {

	return QObject::tr("[Resize Batch]");
}

void DkResizeBatch::setProperties(float scaleFactor, int mode, int prop, int iplMethod, bool correctGamma) {

	this->scaleFactor = scaleFactor;
	this->mode = mode;
	this->property = prop;
	this->iplMethod = iplMethod;
	this->correctGamma = correctGamma;
}

bool DkResizeBatch::isActive() const {

	if (mode != mode_default)
		return true;

	if (scaleFactor != 1.0f)
		return true;

	return false;
}

bool DkResizeBatch::compute(QImage& img, QStringList& logStrings) const {

	if (scaleFactor == 1.0f) {
		logStrings.append(QObject::tr("%1 scale factor is 1 -> ignoring").arg(name()));
		return true;
	}

	QSize size;
	float sf = 1.0f;
	QImage tmpImg;

	if (prepareProperties(img.size(), size, sf, logStrings))
		tmpImg = DkImage::resizeImage(img, size, sf, iplMethod, correctGamma);
	else {
		logStrings.append(QObject::tr("%1 no need for resizing.").arg(name()));
		return true;
	}

	if (tmpImg.isNull()) {
		logStrings.append(QObject::tr("%1 could not resize image.").arg(name()));
		return false;
	}

	if (mode == mode_default)
		logStrings.append(QObject::tr("%1 image resized, scale factor: %2%").arg(name()).arg(scaleFactor*100.0f));
	else
		logStrings.append(QObject::tr("%1 image resized, new side: %2 px").arg(name()).arg(scaleFactor));

	img = tmpImg;

	return true;
}

bool DkResizeBatch::prepareProperties(const QSize& imgSize, QSize& size, float& scaleFactor, QStringList& logStrings) const {

	float sf = 1.0f;
	QSize normalizedSize = imgSize; 

	if (mode == mode_default) {
		scaleFactor = this->scaleFactor;
		return true;
	}
	else if (mode == mode_long_side) {
		
		if (imgSize.width() < imgSize.height())
			normalizedSize.transpose();
	}
	else if (mode == mode_short_side) {

		if (imgSize.width() > imgSize.height())
			normalizedSize.transpose();
	}
	else if (mode == mode_height)
		normalizedSize.transpose();

	sf = this->scaleFactor/normalizedSize.width();

	if (sf > 1.0 && this->property == prop_decrease_only) {
		
		logStrings.append(QObject::tr("%1 I need to increase the image, but the option is set to decrease only -> skipping.").arg(name()));
		return false;
	}
	else if (sf < 1.0f && this->property == prop_increase_only) {
		logStrings.append(QObject::tr("%1 I need to decrease the image, but the option is set to increase only -> skipping.").arg(name()));
		return false;
	}
	else if (sf == 1.0f) {
		logStrings.append(QObject::tr("%1 image size matches scale factor -> skipping.").arg(name()));
		return false;
	}

	size.setWidth(qRound(this->scaleFactor));
	size.setHeight(qRound(sf*normalizedSize.height()));

	if (normalizedSize != imgSize)
		size.transpose();

	return true;
}

// DkTransformBatch --------------------------------------------------------------------
DkBatchTransform::DkBatchTransform() {
	angle = 0;
	horizontalFlip = false;
	verticalFlip = false;
}

QString DkBatchTransform::name() const {
	return QObject::tr("[Transform Batch]");
}

void DkBatchTransform::setProperties(int angle, bool horizontalFlip /* = false */, bool verticalFlip /* = false */) {
	
	this->angle = angle;
	this->horizontalFlip = horizontalFlip;
	this->verticalFlip = verticalFlip;
}

bool DkBatchTransform::isActive() const {

	return horizontalFlip || verticalFlip || angle != 0;
}

bool DkBatchTransform::compute(QImage& img, QStringList& logStrings) const {

	if (!isActive()) {
		logStrings.append(QObject::tr("%1 inactive -> skipping").arg(name()));
		return true;
	}

	QImage tmpImg;

	if (angle != 0) {
		QTransform rotationMatrix;
		rotationMatrix.rotate((double)angle);
		tmpImg = img.transformed(rotationMatrix);
	}
	else
		tmpImg = img;

	tmpImg = tmpImg.mirrored(horizontalFlip, verticalFlip);

	if (!tmpImg.isNull()) {
		img = tmpImg;
		logStrings.append(QObject::tr("%1 image transformed.").arg(name()));
	}
	else {
		logStrings.append(QObject::tr("%1 error, could not transform image.").arg(name()));
		return false;
	}

	return true;
}

// DkPluginBatch --------------------------------------------------------------------
DkPluginBatch::DkPluginBatch() {
}

void DkPluginBatch::setProperties(const QStringList & pluginList) {

}

bool DkPluginBatch::compute(QSharedPointer<DkImageContainer> container, QStringList & logStrings) const {

	

	return false;
}

QString DkPluginBatch::name() const {
	return QObject::tr("[Plugin Batch]");
}

bool DkPluginBatch::isActive() const {
	
	return !mPluginList.empty();
}

// DkBatchProcess --------------------------------------------------------------------
DkBatchProcess::DkBatchProcess(const QString& filePathIn, const QString& filePathOut) {
	mFilePathIn = filePathIn;
	mFilePathOut = filePathOut;
	compression = -1;
	failure = 0;
	isProcessed = false;

	mode = DkBatchConfig::mode_skip_existing;
}

void DkBatchProcess::setProcessChain(const QVector<QSharedPointer<DkAbstractBatch> > processes) {

	this->processFunctions = processes;
}

void DkBatchProcess::setMode(int mode) {

	this->mode = mode;
}

void DkBatchProcess::setDeleteOriginal(bool deleteOriginal) {

	this->deleteOriginal = deleteOriginal;
}

QString DkBatchProcess::inputFile() const {

	return mFilePathIn;
}

QString DkBatchProcess::outputFile() const {

	return mFilePathOut;
}

bool DkBatchProcess::hasFailed() const {

	return failure != 0;
}

bool DkBatchProcess::wasProcessed() const {
	
	return isProcessed;
}

bool DkBatchProcess::compute() {

	isProcessed = true;

	QFileInfo fInfoIn(mFilePathIn);
	QFileInfo fInfoOut(mFilePathOut);

	// check errors
	if (fInfoOut.exists() && mode == DkBatchConfig::mode_skip_existing) {
		logStrings.append(QObject::tr("%1 already exists -> skipping (check 'overwrite' if you want to overwrite the file)").arg(mFilePathOut));
		failure++;
		return failure == 0;
	}
	else if (!fInfoIn.exists()) {
		logStrings.append(QObject::tr("Error: input file does not exist"));
		logStrings.append(QObject::tr("Input: %1").arg(mFilePathIn));
		failure++;
		return failure == 0;
	}
	else if (mFilePathIn == mFilePathOut && processFunctions.empty()) {
		logStrings.append(QObject::tr("Skipping: nothing to do here."));
		failure++;
		return failure == 0;
	}
	
	// do the work
	if (processFunctions.empty() && mFilePathIn == mFilePathOut && fInfoIn.suffix() == fInfoOut.suffix()) {	// rename?
		if (!renameFile())
			failure++;
		return failure == 0;
	}
	else if (processFunctions.empty() && fInfoIn.suffix() == fInfoOut.suffix()) {	// copy?
		if (!copyFile())
			failure++;
		else
			deleteOriginalFile();

		return failure == 0;
	}

	process();

	return failure == 0;
}

QStringList DkBatchProcess::getLog() const {

	return logStrings;
}

bool DkBatchProcess::process() {

	logStrings.append(QObject::tr("processing %1").arg(mFilePathIn));

	QSharedPointer<DkImageContainer> imgC(new DkImageContainer(mFilePathIn));

	if (!imgC->loadImage() || imgC->image().isNull()) {
		logStrings.append(QObject::tr("Error while loading..."));
		failure++;
		return false;
	}

	for (QSharedPointer<DkAbstractBatch> batch : processFunctions) {

		if (!batch) {
			logStrings.append(QObject::tr("Error: cannot process a NULL function."));
			continue;
		}

		if (!batch->compute(imgC, logStrings)) {
			logStrings.append(QObject::tr("%1 failed").arg(batch->name()));
			failure++;
		}
	}

	deleteExisting();

	if (imgC->saveImage(mFilePathOut, compression))
		logStrings.append(QObject::tr("%1 saved...").arg(mFilePathOut));
	else {
		logStrings.append(QObject::tr("Could not save: %1").arg(mFilePathOut));
		failure++;
	}

	deleteOriginalFile();

	return true;
}

bool DkBatchProcess::renameFile() {

	if (QFileInfo(mFilePathOut).exists()) {
		logStrings.append(QObject::tr("Error: could not rename file, the target file exists already."));
		return false;
	}

	QFile file(mFilePathIn);

	// Note: if two images are renamed at the same time to the same name, one image is lost -> see Qt comment Race Condition
	if (!file.rename(mFilePathOut)) {
		logStrings.append(QObject::tr("Error: could not rename file"));
		logStrings.append(file.errorString());
		return false;
	}
	else
		logStrings.append(QObject::tr("Renaming: %1 -> %2").arg(mFilePathIn).arg(mFilePathOut));

	return true;
}

bool DkBatchProcess::copyFile() {

	QFile file(mFilePathIn);

	if (QFileInfo(mFilePathOut).exists() && mode == DkBatchConfig::mode_overwrite) {
		if (!deleteExisting())
			return false;	// early break
	}

	if (!file.copy(mFilePathOut)) {
		logStrings.append(QObject::tr("Error: could not copy file"));
		logStrings.append(QObject::tr("Input: %1").arg(mFilePathIn));
		logStrings.append(QObject::tr("Output: %1").arg(mFilePathOut));
		logStrings.append(file.errorString());
		return false;
	}
	else
		logStrings.append(QObject::tr("Copying: %1 -> %2").arg(mFilePathIn).arg(mFilePathOut));

	return true;
}

bool DkBatchProcess::deleteExisting() {

	if (QFileInfo(mFilePathOut).exists() && mode == DkBatchConfig::mode_overwrite) {
		QFile file(mFilePathOut);

		if (!file.remove()) {
			logStrings.append(QObject::tr("Error: could not delete existing file"));
			logStrings.append(file.errorString());
			return false;
		}
	}

	return true;
}

bool DkBatchProcess::deleteOriginalFile() {

	if (mFilePathIn == mFilePathOut)
		return true;

	if (!failure && deleteOriginal) {
		QFile oFile(mFilePathIn);

		if (oFile.remove())
			logStrings.append(QObject::tr("%1 deleted.").arg(mFilePathIn));
		else {
			failure++;
			logStrings.append(QObject::tr("I could not delete %1").arg(mFilePathIn));
			return false;
		}
	}
	else if (failure)
		logStrings.append(QObject::tr("I did not delete the original because I detected %1 failure(s).").arg(failure));

	return true;
}

// DkBatchConfig --------------------------------------------------------------------
DkBatchConfig::DkBatchConfig(const QStringList& fileList, const QString& outputDir, const QString& fileNamePattern) {

	mFileList = fileList;
	mOutputDirPath = outputDir;
	mFileNamePattern = fileNamePattern;
	
	init();
};

void DkBatchConfig::init() {

	mCompression = -1;
	mMode = mode_skip_existing;
}

bool DkBatchConfig::isOk() const {

	if (mOutputDirPath.isEmpty())
		return false;

	QDir oDir(mOutputDirPath);

	if (!oDir.exists()) {
		if (!oDir.mkpath("."))
			return false;	// output dir does not exist & I cannot create it
	}

	if (mFileList.empty())
		return false;

	if (mFileNamePattern.isEmpty())
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
	
	QStringList fileList = batchConfig.getFileList();

	for (int idx = 0; idx < fileList.size(); idx++) {

		QFileInfo cFileInfo = QFileInfo(fileList.at(idx));

		DkFileNameConverter converter(cFileInfo.fileName(), batchConfig.getFileNamePattern(), idx);
		QFileInfo newFileInfo(batchConfig.getOutputDirPath(), converter.getConvertedFileName());

		DkBatchProcess cProcess(fileList.at(idx), newFileInfo.absoluteFilePath());
		cProcess.setMode(batchConfig.getMode());
		cProcess.setDeleteOriginal(batchConfig.getDeleteOriginal());
		cProcess.setProcessChain(batchConfig.getProcessFunctions());

		batchItems.push_back(cProcess);
	}
}

void DkBatchProcessing::compute() {

	init();

	qDebug() << "computing...";

	if (batchWatcher.isRunning())
		batchWatcher.waitForFinished();

	QFuture<void> future = QtConcurrent::map(batchItems, &nmc::DkBatchProcessing::computeItem);
	batchWatcher.setFuture(future);
}

bool DkBatchProcessing::computeItem(DkBatchProcess& item) {

	return item.compute();
}

QStringList DkBatchProcessing::getLog() const {

	QStringList log;

	for (DkBatchProcess batch : batchItems) {

		log << batch.getLog();
		log << "";	// add empty line between images
	}

	return log;
}

int DkBatchProcessing::getNumFailures() const {

	int numFailures = 0;

	for (DkBatchProcess batch : batchItems) {
		
		if (batch.hasFailed())
			numFailures++;
	}

	return numFailures;
}

int DkBatchProcessing::getNumProcessed() const {

	int numProcessed = 0;

	for (DkBatchProcess batch : batchItems) {

		if (batch.wasProcessed())
			numProcessed++;
	}

	return numProcessed;
}

QList<int> DkBatchProcessing::getCurrentResults() {

	if (resList.empty()) {
		for (int idx = 0; idx < batchItems.size(); idx++)
			resList.append(batch_item_not_computed);
	}

	for (int idx = 0; idx < resList.size(); idx++) {

		if (resList.at(idx) != batch_item_not_computed)
			continue;

		if (batchItems.at(idx).wasProcessed())
			resList[idx] = batchItems.at(idx).hasFailed() ? batch_item_failed : batch_item_succeeded;
	}

	return resList;
}

QStringList DkBatchProcessing::getResultList() const {

	QStringList results;

	for (DkBatchProcess batch : batchItems) {

		if (batch.wasProcessed())
			results.append(getBatchSummary(batch));
	}

	return results;
}

QString DkBatchProcessing::getBatchSummary(const DkBatchProcess& batch) const {

	QString res = batch.inputFile() + "\t";

	if (!batch.hasFailed())
		res += " <span style=\" color:#00aa00;\">" + tr("[OK]") + "</span>";
	else
		res += " <span style=\" color:#aa0000;\">" + tr("[FAIL]") + "</span>";

	return res;
}

int DkBatchProcessing::getNumItems() const {

	return batchItems.size();
}

bool DkBatchProcessing::isComputing() const {

	return batchWatcher.isRunning();
}

void DkBatchProcessing::cancel() {

	batchWatcher.cancel();
}

}