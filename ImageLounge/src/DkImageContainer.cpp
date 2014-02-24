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
	edited = false;
	
	QSharedPointer<DkBasicLoader> loader(new DkBasicLoader());
	this->loader = loader;
}

bool operator==(const DkImageContainer& lic, const DkImageContainer& ric) {

	return lic.file() == ric.file();
}

bool DkImageContainer::operator<=(const DkImageContainer& o) const {

	if (*this == o)
		return true;

	return *this < o;
}

bool DkImageContainer::operator<(const DkImageContainer& o) const {

	switch(DkSettings::global.sortMode) {

	case DkSettings::sort_filename:

		if (DkSettings::global.sortDir == DkSettings::sort_ascending)
			return DkUtils::compFilename(fileInfo, o.file());
		else
			return DkUtils::compFilenameInv(fileInfo, o.file());
		break;

	case DkSettings::sort_date_created:
		if (DkSettings::global.sortDir == DkSettings::sort_ascending)
			return DkUtils::compDateCreated(fileInfo, o.file());
		else
			return DkUtils::compDateCreatedInv(fileInfo, o.file());
		break;

	case DkSettings::sort_date_modified:
		if (DkSettings::global.sortDir == DkSettings::sort_ascending)
			return DkUtils::compDateModified(fileInfo, o.file());
		else
			return DkUtils::compDateModifiedInv(fileInfo, o.file());
		
	case DkSettings::sort_random:
		return DkUtils::compRandom(fileInfo, o.file());

	default:
		// filename
		return DkUtils::compFilename(fileInfo, o.file());
	}
}

QFileInfo DkImageContainer::file() const {

	return fileInfo;
}

bool DkImageContainer::exists() {

	fileInfo.refresh();
	return fileInfo.exists();
}

QString DkImageContainer::getTitleAttribute() const {

	if (loader->getNumPages() <= 1)
		return QString();

	QString attr = "[" + QString::number(loader->getPageIdx()) + "/" + 
		QString::number(loader->getNumPages()) + "]";

	return attr;
}

QImage DkImageContainer::image() {

	if (loader->image().isNull() && hasImage() == not_loaded)
		loadImage();

	return loader->image();
}

void DkImageContainer::setImage(const QImage& img, const QFileInfo& fileInfo) {

	this->fileInfo = fileInfo;
	loader->setImage(img, fileInfo);
	edited = true;
}

bool DkImageContainer::hasImage() const {

	return loader->hasImage();
}

int DkImageContainer::imgLoaded() const {

	return loadState;
}

bool DkImageContainer::loadImage() {

	if (fileBuffer.isNull())
		fileBuffer = loadFileToBuffer(fileInfo);

	loader = loadImageIntern(fileInfo, fileBuffer);

	return loader->hasImage();
}

QByteArray DkImageContainer::loadFileToBuffer(const QFileInfo fileInfo) {

	QFile file(fileInfo.absoluteFilePath());
	file.open(QIODevice::ReadOnly);

	QByteArray ba;
	ba = file.readAll();

	return ba;
}


QSharedPointer<DkBasicLoader> DkImageContainer::loadImageIntern(const QFileInfo fileInfo, const QByteArray fileBuffer) {

	// checks performed so load the file
	QSharedPointer<DkBasicLoader> basicLoader(new DkBasicLoader());
	
	try {
		basicLoader->loadGeneral(fileInfo, fileBuffer, true);
	} catch(...) {}

	return basicLoader;
}

QFileInfo DkImageContainer::saveImageIntern(const QFileInfo file, QImage saveImg, int compression) {

	loader->save(file, saveImg, compression);
}


// DkImageContainerT --------------------------------------------------------------------
DkImageContainerT::DkImageContainerT(const QFileInfo& file) : DkImageContainer(file) {

	fetchingBuffer = false;
	fetchingImage = false;
	connect(&saveImageWatcher, SIGNAL(finished()), this, SLOT(savingFinished()));
	connect(&bufferWatcher, SIGNAL(finished()), this, SLOT(bufferLoaded()));
	connect(&imageWatcher, SIGNAL(finished()), this, SLOT(imageLoaded()));
	connect(&imageWatcher, SIGNAL(canceled()), this, SLOT(cancelFinished()));
	connect(&imageWatcher, SIGNAL(canceled()), this, SLOT(cancelFinished()));
	//connect(&metaDataWatcher, SIGNAL(finished()), this, SLOT(metaDataLoaded()));
}

