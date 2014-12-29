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

	mode = mode_skip_existing;
}

void DkBatchProcess::setProcessChain(const QVector<DkAbstractBatch*> processes) {

	this->processFunctions = processes;
}

void DkBatchProcess::setMode(int mode) {

	this->mode = mode;
}

void DkBatchProcess::compute() {

	// check errors
	if (fileInfoOut.exists() && mode == mode_skip_existing) {
		logStrings.append(QObject::tr("Skipping because file already exists."));
		return;
	}
	else if (fileInfoIn.exists()) {
		logStrings.append(QObject::tr("Error: input file does not exist"));
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
	else if (processFunctions.empty())	{	// copy?
		copyFile();
		return;
	}

	process();
}

QStringList DkBatchProcess::getLog() const {

	return logStrings;
}

bool DkBatchProcess::process() {

	QSharedPointer<DkImageContainer> imgC(new DkImageContainer(fileInfoIn));

	if (!imgC->loadImage() || imgC->image().isNull()) {
		logStrings.append(QObject::tr("Error while loading..."));
		return false;
	}

	for (DkAbstractBatch* batch : processFunctions) {

		if (!batch) {
			logStrings.append(QObject::tr("Warning: cannot compute, NULL process"));
			continue;
		}

		if (!batch->compute(imgC, logStrings)) {
			logStrings.append(QObject::tr("Warning: %1 failed").arg(batch->name()));
		}
	}

	imgC->saveImage(fileInfoOut, compression);

	return true;
}

bool DkBatchProcess::renameFile() {

	// if processes are empty - it is a simple copy operation
	QFile file(fileInfoIn.absoluteFilePath());

	if (!file.rename(fileInfoOut.absoluteFilePath())) {
		logStrings.append(QObject::tr("Error: could not rename file"));
		logStrings.append(file.errorString());
		return false;
	}
	else
		logStrings.append(QObject::tr("Success"));

	return true;
}

bool DkBatchProcess::copyFile() {

	// if processes are empty - it is a simple copy operation
	QFile file(fileInfoIn.absoluteFilePath());

	if (fileInfoOut.exists() && mode == mode_overwrite) {
		if (!deleteExisting())
			return false;	// early break
	}

	if (!file.copy(fileInfoOut.absoluteFilePath())) {
		logStrings.append(QObject::tr("Error: could not copy file"));
		logStrings.append(file.errorString());
		return false;
	}
	else
		logStrings.append(QObject::tr("Success"));

	return true;
}

bool DkBatchProcess::deleteExisting() {

	if (fileInfoOut.exists() && mode == mode_overwrite) {
		QFile file(fileInfoIn.absoluteFilePath());

		if (!file.remove()) {
			logStrings.append(QObject::tr("Error: could not delete existing file"));
			logStrings.append(file.errorString());
			return false;
		}
	}

	return true;
}


}