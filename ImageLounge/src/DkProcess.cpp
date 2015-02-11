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
#include "DkImage.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QUrl>
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
		logStrings.append(QObject::tr("%1 image resized, scale factor: %2%%").arg(name()).arg(scaleFactor*100.0f));
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
		logStrings.append(QObject::tr("%1 already exists -> skipping (check 'overwrite' if you want to overwrite the file)").arg(fileInfoOut.absoluteFilePath()));
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
	if (processFunctions.empty() && fileInfoIn.absolutePath() == fileInfoOut.absolutePath() && fileInfoIn.suffix() == fileInfoOut.suffix()) {	// rename?
		renameFile();
		return;
	}
	else if (processFunctions.empty() && fileInfoIn.suffix() == fileInfoOut.suffix()) {	// copy?
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
			logStrings.append(QObject::tr("%1 failed").arg(batch->name()));
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

	// Note: if two images are renamed at the same time to the same name, one image is lost -> see Qt comment Race Condition
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
	init();
};

void DkBatchConfig::init() {

	compression = -1;
	mode = mode_skip_existing;
}

bool DkBatchConfig::isOk() const {

	if (!outputDir.exists()) {
		if (!outputDir.mkpath("."))
			return false;	// output dir does not exist & I cannot create it
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
	
	QList<QUrl> urls = batchConfig.getUrls();

	for (int idx = 0; idx < urls.size(); idx++) {

		QFileInfo cFileInfo(urls.at(idx).toLocalFile());

		DkFileNameConverter converter(cFileInfo.fileName(), batchConfig.getFileNamePattern(), idx);
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

	if (batchWatcher.isRunning())
		batchWatcher.waitForFinished();

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