DkImageContainerT::~DkImageContainerT() {
	bufferWatcher.blockSignals(true);
	bufferWatcher.cancel();
	imageWatcher.blockSignals(true);
	imageWatcher.cancel();
	//metaDataWatcher.blockSignals(true);
	//metaDataWatcher.cancel();

	saveImageWatcher.waitForFinished();
}

bool DkImageContainerT::loadImageThreaded() {

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

bool DkImageContainerT::saveImageThreaded(const QFileInfo& file, int compression /* = -1 */) {

	return saveImageThreaded(file, loader->image(), compression);
}


bool DkImageContainerT::saveImageThreaded(const QFileInfo& file, const QImage& saveImg, int compression /* = -1 */) {

	if (saveImg.isNull()) {
		QString msg = tr("I can't save an empty file, sorry...\n");
		emit errorDialogSignal(msg);
		return false;
	}
	if (!file.absoluteDir().exists()) {
		QString msg = tr("Sorry, the directory: %1  does not exist\n").arg(file.absolutePath());
		emit errorDialogSignal(msg);
		return false;
	}
	if (file.exists() && !file.isWritable()) {
		QString msg = tr("Sorry, I can't write to the file: %1").arg(file.fileName());
		emit errorDialogSignal(msg);
		return false;
	}

	// TODO: add thumbnail?!
	QFuture<QFileInfo> future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::saveImageIntern, fileInfo, saveImg, compression);

	saveImageWatcher.setFuture(future);
}

void DkImageContainerT::savingFinished() {

	QFileInfo saveFile = saveImageWatcher.result();

	if (!saveFile.exists())
		emit errorDialogSignal(tr("Sorry, I could not save the image"));
	else {
		emit fileSavedSignal(saveFile);
		edited = false;
	}
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

	bufferWatcher.setFuture(future);
}

void DkImageContainerT::bufferLoaded() {

	if (bufferWatcher.isCanceled())
		return;

	fileBuffer = bufferWatcher.result();

	if (imgLoaded() == loading)
		fetchImage();

	fetchingBuffer = false;
}

void DkImageContainerT::fetchImage() {

	if (!loader->hasImage() || fileBuffer.isNull() || loadState == exists_not)
		loadingFinished();
	
	// we have to do our own bool here
	// watcher.isRunning() returns false if the thread is waiting in the pool
	fetchingImage = true;

	QFuture<QSharedPointer<DkBasicLoader> > future = QtConcurrent::run(this, 
		&nmc::DkImageContainerT::loadImageIntern, fileInfo, fileBuffer);

	imageWatcher.setFuture(future);
}

void DkImageContainerT::imageLoaded() {

	if (imageWatcher.isCanceled())
		return;

	// deliver image
	loader = imageWatcher.result();
	connect(loader.data(), SIGNAL(errorDialogSignal(const QString&)), this, SIGNAL(errorDialogSignal(const QString&)));

	fetchingImage = false;

	//if (img.isNull())
		loadingFinished();
	//else
	//	fetchMetaData();
}

void DkImageContainerT::loadingFinished() {

	DkTimer dt;

	if (!loader->hasImage()) {
		QString msg = tr("Sorry, I could not load: %1").arg(fileInfo.fileName());
		emit showInfoSignal(msg);
		emit fileLoadedSignal(false);
		loadState = exists_not;
		return;
	}

	// see if this costs any time
	emit fileLoadedSignal(true);
	loadState = loaded;
	
	qDebug() << "metadata loaded and image rotated in: " << QString::fromStdString(dt.getTotal());
}

void DkImageContainerT::cancel() {

	if (loadState != loading)
		cancelFinished();

	bufferWatcher.cancel();
	imageWatcher.cancel();
}

void DkImageContainerT::cancelFinished() {

	if (bufferWatcher.isCanceled() && imageWatcher.isCanceled())
		loader->release();
}


QByteArray DkImageContainerT::loadFileToBuffer(const QFileInfo fileInfo) {

	return DkImageContainer::loadFileToBuffer(fileInfo);
}

QSharedPointer<DkBasicLoader> DkImageContainerT::loadImageIntern(const QFileInfo fileInfo, const QByteArray fileBuffer) {

	return DkImageContainer::loadImageIntern(fileInfo, fileBuffer);
}

QFileInfo DkImageContainerT::saveImageIntern(const QFileInfo file, QImage saveImg, int compression) {

	return DkImageContainer::saveImageIntern(file, saveImg, compression);
}


};