/*******************************************************************************************************
 DkImageContainer.cpp
 Created on:	21.02.2014
 
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

#include "DkImageContainer.h"

namespace nmc {

// DkImageContainer --------------------------------------------------------------------
DkImageContainer::DkImageContainer(const QFileInfo& fileInfo) {

	this->fileInfo = fileInfo;
	loadState = not_loaded;
}

bool DkImageContainer::operator==(const DkImageContainer& o) {

	return fileInfo == o.file();
}

QFileInfo DkImageContainer::file() const {

	return fileInfo;
}

QImage DkImageContainer::image() {

	if (img.isNull() && hasImage() == not_loaded)
		loadFile();

	return img;
}

int DkImageContainer::hasImage() const {

	return loadState;
}

void DkImageContainer::loadFile() {

	DkBasicLoader basicLoader;

	if (fileBuffer.isNull())
		fileBuffer = loadFileToBuffer(fileInfo);

	img = loadImage(fileInfo, fileBuffer);

	metaData.setBuffer(fileBuffer);
	int orientation = metaData.getOrientation();

	if (!metaData.isTiff() && !DkSettings::metaData.ignoreExifOrientation)
		basicLoader.rotate(orientation);
	
}

QByteArray DkImageContainer::loadFileToBuffer(const QFileInfo fileInfo) {

	QFile file(fileInfo.absoluteFilePath());
	file.open(QIODevice::ReadOnly);

	QByteArray ba;
	ba = file.readAll();

	return ba;
}


QImage DkImageContainer::loadImage(const QFileInfo fileInfo, const QByteArray fileBuffer) {

	// checks performed so load the file
	DkBasicLoader basicLoader;
	
	try {
		basicLoader.loadGeneral(fileInfo, fileBuffer);
	} catch(...) {}

	return basicLoader.image();
}




// DkImageContainerT --------------------------------------------------------------------
DkImageContainerT::DkImageContainerT(const QFileInfo& file) : DkImageContainer(file) {

	fetchingBuffer = false;
	fetchingImage = false;
	connect(&bufferWatcher, SIGNAL(finished()), this, SLOT(bufferLoaded()));
	connect(&imageWatcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
	//connect(&metaDataWatcher, SIGNAL(finished()), this, SLOT(metaDataLoaded()));
}

DkImageContainerT::~DkImageContainerT() {
	bufferWatcher.blockSignals(true);
	bufferWatcher.cancel();
	imageWatcher.blockSignals(true);
	imageWatcher.cancel();
	//metaDataWatcher.blockSignals(true);
	//metaDataWatcher.cancel();
}

bool DkImageContainerT::loadImage() {

	fileInfo.refresh();

	// null file?
	if (fileInfo.fileName().isEmpty() || !fileInfo.exists()) {

		QString msg = tr("Sorry, the file: %1 does not exist... ").arg(fileInfo.fileName());
		emit showInfoSignal(msg);
		loadState = exists_not;
		return false;
	}
	else if (!fileInfo.permission(QFile::ReadUser)) {

		QString msg = tr("Sorry, you are not allowed to read: %1").arg(fileInfo.fileName());
		emit showInfoSignal(msg);
		loadState = exists_not;
		return false;
	}

	loadState = loading;
	fetchFile();
	return true;
}

void DkImageContainerT::fetchFile() {

	// ignore doubled calls
	if (fetchingBuffer)
		return;
	else if (!fileBuffer.isEmpty())
		bufferLoaded();

	// we have to do our own bool here
	// watcher.isRunning() returns false if the thread is waiting in the pool
	fetchingBuffer = true;

	QFuture<QByteArray> future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::loadFileToBuffer, fileInfo);

	//bufferWatcher.setFuture(future);
}

void DkImageContainerT::bufferLoaded() {

	fileBuffer = bufferWatcher.result();

	if (hasImage() == loading)
		fetchImage();

	fetchingBuffer = false;
}

void DkImageContainerT::fetchImage() {

	if (!img.isNull() || fileBuffer.isNull() || loadState == exists_not)
		loadingFinished();
	
	// we have to do our own bool here
	// watcher.isRunning() returns false if the thread is waiting in the pool
	fetchingImage = true;

	QFuture<QImage> future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::loadImage, fileInfo, fileBuffer);

	imageWatcher.setFuture(future);
}

void DkImageContainerT::imageLoaded() {

	// deliver image
	img = imageWatcher.result();
	fetchingImage = false;

	//if (img.isNull())
		loadingFinished();
	//else
	//	fetchMetaData();
}

void DkImageContainerT::loadingFinished() {

	if (img.isNull()) {
		QString msg = tr("Sorry, I could not load: %1").arg(fileInfo.fileName());
		emit showInfoSignal(msg);
		emit fileLoadedSignal(fileInfo, false);
		return;
	}

	// see if this costs any time
	metaData.setBuffer(fileBuffer);
	int orientation = metaData.getOrientation();

	DkBasicLoader basicLoader;
	if (!metaData.isTiff() && !DkSettings::metaData.ignoreExifOrientation)
		basicLoader.rotate(orientation);

	emit fileLoadedSignal(fileInfo, true);
}

QByteArray DkImageContainerT::loadFileToBuffer(const QFileInfo fileInfo) {

	return DkImageContainer::loadFileToBuffer(fileInfo);
}

QImage DkImageContainerT::loadImage(const QFileInfo fileInfo, const QByteArray fileBuffer) {

	return DkImageContainer::loadImage(fileInfo, fileBuffer);
}


};